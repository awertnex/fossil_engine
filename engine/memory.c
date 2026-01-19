#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <inttypes.h>

#include "h/diagnostics.h"
#include "h/memory.h"
#include "h/limits.h"
#include "h/logger.h"

u32 _mem_alloc(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (*x)
    {
        engine_err = ERR_POINTER_NOT_NULL;
        return engine_err;
    }

    *x = calloc(1, size);
    if (!x || !*x)
    {
        _LOGFATALEX(TRUE, file, line, ERR_MEM_ALLOC_FAIL,
                "%s[%p] Failed to Allocate Memory, Process Aborted\n", name, NULL);
        return engine_err;
    }
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Allocated [%"PRIu64"B]\n", name, *x, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_alloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    if (*x)
        return ERR_SUCCESS;

    *x = calloc(memb, size);
    if (!x || !*x)
    {
        _LOGFATALEX(TRUE, file, line, ERR_MEM_ALLOC_FAIL,
                "%s[%p] Failed to Allocate Memory, Process Aborted\n", name, NULL);
        return engine_err;
    }
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Allocated [%"PRIu64"B]\n", name, *x, memb * size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_alloc_buf(Buf *x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    str name_i[NAME_MAX] = {0};
    str name_buf[NAME_MAX] = {0};
    u64 i = 0;

    if (!x)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Allocate Memory, Pointer NULL\n", name, NULL);
        return engine_err;
    }

    snprintf(name_i, NAME_MAX, "%s.i", name);
    snprintf(name_buf, NAME_MAX, "%s.buf", name);

    if (_mem_alloc_memb((void*)&x->i,
                memb, sizeof(str*), name_i, file, line) != ERR_SUCCESS)
        return engine_err;

    if (_mem_alloc_memb((void*)&x->buf, memb, size, name_buf, file, line) != ERR_SUCCESS)
    {
        _mem_free((void*)&x->i, memb * sizeof(str*), name_i, file, line);
        return engine_err;
    }

    for (i = 0; i < memb; ++i)
        x->i[i] = x->buf + i * size;
    x->memb = memb;
    x->size = size;
    x->loaded = TRUE;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_alloc_key_val(KeyValue *x, u64 memb, u64 size_key, u64 size_val,
        const str *name, const str *file, u64 line)
{
    str name_key[NAME_MAX] = {0};
    str name_val[NAME_MAX] = {0};
    str name_buf_key[NAME_MAX] = {0};
    str name_buf_val[NAME_MAX] = {0};
    u64 i = 0;

    if (!x)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Allocate Memory, Pointer NULL\n", name, NULL);
        return engine_err;
    }

    snprintf(name_key, NAME_MAX, "%s.key", name);
    snprintf(name_val, NAME_MAX, "%s.val", name);
    snprintf(name_buf_key, NAME_MAX, "%s.buf_key", name);
    snprintf(name_buf_val, NAME_MAX, "%s.buf_val", name);

    if (
            _mem_alloc_memb((void*)&x->key,
                memb, sizeof(str*), name_key, file, line) != ERR_SUCCESS ||
            _mem_alloc_memb((void*)&x->val,
                memb, sizeof(str*), name_val, file, line) != ERR_SUCCESS ||
            _mem_alloc_memb((void*)&x->buf_key,
                memb, size_key, name_buf_key, file, line) != ERR_SUCCESS ||
            _mem_alloc_memb((void*)&x->buf_val,
                memb, size_val, name_buf_val, file, line) != ERR_SUCCESS)
    {
        _mem_free_key_val(x, name, file, line);
        return engine_err;
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

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_realloc(void **x, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;

    if (!x || !*x)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Reallocate Memory, Pointer NULL\n", name, NULL);
        return engine_err;
    }

    temp = realloc(*x, size);
    if (!temp)
    {
        _LOGFATALEX(TRUE, file, line, ERR_MEM_REALLOC_FAIL,
                "%s[%p] Failed to Reallocate Memory, Process Aborted\n", name, *x);
        return engine_err;
    }

    *x = temp;
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Reallocated [%"PRIu64"B]\n", name, *x, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_realloc_memb(void **x, u64 memb, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;

    if (!x || !*x)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Reallocate Memory, Pointer NULL\n", name, NULL);
        return engine_err;
    }

    temp = realloc(*x, memb * size);
    if (!temp)
    {
        _LOGFATALEX(TRUE, file, line, ERR_MEM_REALLOC_FAIL,
                "%s[%p] Failed to Reallocate Memory, Process Aborted\n", name, *x);
        return engine_err;
    }

    *x = temp;
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Reallocated [%"PRIu64"B]\n", name, *x, memb * size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void _mem_free(void **x, u64 size, const str *name, const str *file, u64 line)
{
    void *temp = NULL;
    if (!x || !*x)
        return;

    temp = *x;
    mem_clear(x, size, name);
    free(*x);
    *x = NULL;
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name, temp);
}

void _mem_free_buf(Buf *x, const str *name, const str *file, u64 line)
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
        mem_clear((void*)&x->i, x->memb * sizeof(str*), name_i);
        free(x->i);
        _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name_i, temp);
    }

    if (x->buf)
    {
        temp = x->buf;
        mem_clear((void*)&x->buf, x->memb * x->size, name_buf);
        free(x->buf);
        _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name_buf, temp);
    }

    *x = (Buf){0};
}

void _mem_free_key_val(KeyValue *x, const str *name, const str *file, u64 line)
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
        mem_clear((void*)&x->key, x->memb * sizeof(str*), name_key);
        free(x->key);
        _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name_key, temp);
    }

    if (x->val)
    {
        temp = x->val;
        mem_clear((void*)&x->val, x->memb * sizeof(str*), name_val);
        free(x->val);
        _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name_val, temp);
    }

    if (x->buf_key)
    {
        temp = x->buf_key;
        mem_clear((void*)&x->buf_key, x->memb * x->size_key, name_buf_key);
        free(x->buf_key);
        _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name_buf_key, temp);
    }

    if (x->buf_val)
    {
        temp = x->buf_val;
        mem_clear((void*)&x->buf_val, x->memb * x->size_val, name_buf_val);
        free(x->buf_val);
        _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Unloaded\n", name_buf_val, temp);
    }

    *x = (KeyValue){0};
}

u32 _mem_map_arena(MemArena* x, u64 size, const str *name, const str *file, u64 line)
{
    MemArena *temp = NULL;

    if (!x)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Map Memory Arena, Pointer NULL\n", name, NULL);
        engine_err = ERR_POINTER_NULL;
        return engine_err;
    }

    temp = x;
    if (_mem_map((void*)&x->buf, size, name, file, line) != ERR_SUCCESS)
    {
        _LOGFATALEX(TRUE, file, line, ERR_MEM_ARENA_MAP_FAIL,
                "%s[%p] Failed to Map Memory Arena, Process Aborted\n", name, temp);
        return engine_err;
    }
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Arena Mapped [%"PRIu64"B]\n", name, x->buf, size);

    x->size = size;
    x->cursor = 0;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_push_arena(MemArena *x, void **p, u64 size, const str *name, const str *file, u64 line)
{
    if (!p)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Push Memory Arena, Pointer NULL\n", name, NULL);
        return engine_err;
    }

    if (!x)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Push Memory Arena, Arena Pointer NULL\n", name, NULL);
        return engine_err;
    }

    if (!x->buf)
    {
        _LOGERROREX(TRUE, file, line, ERR_POINTER_NULL,
                "%s[%p] Failed to Push Memory Arena, Arena Buf Pointer NULL\n", name, NULL);
        return engine_err;
    }

    if (size == 0)
    {
        _LOGERROREX(TRUE, file, line, ERR_SIZE_TOO_SMALL,
                "%s[%p] Failed to Push Memory Arena, Size Too Small\n",
                name, x->buf + x->cursor);
        return engine_err;
    }

    if (size > x->size - x->cursor)
    {
        _LOGERROREX(TRUE, file, line, ERR_SIZE_LIMIT,
                "%s[%p] Failed to Push Memory Arena, Size Limit Exceeded By [%"PRIu64"B]\n",
                name, x->buf + x->cursor, x->cursor - x->size + size);
        return engine_err;
    }

    *p = x->buf + x->cursor;
    x->cursor += size;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 _mem_clear(void **x, u64 size, const str *name, const str *file, u64 line)
{
    if (!x || !*x)
    {
        engine_err = ERR_POINTER_NULL;
        return engine_err;
    }

    bzero(*x, size);
    _LOGTRACEEX(TRUE, file, line, "%s[%p] Memory Cleared [%"PRIu64"B]\n", name, *x, size);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void print_bits(u64 x, u8 bit_count)
{
    while(bit_count--)
        putchar('0' + ((x >> bit_count) & 1));
    putchar('\n');
}

void swap_bits(char *c1, char *c2)
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

void swap_bits_u8(u8 *a, u8 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void swap_bits_u32(u32 *a, u32 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

void swap_bits_u64(u64 *a, u64 *b)
{
    *a ^= *b;
    *b ^= *a;
    *a ^= *b;
}

#if 0
void sort_buf(Buf *buf) /* TODO: fucking fix this */
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
