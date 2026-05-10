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

#include "../h/common.h"

#include "../h/diagnostics.h"
#include "../h/math.h"
#include "memory.h"
#include "../h/limits.h"
#include "../logger/log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <inttypes.h>

fsl_mem_arena mem_arena_internal = {0};
fsl_mem_arena mem_arena_name_internal = {0};
fsl_mem_arena mem_arena_name_id_internal = {0};
fsl_mem_arena mem_arena_file_internal = {0};
fsl_mem_arena mem_arena_path_internal = {0};

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
        LOGERROREX(FSL_ERR_MEM_ALLOC_FAIL, 0,
                file, line,
                MSG_MEM_ALLOC_POINTER_NULL_FAIL(name));
        return fsl_err;
    }
    LOGTRACEEX(0,
            file, line,
            MSG_MEM_ALLOC(name, *x, size));

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
        LOGERROREX(FSL_ERR_MEM_ALLOC_FAIL, 0,
                file, line,
                MSG_MEM_ALLOC_POINTER_NULL_FAIL(name));
        return fsl_err;
    }
    LOGTRACEEX(0,
            file, line,
            MSG_MEM_ALLOC(name, *x, memb * size));

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
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_ALLOC_POINTER_NULL_FAIL(name));
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
        x->i[i] = (u8*)x->buf + i * size;

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
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_ALLOC_POINTER_NULL_FAIL(name));
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
        x->key[i] = (u8*)x->buf_key + i * size_key;
        x->val[i] = (u8*)x->buf_val + i * size_val;
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
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_REALLOC_POINTER_NULL_FAIL(name));
        return fsl_err;
    }

    temp = realloc(*x, size);
    if (!temp)
    {
        LOGERROREX(FSL_ERR_MEM_REALLOC_FAIL, 0,
                file, line,
                MSG_MEM_REALLOC_FAIL(name, *x));
        return fsl_err;
    }

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_REALLOC(name, *x, temp, size));
    *x = temp;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_realloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;

    if (!x || !*x)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_REALLOC_POINTER_NULL_FAIL(name));
        return fsl_err;
    }

    temp = realloc(*x, memb * size);
    if (!temp)
    {
        LOGERROREX(FSL_ERR_MEM_REALLOC_FAIL, 0,
                file, line,
                MSG_MEM_REALLOC_FAIL(name, *x));
        return fsl_err;
    }

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_REALLOC(name, *x, temp, size));
    *x = temp;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void _fsl_mem_free(void **x, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;

    if (!x || !*x || !size)
        return;

    temp = *x;
    if (size)
        _fsl_mem_clear(*x, size, name, file, line);
    free(*x);
    *x = NULL;
    LOGTRACEEX(0,
            file, line,
            MSG_MEM_FREE(name, temp, size));
}

void _fsl_mem_free_buf(fsl_buf *x, const str *name, const str *file, u64 line)
{
    fsl_buf nobuf = {0};
    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    void *temp = NULL;

    if (!x) return;

    snprintf(name_i, NAME_MAX, "%s.i", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (x->i)
    {
        temp = x->i;
        _fsl_mem_clear(x->i, x->memb * sizeof(str*), name_i, file, line);
        free(x->i);
        LOGTRACEEX(0,
                file, line,
                MSG_MEM_FREE(name_i, temp, x->memb * sizeof(str*)));
    }

    if (x->buf)
    {
        temp = x->buf;
        _fsl_mem_clear(x->buf, x->memb * x->size, name_buf, file, line);
        free(x->buf);
        LOGTRACEEX(0,
                file, line,
                MSG_MEM_FREE(name_buf, temp, x->memb * x->size));
    }

    *x = nobuf;
}

void _fsl_mem_free_key_val(fsl_key_value *x, const str *name, const str *file, u64 line)
{
    fsl_key_value nokey_value = {0};
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
        _fsl_mem_clear(x->key, x->memb * sizeof(str*), name_key, file, line);
        free(x->key);
        LOGTRACEEX(0,
                file, line,
                MSG_MEM_FREE(name_key, temp, x->memb * sizeof(str*)));
    }

    if (x->val)
    {
        temp = x->val;
        _fsl_mem_clear(x->val, x->memb * sizeof(str*), name_val, file, line);
        free(x->val);
        LOGTRACEEX(0,
                file, line,
                MSG_MEM_FREE(name_key, temp, x->memb * sizeof(str*)));
    }

    if (x->buf_key)
    {
        temp = x->buf_key;
        _fsl_mem_clear(x->buf_key, x->memb * x->size_key, name_buf_key, file, line);
        free(x->buf_key);
        LOGTRACEEX(0,
                file, line,
                MSG_MEM_FREE(name_buf_key, temp, x->memb * x->size_key));
    }

    if (x->buf_val)
    {
        temp = x->buf_val;
        _fsl_mem_clear(x->buf_val, x->memb * x->size_val, name_buf_val, file, line);
        free(x->buf_val);
        LOGTRACEEX(0,
                file, line,
                MSG_MEM_FREE(name_buf_val, temp, x->memb * x->size_val));
    }

    *x = nokey_value;
}

u32 _fsl_mem_arena_init(fsl_mem_arena *x, const str *name, const str *file, u64 line)
{
    u64 entry_cap = sizeof(fsl_mem_arena_handle) * 2;
    u64 freelist_cap = sizeof(fsl_mem_arena_handle) * 2;
    u64 buf_cap = 2;

    if (!x)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_ARENA_INIT_POINTER_NULL_FAIL(name, 2));
        return fsl_err;
    }

    if (x->buf)
        return FSL_ERR_SUCCESS;

    if (
            _fsl_mem_map((void*)&x->entry, entry_cap, name, file, line) != FSL_ERR_SUCCESS ||
            _fsl_mem_map((void*)&x->freelist, freelist_cap, name, file, line) != FSL_ERR_SUCCESS ||
            _fsl_mem_map((void*)&x->buf, buf_cap, name, file, line) != FSL_ERR_SUCCESS)
    {
        LOGERROREX(FSL_ERR_MEM_ARENA_MAP_FAIL, 0,
                file, line,
                MSG_MEM_ARENA_INIT_FAIL(name, x, buf_cap));
        return fsl_err;
    }

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_ARENA_INIT(name, x->buf, buf_cap));

    x->entry[0].offset = FSL_OFFSET_INVALID;
    x->entry[1].offset = FSL_OFFSET_INVALID;
    x->entry_cap = entry_cap;
    x->entry_count = 0;
    x->freelist_cap = freelist_cap;
    x->buf_cap = buf_cap;
    x->buf_cursor = 0;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_arena_push(fsl_mem_arena *x, fsl_mem_handle *handle, u64 size, const str *name, const str *file, u64 line)
{
    u64 i = 0;
    u64 buf_cap_old = 0;
    fsl_off entry_pos = 0;
    u64 entry_cap = 0;

    if (!handle)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_ARENA_PUSH_REASON_FAIL(name, NULL, size, "Pointer `NULL`"));
        return fsl_err;
    }

    if (!x)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_ARENA_PUSH_REASON_FAIL(name, NULL, size, "Arena Pointer `NULL`"));
        return fsl_err;
    }

    if (!x->buf)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_ARENA_PUSH_REASON_FAIL(name, NULL, size, "Arena Buf Pointer `NULL`"));
        return fsl_err;
    }

    if (!x->entry)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_ARENA_PUSH_REASON_FAIL(name, x->buf, size, "Arena Entry Pointer `NULL`"));
        return fsl_err;
    }

    if (!x->freelist)
    {
        LOGERROREX(FSL_ERR_POINTER_NULL, 0,
                file, line,
                MSG_MEM_ARENA_PUSH_REASON_FAIL(name, x->buf, size, "Arena Freelist Pointer `NULL`"));
        return fsl_err;
    }

    if (size == 0)
    {
        LOGERROREX(FSL_ERR_SIZE_TOO_SMALL, 0,
                file, line,
                MSG_MEM_ARENA_PUSH_REASON_FAIL(name, x->buf, size, "Size Too Small"));
        return fsl_err;
    }

    /* expand arena if needed */

    if (size > x->buf_cap - x->buf_cursor)
    {
        buf_cap_old = x->buf_cap;
        x->buf_cap = (x->buf_cap + size) * 2;
        if (_fsl_mem_remap((void*)&x->buf, buf_cap_old, x->buf_cap, name, file, line) != FSL_ERR_SUCCESS)
        {
            LOGERROREX(fsl_err, 0,
                    file, line,
                    MSG_MEM_ARENA_PUSH_REASON_FAIL(name, (u8*)x->buf + x->buf_cursor, x->buf_cap, "`_fsl_mem_remap()` Failed"));
            return fsl_err;
        }
    }

    entry_cap = fsl_arr_len(x->entry);
    while (x->entry[i].offset != FSL_OFFSET_INVALID && i < entry_cap)
    {++i;}
    entry_pos = i;

    /* expand members array if needed */

    if (entry_pos >= entry_cap)
    {
        if (_fsl_mem_remap((void*)&x->entry, x->entry_cap, x->entry_cap * 2, name, file, line) != FSL_ERR_SUCCESS)
        {
            LOGERROREX(fsl_err, 0,
                    file, line,
                    MSG_MEM_ARENA_PUSH_REASON_FAIL(name, x->entry, x->entry_cap, "`_fsl_mem_remap()` Failed"));
            return fsl_err;
        }
        x->entry_cap *= 2;
        entry_cap = fsl_arr_len(x->entry);
        for (; i < entry_cap; ++i)
            x->entry[i].offset = FSL_OFFSET_INVALID;
    }

    /* assign new parameters */

    handle->arena = x;
    handle->offset = x->buf_cursor;
    handle->size = size;
    x->entry[entry_pos].offset = x->buf_cursor;
    x->entry[entry_pos].size = size;
    x->buf_cursor += size;
    ++x->entry_count;

    LOGTRACEEX(0,
            file, line,
            MSG_MEM_ARENA_PUSH(name, x->buf, handle->offset, size, x->entry_count, x->entry_cap));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 _fsl_mem_clear(void *x, u64 size, const str *name, const str *file, u64 line)
{
    if (!x)
    {
        fsl_err = FSL_ERR_POINTER_NULL;
        return fsl_err;
    }

    memset(x, '\0', size);
    LOGTRACEEX(0,
            file, line,
            MSG_MEM_CLEAR(name, x, size));

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
