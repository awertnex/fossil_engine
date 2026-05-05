#ifndef BUILDTOOL_LOGGER_H
#define BUILDTOOL_LOGGER_H

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

enum log_level
{
    LOGLEVEL_FATAL,
    LOGLEVEL_ERROR,
    LOGLEVEL_WARNING,
    LOGLEVEL_SUCCESS,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_TRACE,
    LOGLEVEL_COUNT
}; /* log_level */

#define LOGFATAL(err, verbose, message) \
    _log_output(err, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_FATAL, message)

#define LOGERROR(err, verbose, message) \
    _log_output(err, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_ERROR, message)

#define LOGWARNING(err, verbose, message) \
    _log_output(err, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_WARNING, message)

#define LOGSUCCESS(verbose, message) \
    _log_output(ERR_SUCCESS, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_SUCCESS, message)

#define LOGINFO(verbose, message) \
    _log_output(ERR_SUCCESS, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_INFO, message)

#define LOGDEBUG(verbose, message) \
    _log_output(ERR_SUCCESS, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_DEBUG, message)

#define LOGTRACE(verbose, message) \
    _log_output(ERR_SUCCESS, verbose, __BASE_FILE__, __LINE__, LOGLEVEL_TRACE, message)

#define LOGFATALEX(err, verbose, file, line, message) \
    _log_output(err, verbose, file, line, LOGLEVEL_FATAL, message)

#define LOGERROREX(err, verbose, file, line, message) \
    _log_output(err, verbose, file, line, LOGLEVEL_ERROR, message)

#define LOGWARNINGEX(err, verbose, file, line, message) \
    _log_output(err, verbose, file, line, LOGLEVEL_WARNING, message)

#define LOGSUCCESSEX(verbose, file, line, message) \
    _log_output(ERR_SUCCESS, verbose, file, line, LOGLEVEL_SUCCESS, message)

#define LOGINFOEX(verbose, file, line, message) \
    _log_output(ERR_SUCCESS, verbose, file, line, LOGLEVEL_INFO, message)

#define LOGDEBUGEX(verbose, file, line, message) \
    _log_output(ERR_SUCCESS, verbose, file, line, LOGLEVEL_DEBUG, message)

#define LOGTRACEEX(verbose, file, line, message) \
    _log_output(ERR_SUCCESS, verbose, file, line, LOGLEVEL_TRACE, message)

/* ---- section: declarations ----------------------------------------------- */

extern u32 log_level_max;

static str log_tag[][16] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "SUCCESS",
    "INFO",
    "DEBUG",
    "TRACE"
};

static str *esc_code_nocolor = "\033[0m";
static str *esc_code_color[LOGLEVEL_COUNT] =
{
    "\033[31m",
    "\033[91m",
    "\033[95m",
    "\033[32m",
    "\033[0m",
    "\033[0m",
    "\033[33m"
};

/* ---- section: signatures ------------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 */
extern void _log_output(u32 error_code, b8 verbose, const str *file, u64 line, u8 level, const str *message);

/*! -- INTERNAL USE ONLY --;
 */
extern void _get_log_str(const str *str_in, str *str_out, b8 verbose,
        u8 level, u32 error_code, const str *file, u64 line);

extern str *logger_stringf(const str *format, ...);

/* ---- section: implementation --------------------------------------------- */

void _log_output(u32 error_code, b8 verbose, const str *file, u64 line, u8 level, const str *message)
{
    str str_in[STRING_MAX] = {0};
    str str_out[OUT_STRING_MAX] = {0};

    build_err = error_code;

    if (level > log_level_max) return;

    snprintf(str_in, STRING_MAX, "%s", message);
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
    b8 is_error = level <= LOGLEVEL_WARNING;

    get_time_str(str_time, "[%F %T] ");

    if (is_error)
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

str *logger_stringf(const str *format, ...)
{
    static str buf[STRINGF_BUFFERS_MAX][STRING_MAX] = {0};
    static u64 index = 0;
    str *string = buf[index];
    __builtin_va_list args;

    va_start(args, format);
    vsnprintf(string, OUT_STRING_MAX, format, args);
    va_end(args);

    index = (index + 1) % STRINGF_BUFFERS_MAX;
    return string;
}

#endif /* BUILDTOOL_LOGGER_H */
