#ifndef ENGINE_TEXT_H
#define ENGINE_TEXT_H

#include <engine/include/stb_truetype.h>

#include "common.h"
#include "core.h"
#include "types.h"

typedef struct Glyph
{
    v2i32 scale;
    v2i32 bearing;
    i32 advance;
    i32 x0, y0, x1, y1;
    v2f32 texture_sample;
    b8 loaded;
} Glyph;

typedef struct Font
{
    /*! @brief font name, initialized in @ref font_init() if empty.
     */
    str name[NAME_MAX];

    /*! @brief font file name, initialized in @ref font_init() if `NULL`.
     */
    str *path;

    u32 resolution; /* glyph bitmap diameter in bytes */
    f32 char_size; /* for font atlas sampling */

    /*! @brief glyphs highest points' deviation from baseline.
     */
    i32 ascent;

    /*! @brief glyphs lowest points' deviation from baseline.
     */
    i32 descent;

    i32 line_gap;
    i32 line_height;
    f32 size; /* global font size, for text uniformity */
    v2i32 scale; /* biggest glyph bounding box size in font units */

    /*! @brief used by @ref stbtt_InitFont().
     */
    stbtt_fontinfo info;

    /*! @brief font file contents.
     *
     *  used by @ref stbtt_InitFont().
     */
    u8 *buf;

    u64 buf_len; /* `buf` size in bytes */
    u8 *bitmap; /* memory block for all font glyph bitmaps */

    GLuint id; /* used by @ref glGenTextures() */
    Glyph glyph[GLYPH_MAX];
} Font;

/*! @brief default fonts.
 *
 *  @remark declared internally in @ref text_init().
 */
FSLAPI extern Font engine_font[ENGINE_FONT_COUNT];

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
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 font_init(Font *font, u32 resolution, const str *name, const str *file_name);

FSLAPI void font_free(Font *font);

/*! @brief init text rendering settings.
 *
 *  @param multisample turn on multisampling.
 *
 *  @return non-zero on failure and @ref engine_err is set accordingly.
 */
FSLAPI u32 text_init(u32 resolution, b8 multisample);

/*! @brief start text rendering batch.
 *
 *  @param size font height in pixels.
 *
 *  @param fbo fbo to draw text to, if `NULL`, internal fbo is used
 *  (must then call @ref text_fbo_blit() to blit result onto desired fbo).
 *
 *  @param length pre-allocate buffer for string (if 0, @ref STRING_MAX is allocated).
 *  @param clear clear the framebuffer before rendering.
 *
 *  @remark disables @ref GL_DEPTH_TEST, @ref text_stop() re-enables it.
 *  @remark can re-allocate `fbo` with `multisample` setting used in @ref text_init().
 */
FSLAPI void text_start(Font *font, f32 size, u64 length, FBO *fbo, b8 clear);

/*! @brief push string's glyph metrics, position, alignment and color to internal text queue.
 *
 *  @param align_x enum @ref TextAlignment:
 *      TEXT_ALIGN_RIGHT.
 *      TEXT_ALIGN_CENTER.
 *      TEXT_ALIGN_LEFT.
 *
 *  @param align_y enum @ref TextAlignment:
 *      TEXT_ALIGN_TOP.
 *      TEXT_ALIGN_CENTER.
 *      TEXT_ALIGN_BOTTOM.
 *
 *  @param color text color, format: 0xrrggbbaa.
 *
 *  @remark default alignment is top left (0, 0).
 *
 *  @remark the macros @ref color_hex_to_v4(), @ref color_v4_to_hex() can be
 *  used to convert from u32 hex color to v4f32 color and vice-versa.
 *
 *  @remark can be called multiple times within a text rendering batch.
 */
FSLAPI void text_push(const str *text, f32 pos_x, f32 pos_y, i8 align_x, i8 align_y, u32 color);

/*! @brief render text to framebuffer.
 *
 *  @param shadow_color shadow color if `shadow` is `TRUE`, can be empty,
 *  format: 0xrrggbbaa.
 *
 *  @remark the macros @ref color_hex_to_v4(), @ref color_v4_to_hex() can be
 *  used to convert from u32 hex color to v4f32 color and vice-versa.
 *
 *  @remark can be called multiple times within a text rendering batch.
 */
FSLAPI void text_render(b8 shadow, u32 shadow_color);

/*! @brief stop text rendering batch.
 *
 *  @remark enables @ref GL_DEPTH_TEST.
 */
FSLAPI void text_stop(void);

/*! @brief blit rendered text onto `fbo`.
 */
FSLAPI void text_fbo_blit(GLuint fbo);

FSLAPI void text_free(void);

#endif /* ENGINE_TEXT_H */
