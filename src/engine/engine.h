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
 *  @file engine.h
 *
 *  @brief main engine module header; engine init, running, close, windowing, opengl loading.
 */

#ifndef FSL_ENGINE_H
#define FSL_ENGINE_H

#include "../common/api.h"
#include "../common/limits.h"
#include "../common/types.h"
#include "../assets/asset_types.h"
#include "../math/vector.h"

#define GLFW_INCLUDE_NONE
#include "../external/glfw3.h"

typedef struct fsl_render fsl_render;

enum fsl_engine_string_index
{
    FSL_ENGINE_STR_INDEX_TITLE, /* "ENGINE_NAME: ENGINE_VERSION" */
    FSL_ENGINE_STR_INDEX_VERSION,
    FSL_ENGINE_STR_INDEX_COUNT
}; /* fsl_engine_string_index */

struct fsl_render
{
    GLFWwindow *window;
    char title[FSL_ID_CAP];
    v2i32 size;

    /*!
     *  @brief conversion from world-space to screen-space.
     *
     *  @remark read-only, updated internally in @ref fsl_running().
     */
    v2f32 ndc_scale;

    GLFWimage icon;
    v2f64 mouse_pos;
    v2f64 mouse_delta;
    u64 time;
    u64 time_delta;

    /*!
     *  @brief for reading screen pixels back to RAM (e.g., screenshots).
     */
    u8 *screen_buf;
}; /* fsl_render */

/* ---- section: declarations ----------------------------------------------- */

/*!
 *  @internal
 *
 *  @brief engine's default render.
 *
 *  @remark declared and initialized internally.
 */
extern fsl_render render_internal;

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @brief initialize engine stuff.
 *
 *  - set 'GLFW' error callback.
 *  - call @ref fsl_change_dir() to change current working directory to the main process'.
 *  - call @ref fsl_logger_init(), @ref fsl_glfw_init(), @ref fsl_window_init(),
 *    @ref fsl_glad_init(), @ref fsl_assets_init() and @ref fsl_ui_init().
 *
 *  @param argc number of arguments in `argv` (if `argv` provided).
 *  @param argv used for logger log level (optional).
 *
 *  @param flags enum @ref fsl_flag.
 *
 *  @param title window/application title (if `NULL`, default title is used)
 *  (@ref fsl_window_init() parameter).
 *
 *  @param size_x window width (if 0, @ref FSL_RENDER_WIDTH_DEFAULT is used)
 *  (@ref fsl_window_init() parameter).
 *
 *  @param size_y window height (if 0, @ref FSL_RENDER_HEIGHT_DEFAULT is used)
 *  (@ref fsl_window_init() parameter).
 *
 *  @remark @ref fsl_flag.FSL_FLAG_RELEASE_BUILD can be overridden with these args in `argv`:
 *      logfatal:   only output fatal logs (least verbose).
 *      logerror:   only output <= error logs.
 *      logwarn:    only output <= warning logs.
 *      loginfo:    only output <= info logs (default).
 *      logdebug:   only output <= debug logs.
 *      logtrace:   only output <= trace logs (most verbose).
 *
 *  @note @ref fsl_engine_close() will be called on failure.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_engine_init(int argc, char **argv, const str *title,
        i32 size_x, i32 size_y, u64 flags);

/*!
 *  @brief engine main loop check.
 *
 *  - update @ref fsl_render.time and @ref fsl_render.time_delta of the currently bound @ref fsl_render.
 *
 *  @return `TRUE` unless @ref glfwWindowShouldClose() returns `FALSE` or
 *  @ref fsl_request_engine_close() has been called.
 */
FSLAPI b8 fsl_engine_running(void (*callback_framebuffer_size)(i32, i32));

/*!
 *  @brief update render settings (e.g., render size).
 *
 *  - update @ref fsl_render.size of the currently bound @ref fsl_render to window size.
 *  - update @ref fsl_render.ndc_scale of the currently bound @ref fsl_render.
 *
 *  @remark called automatically from @ref fsl_engine_running().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_update_render_settings(void (*callback_framebuffer_size)(i32, i32));

/*!
 *  @brief send engine close request to then be processed by @ref fsl_engine_running().
 */
FSLAPI void fsl_request_engine_close(void);

/*!
 *  @brief free all engine's internal resources.
 *
 *  free logger, assets, internal memory arenas, destroy window (if not `NULL`)
 *  and terminate 'GLFW'.
 */
FSLAPI void fsl_engine_close(void);

/*!
 *  @brief get engine-specific string no longer than @ref FSL_ID_CAP bytes.
 *
 *  @param dst pointer to buffer to store string.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_engine_get_string(str *dst, enum fsl_engine_string_index type);

/*!
 *  @brief initialize 'GLFW'.
 *
 *  @param multisample turn on multisampling.
 *
 *  @remark called automatically from @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_glfw_init(b8 multisample);

/*!
 *  @brief initialize a new 'GLFW' window for the currently bound @ref fsl_render.
 *
 *  @param title window/application title (if `NULL`, default title is used).
 *  @param size_x window width (if 0, @ref FSL_RENDER_WIDTH_DEFAULT is used).
 *  @param size_y window height (if 0, @ref FSL_RENDER_HEIGHT_DEFAULT is used).
 *
 *  @remark called automatically from @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_window_init(const str *title, i32 size_x, i32 size_y);

/*!
 *  @brief initialize 'OpenGL' function loader 'GLAD'.
 *
 *  @remark called automatically from @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_glad_init(void);

/*!
 *  @brief get address of currently bound @ref fsl_render.
 *
 *  get metadata like render size, current process time, delta time and screen buffer.
 */
FSLAPI fsl_render *fsl_render_get(void);

/*!
 *  @brief switch engine's currently bound @ref fsl_render to `r`.
 */
FSLAPI u32 fsl_change_render(fsl_render *r);

/*!
 *  @remark send screenshot request to then be processed by @ref fsl_process_screenshot_request().
 *
 *  can be called from anywhere, then @ref fsl_process_screenshot_request() can be called
 *  to take the screenshot at the end of the render loop.
 */
FSLAPI void fsl_request_screenshot(void);

/*!
 *  @remark take screenshot requested by @ref fsl_request_screenshot() and save into dir at `dir_screenshots`.
 *
 *  @param dir_screenshots directory to save screenshot to.
 *  @param special_text string appended to file name, before file extension.
 *
 *  @remark if directory not found, screenshot is still saved at @ref fsl_render.screen_buf
 *  of the currently bound @ref fsl_render.
 *
 *  @remark this function is a thin wrapper around an internal, heavier function that carries all the logic.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_process_screenshot_request(const str *dir_screenshots, const str *special_text);

/*!
 *  @brief bind internal fbo for rendering.
 */
FSLAPI void fsl_fbo_bind(void);

/*!
 *  @brief blit rendered internal fbo (e.g., text, ui elements) onto `fbo`.
 */
FSLAPI void fsl_fbo_blit(GLuint fbo);

#endif /* FSL_ENGINE_H */
