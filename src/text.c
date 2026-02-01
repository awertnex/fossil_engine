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

/*  text.c - text rendering, font loading and unloading
 */

#include "h/common.h"

#include "h/core.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/string.h"
#include "h/shaders.h"
#include "h/text.h"

#include <string.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <deps/stb_truetype.h>

static f32 vbo_data_unit_quad[] =
{
    0.0f, -1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 1.0f, 1.0f,
};

fsl_font fsl_font_buf[FSL_FONT_INDEX_COUNT] =
{
    [FSL_FONT_INDEX_DEJAVU_SANS] =
    {
        .name = "DejaVu Sans (ANSI)",
        .path = FSL_DIR_NAME_FONTS"dejavu-fonts-ttf-2.37/dejavu_sans_ansi.ttf",
    },

    [FSL_FONT_INDEX_DEJAVU_SANS_BOLD]
    {
        .name = "DejaVu Sans Bold (ANSI)",
        .path = FSL_DIR_NAME_FONTS"dejavu-fonts-ttf-2.37/dejavu_sans_bold_ansi.ttf",
    },

    [FSL_FONT_INDEX_DEJAVU_SANS_MONO]
    {
        .name = "DejaVu Sans Mono (ANSI)",
        .path = FSL_DIR_NAME_FONTS"dejavu-fonts-ttf-2.37/dejavu_sans_mono_ansi.ttf",
    },

    [FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD]
    {
        .name = "DejaVu Sans Mono Bold (ANSI)",
        .path = FSL_DIR_NAME_FONTS"dejavu-fonts-ttf-2.37/dejavu_sans_mono_bold_ansi.ttf",
    },
};

struct fsl_glyphf
{
    v2f32 scale;
    v2f32 bearing;
    f32 advance;
    f32 x0, y0, x1, y1;
    v2f32 texture_sample;
    b8 loaded;
}; /* fsl_glyphf */

struct fsl_text_data
{
    v2f32 pos;
    v2f32 tex_coords;
    u32 color;
}; /* fsl_text_data */

static struct fsl_text_core
{
    b8 multisample;
    fsl_font *font;
    struct fsl_glyphf glyph[FSL_GLYPH_MAX];
    f32 font_size;
    f32 line_height_total;
    f32 advance;
    v2f32 text_scale;

    /*! @brief iterator for 'buf',
     *  resets at 'fsl_text_start()' and 'fsl_text_render()'.
     */
    GLuint cursor;

    /*! @brief text buffer, raw text data.
    */
    struct fsl_text_data *buf;

    u64 buf_len;
    GLuint vao;
    GLuint vbo_unit_quad;
    GLuint vbo_text_data;
    b8 vao_loaded;
    fsl_fbo fbo;

    struct /* uniform */
    {
        GLint char_size;
        GLint font_size;
        GLint draw_shadow;
        GLint shadow_color;
        GLint shadow_offset;
    } uniform;

} fsl_text_core;

/* ---- section: font ------------------------------------------------------- */

u32 fsl_font_init(fsl_font *font, u32 resolution, const str *name, const str *file_name)
{
    f32 scale;
    u32 i, x, y, col, row;
    u8 *canvas = NULL;
    fsl_glyph *g = NULL;

    if (resolution <= 2)
    {
        _LOGERROR(FSL_ERR_IMAGE_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Initialize Font '%s', Font Size Too Small\n", file_name);
        return fsl_err;
    }

    if (strlen(file_name) >= PATH_MAX)
    {
        _LOGERROR(FSL_ERR_PATH_TOO_LONG,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Initialize Font '%s', File Path Too Long\n", file_name);
        return fsl_err;
    }

    if (fsl_is_file_exists(file_name, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    font->buf_len = fsl_get_file_contents(file_name, (void*)&font->buf, 1, TRUE);
    if (!font->buf)
        return fsl_err;

    if (!stbtt_InitFont(&font->info, (const unsigned char*)font->buf, 0))
    {
        _LOGERROR(FSL_ERR_FONT_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Initialize Font '%s', 'stbtt_InitFont()' Failed\n", file_name);
        goto cleanup;
    }

    if (fsl_mem_alloc((void*)&font->bitmap, FSL_GLYPH_MAX * resolution * resolution,
                fsl_stringf("fsl_font_init().%s", file_name)) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_mem_alloc((void*)&canvas, resolution * resolution,
                "fsl_font_init().canvas") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (name && !font->name[0])
        snprintf(font->name, NAME_MAX, "%s", name);

    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
    font->resolution = resolution;
    font->char_size = 1.0f / FSL_FONT_ATLAS_CELL_RESOLUTION;
    font->line_height = font->ascent - font->descent + font->line_gap;
    font->size = resolution;
    scale = stbtt_ScaleForPixelHeight(&font->info, resolution);

    for (i = 0; i < FSL_GLYPH_MAX; ++i)
    {
        int glyph_index = stbtt_FindGlyphIndex(&font->info, i);
        if (!glyph_index) continue;

        g = &font->glyph[i];

        stbtt_GetGlyphHMetrics(&font->info, glyph_index, &g->advance, &g->bearing.x);
        stbtt_GetGlyphBitmapBoxSubpixel(&font->info, glyph_index,
                1.0f, 1.0f, 0.0f, 0.0f, &g->x0, &g->y0, &g->x1, &g->y1);

        g->bearing.y = g->y0;
        g->scale.x = g->x1 - g->x0;
        g->scale.y = g->y1 - g->y0;
        g->scale.x > font->scale.x ? font->scale.x = g->scale.x : 0;
        g->scale.y > font->scale.y ? font->scale.y = g->scale.y : 0;

        col = i % FSL_FONT_ATLAS_CELL_RESOLUTION;
        row = i / FSL_FONT_ATLAS_CELL_RESOLUTION;
        if (!stbtt_IsGlyphEmpty(&font->info, glyph_index))
        {
            stbtt_MakeGlyphBitmapSubpixel(&font->info, canvas,
                    resolution, resolution, resolution, scale, scale, 0.0f, 0.0f, glyph_index);

            void *bitmap_offset = font->bitmap +
                col * resolution +
                row * resolution * resolution * FSL_FONT_ATLAS_CELL_RESOLUTION +
                1 + resolution * FSL_FONT_ATLAS_CELL_RESOLUTION;

            for (y = 0; y < resolution - 1; ++y)
                for (x = 0; x < resolution - 1; ++x)
                    memcpy(bitmap_offset + x +
                            y * resolution * FSL_FONT_ATLAS_CELL_RESOLUTION,
                            canvas + x + y * resolution, 1);

            bzero(canvas, resolution * resolution);
        }

        g->texture_sample.x = col * font->char_size;
        g->texture_sample.y = row * font->char_size;
        font->glyph[i].loaded = TRUE;
    }

    if (_fsl_texture_generate(&font->id, GL_RED, GL_RED, GL_LINEAR,
                FSL_FONT_ATLAS_CELL_RESOLUTION * resolution,
                FSL_FONT_ATLAS_CELL_RESOLUTION * resolution,
                font->bitmap, TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_mem_free((void*)&canvas, resolution * resolution, "fsl_font_init().canvas");
    fsl_mem_free((void*)&font->bitmap, FSL_GLYPH_MAX * font->resolution * font->resolution, font->name);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mem_free((void*)&canvas, resolution * resolution, "fsl_font_init().canvas");
    fsl_font_free(font);
    return fsl_err;
}

void fsl_font_free(fsl_font *font)
{
    if (!font) return;
    fsl_mem_free((void*)&font->buf, font->buf_len, font->name);
    fsl_mem_free((void*)&font->bitmap, FSL_GLYPH_MAX * font->resolution * font->resolution, font->name);
    *font = (fsl_font){0};
}

/* ---- section: text ------------------------------------------------------- */

u32 fsl_text_init(u32 resolution, b8 multisample)
{
    u32 i = 0;

    /* ---- mandatory engine fonts ------------------------------------------ */

    for (i = 0; i < FSL_FONT_INDEX_COUNT; ++i)
        if (fsl_font_init(&fsl_font_buf[i],
                    resolution ? resolution : FSL_FONT_RESOLUTION_DEFAULT,
                    NULL, fsl_font_buf[i].path) != FSL_ERR_SUCCESS)
            goto cleanup;

    if (
            fsl_mem_map((void*)&fsl_text_core.buf, FSL_STRING_MAX * sizeof(struct fsl_text_data),
                "fsl_text_init().fsl_text_core.buf") != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_text_core.buf_len = FSL_STRING_MAX;

    if (fsl_fbo_init(&fsl_text_core.fbo, &fsl_mesh_unit_quad, multisample, 4) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (!fsl_text_core.vao_loaded)
    {
        glGenVertexArrays(1, &fsl_text_core.vao);
        glBindVertexArray(fsl_text_core.vao);

        /* ---- unit quad --------------------------------------------------- */

        glGenBuffers(1, &fsl_text_core.vbo_unit_quad);
        glBindBuffer(GL_ARRAY_BUFFER, fsl_text_core.vbo_unit_quad);
        glBufferData(GL_ARRAY_BUFFER, fsl_arr_len(vbo_data_unit_quad) * sizeof(f32),
                &vbo_data_unit_quad, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

        /* ---- text data --------------------------------------------------- */

        glGenBuffers(1, &fsl_text_core.vbo_text_data);
        glBindBuffer(GL_ARRAY_BUFFER, fsl_text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, fsl_text_core.buf_len * sizeof(struct fsl_text_data),
                NULL, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct fsl_text_data), (void*)0);
        glVertexAttribDivisor(2, 1);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE,
                sizeof(struct fsl_text_data), (void*)(2 * sizeof(GLfloat)));
        glVertexAttribDivisor(3, 1);

        glEnableVertexAttribArray(4);
        glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT,
                sizeof(struct fsl_text_data), (void*)(4 * sizeof(GLfloat)));
        glVertexAttribDivisor(4, 1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    fsl_text_core.multisample = multisample;

    fsl_text_core.uniform.char_size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_TEXT].id, "char_size");
    fsl_text_core.uniform.font_size =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_TEXT].id, "font_size");
    fsl_text_core.uniform.draw_shadow =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_TEXT].id, "draw_shadow");
    fsl_text_core.uniform.shadow_color =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_TEXT].id, "shadow_color");
    fsl_text_core.uniform.shadow_offset =
        glGetUniformLocation(fsl_shader_buf[FSL_SHADER_INDEX_TEXT].id, "shadow_offset");

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_text_free();
    _LOGFATAL(FSL_ERR_TEXT_INIT_FAIL,
            FSL_FLAG_LOG_NO_VERBOSE,
            "%s\n", "Failed to Initialize Text, Process Aborted");
    return fsl_err;
}

void fsl_text_start(fsl_font *font, f32 size, u64 length, fsl_fbo *fbo, b8 clear)
{
    static v2i32 render_size = {0};
    fsl_fbo *_fbo = &fsl_text_core.fbo;
    f32 scale = 0.0f;
    struct fsl_glyphf *g = NULL;
    u32 i = 0;

    if (fbo) _fbo = fbo;
    if (render_size.x != render->size.x || render_size.y != render->size.y)
        fsl_fbo_realloc(_fbo, fsl_text_core.multisample, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo->fbo);

    if (!length)
        length = FSL_STRING_MAX;
    else if (length > fsl_text_core.buf_len)
    {
        if (
                fsl_mem_remap((void*)&fsl_text_core.buf,
                    fsl_text_core.buf_len * sizeof(struct fsl_text_data),
                    length * sizeof(struct fsl_text_data),
                    "fsl_text_start().fsl_text_core.buf") != FSL_ERR_SUCCESS)
            goto cleanup;

        fsl_text_core.buf_len = length;

        glBindBuffer(GL_ARRAY_BUFFER, fsl_text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, fsl_text_core.buf_len * sizeof(struct fsl_text_data),
                NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    fsl_text_core.font = font;
    fsl_text_core.font_size = size;
    fsl_text_core.line_height_total = 0.0f;
    fsl_text_core.advance = 0.0f;
    fsl_text_core.cursor = 0;

    scale = stbtt_ScaleForPixelHeight(&font->info, size);
    fsl_text_core.text_scale.x = scale * render->ndc_scale.x;
    fsl_text_core.text_scale.y = scale * render->ndc_scale.y;

    for (i = 0; i < FSL_GLYPH_MAX; ++i)
    {
        if (!fsl_text_core.font->glyph[i].loaded)
            continue;
        g = &fsl_text_core.glyph[i];

        g->bearing.x = fsl_text_core.font->glyph[i].bearing.x * fsl_text_core.text_scale.x;
        g->bearing.y = fsl_text_core.font->glyph[i].bearing.y * fsl_text_core.text_scale.y;
        g->advance = fsl_text_core.font->glyph[i].advance * fsl_text_core.text_scale.x;
        g->texture_sample.x = fsl_text_core.font->glyph[i].texture_sample.x;
        g->texture_sample.y = fsl_text_core.font->glyph[i].texture_sample.y;
        g->loaded = TRUE;
    }

    glUseProgram(fsl_shader_buf[FSL_SHADER_INDEX_TEXT].id);
    glUniform1f(fsl_text_core.uniform.char_size, fsl_text_core.font->char_size);
    glUniform2f(fsl_text_core.uniform.font_size,
            fsl_text_core.font_size * render->ndc_scale.x,
            fsl_text_core.font_size * render->ndc_scale.y);

    glBindTexture(GL_TEXTURE_2D, fsl_text_core.font->id);
    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);

    return;

cleanup:

    _LOGERROR(fsl_err,
            FSL_FLAG_LOG_NO_VERBOSE,
            "%s\n", "Failed to Start Text");
}

void fsl_text_push(const str *text, f32 pos_x, f32 pos_y, i8 align_x, i8 align_y,
        i32 window_x, u32 color)
{
    u64 len = 0, i = 0;
    i64 j = 0;
    f32 descent = 0.0f, line_height = 0.0f;
    struct fsl_glyphf *g = NULL;
    v2u8 align = {0};
    v2f32 alignment = {0};
    f32 _window_x = (f32)window_x * render->ndc_scale.x;

    if (!fsl_text_core.buf)
    {
        _LOGERROR(FSL_ERR_BUFFER_EMPTY,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Push Text, 'fsl_text_core.buf' Null");
        return;
    }

    len = strlen(text);
    if (len >= FSL_STRING_MAX)
    {
        _LOGERROR(FSL_ERR_STRING_TOO_LONG,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Push Text, Text Too Long");
        return;
    }

    if (fsl_text_core.cursor + len >= fsl_text_core.buf_len)
    {
        if (
                fsl_mem_remap((void*)&fsl_text_core.buf,
                    fsl_text_core.buf_len * sizeof(struct fsl_text_data),
                    (fsl_text_core.buf_len + FSL_STRING_MAX) * sizeof(struct fsl_text_data),
                    "fsl_text_push().fsl_text_core.buf_len") != FSL_ERR_SUCCESS)
        {
            _LOGERROR(fsl_err,
                    FSL_FLAG_LOG_NO_VERBOSE,
                    "%s\n", "Failed to Push Text");
            return;
        }

        fsl_text_core.buf_len += FSL_STRING_MAX;

        glBindBuffer(GL_ARRAY_BUFFER, fsl_text_core.vbo_text_data);
        glBufferData(GL_ARRAY_BUFFER, fsl_text_core.buf_len * sizeof(struct fsl_text_data),
                NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    if (align_x == FSL_TEXT_ALIGN_CENTER)
    {
        align.x = 1;
        alignment.x = 0.5f;
    }
    else if (align_x == FSL_TEXT_ALIGN_RIGHT)
    {
        align.x = 1;
        alignment.x = 1.0f;
    }

    if (align_y == FSL_TEXT_ALIGN_CENTER)
    {
        align.y = 1;
        alignment.y = 0.5f;
    }
    else if (align_y == FSL_TEXT_ALIGN_BOTTOM)
    {
        align.y = 1;
        alignment.y = 1.0f;
    }

    pos_x *= render->ndc_scale.x;
    pos_y *= render->ndc_scale.y;
    pos_y += fsl_text_core.font->scale.y * fsl_text_core.text_scale.y;

    descent = fsl_text_core.font->descent * fsl_text_core.text_scale.y;
    line_height = fsl_text_core.font->line_height;
    for (i = 0; i < len; ++i)
    {
        g = &fsl_text_core.glyph[(u64)text[i]];

        if (text[i] == '\n' || text[i] == '\r')
        {
            if (align.x)
                for (j = 1; (i64)i - j >= 0 && text[i - j] != '\n' && text[i - j] != '\r'; ++j)
                    fsl_text_core.buf[fsl_text_core.cursor - j].pos.x -= fsl_text_core.advance * alignment.x;

            fsl_text_core.advance = 0.0f;
            if (text[i] == '\n')
                fsl_text_core.line_height_total += line_height * fsl_text_core.text_scale.y;
            continue;
        }
        else if (text[i] == '\t')
        {
            fsl_text_core.advance += fsl_text_core.glyph[' '].advance * FSL_TEXT_TAB_SIZE;
            continue;
        }

        if (window_x && fsl_text_core.advance + g->advance >= _window_x)
        {
            if (align.x)
                for (j = 1; (i64)i - j >= 0 && text[i - j] != '\n' && text[i - j] != '\r'; ++j)
                    fsl_text_core.buf[fsl_text_core.cursor - j].pos.x -= fsl_text_core.advance * alignment.x;

            fsl_text_core.advance = 0.0f;
            fsl_text_core.line_height_total += line_height * fsl_text_core.text_scale.y;
        }

        fsl_text_core.buf[fsl_text_core.cursor].pos.x =
            pos_x + fsl_text_core.advance + g->bearing.x;

        fsl_text_core.buf[fsl_text_core.cursor].pos.y =
            -pos_y - descent - fsl_text_core.line_height_total - g->bearing.y;

        fsl_text_core.buf[fsl_text_core.cursor].tex_coords.x =
            g->texture_sample.x;

        fsl_text_core.buf[fsl_text_core.cursor].tex_coords.y =
            g->texture_sample.y;

        fsl_text_core.buf[fsl_text_core.cursor].color =
            color;

        ++fsl_text_core.cursor;

        fsl_text_core.advance += g->advance;
    }

    if (align.y)
        for (i = 0; i < fsl_text_core.cursor; ++i)
            fsl_text_core.buf[i].pos.y += fsl_text_core.line_height_total * alignment.y;
}

void fsl_text_render(b8 shadow, u32 shadow_color)
{
    if (!fsl_text_core.buf)
    {
        _LOGERROR(FSL_ERR_BUFFER_EMPTY,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Render Text, 'fsl_text_core.buf' Null");
        return;
    }

    glBindBuffer(GL_ARRAY_BUFFER, fsl_text_core.vbo_text_data);
    glBufferSubData(GL_ARRAY_BUFFER, 0, fsl_text_core.cursor * sizeof(struct fsl_text_data),
            fsl_text_core.buf);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(fsl_text_core.vao);

    if (shadow)
    {
        glUniform1i(fsl_text_core.uniform.draw_shadow, TRUE);
        glUniform4f(fsl_text_core.uniform.shadow_color,
                (f32)((shadow_color >> 0x18) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x10) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x08) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x00) & 0xff) / 0xff);
        glUniform2f(fsl_text_core.uniform.shadow_offset, FSL_TEXT_OFFSET_SHADOW, FSL_TEXT_OFFSET_SHADOW);
        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, fsl_text_core.cursor);
    }

    glUniform1i(fsl_text_core.uniform.draw_shadow, FALSE);
    glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 4, fsl_text_core.cursor);

    fsl_text_core.cursor = 0;
    fsl_text_core.line_height_total = 0;
    fsl_text_core.advance = 0.0f;
}

void fsl_text_stop(void)
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void fsl_text_fbo_blit(GLuint fbo)
{
    glUseProgram(fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, fsl_text_core.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void fsl_text_free(void)
{
    if (fsl_text_core.vao_loaded)
    {
        glDeleteVertexArrays(1, &fsl_text_core.vao);
        glDeleteBuffers(1, &fsl_text_core.vbo_unit_quad);
        glDeleteBuffers(1, &fsl_text_core.vbo_text_data);
    }

    fsl_mem_unmap((void*)&fsl_text_core.buf, fsl_text_core.buf_len * sizeof(struct fsl_text_data),
            "fsl_text_free().fsl_text_core.buf");
}

f32 fsl_get_text_height(void)
{
    return fsl_text_core.line_height_total / render->ndc_scale.y;
}
