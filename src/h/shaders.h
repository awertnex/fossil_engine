/*  @file shaders.h
 *
 *  @brief loading, pre-processing, parsing and unloading glsl shaders.
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

#ifndef FSL_SHADERS_H
#define FSL_SHADERS_H

#include "common.h"
#include "types.h"

#include <deps/glad/glad.h>

typedef struct fsl_shader
{
    str *file_name;
    GLuint id;          /* used by @ref glCreateShader() */
    GLuint type;        /* `GL_<x>_SHADER` */
    GLchar *source;     /* shader file source code */
    GLint loaded;       /* used by @ref glGetShaderiv() */
} fsl_shader;

typedef struct fsl_shader_program
{
    str *name;          /* for debugging */
    GLuint id;          /* used by @ref glCreateProgram() */
    GLint loaded;       /* used by @ref glGetProgramiv() */
    fsl_shader vertex;
    fsl_shader geometry;
    fsl_shader fragment;
} fsl_shader_program;

/*! @brief default shaders.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_init().
 */
FSLAPI extern fsl_shader_program fsl_shader_buf[FSL_SHADER_INDEX_COUNT];

/*! @brief initialize single shader.
 *
 *  - call @ref fsl_shader_pre_process() on `shader->file_name` before compiling shader and compile shader.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_shader_init(const str *shaders_dir, fsl_shader *shader);

FSLAPI void fsl_shader_free(fsl_shader *shader);

/*! @brief initialize shader program.
 *
 *  - call @ref fsl_shader_init() on all shaders in `program` if `shader->type` is set.
 *
 *  @param shaders_dir path to shader files of `program`,
 *  shader file names must be pre-defined in `program->shader.file_name`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_shader_program_init(const str *shaders_dir, fsl_shader_program *program);

FSLAPI void fsl_shader_program_free(fsl_shader_program *program);

#endif /* FSL_SHADERS_H */
