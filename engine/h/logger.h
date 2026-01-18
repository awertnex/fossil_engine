#ifndef ENGINE_LOGGER_H
#define ENGINE_LOGGER_H

#include "common.h"
#include "diagnostics.h"
#include "limits.h"
#include "memory.h"
#include "types.h"

enum /* LoggerFlag */
{
    FLAG_LOGGER_GUI_OPEN = 0x0001,
}; /* LoggerFlag */

enum /* LogMessageFlag */
{
    FLAG_LOG_TIMESTAMP =    0x0001,
    FLAG_LOG_DATE =         0x0002,
    FLAG_LOG_TIME =         0x0004,
    FLAG_LOG_DATE_TIME =    0x0006,
    FLAG_LOG_FULL_TIME =    0x0007,
    FLAG_LOG_TAG =          0x0008,
    FLAG_LOG_TERM_COLOR =   0x0010,
}; /* LogMessageFlag */

enum /* LogLevel */
{
    LOGLEVEL_FATAL,
    LOGLEVEL_ERROR,
    LOGLEVEL_WARNING,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_TRACE,
    LOGLEVEL_COUNT,
}; /* LogLevel */

enum /* LogFile */
{
    LOGFILE_FATAL = 0,
    LOGFILE_ERROR = 0,
    LOGFILE_WARNING = 0,
    LOGFILE_INFO = 1,
    LOGFILE_DEBUG = 2,
    LOGFILE_TRACE = 2,
    LOGFILE_COUNT,
};

/* ---- external-use macros ------------------------------------------------- */

#define LOGFATAL(verbose, cmd, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, cmd, log_dir, __BASE_FILE__, __LINE__, LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERROR(verbose, cmd, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, cmd, log_dir, __BASE_FILE__, __LINE__, LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNING(verbose, cmd, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, cmd, log_dir, __BASE_FILE__, __LINE__, LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define LOGINFO(verbose, cmd, format, ...) \
    _log_output(verbose, cmd, log_dir, __BASE_FILE__, __LINE__, LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#define LOGDEBUG(verbose, cmd, format, ...) \
    _log_output(verbose, cmd, log_dir, __BASE_FILE__, __LINE__, LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

#define LOGTRACE(verbose, cmd, format, ...) \
    _log_output(verbose, cmd, log_dir, __BASE_FILE__, __LINE__, LOGLEVEL_TRACE, 0, format, ##__VA_ARGS__)

#define LOGFATALEX(verbose, cmd, file, line, err, format, ...); \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, cmd, log_dir, file, line, LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERROREX(verbose, cmd, file, line, err, format, ...); \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, cmd, log_dir, file, line, LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNINGEX(verbose, cmd, file, line, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, cmd, log_dir, file, line, LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define LOGINFOEX(verbose, cmd, file, line, format, ...) \
    _log_output(verbose, cmd, log_dir, file, line, LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#define LOGDEBUGEX(verbose, cmd, file, line, format, ...) \
    _log_output(verbose, cmd, log_dir, file, line, LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

#define LOGTRACEEX(verbose, cmd, file, line, format, ...) \
    _log_output(verbose, cmd, log_dir, file, line, LOGLEVEL_TRACE, 0, format, ##__VA_ARGS__)

#define LOG_MESH_GENERATE(err, cmd, mesh_name) \
{ \
    if (err == ERR_SUCCESS) \
    LOGINFO(FALSE, cmd, "Mesh '%s' Generated\n", mesh_name); \
    else if (err == ERR_MESH_GENERATION_FAIL) \
    LOGERROR(TRUE, cmd, ERR_MESH_GENERATION_FAIL, "Failed to Generate Mesh '%s'\n", mesh_name); \
}

/* ---- internal-use macros ------------------------------------------------- */

#define _LOGFATAL(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define _LOGERROR(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define _LOGWARNING(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define _LOGINFO(verbose, format, ...) \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#define _LOGDEBUG(verbose, format, ...) \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

#define _LOGTRACE(verbose, format, ...) \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, __BASE_FILE__, __LINE__, LOGLEVEL_TRACE, 0, format, ##__VA_ARGS__)

#define _LOGFATALEX(verbose, file, line, err, format, ...); \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, file, line, LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define _LOGERROREX(verbose, file, line, err, format, ...); \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, file, line, LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define _LOGWARNINGEX(verbose, file, line, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, file, line, LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define _LOGINFOEX(verbose, file, line, format, ...) \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, file, line, LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#define _LOGDEBUGEX(verbose, file, line, format, ...) \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, file, line, LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

#define _LOGTRACEEX(verbose, file, line, format, ...) \
    _log_output(verbose, FALSE, ENGINE_DIR_NAME_LOGS, file, line, LOGLEVEL_TRACE, 0, format, ##__VA_ARGS__)

extern u32 log_level_max;
extern str log_dir[PATH_MAX];

/*! @brief logger pointer look-up table that points to 'logger_buf' addresses.
 *
 *  @remark read-only, initialized internally in 'logger_init()'.
 */
extern str **logger_tab;

/*! @brief logger color look-up table for 'logger_tab' entries.
 *
 *  @remark read-only, initialized internally in 'logger_init()'.
 */
extern u32 *logger_color;

/*! @brief current position in 'logger_tab'.
 *
 *  @remark read-only, updated internally in '_log_output()'.
 */
extern i32 logger_tab_index;

/*! @brief initialize logger.
 *
 *  @param argc, argv = used for logger log level if args provided.
 *
 *  @param flags = enum 'common.h/EngineFlag'.
 *
 *  @param _log_dir = directory to write log files into for the lifetime of the process,
 *  if NULL, logs won't be written to disk.
 *
 *  @param log_dir_not_found = if TRUE, will log ERR_DIR_NOT_FOUND if '_log_dir()' not found
 *  ('is_dir_exists()' parameter).
 *
 *  @remark called automatically from 'core.h/engine_init()'.
 *
 *  @remark args:
 *      LOGLEVEL FATAL = log only fatal errors.
 *      LOGLEVEL ERROR = log errors and above.
 *      LOGLEVEL WARNING = log warnings and above.
 *      LOGLEVEL INFO = log info and above.
 *      LOGLEVEL DEBUG = log debug and above.
 *      LOGLEVEL TRACE = log everything, default.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 logger_init(int argc, char **argv, u64 flags, const str *_log_dir, b8 log_dir_not_found);

void logger_close(void);

/*! -- INTERNAL USE ONLY --;
 *
 *  @param cmd = log a command, used for on-screen output of basic commands.
 *  @param _log_dir = directory to write log files into, if NULL, logs won't be written to disk.
 */
void _log_output(b8 verbose, b8 cmd, const str *_log_dir, const str *file, u64 line,
        u8 level, u32 error_code, const str *format, ...);

#endif /* ENGINE_LOGGER_H */
