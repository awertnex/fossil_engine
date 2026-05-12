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
 *  @file shaders.h
 *
 *  @brief main shaders module header; shader definitions, pre-processing,
 *  loading and unloading glsl shaders.
 */

#ifndef FSL_SHADERS_H
#define FSL_SHADERS_H

#include "../common/engine_info.h"
#include "../common/types.h"

#include "shader_types.h"

/*!
 *  @brief initialize a single shader.
 *
 *  - pre-process shader before compiling shader and compile shader.
 *
 *  @param shader_created destination for whether an 'OpenGL' shader was successfully created,
 *  used by function @ref fsl_shader_program_init() to take care of dangling shader IDs in VRAM.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_shader_init(fsl_shader *shader, b8 *shader_created);

/*!
 *  @brief unload a single shader.
 */
FSLAPI void fsl_shader_free(fsl_shader *shader);

/*!
 *  @brief initialize a shader program.
 *
 *  - call @ref fsl_shader_init() on all shaders in `program` if
 *    @ref fsl_shader.asset.file and @ref fsl_shader.asset.path are not `NULL`.
 *
 *  @remark function @ref fsl_asset_set_metadata() must be used to initialize paths
 *  and file names for each shader within the program before calling this function.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_shader_program_init(fsl_shader_program *program);

/*!
 *  @brief unload a shader program.
 */
FSLAPI void fsl_shader_program_free(fsl_shader_program *program);

/*!
 *  @brief set a `vec3` attribute array for a `vao`.
 */
FSLAPI void fsl_attrib_vec3(void);

/*!
 *  @brief set a `vec3` and a `vec2` attribute arrays for a `vao`.
 */
FSLAPI void fsl_attrib_vec3_vec2(void);

/*!
 *  @brief set a `vec3` and a `vec3` attribute arrays for a `vao`.
 */
FSLAPI void fsl_attrib_vec3_vec3(void);

/*!
 *  @brief set a `vec3` and a `vec4` attribute arrays for a `vao`.
 */
FSLAPI void fsl_attrib_vec3_vec4(void);

#endif /* FSL_SHADERS_H */
