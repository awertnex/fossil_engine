#ifndef BUILD_PLATFORM_H
#define BUILD_PLATFORM_H

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
#include "diagnostics.h"
#include "logger.h"
#include "memory.h"

#if PLATFORM_LINUX
#   include "platform_linux.h"
#else
#   include "platform_windows.h"
#endif /* PLATFORM */

/* ---- section: signatures ------------------------------------------------- */

/*! @param log enable/disable logging.
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 is_file_exists(const str *name, b8 log);

/*! @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 is_dir(const str *name);

/*! @param log enable/disable logging.
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 is_dir_exists(const str *name, b8 log);

/*! -- IMPLEMENTATION: platform_<PLATFORM>.h --;
 *
 *  @brief make directory `path` if it doesn't exist.
 *
 *  @remark failure includes "directory already exists".
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 make_dir(const str *path);

/*! -- IMPLEMENTATION: platform_<PLATFORM>.h --;
 *
 *  @brief change current working directory.
 */
extern int change_dir(const str *path);

/*! @param dst pointer to `NULL` buffer to store file contents.
 *  @remark `dst` is allocated file size, + 1 if `terminate` is `TRUE`.
 *
 *  @param format read file `name` using `format` (@ref fopen() parameter).
 *  @param terminate enable/disable null (`\0`) termination.
 *
 *  @return file size in bytes.
 *  @return 0 on failure and @ref build_err is set accordingly.
 */
extern u64 get_file_contents(const str *name, void **dst, u64 size, b8 terminate);

/*! @brief get directory entries at `name`.
 *
 *  @return `(_buf){0}` on failure and @ref build_err is set accordingly.
 */
extern _buf get_dir_contents(const str *name);

/*! @brief copy `src` into `dst`.
 *
 *  @remark can overwrite files.
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 copy_file(const str *src, const str *dst);

/*! @brief copy `src` into `dst`.
 *
 *  @param contents_only
 *      TRUE: copy directory contents of `src` and place inside `dst`.
 *      FALSE: copy directory `src` and place inside `dst`.
 *
 *  @remark can overwrite directories and files.
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 copy_dir(const str *src, const str *dst, b8 contents_only);

/*! @brief get calloc'd string of resolved `name`.
 *
 *  @return `NULL` on failure and @ref build_err is set accordingly.
 */
extern str *get_path_absolute(const str *name);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.h --;
 *
 *  @brief get real path.
 * 
 *  @param path relative path.
 *  @param path_real result/canonical `path`, ending with slash (`/`).
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 _get_path_absolute(const str *path, str *path_real);

/*! @brief get calloc'd string of executable's path, slash (`/`) and null (`\0`) terminated.
 *
 *  @return `NULL` on failure and @ref build_err is set accordingly.
 */
extern str *get_path_bin_root(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  -- IMPLEMENTATION: platform_<PLATFORM>.h --;
 *
 *  @brief get current path of binary/executable, assign allocated path string to `path`.
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 _get_path_bin_root(str *path);

/*! @brief execute command in a separate child process (based on @ref execvp()).
 * 
 *  -- IMPLEMENTATION: platform_<PLATFORM>.h --;
 *
 *  @param cmd command and args to execute.
 *  @param cmd_name command name (for logging).
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 exec(_buf *cmd, str *cmd_name);

/*! @brief append @ref SLASH_NATIVE onto `path` if `path` not ending in @ref SLASH_NATIVE, null (`\n`) terminated.
 *
 *  @remark @ref build_err is set accordingly on failure.
 */
extern void check_slash(str *path);

/*! @brief change all backslashes (`\`) to slashes ('\').
 *
 *  @remark @ref build_err is set accordingly on failure.
 */
extern void posix_slash(str *path);

/*! @brief get base name of `path`.
 *
 *  @param size max size of `dst` to use.
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 get_base_name(const str *path, str *dst, u64 size);

/* ---- section: implementation --------------------------------------------- */

u32 is_file_exists(const str *name, b8 log)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
        {
            build_err = ERR_SUCCESS;
            return build_err;
        }
        else
        {
            if (log)
                LOGERROR(ERR_IS_NOT_FILE, TRUE,
                        "'%s' is Not a Regular File\n", name);
            else build_err = ERR_IS_NOT_FILE;
            return build_err;
        }
    }

    if (log)
        LOGERROR(ERR_FILE_NOT_FOUND, TRUE,
                "File '%s' Not Found\n", name);
    else build_err = ERR_FILE_NOT_FOUND;
    return build_err;
}

u32 is_dir(const str *name)
{
    if (is_dir_exists(name, FALSE) != ERR_SUCCESS)
        return build_err;

    struct stat stats;
    if (stat(name, &stats) == 0 && S_ISDIR(stats.st_mode))
    {
        build_err = ERR_SUCCESS;
        return build_err;
    }

    build_err = ERR_IS_NOT_DIR;
    return build_err;
}

u32 is_dir_exists(const str *name, b8 log)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
        {
            build_err = ERR_SUCCESS;
            return build_err;
        }
        else
        {
            if (log)
                LOGERROR(ERR_IS_NOT_DIR, TRUE,
                        "'%s' is Not a Directory\n", name);
            else build_err = ERR_IS_NOT_DIR;
            return build_err;
        }
    }

    if (log)
        LOGERROR(ERR_DIR_NOT_FOUND, TRUE,
                "Directory '%s' Not Found\n", name);
    else build_err = ERR_DIR_NOT_FOUND;
    return build_err;
}

u64 get_file_contents(const str *name, void **dst, u64 size, b8 terminate)
{
    FILE *file = NULL;
    u64 cursor;

    if (is_file_exists(name, TRUE) != ERR_SUCCESS)
            return 0;

    if ((file = fopen(name, "rb")) == NULL)
    {
        LOGERROR(ERR_FILE_OPEN_FAIL, TRUE,
                "Failed to Open File '%s'\n", name);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (mem_alloc(dst, len + (terminate ? 1 : 0),
                "get_file_contents().dst") != ERR_SUCCESS)
        goto cleanup;

    cursor = fread(*dst, size, len, file);

    fclose(file);
    if (terminate) ((u8*)(*dst))[len] = 0;
    build_err = ERR_SUCCESS;
    return cursor;

cleanup:

    if (file) fclose(file);
    return 0;
}

_buf get_dir_contents(const str *name)
{
    str *dir_name_absolute = NULL;
    str dir_name_absolute_usable[PATH_MAX] = {0};
    str entry_name_full[PATH_MAX] = {0};
    DIR *dir = NULL;
    struct dirent *entry;
    _buf contents = {0};
    u64 i;

    if (!name)
    {
        build_err = ERR_POINTER_NULL;
        return (_buf){0};
    }

    if (is_dir_exists(name, TRUE) != ERR_SUCCESS)
        return (_buf){0};

    dir_name_absolute = get_path_absolute(name);
    if (!dir_name_absolute)
        goto cleanup;

    snprintf(dir_name_absolute_usable, PATH_MAX, "%s", dir_name_absolute);

    dir = opendir(dir_name_absolute);
    if (!dir)
    {
        build_err = ERR_DIR_OPEN_FAIL;
        goto cleanup;
    }

    while ((entry = readdir(dir)) != NULL)
        ++contents.memb;

    /* subtract for '.' and '..' */
    contents.memb -= 2;

    if (!contents.memb || mem_alloc_buf(&contents, contents.memb,
                NAME_MAX, "get_dir_contents().dir_contents") != ERR_SUCCESS)
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

        if (is_dir(entry_name_full) == ERR_SUCCESS)
            check_slash(contents.i[i]);
        ++i;
    }

    closedir(dir);
    mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute),
            "get_dir_contents().dir_name_absolute");

    build_err = ERR_SUCCESS;
    return contents;

cleanup:

    if (dir) closedir(dir);
    mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute),
            "get_dir_contents().dir_name_absolute");
    mem_free_buf((void*)&contents, "get_dir_contents().dir_contents");
    return (_buf){0};
}

u32 copy_file(const str *src, const str *dst)
{
    str str_dst[PATH_MAX] = {0};
    str *in_file = NULL;
    FILE *out_file = NULL;
    u64 len = 0;

    if (is_file_exists(src, TRUE) != ERR_SUCCESS)
            return build_err;

    snprintf(str_dst, PATH_MAX, "%s", dst);

    if (is_dir(dst) == ERR_SUCCESS)
    {
        check_slash(str_dst);
        get_base_name(src, str_dst + strlen(str_dst), PATH_MAX);
        posix_slash(str_dst);
    }

    if ((out_file = fopen(str_dst, "wb")) == NULL)
    {
        LOGERROR(ERR_FILE_OPEN_FAIL, FALSE,
                "Failed to Copy File '%s' -> '%s'\n", src, str_dst);
        return build_err;
    }

    len = get_file_contents(src, (void*)&in_file, 1, FALSE);
    if (build_err != ERR_SUCCESS || !in_file)
    {
        fclose(out_file);
        return build_err;
    }

    fwrite(in_file, 1, len, out_file);
    fclose(out_file);

    LOGTRACE(FALSE,
            "File Copied '%s' -> '%s'\n", src, str_dst);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 copy_dir(const str *src, const str *dst, b8 contents_only)
{
    _buf dir_contents = {0};
    str str_src[PATH_MAX] = {0};
    str str_dst[PATH_MAX] = {0};
    str in_dir[PATH_MAX] = {0};
    str out_dir[PATH_MAX] = {0};
    u64 i;

    if (is_dir_exists(src, TRUE) != ERR_SUCCESS)
        return build_err;

    dir_contents = get_dir_contents(src);
    if (!dir_contents.loaded)
        return build_err;

    snprintf(str_src, PATH_MAX, "%s", src);
    check_slash(str_src);
    posix_slash(str_src);

    snprintf(str_dst, PATH_MAX, "%s", dst);
    check_slash(str_dst);
    posix_slash(str_dst);


    if (is_dir_exists(str_dst, FALSE) == ERR_SUCCESS && !contents_only)
    {
        strncat(str_dst, strrchr(str_src, SLASH_NATIVE), PATH_MAX - 1);
        check_slash(str_dst);
    }
    else make_dir(str_dst);

    for (i = 0; i < dir_contents.memb; ++i)
    {
        snprintf(in_dir, PATH_MAX - 1, "%s%s", str_src, (str*)dir_contents.i[i]);
        snprintf(out_dir, PATH_MAX - 1, "%s%s", str_dst, (str*)dir_contents.i[i]);

        if (is_dir(in_dir) == ERR_SUCCESS)
        {
            copy_dir(in_dir, out_dir, TRUE);
            continue;
        }
        copy_file(in_dir, out_dir);
    }

    LOGTRACE(FALSE,
            "Directory Copied '%s' -> '%s'\n", src, str_dst);

    build_err = ERR_SUCCESS;
    return build_err;
}

str *get_path_absolute(const str *name)
{
    str path_absolute[PATH_MAX] = {0};
    str *result = NULL;
    u64 len = 0;

    if (strlen(name) >= PATH_MAX - 1)
    {
        LOGERROR(ERR_GET_PATH_ABSOLUTE_FAIL, TRUE,
                "%s\n", "Failed to Get Absolute Path, Path Too Long");
        return NULL;
    }

    if (is_dir_exists(name, TRUE) != ERR_SUCCESS)
        return NULL;

    if (_get_path_absolute(name, path_absolute) != ERR_SUCCESS)
        return NULL;

    len = strlen(path_absolute) + 1;

    if (mem_alloc((void*)&result, sizeof(str*) * (len + 1),
                "get_path_absolute().result") != ERR_SUCCESS)
        return NULL;

    strncpy(result, path_absolute, len);
    check_slash(result);

    build_err = ERR_SUCCESS;
    return result;
}

str *get_path_bin_root(void)
{
    str path_bin_root[PATH_MAX] = {0};
    str *result = NULL;
    u64 len = 0;
    char *last_slash = NULL;

    if (_get_path_bin_root(path_bin_root) != ERR_SUCCESS)
        return NULL;

    len = strlen(path_bin_root) + 1;
    if (len >= PATH_MAX - 1)
    {
        LOGFATAL(ERR_PATH_TOO_LONG, TRUE,
                "Path Too Long '%s', Process Aborted\n", path_bin_root);
        return NULL;
    }

    path_bin_root[len] = 0;
    if (mem_alloc((void*)&result, PATH_MAX,
                "get_path_bin_root().path_bin_root") != ERR_SUCCESS)
        return NULL;

    strncpy(result, path_bin_root, len);

    last_slash = strrchr(result, '/');
    if (last_slash)
        *last_slash = 0;
    check_slash(result);
    posix_slash(result);

    build_err = ERR_SUCCESS;
    return result;
}

void check_slash(str *path)
{
    u64 len = 0;

    if (!path)
    {
        build_err = ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);
    if (len >= PATH_MAX - 1)
    {
        build_err = ERR_PATH_TOO_LONG;
        return;
    }

    if (path[len - 1] == '/' || path[len - 1] == '\\')
    {
        build_err = ERR_SUCCESS;
        return;
    }

    path[len] = SLASH_NATIVE;
    path[len + 1] = 0;
    build_err = ERR_SUCCESS;
}

void posix_slash(str *path)
{
    u64 len, i;

    if (!path)
    {
        build_err = ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);

    for (i = 0; i < len; ++i)
    {
        if (path[i] == '\\')
            path[i] = '/';
    }

    build_err = ERR_SUCCESS;
}

u32 get_base_name(const str *path, str *dst, u64 size)
{
    i64 i = 0;
    u64 len = 0;
    str path_resolved[PATH_MAX] = {0};

    if (size == 0)
    {
        LOGERROR(ERR_SIZE_TOO_SMALL, TRUE,
                "Failed to Get Base Name of '%s', 'size' Too Small\n", path);
        return build_err;
    }

    if (!path || !path[0] || !dst)
    {
        LOGERROR(ERR_POINTER_NULL, TRUE,
                "%s\n", "Failed to Get Base Name, Pointer NULL");
        return build_err;
    }

    len = strlen(path);
    if (len >= PATH_MAX - 1)
    {
        LOGERROR(ERR_PATH_TOO_LONG, TRUE,
                "Failed to Get Base Name of '%s', Path Too Long\n", path);
        return build_err;
    }

    snprintf(path_resolved, PATH_MAX, "%s", path);
    posix_slash(path_resolved);

    i = (i64)len - 1;
    if (path_resolved[i] == '/')
    {
        if (i == 0)
        {
            dst[0] = '/';
            build_err = ERR_SUCCESS;
            return build_err;
        }
        else --i;
    }

    while (i > 0 && path_resolved[i - 1] != '/')
        --i;

    snprintf(dst, size, "%s", path_resolved + i);

    build_err = ERR_SUCCESS;
    return build_err;
}
#endif /* BUILD_PLATFORM_H */
