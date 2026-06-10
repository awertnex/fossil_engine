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
 *  @file ui.c
 *
 *  @brief initializing UI module.
 */

#include "../common/config.h"
#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../common/types.h"
#include "../assets/assets.h"
#include "../assets/asset_types.h"
#include "../engine/engine.h"
#include "../engine/engine_assets.h"
#include "../logger/logger.h"
#include "../logger/logger_messages_internal.h"
#include "../memory/memory.h"
#include "../math/vector.h"
#include "../shaders/shader_types.h"

#include "ui_core.h"
#include "ui_types.h"

#include <string.h>

/* ---- section: definitions ------------------------------------------------ */

struct fsl_glyphf
{
    v2f32 scale;
    v2f32 bearing;
    f32 advance;
    b8 loaded;
}; /* fsl_glyphf */

struct fsl_text_data
{
    v2f32 pos;
    u32 char_index;
    u32 color;
}; /* fsl_text_data */

static struct text_core
{
    fsl_font *font;
    struct fsl_glyphf glyph[FSL_GLYPH_MAX];
    f32 font_size;
    f32 line_height_total;
    f32 advance;
    v2f32 text_scale;

    /*!
     *  @brief iterator for `buf`, resets at @ref fsl_text_start() and @ref fsl_text_render().
     */
    GLuint cursor;

    /*!
     *  @brief text buffer, raw text data.
    */
    struct fsl_text_data *buf;

    u64 buf_len;
    GLuint vao;
    GLuint vbo_unit_quad;
    GLuint vbo_text_data;
    b8 vao_loaded;
} text_core;

static struct ui_core
{
    GLuint vao;
    GLuint vbo_unit_quad;
    GLuint vbo_nine_slice;
    b8 vao_loaded;

    /*!
     *  @brief panel buffer, raw panel data.
     */
    fsl_panel_nine_slice *panel_buf;

    u64 panel_count;

    struct /* shader */
    {
        fsl_shader_program *text;
        fsl_shader_program *ui;
        fsl_shader_program *ui_9_slice;
    } shader;

    struct /* uniform */
    {
        struct /* text */
        {
            GLint font_size;
            GLint draw_shadow;
            GLint shadow_color;
            GLint shadow_offset;
        } text;

        struct /* ui */
        {
            GLint position;
            GLint size;
            GLint texture_size;
            GLint offset;
            GLint alignment;
            GLint uv_pos; /* temp, for testing */
            GLint uv_size; /* temp, for testing */
            GLint tint;
        } ui;

        struct /* nine_slice */
        {
            GLint tint;
        } nine_slice;

    } uniform;

} ui_core;

/* ---- section: declarations ----------------------------------------------- */

static f32 vbo_data_unit_quad_internal[] =
{
    0.0f, -1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f
};

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @brief init text rendering settings (and engine default fonts at @ref fsl_font_buf).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
static u32 text_init_internal(void);

static void text_align_x_none_internal(const str *text, i64 i, f32 advance);
static void text_align_x_center_internal(const str *text, i64 i, f32 advance);
static void text_align_x_right_internal(const str *text, i64 i, f32 advance);
static void text_align_y_none_internal(u64 end, f32 height);
static void text_align_y_center_internal(u64 end, f32 height);
static void text_align_y_bottom_internal(u64 end, f32 height);

/* ---- section: text ------------------------------------------------------- */

static u32 text_init_internal(void)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(fsl_shader_buf);

    if (fsl_mem_map((void*)&text_core.buf, FSL_STRING_MAX * sizeof(struct fsl_text_data),
                "text_init_internal().text_core.buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    text_core.buf_len = FSL_STRING_MAX;

    if (!text_core.vao_loaded)
    {
        glGenVertexArrays(1, &text_core.vao);
        glBindVertexArray(text_core.vao);

        /* ---- unit quad --------------------------------------------------- */

        glGenBuffers(1, &text_core.vbo_unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo_unit_quad);
        glBufferData(GL_ARRAY_BUFFER, fsl_arr_len(vbo_data_unit_quad_internal) * sizeof(f32),
                &vbo_data_unit_quad_internal, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

        /* ---- text data --------------------------------------------------- */

        glGenBuffers(1, &text_core.vbo_text_data);
        glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, text_core.buf_len * sizeof(struct fsl_text_data),
                NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct fsl_text_data), (void*)0);
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT,
                sizeof(struct fsl_text_data), (void*)(2 * sizeof(f32)));
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4);
        glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT,
                sizeof(struct fsl_text_data), (void*)(2 * sizeof(f32) + sizeof(u32)));
        glVertexAttribDivisor(4, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    ui_core.shader.text = &shader_p[FSL_SHADER_INDEX_TEXT];
    ui_core.uniform.text.font_size = glGetUniformLocation(ui_core.shader.text->asset.id, "font_size");
    ui_core.uniform.text.draw_shadow = glGetUniformLocation(ui_core.shader.text->asset.id, "draw_shadow");
    ui_core.uniform.text.shadow_color = glGetUniformLocation(ui_core.shader.text->asset.id, "shadow_color");
    ui_core.uniform.text.shadow_offset = glGetUniformLocation(ui_core.shader.text->asset.id, "shadow_offset");

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_ui_free();
    LOGFATAL(FSL_ERR_TEXT_INIT_FAIL,
            FSL_FLAG_LOG_NO_VERBOSE,
            MSG_ACTION_FATAL("Initialize Text"));
    return fsl_err;
}

void fsl_text_start(fsl_font *font, f32 size, u64 length, b8 clear)
{
    static v2i32 render_size = {0};
    f32 scale = 0.0f;
    struct fsl_glyphf *g = NULL;
    u32 i = 0;

    fsl_fbo_bind();

    if (!length)
        length = FSL_STRING_MAX;
    else if (length > text_core.buf_len)
    {
        if (
                fsl_mem_remap((void*)&text_core.buf,
                    text_core.buf_len * sizeof(struct fsl_text_data),
                    length * sizeof(struct fsl_text_data),
                    "fsl_text_start().text_core.buf") != FSL_ERR_SUCCESS)
            goto cleanup;

        text_core.buf_len = length;

        glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, text_core.buf_len * sizeof(struct fsl_text_data),
                NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (text_core.font == NULL || font->asset.id != text_core.font->asset.id ||
            render_size.x != render_internal.size.x || render_size.y != render_internal.size.y)
    {
        text_core.font = font;
        text_core.font_size = size;
        text_core.line_height_total = 0.0f;
        text_core.advance = 0.0f;
        text_core.cursor = 0;

        scale = size / font->fheight;
        text_core.text_scale.x = scale * render_internal.ndc_scale.x;
        text_core.text_scale.y = scale * render_internal.ndc_scale.y;

        for (i = 0; i < FSL_GLYPH_MAX; ++i)
        {
            if (!text_core.font->glyph[i].loaded)
                continue;
            g = &text_core.glyph[i];

            g->bearing.x = text_core.font->glyph[i].bearing.x * text_core.text_scale.x;
            g->bearing.y = text_core.font->glyph[i].bearing.y * text_core.text_scale.y;
            g->advance = text_core.font->glyph[i].advance * text_core.text_scale.x;
            g->loaded = TRUE;
        }
    }

    ui_core.shader.text = fsl_mem_handle_get(fsl_shader_buf);
    ui_core.shader.text = &ui_core.shader.text[FSL_SHADER_INDEX_TEXT];
    glUseProgram(ui_core.shader.text->asset.id);
    glUniform2f(ui_core.uniform.text.font_size,
            text_core.font_size * render_internal.ndc_scale.x,
            text_core.font_size * render_internal.ndc_scale.y);

    glBindTexture(GL_TEXTURE_2D, text_core.font->asset.id);
    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
    return;

cleanup:

    LOGERROR(fsl_err,
            FSL_FLAG_LOG_NO_VERBOSE,
            MSG_ACTION_ERROR("Start Text"));
}

void fsl_text_push(const str *text, f32 pos_x, f32 pos_y, i8 align_x, i8 align_y,
        i32 window_x, u32 color)
{
    u64 len = 0, i = 0;
    f32 descent = 0.0f, line_height = 0.0f;
    struct fsl_glyphf *g = NULL;
    f32 _window_x = (f32)window_x * render_internal.ndc_scale.x;
    void (*align_x_func)(const str *, i64, f32) = &text_align_x_none_internal;
    void (*align_y_func)(u64, f32) = &text_align_y_none_internal;

    if (!text_core.buf)
    {
        LOGERROR(FSL_ERR_BUFFER_EMPTY,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Push Text", "`text_core.buf` `NULL`"));
        return;
    }

    len = strlen(text);
    if (len >= FSL_STRING_MAX)
    {
        LOGERROR(FSL_ERR_STRING_TOO_LONG,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Push Text", "Text Too Long"));
        return;
    }

    if (text_core.cursor + len >= text_core.buf_len)
    {
        if (
                fsl_mem_remap((void*)&text_core.buf,
                    text_core.buf_len * sizeof(struct fsl_text_data),
                    (text_core.buf_len + FSL_STRING_MAX) * sizeof(struct fsl_text_data),
                    "fsl_text_push().text_core.buf") != FSL_ERR_SUCCESS)
        {
            LOGERROR(fsl_err,
                    FSL_FLAG_LOG_NO_VERBOSE,
                    MSG_ACTION_REASON_ERROR("Push Text", "`fsl_mem_remap()` Failed"));
            return;
        }

        text_core.buf_len += FSL_STRING_MAX;

        glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, text_core.buf_len * sizeof(struct fsl_text_data),
                NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (align_x == FSL_TEXT_ALIGN_CENTER)
        align_x_func = &text_align_x_center_internal;
    else if (align_x == FSL_TEXT_ALIGN_RIGHT)
        align_x_func = &text_align_x_right_internal;

    if (align_y == FSL_TEXT_ALIGN_CENTER)
        align_y_func = &text_align_y_center_internal;
    else if (align_y == FSL_TEXT_ALIGN_BOTTOM)
        align_y_func = &text_align_y_bottom_internal;

    pos_x *= render_internal.ndc_scale.x;
    pos_y *= render_internal.ndc_scale.y;
    pos_y += text_core.font->scale.y * text_core.text_scale.y;

    descent = text_core.font->descent * text_core.text_scale.y;
    line_height = text_core.font->line_height;
    for (i = 0; i < len; ++i)
    {
        g = &text_core.glyph[(u64)text[i]];

        if (text[i] == '\n' || text[i] == '\r')
        {
            align_x_func(text, (i64)i, text_core.advance);

            text_core.advance = 0.0f;
            if (text[i] == '\n')
                text_core.line_height_total += line_height * text_core.text_scale.y;
            continue;
        }
        else if (text[i] == '\t')
        {
            text_core.advance += text_core.glyph[' '].advance * FSL_TEXT_TAB_SIZE;
            continue;
        }

        if (window_x && text_core.advance + g->advance >= _window_x)
        {
            align_x_func(text, (i64)i, text_core.advance);

            text_core.advance = 0.0f;
            text_core.line_height_total += line_height * text_core.text_scale.y;
        }

        text_core.buf[text_core.cursor].pos.x =
            pos_x + text_core.advance + g->bearing.x;

        text_core.buf[text_core.cursor].pos.y =
            -pos_y - descent - text_core.line_height_total - g->bearing.y;

        text_core.buf[text_core.cursor].char_index = (u32)text[i];
        text_core.buf[text_core.cursor].color = color;

        ++text_core.cursor;

        text_core.advance += g->advance;
    }

    align_y_func(text_core.cursor, text_core.line_height_total);
}

static void text_align_x_center_internal(const str *text, i64 i, f32 advance)
{
    i64 j = 1;
    advance *= 0.5f;
    for (; i - j >= 0 && text[i - j] != '\n' && text[i - j] != '\r'; ++j)
        text_core.buf[text_core.cursor - j].pos.x -= advance;
}

static void text_align_x_right_internal(const str *text, i64 i, f32 advance)
{
    i64 j = 1;
    for (; i - j >= 0 && text[i - j] != '\n' && text[i - j] != '\r'; ++j)
        text_core.buf[text_core.cursor - j].pos.x -= advance;
}

static void text_align_x_none_internal(const str *text, i64 i, f32 advance)
{
    (void)text;
    (void)i;
    (void)advance;
    return;
}

static void text_align_y_center_internal(u64 end, f32 height)
{
    u64 i = 0;
    height *= 0.5f;
    for (; i < end; ++i)
        text_core.buf[i].pos.y += height;
}

static void text_align_y_bottom_internal(u64 end, f32 height)
{
    u64 i = 0;
    for (; i < end; ++i)
        text_core.buf[i].pos.y += height;
}

static void text_align_y_none_internal(u64 end, f32 height)
{
    (void)end;
    (void)height;
    return;
}

void fsl_text_render(b8 shadow, u32 shadow_color)
{
    if (!text_core.buf)
    {
        LOGERROR(FSL_ERR_BUFFER_EMPTY,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Render Text", "`text_core.buf` `NULL`"));
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo_text_data);
    glBufferSubData(GL_ARRAY_BUFFER, 0, text_core.cursor * sizeof(struct fsl_text_data),
            text_core.buf);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(text_core.vao);

    if (shadow)
    {
        glUniform1i(ui_core.uniform.text.draw_shadow, TRUE);
        glUniform4f(ui_core.uniform.text.shadow_color,
                (f32)((shadow_color >> 0x18) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x10) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x08) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x00) & 0xff) / 0xff);
        glUniform2f(ui_core.uniform.text.shadow_offset, FSL_TEXT_OFFSET_SHADOW, FSL_TEXT_OFFSET_SHADOW);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, text_core.cursor);
    }

    glUniform1i(ui_core.uniform.text.draw_shadow, FALSE);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, text_core.cursor);

    text_core.cursor = 0;
    text_core.line_height_total = 0;
    text_core.advance = 0.0f;
}

f32 fsl_get_text_height(void)
{
    return text_core.line_height_total * text_core.text_scale.y;
}

/* ---- section: ui --------------------------------------------------------- */

u32 fsl_ui_init(void)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(fsl_shader_buf);

    if (text_init_internal() != FSL_ERR_SUCCESS)
        return fsl_err;

    ui_core.shader.ui = &shader_p[FSL_SHADER_INDEX_UI];
    ui_core.shader.ui_9_slice = &shader_p[FSL_SHADER_INDEX_UI_9_SLICE];

    if (!ui_core.vao_loaded)
    {
        glGenVertexArrays(1, &ui_core.vao);
        glBindVertexArray(ui_core.vao);

        /* ---- unit quad --------------------------------------------------- */

        glGenBuffers(1, &ui_core.vbo_unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, ui_core.vbo_unit_quad);
        glBufferData(GL_ARRAY_BUFFER, fsl_arr_len(vbo_data_unit_quad_internal) * sizeof(f32),
                &vbo_data_unit_quad_internal, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

        /* ---- nine-slice data --------------------------------------------- */

        glGenBuffers(1, &ui_core.vbo_nine_slice);
        glBindBuffer(GL_ARRAY_BUFFER, ui_core.vbo_nine_slice);
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

        ui_core.vao_loaded = TRUE;
    }

    ui_core.uniform.ui.position =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "position");
    ui_core.uniform.ui.size =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "size");
    ui_core.uniform.ui.uv_pos =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "uv_pos");
    ui_core.uniform.ui.uv_size =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "uv_size");
    ui_core.uniform.ui.texture_size =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "texture_size");
    ui_core.uniform.ui.offset =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "offset");
    ui_core.uniform.ui.alignment =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "alignment");
    ui_core.uniform.ui.tint =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "tint");

    ui_core.uniform.nine_slice.tint =
        glGetUniformLocation(ui_core.shader.ui_9_slice->asset.id, "tint");

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_ui_start(b8 nine_slice, b8 clear)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(fsl_shader_buf);
    fsl_fbo_bind();

    if (nine_slice)
    {
        ui_core.shader.ui_9_slice = &shader_p[FSL_SHADER_INDEX_UI_9_SLICE];
        glUseProgram(ui_core.shader.ui_9_slice->asset.id);
    }
    else
    {
        ui_core.shader.ui = &shader_p[FSL_SHADER_INDEX_UI];
        glUseProgram(ui_core.shader.ui->asset.id);
    }

    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
}

void fsl_ui_push_panel(i32 pos_x, i32 pos_y, i32 size_x, i32 size_y, u32 tint)
{
    fsl_texture *texture = fsl_mem_handle_get(fsl_texture_buf);
    fsl_panel_nine_slice _panel = fsl_get_nine_slice(&texture[FSL_TEXTURE_INDEX_PANEL_ACTIVE],
            pos_x, pos_y, size_x, size_y, FSL_UI_SLICE_SIZE_DEFAULT);

    glBindBuffer(GL_ARRAY_BUFFER, ui_core.vbo_nine_slice);
    glBufferData(GL_ARRAY_BUFFER, ui_core.panel_count * sizeof(fsl_panel_nine_slice),
            ui_core.panel_buf, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void ui_element_draw_internal(fsl_ui_element *element)
{
    glUniform2f(ui_core.uniform.ui.position,
            element->sprite.pos_baked.x,
            element->sprite.pos_baked.y);
    glUniform2f(ui_core.uniform.ui.size,
            element->sprite.size_baked.x,
            element->sprite.size_baked.y);
    glUniform2f(ui_core.uniform.ui.uv_pos,
            element->sprite.uv_pos_baked.x,
            element->sprite.uv_pos_baked.y);
    glUniform2f(ui_core.uniform.ui.uv_size,
            element->sprite.uv_size_baked.x,
            element->sprite.uv_size_baked.y);
    glUniform4f(ui_core.uniform.ui.tint, 1.0f, 1.0f, 1.0f, 1.0f);

    glBindVertexArray(ui_core.vao);
    glBindTexture(GL_TEXTURE_2D, element->texture->asset.id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void fsl_ui_draw(fsl_texture *texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint)
{
    if (!size_x) size_x = texture->size.x;
    if (!size_y) size_y = texture->size.y;

    glUniform2i(ui_core.uniform.ui.position, pos_x, pos_y);
    glUniform2i(ui_core.uniform.ui.size, size_x, size_y);
    glUniform2i(ui_core.uniform.ui.texture_size, texture->size.x, texture->size.y);
    glUniform2f(ui_core.uniform.ui.offset, offset_x, offset_y);
    glUniform2i(ui_core.uniform.ui.alignment, align_x, align_y);
    glUniform4f(ui_core.uniform.ui.tint,
            (f32)((tint >> 0x18) & 0xff) / 0xff,
            (f32)((tint >> 0x10) & 0xff) / 0xff,
            (f32)((tint >> 0x08) & 0xff) / 0xff,
            (f32)((tint >> 0x00) & 0xff) / 0xff);

    glBindVertexArray(ui_core.vao);
    glBindTexture(GL_TEXTURE_2D, texture->asset.id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void fsl_ui_draw_nine_slice(fsl_texture *texture, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size, u32 tint)
{
    fsl_panel_nine_slice _panel = fsl_get_nine_slice(texture, pos_x, pos_y, size_x, size_y, slice_size);

    glUniform4f(ui_core.uniform.nine_slice.tint,
            (f32)((tint >> 0x18) & 0xff) / 0xff,
            (f32)((tint >> 0x10) & 0xff) / 0xff,
            (f32)((tint >> 0x08) & 0xff) / 0xff,
            (f32)((tint >> 0x00) & 0xff) / 0xff);

    glBindBuffer(GL_ARRAY_BUFFER, ui_core.vbo_nine_slice);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fsl_panel_nine_slice), &_panel);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(ui_core.vao);
    glBindTexture(GL_TEXTURE_2D, texture->asset.id);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, 9);
}

void fsl_ui_stop(void)
{
    glBindVertexArray(0);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void fsl_ui_free(void)
{
    if (text_core.vao_loaded)
    {
        glDeleteVertexArrays(1, &text_core.vao);
        glDeleteBuffers(1, &text_core.vbo_unit_quad);
        glDeleteBuffers(1, &text_core.vbo_text_data);
    }

    fsl_mem_unmap((void*)&text_core.buf, text_core.buf_len * sizeof(struct fsl_text_data),
            "fsl_ui_free().text_core.buf");

    if (ui_core.vao_loaded)
    {
        glDeleteVertexArrays(1, &ui_core.vao);
        glDeleteBuffers(1, &ui_core.vbo_unit_quad);
        glDeleteBuffers(1, &ui_core.vbo_nine_slice);
    }
}

fsl_panel_nine_slice fsl_get_nine_slice(fsl_texture *texture, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size)
{
    fsl_panel_nine_slice panel = {0};
    f32 _pos_x = (f32)pos_x;
    f32 _pos_y = (f32)pos_y;
    f32 _size_x = (f32)size_x;
    f32 _size_y = (f32)size_y;
    f32 _slice_size = (f32)slice_size;
    v2f32 _texture_scale = {0};
    v2f32 _pos[3] = {0};
    v2f32 _size[3] = {0};
    v2f32 _tex_coords_pos[3] = {0};
    v2f32 _tex_coords_size[3] = {0};

    _texture_scale.x = 1.0f / (f32)texture->size.x;
    _texture_scale.y = 1.0f / (f32)texture->size.y;

    _pos[0].x = _pos_x;
    _pos[0].y = _pos_y;
    _pos[1].x = _pos_x + _slice_size;
    _pos[1].y = _pos_y + _slice_size;
    _pos[2].x = _pos_x + _size_x - _slice_size;
    _pos[2].y = _pos_y + _size_y - _slice_size;

    _size[0].x = _slice_size;
    _size[0].y = _slice_size;
    _size[1].x = _size_x - _slice_size * 2;
    _size[1].y = _size_y - _slice_size * 2;
    _size[2].x = _slice_size;
    _size[2].y = _slice_size;

    _tex_coords_pos[0].x = 0.0f;
    _tex_coords_pos[0].y = 0.0f;
    _tex_coords_pos[1].x = _texture_scale.x * _slice_size;
    _tex_coords_pos[1].y = _texture_scale.y * _slice_size;
    _tex_coords_pos[2].x = 1.0f - _texture_scale.x * _slice_size;
    _tex_coords_pos[2].y = 1.0f - _texture_scale.y * _slice_size;

    _tex_coords_size[0].x = _texture_scale.x * _slice_size;
    _tex_coords_size[0].y = _texture_scale.y * _slice_size;
    _tex_coords_size[1].x = 1.0f - _texture_scale.x * _slice_size * 2.0f;
    _tex_coords_size[1].y = 1.0f - _texture_scale.y * _slice_size * 2.0f;
    _tex_coords_size[2].x = _texture_scale.x * _slice_size;
    _tex_coords_size[2].y = _texture_scale.y * _slice_size;

    panel.slice[0].pos.x = _pos[0].x;
    panel.slice[0].pos.y = _pos[0].y;
    panel.slice[0].size.x = _size[0].x;
    panel.slice[0].size.y = _size[0].y;
    panel.slice[0].tex_coords_pos.x = _tex_coords_pos[0].x;
    panel.slice[0].tex_coords_pos.y = _tex_coords_pos[0].y;
    panel.slice[0].tex_coords_size.x = _tex_coords_size[0].x;
    panel.slice[0].tex_coords_size.y = _tex_coords_size[0].y;

    panel.slice[1].pos.x = _pos[1].x;
    panel.slice[1].pos.y = _pos[0].y;
    panel.slice[1].size.x = _size[1].x;
    panel.slice[1].size.y = _size[0].y;
    panel.slice[1].tex_coords_pos.x = _tex_coords_pos[1].x;
    panel.slice[1].tex_coords_pos.y = _tex_coords_pos[0].y;
    panel.slice[1].tex_coords_size.x = _tex_coords_size[1].x;
    panel.slice[1].tex_coords_size.y = _tex_coords_size[0].y;

    panel.slice[2].pos.x = _pos[2].x;
    panel.slice[2].pos.y = _pos[0].y;
    panel.slice[2].size.x = _size[2].x;
    panel.slice[2].size.y = _size[0].y;
    panel.slice[2].tex_coords_pos.x = _tex_coords_pos[2].x;
    panel.slice[2].tex_coords_pos.y = _tex_coords_pos[0].y;
    panel.slice[2].tex_coords_size.x = _tex_coords_size[2].x;
    panel.slice[2].tex_coords_size.y = _tex_coords_size[0].y;

    panel.slice[3].pos.x = _pos[0].x;
    panel.slice[3].pos.y = _pos[1].y;
    panel.slice[3].size.x = _size[0].x;
    panel.slice[3].size.y = _size[1].y;
    panel.slice[3].tex_coords_pos.x = _tex_coords_pos[0].x;
    panel.slice[3].tex_coords_pos.y = _tex_coords_pos[1].y;
    panel.slice[3].tex_coords_size.x = _tex_coords_size[0].x;
    panel.slice[3].tex_coords_size.y = _tex_coords_size[1].y;

    panel.slice[4].pos.x = _pos[1].x;
    panel.slice[4].pos.y = _pos[1].y;
    panel.slice[4].size.x = _size[1].x;
    panel.slice[4].size.y = _size[1].y;
    panel.slice[4].tex_coords_pos.x = _tex_coords_pos[1].x;
    panel.slice[4].tex_coords_pos.y = _tex_coords_pos[1].y;
    panel.slice[4].tex_coords_size.x = _tex_coords_size[1].x;
    panel.slice[4].tex_coords_size.y = _tex_coords_size[1].y;

    panel.slice[5].pos.x = _pos[2].x;
    panel.slice[5].pos.y = _pos[1].y;
    panel.slice[5].size.x = _size[2].x;
    panel.slice[5].size.y = _size[1].y;
    panel.slice[5].tex_coords_pos.x = _tex_coords_pos[2].x;
    panel.slice[5].tex_coords_pos.y = _tex_coords_pos[1].y;
    panel.slice[5].tex_coords_size.x = _tex_coords_size[2].x;
    panel.slice[5].tex_coords_size.y = _tex_coords_size[1].y;

    panel.slice[6].pos.x = _pos[0].x;
    panel.slice[6].pos.y = _pos[2].y;
    panel.slice[6].size.x = _size[0].x;
    panel.slice[6].size.y = _size[2].y;
    panel.slice[6].tex_coords_pos.x = _tex_coords_pos[0].x;
    panel.slice[6].tex_coords_pos.y = _tex_coords_pos[2].y;
    panel.slice[6].tex_coords_size.x = _tex_coords_size[0].x;
    panel.slice[6].tex_coords_size.y = _tex_coords_size[2].y;

    panel.slice[7].pos.x = _pos[1].x;
    panel.slice[7].pos.y = _pos[2].y;
    panel.slice[7].size.x = _size[1].x;
    panel.slice[7].size.y = _size[2].y;
    panel.slice[7].tex_coords_pos.x = _tex_coords_pos[1].x;
    panel.slice[7].tex_coords_pos.y = _tex_coords_pos[2].y;
    panel.slice[7].tex_coords_size.x = _tex_coords_size[1].x;
    panel.slice[7].tex_coords_size.y = _tex_coords_size[2].y;

    panel.slice[8].pos.x = _pos[2].x;
    panel.slice[8].pos.y = _pos[2].y;
    panel.slice[8].size.x = _size[2].x;
    panel.slice[8].size.y = _size[2].y;
    panel.slice[8].tex_coords_pos.x = _tex_coords_pos[2].x;
    panel.slice[8].tex_coords_pos.y = _tex_coords_pos[2].y;
    panel.slice[8].tex_coords_size.x = _tex_coords_size[2].x;
    panel.slice[8].tex_coords_size.y = _tex_coords_size[2].y;

    return panel;
}
