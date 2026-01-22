#ifndef BUILD_MEMORY_H
#define BUILD_MEMORY_H

#include "common.h"
#include "diagnostics.h"
#include "logger.h"

#define arr_len(arr) ((u64)sizeof(arr) / sizeof(arr[0]))

extern u64 align_up_u64(u64 n, u64 size);

#define mem_alloc(x, size, name) \
    _mem_alloc(x, size, name, __BASE_FILE__, __LINE__)

#define mem_alloc_memb(x, memb, size, name) \
    _mem_alloc_memb(x, memb, size, name, __BASE_FILE__, __LINE__)

#define mem_free(x, size, name) \
    _mem_free(x, size, name, __BASE_FILE__, __LINE__)

#define mem_alloc_buf(x, memb, size, name) \
    _mem_alloc_buf(x, memb, size, name, __BASE_FILE__, __LINE__)

#define mem_free_buf(x, name) \
    _mem_free_buf(x, name, __BASE_FILE__, __LINE__)

#define mem_clear(x, size, name) \
    _mem_clear(x, size, name, __BASE_FILE__, __LINE__)

/* ---- section: signatures ------------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
extern u32 _mem_alloc(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members.
 *  @param size member size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 _mem_alloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 */
extern void _mem_free(void **x, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param memb number of members.
 *  @param size member size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 _mem_alloc_buf(_buf *x, u64 memb, u64 size, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param name pointer name (for logging).
 */
extern void _mem_free_buf(_buf *x, const str *name, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param size size in bytes.
 *  @param name pointer name (for logging).
 *
 *  @return non-zero on failure and @ref build_err is set accordingly.
 */
extern u32 _mem_clear(void **x, u64 size, const str *name, const str *file, u64 line);

/* ---- section: implementation --------------------------------------------- */

u64 align_up_u64(u64 n, u64 size)
{
    return (n + (size - 1)) & ~(size - 1);
}

u32 _mem_alloc(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (*x)
    {
        build_err = ERR_POINTER_NOT_NULL;
        return build_err;
    }

    *x = calloc(1, size);
    if (!x || !*x)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_ALLOC_FAIL,
                "%s[%p] Failed to Allocate Memory, Process Aborted\n", name, NULL);
        return build_err;
    }
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Allocated [%"PRIu64"B]\n", name, *x, size);

    build_err = ERR_SUCCESS;
    return build_err;
}

u32 _mem_alloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    if (*x)
        return ERR_SUCCESS;

    *x = calloc(memb, size);
    if (!x || !*x)
    {
        LOGFATALEX(TRUE, file, line, ERR_MEM_ALLOC_FAIL,
                "%s[%p] Failed to Allocate Memory, Process Aborted\n", name, NULL);
        return build_err;
    }
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Allocated [%"PRIu64"B]\n", name, *x, memb * size);

    build_err = ERR_SUCCESS;
    return build_err;
}

void _mem_free(void **x, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;
    if (!x || !*x)
        return;

    temp = *x;
    _mem_clear(x, size, name, file, line);
    free(*x);
    *x = NULL;
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name, temp);
}

u32 _mem_alloc_buf(_buf *x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    u64 i = 0;

    if (!x)
    {
        LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Allocate Memory, Pointer NULL\n", name, NULL);
        return build_err;
    }

    snprintf(name_i, NAME_MAX, "%s.i", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (_mem_alloc_memb((void*)&x->i,
                memb, sizeof(str*), name_i, file, line) != ERR_SUCCESS)
        return build_err;

    if (_mem_alloc_memb((void*)&x->buf, memb, size, name_buf, file, line) != ERR_SUCCESS)
    {
        _mem_free((void*)&x->i, memb * sizeof(str*), name_i, file, line);
        return build_err;
    }

    for (i = 0; i < memb; ++i)
        x->i[i] = x->buf + i * size;

    x->memb = memb;
    x->size = size;
    x->loaded = TRUE;

    build_err = ERR_SUCCESS;
    return build_err;
}

void _mem_free_buf(_buf *x, const str *name, const str *file, u64 line)
{
    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    void *temp = NULL;

    if (!x) return;

    snprintf(name_i, NAME_MAX, "%s.i", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (x->i)
    {
        temp = x->i;
        _mem_clear((void*)&x->i, x->memb * sizeof(str*), name_i, file, line);
        free(x->i);
        LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name_i, temp);
    }

    if (x->buf)
    {
        temp = x->buf;
        _mem_clear((void*)&x->buf, x->memb * x->size, name_buf, file, line);
        free(x->buf);
        LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name_buf, temp);
    }

    *x = (_buf){0};
}

u32 _mem_clear(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (!x || !*x)
    {
        build_err = ERR_POINTER_NULL;
        return build_err;
    }

    bzero(*x, size);
    LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Cleared [%"PRIu64"B]\n", name, *x, size);

    build_err = ERR_SUCCESS;
    return build_err;
}

#endif /* BUILD_MEMORY_H */
