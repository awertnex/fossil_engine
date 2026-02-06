/*  @file ui.h
 *
 *  @brief everything about drawing ui elements.
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

#ifndef FSL_UI_H
#define FSL_UI_H

#include "common.h"
#include "core.h"
#include "types.h"

#include <deps/stb_truetype.h>

/* ---- section: definitions ------------------------------------------------ */

#define FSL_UI_PANEL_GAP_DEFAULT 10
#define FSL_UI_PANEL_PADDING_DEFAULT 10
#define FSL_UI_SLICE_SIZE_DEFAULT 8

typedef struct fsl_glyph
{
    v2i32 scale;
    v2i32 bearing;
    i32 advance;
    b8 loaded;
} fsl_glyph;

typedef struct fsl_font
{
    /*! @brief font name, initialized in @ref fsl_font_init() if empty.
     */
    str name[NAME_MAX];

    /*! @brief font file name, initialized in @ref fsl_font_init() if `NULL`.
     */
    str *path;

    u32 resolution; /* glyph bitmap diameter in bytes */
    i32 ascent; /* glyphs highest points' deviation from baseline */
    i32 descent; /* glyphs lowest points' deviation from baseline */
    i32 line_gap;
    i32 line_height;
    f32 size; /* global font size, for text uniformity */
    v2i32 scale; /* biggest glyph bounding box size in font units */

    stbtt_fontinfo info; /* used by @ref stbtt_InitFont() */

    /*! @brief font file contents.
     *
     *  used by @ref stbtt_InitFont().
     */
    u8 *buf;

    u64 buf_len; /* `buf` size in bytes */
    u8 *bitmap; /* memory block for all font glyph bitmaps */

    GLuint id; /* used by @ref glGenTextures() */
    fsl_glyph glyph[FSL_GLYPH_MAX];
} fsl_font;

/*! @brief one slice in a 9-slice panel.
 */
typedef struct fsl_panel_slice
{
    v2f32 pos;
    v2f32 size;
    v2f32 tex_coords_pos;
    v2f32 tex_coords_size;
} fsl_panel_slice;

/*! @brief one 9-slice panel.
 */
typedef struct fsl_panel_nine_slice
{
    struct fsl_panel_slice slice[9];
} fsl_panel_nine_slice;

/* ---- section: declarations ----------------------------------------------- */

/*! @brief default fonts.
 *
 *  @remark declared internally in @ref fsl_text_init().
 */
FSLAPI extern fsl_font fsl_font_buf[FSL_FONT_INDEX_COUNT];

/* ---- section: signatures ------------------------------------------------- */

/*! @brief load font from file at `file_name` or at `font->path`.
 *
 *  1. allocate memory for `font->buf` and load file contents into it in binary format.
 *  2. allocate memory for `font->bitmap` and render glyphs onto it.
 *  3. generate square texture of diameter `resolution` * 16 and bake bitmap onto it.
 *
 *  @param resolution font size (font atlas cell diameter).
 *  @param name font name.
 *  @param file_name font file name.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_font_init(fsl_font *font, u32 resolution, const str *name, const str *file_name);

FSLAPI void fsl_font_free(fsl_font *font);

/*! @brief start text rendering batch.
 *
 *  @param size font height in pixels.
 *
 *  @param length pre-allocate buffer for string (if 0, @ref FSL_STRING_MAX is allocated).
 *  @param clear clear the framebuffer before rendering.
 *
 *  @remark disables @ref GL_DEPTH_TEST, @ref fsl_text_stop() re-enables it.
 *  @remark can re-allocate `fbo` with `multisample` setting used in @ref fsl_text_init().
 */
FSLAPI void fsl_text_start(fsl_font *font, f32 size, u64 length, b8 clear);

/*! @brief push string's glyph metrics, position, alignment and color to internal text queue.
 *
 *  @param align_x enum @ref fsl_text_alignment:
 *      FSL_TEXT_ALIGN_RIGHT.
 *      FSL_TEXT_ALIGN_CENTER.
 *      FSL_TEXT_ALIGN_LEFT.
 *
 *  @param align_y enum @ref fsl_text_alignment:
 *      FSL_TEXT_ALIGN_TOP.
 *      FSL_TEXT_ALIGN_CENTER.
 *      FSL_TEXT_ALIGN_BOTTOM.
 *
 *  @param window_x restrict text width to specified window size.
 *
 *  @param color text color, format: 0xrrggbbaa.
 *
 *  @remark default alignment is top left (0, 0).
 *
 *  @remark the macros @ref fsl_color_hex_to_v4(), @ref fsl_color_v4_to_hex() can be
 *  used to convert from u32 hex color to v4f32 color and vice-versa.
 *
 *  @remark can be called multiple times within a text rendering batch.
 */
FSLAPI void fsl_text_push(const str *text, f32 pos_x, f32 pos_y, i8 align_x, i8 align_y,
        i32 window_x, u32 color);

/*! @brief render text to framebuffer.
 *
 *  @param shadow_color shadow color if `shadow` is `TRUE`, can be empty,
 *  format: 0xrrggbbaa.
 *
 *  @remark the macros @ref fsl_color_hex_to_v4(), @ref fsl_color_v4_to_hex() can be
 *  used to convert from u32 hex color to v4f32 color and vice-versa.
 *
 *  @remark can be called multiple times within a text rendering batch.
 */
FSLAPI void fsl_text_render(b8 shadow, u32 shadow_color);

/*! @brief stop text rendering batch.
 *
 *  @remark enables @ref GL_DEPTH_TEST.
 */
FSLAPI void fsl_text_stop(void);

/*! @brief get total string height of current rendering batch.
 *
 *  @remark call before @ref fsl_text_render.
 */
FSLAPI f32 fsl_get_text_height(void);

/*! @brief initialize ui.
 *
 *  @param multisample turn on multisampling.
 *
 *  @remark must be called after @ref fsl_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_ui_init(void);

/*! @brief start ui rendering batch.
 *
 *  @param nine_slice use a nine_slice shader
 *  (ui elements with separate edge and corner slices of a texture).
 *
 *  @param clear clear the framebuffer before rendering.
 *
 *  @remark disables @ref GL_DEPTH_TEST, @ref fsl_ui_stop() re-enables it.
 *  @remark can re-allocate `fbo` with `multisample` setting used in @ref fsl_ui_init().
 */
FSLAPI void fsl_ui_start(b8 nine_slice, b8 clear);

/*! @brief push default engine panel onto internal panel buffer.
 */
FSLAPI void fsl_ui_push_panel(i32 pos_x, i32 pos_y, i32 size_x, i32 size_y, u32 tint);

FSLAPI void fsl_ui_render(void);
FSLAPI void fsl_ui_draw(fsl_texture *texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint);

FSLAPI void fsl_ui_draw_nine_slice(fsl_texture *texture, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size, u32 tint);

/*! @remark enables @ref GL_DEPTH_TEST.
 */
FSLAPI void fsl_ui_stop(void);

FSLAPI void fsl_ui_free(void);

/*! @brief make a 9-slice panel.
 */
FSLAPI fsl_panel_nine_slice fsl_get_nine_slice(v2i32 texture_size, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size);

#endif /* FSL_UI_H */
