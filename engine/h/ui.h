#ifndef ENGINE_UI_H
#define ENGINE_UI_H

#include "core.h"
#include "types.h"

#define PANEL_GAP_DEFAULT 10
#define PANEL_PADDING_DEFAULT 10

/*! @brief initialize ui.
 *
 *  @param multisample = turn on multisampling.
 *
 *  @remark must be called after 'core.h/engine_init()'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 ui_init(b8 multisample);

/*! @brief start ui rendering batch.
 *
 *  @param clear = clear the framebuffer before rendering.
 *
 *  @param fbo = fbo to draw ui to, if NULL, internal fbo is used
 *  (must then call 'ui_fbo_blit()' to blit result onto desired fbo).
 *
 *  @param nine_slice = use a nine_slice shader
 *  (ui elements with separate edge and corner slices of a texture).
 *
 *  @remark disables 'GL_DEPTH_TEST', 'ui_stop()' re-enables it.
 *  @remark can re-allocate 'fbo' with 'multisample' setting used in 'ui_init()'.
 */
void ui_start(FBO *fbo, b8 nine_slice, b8 clear);

void ui_render(void);

void ui_draw(Texture texture, i32 pos_x, i32 pos_y, i32 size_x, i32 size_y,
        f32 offset_x, f32 offset_y, i32 align_x, i32 align_y, u32 tint);
/*!
 *  @remark enables 'GL_DEPTH_TEST'.
 */
void ui_stop(void);

/*! @brief blit rendered ui onto 'fbo.
 */
void ui_fbo_blit(GLuint fbo);

void ui_free(void);

#endif /* ENGINE_UI_H */
