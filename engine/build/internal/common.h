#ifndef BUILD_COMMON_H
#define BUILD_COMMON_H

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

#ifdef __STDC_VERSION__
#   if (__STDC_VERSION__ == 199901)
#       define STD __STDC_VERSION__
#   else
#       define STD 0
#   endif
#else
#   define STD 0
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

#define TRUE        1
#define FALSE       0

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

#define MSEC2SEC 1e-3
#define NSEC2SEC 1e-9
#define SEC2USEC 1e6

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
    for (; (int)i < argc; ++i)
        if (!strncmp(argv[i], arg, strlen(arg) + 1))
            return i;
    return 0;
}

void get_time_str(str *dst, const str *format)
{
    u64 _time_nsec = 0;
    time_t _time = 0;
    struct timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    _time_nsec = (u64)(ts.tv_sec * SEC2USEC + ts.tv_nsec * MSEC2SEC);
    _time = _time_nsec * NSEC2SEC;

    struct tm *_tm = localtime(&_time);
    strftime(dst, TIME_STRING_MAX, format, _tm);
}

#endif /* BUILD_COMMON_H */
