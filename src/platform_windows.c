/*  @file platform_windows.c
 *
 *  @brief code specific to the platform: windows, abstracted.
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
#include "h/logger.h"
#include "h/memory.h"
#include "h/process.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <windows.h>
#include <direct.h>

u32 _fsl_get_path_absolute(const str *name, str *dst)
{
    if (!GetFullPathNameA(name, PATH_MAX, dst, NULL))
    {
        fsl_err = FSL_ERR_GET_PATH_ABSOLUTE_FAIL;
        return fsl_err;
    }

    posix_slash(dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_get_path_bin_root(str *dst)
{
    if (strlen(_pgmptr) + 1 >= PATH_MAX)
    {
        _LOGFATAL(FSL_ERR_GET_PATH_BIN_ROOT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed 'get_path_bin_root()', Process Aborted");
        return fsl_err;
    }
    strncpy(dst, _pgmptr, PATH_MAX);
    fsl_retract_path(dst);
    fsl_posix_slash(dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_exec(fsl_buf *cmd, str *cmd_name)
{
    STARTUPINFOA        startup_info = {0};
    PROCESS_INFORMATION process_info = {0};
    DWORD exit_code = 0;
    str *cmd_cat = NULL;
    u32 i = 0;

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (!cmd->loaded || !cmd->buf)
    {
        _LOGERROR(FSL_ERR_BUFFER_EMPTY, 0,
                "Failed to Execute '%s', cmd Empty\n", cmd_name);
        return fsl_err;
    }

    if (fsl_mem_alloc((void*)&cmd_cat, cmd->size * cmd->memb,
            stringf("exec().%s", cmd_name)) != FSL_ERR_SUCCESS)
        return fsl_err;

    for (i = 0; i < cmd->memb; ++i)
        strncat(cmd_cat, stringf("%s ", cmd->i[i]), cmd->size);

    if(!CreateProcessA(NULL, cmd_cat, NULL, NULL, FALSE, 0, NULL, NULL,
                &startup_info, &process_info))
    {
        _LOGFATAL(FSL_ERR_EXEC_FAIL, 0,
                "Failed to Fork '%s', Process Aborted\n", cmd_name);
        goto cleanup;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);
    GetExitCodeProcess(process_info.hProcess, &exit_code);
    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    if (exit_code == 0)
        _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
                "'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
    else
    {
        _LOGDEBUG(0,
                "'%s' Exit Code: %d\n", cmd_name, exit_code);
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

u64 _fsl_mem_request_page_size(void)
{
    static SYSTEM_INFO _si;
    GetSystemInfo(&si);
    return (u64)si.dwPageSize;
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

    temp = VirtualAlloc(NULL, size_aligned, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!temp)
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

    if (!x || !*X || !offset)
    {
        _LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                "%s[%p][%p] Failed to Commit Memory [%"PRIu64"B], Pointer NULL\n",
                name, x, offset, size);
        return fsl_err;
    }

    fsl_mem_request_page_size();
    size_aligned = fsl_align_up_u64(size, FSL_PAGE_SIZE);

    if (!VirtualAlloc((*(u8*)x + (u8*)offset), size_aligned, MEM_COMMIT, PAGE_READWRITE))
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

/* TODO: make '_fsl_mem_remap()' for windows */
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
    if (!x || !*x) return;

    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Unmapped [%"PRIu64"B]\n", name, *x, size);

    VirtualFree(x, 0, MEM_RELEASE);
    *x = NULL;
}

void _fsl_mem_unmap_arena(fsl_mem_arena *x, const str *name, const str *file, u64 line)
{
    if (!x || !x->buf) return;

    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Arena Unmapped [%"PRIu64"B] Memb Total [%"PRIu64"][%"PRIu64"B]\n",
            name, x->buf, x->size_buf, x->memb, x->size_i);

    for (i = 0; i < x->memb; ++i)
        *x->i[i] = NULL;

    VirtualFree(x->i, 0, MEM_RELEASE);
    VirtualFree(x->buf, 0, MEM_RELEASE);

    *x = (fsl_mem_arena){0};
}
