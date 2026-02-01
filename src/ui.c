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

/*  ui.c - everything about drawing ui elements
 */

#include "h/common.h"
#include "h/core.h"
#include "h/diagnostics.h"
#include "h/logger.h"
#include "h/shaders.h"
#include "h/ui.h"

static f32 vbo_data_unit_quad[] =
{
    0.0f, -1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f,
};

static struct fsl_ui_core
{
    b8 multisample;

    GLuint vao;
    GLuint vbo_unit_quad;
    GLuint vbo_nine_slice;
    b8 vao_loaded;
    fsl_fbo fbo;

    struct /* uniform */
    {
        struct /* ui */
        {
            GLint position;
            GLint size;
            GLint texture_size;
            GLint offset;
            GLint alignment;
            GLint tint;
        } ui;

        struct /* nine_slice */
        {
            GLint tint;
        } nine_slice;

    } uniform;

} fsl_ui_core;

u32 fsl_ui_init(b8 multisample)
{
    u32 i = 0;

    if (
            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_ACTIVE], (v2i32){16, 16},
                GL_RGB, GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE,
                FSL_DIR_NAME_TEXTURES"panel_active.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_INACTIVE], (v2i32){16, 16},
                GL_RGB, GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE,
                FSL_DIR_NAME_TEXTURES"panel_inactive.png") != FSL_ERR_SUCCESS ||

            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE], (v2i32){128, 128},
                GL_RGB, GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE,
                FSL_DIR_NAME_TEXTURES"panel_debug_nine_slice.png") != FSL_ERR_SUCCESS)
        goto cleanup;

    for (i = 0; i < FSL_TEXTURE_INDEX_COUNT; ++i)
        if (fsl_texture_generate(&fsl_texture_buf[i], FALSE) != FSL_ERR_SUCCESS)
            goto cleanup;

    if (fsl_fbo_init(&fsl_ui_core.fbo, &fsl_mesh_unit_quad, multisample, 4) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (!fsl_ui_core.vao_loaded)
    {
        glGenVertexArrays(1, &fsl_ui_core.vao);
        glBindVertexArray(fsl_ui_core.vao);

        /* ---- unit quad --------------------------------------------------- */

        glGenBuffers(1, &fsl_ui_core.vbo_unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, fsl_ui_core.vbo_unit_quad);
        glBufferData(GL_ARRAY_BUFFER, fsl_arr_len(vbo_data_unit_quad) * sizeof(f32),
                &vbo_data_unit_quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

        /* ---- nine-slice data --------------------------------------------- */

        glGenBuffers(1, &fsl_ui_core.vbo_nine_slice);
        glBindBuffer(GL_ARRAY_BUFFER, fsl_ui_core.vbo_nine_slice);
        glBufferData(GL_ARRAY_BUFFER, sizeof(fsl_panel_nine_slice),
                NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                sizeof(fsl_panel_slice), (void*)0);
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                sizeof(fsl_panel_slice), (void*)(2 * sizeof(f32)));
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE,
                sizeof(fsl_panel_slice), (void*)(4 * sizeof(f32)));
        glVertexAttribDivisor(4, 1);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE,
                sizeof(fsl_panel_slice), (void*)(6 * sizeof(f32)));
        glVertexAttribDivisor(5, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        fsl_ui_core.vao_loaded = TRUE;
    }

    fsl_ui_core.multisample = multisample;

    fsl_ui_core.uniform.ui.position =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "position");
    fsl_ui_core.uniform.ui.size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "size");
    fsl_ui_core.uniform.ui.texture_size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "texture_size");
    fsl_ui_core.uniform.ui.offset =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "offset");
    fsl_ui_core.uniform.ui.alignment =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "alignment");
    fsl_ui_core.uniform.ui.tint =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI].id, "tint");

    fsl_ui_core.uniform.nine_slice.tint =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id, "tint");

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_ui_free();
    _LOGFATAL(FSL_ERR_UI_INIT_FAIL,
            FSL_FLAG_LOG_NO_VERBOSE,
            "%s\n", "Failed to Initialize UI, Process Aborted");
    return fsl_err;
}

void fsl_ui_start(fsl_fbo *fbo, b8 nine_slice, b8 clear)
{
    static v2i32 render_size = {0};
    fsl_fbo *_fbo = &fsl_ui_core.fbo;

    if (fbo) _fbo = fbo;
    if (render_size.x != render->size.x || render_size.y != render->size.y)
        fsl_fbo_realloc(_fbo, fsl_ui_core.multisample, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo->fbo);

    if (nine_slice)
        glUseProgram(fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].id);
    else
        glUseProgram(fsl_shader_buf[FSL_SHADER_INDEX_UI].id);

    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
}

void fsl_ui_push_panel()
{
}

void fsl_ui_draw(fsl_texture *texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint)
{
    glUniform2i(fsl_ui_core.uniform.ui.position, pos_x, pos_y);
    glUniform2i(fsl_ui_core.uniform.ui.size, size_x, size_y);
    glUniform2i(fsl_ui_core.uniform.ui.texture_size, texture->size.x, texture->size.y);
    glUniform2f(fsl_ui_core.uniform.ui.offset, offset_x, offset_y);
    glUniform2i(fsl_ui_core.uniform.ui.alignment, align_x, align_y);
    glUniform4f(fsl_ui_core.uniform.ui.tint,
            (f32)((tint >> 0x18) & 0xff) / 0xff,
            (f32)((tint >> 0x10) & 0xff) / 0xff,
            (f32)((tint >> 0x08) & 0xff) / 0xff,
            (f32)((tint >> 0x00) & 0xff) / 0xff);

    glBindVertexArray(fsl_ui_core.vao);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void fsl_ui_draw_nine_slice(fsl_texture *texture, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size, u32 tint)
{
    fsl_panel_nine_slice _panel = fsl_get_nine_slice(texture->size,
            pos_x, pos_y,
            size_x, size_y, slice_size);

    glUniform4f(fsl_ui_core.uniform.nine_slice.tint,
            (f32)((tint >> 0x18) & 0xff) / 0xff,
            (f32)((tint >> 0x10) & 0xff) / 0xff,
            (f32)((tint >> 0x08) & 0xff) / 0xff,
            (f32)((tint >> 0x00) & 0xff) / 0xff);

    glBindBuffer(GL_ARRAY_BUFFER, fsl_ui_core.vbo_nine_slice);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fsl_panel_nine_slice), &_panel);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(fsl_ui_core.vao);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 9);
}

void fsl_ui_stop(void)
{
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void fsl_ui_fbo_blit(GLuint fbo)
{
    glUseProgram(fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, fsl_ui_core.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void fsl_ui_free(void)
{
    if (fsl_ui_core.vao_loaded)
    {
        glDeleteVertexArrays(1, &fsl_ui_core.vao);
        glDeleteBuffers(1, &fsl_ui_core.vbo_unit_quad);
        glDeleteBuffers(1, &fsl_ui_core.vbo_nine_slice);
    }

    fsl_fbo_free(&fsl_ui_core.fbo);
}

fsl_panel_nine_slice fsl_get_nine_slice(v2i32 texture_size, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size)
{
    f32 _pos_x = (f32)pos_x;
    f32 _pos_y = (f32)pos_y;
    f32 _size_x = (f32)size_x;
    f32 _size_y = (f32)size_y;
    f32 _slice_size = (f32)slice_size;
    v2f32 _texture_scale =
    {
        1.0f / (f32)texture_size.x,
        1.0f / (f32)texture_size.y,
    };

    v2f32 _pos[3] =
    {
        [0] = {_pos_x, _pos_y},
        [1] = {_pos_x + _slice_size, _pos_y + _slice_size},
        [2] = {_pos_x + _size_x - _slice_size, _pos_y + _size_y - _slice_size},
    };

    v2f32 _size[3] =
    {
        [0] = {_slice_size, _slice_size},
        [1] = {_size_x - _slice_size * 2, _size_y - _slice_size * 2},
        [2] = {_slice_size, _slice_size},
    };

    v2f32 _tex_coords_pos[3] =
    {
        [0] = {0.0f, 0.0f},
        [1] = {_texture_scale.x * _slice_size, _texture_scale.y * _slice_size},
        [2] = {1.0f - _texture_scale.x * _slice_size, 1.0f - _texture_scale.y * _slice_size},
    };

    v2f32 _tex_coords_size[3] =
    {
        [0] = {_texture_scale.x * _slice_size, _texture_scale.y * _slice_size},
        [1] = {1.0f - _texture_scale.x * _slice_size * 2.0f, 1.0f - _texture_scale.y * _slice_size * 2.0f},
        [2] = {_texture_scale.x * _slice_size, _texture_scale.y * _slice_size},
    };

    return (fsl_panel_nine_slice){
        .slice[0] =
        {
            .pos.x = _pos[0].x,
            .pos.y = _pos[0].y,
            .size.x = _size[0].x,
            .size.y = _size[0].y,
            .tex_coords_pos.x = _tex_coords_pos[0].x,
            .tex_coords_pos.y = _tex_coords_pos[0].y,
            .tex_coords_size.x = _tex_coords_size[0].x,
            .tex_coords_size.y = _tex_coords_size[0].y,
        },

        .slice[1] =
        {
            .pos.x = _pos[1].x,
            .pos.y = _pos[0].y,
            .size.x = _size[1].x,
            .size.y = _size[0].y,
            .tex_coords_pos.x = _tex_coords_pos[1].x,
            .tex_coords_pos.y = _tex_coords_pos[0].y,
            .tex_coords_size.x = _tex_coords_size[1].x,
            .tex_coords_size.y = _tex_coords_size[0].y,
        },

        .slice[2] =
        {
            .pos.x = _pos[2].x,
            .pos.y = _pos[0].y,
            .size.x = _size[2].x,
            .size.y = _size[0].y,
            .tex_coords_pos.x = _tex_coords_pos[2].x,
            .tex_coords_pos.y = _tex_coords_pos[0].y,
            .tex_coords_size.x = _tex_coords_size[2].x,
            .tex_coords_size.y = _tex_coords_size[0].y,
        },

        .slice[3] =
        {
            .pos.x = _pos[0].x,
            .pos.y = _pos[1].y,
            .size.x = _size[0].x,
            .size.y = _size[1].y,
            .tex_coords_pos.x = _tex_coords_pos[0].x,
            .tex_coords_pos.y = _tex_coords_pos[1].y,
            .tex_coords_size.x = _tex_coords_size[0].x,
            .tex_coords_size.y = _tex_coords_size[1].y,
        },

        .slice[4] =
        {
            .pos.x = _pos[1].x,
            .pos.y = _pos[1].y,
            .size.x = _size[1].x,
            .size.y = _size[1].y,
            .tex_coords_pos.x = _tex_coords_pos[1].x,
            .tex_coords_pos.y = _tex_coords_pos[1].y,
            .tex_coords_size.x = _tex_coords_size[1].x,
            .tex_coords_size.y = _tex_coords_size[1].y,
        },

        .slice[5] =
        {
            .pos.x = _pos[2].x,
            .pos.y = _pos[1].y,
            .size.x = _size[2].x,
            .size.y = _size[1].y,
            .tex_coords_pos.x = _tex_coords_pos[2].x,
            .tex_coords_pos.y = _tex_coords_pos[1].y,
            .tex_coords_size.x = _tex_coords_size[2].x,
            .tex_coords_size.y = _tex_coords_size[1].y,
        },

        .slice[6] =
        {
            .pos.x = _pos[0].x,
            .pos.y = _pos[2].y,
            .size.x = _size[0].x,
            .size.y = _size[2].y,
            .tex_coords_pos.x = _tex_coords_pos[0].x,
            .tex_coords_pos.y = _tex_coords_pos[2].y,
            .tex_coords_size.x = _tex_coords_size[0].x,
            .tex_coords_size.y = _tex_coords_size[2].y,
        },

        .slice[7] =
        {
            .pos.x = _pos[1].x,
            .pos.y = _pos[2].y,
            .size.x = _size[1].x,
            .size.y = _size[2].y,
            .tex_coords_pos.x = _tex_coords_pos[1].x,
            .tex_coords_pos.y = _tex_coords_pos[2].y,
            .tex_coords_size.x = _tex_coords_size[1].x,
            .tex_coords_size.y = _tex_coords_size[2].y,
        },

        .slice[8] =
        {
            .pos.x = _pos[2].x,
            .pos.y = _pos[2].y,
            .size.x = _size[2].x,
            .size.y = _size[2].y,
            .tex_coords_pos.x = _tex_coords_pos[2].x,
            .tex_coords_pos.y = _tex_coords_pos[2].y,
            .tex_coords_size.x = _tex_coords_size[2].x,
            .tex_coords_size.y = _tex_coords_size[2].y,
        },
    };
}
