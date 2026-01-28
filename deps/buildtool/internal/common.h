#ifndef BUILDTOOL_COMMON_H
#define BUILDTOOL_COMMON_H

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

#if defined(__linux__) || defined(__linux)
#   define _GNU_SOURCE
#   include <linux/limits.h>
#   include <time.h>
#   include <sys/time.h>

#   define PLATFORM_LINUX 1
#   define PLATFORM "linux"
#   define EXE ""
#   define RUNTIME_PATH "$ORIGIN"

#   define SLASH_NATIVE '/'
#   define SLASH_NON_NATIVE '\\'
#elif defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
#   define NAME_MAX 255
#   include <limits.h>

#   define PLATFORM_WIN 1
#   define PLATFORM "win"
#   define EXE ".exe"
#   define RUNTIME_PATH "%CD%"

#   define SLASH_NATIVE '\\'
#   define SLASH_NON_NATIVE '/'
#endif /* PLATFORM */

#if defined(__STDC_VERSION__)
#   define STD 0
#elif defined(__STDC__)
#   define STD 89
#endif /* STD */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <inttypes.h>

/* ---- section: types ------------------------------------------------------ */

#ifndef TRUE
#   define TRUE     1
#endif /* TRUE */

#ifndef FALSE
#   define FALSE    0
#endif /* FALSE */

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef float       f32;
typedef double      f64;

typedef char        str;

typedef uint8_t     b8;
typedef uint32_t    b32;

typedef struct _buf
{
    b8 loaded;
    void **i;       /* members of `buf` */
    void *buf;      /* raw data */
    u64 memb;       /* number of `i` members */
    u64 size;       /* size of each member in bytes */
    u64 cursor;     /* for iteration, optional */
} _buf;

/* ---- section: definitions ------------------------------------------------ */

#define STRING_MAX      1024
#define OUT_STRING_MAX  (STRING_MAX + 256)
#define TIME_STRING_MAX 256
#define STRINGF_BUFFERS_MAX 4

/* ---- section: signatures ------------------------------------------------- */

/*! @brief write temporary formatted string.
 *
 *  @remark use temporary static buffers internally.
 *  @remark inspired by Raylib: `github.com/raysan5/raylib`: `raylib/src/rtext.c/TextFormat()`.
 *
 *  @return static formatted string.
 */
extern str *stringf(const str *format, ...);

/*! @brief compare `arg` to any of `argv` entries.
 *
 *  @return `argc` of match if found, 0 otherwise.
 */
extern u64 find_token(str *arg, int argc, str **argv);

/*! @brief get time string with max length of @ref TIME_STRING_MAX as per `format`
 *  and load into `dst`.
 */
extern void get_time_str(str *dst, const str *format);

/* ---- section: implementation --------------------------------------------- */

str *stringf(const str *format, ...)
{
    static str buf[STRINGF_BUFFERS_MAX][OUT_STRING_MAX] = {0};
    static u64 index = 0;
    str *string = buf[index];
    str *trunc = NULL;
    int cursor = 0;
    va_list args;

    va_start(args, format);
    cursor = vsnprintf(string, OUT_STRING_MAX, format, args);
    va_end(args);

    if (cursor >= OUT_STRING_MAX - 1)
    {
        trunc = string + OUT_STRING_MAX - 4;
        snprintf(trunc, 4, "...");
    }

    index = (index + 1) % STRINGF_BUFFERS_MAX;
    return string;
}

u64 find_token(str *arg, int argc, char **argv)
{
    u32 i = 0;
    if (!argc || !argv || !argv[0])
        return 0;
    for (; (int)i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

void get_time_str(str *dst, const str *format)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *_tm = localtime(&ts.tv_sec);
    strftime(dst, TIME_STRING_MAX, format, _tm);
}

#endif /* BUILDTOOL_COMMON_H */
