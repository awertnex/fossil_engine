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
 *  @file shader_types.h
 *
 *  @brief shader types.
 */

#ifndef FSL_SHADER_TYPES_H
#define FSL_SHADER_TYPES_H

#include "../h/asset_types.h"

#include <deps/glad/glad.h>

typedef struct fsl_shader           fsl_shader;
typedef struct fsl_shader_program   fsl_shader_program;

struct fsl_shader
{
    fsl_asset asset;
    GLint status;       /* used by @ref glGetShaderiv() */
    GLchar *source;     /* shader file source code */
}; /* fsl_shader */

struct fsl_shader_program
{
    fsl_asset asset;
    GLint status;       /* used by @ref glGetProgramiv() */
    fsl_shader vertex;
    fsl_shader geometry;
    fsl_shader fragment;
}; /* fsl_shader_program */

#endif /* FSL_SHADER_TYPES_H */
