#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#include "h/types.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/string.h"
#include "h/time.h"

u32 log_level_max = LOGLEVEL_TRACE;
u64 log_flag = 0;
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

static str *esc_code_none = 0;
static str esc_code_nocolor[16] = "\033[0m";
static str esc_code_color[LOGLEVEL_COUNT][16] =
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
static void _get_log_str(const str *str_in, str *str_out, b8 date, b8 time,
        b8 verbose, b8 color, u8 level, u32 error_code, const str *file, u64 line);

u32 logger_init(b8 release_build, int argc, char **argv, const str *_log_dir)
{
    str temp[3][PATH_MAX] = {0};

    if (release_build)
        log_level_max = LOGLEVEL_INFO;

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

    if (_log_dir)
    {
        log_flag |= FLAG_LOG_WRITE_TO_DISK;

        if (is_dir_exists(_log_dir, FALSE) != ERR_SUCCESS)
            make_dir(_log_dir);

        snprintf(log_dir, PATH_MAX, "%s", _log_dir);
        check_slash(log_dir);
        posix_slash(log_dir);

        snprintf(temp[0], PATH_MAX, "%s"LOGFILE_NAME_ERROR, log_dir);
        snprintf(temp[1], PATH_MAX, "%s"LOGFILE_NAME_INFO, log_dir);
        snprintf(temp[2], PATH_MAX, "%s"LOGFILE_NAME_EXTRA, log_dir);

        if (is_file_exists(temp[0], FALSE) != ERR_SUCCESS)
            write_file(temp[0], 1, 0, 0, FALSE, FALSE);
        if (is_file_exists(temp[1], FALSE) != ERR_SUCCESS)
            write_file(temp[1], 1, 0, 0, FALSE, FALSE);
        if (is_file_exists(temp[2], FALSE) != ERR_SUCCESS)
            write_file(temp[2], 1, 0, 0, FALSE, FALSE);
    }
    else
        log_flag &= ~FLAG_LOG_WRITE_TO_DISK;

    if (mem_map((void*)&logger_buf, LOGGER_LINES_MAX * STRING_MAX,
                "logger_init().logger_buf") != ERR_SUCCESS)
    {
        LOGFATAL(FALSE, ERR_LOGGER_INIT_FAIL,
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

void _log_output(b8 verbose, const str *file, u64 line,
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

    _get_log_str(str_in, str_out, FALSE, FALSE, verbose, TRUE, level, error_code, file, line);
    fprintf(stderr, "%s", str_out);

    if (log_flag & FLAG_LOG_WRITE_TO_DISK)
    {
        _get_log_str(str_in, str_out, TRUE, TRUE, verbose, FALSE, level, error_code, file, line);
        snprintf(temp, PATH_MAX, "%s%s", log_dir, LOG_FILE_NAME[level]);
        _append_file(temp, 1, strlen(str_out), str_out, FALSE, FALSE);
    }
}

static void _get_log_str(const str *str_in, str *str_out, b8 date, b8 time,
        b8 verbose, b8 color, u8 level, u32 error_code, const str *file, u64 line)
{
    str str_time[TIME_STRING_MAX] = {0};
    str str_tag[32] = {0};
    str str_file[STRING_MAX] = {0};
    str *str_nocolor = esc_code_none;
    str *str_color = esc_code_none;

    if (date && time) get_time_str(str_time, "[%F %T] ");
    else if (date) get_time_str(str_time, "[%F] ");
    else if (time) get_time_str(str_time, "[%T] ");

    if (color)
    {
        str_color = esc_code_color[level];
        str_nocolor = esc_code_nocolor;
    }

    if (level <= LOGLEVEL_WARNING)
        snprintf(str_tag, 32, "[%s][%"PRIu32"] ", log_tag[level], error_code);
    else snprintf(str_tag, 32, "[%s] ", log_tag[level]);

    if (verbose)
        snprintf(str_file, STRING_MAX, "[%s:%"PRIu64"] ", file, line);

    snprintf(str_out, OUT_STRING_MAX, "%s%s%s%s%s%s",
            str_color, str_time, str_tag, str_file, str_in, str_nocolor);
}
