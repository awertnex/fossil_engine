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
 *  @file ui.h
 *
 *  @brief main UI module header; creating menus and drawing UI elements.
 */

#ifndef FSL_UI_H
#define FSL_UI_H

#include "../common/api.h"
#include "../common/types.h"
#include "../assets/asset_types.h"

#include "ui_types.h"
#include "ui_element.h"

/* ---- section: definitions ------------------------------------------------ */

#define FSL_UI_PANEL_GAP_DEFAULT 10
#define FSL_UI_PANEL_PADDING_DEFAULT 10
#define FSL_UI_SLICE_SIZE_DEFAULT 8

/*!
 *  @brief one 9-slice panel.
 */
typedef struct fsl_ui_panel_9_slice
{
    fsl_ui_transform slice[9];
} fsl_ui_panel_9_slice;

typedef struct fsl_ui_drawable_quad
{
    v2f32 uv_pos;
    v2f32 uv_size;
    v2f32 pos;
    v2f32 size;
} fsl_ui_drawable_quad;

/*!
 *  @brief one 9-slice panel.
 *
 *  -- DEPRECATED IN v0.10.0-beta --;
 */
typedef struct fsl_panel_nine_slice
{
    fsl_ui_drawable_quad slice[9];
} fsl_panel_nine_slice;

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @brief start text rendering batch.
 *
 *  @param size font height, in pixels.
 *
 *  @param length pre-allocate buffer for string (if 0, @ref FSL_STRING_MAX is allocated).
 *  @param clear clear the framebuffer before rendering.
 *
 *  @remark disables @ref GL_DEPTH_TEST, @ref fsl_text_stop() re-enables it.
 *  @remark can re-allocate `fbo` with `multisample` setting used in @ref fsl_text_init().
 */
FSLAPI void fsl_text_start(fsl_font *font, f32 size, u64 length, b8 clear);

/*!
 *  @brief push string's glyph metrics, position, alignment and color to internal text queue.
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

/*!
 *  @brief render text to framebuffer.
 *
 *  @param shadow_color shadow color (if `shadow` is `TRUE`) (optional),
 *  format: 0xrrggbbaa.
 *
 *  @remark the macros @ref fsl_color_hex_to_v4(), @ref fsl_color_v4_to_hex() can be
 *  used to convert from `u32` hex color to `v4f32` color and vice-versa.
 *
 *  @remark can be called multiple times within a text rendering batch.
 */
FSLAPI void fsl_text_render(b8 shadow, u32 shadow_color);

/*!
 *  @brief get total string height of current rendering batch.
 *
 *  @remark call before @ref fsl_text_render().
 */
FSLAPI f32 fsl_get_text_height(void);

/*!
 *  @brief initialize ui.
 *
 *  @remark must be called after @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_ui_init(void);

/*!
 *  @brief start ui rendering batch.
 *
 *  @param nine_slice DEPRECATED IN v0.10.1-dev --; use a nine_slice shader
 *  (ui elements with separate edge and corner slices of a texture).
 *
 *  @param clear clear the currently bound framebuffer before rendering.
 *
 *  @remark disable @ref GL_DEPTH_TEST, @ref fsl_ui_stop() re-enables it.
 */
FSLAPI void fsl_ui_start(b8 nine_slice, b8 clear);

/*!
 *  @brief push default engine panel onto internal panel buffer.
 */
FSLAPI void fsl_ui_push_panel(i32 pos_x, i32 pos_y, i32 size_x, i32 size_y, u32 tint);

/*!
 *  @brief draw a texture as a UI element.
 *
 *  @param offset_x -- DEPRECATED IN v0.10.1-beta --;
 *  @param offset_y -- DEPRECATED IN v0.10.1-beta --;
 *  @param align_x -- DEPRECATED IN v0.10.1-beta --;
 *  @param align_y -- DEPRECATED IN v0.10.1-beta --;
 *
 *  @remark if `size_x` is 0, `texture->size.x` is used, and likewise for `size_y`.
 */
FSLAPI void fsl_ui_draw(fsl_texture *texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint);

/*!
 *  -- DEPRECATED IN v0.10.1-beta --;
 */
FSLAPI void fsl_ui_draw_nine_slice(fsl_texture *texture, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size, u32 tint);

/*!
 *  @remark enable @ref GL_DEPTH_TEST.
 */
FSLAPI void fsl_ui_stop(void);

FSLAPI void fsl_ui_free(void);

/*!
 *  @brief make a 9-slice panel.
 */
FSLAPI fsl_panel_nine_slice fsl_get_nine_slice_internal(fsl_texture *texture,
        i32 pos_x, i32 pos_y, i32 size_x, i32 size_y, i32 slice_size);

#endif /* FSL_UI_H */
