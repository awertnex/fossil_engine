/*!
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
 *  limitations under the License.
 */

/*!
 *  @file string.c
 *
 *  @brief string parsing, token searching.
 */

#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../logger/logger.h"
#include "../logger/logger_messages_internal.h"
#include "../memory/memory.h"

#include "string.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

void fsl_swap_strings(str *s1, str *s2)
{
    u16 len = (strlen(s1) > strlen(s2)) ? strlen(s1) : strlen(s2);
    u16 i = 0;
    for (; i <= len; ++i)
        fsl_swap_bits(&s1[i], &s2[i]);
}

void fsl_swap_string_char(str *string, char c1, char c2)
{
    u64 i, len = strlen(string);
    if (!len) return;

    for (i = 0; i < len; ++i)
    {
        if (string[i] == c1)
            string[i] = c2;
    }
}

str *fsl_stringf(const str *format, ...)
{
    static str buf[FSL_STRINGF_BUFFERS_MAX][FSL_OUT_STRING_MAX] = {0};
    static u64 index = 0;
    str *string = buf[index];
    str *trunc = NULL;
    int cursor = 0;
    __builtin_va_list args;

    va_start(args, format);
    cursor = vsnprintf(string, FSL_OUT_STRING_MAX, format, args);
    va_end(args);

    if (cursor >= FSL_OUT_STRING_MAX - 1)
    {
        trunc = string + FSL_OUT_STRING_MAX - 4;
        snprintf(trunc, 4, "...");
    }

    index = (index + 1) % FSL_STRINGF_BUFFERS_MAX;
    return string;
}

str *stringf_internal(const str *format, ...)
{
    static str buf[FSL_STRINGF_BUFFERS_MAX][FSL_OUT_STRING_MAX] = {0};
    static u64 index = 0;
    str *string = buf[index];
    str *trunc = NULL;
    int cursor = 0;
    __builtin_va_list args;

    va_start(args, format);
    cursor = vsnprintf(string, FSL_OUT_STRING_MAX, format, args);
    va_end(args);

    if (cursor >= FSL_OUT_STRING_MAX - 1)
    {
        trunc = string + FSL_OUT_STRING_MAX - 4;
        snprintf(trunc, 4, "...");
    }

    index = (index + 1) % FSL_STRINGF_BUFFERS_MAX;
    return string;
}

void fsl_skip_spaces(str **string)
{
    while (**string == ' ' || **string == '\t' || **string == '\r')
        ++*string;
}

void fsl_strip_non_printable(str *string)
{
    u64 i = 0;
    u64 j = 0;
    while (string[i])
    {
        if (string[i] >= ' ' && string[i] < 127)
        {
            string[j] = string[i];
            ++j;
        }
        ++i;
    }

    if (j < i)
        string[j] = 0;
}

u64 fsl_find_token(str *arg, int argc, char **argv)
{
    u32 i = 0;
    for (; (int)i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

b8 fsl_is_digit(char n)
{
    return n >= '0' && n <= '9' ? TRUE : FALSE;
}

i32 fsl_convert_char_to_int(char n)
{
    return (i32)n - '0';
}

u32 fsl_convert_str_to_u32(const str *n, u32 *dst)
{
    i32 i = 0;
    i32 len = strlen(n);
    i32 result = 0;
    b8 is_negative = FALSE;

    if (n[0] == '-')
    {
        ++i;
        is_negative = TRUE;
    }

    for (; i < len; ++i)
    {
        if (fsl_is_digit(n[i]))
            result = result * 10 + fsl_convert_char_to_int(n[i]);
        else break;
    }

    if (is_negative)
        result = -result;

    *dst = (u32)result;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_convert_str_to_u64(const str *n, u64 *dst)
{
    i32 i = 0;
    i32 len = strlen(n);
    i64 result = 0;
    b8 is_negative = FALSE;

    if (n[0] == '-')
    {
        ++i;
        is_negative = TRUE;
    }

    for (; i < len; ++i)
    {
        if (fsl_is_digit(n[i]))
            result = result * 10 + fsl_convert_char_to_int(n[i]);
        else break;
    }

    if (is_negative)
        result = -result;

    *dst = (u64)result;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_convert_str_to_i64(const str *n, i64 *dst)
{
    i32 i = 0;
    i32 len = strlen(n);
    i64 result = 0;
    b8 is_negative = FALSE;

    if (n[0] == '-')
    {
        ++i;
        is_negative = TRUE;
    }

    for (; i < len; ++i)
    {
        if (fsl_is_digit(n[i]))
            result = result * 10 + fsl_convert_char_to_int(n[i]);
        else break;
    }

    if (is_negative)
        result = -result;

    *dst = result;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_convert_str_to_f32(const str *n, f32 *dst)
{
    i32 i = 0;
    i32 len = strlen(n);
    f64 result = 0.0;
    f64 divisor = 1.0;
    b8 is_decimal = FALSE;
    b8 is_negative = FALSE;

    if (n[0] == '-')
    {
        ++i;
        is_negative = TRUE;
    }

    for (; i < len; ++i)
    {
        if (n[i] == '.')
        {
            is_decimal = TRUE;
            continue;
        }

        if (fsl_is_digit(n[i]))
            result = result * 10.0 + fsl_convert_char_to_int(n[i]);

        if (is_decimal)
            divisor *= 10.0;
    }

    result /= divisor;
    if (is_negative)
        result = -result;

    *dst = (f32)result;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_convert_str_to_f64(const str *n, f64 *dst)
{
    i32 i = 0;
    i32 len = strlen(n);
    f64 result = 0.0;
    f64 divisor = 1.0;
    b8 is_decimal = FALSE;
    b8 is_negative = FALSE;

    if (n[0] == '-')
    {
        ++i;
        is_negative = TRUE;
    }

    for (; i < len; ++i)
    {
        if (n[i] == '.')
        {
            is_decimal = TRUE;
            continue;
        }

        if (fsl_is_digit(n[i]))
            result = result * 10.0 + fsl_convert_char_to_int(n[i]);

        if (is_decimal)
            divisor *= 10.0;
    }

    result /= divisor;
    if (is_negative)
        result = -result;

    *dst = result;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_convert_i32_to_str(str *dst, i32 size, i32 n)
{
    i32 i = 1, j = 0, len = 0, sign = n;

    if (size <= 0)
    {
        LOGERROR(FSL_ERR_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Convert `i32` to `str`", "Size Too Small"));
        return fsl_err;
    }

    if (n == 0)
    {
        dst[0] = '0';
        dst[1] = '\0';
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }

    (n < 0) ? ++len : 0;
    n = abs(n);
    for (; i < n + 1 && len < size - 1; i *= 10)
        ++len;

    i = 0;
    while (n > 0 && i < size - 1)
    {
        dst[i++] = (n % 10) + '0';
        n /= 10;
    }

    if (sign < 0)
        dst[i++] = '-';
    dst[i] = '\0';

    for (j = i - 1, i = 0; i < j; i++, j--)
        fsl_swap_bits(&dst[i], &dst[j]);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_convert_u64_to_str(str *dst, u64 size, u64 n)
{
    u64 i = 1, j = 0, len = 0;

    if (size == 0)
    {
        LOGERROR(FSL_ERR_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Convert `u64` to `str`", "Size Too Small"));
        return fsl_err;
    }

    if (n == 0)
    {
        dst[0] = '0';
        dst[1] = '\0';
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }

    for (; i < n + 1 && len < size - 1; i *= 10)
        ++len;

    i = 0;
    while (n > 0 && i < size - 1)
    {
        dst[i++] = (n % 10) + '0';
        n /= 10;
    }

    dst[i] = '\0';

    for (j = i - 1, i = 0; i < j; i++, j--)
        fsl_swap_bits(&dst[i], &dst[j]);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u64 fsl_hash_djb2_u64(const void *data, fsl_cap len)
{
    u8* d = (u8*)data;
    u64 i = 0;
    u64 h = 5381;
    for (; (len && i < len) || *d; ++i, ++d)
        h = ((h << 5) + h) + *d;
    return h;
}

u64 fsl_hash_fnv1a_u64(const void *data, fsl_cap len)
{
    u8 *d = (u8*)data;
    u64 i = 0;
    u64 h = 2166136261;
    for (i = 0; (len && i < len) || *d; ++i, ++d)
    {
        h ^= *d;
        h *= 16777619;
    }
    return h;
}

b8 fsl_find_hash_u64(u64 hash, u64 *buf, u64 *dst, fsl_len len)
{
    u64 i = 0;
    while (i < len)
    {
        if (hash == *buf)
        {
            *dst = i;
            return TRUE;
        }
        ++i;
        ++buf;
    }
    return FALSE;
}
