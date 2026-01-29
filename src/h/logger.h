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

/*  logger.h - logger
 */

#ifndef FSL_LOGGER_H
#define FSL_LOGGER_H

/*  notes:
 *      log macros beginning with an underscore (`_`) are used by the engine.
 *      log macros ending in `EX` are used to pass `file` and `line` manually.
 */

#include "common.h"
#include "diagnostics.h"
#include "limits.h"
#include "memory.h"
#include "types.h"

enum fsl_logger_flag
{
    FSL_FLAG_LOGGER_GUI_OPEN = 0x0001,
}; /* LoggerFlag */

enum fsl_log_output_flag
{
    FSL_FLAG_LOG_NO_VERBOSE =   0x0001, /* don't log file and line */
    FSL_FLAG_LOG_CMD =          0x0002, /* log a command (e.g. "Gravity Toggled On" and nothing else) */
    FSL_FLAG_LOG_NO_FILE =      0x0004, /* don't write to log file */
}; /* fsl_log_output_flag */

enum fsl_log_message_flag
{
    FSL_FLAG_LOG_TIMESTAMP =    0x0001,
    FSL_FLAG_LOG_DATE =         0x0002,
    FSL_FLAG_LOG_TIME =         0x0004,
    FSL_FLAG_LOG_DATE_TIME =    0x0006,
    FSL_FLAG_LOG_FULL_TIME =    0x0007,
    FSL_FLAG_LOG_TAG =          0x0008,
    FSL_FLAG_LOG_TERM_COLOR =   0x0010,
}; /* fsl_log_message_flag */

enum fsl_log_level
{
    FSL_LOG_LEVEL_FATAL,
    FSL_LOG_LEVEL_ERROR,
    FSL_LOG_LEVEL_WARNING,
    FSL_LOG_LEVEL_INFO,
    FSL_LOG_LEVEL_DEBUG,
    FSL_LOG_LEVEL_TRACE,
    FSL_LOG_LEVEL_COUNT,
}; /* fsl_log_level */

/* ---- external-use macros ------------------------------------------------- */

#define LOGFATAL(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_FATAL, fsl_log_dir, format, ##__VA_ARGS__)

#define LOGERROR(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_ERROR, fsl_log_dir, format, ##__VA_ARGS__)

#define LOGWARNING(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_WARNING, fsl_log_dir, format, ##__VA_ARGS__)

#define LOGINFO(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_INFO, fsl_log_dir, format, ##__VA_ARGS__)

#ifdef FOSSIL_RELEASE_BUILD
#   define LOGDEBUG(flags, format, ...)
#   define LOGTRACE(flags, format, ...)
#else
#   define LOGDEBUG(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_DEBUG, fsl_log_dir, format, ##__VA_ARGS__)

#   define LOGTRACE(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_TRACE, fsl_log_dir, format, ##__VA_ARGS__)
#endif /* FOSSIL_RELEASE_BUILD */

#define LOG_MESH_GENERATE(err, mesh_name) \
{ \
    if (err == FSL_ERR_SUCCESS) \
    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE, "Mesh '%s' Generated\n", mesh_name); \
    else if (err == FSL_ERR_MESH_GENERATION_FAIL) \
    LOGERROR(FSL_ERR_MESH_GENERATION_FAIL, 0, "Failed to Generate Mesh '%s'\n", mesh_name); \
}

/* ---- internal-use macros ------------------------------------------------- */

#define _LOGFATAL(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_FATAL, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define _LOGERROR(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_ERROR, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define _LOGWARNING(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_WARNING, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define _LOGINFO(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_INFO, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define _LOGFATALEX(err, flags, file, line, format, ...); \
    _fsl_log_output(err, flags, file, line, FSL_LOG_LEVEL_FATAL, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define _LOGERROREX(err, flags, file, line, format, ...); \
    _fsl_log_output(err, flags, file, line, FSL_LOG_LEVEL_ERROR, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define _LOGWARNINGEX(err, flags, file, line, format, ...) \
    _fsl_log_output(err, flags, file, line, FSL_LOG_LEVEL_WARNING, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define _LOGINFOEX(flags, file, line, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, file, line, FSL_LOG_LEVEL_INFO, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#ifdef FOSSIL_RELEASE_BUILD
#   define _LOGDEBUG(flags, format, ...)
#   define _LOGTRACE(flags, format, ...)
#   define _LOGDEBUGEX(flags, file, line, format, ...)
#   define _LOGTRACEEX(flags, file, line, format, ...)
#else
#   define _LOGDEBUG(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_DEBUG, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#   define _LOGTRACE(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_TRACE, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#   define _LOGDEBUGEX(flags, file, line, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, file, line, FSL_LOG_LEVEL_DEBUG, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#   define _LOGTRACEEX(flags, file, line, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, file, line, FSL_LOG_LEVEL_TRACE, FSL_DIR_NAME_LOGS, format, ##__VA_ARGS__)
#endif /* FOSSIL_RELEASE_BUILD */

extern u32 fsl_log_level_max;
FSLAPI extern str fsl_log_dir[PATH_MAX];

/*! @brief logger pointer look-up table that points to `fsl_logger_buf` addresses.
 *
 *  @remark read-only, initialized internally in @ref fsl_logger_init().
 */
FSLAPI extern str **fsl_logger_tab;

/*! @brief logger color look-up table for @ref logger_tab entries.
 *
 *  @remark read-only, initialized internally in @ref fsl_logger_init().
 */
FSLAPI extern u32 *fsl_logger_color;

/*! @brief current position in @ref fsl_logger_tab.
 *
 *  @remark read-only, updated internally in @ref _fsl_log_output().
 */
FSLAPI extern i32 fsl_logger_tab_index;

/*! @brief initialize logger.
 *
 *  @param argc number of arguments in `argv` if `argv` provided.
 *  @param argv used for logger log level if args provided.
 *
 *  @param flags enum @ref fsl_flag.
 *
 *  @param _log_dir directory to write log files into for the lifetime of the process,
 *  if `NULL`, logs won't be written to disk.
 *
 *  @param log_dir_not_found enable/disable logging @ref FSL_ERR_DIR_NOT_FOUND when `_log_dir` isn't found
 *  (@ref fsl_is_dir_exists() parameter).
 *
 *  @remark called automatically from @ref fsl_init().
 *
 *  @remark args:
 *      logfatal:   only output fatal logs (least verbose).
 *      logerror:   only output <= error logs.
 *      logwarn:    only output <= warning logs.
 *      loginfo:    only output <= info logs (default).
 *      logdebug:   only output <= debug logs.
 *      logtrace:   only output <= trace logs (most verbose).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_logger_init(int argc, char **argv, u64 flags, const str *_log_dir, b8 log_dir_not_found);

FSLAPI void fsl_logger_close(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param flags enum: @ref fsl_log_output_flag.
 *  @param _log_dir directory to write log files into, if `NULL`, logs won't be written to disk.
 */
FSLAPI void _fsl_log_output(u32 error_code, u32 flags, const str *file, u64 line, u8 level,
        const str *_log_dir, const str *format, ...);

#endif /* FSL_LOGGER_H */
