/*  Copyright 2026 Lily Awertnex
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

/*
 *	logger.c - logger
 */

#include "h/common.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
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

/* ---- section: definitions ------------------------------------------------ */

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

/* ---- section: declarations ----------------------------------------------- */

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

/* ---- section: signatures ------------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 */
static void _fsl_get_log_str(const str *str_in, str *str_out, u32 flags, b8 verbose,
        u8 level, u32 error_code, const str *file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief like @ref fsl_is_dir_exists(), but no logging on success, no writing to
 *  log file and no modifying @ref fsl_err (used for logger dir checks).
 *
 *  @return non-zero on failure, error codes can be found in @ref diagnostics.h.
 */
static u32 fsl_logger_is_dir_exists(const str *name);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief like @ref fsl_append_file(), but no logging on success and no modifying
 *  @ref fsl_err (used for logger file writes).
 *
 *  @return non-zero on failure, error codes can be found in @ref diagnostics.h.
 */
static u32 fsl_logger_append_file(const str *name, u64 size, u64 length, void *buf);

/* ---- section: implementation --------------------------------------------- */

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
        if (fsl_get_path_bin_root(&FSL_DIR_PROC_ROOT) != FSL_ERR_SUCCESS)
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
        _LOGFATAL(FSL_ERR_LOGGER_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Initialize Logger, Process Aborted");
        return fsl_err;
    }

    for (i = 0; i < FSL_LOGGER_HISTORY_MAX; ++i)
        fsl_logger_tab[i] = fsl_logger_buf + i * FSL_LOGGER_STRING_MAX;

    fsl_log_flag |= FSL_FLAG_LOGGER_GUI_OPEN;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_logger_close(void)
{
    _LOGTRACE(0, "%s\n", "Closing Logger..");

    fsl_log_flag &= ~FSL_FLAG_LOGGER_GUI_OPEN;
    fsl_mem_unmap_arena(&fsl_logger_arena, "logger_close().fsl_logger_arena");
}

void _fsl_log_output(u32 error_code, u32 flags, const str *file, u64 line, u8 level,
        const str *_log_dir, const str *format, ...)
{
    __builtin_va_list args;
    str str_in[FSL_STRING_MAX] = {0};
    str str_out[FSL_LOGGER_STRING_MAX] = {0};
    str temp[PATH_MAX] = {0};
    b8 verbose =    !(flags & FSL_FLAG_LOG_NO_VERBOSE);
    b8 cmd =        (flags & FSL_FLAG_LOG_CMD);
    b8 write_file = !(flags & FSL_FLAG_LOG_NO_FILE);

    fsl_err = error_code;

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

    if (write_file && fsl_logger_is_dir_exists(_log_dir) == FSL_ERR_SUCCESS)
    {
        _fsl_get_log_str(str_in, str_out, FSL_FLAG_LOG_TAG | FSL_FLAG_LOG_FULL_TIME,
                verbose, level, error_code, file, line);
        snprintf(temp, PATH_MAX, "%s%s", _log_dir, FSL_LOG_FILE_NAME[level]);
        fsl_logger_append_file(temp, 1, strlen(str_out), str_out);
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

u32 fsl_logger_is_dir_exists(const str *name)
{
    struct stat stats;
    if (stat(name, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
            return FSL_ERR_SUCCESS;
        else return FSL_ERR_IS_NOT_DIR;
    }
    return FSL_ERR_DIR_NOT_FOUND;
}

u32 fsl_logger_append_file(const str *name, u64 size, u64 length, void *buf)
{
    FILE *file = NULL;
    if ((file = fopen(name, "ab")) == NULL)
        return FSL_ERR_FILE_OPEN_FAIL;
    fwrite(buf, size, length, file);
    fclose(file);
    return FSL_ERR_SUCCESS;
}
