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

#define LOGFATALEX(verbose, cmd, file, line, err, format, ...); \
{ \
    fsl_err = (u32)err; \
    _log_output(verbose, cmd, fsl_log_dir, file, line, FSL_LOG_LEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERROREX(verbose, cmd, file, line, err, format, ...); \
{ \
    fsl_err = (u32)err; \
    _log_output(verbose, cmd, fsl_log_dir, file, line, FSL_LOG_LEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNINGEX(verbose, cmd, file, line, err, format, ...) \
{ \
    fsl_err = (u32)err; \
    _log_output(verbose, cmd, fsl_log_dir, file, line, FSL_LOG_LEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#ifdef FOSSIL_RELEASE_BUILD
#   define LOGDEBUGEX(verbose, cmd, file, line, format, ...)
#   define LOGTRACEEX(verbose, cmd, file, line, format, ...)
#else
#   define LOGDEBUGEX(verbose, cmd, file, line, format, ...) \
    _log_output(verbose, cmd, fsl_log_dir, file, line, FSL_LOG_LEVEL_DEBUG, ERR_SUCCESS, format, ##__VA_ARGS__)

#   define LOGTRACEEX(verbose, cmd, file, line, format, ...) \
    _log_output(verbose, cmd, fsl_log_dir, file, line, FSL_LOG_LEVEL_TRACE, ERR_SUCCESS, format, ##__VA_ARGS__)
#endif /* FOSSIL_RELEASE_BUILD */

enum fsl_log_file_index
{
    FSL_LOG_FILE_INDEX_FATAL = 0,
    FSL_LOG_FILE_INDEX_ERROR = 0,
    FSL_LOG_FILE_INDEX_WARNING = 0,
    FSL_LOG_FILE_INDEX_INFO = 1,
    FSL_LOG_FILE_INDEX_DEBUG = 2,
    FSL_LOG_FILE_INDEX_TRACE = 2,
    FSL_LOG_FILE_INDEX_COUNT,
}; /* fsl_log_file_index */

u32 fsl_log_level_max = FSL_LOG_LEVEL_TRACE;
static u64 fsl_log_flag = 0;
str fsl_log_dir[PATH_MAX] = {0};

/*! @brief logger arena, raw logger memory data.
 */
static fsl_mem_arena fsl_logger_arena = {0};

/*! @brief logger ring buffer.
 */
static str *fsl_logger_buf = NULL;

str **fsl_logger_tab = NULL;
i32 fsl_logger_tab_index = 0;
u32 *fsl_logger_color = NULL;

static const str FSL_LOG_FILE_NAME[FSL_LOG_LEVEL_COUNT][NAME_MAX] =
{
    [FSL_LOG_LEVEL_FATAL] = FSL_FILE_NAME_LOG_ERROR,
    [FSL_LOG_LEVEL_ERROR] = FSL_FILE_NAME_LOG_ERROR,
    [FSL_LOG_LEVEL_WARNING] = FSL_FILE_NAME_LOG_ERROR,
    [FSL_LOG_LEVEL_INFO] = FSL_FILE_NAME_LOG_INFO,
    [FSL_LOG_LEVEL_DEBUG] = FSL_FILE_NAME_LOG_EXTRA,
    [FSL_LOG_LEVEL_TRACE] = FSL_FILE_NAME_LOG_EXTRA,
};

static u32 fsl_logger_color_tab[FSL_LOG_LEVEL_COUNT + 1] =
{
    FSL_DIAGNOSTIC_COLOR_FATAL,
    FSL_DIAGNOSTIC_COLOR_ERROR,
    FSL_DIAGNOSTIC_COLOR_WARNING,
    FSL_DIAGNOSTIC_COLOR_INFO,
    FSL_DIAGNOSTIC_COLOR_DEBUG,
    FSL_DIAGNOSTIC_COLOR_TRACE,
    FSL_DIAGNOSTIC_COLOR_SUCCESS,
};

static str fsl_log_tag[][16] =
{
    "FATAL",
    "ERROR",
    "WARNING",
    "INFO",
    "DEBUG",
    "TRACE",
};

static str *fsl_esc_code_none = "";
static str *fsl_esc_code_nocolor = "\033[0m";
static str *fsl_esc_code_color[FSL_LOG_LEVEL_COUNT] =
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
static void _fsl_get_log_str(const str *str_in, str *str_out, u32 flags, b8 verbose,
        u8 level, u32 error_code, const str *file, u64 line);

u32 fsl_logger_init(int argc, char **argv, u64 flags, const str *_log_dir, b8 log_dir_not_found)
{
    u32 i = 0;

    if (flags & FSL_FLAG_RELEASE_BUILD)
        fsl_log_level_max = FSL_LOG_LEVEL_INFO;

    if (argc && argc > 2 && argv)
    {
        if (strncmp(argv[2], "logfatal", 8ul) == 0)        fsl_log_level_max = FSL_LOG_LEVEL_FATAL;
        else if (strncmp(argv[2], "logerror", 8ul) == 0)   fsl_log_level_max = FSL_LOG_LEVEL_ERROR;
        else if (strncmp(argv[2], "logwarn", 7ul) == 0)    fsl_log_level_max = FSL_LOG_LEVEL_WARNING;
        else if (strncmp(argv[2], "loginfo", 7ul) == 0)    fsl_log_level_max = FSL_LOG_LEVEL_INFO;
        else if (strncmp(argv[2], "logdebug", 8ul) == 0)   fsl_log_level_max = FSL_LOG_LEVEL_DEBUG;
        else if (strncmp(argv[2], "logtrace", 8ul) == 0)   fsl_log_level_max = FSL_LOG_LEVEL_TRACE;
    }

    if (!fsl_init_time)
    {
        fsl_init_time = fsl_get_time_raw_usec();
        fsl_get_time_nsec(); /* initialize start time */
        fsl_get_time_nsecf(); /* initialize start time */
    }

    if (!FSL_DIR_PROC_ROOT)
    {
        FSL_DIR_PROC_ROOT = fsl_get_path_bin_root();
        if (!FSL_DIR_PROC_ROOT)
            return fsl_err;
        fsl_change_dir(FSL_DIR_PROC_ROOT);
    }

    if (fsl_is_dir_exists(_log_dir, log_dir_not_found) == FSL_ERR_SUCCESS)
        snprintf(fsl_log_dir, PATH_MAX, "%s", _log_dir);

    if (
            fsl_mem_map_arena(&fsl_logger_arena,
                FSL_LOGGER_HISTORY_MAX * sizeof(u32) +
                FSL_LOGGER_HISTORY_MAX * sizeof(str*) +
                FSL_LOGGER_HISTORY_MAX * FSL_LOGGER_STRING_MAX,
                "fsl_logger_init().fsl_logger_arena") != FSL_ERR_SUCCESS ||

            fsl_mem_push_arena(&fsl_logger_arena, (void*)&fsl_logger_color,
                FSL_LOGGER_HISTORY_MAX * sizeof(u32),
                "fsl_logger_init().fsl_logger_color") != FSL_ERR_SUCCESS ||

            fsl_mem_push_arena(&fsl_logger_arena, (void*)&fsl_logger_tab,
                FSL_LOGGER_HISTORY_MAX * sizeof(str*),
                "fsl_logger_init().fsl_logger_tab") != FSL_ERR_SUCCESS ||

            fsl_mem_push_arena(&fsl_logger_arena, (void*)&fsl_logger_buf,
                FSL_LOGGER_HISTORY_MAX * FSL_LOGGER_STRING_MAX,
                "fsl_logger_init().fsl_logger_buf") != FSL_ERR_SUCCESS)
    {
        _LOGFATAL(FALSE, FSL_ERR_LOGGER_INIT_FAIL,
                "%s\n", "Failed to Initialize Logger, Process Aborted");
        return fsl_err;
    }

    for (i = 0; i < FSL_LOGGER_HISTORY_MAX; ++i)
        fsl_logger_tab[i] = fsl_logger_buf + i * FSL_LOGGER_STRING_MAX;

    fsl_log_flag |= FSL_FLAG_LOGGER_GUI_OPEN;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void logger_close(void)
{
    _LOGTRACE(TRUE, "%s\n", "Closing Logger..");

    fsl_log_flag &= ~FSL_FLAG_LOGGER_GUI_OPEN;
    fsl_mem_unmap_arena(&fsl_logger_arena, "logger_close().fsl_logger_arena");
}

void _fsl_log_output(b8 verbose, b8 cmd, const str *_log_dir, const str *file, u64 line,
        u8 level, u32 error_code, const str *format, ...)
{
    __builtin_va_list args;
    str str_in[FSL_STRING_MAX] = {0};
    str str_out[FSL_LOGGER_STRING_MAX] = {0};
    str temp[PATH_MAX] = {0};

    if (level > fsl_log_level_max) return;

    va_start(args, format);
    vsnprintf(str_in, FSL_STRING_MAX, format, args);
    va_end(args);

    _fsl_get_log_str(str_in, str_out, FSL_FLAG_LOG_TAG | FSL_FLAG_LOG_TERM_COLOR,
            verbose, level, error_code, file, line);
    fprintf(stderr, "%s", str_out);

    if (fsl_log_flag & FSL_FLAG_LOGGER_GUI_OPEN)
    {
        if (cmd)
            _fsl_get_log_str(str_in, str_out, 0, FALSE, level, 0, file, line);
        else
            _fsl_get_log_str(str_in, str_out, FSL_FLAG_LOG_TAG | FSL_FLAG_LOG_DATE_TIME,
                    verbose, level, error_code, file, line);

        snprintf(fsl_logger_tab[fsl_logger_tab_index], strnlen(str_out, FSL_LOGGER_STRING_MAX), "%s", str_out);
        fsl_logger_color[fsl_logger_tab_index] = fsl_logger_color_tab[level];
        fsl_logger_tab_index = (fsl_logger_tab_index + 1) % FSL_LOGGER_HISTORY_MAX;
    }

    if (fsl_is_dir_exists(_log_dir, FALSE) == FSL_ERR_SUCCESS)
    {
        _fsl_get_log_str(str_in, str_out, FSL_FLAG_LOG_TAG | FSL_FLAG_LOG_FULL_TIME,
                verbose, level, error_code, file, line);
        snprintf(temp, PATH_MAX, "%s%s", _log_dir, FSL_LOG_FILE_NAME[level]);
        _fsl_append_file(temp, 1, strlen(str_out), str_out, FALSE, FALSE);
    }
}

static void _fsl_get_log_str(const str *str_in, str *str_out, u32 flags, b8 verbose,
        u8 level, u32 error_code, const str *file, u64 line)
{
    str str_time[FSL_TIME_STRING_MAX] = {0};
    str str_timestamp[FSL_TIME_STRING_MAX] = {0};
    str str_time_full[FSL_TIME_STRING_MAX] = {0};
    str str_tag[32] = {0};
    str str_file[FSL_STRING_MAX] = {0};
    str *str_nocolor = fsl_esc_code_none;
    str *str_color = fsl_esc_code_none;
    str *trunc = NULL;
    int cursor = 0;

    if (flags & FSL_FLAG_LOG_FULL_TIME)
    {
        if ((flags & FSL_FLAG_LOG_DATE_TIME) == FSL_FLAG_LOG_DATE_TIME)
            fsl_get_time_str(str_time, "[%F %T]");
        else if (flags & FSL_FLAG_LOG_DATE)
            fsl_get_time_str(str_time, "[%F]");
        else if (flags & FSL_FLAG_LOG_TIME)
            fsl_get_time_str(str_time, "[%T]");
        if (flags & FSL_FLAG_LOG_TIMESTAMP)
            snprintf(str_timestamp, FSL_TIME_STRING_MAX, "[%"PRIu64"]", fsl_init_time);

        snprintf(str_time_full, FSL_TIME_STRING_MAX, "%s%s ", str_timestamp, str_time);
    }

    if (flags & FSL_FLAG_LOG_TERM_COLOR)
    {
        str_color = fsl_esc_code_color[level];
        str_nocolor = fsl_esc_code_nocolor;
    }

    if (level <= FSL_LOG_LEVEL_WARNING)
        snprintf(str_tag, 32, "[%s][%"PRIu32"] ", fsl_log_tag[level], error_code);
    else if (flags & FSL_FLAG_LOG_TAG)
        snprintf(str_tag, 32, "[%s] ", fsl_log_tag[level]);

    if (verbose)
        snprintf(str_file, FSL_STRING_MAX, "[%s:%"PRIu64"] ", file, line);

    cursor = snprintf(str_out, FSL_LOGGER_STRING_MAX, "%s%s%s%s%s%s",
            str_color, str_time_full, str_tag, str_file, str_in, str_nocolor);

    if (cursor >= FSL_LOGGER_STRING_MAX - 1)
    {
        trunc = str_out + FSL_LOGGER_STRING_MAX - 4;
        snprintf(trunc, 4, "...");
    }
}
