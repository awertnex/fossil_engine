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
 *  @file engine.c
 *
 *  @brief engine init, running, close, windowing, opengl loading and default assets.
 */

#include "../common/engine_info.h"
#include "../common/common_values.h"
#include "../common/config.h"
#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../common/session.h"
#include "../common/types.h"
#include "../assets/assets.h"
#include "../input/input_internal.h"
#include "../logger/logger.h"
#include "../logger/logger_messages_internal.h"
#include "../memory/memory.h"
#include "../math/math.h"
#include "../math/math_internal.h"
#include "../math/vector.h"
#include "../shaders/shader_types.h"
#include "../ui/ui.h"

#include "../h/dir.h"
#include "../h/process.h"
#include "../h/time.h"

#include "engine.h"
#include "engine_assets.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#   define STB_IMAGE_WRITE_IMPLEMENTATION
#   include "../external/stb_image_write.h"
#pragma GCC diagnostic pop /* ignored "-Wpedantic" */

#include <stdio.h>
#include <sys/stat.h>
#include <inttypes.h>

/* ---- section: declarations ----------------------------------------------- */

struct /* fsl_core */
{
    struct /* request */
    {
        b8 skip_mouse_delta;
        b8 take_screenshot;
        b8 close_engine;
    } request;

    struct /* flag */
    {
        b8 active;
        b8 glfw_initialized;
    } flag;

    struct /* ubo */
    {
        GLuint ndc_scale;
    } ubo;

    /*!
     *  @brief global fbo for rendering mostly ui elements.
     *
     *  @remark initialized in @ref fsl_engine_init().
     */
    fsl_fbo fbo;

    /*!
     *  @brief global fbo for rendering mostly ui elements, multisampled.
     *
     *  @remark initialized in @ref fsl_engine_init().
     */
    fsl_fbo fbo_msaa;

    /*!
     *  @brief function to bind a final framebuffer to draw to based on anti-aliasing setting.
     *
     *  @remark initialized in @ref fsl_engine_init().
     */
    void (*fbo_bind)(void);

    /*!
     *  @brief function to draw onto a final framebuffer based on anti-aliasing setting.
     *
     * @remark initialized in @ref fsl_engine_init().
     */
    void (*fbo_blit)(GLuint fbo);
} fsl_core;

fsl_engine_session FSL_SESSION = {0};
u32 fsl_err = FSL_ERR_SUCCESS;
fsl_render render_internal = {0};

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

    fsl_core.flag.active = TRUE;

    if (!FSL_SESSION.init_time)
    {
        FSL_SESSION.init_time = fsl_get_time_raw_usec();
        fsl_get_time_nsec(); /* initialize start time */
        fsl_get_time_nsecf(); /* initialize start time */
    }

    if (fsl_logger_init(argc, argv, is_release) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (
            fsl_mem_arena_init(&mem_arena_internal,
                "fsl_engine_init().mem_arena_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_sub_data_internal,
                "fsl_engine_init().mem_arena_sub_data_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_name_internal,
                "fsl_engine_init().mem_arena_name_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_name_id_internal,
                "fsl_engine_init().mem_arena_name_id_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_file_internal,
                "fsl_engine_init().mem_arena_file_internal") != FSL_ERR_SUCCESS ||
            fsl_mem_arena_init(&mem_arena_path_internal,
                "fsl_engine_init().mem_arena_path_internal") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (FSL_SESSION.bin_root == NULL)
    {
        if (fsl_get_path_bin_root(&FSL_SESSION.bin_root) != FSL_ERR_SUCCESS)
            goto cleanup;
        fsl_change_dir(FSL_SESSION.bin_root);
    }

    if (
            fsl_glfw_init(flags & FSL_FLAG_MULTISAMPLE) != FSL_ERR_SUCCESS ||
            fsl_window_init(title, size_x, size_y) != FSL_ERR_SUCCESS ||
            fsl_glad_init() != FSL_ERR_SUCCESS ||
            fsl_engine_assets_init() != FSL_ERR_SUCCESS)
        goto cleanup;

    glfwSwapInterval(0);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    /* ---- engine framebuffers --------------------------------------------- */

    if (fsl_fbo_init(&fsl_core.fbo, render_internal.size.x, render_internal.size.y,
                &fsl_mesh_unit_quad, FALSE, 4) != FSL_ERR_SUCCESS)
        goto cleanup;
    if (fsl_fbo_init(&fsl_core.fbo_msaa, render_internal.size.x, render_internal.size.y,
                NULL, TRUE, 4) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (flags & FSL_FLAG_MULTISAMPLE)
    {
        fsl_core.fbo_bind = &fbo_bind_msaa_internal;
        fsl_core.fbo_blit = &fbo_blit_msaa_internal;
    }
    else
    {
        fsl_core.fbo_bind = &fbo_bind_internal;
        fsl_core.fbo_blit = &fbo_blit_internal;
    }

    /* ---- engine uniform buffers ------------------------------------------ */

    glGenBuffers(1, &fsl_core.ubo.ndc_scale);
    glBindBuffer(GL_UNIFORM_BUFFER, fsl_core.ubo.ndc_scale);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(v2f32), &render_internal.ndc_scale, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBufferBase(GL_UNIFORM_BUFFER, FSL_SHADER_BUFFER_BINDING_UBO_NDC_SCALE,
            fsl_core.ubo.ndc_scale);

    if (noise_init_internal() != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_ui_init() != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_engine_close();
    return fsl_err;
}

b8 fsl_engine_running(void (*callback_framebuffer_size)(i32 size_x, i32 size_y))
{
    static u64 time_last = 0;
    if (fsl_core.flag.active == FALSE ||
            fsl_core.request.close_engine == TRUE ||
            glfwWindowShouldClose(render_internal.window))
        return FALSE;

    if (fsl_update_render_settings(callback_framebuffer_size) != FSL_ERR_SUCCESS)
    {
        LOGWARNING(fsl_err, 0,
                MSG_UPDATE_RENDER_SETTINGS_FAIL);
    }

    render_internal.time = fsl_get_time_nsec();
    if (!time_last)
        time_last = render_internal.time;
    render_internal.time_delta = render_internal.time - time_last;
    time_last = render_internal.time;

    glfwSwapBuffers(render_internal.window);
    glfwPollEvents();

    input_mouse_movement_update_internal();
    if (fsl_core.request.skip_mouse_delta)
    {
        fsl_core.request.skip_mouse_delta = FALSE;
        render_internal.mouse_delta.x = 0.0;
        render_internal.mouse_delta.y = 0.0;
    }

    input_key_states_update_internal();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    return TRUE;
}

u32 fsl_update_render_settings(void (*callback_framebuffer_size)(i32 size_x, i32 size_y))
{
    static v2i32 size = {0};

    glfwGetFramebufferSize(render_internal.window, &size.x, &size.y);

    if (size.x != render_internal.size.x || size.y != render_internal.size.y)
    {
        fsl_request_skip_mouse_delta();

        render_internal.size.x = size.x;
        render_internal.size.y = size.y;
        glViewport(0, 0, size.x, size.y);

        render_internal.ndc_scale.x = 2.0f / size.x;
        render_internal.ndc_scale.y = 2.0f / size.y;

        glBindBuffer(GL_UNIFORM_BUFFER, fsl_core.ubo.ndc_scale);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(v2f32), &render_internal.ndc_scale);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        if (fsl_mem_realloc((void*)&render_internal.screen_buf, size.x * size.y * FSL_COLOR_CHANNELS_RGB,
                    "fsl_update_render_settings().render_internal.screen_buf") != FSL_ERR_SUCCESS)
            return fsl_err;

        fsl_fbo_realloc(&fsl_core.fbo, render_internal.size.x, render_internal.size.y, FALSE, 4);
        fsl_fbo_realloc(&fsl_core.fbo_msaa,render_internal.size.x, render_internal.size.y, TRUE, 4);

        if (callback_framebuffer_size)
            callback_framebuffer_size(size.x, size.y);
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_request_engine_close(void)
{
    fsl_core.request.close_engine = TRUE;

    if (render_internal.window)
        glfwSetWindowShouldClose(render_internal.window, GL_TRUE);
}

void fsl_engine_close(void)
{
    u32 fsl_err_temp = fsl_err;

    if (fsl_core.flag.active == FALSE)
        return;

    fsl_core.flag.active = FALSE;

    noise_free_internal();
    fsl_ui_free();
    fsl_assets_free();

    fsl_fbo_free(&fsl_core.fbo);
    fsl_fbo_free(&fsl_core.fbo_msaa);

    if (render_internal.window)
        glfwDestroyWindow(render_internal.window);

    if (fsl_core.flag.glfw_initialized)
    {
        fsl_core.flag.glfw_initialized = 0;
        glfwTerminate();
    }

    fsl_mem_arena_free(&mem_arena_path_internal, "fsl_engine_close().mem_arena_path_internal");
    fsl_mem_arena_free(&mem_arena_file_internal, "fsl_engine_close().mem_arena_file_internal");
    fsl_mem_arena_free(&mem_arena_name_id_internal, "fsl_engine_close().mem_arena_name_id_internal");
    fsl_mem_arena_free(&mem_arena_name_internal, "fsl_engine_close().mem_arena_name_internal");
    fsl_mem_free((void*)&render_internal.screen_buf, render_internal.size.x * render_internal.size.y * FSL_COLOR_CHANNELS_RGB,
            "fsl_engine_close().render_internal.screen_buf");
    fsl_mem_free((void*)&FSL_SESSION.bin_root, FSL_PATH_CAP, "fsl_engine_close().FSL_SESSION.bin_root");
    fsl_mem_arena_free(&mem_arena_sub_data_internal, "fsl_engine_close().mem_arena_sub_data_internal");
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

    fsl_core.flag.glfw_initialized = 1;

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
    if (title)
        snprintf(render_internal.title, FSL_ID_CAP, "%s", title);
    else
        fsl_engine_get_string(render_internal.title, FSL_ENGINE_STR_INDEX_TITLE);

    if (size_x)
        render_internal.size.x = fsl_clamp_i32(size_x, FSL_RENDER_WIDTH_MIN, FSL_RENDER_WIDTH_MAX);
    else
        render_internal.size.x = FSL_RENDER_WIDTH_DEFAULT;

    if (size_y)
        render_internal.size.y = fsl_clamp_i32(size_y, FSL_RENDER_HEIGHT_MIN, FSL_RENDER_HEIGHT_MAX);
    else
        render_internal.size.y = FSL_RENDER_HEIGHT_DEFAULT;

    render_internal.window = glfwCreateWindow(render_internal.size.x, render_internal.size.y,
                render_internal.title, NULL, NULL);

    if (!render_internal.window)
    {
        LOGFATAL(FSL_ERR_WINDOW_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_FATAL("Initialize Window or OpenGL Context"));
        return fsl_err;
    }

    glfwMakeContextCurrent(render_internal.window);
    glfwSetWindowSizeLimits(render_internal.window,
            FSL_RENDER_WIDTH_MIN, FSL_RENDER_HEIGHT_MIN,
            FSL_RENDER_WIDTH_MAX, FSL_RENDER_HEIGHT_MAX);

    if (render_internal.size.x && render_internal.size.y)
    {
        render_internal.ndc_scale.x = 2.0f / render_internal.size.x;
        render_internal.ndc_scale.y = 2.0f / render_internal.size.y;
    }

    if (fsl_mem_alloc((void*)&render_internal.screen_buf,
                render_internal.size.x * render_internal.size.y * FSL_COLOR_CHANNELS_RGB,
                "fsl_window_init().render_internal.screen_buf") != FSL_ERR_SUCCESS)
        return fsl_err;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_glad_init(void)
{
    str str_engine_version[FSL_ID_CAP] = {0};

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

fsl_render *fsl_render_get(void)
{
    return &render_internal;
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

    render_internal = *r;
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_request_skip_mouse_delta(void)
{
    fsl_core.request.skip_mouse_delta = TRUE;
}

void fsl_request_screenshot(void)
{
    fsl_core.request.take_screenshot = TRUE;
}

u32 fsl_process_screenshot_request(const str *dir_screenshots, const str *special_text)
{
    if (fsl_core.request.take_screenshot)
    {
        fsl_core.request.take_screenshot = FALSE;
        return take_screenshot_internal(dir_screenshots, special_text);
    }
    return FSL_ERR_SUCCESS;
}

static u32 take_screenshot_internal(const str *dir_screenshots, const str *special_text)
{
    u64 i = 0;
    str str_time[FSL_TIME_STRING_MAX] = {0};
    str str_special_text[FSL_ID_CAP] = {0};
    str file_name[FSL_PATH_CAP] = {0};
    str file_name_full[FSL_PATH_CAP] = {0};
    str str_dir_screenshots[FSL_PATH_CAP] = {0};

    if (fsl_is_dir_exists(dir_screenshots, FALSE) != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_SCREENSHOT_FAIL,
                FSL_FLAG_LOG_CMD,
                MSG_DIR_NOT_FOUND_ACTION("Take Screenshot", dir_screenshots));
        return fsl_err;
    }

    snprintf(str_dir_screenshots, FSL_PATH_CAP, "%s", dir_screenshots);
    fsl_check_slash(str_dir_screenshots);
    fsl_posix_slash(str_dir_screenshots);

    fsl_get_time_str(str_time, "%F_%H-%M-%S");

    if (special_text[0])
        snprintf(str_special_text, FSL_ID_CAP, "_%s", special_text);

    snprintf(file_name, FSL_PATH_CAP, "%s%s", str_dir_screenshots, str_time);

    for (i = 0; i < FSL_SCREENSHOT_RATE_MAX; ++i)
    {
        snprintf(file_name_full, FSL_PATH_CAP, "%s_%"PRIu64"%s.png", file_name, i, str_special_text);
        if (fsl_is_file_exists(file_name_full, FALSE) != FSL_ERR_SUCCESS)
        {
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, render_internal.size.x, render_internal.size.y,
                    GL_RGB, GL_UNSIGNED_BYTE, render_internal.screen_buf);

            stbi_flip_vertically_on_write(TRUE);
            LOGINFO(FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("Screenshot: %s\n", file_name_full));

            stbi_write_png(file_name_full, render_internal.size.x, render_internal.size.y, FSL_COLOR_CHANNELS_RGB,
                    render_internal.screen_buf, render_internal.size.x * FSL_COLOR_CHANNELS_RGB);

            fsl_err = FSL_ERR_SUCCESS;
            return fsl_err;
        }
    }

    LOGERROR(FSL_ERR_SCREENSHOT_FAIL,
            FSL_FLAG_LOG_CMD,
            MSG_RATE_LIMIT_EXCEED_ACTION("Take Screenshot"));
    return fsl_err;
}

void fsl_fbo_bind(void)
{
    fsl_core.fbo_bind();
}

static void fbo_bind_internal(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fsl_core.fbo.fbo);
}

static void fbo_bind_msaa_internal(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fsl_core.fbo_msaa.fbo);
}

void fsl_fbo_blit(GLuint fbo)
{
    fsl_core.fbo_blit(fbo);
}

static void fbo_blit_internal(GLuint fbo)
{
    fsl_shader_program *shader_unit_quad = fsl_mem_handle_get(fsl_shader_buf);
    shader_unit_quad = &shader_unit_quad[FSL_SHADER_INDEX_UNIT_QUAD];

    glUseProgram(shader_unit_quad->asset.id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, fsl_core.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

static void fbo_blit_msaa_internal(GLuint fbo)
{
    fsl_shader_program *shader_unit_quad = fsl_mem_handle_get(fsl_shader_buf);
    shader_unit_quad = &shader_unit_quad[FSL_SHADER_INDEX_UNIT_QUAD];

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fsl_core.fbo_msaa.fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fsl_core.fbo.fbo);
    glBlitFramebuffer(0, 0, render_internal.size.x, render_internal.size.y, 0, 0,
            render_internal.size.x, render_internal.size.y, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glUseProgram(shader_unit_quad->asset.id);
    glBindVertexArray(fsl_mesh_unit_quad.vao);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glBindTexture(GL_TEXTURE_2D, fsl_core.fbo.color_buf);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
