/*  @file core.h
 *
 *  @brief engine init, running, close, windowing, opengl loading.
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
 *  limitations under the License.
 */

#ifndef FSL_CORE_H
#define FSL_CORE_H

#include "common.h"
#include "assets.h"
#include "limits.h"
#include "types.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#   include <deps/glad/glad.h>

#   define GLFW_INCLUDE_NONE
#   include <deps/glfw3.h>

#   include <deps/stb_image.h>
#   include <deps/stb_image_write.h>
#pragma GCC diagnostic pop /* ignored "-Wpedantic" */

/* ---- section: definitions ------------------------------------------------ */

typedef struct render       fsl_render;
typedef struct camera       fsl_camera;
typedef struct projection   fsl_projection;
typedef struct core         fsl_core;

struct render
{
    GLFWwindow *window;
    char title[NAME_MAX];
    v2i32 size;

    /*! @brief conversion from world-space to screen-space.
     *
     *  @remark read-only, updated internally in @ref fsl_running().
     */
    v2f32 ndc_scale;

    GLFWimage icon;
    v2f64 mouse_pos;
    v2f64 mouse_delta;
    u64 time;
    u64 time_delta;

    /*! @brief for reading screen pixels back to RAM (e.g., screenshots).
     */
    u8 *screen_buf;
}; /* render */

struct camera
{
    v3f64 pos;
    f64 roll, pitch, yaw;
    f64 sin_roll, sin_pitch, sin_yaw;
    f64 cos_roll, cos_pitch, cos_yaw;
    f32 fovy;
    f32 fovy_smooth;
    f32 ratio;
    f32 far;
    f32 near;
    f32 zoom;
}; /* camera */

struct projection
{
    m4f32 target;
    m4f32 translation;
    m4f32 rotation;
    m4f32 orientation;
    m4f32 view;
    m4f32 projection;
    m4f32 perspective;
}; /* projection */

struct core
{
    struct /* flag */
    {
        b8 active;
        b8 glfw_initialized;
        b8 request_screenshot;
        b8 request_engine_close;
    } flag;

    struct /* ubo */
    {
        GLuint ndc_scale;
    } ubo;

    /*! @brief global fbo for rendering mostly ui elements.
     *
     *  @remark initialized in @ref fsl_engine_init().
     */
    fsl_fbo fbo;

    /*! @brief global fbo for rendering mostly ui elements, multisampled.
     *
     *  @remark initialized in @ref fsl_engine_init().
     */
    fsl_fbo fbo_msaa;

    /*! @brief function to bind a final framebuffer to draw to based on anti-aliasing setting.
     *
     *  @remark initialized in @ref fsl_engine_init().
     */
    void (*fbo_bind)(void);

    /*! @brief function to draw onto a final framebuffer based on anti-aliasing setting.
     *
     * @remark initialized in @ref fsl_engine_init().
     */
    void (*fbo_blit)(GLuint fbo);
}; /* core */

/* ---- section: declarations ----------------------------------------------- */

/*! @brief global core module.
 *
 *  @remark declared and initialized internally.
 */
extern fsl_core fsl_core_internal;

/*! @brief engine's default render.
 *
 *  @remark declared and initialized internally.
 */
FSLAPI extern fsl_render *render;

/* ---- section: signatures ------------------------------------------------- */

/*! @brief initialize engine stuff.
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

/*! @brief engine main loop check.
 *
 *  - update @ref fsl_render.time and @ref fsl_render.time_delta of the currently bound `fsl_render`.
 *
 *  @return `TRUE` unless @ref glfwWindowShouldClose() returns `FALSE` or
 *  @ref fsl_request_engine_close() has been called.
 */
FSLAPI b8 fsl_engine_running(void (*callback_framebuffer_size)(i32, i32));

/*! @brief update render settings (e.g., render size).
 *
 *  - update @ref fsl_render.size of the currently bound `fsl_render` to window size.
 *  - update @ref fsl_render.ndc_scale of the currently bound `fsl_render`.
 *
 *  @remark called automatically from @ref fsl_engine_running().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_update_render_settings(void (*callback_framebuffer_size)(i32, i32));

/*! @brief send engine close request to then be processed by @ref fsl_engine_running().
 */
FSLAPI void fsl_request_engine_close(void);

/*! @brief free all engine's internal resources.
 *
 *  free logger, assets, internal memory arenas, destroy window (if not `NULL`)
 *  and terminate 'GLFW'.
 */
FSLAPI void fsl_engine_close(void);

/*! @brief get engine-specific string no longer than @ref FSL_ID_CAP bytes.
 *
 *  @param dst pointer to buffer to store string.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_engine_get_string(str *dst, enum fsl_engine_string_index type);

/*! @brief initialize 'GLFW'.
 *
 *  @param multisample turn on multisampling.
 *
 *  @remark called automatically from @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_glfw_init(b8 multisample);

/*! @brief initialize a new 'GLFW' window for the currently bound `fsl_render`.
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

/*! @brief initialize 'OpenGL' function loader 'GLAD'.
 *
 *  @remark called automatically from @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_glad_init(void);

/*! @brief switch engine's current bound `fsl_render` to `r`.
 */
FSLAPI u32 fsl_change_render(fsl_render *r);

/*! @remark send screenshot request to then be processed by @ref fsl_process_screenshot_request().
 *
 *  can be called from anywhere, then @ref fsl_process_screenshot_request() can be called
 *  to take the screenshot at the end of the render loop.
 */
FSLAPI void fsl_request_screenshot(void);

/*! @remark take screenshot requested by @ref fsl_request_screenshot() and save into dir at `dir_screenshots`.
 *
 *  @param dir_screenshots directory to save screenshot to.
 *  @param special_text string appended to file name, before file extension.
 *
 *  @remark if directory not found, screenshot is still saved at @ref fsl_render.screen_buf
 *  of the currently bound `fsl_render`.
 *
 *  @remark this function is a thin wrapper around an internal, heavier function that carries all the logic.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_process_screenshot_request(const str *dir_screenshots, const str *special_text);

/*! @brief blit rendered internal fbo (e.g., text, ui elements) onto `fbo`.
 */
FSLAPI void fsl_fbo_blit(GLuint fbo);

/*! @brief update `sine` and `cosine` of camera roll, pitch and yaw.
 *
 *  @param roll enable/disable roll rotation.
 *
 *  @remark rotation limits:
 *      roll:  [  0, 360].
 *      pitch: [-90,  90].
 *      yaw:   [  0, 360].
 */
FSLAPI void fsl_update_camera_movement(fsl_camera *camera, b8 roll);

/*! @brief make perspective projection matrices from camera parameters.
 *
 *  - setup camera matrices for Z-up, right-handed coordinates and vertical fov (fovy):
 *      - +X: forward.
 *      - +Y: left.
 *      - +Z: up.
 *
 *  @param roll enable/disable roll rotation.
 */
FSLAPI void fsl_update_projection_perspective(fsl_camera camera, fsl_projection *projection, b8 roll);

/*! @brief get camera look-at angles from camera position and target position.
 *
 *  assign vertical angle to `pitch` and horizontal angle to `yaw`.
 */
FSLAPI void fsl_get_camera_lookat_angles(v3f64 camera_pos, v3f64 target, f64 *pitch, f64 *yaw);

#endif /* FSL_CORE_H */
