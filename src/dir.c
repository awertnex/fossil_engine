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
 *	dir.c - directory and file parsing, writing, copying and path resolution
 */

#include "h/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/process.h"

/* TODO: fix 'fsl_get_file_type()' */
u64 fsl_get_file_type(const str *name)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
        return S_ISREG(stats.st_mode) | (S_ISDIR(stats.st_mode) * 2);

    _LOGERROR(FSL_ERR_FILE_NOT_FOUND, 0,
            "File '%s' Not Found\n", name);
    return 0;
}

u32 fsl_is_file(const str *name)
{
    if (fsl_is_file_exists(name, FALSE) != FSL_ERR_SUCCESS)
        return fsl_err;

    struct stat stats;
    if (stat(name, &stats) == 0 && S_ISREG(stats.st_mode))
    {
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }

    fsl_err = FSL_ERR_IS_NOT_FILE;
    return fsl_err;
}

u32 fsl_is_file_exists(const str *name, b8 log)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
        {
            fsl_err = FSL_ERR_SUCCESS;
            return fsl_err;
        }
        else
        {
            if (log)
                _LOGERROR(FSL_ERR_IS_NOT_FILE, 0,
                        "'%s' is Not a Regular File\n", name);
            else fsl_err = FSL_ERR_IS_NOT_FILE;
            return fsl_err;
        }
    }
    if (log)
        _LOGERROR(FSL_ERR_FILE_NOT_FOUND, 0,
                "File '%s' Not Found\n", name);

    fsl_err = FSL_ERR_FILE_NOT_FOUND;
    return fsl_err;
}

u32 fsl_is_dir(const str *name)
{
    if (fsl_is_dir_exists(name, FALSE) != FSL_ERR_SUCCESS)
        return fsl_err;
    struct stat stats;
    if (stat(name, &stats) == 0 && S_ISDIR(stats.st_mode))
    {
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }

    fsl_err = FSL_ERR_IS_NOT_DIR;
    return fsl_err;
}

u32 fsl_is_dir_exists(const str *name, b8 log)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
        {
            fsl_err = FSL_ERR_SUCCESS;
            return fsl_err;
        }
        else
        {
            if (log)
                _LOGERROR(FSL_ERR_IS_NOT_DIR, 0,
                        "'%s' is Not a Directory\n", name);
            else fsl_err = FSL_ERR_IS_NOT_DIR;
            return fsl_err;
        }
    }
    if (log)
        _LOGERROR(FSL_ERR_DIR_NOT_FOUND, 0,
                "Directory '%s' Not Found\n", name);

    fsl_err = FSL_ERR_DIR_NOT_FOUND;
    return fsl_err;
}

u64 fsl_get_file_contents(const str *name, void **dst, u64 size, b8 terminate)
{
    FILE *file = NULL;
    u64 cursor;

    if (fsl_is_file_exists(name, TRUE) != FSL_ERR_SUCCESS)
            return 0;

    if ((file = fopen(name, "rb")) == NULL)
    {
        _LOGERROR(FSL_ERR_FILE_OPEN_FAIL, 0,
                "Failed to Open File '%s'\n", name);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fsl_mem_alloc(dst, len + (terminate ? 1 : 0),
                "fsl_get_file_contents().dst") != FSL_ERR_SUCCESS)
        goto cleanup;

    cursor = fread(*dst, size, len, file);

    fclose(file);
    if (terminate) ((u8*)(*dst))[len] = 0;
    fsl_err = FSL_ERR_SUCCESS;
    return cursor;

cleanup:

    if (file) fclose(file);
    return 0;
}

fsl_buf fsl_get_dir_contents(const str *name)
{
    str *dir_name_absolute = NULL;
    str dir_name_absolute_usable[PATH_MAX] = {0};
    str entry_name_full[PATH_MAX] = {0};
    DIR *dir = NULL;
    struct dirent *entry;
    fsl_buf contents = {0};
    u64 i;

    if (!name)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return (fsl_buf){0};
    }

    if (fsl_is_dir_exists(name, TRUE) != FSL_ERR_SUCCESS)
        return (fsl_buf){0};

    if (fsl_get_path_absolute(name, &dir_name_absolute) != FSL_ERR_SUCCESS)
        goto cleanup;

    snprintf(dir_name_absolute_usable, PATH_MAX, "%s", dir_name_absolute);

    dir = opendir(dir_name_absolute);
    if (!dir)
    {
        fsl_err = FSL_ERR_DIR_OPEN_FAIL;
        goto cleanup;
    }

    while ((entry = readdir(dir)) != NULL)
        ++contents.memb;

    /* subtract for '.' and '..' */
    contents.memb -= 2;

    if (!contents.memb || fsl_mem_alloc_buf(&contents, contents.memb,
                NAME_MAX, "fsl_get_dir_contents().contents") != FSL_ERR_SUCCESS)
        goto cleanup;

    rewinddir(dir);
    i = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!strncmp(entry->d_name, ".\0", 2) ||
                !strncmp(entry->d_name, "..\0", 3))
            continue;

        contents.i[i] = contents.buf + (i * NAME_MAX);
        memcpy(contents.i[i], entry->d_name, NAME_MAX - 1);
        snprintf(entry_name_full, PATH_MAX, "%s%s", dir_name_absolute_usable, entry->d_name);

        if (fsl_is_dir(entry_name_full) == FSL_ERR_SUCCESS)
            fsl_check_slash(contents.i[i]);
        ++i;
    }

    closedir(dir);
    fsl_mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute),
            "fsl_get_dir_contents().dir_name_absolute");

    fsl_err = FSL_ERR_SUCCESS;
    return contents;

cleanup:

    if (dir) closedir(dir);
    fsl_mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute),
            "fsl_get_dir_contents().dir_name_absolute");
    fsl_mem_free_buf((void*)&contents, "fsl_get_dir_contents().contents");
    return (fsl_buf){0};
}

u64 fsl_get_dir_entry_count(const str *name)
{
    DIR *dir = NULL;
    u64 count;
    struct dirent *entry;

    if (!name)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return 0;
    }

    if (fsl_is_dir_exists(name, TRUE) != FSL_ERR_SUCCESS)
        return 0;

    dir = opendir(name);
    if (!dir)
        return 0;

    count = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!strncmp(entry->d_name, ".\0", 2) ||
                !strncmp(entry->d_name, "..\0", 3))
            continue;
        ++count;
    }

    closedir(dir);

    fsl_err = FSL_ERR_SUCCESS;
    return count;
}

u32 fsl_copy_file(const str *src, const str *dst)
{
    str str_dst[PATH_MAX] = {0};
    str *in_file = NULL;
    FILE *out_file = NULL;
    u64 len = 0;
    struct stat stats;
    struct timespec ts[2] = {0};

    if (fsl_is_file_exists(src, TRUE) != FSL_ERR_SUCCESS)
            return fsl_err;

    snprintf(str_dst, PATH_MAX, "%s", dst);

    if (fsl_is_dir(dst) == FSL_ERR_SUCCESS)
    {
        fsl_check_slash(str_dst);
        fsl_get_base_name(src, str_dst + strlen(str_dst), PATH_MAX);
        fsl_posix_slash(str_dst);
    }

    if ((out_file = fopen(str_dst, "wb")) == NULL)
    {
        _LOGERROR(FSL_ERR_FILE_OPEN_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Copy File '%s' -> '%s'\n", src, str_dst);
        return fsl_err;
    }

    len = fsl_get_file_contents(src, (void*)&in_file, 1, FALSE);
    if (fsl_err != FSL_ERR_SUCCESS || !in_file)
    {
        fclose(out_file);
        return fsl_err;
    }

    fwrite(in_file, 1, len, out_file);
    fclose(out_file);

    _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            "File Copied '%s' -> '%s'\n", src, str_dst);

    if (stat(src, &stats) == 0)
        chmod(str_dst, stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
    else
        _LOGWARNING(FSL_ERR_FILE_STAT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Copy File Permissions '%s' -> '%s', 'stat()' Failed\n",
                src, str_dst);

    ts[0].tv_sec = stats.st_atim.tv_sec;
    ts[0].tv_nsec = stats.st_atim.tv_nsec;
    ts[1].tv_sec = stats.st_mtim.tv_sec;
    ts[1].tv_nsec = stats.st_mtim.tv_nsec;
    utimensat(AT_FDCWD, str_dst, ts, 0);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_copy_dir(const str *src, const str *dst, b8 contents_only)
{
    fsl_buf dir_contents = {0};
    str str_src[PATH_MAX] = {0};
    str str_dst[PATH_MAX] = {0};
    str in_dir[PATH_MAX] = {0};
    str out_dir[PATH_MAX] = {0};
    u64 i;
    struct stat stats;
    struct timespec ts[2] = {0};

    if (fsl_is_dir_exists(src, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    dir_contents = fsl_get_dir_contents(src);
    if (!dir_contents.loaded)
        return fsl_err;

    snprintf(str_src, PATH_MAX, "%s", src);
    fsl_check_slash(str_src);
    fsl_normalize_slash(str_src);

    snprintf(str_dst, PATH_MAX, "%s", dst);
    fsl_check_slash(str_dst);
    fsl_normalize_slash(str_dst);

    if (fsl_is_dir_exists(str_dst, FALSE) == FSL_ERR_SUCCESS && !contents_only)
    {
        get_base_name(str_src, str_dst + strlen(str_dst), PATH_MAX - strlen(str_dst));
        check_slash(str_dst);
        posix_slash(str_dst);
        make_dir(str_dst);
    }
    else fsl_make_dir(str_dst);

    for (i = 0; i < dir_contents.memb; ++i)
    {
        snprintf(in_dir, PATH_MAX - 1, "%s%s", str_src, (str*)dir_contents.i[i]);
        snprintf(out_dir, PATH_MAX - 1, "%s%s", str_dst, (str*)dir_contents.i[i]);

        if (fsl_is_dir(in_dir) == FSL_ERR_SUCCESS)
        {
            fsl_copy_dir(in_dir, out_dir, TRUE);
            continue;
        }
        fsl_copy_file(in_dir, out_dir);
    }

    _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            "Directory Copied '%s' -> '%s'\n", src, str_dst);

    if (stat(str_src, &stats) == 0)
        chmod(str_dst, stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
    else
        _LOGWARNING(FSL_ERR_FILE_STAT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Copy Directory Permissions '%s' -> '%s', 'stat()' Failed\n",
                str_src, str_dst);

    ts[0].tv_sec = stats.st_atim.tv_sec;
    ts[0].tv_nsec = stats.st_atim.tv_nsec;
    ts[1].tv_sec = stats.st_mtim.tv_sec;
    ts[1].tv_nsec = stats.st_mtim.tv_nsec;
    utimensat(AT_FDCWD, str_dst, ts, 0);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_write_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    FILE *file = NULL;
    if ((file = fopen(name, "wb")) == NULL)
    {
        if (log)
            _LOGERROR(FSL_ERR_FILE_OPEN_FAIL, 0,
                    "Failed to Write File '%s'\n", name);

        fsl_err = FSL_ERR_FILE_OPEN_FAIL;
        return fsl_err;
    }

    fwrite(buf, size, length, file);
    if (text) fprintf(file, "%c", '\n');
    fclose(file);

    _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            "File Written '%s'\n", name);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    FILE *file = NULL;
    if ((file = fopen(name, "ab")) == NULL)
    {
        if (log)
            _LOGERROR(FSL_ERR_FILE_OPEN_FAIL, 0,
                    "Failed to Append File '%s'\n", name);
        else
        {
            fsl_err = FSL_ERR_FILE_OPEN_FAIL;
            return fsl_err;
        }
    }

    fwrite(buf, size, length, file);
    if (text) fprintf(file, "%c", '\n');
    fclose(file);

    _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            "File Appended '%s'\n", name);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_get_path_absolute(const str *name, str **dst)
{
    str path_absolute[PATH_MAX] = {0};
    u64 len = 0;

    if (strlen(name) >= PATH_MAX - 1)
    {
        _LOGERROR(FSL_ERR_GET_PATH_ABSOLUTE_FAIL, 0,
                "%s\n", "Failed to Get Absolute Path, Path Too Long");
        return fsl_err;
    }

    if (fsl_is_dir_exists(name, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    if (_fsl_get_path_absolute(name, path_absolute) != FSL_ERR_SUCCESS)
        return fsl_err;

    len = strlen(path_absolute) + 1;

    if (!*dst && fsl_mem_alloc((void*)dst, sizeof(str*) * (len + 1),
                "fsl_get_path_absolute().dst") != FSL_ERR_SUCCESS)
        return fsl_err;

    strncpy(*dst, path_absolute, len);
    fsl_check_slash(*dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_get_path_bin_root(str **dst)
{
    str path_bin_root[PATH_MAX] = {0};
    u64 len = 0;
    char *last_slash = NULL;

    if (_fsl_get_path_bin_root(path_bin_root) != FSL_ERR_SUCCESS)
        return fsl_err;

    len = strlen(path_bin_root) + 1;
    if (len >= PATH_MAX - 1)
    {
        _LOGFATAL(FSL_ERR_PATH_TOO_LONG, 0,
                "Path Too Long '%s', Process Aborted\n", path_bin_root);
        return fsl_err;
    }

    path_bin_root[len] = 0;
    if (!*dst && fsl_mem_alloc((void*)dst, PATH_MAX,
                "fsl_get_path_bin_root().dst") != FSL_ERR_SUCCESS)
        return fsl_err;

    strncpy(*dst, path_bin_root, len);

    last_slash = strrchr(*dst, '/');
    if (last_slash)
        *last_slash = 0;
    fsl_check_slash(*dst);
    fsl_normalize_slash(*dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_check_slash(str *path)
{
    u64 len = 0;

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);
    if (len >= PATH_MAX - 1)
    {
        fsl_err = FSL_ERR_PATH_TOO_LONG;
        return;
    }

    if (path[len - 1] == FSL_SLASH_NATIVE || path[len - 1] == FSL_SLASH_NON_NATIVE)
    {
        fsl_err = FSL_ERR_SUCCESS;
        return;
    }

    path[len] = FSL_SLASH_NATIVE;
    path[len + 1] = 0;
    fsl_err = FSL_ERR_SUCCESS;
}

void fsl_normalize_slash(str *path)
{
    u64 len, i;

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);

    for (i = 0; i < len; ++i)
    {
        if (path[i] == FSL_SLASH_NON_NATIVE)
            path[i] = FSL_SLASH_NATIVE;
    }

    fsl_err = FSL_ERR_SUCCESS;
}

void fsl_posix_slash(str *path)
{
    u64 len, i;

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);

    for (i = 0; i < len; ++i)
    {
        if (path[i] == '\\')
            path[i] = '/';
    }

    fsl_err = FSL_ERR_SUCCESS;
}

u32 fsl_retract_path(str *path)
{
    u64 len, i, stage = 0;

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return fsl_err;
    }

    len = strlen(path);
    if (len <= 1)
    {
        _LOGERROR(FSL_ERR_SIZE_TOO_SMALL, 0,
                "%s\n", "Failed to Retract Path, Size Too Small");
        return fsl_err;
    }

    for (i = 0; i < len; ++i)
    {
        if (stage == 1 &&
                (path[len - i - 1] == FSL_SLASH_NATIVE ||
                 path[len - i - 1] == FSL_SLASH_NON_NATIVE))
            break;
        if (path[len - i - 1])
        {
            path[len - i - 1] = 0;
            stage = 1;
        }
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_get_base_name(const str *path, str *dst, u64 size)
{
    i64 i = 0;
    u64 len = 0;
    str path_resolved[PATH_MAX] = {0};

    if (size == 0)
    {
        _LOGERROR(FSL_ERR_SIZE_TOO_SMALL, 0,
                "Failed to Get Base Name of '%s', 'size' Too Small\n", path);
        return fsl_err;
    }

    if (!path || !path[0] || !dst)
    {
        _LOGERROR(FSL_ERR_POINTER_NULL, 0,
                "%s\n", "Failed to Get Base Name, Pointer NULL");
        return fsl_err;
    }

    len = strlen(path);
    if (len >= PATH_MAX - 1)
    {
        _LOGERROR(FSL_ERR_PATH_TOO_LONG, 0,
                "Failed to Get Base Name of '%s', Path Too Long\n", path);
        return fsl_err;
    }

    snprintf(path_resolved, PATH_MAX, "%s", path);
    fsl_posix_slash(path_resolved);

    i = (i64)len - 1;
    if (path_resolved[i] == '/')
    {
        if (i == 0)
        {
            dst[0] = '/';
            fsl_err = FSL_ERR_SUCCESS;
            return fsl_err;
        }
        else --i;
    }

    while (i > 0 && path_resolved[i - 1] != '/')
        --i;

    snprintf(dst, size, "%s", path_resolved + i);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}
