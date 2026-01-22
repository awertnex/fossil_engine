#include "h/common.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/math.h"
#include "h/process.h"
#include "h/string.h"
#include "h/time.h"
#include "h/types.h"

enum LogFileIndex
{
    LOGFILE_FATAL = 0,
    LOGFILE_ERROR = 0,
    LOGFILE_WARNING = 0,
    LOGFILE_INFO = 1,
    LOGFILE_DEBUG = 2,
    LOGFILE_TRACE = 2,
    LOGFILE_COUNT,
}; /* LogFileIndex */

u32 log_level_max = LOGLEVEL_TRACE;
static u64 log_flag = 0;
str log_dir[PATH_MAX] = {0};

/*! @brief logger arena, raw logger memory data.
 */
static MemArena logger_arena = {0};

/*! @brief logger ring buffer.
 */
static str *logger_buf = NULL;

str **logger_tab = NULL;
i32 logger_tab_index = 0;
u32 *logger_color = NULL;

static const str LOG_FILE_NAME[LOGLEVEL_COUNT][NAME_MAX] =
{
    [LOGLEVEL_FATAL] = ENGINE_FILE_NAME_LOG_ERROR,
    [LOGLEVEL_ERROR] = ENGINE_FILE_NAME_LOG_ERROR,
    [LOGLEVEL_WARNING] = ENGINE_FILE_NAME_LOG_ERROR,
    [LOGLEVEL_INFO] = ENGINE_FILE_NAME_LOG_INFO,
    [LOGLEVEL_DEBUG] = ENGINE_FILE_NAME_LOG_EXTRA,
    [LOGLEVEL_TRACE] = ENGINE_FILE_NAME_LOG_EXTRA,
};

static u32 logger_color_tab[LOGLEVEL_COUNT + 1] =
{
    DIAGNOSTIC_COLOR_FATAL,
    DIAGNOSTIC_COLOR_ERROR,
    DIAGNOSTIC_COLOR_WARNING,
    DIAGNOSTIC_COLOR_INFO,
    DIAGNOSTIC_COLOR_DEBUG,
    DIAGNOSTIC_COLOR_TRACE,
    DIAGNOSTIC_COLOR_SUCCESS,
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

u32 logger_init(int argc, char **argv, u64 flags, const str *_log_dir, b8 log_dir_not_found)
{
    u32 i = 0;

    if (flags & FLAG_ENGINE_RELEASE_BUILD)
        log_level_max = LOGLEVEL_INFO;

    if (argc && argc > 2 && argv)
    {
        if (strncmp(argv[2], "logfatal", 8ul) == 0)        log_level_max = LOGLEVEL_FATAL;
        else if (strncmp(argv[2], "logerror", 8ul) == 0)   log_level_max = LOGLEVEL_ERROR;
        else if (strncmp(argv[2], "logwarn", 7ul) == 0)    log_level_max = LOGLEVEL_WARNING;
        else if (strncmp(argv[2], "loginfo", 7ul) == 0)    log_level_max = LOGLEVEL_INFO;
        else if (strncmp(argv[2], "logdebug", 8ul) == 0)   log_level_max = LOGLEVEL_DEBUG;
        else if (strncmp(argv[2], "logtrace", 8ul) == 0)   log_level_max = LOGLEVEL_TRACE;
    }

    if (!init_time)
    {
        init_time = get_time_raw_usec();
        get_time_nsec(); /* initialize start time */
        get_time_nsecf(); /* initialize start time */
    }

    if (!DIR_PROC_ROOT)
    {
        DIR_PROC_ROOT = get_path_bin_root();
        if (!DIR_PROC_ROOT)
            return engine_err;
        change_dir(DIR_PROC_ROOT);
    }

    if (is_dir_exists(_log_dir, log_dir_not_found) == ERR_SUCCESS)
        snprintf(log_dir, PATH_MAX, "%s", _log_dir);

    if (
            mem_map_arena(&logger_arena,
                LOGGER_HISTORY_MAX * sizeof(u32) +
                LOGGER_HISTORY_MAX * sizeof(str*) +
                LOGGER_HISTORY_MAX * LOGGER_STRING_MAX,
                "logger_init().logger_arena") != ERR_SUCCESS ||

            mem_push_arena(&logger_arena, (void*)&logger_color, LOGGER_HISTORY_MAX * sizeof(u32),
                "logger_init().logger_color") != ERR_SUCCESS ||

            mem_push_arena(&logger_arena, (void*)&logger_tab, LOGGER_HISTORY_MAX * sizeof(str*),
                "logger_init().logger_tab") != ERR_SUCCESS ||

            mem_push_arena(&logger_arena, (void*)&logger_buf, LOGGER_HISTORY_MAX * LOGGER_STRING_MAX,
                "logger_init().logger_buf") != ERR_SUCCESS)
    {
        _LOGFATAL(FALSE, ERR_LOGGER_INIT_FAIL,
                "%s\n", "Failed to Initialize Logger, Process Aborted");
        return engine_err;
    }

    for (i = 0; i < LOGGER_HISTORY_MAX; ++i)
        logger_tab[i] = logger_buf + i * LOGGER_STRING_MAX;

    log_flag |= FLAG_LOGGER_GUI_OPEN;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void logger_close(void)
{
    _LOGTRACE(TRUE, "%s\n", "Closing Logger..");

    log_flag &= ~FLAG_LOGGER_GUI_OPEN;
    mem_unmap_arena(&logger_arena, "logger_close().logger_color");
}

void _log_output(b8 verbose, b8 cmd, const str *_log_dir, const str *file, u64 line,
        u8 level, u32 error_code, const str *format, ...)
{
    __builtin_va_list args;
    str str_in[STRING_MAX] = {0};
    str str_out[LOGGER_STRING_MAX] = {0};
    str temp[PATH_MAX] = {0};

    if (level > log_level_max) return;

    va_start(args, format);
    vsnprintf(str_in, STRING_MAX, format, args);
    va_end(args);

    _get_log_str(str_in, str_out, FLAG_LOG_TAG | FLAG_LOG_TERM_COLOR,
            verbose, level, error_code, file, line);
    fprintf(stderr, "%s", str_out);

    if (log_flag & FLAG_LOGGER_GUI_OPEN)
    {
        if (cmd)
            _get_log_str(str_in, str_out, 0, FALSE, level, 0, file, line);
        else
            _get_log_str(str_in, str_out, FLAG_LOG_TAG | FLAG_LOG_DATE_TIME,
                    verbose, level, error_code, file, line);

        snprintf(logger_tab[logger_tab_index], strnlen(str_out, LOGGER_STRING_MAX), "%s", str_out);
        logger_color[logger_tab_index] = logger_color_tab[level];
        logger_tab_index = (logger_tab_index + 1) % LOGGER_HISTORY_MAX;
    }

    if (is_dir_exists(_log_dir, FALSE) == ERR_SUCCESS)
    {
        _get_log_str(str_in, str_out, FLAG_LOG_TAG | FLAG_LOG_FULL_TIME,
                verbose, level, error_code, file, line);
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
    str *trunc = NULL;
    int cursor = 0;

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

    if (flags & FLAG_LOG_TERM_COLOR)
    {
        str_color = esc_code_color[level];
        str_nocolor = esc_code_nocolor;
    }

    if (level <= LOGLEVEL_WARNING)
        snprintf(str_tag, 32, "[%s][%"PRIu32"] ", log_tag[level], error_code);
    else if (flags & FLAG_LOG_TAG)
        snprintf(str_tag, 32, "[%s] ", log_tag[level]);

    if (verbose)
        snprintf(str_file, STRING_MAX, "[%s:%"PRIu64"] ", file, line);

    cursor = snprintf(str_out, LOGGER_STRING_MAX, "%s%s%s%s%s%s",
            str_color, str_time_full, str_tag, str_file, str_in, str_nocolor);

    if (cursor >= LOGGER_STRING_MAX - 1)
    {
        trunc = str_out + LOGGER_STRING_MAX - 4;
        snprintf(trunc, 4, "...");
    }
}
