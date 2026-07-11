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
 *  @file model.c
 *
 *  @brief model parsing and drawing.
 */

#include "../engine/engine_assets.h"
#include "../math/matrix.h"
#include "../memory/memory.h"
#include "../shaders/shader_types.h"

#include "assets.h"

#include <stddef.h>

void fsl_model_draw(const fsl_model *model, const fsl_camera *camera,
        GLuint texture_id)
{
    fsl_shader_program *shader = fsl_mem_handle_get(fsl_shader_buf);
    m4f32 transform = fsl_multiply_m4f32(model->mat_model, camera->projection.perspective);

    glBindBuffer(GL_ARRAY_BUFFER, model->mesh.transform_buf.id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m4f32), &transform, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUseProgram(shader[FSL_SHADER_INDEX_OBJECT].asset.id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glBindVertexArray(model->mesh.vao);
    glDrawElementsInstanced(GL_TRIANGLES, model->mesh.index_buf.len, GL_UNSIGNED_INT, NULL, 1);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
