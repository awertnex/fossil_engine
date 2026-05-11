/*!
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
 *  limitations under the License.
 */

/*!
 *  @file logger.h
 *
 *  @brief main logger module header; include everything regarding logging.
 */

#ifndef FSL_LOGGER_H
#define FSL_LOGGER_H

#include "../common/engine_info.h"
#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../common/types.h"
#include "../memory/memory.h"

#include "logger_messages.h"
#include "logger_macros.h"

typedef struct fsl_log_entry fsl_log_entry;
typedef struct fsl_logger_core fsl_logger_core;

enum fsl_log_output_flag
{
    FSL_FLAG_LOG_NO_VERBOSE =   0x0001, /* don't log 'file' and 'line' */
    FSL_FLAG_LOG_CMD =          0x0002, /* log a command (e.g., "Gravity Disabled" and nothing else) */
    FSL_FLAG_LOG_NO_FILE =      0x0004  /* don't write to log file */
}; /* fsl_log_output_flag */

enum fsl_log_message_flag
{
    FSL_FLAG_LOG_TIMESTAMP =    0x0001,
    FSL_FLAG_LOG_DATE =         0x0002,
    FSL_FLAG_LOG_TIME =         0x0004,
    FSL_FLAG_LOG_DATE_TIME =    0x0006,
    FSL_FLAG_LOG_FULL_TIME =    0x0007,
    FSL_FLAG_LOG_TAG =          0x0008,
    FSL_FLAG_LOG_TERM_COLOR =   0x0010
}; /* fsl_log_message_flag */

enum fsl_log_level
{
    FSL_LOG_LEVEL_FATAL,
    FSL_LOG_LEVEL_ERROR,
    FSL_LOG_LEVEL_WARNING,
    FSL_LOG_LEVEL_SUCCESS,
    FSL_LOG_LEVEL_INFO,
    FSL_LOG_LEVEL_DEBUG,
    FSL_LOG_LEVEL_TRACE,
    FSL_LOG_LEVEL_COUNT
}; /* fsl_log_level */

/*!
 *  @brief global logger buffer, raw log data.
 *
 *  @remark buffer mapped onto @ref mem_arena_internal.
 */
struct fsl_logger_core
{
    struct /* flag */
    {
        b8 gui_open;
    } flag;

    str log_dir[FSL_PATH_CAP];
    fsl_mem_handle buf;     /* logger strings */
    fsl_mem_arena arena;    /* logger's memory arena */
    i32 cursor;             /* current position in `buf` */
}; /* fsl_logger_core */

struct fsl_log_entry
{
    u32 color;
    str message[FSL_LOGGER_STRING_MAX];
}; /* fsl_log_entry */

/*!
 *  @brief logger core, all logger data.
 *
 *  @remark read-only, initialized internally in @ref _fsl_logger_init().
 */
FSLAPI extern fsl_logger_core logger_core;

/*!
 *  @remark read-only, initialized internally in @ref _fsl_logger_init().
 */
FSLAPI extern u32 fsl_log_level_max;

/*!
 *  @internal
 *
 *  @brief allocate and initialize logger resources.
 *
 *  @param argc number of arguments in `argv` if `argv` provided.
 *  @param argv used for logger log level (optional).
 *  @param flags enum @ref fsl_flag.
 *
 *  @remark called automatically from @ref fsl_engine_init().
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
FSLAPI u32 fsl_logger_init(int argc, char **argv, u64 flags);

FSLAPI void fsl_logger_close(void);

/*!
 *  @internal
 *
 *  @param flags enum @ref fsl_log_output_flag.
 *
 *  @param message @ref fsl_logger_stringf() can be used to create a temporary
 *  static formatted string for the log message.
 */
FSLAPI void fsl_log_output_internal(u32 error_code, u32 flags, const str *src_file, u64 src_line,
        u8 level, const str *message);

/*!
 *  @brief like @ref fsl_stringf(), but used for the logger, since @ref fsl_stringf()
 *  uses features like buffer truncation with `...`, the logger needs the raw string.
 *
 *  @note the use of @ref fsl_logger_stringf more than once in a single expression is not advised.
 *
 *  @remark use temporary static buffers internally.
 *  @remark inspired by Raylib: `github.com/raysan5/raylib`: `raylib/src/rtext.c/TextFormat()`.
 *
 *  @return static formatted string.
 */
FSLAPI str *fsl_logger_stringf(const str *format, ...);

#endif /* FSL_LOGGER_H */
