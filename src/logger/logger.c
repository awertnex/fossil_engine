/*  @file logger.c
 *
 *  @brief logger.
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

#include "../h/common.h"
#include "logger.h"

#include "../h/diagnostics.h"
#include "../h/dir.h"
#include "../h/limits.h"
#include "../h/math.h"
#include "../h/process.h"
#include "../h/string.h"
#include "../h/time.h"
#include "../h/types.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <inttypes.h>

/* ---- section: declarations ----------------------------------------------- */

fsl_logger_core logger_core = {0};
u32 fsl_log_level_max = FSL_LOG_LEVEL_TRACE;

/*! @brief each log level's log file name.
 *
 *  @remark initialized in @ref fsl_logger_init().
 */
static str LOG_FILE_NAME[FSL_LOG_LEVEL_COUNT][FSL_ID_CAP] = {0};

static u32 logger_color_tab[FSL_LOG_LEVEL_COUNT + 1] =
{
    FSL_DIAGNOSTIC_COLOR_FATAL,
    FSL_DIAGNOSTIC_COLOR_ERROR,
    FSL_DIAGNOSTIC_COLOR_WARNING,
    FSL_DIAGNOSTIC_COLOR_SUCCESS,
    FSL_DIAGNOSTIC_COLOR_INFO,
    FSL_DIAGNOSTIC_COLOR_DEBUG,
    FSL_DIAGNOSTIC_COLOR_TRACE
};

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

static str *esc_code_none = "";
static str *esc_code_nocolor = "\033[0m";
static str *esc_code_color[FSL_LOG_LEVEL_COUNT] =
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
static void logger_get_log_str_internal(const str *str_in, str *str_out, u32 flags, b8 verbose,
        u8 level, u32 error_code, const str *src_file, u64 line);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief like @ref fsl_is_dir_exists(), but no logging on success, no writing to
 *  log file and no modifying @ref fsl_err (used for logger dir checks).
 *
 *  @return non-zero on failure, error codes can be found in @ref diagnostics.h.
 */
static u32 logger_is_dir_exists_internal(const fsl_fs_path *path);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief like @ref fsl_append_file(), but no logging on success and no modifying
 *  @ref fsl_err (used for logger file writes).
 *
 *  @return non-zero on failure, error codes can be found in @ref diagnostics.h.
 */
static u32 logger_append_file_internal(const fsl_fs_path *path, u64 size, void *buf);

/* ---- section: implementation --------------------------------------------- */

u32 fsl_logger_init(int argc, char **argv, u64 flags)
{
    str str_in[FSL_STRING_MAX] = {0};
    str str_out[FSL_LOGGER_STRING_MAX] = {0};

    snprintf(logger_core.log_dir, PATH_MAX, "%s", FSL_DIR_NAME_LOGS);

    snprintf(LOG_FILE_NAME[FSL_LOG_LEVEL_FATAL], FSL_ID_CAP, "%s", FSL_FILE_NAME_LOG_ERROR);
    snprintf(LOG_FILE_NAME[FSL_LOG_LEVEL_ERROR], FSL_ID_CAP, "%s", FSL_FILE_NAME_LOG_ERROR);
    snprintf(LOG_FILE_NAME[FSL_LOG_LEVEL_WARNING], FSL_ID_CAP, "%s", FSL_FILE_NAME_LOG_ERROR);
    snprintf(LOG_FILE_NAME[FSL_LOG_LEVEL_SUCCESS], FSL_ID_CAP, "%s", FSL_FILE_NAME_LOG_INFO);
    snprintf(LOG_FILE_NAME[FSL_LOG_LEVEL_INFO], FSL_ID_CAP, "%s", FSL_FILE_NAME_LOG_INFO);
    snprintf(LOG_FILE_NAME[FSL_LOG_LEVEL_DEBUG], FSL_ID_CAP, "%s", FSL_FILE_NAME_LOG_EXTRA);
    snprintf(LOG_FILE_NAME[FSL_LOG_LEVEL_TRACE], FSL_ID_CAP, "%s", FSL_FILE_NAME_LOG_EXTRA);

    if (flags & FSL_FLAG_RELEASE_BUILD)
        fsl_log_level_max = FSL_LOG_LEVEL_INFO;

    if (argc && argc > 2 && argv)
    {
        if (strncmp(argv[2], "logfatal", 8ul) == 0)         fsl_log_level_max = FSL_LOG_LEVEL_FATAL;
        else if (strncmp(argv[2], "logerror", 8ul) == 0)    fsl_log_level_max = FSL_LOG_LEVEL_ERROR;
        else if (strncmp(argv[2], "logwarn", 7ul) == 0)     fsl_log_level_max = FSL_LOG_LEVEL_WARNING;
        else if (strncmp(argv[2], "logsuccess", 7ul) == 0)  fsl_log_level_max = FSL_LOG_LEVEL_SUCCESS;
        else if (strncmp(argv[2], "loginfo", 7ul) == 0)     fsl_log_level_max = FSL_LOG_LEVEL_INFO;
        else if (strncmp(argv[2], "logdebug", 8ul) == 0)    fsl_log_level_max = FSL_LOG_LEVEL_DEBUG;
        else if (strncmp(argv[2], "logtrace", 8ul) == 0)    fsl_log_level_max = FSL_LOG_LEVEL_TRACE;
    }

    if (fsl_mem_arena_init(&mem_arena_internal,
                "fsl_logger_init().mem_arena_internal") != FSL_ERR_SUCCESS ||

            fsl_mem_arena_push(&mem_arena_internal, &logger_core.buf,
                FSL_LOGGER_HISTORY_MAX * sizeof(fsl_log_entry),
                "fsl_logger_init().logger_core.buf") != FSL_ERR_SUCCESS)
    {
        fsl_err = FSL_ERR_LOGGER_INIT_FAIL;
        logger_get_log_str_internal(str_in, str_out, FSL_FLAG_LOG_TAG | FSL_FLAG_LOG_TERM_COLOR,
                TRUE, FSL_LOG_LEVEL_FATAL, FSL_ERR_LOGGER_INIT_FAIL, __BASE_FILE__, __LINE__);
        fprintf(stderr, "%s", str_out);
        return fsl_err;
    }

    logger_core.flag.gui_open = TRUE;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_logger_close(void)
{
    LOGTRACE(0, fsl_logger_stringf("%s\n", "Closing Logger.."));

    logger_core.flag.gui_open = FALSE;
}

#include <assert.h>
void fsl_log_output_internal(u32 error_code, u32 flags, const str *src_file, u64 line,
        u8 level, const str *message)
{
    str str_in[FSL_STRING_MAX] = {0};
    str str_out[FSL_LOGGER_STRING_MAX] = {0};
    str path_temp[PATH_MAX] = {0};
    b8 verbose =    !(flags & FSL_FLAG_LOG_NO_VERBOSE);
    b8 cmd =        (flags & FSL_FLAG_LOG_CMD);
    b8 write_file = !(flags & FSL_FLAG_LOG_NO_FILE);
    fsl_log_entry *logger_gui_entry = NULL;

    fsl_err = error_code;

    snprintf(str_in, FSL_STRING_MAX, "%s", message);
    logger_get_log_str_internal(str_in, str_out, FSL_FLAG_LOG_TAG | FSL_FLAG_LOG_TERM_COLOR,
            verbose, level, error_code, src_file, line);
    fprintf(stderr, "%s", str_out);

    if (logger_core.flag.gui_open)
    {
        if (cmd)
            logger_get_log_str_internal(str_in, str_out, 0, FALSE, level, 0, src_file, line);
        else
            logger_get_log_str_internal(str_in, str_out, FSL_FLAG_LOG_TAG | FSL_FLAG_LOG_DATE_TIME,
                    verbose, level, error_code, src_file, line);

        logger_gui_entry = fsl_mem_handle_get_i(fsl_log_entry, logger_core.buf, logger_core.cursor);
        snprintf(logger_gui_entry->message, strnlen(str_out, FSL_LOGGER_STRING_MAX), "%s", str_out);
        logger_gui_entry->color = logger_color_tab[level];
        logger_core.cursor = (logger_core.cursor + 1) % FSL_LOGGER_HISTORY_MAX;
    }

    if (write_file && logger_is_dir_exists_internal(FSL_DIR_NAME_LOGS) == FSL_ERR_SUCCESS)
    {
        logger_get_log_str_internal(str_in, str_out, FSL_FLAG_LOG_TAG | FSL_FLAG_LOG_FULL_TIME,
                verbose, level, error_code, src_file, line);
        snprintf(path_temp, PATH_MAX, "%s%s", FSL_DIR_NAME_LOGS, LOG_FILE_NAME[level]);
        logger_append_file_internal(path_temp, strnlen(str_out, FSL_LOGGER_STRING_MAX) * sizeof(str), str_out);
    }
}

static void logger_get_log_str_internal(const str *str_in, str *str_out, u32 flags, b8 verbose,
        u8 level, u32 error_code, const str *src_file, u64 line)
{
    str str_time[FSL_TIME_STRING_MAX] = {0};
    str str_timestamp[FSL_TIME_STRING_MAX] = {0};
    str str_time_full[FSL_TIME_STRING_MAX] = {0};
    str str_tag[32] = {0};
    str str_file[FSL_STRING_MAX] = {0};
    str *str_nocolor = esc_code_none;
    str *str_color = esc_code_none;
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
        str_color = esc_code_color[level];
        str_nocolor = esc_code_nocolor;
    }

    if (level <= FSL_LOG_LEVEL_WARNING)
        snprintf(str_tag, 32, "[%s][%"PRIu32"] ", log_tag[level], error_code);
    else if (flags & FSL_FLAG_LOG_TAG)
        snprintf(str_tag, 32, "[%s] ", log_tag[level]);

    if (verbose)
        snprintf(str_file, FSL_STRING_MAX, "[%s:%"PRIu64"] ", src_file, line);

    cursor = snprintf(str_out, FSL_LOGGER_STRING_MAX, "%s%s%s%s%s%s",
            str_color, str_time_full, str_tag, str_file, str_in, str_nocolor);

    if (cursor >= FSL_LOGGER_STRING_MAX - 1)
    {
        trunc = str_out + FSL_LOGGER_STRING_MAX - 4;
        snprintf(trunc, 4, "...");
    }
}

str *fsl_logger_stringf(const str *format, ...)
{
    static str buf[FSL_STRINGF_BUFFERS_MAX][FSL_STRING_MAX] = {0};
    static u64 index = 0;
    str *string = buf[index];
    __builtin_va_list args;

    va_start(args, format);
    vsnprintf(string, FSL_STRING_MAX, format, args);
    va_end(args);

    index = (index + 1) % FSL_STRINGF_BUFFERS_MAX;
    return string;
}

u32 logger_is_dir_exists_internal(const fsl_fs_path *path)
{
    struct stat stats;
    if (stat(path, &stats) == 0)
    {
        if (S_ISDIR(stats.st_mode))
            return FSL_ERR_SUCCESS;
        else return FSL_ERR_IS_NOT_DIR;
    }
    return FSL_ERR_DIR_NOT_FOUND;
}

u32 logger_append_file_internal(const fsl_fs_path *path, u64 size, void *buf)
{
    FILE *file = NULL;
    if ((file = fopen(path, "ab")) == NULL)
        return FSL_ERR_FILE_OPEN_FAIL;
    fwrite(buf, 1, size, file);
    fclose(file);
    return FSL_ERR_SUCCESS;
}
