/*  @file diagnostics.h
 *
 *  @brief error codes, fsl_err, diagnostic colors.
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

#ifndef FSL_DIAGNOSTICS_H
#define FSL_DIAGNOSTICS_H

#include "common.h"
#include "types.h"
#include "limits.h"

#define FSL_DIAGNOSTIC_COLOR_DEFAULT        0xecececff
#define FSL_DIAGNOSTIC_COLOR_FATAL          0xa33021ff
#define FSL_DIAGNOSTIC_COLOR_ERROR          0xcc5829ff
#define FSL_DIAGNOSTIC_COLOR_WARNING        0xa3729eff
#define FSL_DIAGNOSTIC_COLOR_INFO           0x8fccccff
#define FSL_DIAGNOSTIC_COLOR_DEBUG          0xccccccff
#define FSL_DIAGNOSTIC_COLOR_TRACE          0xc4ae29ff
#define FSL_DIAGNOSTIC_COLOR_SUCCESS        0x6ccc49ff

#define FSL_ERR_SUCCESS                     0
#define FSL_ERR_POINTER_NULL                4096
#define FSL_ERR_POINTER_NOT_NULL            4097
#define FSL_ERR_BUFFER_EMPTY                4098
#define FSL_ERR_BUFFER_FULL                 4099
#define FSL_ERR_BUFFER_OVERFLOW             4100
#define FSL_ERR_FILE_NOT_FOUND              4101
#define FSL_ERR_DIR_NOT_FOUND               4102
#define FSL_ERR_IS_NOT_FILE                 4103
#define FSL_ERR_IS_NOT_DIR                  4104
#define FSL_ERR_FILE_OPEN_FAIL              4105
#define FSL_ERR_DIR_OPEN_FAIL               4106
#define FSL_ERR_FILE_EXISTS                 4107
#define FSL_ERR_DIR_EXISTS                  4108
#define FSL_ERR_SOURCE_NOT_CHANGE           4109
#define FSL_ERR_PATH_TOO_LONG               4110
#define FSL_ERR_STRING_TOO_LONG             4111
#define FSL_ERR_PROCESS_FORK_FAIL           4112
#define FSL_ERR_EXEC_FAIL                   4113
#define FSL_ERR_EXEC_PROCESS_NON_ZERO       4114
#define FSL_ERR_EXEC_TERMINATE_BY_SIGNAL    4115
#define FSL_ERR_EXEC_ABNORMAL_EXIT          4116
#define FSL_ERR_EXECVP_FAIL                 4117
#define FSL_ERR_WAITPID_FAIL                4118
#define FSL_ERR_MEM_ALLOC_FAIL              4119
#define FSL_ERR_MEM_REALLOC_FAIL            4120
#define FSL_ERR_MEM_MAP_FAIL                4121
#define FSL_ERR_GET_PATH_ABSOLUTE_FAIL      4122
#define FSL_ERR_GET_PATH_BIN_ROOT_FAIL      4123
#define FSL_ERR_MEM_COMMIT_FAIL             4124
#define FSL_ERR_LOGGER_INIT_FAIL            4125
#define FSL_ERR_GLFW                        4126
#define FSL_ERR_GLFW_INIT_FAIL              4127
#define FSL_ERR_WINDOW_INIT_FAIL            4128
#define FSL_ERR_GLAD_INIT_FAIL              4129
#define FSL_ERR_GL_VERSION_NOT_SUPPORT      4130
#define FSL_ERR_SHADER_COMPILE_FAIL         4131
#define FSL_ERR_SHADER_PROGRAM_LINK_FAIL    4132
#define FSL_ERR_FBO_INIT_FAIL               4133
#define FSL_ERR_FBO_REALLOC_FAIL            4134
#define FSL_ERR_IMAGE_LOAD_FAIL             4135
#define FSL_ERR_IMAGE_SIZE_TOO_SMALL        4136
#define FSL_ERR_FONT_INIT_FAIL              4137
#define FSL_ERR_TEXT_INIT_FAIL              4138
#define FSL_ERR_MESH_GENERATION_FAIL        4139
#define FSL_ERR_INCLUDE_RECURSION_LIMIT     4140
#define FSL_ERR_SELF_INCLUDE                4141
#define FSL_ERR_DLL_LOAD_FAIL               4142
#define FSL_ERR_SIZE_TOO_SMALL              4143
#define FSL_ERR_DIR_CREATE_FAIL             4144
#define FSL_ERR_WINDOW_NOT_FOUND            4145
#define FSL_ERR_UI_INIT_FAIL                4146
#define FSL_ERR_SHADER_TYPE_NULL            4147
#define FSL_ERR_SCREENSHOT_FAIL             4148
#define FSL_ERR_BUILD_FUNCTION_NOT_FOUND    4149
#define FSL_ERR_MEM_ARENA_MAP_FAIL          4150
#define FSL_ERR_SIZE_LIMIT                  4151
#define FSL_ERR_MEM_REMAP_FAIL              4152
#define FSL_ERR_FILE_STAT_FAIL              4153
#define FSL_ERR_READ_LINK_FAIL              4154

/*! @brief global variable for engine-specific error codes.
 *
 *  @remark must be declared by the user if not including @ref core.c.
 */
FSLAPI extern u32 fsl_err;

#endif /* FSL_DIAGNOSTICS_H */
