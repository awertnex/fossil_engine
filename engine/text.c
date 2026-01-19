#include <string.h>

#include "h/common.h"
#include "h/core.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/string.h"
#include "h/shaders.h"
#include "h/text.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include <engine/include/stb_truetype.h>

struct Glyphf
{
    v2f32 scale;
    v2f32 bearing;
    f32 advance;
    f32 x0, y0, x1, y1;
    v2f32 texture_sample;
    b8 loaded;
}; /* Glyphf */

struct TextData
{
    v2f32 pos;
    v2f32 tex_coords;
    u32 color;
}; /* TextData */

Font engine_font[ENGINE_FONT_COUNT] =
{
    [ENGINE_FONT_DEJAVU_SANS] =
    {
        .name = "DejaVu Sans (ANSI)",
        .path = ENGINE_DIR_NAME_FONTS"dejavu-fonts-ttf-2.37/dejavu_sans_ansi.ttf",
    },

    [ENGINE_FONT_DEJAVU_SANS_BOLD]
    {
        .name = "DejaVu Sans Bold (ANSI)",
        .path = ENGINE_DIR_NAME_FONTS"dejavu-fonts-ttf-2.37/dejavu_sans_bold_ansi.ttf",
    },

    [ENGINE_FONT_DEJAVU_SANS_MONO]
    {
        .name = "DejaVu Sans Mono (ANSI)",
        .path = ENGINE_DIR_NAME_FONTS"dejavu-fonts-ttf-2.37/dejavu_sans_mono_ansi.ttf",
    },

    [ENGINE_FONT_DEJAVU_SANS_MONO_BOLD]
    {
        .name = "DejaVu Sans Mono Bold (ANSI)",
        .path = ENGINE_DIR_NAME_FONTS"dejavu-fonts-ttf-2.37/dejavu_sans_mono_bold_ansi.ttf",
    },
};

static struct /* text_core */
{
    b8 multisample;
    Font *font;
    struct Glyphf glyph[GLYPH_MAX];
    f32 font_size;
    f32 line_height_total;
    f32 advance;
    v2f32 text_scale;

    /*! @brief iterator for 'buf',
     *  resets at 'text_start()' and 'text_render()'.
     */
    GLuint cursor;

    /*! @brief text buffer, raw text data.
    */
    struct TextData *buf;

    u64 buf_len;
    GLuint vao, vbo;
    FBO fbo;

    struct /* uniform */
    {
        GLint char_size;
        GLint font_size;
        GLint draw_shadow;
        GLint shadow_color;
        GLint shadow_offset;
    } uniform;

} text_core;

/* ---- section: font ------------------------------------------------------- */

u32 font_init(Font *font, u32 resolution, const str *name, const str *file_name)
{
    f32 scale;
    u32 i, x, y, col, row;
    u8 *canvas = NULL;
    Glyph *g = NULL;

    if (resolution <= 2)
    {
        _LOGERROR(FALSE, ERR_IMAGE_SIZE_TOO_SMALL,
                "Failed to Initialize Font '%s', Font Size Too Small\n", file_name);
        return engine_err;
    }

    if (strlen(file_name) >= PATH_MAX)
    {
        _LOGERROR(FALSE, ERR_PATH_TOO_LONG,
                "Failed to Initialize Font '%s', File Path Too Long\n", file_name);
        return engine_err;
    }

    if (is_file_exists(file_name, TRUE) != ERR_SUCCESS)
        return engine_err;

    font->buf_len = get_file_contents(file_name, (void*)&font->buf, 1, TRUE);
    if (!font->buf)
        return engine_err;

    if (!stbtt_InitFont(&font->info, (const unsigned char*)font->buf, 0))
    {
        _LOGERROR(FALSE, ERR_FONT_INIT_FAIL,
                "Failed to Initialize Font '%s', 'stbtt_InitFont()' Failed\n", file_name);
        goto cleanup;
    }

    if (mem_alloc((void*)&font->bitmap, GLYPH_MAX * resolution * resolution,
                stringf("font_init().%s", file_name)) != ERR_SUCCESS)
        goto cleanup;

    if (mem_alloc((void*)&canvas, resolution * resolution,
                "font_init().font_glyph_canvas") != ERR_SUCCESS)
        goto cleanup;

    if (name && !font->name[0])
        snprintf(font->name, NAME_MAX, "%s", name);

    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
    font->resolution = resolution;
    font->char_size = 1.0f / FONT_ATLAS_CELL_RESOLUTION;
    font->line_height = font->ascent - font->descent + font->line_gap;
    font->size = resolution;
    scale = stbtt_ScaleForPixelHeight(&font->info, resolution);

    for (i = 0; i < GLYPH_MAX; ++i)
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

        col = i % FONT_ATLAS_CELL_RESOLUTION;
        row = i / FONT_ATLAS_CELL_RESOLUTION;
        if (!stbtt_IsGlyphEmpty(&font->info, glyph_index))
        {
            stbtt_MakeGlyphBitmapSubpixel(&font->info, canvas,
                    resolution, resolution, resolution, scale, scale, 0.0f, 0.0f, glyph_index);

            void *bitmap_offset = font->bitmap +
                col * resolution +
                row * resolution * resolution * FONT_ATLAS_CELL_RESOLUTION +
                1 + resolution * FONT_ATLAS_CELL_RESOLUTION;

            for (y = 0; y < resolution - 1; ++y)
                for (x = 0; x < resolution - 1; ++x)
                    memcpy(bitmap_offset + x +
                            y * resolution * FONT_ATLAS_CELL_RESOLUTION,
                            canvas + x + y * resolution, 1);

            bzero(canvas, resolution * resolution);
        }

        g->texture_sample.x = col * font->char_size;
        g->texture_sample.y = row * font->char_size;
        font->glyph[i].loaded = TRUE;
    }

    if (_texture_generate(&font->id, GL_RED, GL_RED, GL_LINEAR,
                FONT_ATLAS_CELL_RESOLUTION * resolution,
                FONT_ATLAS_CELL_RESOLUTION * resolution,
                font->bitmap, TRUE) != ERR_SUCCESS)
        goto cleanup;

    mem_free((void*)&canvas, resolution * resolution, "font_init().font_glyph_canvas");

    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:

    mem_free((void*)&canvas, resolution * resolution, "font_init().font_glyph_canvas");
    font_free(font);
    return engine_err;
}

void font_free(Font *font)
{
    if (!font) return;
    mem_free((void*)&font->buf, font->buf_len, "font_free().file_contents");
    mem_free((void*)&font->bitmap, GLYPH_MAX * font->resolution * font->resolution, font->name);
    *font = (Font){0};
}

/* ---- section: text ------------------------------------------------------- */

u32 text_init(u32 resolution, b8 multisample)
{
    u32 i = 0;

    /* ---- mandatory engine fonts ------------------------------------------ */

    for (i = 0; i < ENGINE_FONT_COUNT; ++i)
        if (font_init(&engine_font[i],
                    resolution ? resolution : FONT_RESOLUTION_DEFAULT,
                    NULL, engine_font[i].path) != ERR_SUCCESS)
            goto cleanup;

    if (
            mem_alloc((void*)&text_core.buf, STRING_MAX * sizeof(struct TextData),
                "text_init().text_core.buf") != ERR_SUCCESS)
        goto cleanup;

    text_core.buf_len = STRING_MAX;

    if (fbo_init(&text_core.fbo, &engine_mesh_unit, multisample, 4) != ERR_SUCCESS)
        goto cleanup;

    if (!text_core.vao)
    {
        glGenVertexArrays(1, &text_core.vao);
        glGenBuffers(1, &text_core.vbo);

        glBindVertexArray(text_core.vao);
        glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                TEXT_CHAR_STRIDE * sizeof(GLfloat), (void*)0);

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                TEXT_CHAR_STRIDE * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

        glEnableVertexAttribArray(2);
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_INT,
                TEXT_CHAR_STRIDE * sizeof(GLuint), (void*)(4 * sizeof(GLfloat)));

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    text_core.multisample = multisample;

    text_core.uniform.char_size = glGetUniformLocation(engine_shader[ENGINE_SHADER_TEXT].id, "char_size");
    text_core.uniform.font_size = glGetUniformLocation(engine_shader[ENGINE_SHADER_TEXT].id, "font_size");
    text_core.uniform.draw_shadow = glGetUniformLocation(engine_shader[ENGINE_SHADER_TEXT].id, "draw_shadow");
    text_core.uniform.shadow_color = glGetUniformLocation(engine_shader[ENGINE_SHADER_TEXT].id, "shadow_color");
    text_core.uniform.shadow_offset = glGetUniformLocation(engine_shader[ENGINE_SHADER_TEXT].id, "shadow_offset");

    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:

    text_free();
    _LOGFATAL(FALSE, ERR_TEXT_INIT_FAIL, "%s\n", "Failed to Initialize Text, Process Aborted");
    return engine_err;
}

void text_start(Font *font, f32 size, u64 length, FBO *fbo, b8 clear)
{
    static v2i32 render_size = {0};
    FBO *_fbo = &text_core.fbo;
    f32 scale = 0.0f;
    struct Glyphf *g = NULL;
    u32 i = 0;

    if (fbo) _fbo = fbo;
    if (render_size.x != render->size.x || render_size.y != render->size.y)
        fbo_realloc(_fbo, text_core.multisample, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo->fbo);

    if (!length)
        length = STRING_MAX;
    else if (length > text_core.buf_len)
    {
        if (
                mem_realloc((void*)&text_core.buf,
                    length * sizeof(struct TextData),
                    "text_start().text_core.buf") != ERR_SUCCESS)
            goto cleanup;

        text_core.buf_len = length;
    }

    text_core.font = font;
    text_core.font_size = size;
    text_core.line_height_total = 0.0f;
    text_core.advance = 0.0f;
    text_core.cursor = 0;

    scale = stbtt_ScaleForPixelHeight(&font->info, size);
    text_core.text_scale.x = scale * render->ndc_scale.x;
    text_core.text_scale.y = scale * render->ndc_scale.y;

    for (i = 0; i < GLYPH_MAX; ++i)
    {
        if (!text_core.font->glyph[i].loaded)
            continue;
        g = &text_core.glyph[i];

        g->bearing.x = text_core.font->glyph[i].bearing.x * text_core.text_scale.x;
        g->bearing.y = text_core.font->glyph[i].bearing.y * text_core.text_scale.y;
        g->advance = text_core.font->glyph[i].advance * text_core.text_scale.x;
        g->texture_sample.x = text_core.font->glyph[i].texture_sample.x;
        g->texture_sample.y = text_core.font->glyph[i].texture_sample.y;
        g->loaded = TRUE;
    }

    glUseProgram(engine_shader[ENGINE_SHADER_TEXT].id);
    glUniform1f(text_core.uniform.char_size, text_core.font->char_size);
    glUniform2f(text_core.uniform.font_size,
            text_core.font_size * render->ndc_scale.x,
            text_core.font_size * render->ndc_scale.y);

    glBindTexture(GL_TEXTURE_2D, text_core.font->id);
    glDisable(GL_DEPTH_TEST);
    if (clear)
        glClear(GL_COLOR_BUFFER_BIT);

    return;

cleanup:

    _LOGERROR(FALSE, engine_err, "%s\n", "Failed to Start Text");
}

void text_push(const str *text, v2f32 pos, i8 align_x, i8 align_y, u32 color)
{
    u64 len = 0, i = 0;
    i64 j = 0;
    f32 descent = 0.0f, line_height = 0.0f;
    struct Glyphf *g = NULL;
    v2u8 align = {0};
    v2f32 alignment = {0};

    if (!text_core.buf)
    {
        _LOGERROR(FALSE, ERR_BUFFER_EMPTY, "%s\n", "Failed to Push Text, 'text_core.buf' Null");
        return;
    }

    len = strlen(text);
    if (len >= STRING_MAX)
    {
        _LOGERROR(FALSE, ERR_STRING_TOO_LONG, "%s\n", "Failed to Push Text, Text Too Long");
        return;
    }

    if (text_core.cursor + len >= text_core.buf_len)
    {
        if (
                mem_realloc((void*)&text_core.buf,
                    (text_core.buf_len + STRING_MAX) * sizeof(struct TextData),
                    "text_push().text_core.buf_len") != ERR_SUCCESS)
        {
            _LOGERROR(FALSE, engine_err, "%s\n", "Failed to Push Text");
            return;
        }

        text_core.buf_len += STRING_MAX;
    }

    if (align_x == TEXT_ALIGN_CENTER)
    {
        align.x = 1;
        alignment.x = 0.5f;
    }
    else if (align_x == TEXT_ALIGN_RIGHT)
    {
        align.x = 1;
        alignment.x = 1.0f;
    }

    if (align_y == TEXT_ALIGN_CENTER)
    {
        align.y = 1;
        alignment.y = 0.5f;
    }
    else if (align_y == TEXT_ALIGN_BOTTOM)
    {
        align.y = 1;
        alignment.y = 1.0f;
    }

    pos.x *= render->ndc_scale.x;
    pos.y *= render->ndc_scale.y;
    pos.y += text_core.font->scale.y * text_core.text_scale.y;

    descent = text_core.font->descent * text_core.text_scale.y;
    line_height = text_core.font->line_height;
    for (i = 0; i < len; ++i)
    {
        g = &text_core.glyph[(u64)text[i]];
        if (text[i] == '\n' || text[i] == '\r')
        {
            if (align.x)
                for (j = 1; (i64)i - j >= 0 && text[i - j] != '\n' && text[i - j] != '\r'; ++j)
                    text_core.buf[text_core.cursor - j].pos.x -= text_core.advance * alignment.x;

            text_core.advance = 0.0f;
            if (text[i] == '\n')
                text_core.line_height_total += line_height * text_core.text_scale.y;
            continue;
        }
        else if (text[i] == '\t')
        {
            text_core.advance += text_core.glyph[' '].advance * TEXT_TAB_SIZE;
            continue;
        }

        text_core.buf[text_core.cursor].pos.x = pos.x + text_core.advance + g->bearing.x;
        text_core.buf[text_core.cursor].pos.y = -pos.y - descent - text_core.line_height_total - g->bearing.y;
        text_core.buf[text_core.cursor].tex_coords.x = g->texture_sample.x;
        text_core.buf[text_core.cursor].tex_coords.y = g->texture_sample.y;
        text_core.buf[text_core.cursor].color = color;
        ++text_core.cursor;

        text_core.advance += g->advance;
    }

    if (align.y)
        for (i = 0; i < text_core.cursor; ++i)
            text_core.buf[i].pos.y += text_core.line_height_total * alignment.y;
}

void text_render(b8 shadow, u32 shadow_color)
{
    if (!text_core.buf)
    {
        _LOGERROR(FALSE, ERR_BUFFER_EMPTY,
                "%s\n", "Failed to Render Text, 'text_core.buf' Null");
        return;
    }

    glBindVertexArray(text_core.vao);
    glBindBuffer(GL_ARRAY_BUFFER, text_core.vbo);
    glBufferData(GL_ARRAY_BUFFER, text_core.cursor * sizeof(GLfloat) * TEXT_CHAR_STRIDE,
            text_core.buf, GL_DYNAMIC_DRAW);

    if (shadow)
    {
        glUniform1i(text_core.uniform.draw_shadow, TRUE);
        glUniform4f(text_core.uniform.shadow_color,
                (f32)((shadow_color >> 0x18) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x10) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x08) & 0xff) / 0xff,
                (f32)((shadow_color >> 0x00) & 0xff) / 0xff);
        glUniform2f(text_core.uniform.shadow_offset, TEXT_OFFSET_SHADOW, TEXT_OFFSET_SHADOW);
        glDrawArrays(GL_POINTS, 0, text_core.cursor);
    }

    glUniform1i(text_core.uniform.draw_shadow, FALSE);
    glDrawArrays(GL_POINTS, 0, text_core.cursor);

    text_core.cursor = 0;
    text_core.line_height_total = 0;
    text_core.advance = 0.0f;
}

void text_stop(void)
{
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void text_fbo_blit(GLuint fbo)
{
    glUseProgram(engine_shader[ENGINE_SHADER_UNIT_QUAD].id);
    glBindVertexArray(engine_mesh_unit.vao);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, text_core.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

void text_free(void)
{
    fbo_free(&text_core.fbo);
    mem_free((void*)&text_core.buf, text_core.buf_len * sizeof(struct TextData),
            "text_free().text_core.buf");
}
