/*  @file assets.c
 *
 *  @brief engine default assets.
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

#include "h/common.h"
#include "h/core.h"
#include "h/shaders.h"

fsl_mesh fsl_mesh_unit_quad = {0};
fsl_texture fsl_texture_buf[FSL_TEXTURE_INDEX_COUNT] = {0};
fsl_shader_program fsl_shader_buf[FSL_SHADER_INDEX_COUNT] = {0};

void fsl_assets_init(void)
{
    fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].name = "unit_quad";
    fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].vertex.file_name = "unit_quad.vert";
    fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].vertex.type = GL_VERTEX_SHADER;
    fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].fragment.file_name = "unit_quad.frag";
    fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].fragment.type = GL_FRAGMENT_SHADER;

    fsl_shader_buf[FSL_SHADER_INDEX_TEXT].name = "text";
    fsl_shader_buf[FSL_SHADER_INDEX_TEXT].vertex.file_name = "text.vert";
    fsl_shader_buf[FSL_SHADER_INDEX_TEXT].vertex.type = GL_VERTEX_SHADER;
    fsl_shader_buf[FSL_SHADER_INDEX_TEXT].fragment.file_name = "text.frag";
    fsl_shader_buf[FSL_SHADER_INDEX_TEXT].fragment.type = GL_FRAGMENT_SHADER;

    fsl_shader_buf[FSL_SHADER_INDEX_UI].name = "ui";
    fsl_shader_buf[FSL_SHADER_INDEX_UI].vertex.file_name = "ui.vert";
    fsl_shader_buf[FSL_SHADER_INDEX_UI].vertex.type = GL_VERTEX_SHADER;
    fsl_shader_buf[FSL_SHADER_INDEX_UI].fragment.file_name = "ui.frag";
    fsl_shader_buf[FSL_SHADER_INDEX_UI].fragment.type = GL_FRAGMENT_SHADER;

    fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].name = "ui_9_slice";
    fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].vertex.file_name = "ui_9_slice.vert";
    fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].vertex.type = GL_VERTEX_SHADER;
    fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].fragment.file_name = "ui_9_slice.frag";
    fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].fragment.type = GL_FRAGMENT_SHADER;
}
