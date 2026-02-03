/*  @file string.c
 *
 *  @brief string parsing, token searching.
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

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/string.h"

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
    va_list args;

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

u64 fsl_find_token(str *arg, int argc, char **argv)
{
    u32 i = 0;
    for (; (int)i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

u32 fsl_convert_i32_to_str(str *dst, i32 size, i32 n)
{
    i32 i = 1, j = 0, len = 0, sign = n;

    if (size <= 0)
    {
        _LOGERROR(FSL_ERR_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Convert i32 to str, 'size' Too Small");
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
        _LOGERROR(FSL_ERR_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Convert u64 to str, 'size' Too Small");
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
