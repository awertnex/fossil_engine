/*  @file memory.c
 *
 *  @brief memory management.
 *
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
 *  limitations under the License.OFTWARE.
 */

#include "h/common.h"

#include "h/diagnostics.h"
#include "h/math.h"
#include "h/memory.h"
#include "h/limits.h"
#include "h/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <inttypes.h>

u64 FSL_PAGE_SIZE = 0;
fsl_mem_arena _fsl_memory_arena_internal = {0};

u64 fsl_align_up_u64(u64 n, u64 size)
{
    return (n + (size - 1)) & ~(size - 1);
}

void fsl_mem_request_page_size(void)
{
    if (!FSL_PAGE_SIZE) FSL_PAGE_SIZE = _fsl_mem_request_page_size();
}

u32 _fsl_mem_alloc(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (*x)
    {
        fsl_err = FSL_ERR_POINTER_NOT_NULL;
        return fsl_err;
    }

    *x = calloc(1, size);
    if (!x || !*x)
    {
        _LOGFATALEX(0, FSL_ERR_MEM_ALLOC_FAIL,
                file, line,
                "%s[%p] Failed to Allocate Memory, Process Aborted\n", name, NULL);
        return fsl_err;
    }
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Allocated [%"PRIu64"B]\n", name, *x, size);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_alloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    if (*x)
        return FSL_ERR_SUCCESS;

    *x = calloc(memb, size);
    if (!x || !*x)
    {
        _LOGFATALEX(0, FSL_ERR_MEM_ALLOC_FAIL,
                file, line,
                "%s[%p] Failed to Allocate Memory, Process Aborted\n", name, NULL);
        return fsl_err;
    }
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Allocated [%"PRIu64"B]\n", name, *x, memb * size);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_alloc_buf(fsl_buf *x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    u64 i = 0;

    if (!x)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Allocate Memory, Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    if (x->loaded || x->buf || x->i)
        return FSL_ERR_SUCCESS;

    snprintf(name_i, NAME_MAX, "%s.i", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (_fsl_mem_alloc_memb((void*)&x->i,
                memb, sizeof(str*), name_i, file, line) != FSL_ERR_SUCCESS)
        return fsl_err;

    if (_fsl_mem_alloc_memb((void*)&x->buf, memb, size, name_buf, file, line) != FSL_ERR_SUCCESS)
    {
        _fsl_mem_free((void*)&x->i, memb * sizeof(str*), name_i, file, line);
        return fsl_err;
    }

    for (i = 0; i < memb; ++i)
        x->i[i] = x->buf + i * size;

    x->memb = memb;
    x->size = size;
    x->loaded = TRUE;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_alloc_key_val(fsl_key_value *x, u64 memb, u64 size_key, u64 size_val,
        const str *name, const str *file, u64 line)
{
    str name_key[NAME_MAX] = {0};
    str name_val[NAME_MAX] = {0};
    str name_buf_key[NAME_MAX] = {0};
    str name_buf_val[NAME_MAX] = {0};
    u64 i = 0;

    if (!x)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Allocate Memory, Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    snprintf(name_key, NAME_MAX, "%s.key", name);
    snprintf(name_val, NAME_MAX, "%s.val", name);
    snprintf(name_buf_key, NAME_MAX, "%s.buf_key", name);
    snprintf(name_buf_val, NAME_MAX, "%s.buf_val", name);

    if (
            _fsl_mem_alloc_memb((void*)&x->key,
                memb, sizeof(str*), name_key, file, line) != FSL_ERR_SUCCESS ||

            _fsl_mem_alloc_memb((void*)&x->val,
                memb, sizeof(str*), name_val, file, line) != FSL_ERR_SUCCESS ||

            _fsl_mem_alloc_memb((void*)&x->buf_key,
                memb, size_key, name_buf_key, file, line) != FSL_ERR_SUCCESS ||

            _fsl_mem_alloc_memb((void*)&x->buf_val,
                memb, size_val, name_buf_val, file, line) != FSL_ERR_SUCCESS)
    {
        _fsl_mem_free_key_val(x, name, file, line);
        return fsl_err;
    }

    for (i = 0; i < memb; ++i)
    {
        x->key[i] = x->buf_key + i * size_key;
        x->val[i] = x->buf_val + i * size_val;
    }

    x->memb = memb;
    x->size_key = size_key;
    x->size_val = size_val;
    x->loaded = TRUE;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_realloc(void **x, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;

    if (!x || !*x)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Reallocate Memory, Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    temp = realloc(*x, size);
    if (!temp)
    {
        _LOGFATALEX(0, FSL_ERR_MEM_REALLOC_FAIL,
                file, line,
                "%s[%p] Failed to Reallocate Memory, Process Aborted\n", name, *x);
        return fsl_err;
    }

    *x = temp;
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Reallocated [%"PRIu64"B]\n", name, *x, size);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_realloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;

    if (!x || !*x)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Reallocate Memory, Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    temp = realloc(*x, memb * size);
    if (!temp)
    {
        _LOGFATALEX(0, FSL_ERR_MEM_REALLOC_FAIL,
                file, line,
                "%s[%p] Failed to Reallocate Memory, Process Aborted\n", name, *x);
        return fsl_err;
    }

    *x = temp;
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Reallocated [%"PRIu64"B]\n", name, *x, memb * size);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void _fsl_mem_free(void **x, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;
    if (!x || !*x)
        return;

    temp = *x;
    _fsl_mem_clear(x, size, name, file, line);
    free(*x);
    *x = NULL;
    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Unloaded\n", name, temp);
}

void _fsl_mem_free_buf(fsl_buf *x, const str *name, const str *file, u64 line)
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
        _fsl_mem_clear((void*)&x->i, x->memb * sizeof(str*), name_i, file, line);
        free(x->i);
        _LOGTRACEEX(0,
                file, line, "%s[%p] Memory Unloaded\n", name_i, temp);
    }

    if (x->buf)
    {
        temp = x->buf;
        _fsl_mem_clear((void*)&x->buf, x->memb * x->size, name_buf, file, line);
        free(x->buf);
        _LOGTRACEEX(0,
                file, line, "%s[%p] Memory Unloaded\n", name_buf, temp);
    }

    *x = (fsl_buf){0};
}

void _fsl_mem_free_key_val(fsl_key_value *x, const str *name, const str *file, u64 line)
{
    str name_key[NAME_MAX] = {0};
    str name_val[NAME_MAX] = {0};
    str name_buf_key[NAME_MAX] = {0};
    str name_buf_val[NAME_MAX] = {0};
    void *temp = NULL;

    if (!x) return;

    snprintf(name_key, NAME_MAX, "%s.key", name);
    snprintf(name_val, NAME_MAX, "%s.val", name);
    snprintf(name_buf_key, NAME_MAX, "%s.buf_key", name);
    snprintf(name_buf_val, NAME_MAX, "%s.buf_val", name);

    if (x->key)
    {
        temp = x->key;
        _fsl_mem_clear((void*)&x->key, x->memb * sizeof(str*), name_key, file, line);
        free(x->key);
        _LOGTRACEEX(0,
                file, line, "%s[%p] Memory Unloaded\n", name_key, temp);
    }

    if (x->val)
    {
        temp = x->val;
        _fsl_mem_clear((void*)&x->val, x->memb * sizeof(str*), name_val, file, line);
        free(x->val);
        _LOGTRACEEX(0,
                file, line, "%s[%p] Memory Unloaded\n", name_val, temp);
    }

    if (x->buf_key)
    {
        temp = x->buf_key;
        _fsl_mem_clear((void*)&x->buf_key, x->memb * x->size_key, name_buf_key, file, line);
        free(x->buf_key);
        _LOGTRACEEX(0,
                file, line, "%s[%p] Memory Unloaded\n", name_buf_key, temp);
    }

    if (x->buf_val)
    {
        temp = x->buf_val;
        _fsl_mem_clear((void*)&x->buf_val, x->memb * x->size_val, name_buf_val, file, line);
        free(x->buf_val);
        _LOGTRACEEX(0,
                file, line, "%s[%p] Memory Unloaded\n", name_buf_val, temp);
    }

    *x = (fsl_key_value){0};
}

u32 _fsl_mem_map_arena(fsl_mem_arena *x, u64 size, const str *name, const str *file, u64 line)
{
    u64 memb_aligned = 0;
    u64 size_aligned = 0;

    if (!x)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Map Memory Arena, Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    fsl_mem_request_page_size();

    if (x->buf)
        return FSL_ERR_SUCCESS;

    if (size == 0)
        size = 1;

    memb_aligned = fsl_align_up_u64(sizeof(void*), FSL_PAGE_SIZE);
    size_aligned = fsl_align_up_u64(size, FSL_PAGE_SIZE);

    if (
            _fsl_mem_map((void*)&x->i, memb_aligned, name, file, line) != FSL_ERR_SUCCESS ||
            _fsl_mem_map((void*)&x->buf, size_aligned, name, file, line) != FSL_ERR_SUCCESS)
    {
        _LOGFATALEX(0, FSL_ERR_MEM_ARENA_MAP_FAIL,
                file, line,
                "%s[%p] Failed to Map Memory Arena, Process Aborted\n", name, x);
        return fsl_err;
    }

    _LOGTRACEEX(0,
            file, line, "%s[%p] Memory Arena Mapped [%"PRIu64"B] Memb [%"PRIu64"B]\n",
            name, x->buf, size_aligned, memb_aligned);

    x->memb = 0;
    x->size_i = memb_aligned;
    x->size_buf = size_aligned;
    x->cursor = 0;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_push_arena(fsl_mem_arena *x, void **p, u64 size, const str *name, const str *file, u64 line)
{
    u64 i = 0, diff = 0;
    u64 memb_aligned = 0;
    u64 size_aligned = 0;
    u64 cursor_aligned = 0;
    u64 cursor_pos = 0;
    u64 cursor_pos_new = 0;
    void *buf_old = NULL;

    if (!p)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Push Memory Arena, Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    if (!x)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Push Memory Arena, Arena Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    if (!x->buf)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Push Memory Arena, Arena Buf Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    if (!x->i)
    {
        _LOGERROREX(0, FSL_ERR_POINTER_NULL,
                file, line,
                "%s[%p] Failed to Push Memory Arena, Arena Memb Pointer NULL\n", name, NULL);
        return fsl_err;
    }

    if (size == 0)
    {
        _LOGERROREX(0, FSL_ERR_SIZE_TOO_SMALL,
                file, line,
                "%s[%p] Failed to Push Memory Arena, Size Too Small\n",
                name, x->buf + x->cursor);
        return fsl_err;
    }

    fsl_mem_request_page_size();
    memb_aligned = fsl_align_up_u64(x->size_i + sizeof(void*), FSL_PAGE_SIZE);
    cursor_aligned = fsl_align_up_u64(x->cursor, FSL_PAGE_SIZE);

    if (size > cursor_aligned - x->cursor)
        cursor_pos = cursor_aligned;
    else
        cursor_pos = x->cursor;

    cursor_pos_new = cursor_pos + size;
    size_aligned = fsl_align_up_u64(cursor_pos_new, FSL_PAGE_SIZE);

    if (size_aligned > x->size_buf)
    {
        buf_old = x->buf;

        if (_fsl_mem_remap((void*)&x->buf, x->size_buf, size_aligned, name, file, line) != FSL_ERR_SUCCESS)
        {
            _LOGERROREX(0, FSL_ERR_SIZE_TOO_SMALL,
                    file, line,
                    "%s[%p] Failed to Push Memory Arena, Memory Remap Failed\n",
                    name, x->buf + cursor_pos);
            return fsl_err;
        }

        diff = x->buf - buf_old;
        i = x->memb;
        while (i--)
            *x->i[i] += diff;
    }

    if (memb_aligned > x->size_i &&
            _fsl_mem_remap((void*)&x->i, x->size_i, memb_aligned, name, file, line) != FSL_ERR_SUCCESS)
    {
        _LOGERROREX(0, FSL_ERR_SIZE_TOO_SMALL,
                file, line,
                "%s[%p] Failed to Push Memory Arena, Memory Remap Failed\n",
                name, x->i + sizeof(void*));
        return fsl_err;
    }

    _LOGTRACEEX(0, file, line,
            "%s[%p] Memory Arena Pushed [%p][%"PRIu64"B] Memb %"PRIu64"[%"PRIu64"B]\n",
            name, x->buf, x->buf + cursor_pos, size_aligned, x->memb, memb_aligned);

    *p = x->buf + cursor_pos;
    x->cursor = cursor_pos_new;
    x->i[x->memb] = &*p;
    x->size_i = memb_aligned;
    x->size_buf = size_aligned;
    ++x->memb;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_clear(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (!x || !*x)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return fsl_err;
    }

    memset(*x, '\0', size);
    _LOGTRACEEX(0, file, line,
            "%s[%p] Memory Cleared [%"PRIu64"B]\n", name, *x, size);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_print_bits(u64 x, u8 bit_count)
{
    while(bit_count--)
        putchar('0' + ((x >> bit_count) & 1));
    putchar('\n');
}

void fsl_swap_bits(char *c1, char *c2)
{
    u8 i = 0;
    for (; i < 8; ++i)
    {
        if (((*c1 >> i) & 1) == ((*c2 >> i) & 1))
            continue;

        *c1 ^= (1 << i);
        *c2 ^= (1 << i);
    }
}

void fsl_swap_bits_u8(u8 *a, u8 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void fsl_swap_bits_u32(u32 *a, u32 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void fsl_swap_bits_u64(u64 *a, u64 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

#if 0
void fsl_sort_buf(fsl_buf *buf) /* TODO: fucking fix this */
{
    for (u16 i = 0, smallest = 0; i < buf->memb - 1 && buf->i[i] != NULL; ++i)
    {
        smallest = i;
        for (u64 j = i + 1; j < buf->memb && buf->i[j] != NULL; ++j)
        {
            char cmp0 = tolower(buf->i[j] + 0);
            char cmp1 = tolower(buf->i[smallest] + 0);
            if (cmp0 < cmp1 && buf->i[j][0] && buf->i[smallest][0])
                smallest = j;

            if (cmp0 == cmp1 && buf->i[j] && cmp1)
                for (u16 k = 1; k < NAME_MAX - 1 && buf->i[j][k - 1] && buf->i[smallest][k - 1]; ++k)
                {
                    if (tolower(buf->i[j] + k) < tolower(buf->i[smallest] + k))
                    {
                        smallest = j;
                        break;
                    }
                }
        }

        swap_strings(buf->i[i], buf->i[smallest]);
    }
}
#endif
