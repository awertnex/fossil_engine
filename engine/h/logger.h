#ifndef FSL_LOGGER_H
#define FSL_LOGGER_H

/*  Notes:
 *      log macros that begin with an underscore (`_`) are used by the engine.
 *      log macros that end in `EX` (extended) are used to pass `file` and `line` manually.
 */

#include "common.h"
#include "diagnostics.h"
#include "limits.h"
#include "memory.h"
#include "types.h"

enum /* fsl_logger_flag */
{
    FSL_FLAG_LOGGER_GUI_OPEN = 0x0001,
}; /* LoggerFlag */

enum /* fsl_log_message_flag */
{
    FSL_FLAG_LOG_TIMESTAMP =    0x0001,
    FSL_FLAG_LOG_DATE =         0x0002,
    FSL_FLAG_LOG_TIME =         0x0004,
    FSL_FLAG_LOG_DATE_TIME =    0x0006,
    FSL_FLAG_LOG_FULL_TIME =    0x0007,
    FSL_FLAG_LOG_TAG =          0x0008,
    FSL_FLAG_LOG_TERM_COLOR =   0x0010,
}; /* fsl_log_message_flag */

enum /* fsl_log_level */
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

#define LOGFATAL(verbose, cmd, err, format, ...) \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, cmd, fsl_log_dir, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERROR(verbose, cmd, err, format, ...) \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, cmd, fsl_log_dir, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNING(verbose, cmd, err, format, ...) \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, cmd, fsl_log_dir, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define LOGINFO(verbose, cmd, format, ...) \
    _fsl_log_output(verbose, cmd, fsl_log_dir, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_INFO, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#define LOGINFOEX(verbose, cmd, file, line, format, ...) \
    _fsl_log_output(verbose, cmd, fsl_log_dir, file, line, FSL_LOG_LEVEL_INFO, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#ifdef FOSSIL_RELEASE_BUILD
#   define LOGDEBUG(verbose, cmd, format, ...)
#   define LOGTRACE(verbose, cmd, format, ...)
#else
#   define LOGDEBUG(verbose, cmd, format, ...) \
    _fsl_log_output(verbose, cmd, fsl_log_dir, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_DEBUG, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#   define LOGTRACE(verbose, cmd, format, ...) \
    _fsl_log_output(verbose, cmd, fsl_log_dir, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_TRACE, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#endif /* FOSSIL_RELEASE_BUILD */

#define LOG_MESH_GENERATE(err, mesh_name) \
{ \
    if (err == FSL_ERR_SUCCESS) \
    LOGDEBUG(FALSE, FALSE, "Mesh '%s' Generated\n", mesh_name); \
    else if (err == FSL_ERR_MESH_GENERATION_FAIL) \
    LOGERROR(TRUE, FALSE, FSL_ERR_MESH_GENERATION_FAIL, "Failed to Generate Mesh '%s'\n", mesh_name); \
}

/* ---- internal-use macros ------------------------------------------------- */

#define _LOGFATAL(verbose, err, format, ...) \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define _LOGERROR(verbose, err, format, ...) \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define _LOGWARNING(verbose, err, format, ...) \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define _LOGINFO(verbose, format, ...) \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_INFO, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#define _LOGFATALEX(verbose, file, line, err, format, ...); \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, file, line, FSL_LOG_LEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define _LOGERROREX(verbose, file, line, err, format, ...); \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, file, line, FSL_LOG_LEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define _LOGWARNINGEX(verbose, file, line, err, format, ...) \
{ \
    fsl_err = (u32)err; \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, file, line, FSL_LOG_LEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define _LOGINFOEX(verbose, file, line, format, ...) \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, file, line, FSL_LOG_LEVEL_INFO, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#ifdef FOSSIL_RELEASE_BUILD
#   define _LOGDEBUG(verbose, format, ...)
#   define _LOGTRACE(verbose, format, ...)
#   define _LOGDEBUGEX(verbose, file, line, format, ...)
#   define _LOGTRACEEX(verbose, file, line, format, ...)
#else
#   define _LOGDEBUG(verbose, format, ...) \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_DEBUG, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#   define _LOGTRACE(verbose, format, ...) \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_TRACE, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#   define _LOGDEBUGEX(verbose, file, line, format, ...) \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, file, line, FSL_LOG_LEVEL_DEBUG, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)

#   define _LOGTRACEEX(verbose, file, line, format, ...) \
    _fsl_log_output(verbose, FALSE, FSL_DIR_NAME_LOGS, file, line, FSL_LOG_LEVEL_TRACE, FSL_ERR_SUCCESS, format, ##__VA_ARGS__)
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
 *  @param cmd log a command (used for on-screen output of basic commands).
 *  @param _log_dir directory to write log files into, if `NULL`, logs won't be written to disk.
 */
FSLAPI void _fsl_log_output(b8 verbose, b8 cmd, const str *_log_dir, const str *file, u64 line,
        u8 level, u32 error_code, const str *format, ...);

#endif /* FSL_LOGGER_H */
