#ifndef BUILD_PLATFORM_WIN_H
#define BUILD_PLATFORM_WIN_H

/* ---- section: license ---------------------------------------------------- */

/*  MIT License
 *
 *  Copyright (c) 2026 Lily Awertnex
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include "common.h"

#include <windows.h>
#include <direct.h>

#include "logger.h"
#include "memory.h"

/* ---- section: implementation --------------------------------------------- */

u32 make_dir(const str *path)
{
    if (mkdir(path) == 0)
    {
        LOGTRACE(FALSE,
                "Directory Created '%s'\n", path);
        build_err = ERR_SUCCESS;
        return build_err;
    }

    switch (errno)
    {
        case EEXIST:
            build_err = ERR_DIR_EXISTS;
            break;

        default:
            LOGERROR(ERR_DIR_CREATE_FAIL, TRUE,
                    "Failed to Create Directory '%s'\n", path);
    }

    return build_err;
}

int change_dir(const str *path)
{
    int success = _chdir(path);
    LOGTRACE(TRUE,
            "Working Directory Changed to '%s'\n", path);
    return success;
}

u32 _get_path_absolute(const str *path, str *path_real)
{
    if (!GetFullPathNameA(path, PATH_MAX, path_real, NULL))
    {
        build_err = ERR_GET_PATH_ABSOLUTE_FAIL;
        return build_err;
    }

    posix_slash(path_real);
    build_err = ERR_SUCCESS;
    return build_err;
}

u32 _get_path_bin_root(str *path)
{
    if (strlen(_pgmptr) + 1 >= PATH_MAX)
    {
        LOGFATAL(ERR_GET_PATH_BIN_ROOT_FAIL, FALSE,
                "%s\n", "Failed 'get_path_bin_root()', Process Aborted");
        return build_err;
    }
    strncpy(path, _pgmptr, PATH_MAX);
    retract_path(path);
    posix_slash(path);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 exec(_buf *cmd, str *cmd_name)
{
    STARTUPINFOA        startup_info = {0};
    PROCESS_INFORMATION process_info = {0};
    DWORD exit_code = 0;
    str *cmd_cat = NULL;
    u32 i;

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);

    if (!cmd->loaded || !cmd->buf)
    {
        LOGERROR(ERR_PROCESS_FORK_FAIL, TRUE,
                "Failed to Fork '%s'\n", cmd_name);
        return build_err;
    }

    if (mem_alloc((void*)&cmd_cat, cmd->size * cmd->memb,
            stringf("exec().%s", cmd_name)) != ERR_SUCCESS)
        return build_err;

    for (i = 0; i < cmd->memb; ++i)
        strncat(cmd_cat, stringf("%s ", cmd->i[i]), cmd->size);

    if(!CreateProcessA(NULL, cmd_cat, NULL, NULL, FALSE, 0, NULL, NULL,
                &startup_info, &process_info))
    {
        LOGFATAL(ERR_EXEC_FAIL, TRUE,
                "Failed to Fork '%s', Process Aborted\n", cmd_name);
        goto cleanup;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);

    GetExitCodeProcess(process_info.hProcess, &exit_code);

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    if (exit_code == 0)
        LOGINFO(FALSE,
                "'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
    else
    {
        LOGINFO(TRUE,
                "'%s' Exit Code: %d\n", cmd_name, exit_code);
        build_err = ERR_EXEC_PROCESS_NON_ZERO;
        goto cleanup;
    }

    mem_free((void*)&cmd_cat, cmd->memb * cmd->size, stringf("exec().%s", cmd_name));

    build_err = ERR_SUCCESS;
    return build_err;

cleanup:

    mem_free((void*)&cmd_cat, cmd->memb * cmd->size, stringf("exec().%s", cmd_name));
    return build_err;
}

#endif /* BUILD_PLATFORM_WIN_H */
