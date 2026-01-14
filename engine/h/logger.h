#ifndef ENGINE_LOGGER_H
#define ENGINE_LOGGER_H

#include "types.h"
#include "memory.h"
#include "diagnostics.h"

#define LOGFILE_NAME_ERROR  "log_error.log"
#define LOGFILE_NAME_INFO   "log_info.log"
#define LOGFILE_NAME_EXTRA  "log_verbose.log"

enum LogFlag
{
    FLAG_LOG_WRITE_TO_DISK = 1 << 0,
}; /* LogFlag */

enum LogLevel
{
    LOGLEVEL_FATAL,
    LOGLEVEL_ERROR,
    LOGLEVEL_WARNING,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_TRACE,
    LOGLEVEL_COUNT,
}; /* LogLevel */

enum LogFile
{
    LOGFILE_FATAL = 0,
    LOGFILE_ERROR = 0,
    LOGFILE_WARNING = 0,
    LOGFILE_INFO = 1,
    LOGFILE_DEBUG = 2,
    LOGFILE_TRACE = 2,
    LOGFILE_COUNT,
};

#define LOGFATAL(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, __BASE_FILE__, __LINE__, \
            LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERROR(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, __BASE_FILE__, __LINE__, \
            LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNING(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, __BASE_FILE__, __LINE__, \
            LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define LOGINFO(verbose, format, ...) \
    _log_output(verbose, __BASE_FILE__, __LINE__, \
            LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#define LOGDEBUG(verbose, format, ...) \
    _log_output(verbose, __BASE_FILE__, __LINE__, \
            LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

#define LOGTRACE(verbose, format, ...) \
    _log_output(verbose, __BASE_FILE__, __LINE__, \
            LOGLEVEL_TRACE, 0, format, ##__VA_ARGS__)

#define LOGFATALEX(verbose, file, line, err, format, ...); \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, file, line, LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERROREX(verbose, file, line, err, format, ...); \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, file, line, LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNINGEX(verbose, file, line, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, file, line, \
            LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define LOGINFOEX(verbose, file, line, format, ...) \
    _log_output(verbose, file, line, LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#define LOGDEBUGEX(verbose, file, line, format, ...) \
    _log_output(verbose, file, line, \
            LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

#define LOGTRACEEX(verbose, file, line, format, ...) \
    _log_output(verbose, file, line, \
            LOGLEVEL_TRACE, 0, format, ##__VA_ARGS__)

#define LOG_MESH_GENERATE(err, mesh_name) \
{ \
    if (err == ERR_SUCCESS) \
    LOGINFO(FALSE, "Mesh '%s' Generated\n", mesh_name); \
    else if (err == ERR_MESH_GENERATION_FAIL) \
    LOGERROR(TRUE, ERR_MESH_GENERATION_FAIL, "Mesh '%s' Generation Failed\n", mesh_name); \
}

extern u32 log_level_max;

/*! @brief initialize logger.
 *
 *  @param argc, argv = used for logger log level if args provided.
 *  @param log_dir = directory to store log files, if NULL, logs won't be written to disk.
 *
 *  @remark called automatically from 'core.h/engine_init()'.
 *
 *  @remark argc and argv can be NULL.
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
u32 logger_init(b8 release_build, int argc, char **argv, const str *_log_dir);

void logger_close(void);

/*! -- INTERNAL USE ONLY --;
 */
void _log_output(b8 verbose, const str *file, u64 line,
        u8 level, u32 error_code, const str *format, ...);

#endif /* ENGINE_LOGGER_H */
