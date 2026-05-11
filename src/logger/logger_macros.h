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
 *  @file logger_macros.h
 *
 *  @brief logger macros.
 */

#ifndef FSL_LOGGER_MACROS_H
#define FSL_LOGGER_MACROS_H

#define LOGFATAL(err, flags, message) \
    fsl_log_output_internal(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_FATAL, message);

#define LOGERROR(err, flags, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_ERROR) \
            fsl_log_output_internal(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_ERROR, message); \
    } while (0)

#define LOGWARNING(err, flags, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_WARNING) \
            fsl_log_output_internal(err, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_WARNING, message); \
    } while (0)

#define LOGSUCCESS(flags, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_SUCCESS) \
            fsl_log_output_internal(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_SUCCESS, message); \
    } while (0)

#define LOGINFO(flags, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_INFO) \
            fsl_log_output_internal(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_INFO, message); \
    } while (0)

#define LOGDEBUG(flags, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_DEBUG) \
            fsl_log_output_internal(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_DEBUG, message); \
    } while (0)

#define LOGTRACE(flags, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_TRACE) \
            fsl_log_output_internal(FSL_ERR_SUCCESS, flags, __BASE_FILE__, __LINE__, FSL_LOG_LEVEL_TRACE, message); \
    } while (0)

#define LOGFATALEX(err, flags, file, line, message) \
            fsl_log_output_internal(err, flags, file, line, FSL_LOG_LEVEL_FATAL, message)

#define LOGERROREX(err, flags, file, line, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_ERROR) \
            fsl_log_output_internal(err, flags, file, line, FSL_LOG_LEVEL_ERROR, message); \
    } while (0)

#define LOGWARNINGEX(err, flags, file, line, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_WARNING) \
            fsl_log_output_internal(err, flags, file, line, FSL_LOG_LEVEL_WARNING, message); \
    } while (0)

#define LOGSUCCESSEX(flags, file, line, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_SUCCESS) \
            fsl_log_output_internal(FSL_ERR_SUCCESS, flags, file, line, FSL_LOG_LEVEL_SUCCESS, message); \
    } while (0)

#define LOGINFOEX(flags, file, line, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_INFO) \
            fsl_log_output_internal(FSL_ERR_SUCCESS, flags, file, line, FSL_LOG_LEVEL_INFO, message); \
    } while (0)

#define LOGDEBUGEX(flags, file, line, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_DEBUG) \
            fsl_log_output_internal(FSL_ERR_SUCCESS, flags, file, line, FSL_LOG_LEVEL_DEBUG, message); \
    } while (0)

#define LOGTRACEEX(flags, file, line, message) \
    do { \
        if (fsl_log_level_max >= FSL_LOG_LEVEL_TRACE) \
            fsl_log_output_internal(FSL_ERR_SUCCESS, flags, file, line, FSL_LOG_LEVEL_TRACE, message); \
    } while (0)

#endif /* FSL_LOGGER_MACROS_H */
