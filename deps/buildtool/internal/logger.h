#ifndef BUILD_LOGGER_H
#define BUILD_LOGGER_H

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

/* ---- section: definitions ------------------------------------------------ */

enum /* log_level */
{
    LOGLEVEL_FATAL,
    LOGLEVEL_ERROR,
    LOGLEVEL_WARNING,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_TRACE,
    LOGLEVEL_COUNT,
}; /* log_level */

/* ---- section: declarations ----------------------------------------------- */

extern u32 log_level_max;

static str log_tag[][16] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "TRACE",
};

static str *esc_code_nocolor = "\033[0m";
static str *esc_code_color[LOGLEVEL_COUNT] =
{
    "\033[31m",
    "\033[91m",
    "\033[95m",
    "\033[0m",
    "\033[0m",
    "\033[33m",
};

/* ---- section: signatures ------------------------------------------------- */

#define LOGFATAL(err, verbose, format, ...) \
    _log_output(err, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_FATAL, format, ##__VA_ARGS__)

#define LOGERROR(err, verbose, format, ...) \
    _log_output(err, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_ERROR, format, ##__VA_ARGS__)

#define LOGWARNING(err, verbose, format, ...) \
    _log_output(err, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_WARNING, format, ##__VA_ARGS__)

#define LOGINFO(verbose, format, ...) \
    _log_output(ERR_SUCCESS, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_INFO, format, ##__VA_ARGS__)

#define LOGDEBUG(verbose, format, ...) \
    _log_output(ERR_SUCCESS, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_DEBUG, format, ##__VA_ARGS__)

#define LOGTRACE(verbose, format, ...) \
    _log_output(ERR_SUCCESS, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_TRACE, format, ##__VA_ARGS__)

#define LOGFATALEX(err, verbose, file, line, format, ...) \
    _log_output(err, verbose, file, line, LOGLEVEL_FATAL, format, ##__VA_ARGS__)

#define LOGERROREX(err, verbose, file, line, format, ...) \
    _log_output(err, verbose, file, line, LOGLEVEL_ERROR, format, ##__VA_ARGS__)

#define LOGWARNINGEX(err, verbose, file, line, format, ...) \
    _log_output(err, verbose, file, line, LOGLEVEL_WARNING, format, ##__VA_ARGS__)

#define LOGINFOEX(verbose, file, line, format, ...) \
    _log_output(ERR_SUCCESS, verbose, file, line, LOGLEVEL_INFO, format, ##__VA_ARGS__)

#define LOGDEBUGEX(verbose, file, line, format, ...) \
    _log_output(ERR_SUCCESS, verbose, file, line, LOGLEVEL_DEBUG, format, ##__VA_ARGS__)

#define LOGTRACEEX(verbose, file, line, format, ...) \
    _log_output(ERR_SUCCESS, verbose, file, line, LOGLEVEL_TRACE, format, ##__VA_ARGS__)

/*! -- INTERNAL USE ONLY --;
 */
extern void _log_output(u32 error_code, b8 verbose, const str *file, u64 line, u8 level, const str *format, ...);

/*! -- INTERNAL USE ONLY --;
 */
extern void _get_log_str(const str *str_in, str *str_out, b8 verbose,
        u8 level, u32 error_code, const str *file, u64 line);

/* ---- section: implementation --------------------------------------------- */

void _log_output(u32 error_code, b8 verbose, const str *file, u64 line, u8 level, const str *format, ...)
{
    __builtin_va_list args;
    str str_in[STRING_MAX] = {0};
    str str_out[OUT_STRING_MAX] = {0};

    build_err = error_code;

    if (level > log_level_max) return;

    va_start(args, format);
    vsnprintf(str_in, STRING_MAX, format, args);
    va_end(args);

    _get_log_str(str_in, str_out, verbose, level, error_code, file, line);
    fprintf(stderr, "%s", str_out);
}

void _get_log_str(const str *str_in, str *str_out, b8 verbose,
        u8 level, u32 error_code, const str *file, u64 line)
{
    str str_time[TIME_STRING_MAX] = {0};
    str str_tag[32] = {0};
    str str_file[STRING_MAX] = {0};
    str *str_color = esc_code_color[level];
    str *str_nocolor = esc_code_nocolor;
    str *trunc = NULL;
    int cursor = 0;

    get_time_str(str_time, "[%F %T] ");

    if (level <= LOGLEVEL_WARNING)
        snprintf(str_tag, 32, "[%s][%"PRIu32"] ", log_tag[level], error_code);
    else
        snprintf(str_tag, 32, "[%s] ", log_tag[level]);

    if (verbose)
        snprintf(str_file, STRING_MAX, "[%s:%"PRIu64"] ", file, line);

    cursor = snprintf(str_out, OUT_STRING_MAX, "%s%s%s%s%s%s",
            str_color, str_time, str_tag, str_file, str_in, str_nocolor);

    if (cursor >= OUT_STRING_MAX - 1)
    {
        trunc = str_out + OUT_STRING_MAX - 4;
        snprintf(trunc, 4, "...");
    }
}

#endif /* BUILD_LOGGER_H */
