/*  Copyright 2026 Lily Awertnex
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

/*
 *	platform_linux.c - code specific to the platform: linux, abstracted
 */

#include "h/common.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/math.h"
#include "h/process.h"

u32 fsl_make_dir(const str *path)
{
    if (mkdir(path, 0755) == 0)
    {
        _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                "Directory Created '%s'\n", path);

        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }

    switch (errno)
    {
        case EEXIST:
            fsl_err = FSL_ERR_DIR_EXISTS;
            break;

        default:
            _LOGERROR(FSL_ERR_DIR_CREATE_FAIL, 0,
                    "Failed to Create Directory '%s'\n", path);
    }

    return fsl_err;
}

int fsl_change_dir(const str *path)
{
    int success = chdir(path);
    _LOGTRACE(0, "Working Directory Changed to '%s'\n", path);
    return success;
}

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
    if (!readlink("/proc/self/exe", dst, PATH_MAX))
    {
        _LOGFATAL(FSL_ERR_GET_PATH_BIN_ROOT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed 'get_path_bin_root()', Process Aborted");
        return fsl_err;
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_exec(fsl_buf *cmd, str *cmd_name)
{
    pid_t pid = fork();
    int status, exit_code = 0, sig;

    if (pid < 0)
    {
        _LOGERROR(FSL_ERR_PROCESS_FORK_FAIL, 0,
                "Failed to Fork '%s'\n", cmd_name);
        return fsl_err;
    }
    else if (pid == 0)
    {
        execvp((const str*)cmd->i[0], (str *const *)cmd->i);
        _LOGERROR(FSL_ERR_EXEC_FAIL, 0,
                "Failed '%s'\n", cmd_name);
        return fsl_err;
    }

    if (waitpid(pid, &status, 0) == -1)
    {
        _LOGERROR(FSL_ERR_WAITPID_FAIL, 0,
                "Failed to Waitpid '%s'\n", cmd_name);
        return fsl_err;
    }

    if (WIFEXITED(status))
    {
        exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
            _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
                    "'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
        else
        {
            _LOGDEBUG(0,
                    "'%s' Exit Code: %d\n", cmd_name, exit_code);
            fsl_err = FSL_ERR_EXEC_PROCESS_NON_ZERO;
            return fsl_err;
        }
    }
    else if (WIFSIGNALED(status))
    {
        sig = WTERMSIG(status);
        _LOGFATAL(FSL_ERR_EXEC_TERMINATE_BY_SIGNAL, 0,
                "'%s' Terminated by Signal: %d, Process Aborted\n", cmd_name, sig);
        return fsl_err;
    }
    else
    {
        _LOGERROR(FSL_ERR_EXEC_ABNORMAL_EXIT, 0,
                "'%s' Exited Abnormally\n", cmd_name);
        return fsl_err;
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u64 _fsl_mem_request_page_size(void)
{
    return sysconf(_SC_PAGESIZE);
}

u32 _fsl_mem_map(void **x, u64 size, const str *name, const str *file, u64 line)
{
    u64 size_aligned = 0;
    void *temp = NULL;

    if (!x)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return fsl_err;
    }
    if (*x)
    {
        fsl_err = FSL_ERR_POINTER_NOT_NULL;
        return fsl_err;
    }

    fsl_mem_request_page_size();
    size_aligned = fsl_align_up_u64(size, FSL_PAGE_SIZE);

    temp = mmap(NULL, size_aligned,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (temp == MAP_FAILED)
    {
        _LOGFATALEX(FSL_ERR_MEM_MAP_FAIL, 0,
                file, line,
                "%s[%p] Failed to Map Memory, Process Aborted\n", name, *x);
        return fsl_err;
    }
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Mapped [%"PRIu64"B]\n", name, temp, size_aligned);

    *x = temp;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_commit(void **x, void *offset, u64 size, const str *name, const str *file, u64 line)
{
    u64 size_aligned = 0;

    if (!x || !*x || !offset)
    {
        _LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                "%s[%p][%p] Failed to Commit Memory [%"PRIu64"B], Pointer NULL\n",
                name, x, offset, size);
        return fsl_err;
    }

    fsl_mem_request_page_size();
    size_aligned = fsl_align_up_u64(size, FSL_PAGE_SIZE);

    if (mprotect(offset, size_aligned, PROT_READ | PROT_WRITE) != 0)
    {
        _LOGFATALEX(FSL_ERR_MEM_COMMIT_FAIL, 0,
                file, line,
                "%s[%p][%p] Failed to Commit Memory [%"PRIu64"B], Process Aborted\n",
                name, *x, offset, size_aligned);
        return fsl_err;
    }
    _LOGTRACEEX(0,
            file, line, "%s[%p][%p] Memory Committed [%"PRIu64"B]\n",
            name, *x, offset, size_aligned);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_remap(void **x, u64 size_old, u64 size_new, const str *name, const str *file, u64 line)
{
    u64 size_old_aligned = 0;
    u64 size_new_aligned = 0;
    void *temp = NULL;

    if (!x || !*x)
    {
        _LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                "%s[%p] Failed to Remap Memory, Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    fsl_mem_request_page_size();
    size_old_aligned = fsl_align_up_u64(size_old, FSL_PAGE_SIZE);
    size_new_aligned = fsl_align_up_u64(size_new, FSL_PAGE_SIZE);

    temp = mremap(*x, size_old_aligned, size_new_aligned, MREMAP_MAYMOVE);
    if (temp == MAP_FAILED)
    {
        _LOGERROREX(FSL_ERR_MEM_REMAP_FAIL, 0,
                file, line,
                "%s[%p] Failed to Remap Memory\n", name, *x);
        return fsl_err;
    }
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Remapped [%"PRIu64"B] -> [%"PRIu64"B]\n",
            name, *x, size_old_aligned, size_new_aligned);

    *x = temp;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void _fsl_mem_unmap(void **x, u64 size, const str *name, const str *file, u64 line)
{
    u64 size_aligned = 0;

    if (!x || !*x) return;

    fsl_mem_request_page_size();
    size_aligned = fsl_align_up_u64(size, FSL_PAGE_SIZE);
    munmap(*x, size_aligned);
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Unmapped [%"PRIu64"B]\n", name, *x, size_aligned);
    *x = NULL;
}

void _fsl_mem_unmap_arena(fsl_mem_arena *x, const str *name, const str *file, u64 line)
{
    if (!x || !x->buf) return;
    munmap(x->i, x->size_i);
    munmap(x->buf, x->size_buf);
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Arena Unmapped [%"PRIu64"B] Memb Total [%"PRIu64"][%"PRIu64"B]\n",
            name, x->buf, x->size_buf, x->memb, x->size_i);
    *x = (fsl_mem_arena){0};
}
