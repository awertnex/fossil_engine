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
 *  @file shader_pre_processor_internal.h
 *
 *  @brief pre-processing glsl shaders.
 */

#ifndef FSL_SHADER_PRE_PROCESSOR_INTERNAL_H
#define FSL_SHADER_PRE_PROCESSOR_INTERNAL_H

#include "../common/limits.h"
#include "../common/types.h"

#include "../external/glad/glad.h"

#define FSL_SHADER_PRE_PROCESSOR_INCLUDE_RECURSION_MAX 512

/*!
 *  @internal
 *
 *  @brief process shader before compilation.
 *
 *  - parse includes recursively.
 *
 *  @return `NULL` on failure and @ref fsl_err is set accordingly.
 */
str *fsl_shader_pre_process_internal(const str *path, u64 *file_len);

/*!
 *  @internal
 *
 *  @brief get shader type based on file name conventions.
 *
 *  examples:
 *      - file name "fbo.vert" -> shader type "GL_VERTEX_SHADER".
 *      - file name "frag.glsl" or "fragment.glsl" -> shader type "GL_FRAGMENT_SHADER".
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 fsl_shader_get_type_internal(const str *file, GLenum *type);

#endif /* FSL_SHADER_PRE_PROCESSOR_INTERNAL_H */
