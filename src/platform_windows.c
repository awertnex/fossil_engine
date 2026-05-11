/*!
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
 *  limitations under the License.
 */

/*!
 *  @file platform_windows.c
 *
 *  @brief code specific to the platform: windows, abstracted.
 */

#include "common/diagnostics.h"
#include "common/limits.h"
#include "common/types.h"
#include "logger/logger.h"
#include "memory/memory.h"

#include "h/dir.h"
#include "h/process.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <windows.h>
#include <direct.h>

u32 fsl_get_path_absolute_internal(const fsl_fs_path *fs_path, str *dst)
{
    if (!GetFullPathNameA(fs_path, PATH_MAX, dst, NULL))
    {
        fsl_err = FSL_ERR_GET_PATH_ABSOLUTE_FAIL;
        return fsl_err;
    }

    posix_slash(dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_get_path_bin_root_internal(str *dst)
{
    /* here, "PATH_MAX - 2" to leave space for a slash (`/`) and a null (`\0`) terminator */
    if (strlen(_pgmptr) > PATH_MAX - 2)
    {
        LOGFATAL(FSL_ERR_GET_PATH_BIN_ROOT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_FATAL("Get Binary Root", "`fsl_get_path_bin_root_internal()` Failed"));
        return fsl_err;
    }
    memcpy(dst, _pgmptr, PATH_MAX - 2);
    fsl_retract_path(dst);
    fsl_posix_slash(dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_exec(fsl_buf *cmd, str *cmd_name)
{
    STARTUPINFOA startup_info = {0};
    PROCESS_INFORMATION process_info = {0};
    DWORD exit_code = 0;
    str *cmd_cat = NULL;
    u32 i = 0;

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (!cmd->loaded || !cmd->buf)
    {
        LOGERROR(FSL_ERR_BUFFER_EMPTY, 0,
                MSG_ACTION_SUBJECT_REASON_ERROR("Execute CMD", cmd_name, "`cmd` Empty"));
        return fsl_err;
    }

    if (fsl_mem_alloc((void*)&cmd_cat, cmd->size * cmd->memb,
            fsl_stringf("exec().%s", cmd_name)) != FSL_ERR_SUCCESS)
        return fsl_err;

    for (i = 0; i < cmd->memb; ++i)
        strncat(cmd_cat, fsl_stringf("%s ", cmd->i[i]), cmd->size);

    if(!CreateProcessA(NULL, cmd_cat, NULL, NULL, FALSE, 0, NULL, NULL,
                &startup_info, &process_info))
    {
        LOGFATAL(FSL_ERR_PROCESS_FORK_FAIL, 0,
                MSG_ACTION_SUBJECT_REASON_ERROR("Execute CMD", cmd_name, "`CreateProcessA()` Failed"));
        goto cleanup;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);
    GetExitCodeProcess(process_info.hProcess, &exit_code);
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    if (exit_code == 0)
        LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_EXEC(cmd_name, exit_code));
    else
    {
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_EXEC(cmd_name, exit_code));
        fsl_err = FSL_ERR_EXEC_PROCESS_NON_ZERO;
        goto cleanup;
    }

    fsl_mem_free((void*)&cmd_cat, cmd->memb * cmd->size, stringf("exec().%s", cmd_name));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mem_free((void*)&cmd_cat, cmd->memb * cmd->size, stringf("exec().%s", cmd_name));
    return fsl_err;
}

u32 fsl_mem_map_internal(void **x, u64 size,
        const str *name, const str *file, u64 line)
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

    temp = VirtualAlloc(NULL, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!temp)
    {
        LOGERROREX(FSL_ERR_MEM_MAP_FAIL, 0,
                file, line,
                MSG_MEM_MAP_REASON_FAIL(name, *x, size, "`VirtualAlloc()` Failed"));
        return fsl_err;
    }

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_MAP(name, temp, size));
    *x = temp;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_mem_commit_internal(void **x, void *offset, u64 size,
        const str *name, const str *file, u64 line)
{
    if (!x || !*X || !offset)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_COMMIT_REASON_FAIL(name, x, offset, size, "Pointer `NULL`"));
        return fsl_err;
    }

    if (!VirtualAlloc((*(u8*)x + (u8*)offset), size, MEM_COMMIT, PAGE_READWRITE))
    {
        LOGERROREX(FSL_ERR_MEM_COMMIT_FAIL, 0,
                file, line,
                MSG_MEM_COMMIT_REASON_FAIL(name, *x, offset, size, "`VirtualAlloc()` Failed"));
        return fsl_err;
    }

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_COMMIT(name, *x, offset, size));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

/* TODO: make 'fsl_mem_remap_internal()' for windows */
u32 fsl_mem_remap_internal(void **x, u64 size_old, u64 size_new,
        const str *name, const str *file, u64 line)
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

void fsl_mem_unmap_internal(void **x, u64 size,
        const str *name, const str *file, u64 line)
{
    if (!x || !*x) return;

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_UNMAP(name, *x, size));

    VirtualFree(x, 0, MEM_RELEASE);
    *x = NULL;
}

void fsl_mem_arena_free_internal(fsl_mem_arena *x,
        const str *name, const str *file, u64 line)
{
    fsl_mem_arena nomem_arena = {0};

    if (!x || !x->buf) return;

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_ARENA_FREE(name, x->buf, x->size_buf, x->entry_count, x->entry_cap));

    VirtualFree(x->buf, 0, MEM_RELEASE);
    VirtualFree(x->freelist, 0, MEM_RELEASE);
    VirtualFree(x->entry, 0, MEM_RELEASE);
    *x = nomem_arena;
}
