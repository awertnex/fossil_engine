#ifndef HHC_LOGGER_H
#define HHC_LOGGER_H

#include <engine/h/logger.h>

#include "common.h"

#define HHC_LOGFATAL(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_FATAL, GAME_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define HHC_LOGERROR(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_ERROR, GAME_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define HHC_LOGWARNING(err, flags, format, ...) \
    _fsl_log_output(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_WARNING, GAME_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#define HHC_LOGINFO(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_INFO, GAME_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#ifdef HHC_RELEASE_BUILD
#   define HHC_LOGDEBUG(flags, format, ...)
#   define HHC_LOGTRACE(flags, format, ...)
#else
#   define HHC_LOGDEBUG(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_DEBUG, GAME_DIR_NAME_LOGS, format, ##__VA_ARGS__)

#   define HHC_LOGTRACE(flags, format, ...) \
    _fsl_log_output(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_TRACE, GAME_DIR_NAME_LOGS, format, ##__VA_ARGS__)
#endif /* FOSSIL_RELEASE_BUILD */

#define HHC_LOG_MESH_GENERATE(err, mesh_name) \
{ \
    if (err == FSL_ERR_SUCCESS) \
    HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE, "Mesh '%s' Generated\n", mesh_name); \
    else if (err == FSL_ERR_MESH_GENERATION_FAIL) \
    HHC_LOGERROR(FSL_ERR_MESH_GENERATION_FAIL, 0, "Failed to Generate Mesh '%s'\n", mesh_name); \
}

#endif /* HHC_LOGGER_H */
