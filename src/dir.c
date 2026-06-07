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
 *  @file dir.c
 *
 *  @brief directory and file parsing, writing, copying and path resolution.
 */

#include "common/diagnostics.h"
#include "common/limits.h"
#include "logger/logger.h"
#include "logger/logger_messages_internal.h"
#include "memory/memory.h"

#include "h/dir.h"
#include "h/process.h"

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

u32 fsl_is_file(const fsl_fs_path *path)
{
    struct stat stats = {0};

    if (fsl_is_file_exists(path, FALSE) != FSL_ERR_SUCCESS)
        return fsl_err;

    if (fsl_stat(path, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
            fsl_err = FSL_ERR_SUCCESS;
        else
            fsl_err = FSL_ERR_IS_NOT_FILE;
    }
    else
        fsl_err = FSL_ERR_FILE_NOT_FOUND;

    return fsl_err;
}

u32 fsl_is_file_exists(const fsl_fs_path *path, b8 log)
{
    struct stat stats = {0};

    if (fsl_stat(path, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode) || S_ISLNK(stats.st_mode))
            fsl_err = FSL_ERR_SUCCESS;
        else
        {
            if (log)
                LOGERROR(FSL_ERR_IS_NOT_FILE, 0,
                        MSG_IS_NOT_FILE(path));
            else
                fsl_err = FSL_ERR_IS_NOT_FILE;
        }
    }
    else
    {
        if (log)
            LOGERROR(FSL_ERR_FILE_NOT_FOUND, 0,
                    MSG_FILE_NOT_FOUND(path));
        else
            fsl_err = FSL_ERR_FILE_NOT_FOUND;
    }

    return fsl_err;
}

u32 fsl_is_dir(const fsl_fs_path *path)
{
    struct stat stats = {0};

    if (fsl_is_dir_exists(path, FALSE) != FSL_ERR_SUCCESS)
        return fsl_err;
    if (fsl_stat(path, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
            fsl_err = FSL_ERR_SUCCESS;
        else fsl_err = FSL_ERR_IS_NOT_DIR;
    }
    else
        fsl_err = FSL_ERR_DIR_NOT_FOUND;

    return fsl_err;
}

u32 fsl_is_dir_exists(const fsl_fs_path *path, b8 log)
{
    struct stat stats = {0};

    if (fsl_stat(path, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
            fsl_err = FSL_ERR_SUCCESS;
        else
        {
            if (log)
                LOGERROR(FSL_ERR_IS_NOT_DIR, 0,
                        MSG_IS_NOT_DIR(path));
            else
                fsl_err = FSL_ERR_IS_NOT_DIR;
        }
    }
    else
    {
        if (log)
            LOGERROR(FSL_ERR_DIR_NOT_FOUND, 0,
                    MSG_DIR_NOT_FOUND(path));
        else
            fsl_err = FSL_ERR_DIR_NOT_FOUND;
    }

    return fsl_err;
}

u32 fsl_make_dir(const fsl_fs_path *path)
{
    if (fsl_mkdir(path) == 0)
    {
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_DIR_CREATE(path));

        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }

    switch (errno)
    {
        case EEXIST:
            fsl_err = FSL_ERR_DIR_EXISTS;
            break;

        default:
            LOGERROR(FSL_ERR_DIR_CREATE_FAIL, 0,
                    MSG_DIR_CREATE_FAIL(path));
    }

    return fsl_err;
}

int fsl_change_dir(const fsl_fs_path *path)
{
    int success = 0;
    success = fsl_chdir(path);
    LOGTRACE(0,
            MSG_DIR_CHANGE(path));
    return success;
}

u32 fsl_get_file_type(const fsl_fs_path *path, u32 *type)
{
    struct stat stats = {0};

    if (fsl_stat(path, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
            *type = FSL_FILE_TYPE_REG;
        else if (S_ISLNK(stats.st_mode))
            *type = FSL_FILE_TYPE_LNK;
        else if (S_ISDIR(stats.st_mode))
            *type = FSL_FILE_TYPE_DIR;
        else if (S_ISCHR(stats.st_mode))
            *type = FSL_FILE_TYPE_CHR;
        else if (S_ISBLK(stats.st_mode))
            *type = FSL_FILE_TYPE_BLK;
        else if (S_ISFIFO(stats.st_mode))
            *type = FSL_FILE_TYPE_FIFO;

        return FSL_ERR_SUCCESS;
    }

    LOGERROR(FSL_ERR_FILE_NOT_FOUND, 0,
            MSG_FILE_NOT_FOUND(path));
    return fsl_err;
}

u64 fsl_get_file_contents(const fsl_fs_path *path, void **dst, b8 terminate)
{
    FILE *file = NULL;
    u64 cursor = 0;
    u64 len = 0;

    if (fsl_is_file_exists(path, TRUE) != FSL_ERR_SUCCESS)
            return 0;

    if ((file = fopen(path, "rb")) == NULL)
    {
        LOGERROR(FSL_ERR_FILE_OPEN_FAIL, 0,
                MSG_FILE_OPEN_FAIL(path));
        return 0;
    }

    fseek(file, 0, SEEK_END);
    len = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (fsl_mem_alloc(dst, len + (terminate ? 1 : 0),
                "fsl_get_file_contents().dst") != FSL_ERR_SUCCESS)
        goto cleanup;

    cursor = fread(*dst, 1, len, file);

    fclose(file);

    if (terminate)
        ((u8*)(*dst))[len] = 0;

    fsl_err = FSL_ERR_SUCCESS;
    return cursor;

cleanup:

    if (file)
        fclose(file);

    return 0;
}

fsl_buf fsl_get_dir_contents(const fsl_fs_path *path)
{
    fsl_buf nobuf = {0};

    str *dir_name_absolute = NULL;
    str dir_name_absolute_usable[FSL_PATH_CAP] = {0};
    str entry_name_full[FSL_PATH_CAP] = {0};
    DIR *dir = NULL;
    struct dirent *entry = {0};
    fsl_buf contents = {0};
    u64 i = 0;

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return nobuf;
    }

    if (fsl_is_dir_exists(path, TRUE) != FSL_ERR_SUCCESS)
        return nobuf;

    if (fsl_get_path_absolute(path, &dir_name_absolute) != FSL_ERR_SUCCESS)
        goto cleanup;

    snprintf(dir_name_absolute_usable, FSL_PATH_CAP, "%s", dir_name_absolute);

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

    if (!contents.memb)
    {
        closedir(dir);
        if (dir_name_absolute)
            fsl_mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute),
                    "fsl_get_dir_contents().dir_name_absolute");
        fsl_mem_free_buf(&contents, "fsl_get_dir_contents().contents");

        fsl_err = FSL_ERR_DIR_EMPTY;
        return nobuf;
    }

    if (fsl_mem_alloc_buf(&contents, contents.memb,
                FSL_ID_CAP, "fsl_get_dir_contents().contents") != FSL_ERR_SUCCESS)
        goto cleanup;

    rewinddir(dir);
    i = 0;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!strncmp(entry->d_name, ".\0", 2) ||
                !strncmp(entry->d_name, "..\0", 3))
            continue;

        contents.i[i] = (u8*)contents.buf + i * FSL_ID_CAP;
        memcpy(contents.i[i], entry->d_name, FSL_ID_CAP - 1);
        snprintf(entry_name_full, FSL_PATH_CAP, "%s%s", dir_name_absolute_usable, entry->d_name);

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

    if (dir)
        closedir(dir);

    if (dir_name_absolute)
        fsl_mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute),
                "fsl_get_dir_contents().dir_name_absolute");
    fsl_mem_free_buf(&contents, "fsl_get_dir_contents().contents");
    return nobuf;
}

u64 fsl_get_dir_entry_count(const fsl_fs_path *path)
{
    DIR *dir = NULL;
    u64 count = 0;
    struct dirent *entry = {0};

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return 0;
    }

    if (fsl_is_dir_exists(path, TRUE) != FSL_ERR_SUCCESS)
        return 0;

    dir = opendir(path);
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

u32 fsl_copy_file(const fsl_fs_path *src, const fsl_fs_path *dst)
{
    str str_dst[FSL_PATH_CAP] = {0};
    str str_lnk[FSL_PATH_CAP] = {0}; /* if is symlink, store symlink's link in this buffer */
    str *in_file = NULL;
    FILE *out_file = NULL;
    u64 len = 0;
    u32 file_type = 0;
    struct stat stats = {0};
    struct timespec ts[2] = {0};

    if (fsl_is_file_exists(src, TRUE) != FSL_ERR_SUCCESS)
            return fsl_err;

    snprintf(str_dst, FSL_PATH_CAP, "%s", dst);

    if (fsl_is_dir(dst) == FSL_ERR_SUCCESS)
    {
        fsl_check_slash(str_dst);
        fsl_get_base_name(src, str_dst + strlen(str_dst), FSL_PATH_CAP);
        fsl_posix_slash(str_dst);
    }

    if (fsl_get_file_type(src, &file_type) != FSL_ERR_SUCCESS)
        return fsl_err;

    switch (file_type)
    {
        case FSL_FILE_TYPE_REG:
            if ((out_file = fopen(str_dst, "wb")) == NULL)
            {
                LOGERROR(FSL_ERR_FILE_OPEN_FAIL, 0,
                        MSG_FILE_COPY_FAIL(src, str_dst));
                return fsl_err;
            }

            len = fsl_get_file_contents(src, (void*)&in_file, FALSE);
            if (fsl_err != FSL_ERR_SUCCESS || !in_file)
            {
                fclose(out_file);
                return fsl_err;
            }

            fwrite(in_file, 1, len, out_file);

            LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                    MSG_FILE_COPY(src, str_dst));

            fclose(out_file);
            fsl_mem_free((void*)&in_file, strlen(in_file), "fsl_copy_file().in_file");

            if (fsl_stat(src, &stats) == 0)
                fsl_chmod(str_dst, stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
            else
            {
                LOGWARNING(FSL_ERR_FILE_STAT_FAIL,
                        FSL_FLAG_LOG_NO_VERBOSE,
                        MSG_FILE_PERMISSION_COPY_FAIL(src, str_dst));
                return fsl_err;
            }
            break;

        case FSL_FILE_TYPE_LNK:
            if (readlink(src, str_lnk, FSL_PATH_CAP - 1) < 1)
            {
                LOGERROR(FSL_ERR_FILE_OPEN_FAIL, 0,
                        MSG_FILE_SYMLINK_COPY_FAIL(src, str_dst));
                return fsl_err;
            }

            if (fsl_is_file_exists(str_dst, FALSE) == FSL_ERR_SUCCESS)
                remove(str_dst);

            str_lnk[strnlen(str_lnk, FSL_PATH_CAP - 1)] = '\0';
            symlink(str_lnk, str_dst);

            LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                    MSG_FILE_SYMLINK_COPY(src, str_dst));

            if (fsl_stat(src, &stats) == 0)
                fsl_chmod(str_dst, stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
            else
            {
                LOGWARNING(FSL_ERR_FILE_STAT_FAIL,
                        FSL_FLAG_LOG_NO_VERBOSE,
                        MSG_FILE_PERMISSION_COPY_FAIL(src, str_dst));
                return fsl_err;
            }
            break;
    }

    if (stats.st_atim.tv_nsec == 0)
        stats.st_atim.tv_nsec = 1;
    else if (stats.st_atim.tv_nsec >= 1000000000L)
        stats.st_atim.tv_nsec = 1000000000L - 1;

    if (stats.st_mtim.tv_nsec == 0)
        stats.st_mtim.tv_nsec = 1;
    else if (stats.st_mtim.tv_nsec >= 1000000000L)
        stats.st_mtim.tv_nsec = 1000000000L - 1;

    ts[0] = stats.st_atim;
    ts[1] = stats.st_mtim;
    utimensat(AT_FDCWD, str_dst, ts, AT_SYMLINK_NOFOLLOW);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_copy_dir(const fsl_fs_path *src, const fsl_fs_path *dst, b8 contents_only)
{
    fsl_buf dir_contents = {0};
    str str_src[FSL_PATH_CAP] = {0};
    str str_dst[FSL_PATH_CAP] = {0};
    str in_dir[FSL_PATH_CAP] = {0};
    str out_dir[FSL_PATH_CAP] = {0};
    u64 i = 0;
    b8 is_empty = FALSE;
    struct stat stats = {0};
    struct timespec ts[2] = {0};

    if (fsl_is_dir_exists(src, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    dir_contents = fsl_get_dir_contents(src);
    if (contents_only && (fsl_err == FSL_ERR_DIR_EMPTY || !dir_contents.loaded))
        return fsl_err;
    else if (fsl_err != FSL_ERR_SUCCESS && fsl_err != FSL_ERR_DIR_EMPTY)
        return fsl_err;

    snprintf(str_src, FSL_PATH_CAP, "%s", src);
    fsl_check_slash(str_src);
    fsl_normalize_slash(str_src);

    snprintf(str_dst, FSL_PATH_CAP, "%s", dst);
    fsl_check_slash(str_dst);
    fsl_normalize_slash(str_dst);

    if (fsl_is_dir_exists(str_dst, FALSE) == FSL_ERR_SUCCESS && !contents_only)
    {
        fsl_get_base_name(str_src, str_dst + strlen(str_dst), FSL_PATH_CAP - strlen(str_dst));
        fsl_check_slash(str_dst);
        fsl_posix_slash(str_dst);
        fsl_make_dir(str_dst);
    }
    else
        fsl_make_dir(str_dst);

    if (fsl_stat(str_src, &stats) == 0)
        fsl_chmod(str_dst, stats.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
    else
        LOGWARNING(FSL_ERR_FILE_STAT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_DIR_PERMISSION_COPY_FAIL(str_src, str_dst));

    if (stats.st_atim.tv_nsec == 0)
        stats.st_atim.tv_nsec = 1;
    else if (stats.st_atim.tv_nsec >= 1000000000L)
        stats.st_atim.tv_nsec = 1000000000L - 1;

    if (stats.st_mtim.tv_nsec == 0)
        stats.st_mtim.tv_nsec = 1;
    else if (stats.st_mtim.tv_nsec >= 1000000000L)
        stats.st_mtim.tv_nsec = 1000000000L - 1;

    ts[0] = stats.st_atim;
    ts[1] = stats.st_mtim;
    utimensat(AT_FDCWD, str_dst, ts, 0);

    for (i = 0; i < dir_contents.memb; ++i)
    {
        snprintf(in_dir, FSL_PATH_CAP, "%s%s", str_src, (str*)dir_contents.i[i]);
        snprintf(out_dir, FSL_PATH_CAP, "%s%s", str_dst, (str*)dir_contents.i[i]);

        if (fsl_is_dir(in_dir) == FSL_ERR_SUCCESS)
        {
            fsl_copy_dir(in_dir, out_dir, FALSE);
            continue;
        }
        fsl_copy_file(in_dir, out_dir);
    }

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_DIR_COPY(src, str_dst));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_write_file(const fsl_fs_path *path, u64 size, void *buf, b8 log, b8 text)
{
    FILE *file = NULL;
    if ((file = fopen(path, "wb")) == NULL)
    {
        if (log)
            LOGERROR(FSL_ERR_FILE_OPEN_FAIL, 0,
                    MSG_FILE_WRITE_FAIL(path));

        fsl_err = FSL_ERR_FILE_OPEN_FAIL;
        return fsl_err;
    }

    fwrite(buf, 1, size, file);
    if (text) fprintf(file, "%c", '\n');
    fclose(file);

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FILE_WRITE(path));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_append_file(const fsl_fs_path *path, u64 size, void *buf, b8 log, b8 text)
{
    FILE *file = NULL;
    if ((file = fopen(path, "ab")) == NULL)
    {
        if (log)
            LOGERROR(FSL_ERR_FILE_OPEN_FAIL, 0,
                    MSG_FILE_APPEND_FAIL(path));
        else
        {
            fsl_err = FSL_ERR_FILE_OPEN_FAIL;
            return fsl_err;
        }
    }

    fwrite(buf, 1, size, file);
    if (text) fprintf(file, "%c", '\n');
    fclose(file);

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FILE_APPEND(path));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_get_path_absolute(const fsl_fs_path *path, str **dst)
{
    str path_absolute[FSL_PATH_CAP] = {0};
    u64 len = 0;

    if (strlen(path) > FSL_PATH_CAP - 2)
    {
        LOGERROR(FSL_ERR_GET_PATH_ABSOLUTE_FAIL, 0,
                MSG_ACTION_REASON_ERROR("Get Absolute Path", "Path Too Long"));
        return fsl_err;
    }

    if (fsl_is_dir_exists(path, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    if (fsl_get_path_absolute_internal(path, path_absolute) != FSL_ERR_SUCCESS)
        return fsl_err;

    len = strlen(path_absolute);

    if (!*dst && fsl_mem_alloc((void*)dst, len + 1,
                "fsl_get_path_absolute().dst") != FSL_ERR_SUCCESS)
        return fsl_err;

    memcpy(*dst, path_absolute, len);
    fsl_check_slash(*dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_get_path_bin_root(str **dst)
{
    str path_bin_root[FSL_PATH_CAP] = {0};
    u64 len = 0;
    char *last_slash = NULL;

    if (fsl_get_path_bin_root_internal(path_bin_root) != FSL_ERR_SUCCESS)
        return fsl_err;

    len = strlen(path_bin_root);
    if (len > FSL_PATH_CAP - 1)
    {
        LOGFATAL(FSL_ERR_PATH_TOO_LONG, 0,
                MSG_PATH_TOO_LONG_FATAL("Get Binary Root", path_bin_root));
        return fsl_err;
    }

    path_bin_root[len] = 0;
    if (!*dst && fsl_mem_alloc((void*)dst, FSL_PATH_CAP,
                "fsl_get_path_bin_root().dst") != FSL_ERR_SUCCESS)
        return fsl_err;

    memcpy(*dst, path_bin_root, FSL_PATH_CAP - 1);

    fsl_posix_slash(*dst);
    last_slash = strrchr(*dst, '/');
    if (last_slash)
        *last_slash = 0;
    fsl_check_slash(*dst);
    fsl_posix_slash(*dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_check_slash(fsl_fs_path *path)
{
    u64 len = 0;

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);
    if (len > FSL_PATH_CAP - 2)
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

void fsl_normalize_slash(fsl_fs_path *path)
{
    u64 len = 0, i = 0;

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

void fsl_posix_slash(fsl_fs_path *path)
{
    u64 len = 0, i = 0;

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

u32 fsl_retract_path(fsl_fs_path *path)
{
    u64 len = 0, i = 0, stage = 0;

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return fsl_err;
    }

    len = strlen(path);
    if (len <= 1)
    {
        LOGERROR(FSL_ERR_SIZE_TOO_SMALL, 0,
                MSG_ACTION_REASON_ERROR("Retract Path", "Size Too Small"));
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

u32 fsl_get_base_name(const fsl_fs_path *path, str *dst, u64 size)
{
    i64 i = 0;
    u64 len = 0;
    str path_resolved[FSL_PATH_CAP] = {0};

    if (size == 0)
    {
        LOGERROR(FSL_ERR_SIZE_TOO_SMALL, 0,
                MSG_GET_BASE_NAME_FAIL(path, "Size Too Small"));
        return fsl_err;
    }

    if (!path || !path[0] || !dst)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, 0,
                MSG_GET_BASE_NAME_FAIL(path, "Pointer `NULL`"));
        return fsl_err;
    }

    len = strlen(path);
    if (len > FSL_PATH_CAP - 2)
    {
        LOGERROR(FSL_ERR_PATH_TOO_LONG, 0,
                MSG_GET_BASE_NAME_FAIL(path, "Path Too Long"));
        return fsl_err;
    }

    snprintf(path_resolved, FSL_PATH_CAP, "%s", path);
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
