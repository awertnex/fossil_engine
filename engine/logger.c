#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/string.h"
#include "h/time.h"
#include "h/types.h"

u32 log_level_max = LOGLEVEL_TRACE;
str log_dir[PATH_MAX] = {0};
static str *logger_buf = NULL;
static const str LOG_FILE_NAME[LOGLEVEL_COUNT][NAME_MAX] =
{
    [LOGLEVEL_FATAL] = LOGFILE_NAME_ERROR,
    [LOGLEVEL_ERROR] = LOGFILE_NAME_ERROR,
    [LOGLEVEL_WARNING] = LOGFILE_NAME_ERROR,
    [LOGLEVEL_INFO] = LOGFILE_NAME_INFO,
    [LOGLEVEL_DEBUG] = LOGFILE_NAME_EXTRA,
    [LOGLEVEL_TRACE] = LOGFILE_NAME_EXTRA,
};

static str log_tag[][16] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "TRACE",
};

static str *esc_code_none = "";
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


/*! -- INTERNAL USE ONLY --;
 */
static void _get_log_str(const str *str_in, str *str_out, u32 flags, b8 verbose,
        u8 level, u32 error_code, const str *file, u64 line);

u32 logger_init(b8 release_build, int argc, char **argv, const str *_log_dir)
{
    if (!init_time) init_time = get_time_raw_usec();
    if (release_build) log_level_max = LOGLEVEL_INFO;

    if (argc && argv &&
            argc > 2 && !strncmp(argv[1], "LOGLEVEL", 8ul))
        {
            if (!strncmp(argv[2], "FATAL", 5ul))        log_level_max = LOGLEVEL_FATAL;
            else if (!strncmp(argv[2], "ERROR", 5ul))   log_level_max = LOGLEVEL_ERROR;
            else if (!strncmp(argv[2], "WARN", 4ul))    log_level_max = LOGLEVEL_WARNING;
            else if (!strncmp(argv[2], "INFO", 4ul))    log_level_max = LOGLEVEL_INFO;
            else if (!strncmp(argv[2], "DEBUG", 5ul))   log_level_max = LOGLEVEL_DEBUG;
            else if (!strncmp(argv[2], "TRACE", 5ul))   log_level_max = LOGLEVEL_TRACE;
        }

    if (_log_dir && is_dir_exists(_log_dir, TRUE) == ERR_SUCCESS)
        snprintf(log_dir, PATH_MAX, "%s", _log_dir);

    if (mem_map((void*)&logger_buf, LOGGER_LINES_MAX * STRING_MAX,
                "logger_init().logger_buf") != ERR_SUCCESS)
    {
        _LOGFATAL(FALSE, ERR_LOGGER_INIT_FAIL,
                "%s\n", "Failed to Initialize Logger, Process Aborted");
        return engine_err;
    }

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void logger_close(void)
{
    mem_unmap((void*)&logger_buf, LOGGER_LINES_MAX * STRING_MAX, "logger_close().logger_buf");
}

void _log_output(b8 verbose, const str *_log_dir, const str *file, u64 line,
        u8 level, u32 error_code, const str *format, ...)
{
    va_list args;
    str str_in[IN_STRING_MAX] = {0};
    str str_out[OUT_STRING_MAX] = {0};
    str temp[PATH_MAX] = {0};

    if (level > log_level_max) return;

    va_start(args, format);
    vsnprintf(str_in, IN_STRING_MAX, format, args);
    va_end(args);

    _get_log_str(str_in, str_out, FLAG_LOG_COLOR, verbose, level, error_code, file, line);
    fprintf(stderr, "%s", str_out);

    if (_log_dir)
    {
        _get_log_str(str_in, str_out, FLAG_LOG_FULL_TIME, verbose, level, error_code, file, line);
        snprintf(temp, PATH_MAX, "%s%s", _log_dir, LOG_FILE_NAME[level]);
        _append_file(temp, 1, strlen(str_out), str_out, FALSE, FALSE);
    }
}

static void _get_log_str(const str *str_in, str *str_out, u32 flags, b8 verbose,
        u8 level, u32 error_code, const str *file, u64 line)
{
    str str_time[TIME_STRING_MAX] = {0};
    str str_timestamp[TIME_STRING_MAX] = {0};
    str str_time_full[TIME_STRING_MAX] = {0};
    str str_tag[32] = {0};
    str str_file[STRING_MAX] = {0};
    str *str_nocolor = esc_code_none;
    str *str_color = esc_code_none;

    if (flags & FLAG_LOG_FULL_TIME)
    {
        if ((flags & FLAG_LOG_DATE_TIME) == FLAG_LOG_DATE_TIME)
            get_time_str(str_time, "[%F %T]");
        else if (flags & FLAG_LOG_DATE)
            get_time_str(str_time, "[%F]");
        else if (flags & FLAG_LOG_TIME)
            get_time_str(str_time, "[%T]");
        if (flags & FLAG_LOG_TIMESTAMP)
            snprintf(str_timestamp, TIME_STRING_MAX, "[%"PRIu64"]", init_time);

        snprintf(str_time_full, TIME_STRING_MAX, "%s%s ", str_timestamp, str_time);
    }

    if (flags & FLAG_LOG_COLOR)
    {
        str_color = esc_code_color[level];
        str_nocolor = esc_code_nocolor;
    }

    if (level <= LOGLEVEL_WARNING)
        snprintf(str_tag, 32, "[%s][%"PRIu32"] ", log_tag[level], error_code);
    else
        snprintf(str_tag, 32, "[%s] ", log_tag[level]);

    if (verbose)
        snprintf(str_file, STRING_MAX, "[%s:%"PRIu64"] ", file, line);

    snprintf(str_out, OUT_STRING_MAX, "%s%s%s%s%s%s",
            str_color, str_time_full, str_tag, str_file, str_in, str_nocolor);
}
