/*  @file platform_linux.c
 *
 *  @brief code specific to the platform: linux, abstracted.
 *
 *  Copyright 2026 Lily Awertnex
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.OFTWARE.
 */

#include "h/common.h"

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "logger/log.h"
#include "h/math.h"
#include "h/process.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>

u32 _fsl_get_path_absolute(const str *name, str *dst)
{
    if (!realpath(name, dst))
    {
        fsl_err = FSL_ERR_GET_PATH_ABSOLUTE_FAIL;
        return fsl_err;
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_get_path_bin_root(str *dst)
{
    /* here, "PATH_MAX - 2" to leave space for a slash (`/`) and a null (`\0`) terminator */
    if (readlink("/proc/self/exe", dst, PATH_MAX - 2) < 1)
    {
        LOGFATAL(FSL_ERR_GET_PATH_BIN_ROOT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_FATAL("Get Binary Root", "`_fsl_get_path_bin_root()` Failed"));
        return fsl_err;
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_exec(fsl_buf *cmd, str *cmd_name)
{
    pid_t pid = fork();
    int status = 0;
    int exit_code = 0;
    int sig = 0;

    if (!cmd->loaded || !cmd->buf)
    {
        LOGERROR(FSL_ERR_BUFFER_EMPTY, 0,
                MSG_ACTION_SUBJECT_REASON_ERROR("Execute CMD", cmd_name, "`cmd` Empty"));
        return fsl_err;
    }

    if (pid < 0)
    {
        LOGERROR(FSL_ERR_PROCESS_FORK_FAIL, 0,
                MSG_ACTION_SUBJECT_REASON_ERROR("Execute CMD", cmd_name, "`fork()` Failed"));
        return fsl_err;
    }
    else if (pid == 0)
    {
        execvp((const str*)cmd->i[0], (str *const *)cmd->i);
        LOGERROR(FSL_ERR_EXEC_FAIL, 0,
                MSG_ACTION_SUBJECT_REASON_ERROR("Execute CMD", cmd_name, "`execvp()` Failed"));
        return fsl_err;
    }

    if (waitpid(pid, &status, 0) == -1)
    {
        LOGERROR(FSL_ERR_WAITPID_FAIL, 0,
                MSG_ACTION_SUBJECT_REASON_ERROR("Execute CMD", cmd_name, "`waitpid()` Failed"));
        return fsl_err;
    }

    if (WIFEXITED(status))
    {
        exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
            LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
                    MSG_EXEC(cmd_name, exit_code));
        else
        {
            LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
                    MSG_EXEC(cmd_name, exit_code));
            fsl_err = FSL_ERR_EXEC_PROCESS_NON_ZERO;
            return fsl_err;
        }
    }
    else if (WIFSIGNALED(status))
    {
        sig = WTERMSIG(status);
        LOGERROR(FSL_ERR_EXEC_SIGTERM, 0,
                MSG_EXEC_SIGTERM(cmd_name, sig));
        return fsl_err;
    }
    else
    {
        LOGERROR(FSL_ERR_EXEC_ABNORMAL_EXIT, 0,
                MSG_EXEC_ABNORMAL_EXIT(cmd_name));
        return fsl_err;
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_map(void **x, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;

    if (!x)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_MAP_REASON_FAIL(name, *x, size, "Pointer `NULL`"));
        return fsl_err;
    }
    if (*x)
    {
        LOGERROREX(FSL_ERR_POINTER_NOT_NULL, 0,
                file, line,
                MSG_MEM_MAP_REASON_FAIL(name, *x, size, "Memory Already Mapped"));
        return fsl_err;
    }

    if (size == 0)
    {
        LOGERROREX(FSL_ERR_SIZE_TOO_SMALL, 0,
                file, line,
                MSG_MEM_MAP_REASON_FAIL(name, *x, size, "Size Too Small"));
        return fsl_err;
    }

    temp = mmap(NULL, size,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (temp == MAP_FAILED)
    {
        LOGERROREX(FSL_ERR_MEM_MAP_FAIL, 0,
                file, line,
                MSG_MEM_MAP_REASON_FAIL(name, *x, size, "`mmap()` Failed"));
        return fsl_err;
    }

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_MAP(name, temp, size));
    *x = temp;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_commit(void **x, void *offset, u64 size, const str *name, const str *file, u64 line)
{
    if (!x || !*x || !offset)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_COMMIT_REASON_FAIL(name, x, offset, size, "Pointer `NULL`"));
        return fsl_err;
    }

    if (mprotect(offset, size, PROT_READ | PROT_WRITE) != 0)
    {
        LOGERROREX(FSL_ERR_MEM_COMMIT_FAIL, 0,
                file, line,
                MSG_MEM_COMMIT_REASON_FAIL(name, *x, offset, size, "`mprotect()` Failed"));
        return fsl_err;
    }

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_COMMIT(name, *x, offset, size));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_remap(void **x, u64 size_old, u64 size_new, const str *name, const str *file, u64 line)
{
    void *temp = NULL;

    if (!x || !*x)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_REMAP_REASON_FAIL(name, NULL, size_old, "Pointer `NULL`"));
        return fsl_err;
    }

    temp = mremap(*x, size_old, size_new, MREMAP_MAYMOVE);
    if (temp == MAP_FAILED)
    {
        LOGERROREX(FSL_ERR_MEM_REMAP_FAIL, 0,
                file, line,
                MSG_MEM_REMAP_REASON_FAIL(name, *x, size_old, "`mremap()` Failed"));
        return fsl_err;
    }

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_REMAP(name, *x, temp, size_old, size_new));

    *x = temp;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void _fsl_mem_unmap(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (!x || !*x) return;

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_UNMAP(name, *x, size));

    munmap(*x, size);
    *x = NULL;
}

void _fsl_mem_unmap_arena(fsl_mem_arena *x, const str *name, const str *file, u64 line)
{
    u64 i = 0;
    fsl_mem_arena nomem_arena = {0};

    if (!x || !x->buf) return;

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_ARENA_UNMAP(name, x->buf, x->size_buf, x->memb, x->size_i));

    for (i = 0; i < x->memb; ++i)
        *x->i[i] = NULL;

    munmap(x->i, x->size_i);
    munmap(x->buf, x->size_buf);

    *x = nomem_arena;
}
