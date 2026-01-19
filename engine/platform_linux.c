#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/mman.h>

#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/math.h"
#include "h/process.h"

u32 make_dir(const str *path)
{
    if (mkdir(path, 0755) == 0)
    {
        _LOGTRACE(FALSE, "Directory Created '%s'\n", path);
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

    return engine_err;
}

int change_dir(const str *path)
{
    int success = chdir(path);
    _LOGTRACE(TRUE, "Working Directory Changed to '%s'\n", path);
    return success;
}

u32 _get_path_absolute(const str *path, str *path_real)
{
    if (!realpath(path, path_real))
    {
        engine_err = ERR_GET_PATH_ABSOLUTE_FAIL;
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _get_path_bin_root(str *path)
{
    if (!readlink("/proc/self/exe", path, PATH_MAX))
    {
        _LOGFATAL(FALSE, ERR_GET_PATH_BIN_ROOT_FAIL,
                "%s\n", "Failed 'get_path_bin_root()', Process Aborted");
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 exec(Buf *cmd, str *cmd_name)
{
    pid_t pid = fork();
    int status, exit_code = 0, sig;

    if (pid < 0)
    {
        _LOGERROR(TRUE, ERR_PROCESS_FORK_FAIL,
                "Failed to Fork '%s'\n", cmd_name);
        return engine_err;
    }
    else if (pid == 0)
    {
        execvp((const str*)cmd->i[0], (str *const *)cmd->i);
        _LOGERROR(TRUE, ERR_EXEC_FAIL, "Failed '%s'\n", cmd_name);
        return engine_err;
    }

    if (waitpid(pid, &status, 0) == -1)
    {
        _LOGERROR(TRUE, ERR_WAITPID_FAIL, "Failed to Waitpid '%s'\n", cmd_name);
        return engine_err;
    }

    if (WIFEXITED(status))
    {
        exit_code = WEXITSTATUS(status);
        if (exit_code == 0)
        {
            _LOGINFO(FALSE, "'%s' Success, Exit Code: %d\n", cmd_name, exit_code);
        }
        else
        {
            engine_err = ERR_EXEC_PROCESS_NON_ZERO;
            _LOGINFO(TRUE, "'%s' Exit Code: %d\n", cmd_name, exit_code);
            return engine_err;
        }
    }
    else if (WIFSIGNALED(status))
    {
        sig = WTERMSIG(status);
        _LOGFATAL(TRUE, ERR_EXEC_TERMINATE_BY_SIGNAL,
                "'%s' Terminated by Signal: %d, Process Aborted\n", cmd_name, sig);
        return engine_err;
    }
    else
    {
        _LOGERROR(TRUE, ERR_EXEC_ABNORMAL_EXIT, "'%s' Exited Abnormally\n", cmd_name);
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

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
    size_aligned = round_up_u64(size, _PAGE_SIZE);

    temp = mmap(NULL, size_aligned,
            PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
    if (temp == MAP_FAILED)
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

    if (!x || !*x || !offset)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p][%p] Failed to Commit Memory [%"PRIu64"B], Pointer NULL\n",
                name, x, offset, size);
        return engine_err;
    }

    mem_request_page_size();
    size_aligned = round_up_u64(size, _PAGE_SIZE);

    if (mprotect(offset, size_aligned, PROT_READ | PROT_WRITE) != 0)
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
    size_old_aligned = round_up_u64(size_old, _PAGE_SIZE);
    size_new_aligned = round_up_u64(size_new, _PAGE_SIZE);

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
    u64 size_aligned = 0;

    if (!x || !*x) return;

    mem_request_page_size();
    size_aligned = round_up_u64(size, _PAGE_SIZE);
    munmap(*x, size_aligned);
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unmapped [%"PRIu64"B]\n", name, *x, size_aligned);
    *x = NULL;
}

void _mem_unmap_arena(MemArena *x, const str *name, const str *file, u64 line)
{
    if (!x || !x->buf) return;
    munmap(x->i, x->size_i);
    munmap(x->buf, x->size_buf);
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Arena Unmapped [%"PRIu64"B] Memb %"PRIu64"[%"PRIu64"B]\n",
            name, x->buf, x->size_buf, x->memb, x->size_i);
    *x = (MemArena){0};
}
