#ifndef ENGINE_LOGGER_H
#define ENGINE_LOGGER_H

#include "types.h"
#include "memory.h"
#include "diagnostics.h"

enum LogLevel
{
    LOGLEVEL_FATAL = 0,
    LOGLEVEL_ERROR,
    LOGLEVEL_WARNING,
    LOGLEVEL_INFO,
    LOGLEVEL_DEBUG,
    LOGLEVEL_TRACE,
}; /* LogLevel */

#define LOGFATAL(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, __FILE__, __LINE__, \
            LOGLEVEL_FATAL, err, format, ##__VA_ARGS__); \
}

#define LOGERROR(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, __FILE__, __LINE__, \
            LOGLEVEL_ERROR, err, format, ##__VA_ARGS__); \
}

#define LOGWARNING(verbose, err, format, ...) \
{ \
    engine_err = (u32)err; \
    _log_output(verbose, __FILE__, __LINE__, \
            LOGLEVEL_WARNING, err, format, ##__VA_ARGS__); \
}

#define LOGINFO(verbose, format, ...) \
    _log_output(verbose, __FILE__, __LINE__, \
            LOGLEVEL_INFO, 0, format, ##__VA_ARGS__)

#define LOGDEBUG(verbose, format, ...) \
    _log_output(verbose, __FILE__, __LINE__, \
            LOGLEVEL_DEBUG, 0, format, ##__VA_ARGS__)

#define LOGTRACE(verbose, format, ...) \
    _log_output(verbose, __FILE__, __LINE__, \
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

/*! @brief initialize logger.
 *
 *  @param argc, argv = used for logger log level if args provided.
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
u32 logger_init(b8 release_build, int argc, char **argv);

void logger_close(void);

/*! -- INTERNAL USE ONLY --;
 */
void _log_output(b8 verbose, const str *file, u64 line,
        u8 level, u32 error_code, const str* format, ...);

extern u32 log_level_max;
extern str *logger_buf;

#endif /* ENGINE_LOGGER_H */
