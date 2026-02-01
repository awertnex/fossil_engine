/*  Copyright 2026 Lily Awertnex
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

/*  assets.c - engine default assets
 */

#include "h/common.h"
#include "h/core.h"
#include "h/shaders.h"

fsl_mesh fsl_mesh_unit_quad = {0};
fsl_texture fsl_texture_buf[FSL_TEXTURE_INDEX_COUNT] = {0};

fsl_shader_program fsl_shader_buf[FSL_SHADER_INDEX_COUNT] =
{
    [FSL_SHADER_INDEX_UNIT_QUAD] =
    {
        .name = "unit_quad",
        .vertex.file_name = "unit_quad.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "unit_quad.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [FSL_SHADER_INDEX_TEXT] =
    {
        .name = "text",
        .vertex.file_name = "text.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "text.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [FSL_SHADER_INDEX_UI] =
    {
        .name = "ui",
        .vertex.file_name = "ui.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "ui.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [FSL_SHADER_INDEX_UI_9_SLICE] =
    {
        .name = "ui_9_slice",
        .vertex.file_name = "ui_9_slice.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "ui_9_slice.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },
};
