#include "h/common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <inttypes.h>

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

    _LOGERROR(TRUE, FSL_ERR_FILE_NOT_FOUND, "File '%s' Not Found\n", name);
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
            {
                _LOGERROR(TRUE, FSL_ERR_IS_NOT_FILE, "'%s' is Not a Regular File\n", name);
            }
            else fsl_err = FSL_ERR_IS_NOT_FILE;
            return fsl_err;
        }
    }

    if (log)
    {
        _LOGERROR(TRUE, FSL_ERR_FILE_NOT_FOUND, "File '%s' Not Found\n", name);
    }
    else fsl_err = FSL_ERR_FILE_NOT_FOUND;
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
            {
                _LOGERROR(TRUE, FSL_ERR_IS_NOT_DIR, "'%s' is Not a Directory\n", name);
            }
            else fsl_err = FSL_ERR_IS_NOT_DIR;
            return fsl_err;
        }
    }

    if (log)
    {
        _LOGERROR(TRUE, FSL_ERR_DIR_NOT_FOUND, "Directory '%s' Not Found\n", name);
    }
    else fsl_err = FSL_ERR_DIR_NOT_FOUND;
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
        _LOGERROR(TRUE, FSL_ERR_FILE_OPEN_FAIL, "Failed to Open File '%s'\n", name);
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

    dir_name_absolute = fsl_get_path_absolute(name);
    if (!dir_name_absolute)
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

u64 get_dir_entry_count(const str *name)
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

    if (fsl_is_file_exists(src, TRUE) != FSL_ERR_SUCCESS)
            return fsl_err;

    snprintf(str_dst, PATH_MAX, "%s", dst);

    if (fsl_is_dir(dst) == FSL_ERR_SUCCESS)
        strncat(str_dst, strrchr(src, FSL_SLASH_NATIVE), PATH_MAX - 1);

    if ((out_file = fopen(str_dst, "wb")) == NULL)
    {
        _LOGERROR(FALSE, FSL_ERR_FILE_OPEN_FAIL, "Failed to Copy File '%s' -> '%s'\n",
                src, str_dst);
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

    _LOGTRACE(FALSE, "File Copied '%s' -> '%s'\n", src, str_dst);

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
        strncat(str_dst, strrchr(str_src, FSL_SLASH_NATIVE), PATH_MAX - 1);
        fsl_check_slash(str_dst);
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

    _LOGTRACE(FALSE, "Directory Copied '%s' -> '%s'\n", src, str_dst);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 write_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    FILE *file = NULL;
    if ((file = fopen(name, "wb")) == NULL)
    {
        if (log)
        {
            _LOGERROR(TRUE, FSL_ERR_FILE_OPEN_FAIL, "Failed to Write File '%s'\n", name);
        }
        else fsl_err = FSL_ERR_FILE_OPEN_FAIL;
        return fsl_err;
    }

    fwrite(buf, size, length, file);
    if (text) fprintf(file, "%c", '\n');
    fclose(file);

    _LOGTRACE(FALSE, "File Written '%s'\n", name);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    if (_fsl_append_file(name, size, length, buf, log, text) == FSL_ERR_SUCCESS)
        _LOGTRACE(FALSE, "File Appended '%s'\n", name);

    return fsl_err;
}

u32 _fsl_append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    FILE *file = NULL;
    if ((file = fopen(name, "ab")) == NULL)
    {
        if (log)
        {
            _LOGERROR(TRUE, FSL_ERR_FILE_OPEN_FAIL, "Failed to Append File '%s'\n", name);
        }
        else fsl_err = FSL_ERR_FILE_OPEN_FAIL;
        return fsl_err;
    }

    fwrite(buf, size, length, file);
    if (text) fprintf(file, "%c", '\n');
    fclose(file);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

str *fsl_get_path_absolute(const str *name)
{
    str path_absolute[PATH_MAX] = {0};
    str *result = NULL;
    u64 len = 0;

    if (strlen(name) >= PATH_MAX - 1)
    {
        _LOGERROR(TRUE, FSL_ERR_GET_PATH_ABSOLUTE_FAIL, "%s\n", "Failed to Get Absolute Path, Path Too Long");
        return NULL;
    }

    if (fsl_is_dir_exists(name, TRUE) != FSL_ERR_SUCCESS)
        return NULL;

    if (_fsl_get_path_absolute(name, path_absolute) != FSL_ERR_SUCCESS)
        return NULL;

    len = strlen(path_absolute) + 1;

    if (fsl_mem_alloc((void*)&result, sizeof(str*) * (len + 1),
                "fsl_get_path_absolute().result") != FSL_ERR_SUCCESS)
        return NULL;

    strncpy(result, path_absolute, len);
    fsl_check_slash(result);

    fsl_err = FSL_ERR_SUCCESS;
    return result;
}

str *fsl_get_path_bin_root(void)
{
    str path_bin_root[PATH_MAX] = {0};
    str *result = NULL;
    u64 len = 0;
    char *last_slash = NULL;

    if (_fsl_get_path_bin_root(path_bin_root) != FSL_ERR_SUCCESS)
        return NULL;

    len = strlen(path_bin_root) + 1;
    if (len >= PATH_MAX - 1)
    {
        _LOGFATAL(TRUE, FSL_ERR_PATH_TOO_LONG,
                "Path Too Long '%s', Process Aborted\n", path_bin_root);
        return NULL;
    }

    path_bin_root[len] = 0;
    if (fsl_mem_alloc((void*)&result, PATH_MAX,
                "fsl_get_path_bin_root().path_bin_root") != FSL_ERR_SUCCESS)
        return NULL;

    strncpy(result, path_bin_root, len);

    last_slash = strrchr(result, '/');
    if (last_slash)
        *last_slash = 0;
    fsl_check_slash(result);
    fsl_normalize_slash(result);

    fsl_err = FSL_ERR_SUCCESS;
    return result;
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

str *retract_path(str *path)
{
    u64 len, i, stage = 0;

    if (!path)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return NULL;
    }

    len = strlen(path);
    if (len <= 1) return path;

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
    return path;
}

void fsl_get_base_name(const str *path, str *dst, u64 size)
{
    i64 i = 0;
    u64 len = 0;
    str path_resolved[PATH_MAX] = {0};

    if (size == 0)
    {
        _LOGERROR(TRUE, FSL_ERR_SIZE_TOO_SMALL,
                "Failed to Get Base Name of '%s', 'size' Too Small\n", path);
        return;
    }

    if (!path || !path[0] || !dst)
    {
        _LOGERROR(TRUE, FSL_ERR_POINTER_NULL, "%s\n", "Failed to Get Base Name, Pointer NULL");
        return;
    }

    len = strlen(path);
    if (len >= PATH_MAX - 1)
    {
        _LOGERROR(TRUE, FSL_ERR_PATH_TOO_LONG,
                "Failed to Get Base Name of '%s', Path Too Long\n", path);
        return;
    }

    snprintf(path_resolved, PATH_MAX, "%s", path);
    fsl_posix_slash(path_resolved);

    i = (i64)len - 1;
    if (path_resolved[i] == '/')
    {
        if (i == 0)
        {
            dst[0] = '/';
            return;
        }
        else --i;
    }

    while (i > 0 && path_resolved[i - 1] != '/')
        --i;

    snprintf(dst, size, "%s", path_resolved + i);
}
