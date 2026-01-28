#ifndef BUILD_MEMORY_H
#define BUILD_MEMORY_H

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

#define arr_len(arr) ((u64)sizeof(arr) / sizeof(arr[0]))

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
 *  @return non-zero on failure and @ref build_err is set accordingly.
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
        LOGFATALEX(ERR_MEM_ALLOC_FAIL, TRUE, file, line,
                "%s[%p] Failed to Allocate Memory, Process Aborted\n", name, NULL);
        return build_err;
    }
    LOGTRACEEX(TRUE, file, line,
            "%s[%p] Memory Allocated [%"PRIu64"B]\n", name, *x, size);

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
        LOGFATALEX(ERR_MEM_ALLOC_FAIL, TRUE, file, line,
                "%s[%p] Failed to Allocate Memory, Process Aborted\n", name, NULL);
        return build_err;
    }
    LOGTRACEEX(TRUE, file, line,
            "%s[%p] Memory Allocated [%"PRIu64"B]\n", name, *x, memb * size);

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
    LOGTRACEEX(TRUE, file, line,
            "%s[%p] Memory Unloaded\n", name, temp);
}

u32 _mem_alloc_buf(_buf *x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    u64 i = 0;

    if (!x)
    {
        LOGERROREX(ERR_POINTER_NULL, TRUE, file, line,
                "%s[%p] Failed to Allocate Memory, Pointer NULL\n", name, NULL);
        return build_err;
    }

    if (x->loaded || x->buf || x->i)
        return ERR_SUCCESS;

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
        LOGTRACEEX(TRUE, file, line,
                "%s[%p] Memory Unloaded\n", name_i, temp);
    }

    if (x->buf)
    {
        temp = x->buf;
        _mem_clear((void*)&x->buf, x->memb * x->size, name_buf, file, line);
        free(x->buf);
        LOGTRACEEX(TRUE, file, line,
                "%s[%p] Memory Unloaded\n", name_buf, temp);
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
    LOGTRACEEX(TRUE, file, line,
            "%s[%p] Memory Cleared [%"PRIu64"B]\n", name, *x, size);

    build_err = ERR_SUCCESS;
    return build_err;
}

#endif /* BUILD_MEMORY_H */
