#ifndef BUILD_PLATFORM_LINUX_H
#define BUILD_PLATFORM_LINUX_H

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

#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "logger.h"
#include "memory.h"

/* ---- section: implementation --------------------------------------------- */

u32 make_dir(const str *path)
{
    if (mkdir(path, 0755) == 0)
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
    int success = chdir(path);
    LOGTRACE(TRUE,
            "Working Directory Changed to '%s'\n", path);
    return success;
}

u32 _get_path_absolute(const str *path, str *path_real)
{
    if (!realpath(path, path_real))
    {
        build_err = ERR_GET_PATH_ABSOLUTE_FAIL;
        return build_err;
    }

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 _get_path_bin_root(str *path)
{
    if (!readlink("/proc/self/exe", path, PATH_MAX))
    {
        LOGFATAL(ERR_GET_PATH_BIN_ROOT_FAIL, FALSE,
                "%s\n", "Failed 'get_path_bin_root()', Process Aborted");
        return build_err;
    }

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 exec(_buf *cmd, str *cmd_name)
{
    pid_t pid = fork();
    int status, exit_code = 0, sig;

    if (pid < 0)
    {
        LOGERROR(ERR_PROCESS_FORK_FAIL, TRUE,
                "Failed to Fork '%s'\n", cmd_name);
        return build_err;
    }
    else if (pid == 0)
    {
        execvp((const str*)cmd->i[0], (str *const *)cmd->i);
        LOGERROR(ERR_EXEC_FAIL, TRUE,
                "Failed '%s'\n", cmd_name);
        return build_err;
    }

    if (waitpid(pid, &status, 0) == -1)
    {
        LOGERROR(ERR_WAITPID_FAIL, TRUE,
                "Failed to Waitpid '%s'\n", cmd_name);
        return build_err;
    }

    if (WIFEXITED(status))
    {
        exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
        {
            LOGINFO(FALSE,
                    "'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
        }
        else
        {
            LOGINFO(TRUE,
                    "'%s' Exit Code: %d\n", cmd_name, exit_code);
            build_err = ERR_EXEC_PROCESS_NON_ZERO;
            return build_err;
        }
    }
    else if (WIFSIGNALED(status))
    {
        sig = WTERMSIG(status);
        LOGFATAL(ERR_EXEC_TERMINATE_BY_SIGNAL, TRUE,
                "'%s' Terminated by Signal: %d, Process Aborted\n", cmd_name, sig);
        return build_err;
    }
    else
    {
        LOGERROR(ERR_EXEC_ABNORMAL_EXIT, TRUE,
                "'%s' Exited Abnormally\n", cmd_name);
        return build_err;
    }

    build_err = ERR_SUCCESS;
    return build_err;
}

#endif /* BUILD_PLATFORM_LINUX_H */
