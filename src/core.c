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
 *  @file core.c
 *
 *  @brief engine init, running, close, windowing, opengl loading.
 */

#include "h/common.h"

#include "h/core.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/input.h"
#include "logger/log.h"
#include "h/math.h"
#include "memory/memory.h"
#include "h/process.h"
#include "h/shaders.h"
#include "h/string.h"
#include "h/time.h"
#include "h/ui.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#   define STB_IMAGE_WRITE_IMPLEMENTATION
#   include <deps/stb_image_write.h>
#pragma GCC diagnostic pop /* ignored "-Wpedantic" */

#include <stdio.h>
#include <sys/stat.h>
#include <inttypes.h>

/* ---- section: declarations ----------------------------------------------- */

u64 fsl_init_time = 0;
str *FSL_DIR_PROC_ROOT = NULL;
u32 fsl_err = FSL_ERR_SUCCESS;
fsl_core fsl_core_internal = {0};

/*!
 *  @remark initialized in @ref fsl_engine_init().
 */
static fsl_render fsl_render_internal = {0};

fsl_render *render = &fsl_render_internal;

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @internal
 *
 *  @brief engine's default error callback for 'GLFW'.
 */
static void glfw_callback_error(int error, const char* message)
{
    (void)error;
    LOGERROR(FSL_ERR_GLFW, 0, fsl_logger_stringf("GLFW: %s\n", message));
}

/*!
 *  @internal
 *
 *  @brief take screenshot and save it into dir at `dir_screenshots`.
 *
 *  save pixel data as RGB into @ref fsl_render.screen_buf.
 *
 *  @param dir_screenshots directory to save screenshot to.
 *
 *  @param special_text text appended to file name before file extension (usually level name),
 *  if provided, an underscore (`_`) will be inserted to separate `special_text` from default name.
 *
 *  @remark if directory not found, screenshot is still saved at @ref fsl_render.screen_buf of the
 *  currently bound `fsl_render` till next screenshot.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
static u32 take_screenshot_internal(const str *dir_screenshots, const str *special_text);

static void fbo_bind_internal(void);
static void fbo_bind_msaa_internal(void);
static void fbo_blit_internal(GLuint fbo);
static void fbo_blit_msaa_internal(GLuint fbo);

/* ---- section: init ------------------------------------------------------- */

u32 fsl_engine_init(int argc, char **argv, const str *title,
        i32 size_x, i32 size_y, u64 flags)
{
    b8 is_release = flags & FSL_FLAG_RELEASE_BUILD;

    fsl_core_internal.flag.active = TRUE;

    if (!fsl_init_time)
    {
        fsl_init_time = fsl_get_time_raw_usec();
        fsl_get_time_nsec(); /* initialize start time */
        fsl_get_time_nsecf(); /* initialize start time */
    }

    if (fsl_logger_init(argc, argv, is_release) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (
            fsl_mem_arena_init(&mem_arena_internal,
                "fsl_engine_init().mem_arena_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_name_internal,
                "fsl_engine_init().mem_arena_name_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_name_id_internal,
                "fsl_engine_init().mem_arena_name_id_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_file_internal,
                "fsl_engine_init().mem_arena_file_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_path_internal,
                "fsl_engine_init().mem_arena_path_internal") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (FSL_DIR_PROC_ROOT == NULL)
    {
        if (fsl_get_path_bin_root(&FSL_DIR_PROC_ROOT) != FSL_ERR_SUCCESS)
            goto cleanup;
        fsl_change_dir(FSL_DIR_PROC_ROOT);
    }

    if (
            fsl_glfw_init(flags & FSL_FLAG_MULTISAMPLE) != FSL_ERR_SUCCESS ||
            fsl_window_init(title, size_x, size_y) != FSL_ERR_SUCCESS ||
            fsl_glad_init() != FSL_ERR_SUCCESS ||
            fsl_assets_init() != FSL_ERR_SUCCESS)
        goto cleanup;

    glfwSwapInterval(0);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    if (flags & FSL_FLAG_MULTISAMPLE)
    {
        fsl_core_internal.fbo_bind = &fbo_bind_msaa_internal;
        fsl_core_internal.fbo_blit = &fbo_blit_msaa_internal;
    }
    else
    {
        fsl_core_internal.fbo_bind = &fbo_bind_internal;
        fsl_core_internal.fbo_blit = &fbo_blit_internal;
    }

    glGenBuffers(1, &fsl_core_internal.ubo.ndc_scale);
    glBindBuffer(GL_UNIFORM_BUFFER, fsl_core_internal.ubo.ndc_scale);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(v2f32), &render->ndc_scale, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, FSL_SHADER_BUFFER_BINDING_UBO_NDC_SCALE,
            fsl_core_internal.ubo.ndc_scale);

    if (fsl_ui_init() != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_engine_close();
    return fsl_err;
}

b8 fsl_engine_running(void (*callback_framebuffer_size)(i32, i32))
{
    static u64 time_last = 0;
    if (fsl_core_internal.flag.active == FALSE ||
            fsl_core_internal.flag.request_engine_close == TRUE ||
            glfwWindowShouldClose(render->window))
        return FALSE;

    if (fsl_update_render_settings(callback_framebuffer_size) != FSL_ERR_SUCCESS)
    {
        LOGWARNING(fsl_err, 0,
                MSG_UPDATE_RENDER_SETTINGS_FAIL);
    }

    render->time = fsl_get_time_nsec();
    if (!time_last) time_last = render->time;
    render->time_delta = render->time - time_last;
    time_last = render->time;

    glfwSwapBuffers(render->window);
    glfwPollEvents();
    fsl_update_mouse_movement();
    fsl_update_key_states();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return TRUE;
}

u32 fsl_update_render_settings(void (*callback_framebuffer_size)(i32, i32))
{
    static v2i32 size = {0};

    glfwGetFramebufferSize(render->window, &size.x, &size.y);

    if (size.x != render->size.x || size.y != render->size.y)
    {
        render->size.x = size.x;
        render->size.y = size.y;
        glViewport(0, 0, size.x, size.y);

        render->ndc_scale.x = 2.0f / size.x;
        render->ndc_scale.y = 2.0f / size.y;

        glBindBuffer(GL_UNIFORM_BUFFER, fsl_core_internal.ubo.ndc_scale);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(v2f32), &render->ndc_scale);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        if (fsl_mem_realloc((void*)&render->screen_buf, size.x * size.y * FSL_COLOR_CHANNELS_RGB,
                    "fsl_update_render_settings().render.screen_buf") != FSL_ERR_SUCCESS)
            return fsl_err;

        fsl_fbo_realloc(&fsl_core_internal.fbo, FALSE, 4);
        fsl_fbo_realloc(&fsl_core_internal.fbo_msaa, TRUE, 4);

        if (callback_framebuffer_size)
            callback_framebuffer_size(size.x, size.y);
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_request_engine_close(void)
{
    fsl_core_internal.flag.request_engine_close = TRUE;

    if (render && render->window)
        glfwSetWindowShouldClose(render->window, GL_TRUE);
}

void fsl_engine_close(void)
{
    u32 fsl_err_temp = fsl_err;

    if (fsl_core_internal.flag.active == FALSE)
        return;

    fsl_core_internal.flag.active = FALSE;

    fsl_ui_free();
    fsl_assets_free();

    if (render->window)
        glfwDestroyWindow(render->window);

    if (fsl_core_internal.flag.glfw_initialized)
    {
        fsl_core_internal.flag.glfw_initialized = 0;
        glfwTerminate();
    }

    fsl_mem_arena_free(&mem_arena_path_internal, "fsl_engine_close().mem_arena_path_internal");
    fsl_mem_arena_free(&mem_arena_file_internal, "fsl_engine_close().mem_arena_file_internal");
    fsl_mem_arena_free(&mem_arena_name_id_internal, "fsl_engine_close().mem_arena_name_id_internal");
    fsl_mem_arena_free(&mem_arena_name_internal, "fsl_engine_close().mem_arena_name_internal");
    fsl_mem_free((void*)&render->screen_buf, render->size.x * render->size.y * FSL_COLOR_CHANNELS_RGB,
            "fsl_engine_close().render->screen_buf");
    fsl_mem_free((void*)&FSL_DIR_PROC_ROOT, PATH_MAX, "fsl_engine_close().FSL_DIR_PROC_ROOT");
    fsl_mem_arena_free(&mem_arena_internal, "fsl_engine_close().mem_arena_internal");
    fsl_logger_close();
    fsl_err = fsl_err_temp;
}

u32 fsl_engine_get_string(str *dst, enum fsl_engine_string_index type)
{
    if (!dst)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, 0,
                MSG_POINTER_NULL_ACTION("Get Engine String"));
        return fsl_err;
    }

    switch(type)
    {
        case FSL_ENGINE_STR_INDEX_TITLE:
            snprintf(dst, FSL_ID_CAP, FSL_ENGINE_NAME": %u.%u.%u%s",
                    FSL_ENGINE_VERSION_MAJOR,
                    FSL_ENGINE_VERSION_MINOR,
                    FSL_ENGINE_VERSION_PATCH,
                    FSL_ENGINE_VERSION_BUILD);
            break;

        case FSL_ENGINE_STR_INDEX_VERSION:
            snprintf(dst, FSL_ID_CAP, "%u.%u.%u%s",
                    FSL_ENGINE_VERSION_MAJOR,
                    FSL_ENGINE_VERSION_MINOR,
                    FSL_ENGINE_VERSION_PATCH,
                    FSL_ENGINE_VERSION_BUILD);
            break;

        default:
            fsl_err = FSL_ERR_OUT_OF_BOUNDS;
            break;
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_glfw_init(b8 multisample)
{
    glfwSetErrorCallback(glfw_callback_error);

    if (!glfwInit())
    {
        LOGFATAL(FSL_ERR_GLFW_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_FATAL("Initialize GLFW"));
        return fsl_err;
    }

    fsl_core_internal.flag.glfw_initialized = 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (multisample)
        glfwWindowHint(GLFW_SAMPLES, 4);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_window_init(const str *title, i32 size_x, i32 size_y)
{
    if (title) snprintf(render->title, NAME_MAX, "%s", title);
    else fsl_engine_get_string(render->title, FSL_ENGINE_STR_INDEX_TITLE);
    if (size_x) render->size.x = fsl_clamp_i32(size_x, FSL_RENDER_WIDTH_MIN, FSL_RENDER_WIDTH_MAX);
    else render->size.x = FSL_RENDER_WIDTH_DEFAULT;
    if (size_y) render->size.y = fsl_clamp_i32(size_y, FSL_RENDER_HEIGHT_MIN, FSL_RENDER_HEIGHT_MAX);
    else render->size.y = FSL_RENDER_HEIGHT_DEFAULT;

    render->window = glfwCreateWindow(render->size.x, render->size.y, render->title, NULL, NULL);

    if (!render->window)
    {
        LOGFATAL(FSL_ERR_WINDOW_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_FATAL("Initialize Window or OpenGL Context"));
        return fsl_err;
    }

    glfwMakeContextCurrent(render->window);
    glfwSetWindowSizeLimits(render->window,
            FSL_RENDER_WIDTH_MIN, FSL_RENDER_HEIGHT_MIN,
            FSL_RENDER_WIDTH_MAX, FSL_RENDER_HEIGHT_MAX);

    if (render->size.x && render->size.y)
    {
        render->ndc_scale.x = 2.0f / render->size.x;
        render->ndc_scale.y = 2.0f / render->size.y;
    }

    if (fsl_mem_alloc((void*)&render->screen_buf, render->size.x * render->size.y * FSL_COLOR_CHANNELS_RGB,
                "fsl_window_init().render.screen_buf") != FSL_ERR_SUCCESS)
        return fsl_err;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_glad_init(void)
{
    str str_engine_version[NAME_MAX] = {0};

    fsl_engine_get_string(str_engine_version, FSL_ENGINE_STR_INDEX_VERSION);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOGFATAL(FSL_ERR_GLAD_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_FATAL("Initialize GLAD"));
        return fsl_err;
    }

    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 3))
    {
        LOGFATAL(FSL_ERR_GL_VERSION_NOT_SUPPORT,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_GL_VERSION_NOT_SUPPORT(GLVersion.major, GLVersion.minor));
        return fsl_err;
    }

    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf(FSL_ENGINE_NAME ":  %s\n", str_engine_version));

    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("OpenGL:         %s\n", glGetString(GL_VERSION)));

    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("GLSL:           %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION)));

    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("Vendor:         %s\n", glGetString(GL_VENDOR)));

    LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("Renderer:       %s\n", glGetString(GL_RENDERER)));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_change_render(fsl_render *r)
{
    if (r == NULL)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, 0,
                MSG_POINTER_NULL_ACTION("Change Render"));
        return fsl_err;
    }

    if (r->window)
        glfwMakeContextCurrent(r->window);
    else
    {
        LOGWARNING(FSL_ERR_WINDOW_NOT_FOUND, 0,
                MSG_WINDOW_NOT_FOUND_WARNING);
        return fsl_err;
    }

    render = r;
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_request_screenshot(void)
{
    fsl_core_internal.flag.request_screenshot = TRUE;
}

u32 fsl_process_screenshot_request(const str *dir_screenshots, const str *special_text)
{
    if (fsl_core_internal.flag.request_screenshot)
    {
        fsl_core_internal.flag.request_screenshot = FALSE;
        return take_screenshot_internal(dir_screenshots, special_text);
    }
    return FSL_ERR_SUCCESS;
}

static u32 take_screenshot_internal(const str *dir_screenshots, const str *special_text)
{
    u64 i = 0;
    str str_time[FSL_TIME_STRING_MAX] = {0};
    str str_special_text[NAME_MAX] = {0};
    str file_name[PATH_MAX] = {0};
    str file_name_full[PATH_MAX] = {0};
    str str_dir_screenshots[PATH_MAX] = {0};

    if (fsl_is_dir_exists(dir_screenshots, FALSE) != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_SCREENSHOT_FAIL,
                FSL_FLAG_LOG_CMD,
                MSG_DIR_NOT_FOUND_ACTION("Take Screenshot", dir_screenshots));
        return fsl_err;
    }

    snprintf(str_dir_screenshots, PATH_MAX, "%s", dir_screenshots);
    fsl_check_slash(str_dir_screenshots);
    fsl_posix_slash(str_dir_screenshots);

    fsl_get_time_str(str_time, "%F_%H-%M-%S");

    if (special_text[0])
        snprintf(str_special_text, NAME_MAX, "_%s", special_text);

    snprintf(file_name, PATH_MAX, "%s%s", str_dir_screenshots, str_time);

    for (i = 0; i < FSL_SCREENSHOT_RATE_MAX; ++i)
    {
        snprintf(file_name_full, PATH_MAX, "%s_%"PRIu64"%s.png", file_name, i, str_special_text);
        if (fsl_is_file_exists(file_name_full, FALSE) != FSL_ERR_SUCCESS)
        {
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, render->size.x, render->size.y, GL_RGB, GL_UNSIGNED_BYTE, render->screen_buf);

            stbi_flip_vertically_on_write(TRUE);
            LOGINFO(FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("Screenshot: %s\n", file_name_full));

            stbi_write_png(file_name_full, render->size.x, render->size.y, FSL_COLOR_CHANNELS_RGB,
                    render->screen_buf, render->size.x * FSL_COLOR_CHANNELS_RGB);

            fsl_err = FSL_ERR_SUCCESS;
            return fsl_err;
        }
    }

    LOGERROR(FSL_ERR_SCREENSHOT_FAIL,
            FSL_FLAG_LOG_CMD,
            MSG_RATE_LIMIT_EXCEED_ACTION("Take Screenshot"));
    return fsl_err;
}

static void fbo_bind_internal(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fsl_core_internal.fbo.fbo);
}

static void fbo_bind_msaa_internal(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fsl_core_internal.fbo_msaa.fbo);
}

void fsl_fbo_blit(GLuint fbo)
{
    fsl_core_internal.fbo_blit(fbo);
}

static void fbo_blit_internal(GLuint fbo)
{
    fsl_shader_program *shader_unit_quad = fsl_mem_handle_get_i(fsl_shader_program, fsl_shader_buf, FSL_SHADER_INDEX_UNIT_QUAD);
    glUseProgram(shader_unit_quad->asset.id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, fsl_core_internal.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static void fbo_blit_msaa_internal(GLuint fbo)
{
    fsl_shader_program *shader_unit_quad = fsl_mem_handle_get_i(fsl_shader_program, fsl_shader_buf, FSL_SHADER_INDEX_UNIT_QUAD);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fsl_core_internal.fbo_msaa.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fsl_core_internal.fbo.fbo);
    glBlitFramebuffer(0, 0, render->size.x, render->size.y, 0, 0,
            render->size.x, render->size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glUseProgram(shader_unit_quad->asset.id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, fsl_core_internal.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

/* ---- section: camera ----------------------------------------------------- */

void fsl_update_camera_movement(fsl_camera *camera, b8 roll)
{
    if (roll)
    {
        camera->roll = fmod(camera->roll, FSL_CAMERA_RANGE_MAX);
        if (camera->roll < 0.0) camera->roll += FSL_CAMERA_RANGE_MAX;
    }
    else camera->roll = 0.0;

    camera->pitch = fsl_clamp_f64(camera->pitch, -FSL_CAMERA_ANGLE_MAX, FSL_CAMERA_ANGLE_MAX);
    camera->yaw = fmod(camera->yaw, FSL_CAMERA_RANGE_MAX);
    if (camera->yaw < 0.0) camera->yaw += FSL_CAMERA_RANGE_MAX;

    camera->sin_roll = sin(camera->roll * FSL_DEG2RAD);
    camera->cos_roll = cos(camera->roll * FSL_DEG2RAD);
    camera->sin_pitch = sin(camera->pitch * FSL_DEG2RAD);
    camera->cos_pitch = cos(camera->pitch * FSL_DEG2RAD);
    camera->sin_yaw = sin(camera->yaw * FSL_DEG2RAD);
    camera->cos_yaw = cos(camera->yaw * FSL_DEG2RAD);
}

void fsl_update_projection_perspective(fsl_camera camera, fsl_projection *projection, b8 roll)
{
    const f32 SROL = camera.sin_roll;
    const f32 CROL = camera.cos_roll;
    const f32 SPCH = camera.sin_pitch;
    const f32 CPCH = camera.cos_pitch;
    const f32 SYAW = camera.sin_yaw;
    const f32 CYAW = camera.cos_yaw;
    f32 ratio = 0.0f;
    f32 fovy = 0.0f;
    f32 far = 0.0f;
    f32 near = 0.0f;
    f32 clip = 0.0f;
    f32 offset = 0.0f;
    fsl_projection noprojection = {0};
    m4f32 mat_roll = {0};
    m4f32 mat_pitch = {0};
    m4f32 mat_yaw = {0};

    *projection = noprojection;

    ratio = camera.ratio;
    fovy = 1.0f / tanf((camera.fovy_smooth / 2.0f) * FSL_DEG2RAD);
    far = camera.far;
    near = camera.near;
    clip = -(far + near) / (far - near);
    offset = -(2.0f * far * near) / (far - near);

    /* ---- target ---------------------------------------------------------- */

    projection->target.a11 = 1.0f;
    projection->target.a22 = 1.0f;
    projection->target.a33 = 1.0f;
    projection->target.a41 = -CYAW * -CPCH;
    projection->target.a42 = SYAW * -CPCH;
    projection->target.a43 = -SPCH;
    projection->target.a44 = 1.0f;

    /* ---- translation ----------------------------------------------------- */

    projection->translation.a11 = 1.0f;
    projection->translation.a22 = 1.0f;
    projection->translation.a33 = 1.0f;
    projection->translation.a41 = -camera.pos.x;
    projection->translation.a42 = -camera.pos.y;
    projection->translation.a43 = -camera.pos.z;
    projection->translation.a44 = 1.0f;

    /* ---- rotation: yaw --------------------------------------------------- */

    mat_yaw.a11 = CYAW;
    mat_yaw.a12 = SYAW;
    mat_yaw.a21 = -SYAW;
    mat_yaw.a22 = CYAW;
    mat_yaw.a33 = 1.0f;
    mat_yaw.a44 = 1.0f;

    /* ---- rotation: pitch ------------------------------------------------- */

    mat_pitch.a11 = CPCH;
    mat_pitch.a13 = SPCH;
    mat_pitch.a22 = 1.0f;
    mat_pitch.a31 = -SPCH;
    mat_pitch.a33 = CPCH;
    mat_pitch.a44 = 1.0f;

    projection->rotation = fsl_matrix_multiply(mat_yaw, mat_pitch);

    /* ---- rotation: roll -------------------------------------------------- */

    if (roll)
    {
        mat_roll.a11 = 1.0f;
        mat_roll.a22 = CROL;
        mat_roll.a23 = SROL;
        mat_roll.a32 = -SROL;
        mat_roll.a33 = CROL;
        mat_roll.a44 = 1.0f;

        projection->rotation = fsl_matrix_multiply(projection->rotation, mat_roll);
    }

    /* ---- orientation: z-up ----------------------------------------------- */

    projection->orientation.a13 = -1.0f;
    projection->orientation.a21 = -1.0f;
    projection->orientation.a32 = 1.0f;
    projection->orientation.a44 = 1.0f;

    /* ---- view ------------------------------------------------------------ */

    projection->view = fsl_matrix_multiply(projection->translation,
                fsl_matrix_multiply(projection->rotation, projection->orientation));

    /* ---- projection ------------------------------------------------------ */

    projection->projection.a11 = fovy / ratio;
    projection->projection.a22 = fovy;
    projection->projection.a33 = clip;
    projection->projection.a34 = -1.0f;
    projection->projection.a43 = offset;

    projection->perspective = fsl_matrix_multiply(projection->view, projection->projection);
}

void fsl_get_camera_lookat_angles(v3f64 camera_pos, v3f64 target, f64 *pitch, f64 *yaw)
{
    v3f64 direction = {0};

    camera_pos.x -= target.x;
    camera_pos.y -= target.y;
    camera_pos.z -= target.z;

    direction = fsl_normalize_v3f64(camera_pos);

    *pitch = atan2(direction.z, sqrt(direction.x * direction.x + direction.y * direction.y));
    *yaw = atan2(-direction.y, direction.x);
}
