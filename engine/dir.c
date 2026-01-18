#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <inttypes.h>

#include "h/diagnostics.h"
#include "h/platform.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"

u64 get_file_type(const str *name)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
        return S_ISREG(stats.st_mode) | (S_ISDIR(stats.st_mode) * 2);

    _LOGERROR(TRUE, ERR_FILE_NOT_FOUND, "File '%s' Not Found\n", name);
    return 0;
}

u32 is_file(const str *name)
{
    if (is_file_exists(name, FALSE) != ERR_SUCCESS)
        return engine_err;

    struct stat stats;
    if (stat(name, &stats) == 0 && S_ISREG(stats.st_mode))
    {
        engine_err = ERR_SUCCESS;
        return engine_err;
    }

    engine_err = ERR_IS_NOT_FILE;
    return engine_err;
}

u32 is_file_exists(const str *name, b8 log)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
    {
        if (S_ISREG(stats.st_mode))
        {
            engine_err = ERR_SUCCESS;
            return engine_err;
        }
        else
        {
            if (log)
            {
                _LOGERROR(TRUE, ERR_IS_NOT_FILE, "'%s' is Not a Regular File\n", name);
            }
            else engine_err = ERR_IS_NOT_FILE;
            return engine_err;
        }
    }

    if (log)
    {
        _LOGERROR(TRUE, ERR_FILE_NOT_FOUND, "File '%s' Not Found\n", name);
    }
    else engine_err = ERR_FILE_NOT_FOUND;
    return engine_err;
}

u32 is_dir(const str *name)
{
    if (is_dir_exists(name, FALSE) != ERR_SUCCESS)
        return engine_err;

    struct stat stats;
    if (stat(name, &stats) == 0 && S_ISDIR(stats.st_mode))
    {
        engine_err = ERR_SUCCESS;
        return engine_err;
    }

    engine_err = ERR_IS_NOT_DIR;
    return engine_err;
}

u32 is_dir_exists(const str *name, b8 log)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
        {
            engine_err = ERR_SUCCESS;
            return engine_err;
        }
        else
        {
            if (log)
            {
                _LOGERROR(TRUE, ERR_IS_NOT_DIR, "'%s' is Not a Directory\n", name);
            }
            else engine_err = ERR_IS_NOT_DIR;
            return engine_err;
        }
    }

    if (log)
    {
        _LOGERROR(TRUE, ERR_DIR_NOT_FOUND, "Directory '%s' Not Found\n", name);
    }
    else engine_err = ERR_DIR_NOT_FOUND;
    return engine_err;
}

u64 get_file_contents(const str *name, void **destination, u64 size, b8 terminate)
{
    FILE *file = NULL;
    u64 cursor;

    if (is_file_exists(name, TRUE) != ERR_SUCCESS)
            return 0;

    if ((file = fopen(name, "rb")) == NULL)
    {
        _LOGERROR(TRUE, ERR_FILE_OPEN_FAIL, "Failed to Open File '%s'\n", name);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    u64 len = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (mem_alloc(destination, len + (terminate ? 1 : 0),
                "get_file_contents().destination") != ERR_SUCCESS)
        goto cleanup;

    cursor = fread(*destination, size, len, file);

    fclose(file);
    if (terminate) ((u8*)(*destination))[len] = 0;
    engine_err = ERR_SUCCESS;
    return cursor;

cleanup:

    if (file) fclose(file);
    return 0;
}

Buf get_dir_contents(const str *name)
{
    str *dir_name_absolute = NULL;
    str dir_name_absolute_usable[PATH_MAX] = {0};
    str entry_name_full[PATH_MAX] = {0};
    DIR *dir = NULL;
    struct dirent *entry;
    Buf contents = {0};
    u64 i;

    if (!name)
    {
        engine_err = ERR_POINTER_NULL;
        return (Buf){0};
    }

    if (is_dir_exists(name, TRUE) != ERR_SUCCESS)
        return (Buf){0};

    dir_name_absolute = get_path_absolute(name);
    if (!dir_name_absolute)
        goto cleanup;

    snprintf(dir_name_absolute_usable, PATH_MAX, "%s", dir_name_absolute);

    dir = opendir(dir_name_absolute);
    if (!dir)
    {
        engine_err = ERR_DIR_OPEN_FAIL;
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

    engine_err = ERR_SUCCESS;
    return contents;

cleanup:

    if (dir) closedir(dir);
    mem_free((void*)&dir_name_absolute, strlen(dir_name_absolute),
            "get_dir_contents().dir_name_absolute");
    mem_free_buf((void*)&contents, "get_dir_contents().dir_contents");
    return (Buf){0};
}

u64 get_dir_entry_count(const str *name)
{
    DIR *dir = NULL;
    u64 count;
    struct dirent *entry;

    if (!name)
    {
        engine_err = ERR_POINTER_NULL;
        return 0;
    }

    if (is_dir_exists(name, TRUE) != ERR_SUCCESS)
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

    engine_err = ERR_SUCCESS;
    return count;
}

u32 copy_file(const str *source, const str *destination)
{
    str str_dst[PATH_MAX] = {0};
    str *in_file = NULL;
    FILE *out_file = NULL;
    u64 len = 0;

    if (is_file_exists(source, TRUE) != ERR_SUCCESS)
            return engine_err;

    snprintf(str_dst, PATH_MAX, "%s", destination);

    if (is_dir(destination) == ERR_SUCCESS)
        strncat(str_dst, strrchr(source, SLASH_NATIVE), PATH_MAX - 1);

    if ((out_file = fopen(str_dst, "wb")) == NULL)
    {
        _LOGERROR(FALSE, ERR_FILE_OPEN_FAIL, "Failed to Copy File '%s' -> '%s'\n",
                source, str_dst);
        return engine_err;
    }

    len = get_file_contents(source, (void*)&in_file, 1, FALSE);
    if (engine_err != ERR_SUCCESS || !in_file)
    {
        fclose(out_file);
        return engine_err;
    }

    fwrite(in_file, 1, len, out_file);
    fclose(out_file);

    _LOGTRACE(FALSE, "File Copied '%s' -> '%s'\n", source, str_dst);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 copy_dir(const str *source, const str *destination, b8 contents_only)
{
    Buf dir_contents = {0};
    str str_src[PATH_MAX] = {0};
    str str_dst[PATH_MAX] = {0};
    str in_dir[PATH_MAX] = {0};
    str out_dir[PATH_MAX] = {0};
    u64 i;

    if (is_dir_exists(source, TRUE) != ERR_SUCCESS)
        return engine_err;

    dir_contents = get_dir_contents(source);
    if (!dir_contents.loaded)
        return engine_err;

    snprintf(str_src, PATH_MAX, "%s", source);
    check_slash(str_src);
    normalize_slash(str_src);

    snprintf(str_dst, PATH_MAX, "%s", destination);
    check_slash(str_dst);
    normalize_slash(str_dst);


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

    _LOGTRACE(FALSE, "Directory Copied '%s' -> '%s'\n", source, str_dst);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 write_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    if (_write_file(name, size, length, buf, log, text) == ERR_SUCCESS)
        _LOGTRACE(FALSE, "File Written '%s'\n", name);

    return engine_err;
}

u32 _write_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    FILE *file = NULL;
    if ((file = fopen(name, "wb")) == NULL)
    {
        if (log)
        {
            _LOGERROR(TRUE, ERR_FILE_OPEN_FAIL, "Failed to Open File '%s'\n", name);
        }
        else engine_err = ERR_FILE_OPEN_FAIL;
        return engine_err;
    }

    fwrite(buf, size, length, file);
    if (text) fprintf(file, "%c", '\n');
    fclose(file);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    if (_append_file(name, size, length, buf, log, text) == ERR_SUCCESS)
        _LOGTRACE(FALSE, "File Appended '%s'\n", name);

    return engine_err;
}

u32 _append_file(const str *name, u64 size, u64 length, void *buf, b8 log, b8 text)
{
    FILE *file = NULL;
    if ((file = fopen(name, "ab")) == NULL)
    {
        if (log)
        {
            _LOGERROR(TRUE, ERR_FILE_OPEN_FAIL, "Failed to Open File '%s'\n", name);
        }
        else engine_err = ERR_FILE_OPEN_FAIL;
        return engine_err;
    }

    fwrite(buf, size, length, file);
    if (text) fprintf(file, "%c", '\n');
    fclose(file);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

str *get_path_absolute(const str *name)
{
    str path_absolute[PATH_MAX] = {0};
    str *result = NULL;
    u64 len = 0;

    if (strlen(name) >= PATH_MAX - 1)
    {
        _LOGERROR(TRUE, ERR_GET_PATH_ABSOLUTE_FAIL, "%s\n", "Path Too Long");
        return NULL;
    }

    if (is_dir_exists(name, TRUE) != ERR_SUCCESS)
        return NULL;

    if (_get_path_absolute(name, path_absolute) != ERR_SUCCESS)
        return NULL;

    len = strlen(path_absolute) + 1;

    if (mem_alloc((void*)&result, sizeof(str*) * (len + 1),
                "get_path_absolute().path_absolute") != ERR_SUCCESS)
        return NULL;

    strncpy(result, path_absolute, len);
    check_slash(result);

    engine_err = ERR_SUCCESS;
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
        _LOGFATAL(TRUE, ERR_PATH_TOO_LONG,
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
    normalize_slash(result);

    engine_err = ERR_SUCCESS;
    return result;
}

void check_slash(str *path)
{
    u64 len = 0;

    if (!path)
    {
        engine_err = ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);
    if (len >= PATH_MAX - 1)
    {
        engine_err = ERR_PATH_TOO_LONG;
        return;
    }

    if (path[len - 1] == SLASH_NATIVE || path[len - 1] == SLASH_NON_NATIVE)
    {
        engine_err = ERR_SUCCESS;
        return;
    }

    path[len] = SLASH_NATIVE;
    path[len + 1] = 0;
    engine_err = ERR_SUCCESS;
}

void normalize_slash(str *path)
{
    u64 len, i;

    if (!path)
    {
        engine_err = ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);

    for (i = 0; i < len; ++i)
    {
        if (path[i] == SLASH_NON_NATIVE)
            path[i] = SLASH_NATIVE;
    }

    engine_err = ERR_SUCCESS;
}

void posix_slash(str *path)
{
    u64 len, i;

    if (!path)
    {
        engine_err = ERR_POINTER_NULL;
        return;
    }

    len = strlen(path);

    for (i = 0; i < len; ++i)
    {
        if (path[i] == '\\')
            path[i] = '/';
    }

    engine_err = ERR_SUCCESS;
}

str *retract_path(str *path)
{
    u64 len, i, stage = 0;

    if (!path)
    {
        engine_err = ERR_POINTER_NULL;
        return NULL;
    }

    len = strlen(path);
    if (len <= 1) return path;

    for (i = 0; i < len; ++i)
    {
        if (stage == 1 &&
                (path[len - i - 1] == SLASH_NATIVE ||
                 path[len - i - 1] == SLASH_NON_NATIVE))
            break;
        if (path[len - i - 1])
        {
            path[len - i - 1] = 0;
            stage = 1;
        }
    }

    engine_err = ERR_SUCCESS;
    return path;
}
