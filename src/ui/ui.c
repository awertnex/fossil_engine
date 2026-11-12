/**
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

/**
 *  @file ui.c
 *
 *  @brief initializing UI module and drawing things on screen.
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

#include "ui.h"
#include "ui_internal.h"
#include "ui_types.h"

#include <string.h>

/* ---- section: definitions ------------------------------------------------ */

struct glyphf
{
    v2f32 scale;
    v2f32 bearing;
    f32 advance;
    b8 loaded;
}; /* glyphf */

struct glyph_render_data
{
    v2f32 pos;
    u32 char_index;
    u32 color;
}; /* glyph_render_data */

static struct text_core
{
    fsl_font *font;
    struct glyphf glyph[FSL_GLYPH_MAX];
    f32 font_size;
    f32 line_height_total;
    f32 advance;
    v2f32 text_scale;

    /**
     *  @brief iterator for `buf`, resets at @ref fsl_text_start() and
     *  @ref fsl_text_render().
     */
    GLuint cursor;

    /**
     *  @brief glyph buffer, raw glyph data.
     *
     *  stores render data of a text batch.
     */
    struct glyph_render_data *buf;

    u64 buf_len;
    GLuint vao;
    GLuint vertex_data_unit_quad;
    GLuint vbo_text_data;
    b8 vao_loaded;
} text_core = {0};

static struct ui_core
{
    GLuint vao;
    GLuint vertex_data_unit_quad;
    GLuint vertex_data_curr_quad;
    b8 vao_loaded;

    /**
     *  @brief quad buffer, raw UI quad data.
     */
    fsl_ui_transform_baked *quad_buf;

    u64 quad_count;

    struct /* shader */
    {
        fsl_shader_program *text;
        fsl_shader_program *ui;
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
            GLint tint;
        } ui;

    } uniform;

} ui_core = {0};

/* ---- section: declarations ----------------------------------------------- */

static f32 vertex_buf_ui_unit_quad[] =
{
    0.0f, -1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f
};

static void ui_text_align_x_none_func(const str *text, i64 i, f32 advance);
static void ui_text_align_x_center_func(const str *text, i64 i, f32 advance);
static void ui_text_align_x_right_func(const str *text, i64 i, f32 advance);
static void ui_text_align_y_none_func(u64 end, f32 height);
static void ui_text_align_y_center_func(u64 end, f32 height);
static void ui_text_align_y_bottom_func(u64 end, f32 height);

/* ---- section: text ------------------------------------------------------- */

u32 ui_text_init_internal(void)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(fsl_shader_buf);

    if (fsl_mem_map((void*)&text_core.buf, FSL_STRING_MAX * sizeof(struct glyph_render_data),
                "ui_text_init_internal().text_core.buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    text_core.buf_len = FSL_STRING_MAX;

    if (!text_core.vao_loaded)
    {
        glGenVertexArrays(1, &text_core.vao);
        glBindVertexArray(text_core.vao);

        /* ---- unit quad --------------------------------------------------- */

        glGenBuffers(1, &text_core.vertex_data_unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, text_core.vertex_data_unit_quad);
        glBufferData(GL_ARRAY_BUFFER, fsl_arr_len(vertex_buf_ui_unit_quad) * sizeof(f32),
                &vertex_buf_ui_unit_quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

        /* ---- text data --------------------------------------------------- */

        glGenBuffers(1, &text_core.vbo_text_data);
        glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, text_core.buf_len * sizeof(struct glyph_render_data),
                NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct glyph_render_data), (void*)0);
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3);
        glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT,
                sizeof(struct glyph_render_data), (void*)(2 * sizeof(f32)));
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4);
        glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT,
                sizeof(struct glyph_render_data), (void*)(2 * sizeof(f32) + sizeof(u32)));
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
    struct glyphf *g = NULL;
    u32 i = 0;

    fsl_fbo_bind();

    if (!length)
        length = FSL_STRING_MAX;
    else if (length > text_core.buf_len)
    {
        if (
                fsl_mem_remap((void*)&text_core.buf,
                    text_core.buf_len * sizeof(struct glyph_render_data),
                    length * sizeof(struct glyph_render_data),
                    "fsl_text_start().text_core.buf") != FSL_ERR_SUCCESS)
            goto cleanup;

        text_core.buf_len = length;

        glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, text_core.buf_len * sizeof(struct glyph_render_data),
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
    struct glyphf *g = NULL;
    f32 _window_x = (f32)window_x * render_internal.ndc_scale.x;
    void (*align_x_func)(const str *, i64, f32) = &ui_text_align_x_none_func;
    void (*align_y_func)(u64, f32) = &ui_text_align_y_none_func;

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
                    text_core.buf_len * sizeof(struct glyph_render_data),
                    (text_core.buf_len + FSL_STRING_MAX) * sizeof(struct glyph_render_data),
                    "fsl_text_push().text_core.buf") != FSL_ERR_SUCCESS)
        {
            LOGERROR(fsl_err,
                    FSL_FLAG_LOG_NO_VERBOSE,
                    MSG_ACTION_REASON_ERROR("Push Text", "`fsl_mem_remap()` Failed"));
            return;
        }

        text_core.buf_len += FSL_STRING_MAX;

        glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, text_core.buf_len * sizeof(struct glyph_render_data),
                NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (align_x == FSL_TEXT_ALIGN_CENTER)
        align_x_func = &ui_text_align_x_center_func;
    else if (align_x == FSL_TEXT_ALIGN_RIGHT)
        align_x_func = &ui_text_align_x_right_func;

    if (align_y == FSL_TEXT_ALIGN_CENTER)
        align_y_func = &ui_text_align_y_center_func;
    else if (align_y == FSL_TEXT_ALIGN_BOTTOM)
        align_y_func = &ui_text_align_y_bottom_func;

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
    glBufferSubData(GL_ARRAY_BUFFER, 0, text_core.cursor * sizeof(struct glyph_render_data),
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

f32 fsl_text_height_get(void)
{
    return text_core.line_height_total * text_core.text_scale.y;
}

void ui_text_align_x_center_func(const str *text, i64 i, f32 advance)
{
    i64 j = 1;
    advance *= 0.5f;
    for (; i - j >= 0 && text[i - j] != '\n' && text[i - j] != '\r'; ++j)
        text_core.buf[text_core.cursor - j].pos.x -= advance;
}

void ui_text_align_x_right_func(const str *text, i64 i, f32 advance)
{
    i64 j = 1;
    for (; i - j >= 0 && text[i - j] != '\n' && text[i - j] != '\r'; ++j)
        text_core.buf[text_core.cursor - j].pos.x -= advance;
}

void ui_text_align_x_none_func(const str *text, i64 i, f32 advance)
{
    (void)text;
    (void)i;
    (void)advance;
    return;
}

void ui_text_align_y_center_func(u64 end, f32 height)
{
    u64 i = 0;
    height *= 0.5f;
    for (; i < end; ++i)
        text_core.buf[i].pos.y += height;
}

void ui_text_align_y_bottom_func(u64 end, f32 height)
{
    u64 i = 0;
    for (; i < end; ++i)
        text_core.buf[i].pos.y += height;
}

void ui_text_align_y_none_func(u64 end, f32 height)
{
    (void)end;
    (void)height;
    return;
}


/* ---- section: ui --------------------------------------------------------- */

u32 fsl_ui_init(void)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(fsl_shader_buf);

    if (ui_text_init_internal() != FSL_ERR_SUCCESS)
        return fsl_err;

    ui_core.shader.ui = &shader_p[FSL_SHADER_INDEX_UI];

    if (!ui_core.vao_loaded)
    {
        glGenVertexArrays(1, &ui_core.vao);
        glBindVertexArray(ui_core.vao);

        /* ---- unit quad --------------------------------------------------- */

        glGenBuffers(1, &ui_core.vertex_data_unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, ui_core.vertex_data_unit_quad);
        glBufferData(GL_ARRAY_BUFFER, fsl_arr_len(vertex_buf_ui_unit_quad) * sizeof(f32),
                &vertex_buf_ui_unit_quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

        glGenBuffers(1, &ui_core.vertex_data_curr_quad);
        glBindBuffer(GL_ARRAY_BUFFER, ui_core.vertex_data_curr_quad);
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(GLfloat),
                NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                8 * sizeof(GLfloat), (void*)0);
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                8 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE,
                8 * sizeof(GLfloat), (void*)(4 * sizeof(GLfloat)));
        glVertexAttribDivisor(4, 1);

        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE,
                8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
        glVertexAttribDivisor(5, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        ui_core.vao_loaded = TRUE;
    }

    ui_core.uniform.ui.tint =
        glGetUniformLocation(ui_core.shader.ui->asset.id, "tint");

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_ui_start(b8 clear)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(fsl_shader_buf);

    fsl_fbo_bind();
    ui_core.shader.ui = &shader_p[FSL_SHADER_INDEX_UI];
    glUseProgram(ui_core.shader.ui->asset.id);

    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);
}

void fsl_ui_element_bake(fsl_ui_element *element)
{
    fsl_ui_transform *t = &element->transform;
    v2f32 uv_scale = {0};
    v2f32 size = {0};
    v2f32 size_half = {0};
    v2f32 alignment = {0};
    v2f32 pos = {0};

    element->flag &= ~FSL_FLAG_UI_DIRTY_TRANSFORM;

    if (element->texture)
    {
        uv_scale.x = 1.0f / element->texture->size.x;
        uv_scale.y = 1.0f / element->texture->size.y;

        t->baked.uv_pos.x = t->uv_pos.x * uv_scale.x;
        t->baked.uv_pos.y = t->uv_pos.y * uv_scale.y;
        t->baked.uv_size.x = t->uv_size.x * uv_scale.x;
        t->baked.uv_size.y = t->uv_size.y * uv_scale.y;
    }

    size.x = t->size.x + t->size_scaled.x * t->scale.x;
    size.y = t->size.y + t->size_scaled.y * t->scale.y;
    size_half.x = size.x / 2.0f;
    size_half.y = size.y / 2.0f;
    alignment.x = size_half.x + t->align.x * size_half.x;
    alignment.y = size_half.y + t->align.y * size_half.y;
    pos.x = t->pos.x + t->offset.x + t->offset_scaled.x * t->scale.x;
    pos.y = t->pos.y + t->offset.y + t->offset_scaled.y * t->scale.y;

    t->pos_local.x = pos.x - alignment.x;
    t->pos_local.y = pos.y - alignment.y;
    t->baked.pos.x = t->pos_local.x;
    t->baked.pos.y = t->pos_local.y;
    t->baked.size.x = size.x;
    t->baked.size.y = size.y;

    if (element->parent)
    {
        t->baked.pos.x += element->parent->transform.baked.pos.x;
        t->baked.pos.y += element->parent->transform.baked.pos.y;
    }

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_UI_ELEMENT_BAKE(t->baked.pos.x, t->baked.pos.y, t->baked.size.x, t->baked.size.y));
}

void fsl_ui_element_draw(fsl_ui_element *element)
{
    fsl_ui_element_panel_9_slice panel = {0};
    fsl_ui_transform_baked drawable = {0};
    void *buf = NULL;
    u32 buf_size = 0;
    u32 instance_count = 0;

    if (element->flag & FSL_FLAG_UI_DIRTY_TRANSFORM)
        fsl_ui_element_bake(element);

    if (element->parent)
    {
        element->transform.baked.pos.x = element->transform.pos_local.x +
            element->parent->transform.baked.pos.x;
        element->transform.baked.pos.y = element->transform.pos_local.y +
            element->parent->transform.baked.pos.y;
    }

    ui_element_listen_internal(element,
            render_internal.mouse_pos, render_internal.mouse_delta);

    if (element->texture)
    {
        if (element->flag & FSL_FLAG_UI_9_SLICE)
        {
            panel = fsl_ui_element_panel_9_slice_make_internal(element);
            buf = &panel.transform;
            buf_size = sizeof(fsl_ui_transform_baked) * 9;
            instance_count = 9;
        }
        else
        {
            drawable.uv_pos.x = element->transform.baked.uv_pos.x;
            drawable.uv_pos.y = element->transform.baked.uv_pos.y;
            drawable.uv_size.x = element->transform.baked.uv_size.x;
            drawable.uv_size.y = element->transform.baked.uv_size.y;
            drawable.pos.x = element->transform.baked.pos.x * render_internal.ndc_scale.x;
            drawable.pos.y = element->transform.baked.pos.y * render_internal.ndc_scale.y;
            drawable.size.x = element->transform.baked.size.x * render_internal.ndc_scale.x;
            drawable.size.y = element->transform.baked.size.y * render_internal.ndc_scale.y;

            drawable.pos.x = drawable.pos.x - 1.0f;
            drawable.pos.y = 1.0f - drawable.pos.y;

            buf = &drawable;
            buf_size = sizeof(fsl_ui_transform_baked);
            instance_count = 1;
        }

        glBindBuffer(GL_ARRAY_BUFFER, ui_core.vertex_data_curr_quad);
        glBufferData(GL_ARRAY_BUFFER, buf_size, buf, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glUniform4f(ui_core.uniform.ui.tint, 1.0f, 1.0f, 1.0f, 1.0f);

        glBindVertexArray(ui_core.vao);
        glBindTexture(GL_TEXTURE_2D, element->texture->asset.id);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, instance_count);
    }
}

void fsl_ui_draw(fsl_texture *texture, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, u32 tint)
{
    fsl_ui_transform_baked drawable = {0};
    v2f32 ndc_scale = render_internal.ndc_scale;

    if (!size_x) size_x = texture->size.x;
    if (!size_y) size_y = texture->size.y;

    drawable.uv_pos.x = 0.0f;
    drawable.uv_pos.y = 0.0f;
    drawable.uv_size.x = 1.0f;
    drawable.uv_size.y = 1.0f;
    drawable.pos.x = pos_x * ndc_scale.x;
    drawable.pos.y = pos_y * ndc_scale.y;
    drawable.size.x = size_x * ndc_scale.x;
    drawable.size.y = size_y * ndc_scale.y;

    glBindBuffer(GL_ARRAY_BUFFER, ui_core.vertex_data_curr_quad);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fsl_ui_transform_baked), &drawable, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glUniform4f(ui_core.uniform.ui.tint,
            (f32)((tint >> 0x18) & 0xff) / 0xff,
            (f32)((tint >> 0x10) & 0xff) / 0xff,
            (f32)((tint >> 0x08) & 0xff) / 0xff,
            (f32)((tint >> 0x00) & 0xff) / 0xff);

    glBindVertexArray(ui_core.vao);
    glBindTexture(GL_TEXTURE_2D, texture->asset.id);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
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
        glDeleteBuffers(1, &text_core.vertex_data_unit_quad);
        glDeleteBuffers(1, &text_core.vbo_text_data);
    }

    fsl_mem_unmap((void*)&text_core.buf, text_core.buf_len * sizeof(struct glyph_render_data),
            "fsl_ui_free().text_core.buf");

    if (ui_core.vao_loaded)
    {
        glDeleteVertexArrays(1, &ui_core.vao);
        glDeleteBuffers(1, &ui_core.vertex_data_unit_quad);
        glDeleteBuffers(1, &ui_core.vertex_data_curr_quad);
    }
}

fsl_ui_element_panel_9_slice fsl_ui_element_panel_9_slice_make_internal(fsl_ui_element *element)
{
    fsl_ui_element_panel_9_slice panel = {0};
    fsl_ui_transform *t = &element->transform;
    f32 slice_size = t->slice_size;
    v2f32 slice_size_scaled = {0};
    v2f32 ndc_scale = render_internal.ndc_scale;
    v2f32 uv_scale = {0};
    v2f32 uv_pos[3] = {0};
    v2f32 uv_size[3] = {0};
    v2f32 pos = {0};
    v2f32 size = {0};
    v2f32 pos_buf[3] = {0};
    v2f32 size_buf[3] = {0};

    slice_size_scaled.x = slice_size * t->scale.x;
    slice_size_scaled.y = slice_size * t->scale.y;
    uv_scale.x = 1.0f / (f32)element->texture->size.x;
    uv_scale.y = 1.0f / (f32)element->texture->size.y;
    uv_pos[0].x = t->baked.uv_pos.x;
    uv_pos[0].y = t->baked.uv_pos.y;
    uv_pos[1].x = uv_pos[0].x + slice_size * uv_scale.x;
    uv_pos[1].y = uv_pos[0].y + slice_size * uv_scale.y;
    uv_pos[2].x = uv_pos[0].x + (t->uv_size.x - slice_size) * uv_scale.x;
    uv_pos[2].y = uv_pos[0].y + (t->uv_size.y - slice_size) * uv_scale.y;
    uv_size[0].x = slice_size * uv_scale.x;
    uv_size[0].y = slice_size * uv_scale.y;
    uv_size[1].x = (t->uv_size.x - slice_size * 2.0f) * uv_scale.x;
    uv_size[1].y = (t->uv_size.y - slice_size * 2.0f) * uv_scale.y;
    uv_size[2].x = slice_size * uv_scale.x;
    uv_size[2].y = slice_size * uv_scale.y;

    pos.x = t->baked.pos.x;
    pos.y = t->baked.pos.y;
    size.x = t->baked.size.x;
    size.y = t->baked.size.y;
    pos_buf[0].x = pos.x * ndc_scale.x - 1.0f;
    pos_buf[0].y = 1.0f - pos.y * ndc_scale.y;
    pos_buf[1].x = ((pos.x + slice_size_scaled.x) * ndc_scale.x) - 1.0f;
    pos_buf[1].y = 1.0f - (pos.y + slice_size_scaled.y) * ndc_scale.y;
    pos_buf[2].x = ((pos.x + size.x - slice_size_scaled.x) * ndc_scale.x) - 1.0f;
    pos_buf[2].y = 1.0f - (pos.y + size.y - slice_size_scaled.y) * ndc_scale.y;
    size_buf[0].x = slice_size_scaled.x * ndc_scale.x;
    size_buf[0].y = slice_size_scaled.y * ndc_scale.y;
    size_buf[1].x = (size.x - slice_size_scaled.x * 2.0f) * ndc_scale.x;
    size_buf[1].y = (size.y - slice_size_scaled.y * 2.0f) * ndc_scale.y;
    size_buf[2].x = slice_size_scaled.x * ndc_scale.x;
    size_buf[2].y = slice_size_scaled.y * ndc_scale.y;

    panel.transform[0].uv_pos.x = uv_pos[0].x;
    panel.transform[0].uv_pos.y = uv_pos[0].y;
    panel.transform[0].uv_size.x = uv_size[0].x;
    panel.transform[0].uv_size.y = uv_size[0].y;
    panel.transform[0].pos.x = pos_buf[0].x;
    panel.transform[0].pos.y = pos_buf[0].y;
    panel.transform[0].size.x = size_buf[0].x;
    panel.transform[0].size.y = size_buf[0].y;

    panel.transform[1].uv_pos.x = uv_pos[1].x;
    panel.transform[1].uv_pos.y = uv_pos[0].y;
    panel.transform[1].uv_size.x = uv_size[1].x;
    panel.transform[1].uv_size.y = uv_size[0].y;
    panel.transform[1].pos.x = pos_buf[1].x;
    panel.transform[1].pos.y = pos_buf[0].y;
    panel.transform[1].size.x = size_buf[1].x;
    panel.transform[1].size.y = size_buf[0].y;

    panel.transform[2].uv_pos.x = uv_pos[2].x;
    panel.transform[2].uv_pos.y = uv_pos[0].y;
    panel.transform[2].uv_size.x = uv_size[2].x;
    panel.transform[2].uv_size.y = uv_size[0].y;
    panel.transform[2].pos.x = pos_buf[2].x;
    panel.transform[2].pos.y = pos_buf[0].y;
    panel.transform[2].size.x = size_buf[2].x;
    panel.transform[2].size.y = size_buf[0].y;

    panel.transform[3].uv_pos.x = uv_pos[0].x;
    panel.transform[3].uv_pos.y = uv_pos[1].y;
    panel.transform[3].uv_size.x = uv_size[0].x;
    panel.transform[3].uv_size.y = uv_size[1].y;
    panel.transform[3].pos.x = pos_buf[0].x;
    panel.transform[3].pos.y = pos_buf[1].y;
    panel.transform[3].size.x = size_buf[0].x;
    panel.transform[3].size.y = size_buf[1].y;

    panel.transform[4].uv_pos.x = uv_pos[1].x;
    panel.transform[4].uv_pos.y = uv_pos[1].y;
    panel.transform[4].uv_size.x = uv_size[1].x;
    panel.transform[4].uv_size.y = uv_size[1].y;
    panel.transform[4].pos.x = pos_buf[1].x;
    panel.transform[4].pos.y = pos_buf[1].y;
    panel.transform[4].size.x = size_buf[1].x;
    panel.transform[4].size.y = size_buf[1].y;

    panel.transform[5].uv_pos.x = uv_pos[2].x;
    panel.transform[5].uv_pos.y = uv_pos[1].y;
    panel.transform[5].uv_size.x = uv_size[2].x;
    panel.transform[5].uv_size.y = uv_size[1].y;
    panel.transform[5].pos.x = pos_buf[2].x;
    panel.transform[5].pos.y = pos_buf[1].y;
    panel.transform[5].size.x = size_buf[2].x;
    panel.transform[5].size.y = size_buf[1].y;

    panel.transform[6].uv_pos.x = uv_pos[0].x;
    panel.transform[6].uv_pos.y = uv_pos[2].y;
    panel.transform[6].uv_size.x = uv_size[0].x;
    panel.transform[6].uv_size.y = uv_size[2].y;
    panel.transform[6].pos.x = pos_buf[0].x;
    panel.transform[6].pos.y = pos_buf[2].y;
    panel.transform[6].size.x = size_buf[0].x;
    panel.transform[6].size.y = size_buf[2].y;

    panel.transform[7].uv_pos.x = uv_pos[1].x;
    panel.transform[7].uv_pos.y = uv_pos[2].y;
    panel.transform[7].uv_size.x = uv_size[1].x;
    panel.transform[7].uv_size.y = uv_size[2].y;
    panel.transform[7].pos.x = pos_buf[1].x;
    panel.transform[7].pos.y = pos_buf[2].y;
    panel.transform[7].size.x = size_buf[1].x;
    panel.transform[7].size.y = size_buf[2].y;

    panel.transform[8].uv_pos.x = uv_pos[2].x;
    panel.transform[8].uv_pos.y = uv_pos[2].y;
    panel.transform[8].uv_size.x = uv_size[2].x;
    panel.transform[8].uv_size.y = uv_size[2].y;
    panel.transform[8].pos.x = pos_buf[2].x;
    panel.transform[8].pos.y = pos_buf[2].y;
    panel.transform[8].size.x = size_buf[2].x;
    panel.transform[8].size.y = size_buf[2].y;

    return panel;
}
