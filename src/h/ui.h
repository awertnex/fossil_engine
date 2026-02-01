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

/*  ui.h - everything about drawing ui elements
 */

#ifndef FSL_UI_H
#define FSL_UI_H

#include "common.h"
#include "core.h"
#include "types.h"

#define FSL_PANEL_GAP_DEFAULT 10
#define FSL_PANEL_PADDING_DEFAULT 10

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

/*! @brief initialize ui.
 *
 *  @param multisample turn on multisampling.
 *
 *  @remark must be called after @ref fsl_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_ui_init(b8 multisample);

/*! @brief start ui rendering batch.
 *
 *  @param fbo fbo to draw ui to, if `NULL`, internal fbo is used
 *  (must then call @ref fsl_ui_fbo_blit() to blit result onto desired fbo).
 *
 *  @param nine_slice use a nine_slice shader
 *  (ui elements with separate edge and corner slices of a texture).
 *
 *  @param clear clear the framebuffer before rendering.
 *
 *  @remark disables @ref GL_DEPTH_TEST, @ref fsl_ui_stop() re-enables it.
 *  @remark can re-allocate `fbo` with `multisample` setting used in @ref fsl_ui_init().
 */
FSLAPI void fsl_ui_start(fsl_fbo *fbo, b8 nine_slice, b8 clear);

FSLAPI void fsl_ui_render(void);
FSLAPI void fsl_ui_draw(fsl_texture *texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint);

FSLAPI void fsl_ui_draw_nine_slice(fsl_texture *texture, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size, u32 tint);

/*! @remark enables @ref GL_DEPTH_TEST.
 */
FSLAPI void fsl_ui_stop(void);

/*! @brief blit rendered ui onto `fbo`.
 */
FSLAPI void fsl_ui_fbo_blit(GLuint fbo);

FSLAPI void fsl_ui_free(void);

/*! @brief make a 9-slice panel.
 */
FSLAPI fsl_panel_nine_slice fsl_get_nine_slice(v2i32 texture_size, i32 pos_x, i32 pos_y,
        i32 size_x, i32 size_y, i32 slice_size);

#endif /* FSL_UI_H */
