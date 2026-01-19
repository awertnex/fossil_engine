#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <windows.h>
#include <direct.h>

#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/process.h"

u32 make_dir(const str *path)
{
    if (mkdir(path) == 0)
    {
        _LOGINFO(FALSE, "Directory Created '%s'\n", path);
        engine_err = ERR_SUCCESS;
        return engine_err;
    }

    switch (errno)
    {
        case EEXIST:
            engine_err = ERR_DIR_EXISTS;
            break;

        default:
            _LOGERROR(TRUE, ERR_DIR_CREATE_FAIL, "Failed to Create Directory '%s'\n", path);
    }

    return exit_code;
}

/* TODO: make 'change_dir()' for windows */
int change_dir(const str *path)
{
    return _chdir(path);
}

u32 _get_path_absolute(const str *path, str *path_real)
{
    if (!GetFullPathNameA(path, PATH_MAX, path_real, NULL))
    {
        engine_err = ERR_GET_PATH_ABSOLUTE_FAIL;
        return engine_err;
    }

    posix_slash(path_real);
    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _get_path_bin_root(str *path)
{
    if (strlen(_pgmptr) + 1 >= PATH_MAX)
    {
        _LOGFATAL(FALSE, ERR_GET_PATH_BIN_ROOT_FAIL,
                "%s\n", "Failed 'get_path_bin_root()', Process Aborted");
        return engine_err;
    }
    strncpy(path, _pgmptr, PATH_MAX);
    retract_path(path);
    posix_slash(path);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 exec(Buf *cmd, str *cmd_name)
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
        _LOGERROR(TRUE, ERR_BUFFER_EMPTY,
                "Failed to Execute '%s', cmd Empty\n", cmd_name);
        return engine_err;
    }

    if (mem_alloc((void*)&cmd_cat, cmd->size * cmd->memb,
            stringf("exec().%s", cmd_name)) != ERR_SUCCESS)
        return engine_err;

    for (i = 0; i < cmd->memb; ++i)
        strncat(cmd_cat, stringf("%s ", cmd->i[i]), cmd->size);

    if(!CreateProcessA(NULL, cmd_cat, NULL, NULL, FALSE, 0, NULL, NULL,
                &startup_info, &process_info))
    {
        _LOGFATAL(TRUE, ERR_EXEC_FAIL,
                "Failed to Fork '%s', Process Aborted\n", cmd_name);
        goto cleanup;
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);

    GetExitCodeProcess(process_info.hProcess, &exit_code);

    CloseHandle(process_info.hProcess);
    CloseHandle(process_info.hThread);

    if (exit_code == 0)
        _LOGINFO(FALSE, "'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
    else
    {
        engine_err = ERR_EXEC_PROCESS_NON_ZERO;
        _LOGINFO(TRUE, "'%s' Exit Code: %d\n", cmd_name, exit_code);
        goto cleanup;
    }

    mem_free((void*)&cmd_cat, cmd->memb * cmd->size, stringf("exec().%s", cmd_name));

    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:

    mem_free((void*)&cmd_cat, cmd->memb * cmd->size, stringf("exec().%s", cmd_name));
    return engine_err;
}

/* TODO: make '_mem_request_page_size()' for windows */
u64 _mem_request_page_size(void)
{
    return sysconf(_SC_PAGESIZE);
}

u32 _mem_map(void **x, u64 size, const str *name, const str *file, u64 line)
{
    u64 size_aligned = 0;
    void *temp = NULL;

    if (!x)
    {
        engine_err = ERR_POINTER_NULL;
        return engine_err;
    }

    if (*x)
    {
        engine_err = ERR_POINTER_NOT_NULL;
        return engine_err;
    }

    mem_request_page_size();
    size_aligned = align_up_u64(size, _PAGE_SIZE);

    temp = VirtualAlloc(NULL, size_aligned, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!temp)
    {
        _LOGFATALEX(TRUE, file, line, ERR_MEM_MAP_FAIL,
                "%s[%p] Failed to Map Memory, Process Aborted\n", name, *x);
        return engine_err;
    }
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Mapped [%"PRIu64"B]\n", name, temp, size_aligned);

    *x = temp;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_commit(void **x, void *offset, u64 size, const str *name, const str *file, u64 line)
{
    u64 size_aligned = 0;

    if (!x || !*X || !offset)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p][%p] Failed to Commit Memory [%"PRIu64"B], Pointer NULL\n",
                name, x, offset, size);
        return engine_err;
    }

    mem_request_page_size();
    size_aligned = align_up_u64(size, _PAGE_SIZE);

    if (!VirtualAlloc((*(u8*)x + (u8*)offset), size_aligned, MEM_COMMIT, PAGE_READWRITE))
    {
        _LOGFATALEX(TRUE, file, line, ERR_MEM_COMMIT_FAIL,
                "%s[%p][%p] Failed to Commit Memory [%"PRIu64"B], Process Aborted\n",
                name, *x, offset, size_aligned);
        return engine_err;
    }
    _LOGTRACEEX(TRUE, file, line, "%s[%p][%p] Memory Committed [%"PRIu64"B]\n",
            name, *x, offset, size_aligned);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

/* TODO: make '_mem_remap()' for windows */
u32 _mem_remap(void **x, u64 size_old, u64 size_new, const str *name, const str *file, u64 line)
{
    u64 size_old_aligned = 0;
    u64 size_new_aligned = 0;
    void *temp = NULL;

    if (!x || !*x)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Remap Memory, Pointer NULL\n", name, NULL);
        return engine_err;
    }

    mem_request_page_size();
    size_old_aligned = align_up_u64(size_old, _PAGE_SIZE);
    size_new_aligned = align_up_u64(size_new, _PAGE_SIZE);

    temp = mremap(*x, size_old_aligned, size_new_aligned, MREMAP_MAYMOVE);
    if (temp == MAP_FAILED)
    {
        _LOGERROREX(TRUE, file, line, ERR_MEM_REMAP_FAIL,
                "%s[%p] Failed to Remap Memory\n", name, *x);
        return engine_err;
    }
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Remapped [%"PRIu64"B] -> [%"PRIu64"B]\n",
            name, *x, size_old_aligned, size_new_aligned);

    *x = temp;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void _mem_unmap(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (!x || !*x) return;
    VirtualFree(x, 0, MEM_RELEASE);
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unmapped [%"PRIu64"B]\n", name, *x, size);
    *x = NULL;
}

void _mem_unmap_arena(MemArena *x, const str *name, const str *file, u64 line)
{
    if (!x || !x->buf) return;
    VirtualFree(x->i, 0, MEM_RELEASE);
    VirtualFree(x->buf, 0, MEM_RELEASE);
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Arena Unmapped [%"PRIu64"B] Memb Total [%"PRIu64"][%"PRIu64"B]\n",
            name, x->buf, x->size_buf, x->memb, x->size_i);
    *x = (MemArena){0};
}
