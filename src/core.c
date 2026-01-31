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

/*  core.c - engine init, running, close, windowing, opengl loading
 */

#include "h/common.h"

#include "h/core.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/input.h"
#include "h/logger.h"
#include "h/math.h"
#include "h/memory.h"
#include "h/process.h"
#include "h/shaders.h"
#include "h/string.h"
#include "h/time.h"
#include "h/text.h"
#include "h/ui.h"

#define STB_IMAGE_IMPLEMENTATION
#include <deps/stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <deps/stb_image_write.h>

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>

/* ---- section: declarations ----------------------------------------------- */

u64 fsl_init_time = 0;
str *FSL_DIR_PROC_ROOT = NULL;
u32 fsl_err = FSL_ERR_SUCCESS;

static struct /* fsl_flag */
{
    u64 active: 1;
    u64 glfw_initialized: 1;
    u64 request_screenshot: 1;
} fsl_flag;

static fsl_render fsl_render_internal =
{
    .size = (v2i32){FSL_RENDER_WIDTH_DEFAULT, FSL_RENDER_HEIGHT_DEFAULT},
};

fsl_render *render = &fsl_render_internal;

/*! -- INTERNAL USE ONLY --;
 */
static struct /* fsl_ubo */
{
    GLuint ndc_scale;
} fsl_ubo;

static f32 vbo_data_unit_quad[] =
{
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
};

/* ---- section: signatures ------------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief default error callback for 'GLFW'.
 */
static void glfw_callback_error(int error, const char* message)
{
    (void)error;
    _LOGERROR(FSL_ERR_GLFW, 0, "GLFW: %s\n", message);
}

/*! -- INTERNAL USE ONLY --;
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
static u32 _fsl_take_screenshot(const str *dir_screenshots, const str *special_text);

/* ---- section: init ------------------------------------------------------- */

u32 fsl_engine_init(int argc, char **argv, const str *_log_dir, const str *title,
        i32 size_x, i32 size_y, fsl_render *_render, u64 flags)
{
    u32 i = 0;

    fsl_flag.active = 1;
    fsl_engine_get_string(render->title, FSL_STR_INDEX_ENGINE_TITLE);

    if (fsl_logger_init(argc, argv, flags & FSL_FLAG_RELEASE_BUILD, _log_dir,
                TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (!fsl_init_time)
    {
        fsl_init_time = fsl_get_time_raw_usec();
        fsl_get_time_nsec(); /* initialize start time */
        fsl_get_time_nsecf(); /* initialize start time */
    }

    if (!FSL_DIR_PROC_ROOT)
    {
        if (fsl_get_path_bin_root(&FSL_DIR_PROC_ROOT) != FSL_ERR_SUCCESS)
            goto cleanup;
        fsl_change_dir(FSL_DIR_PROC_ROOT);
    }

    glfwSetErrorCallback(glfw_callback_error);

    if (_render && fsl_change_render(_render) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (
            fsl_glfw_init(flags & FSL_FLAG_MULTISAMPLE) != FSL_ERR_SUCCESS ||
            fsl_window_init(title, size_x, size_y) != FSL_ERR_SUCCESS ||
            fsl_glad_init() != FSL_ERR_SUCCESS)
        goto cleanup;

    glfwSwapInterval(0);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    if (fsl_fbo_init(NULL, &fsl_mesh_unit_quad, FALSE, 0) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (flags & FSL_FLAG_LOAD_DEFAULT_SHADERS)
    {
        for (i = 0; i < FSL_SHADER_INDEX_COUNT; ++i)
            if (fsl_shader_program_init(FSL_DIR_NAME_SHADERS, &fsl_shader_buf[i]) != FSL_ERR_SUCCESS)
                goto cleanup;

        glGenBuffers(1, &fsl_ubo.ndc_scale);
        glBindBuffer(GL_UNIFORM_BUFFER, fsl_ubo.ndc_scale);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(v2f32), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, FSL_SHADER_BUFFER_BINDING_UBO_NDC_SCALE,
                fsl_ubo.ndc_scale);
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_engine_close();
    return fsl_err;
}

b8 fsl_engine_running(void (*callback_framebuffer_size)(i32, i32))
{
    static u64 time_last = 0;
    if (glfwWindowShouldClose(render->window) || !fsl_flag.active)
        return FALSE;

    glfwSwapBuffers(render->window);
    glfwPollEvents();
    fsl_update_mouse_movement();
    fsl_update_key_states();

    if (fsl_update_render_settings(callback_framebuffer_size) != FSL_ERR_SUCCESS)
        _LOGWARNING(fsl_err, 0,
                "%s\n", "Something Went Wrong While Updating Render Settings");

    render->time = fsl_get_time_nsec();
    if (!time_last) time_last = render->time;
    render->time_delta = render->time - time_last;
    time_last = render->time;

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

        glBindBuffer(GL_UNIFORM_BUFFER, fsl_ubo.ndc_scale);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(v2f32), &render->ndc_scale);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        if (fsl_mem_realloc((void*)&render->screen_buf, size.x * size.y * FSL_COLOR_CHANNELS_RGB,
                    "fsl_update_render_settings().render.screen_buf") != FSL_ERR_SUCCESS)
            return fsl_err;

        if (callback_framebuffer_size)
            callback_framebuffer_size(size.x, size.y);
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_request_engine_close(void)
{
    fsl_flag.active = 0;

    if (render && render->window)
        glfwSetWindowShouldClose(render->window, GL_TRUE);
}

void fsl_engine_close(void)
{
    u32 i = 0;
    u32 fsl_err_temp = fsl_err;

    if (!fsl_flag.active)
        return;

    fsl_flag.active = FALSE;

    for (i = 0; i < FSL_FONT_INDEX_COUNT; ++i)
        fsl_font_free(&fsl_font_buf[i]);
    for (i = 0; i < FSL_TEXTURE_INDEX_COUNT; ++i)
        fsl_texture_free(&fsl_texture_buf[i]);
    for (i = 0; i < FSL_SHADER_INDEX_COUNT; ++i)
        fsl_shader_program_free(&fsl_shader_buf[i]);

    fsl_mesh_free(&fsl_mesh_unit_quad);
    fsl_text_free();
    fsl_ui_free();

    if (render->window)
        glfwDestroyWindow(render->window);

    if (fsl_flag.glfw_initialized)
    {
        fsl_flag.glfw_initialized = 0;
        glfwTerminate();
    }

    fsl_mem_free((void*)&render->screen_buf, render->size.x * render->size.y * FSL_COLOR_CHANNELS_RGB,
                "fsl_free().render.screen_buf");
    fsl_mem_free((void*)&FSL_DIR_PROC_ROOT, strlen(FSL_DIR_PROC_ROOT), "fsl_close().FSL_DIR_PROC_ROOT");

    fsl_logger_close();
    fsl_err = fsl_err_temp;
}

u32 fsl_engine_get_string(str *dst, enum fsl_string_index type)
{
    if (!dst)
    {
        _LOGERROR(FSL_ERR_POINTER_NULL, 0,
                "Failed to Get String, Pointer NULL\n");
        return fsl_err;
    }

    switch(type)
    {
        case FSL_STR_INDEX_ENGINE_TITLE:
            snprintf(dst, NAME_MAX, FSL_ENGINE_NAME": %u.%u.%u%s",
                    FSL_ENGINE_VERSION_MAJOR,
                    FSL_ENGINE_VERSION_MINOR,
                    FSL_ENGINE_VERSION_PATCH,
                    FSL_ENGINE_VERSION_BUILD);
            break;

        case FSL_STR_INDEX_ENGINE_VERSION:
            snprintf(dst, NAME_MAX, "%u.%u.%u%s",
                    FSL_ENGINE_VERSION_MAJOR,
                    FSL_ENGINE_VERSION_MINOR,
                    FSL_ENGINE_VERSION_PATCH,
                    FSL_ENGINE_VERSION_BUILD);
            break;
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_glfw_init(b8 multisample)
{
    if (!glfwInit())
    {
        _LOGFATAL(FSL_ERR_GLFW_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Initialize GLFW, Process Aborted");
        return fsl_err;
    }

    fsl_flag.glfw_initialized = 1;

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
    if (size_x) render->size.x = fsl_clamp_i32(size_x, FSL_RENDER_WIDTH_MIN, FSL_RENDER_WIDTH_MAX);
    if (size_y) render->size.y = fsl_clamp_i32(size_y, FSL_RENDER_HEIGHT_MIN, FSL_RENDER_HEIGHT_MAX);

    render->window = glfwCreateWindow(render->size.x, render->size.y, render->title, NULL, NULL);

    if (!render->window)
    {
        _LOGFATAL(FSL_ERR_WINDOW_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Initialize Window or OpenGL Context, Process Aborted");
        return fsl_err;
    }

    glfwMakeContextCurrent(render->window);
    glfwSetWindowSizeLimits(render->window,
            FSL_RENDER_WIDTH_MIN, FSL_RENDER_HEIGHT_MIN,
            FSL_RENDER_WIDTH_MAX, FSL_RENDER_HEIGHT_MAX);

    if (fsl_mem_alloc((void*)&render->screen_buf, render->size.x * render->size.y * FSL_COLOR_CHANNELS_RGB,
                "fsl_window_init().render.screen_buf") != FSL_ERR_SUCCESS)
        return fsl_err;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_glad_init(void)
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        _LOGFATAL(FSL_ERR_GLAD_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "%s\n", "Failed to Initialize GLAD, Process Aborted");
        return fsl_err;
    }

    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 3))
    {
        _LOGFATAL(FSL_ERR_GL_VERSION_NOT_SUPPORT,
                FSL_FLAG_LOG_NO_VERBOSE,
                "OpenGL 4.3+ Required, Current Version '%d.%d', Process Aborted\n",
                GLVersion.major, GLVersion.minor);
        return fsl_err;
    }

    _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            "OpenGL:    %s\n", glGetString(GL_VERSION));

    _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            "GLSL:      %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            "Vendor:    %s\n", glGetString(GL_VENDOR));

    _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            "Renderer:  %s\n", glGetString(GL_RENDERER));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_change_render(fsl_render *_render)
{
    if (_render == NULL)
    {
        _LOGERROR(FSL_ERR_POINTER_NULL, 0,
                "%s\n", "Failed to Change Render, Pointer NULL");
        return fsl_err;
    }

    render = _render;

    if (render->window)
        glfwMakeContextCurrent(render->window);
    else
    {
        _LOGWARNING(FSL_ERR_WINDOW_NOT_FOUND, 0,
                "%s\n", "No Window Found for the Currently Bound Render");
        return fsl_err;
    }
    
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_request_screenshot(void)
{
    fsl_flag.request_screenshot = 1;
}

u32 fsl_process_screenshot_request(const str *dir_screenshots, const str *special_text)
{
    if (fsl_flag.request_screenshot)
    {
        fsl_flag.request_screenshot = 0;
        return _fsl_take_screenshot(dir_screenshots, special_text);
    }
    return FSL_ERR_SUCCESS;
}

static u32 _fsl_take_screenshot(const str *dir_screenshots, const str *special_text)
{
    u64 i = 0;
    str str_time[FSL_TIME_STRING_MAX] = {0};
    str str_special_text[NAME_MAX] = {0};
    str file_name[PATH_MAX] = {0};
    str file_name_full[PATH_MAX] = {0};
    str str_dir_screenshots[PATH_MAX] = {0};

    if (fsl_is_dir_exists(dir_screenshots, TRUE) != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_SCREENSHOT_FAIL,
                FSL_FLAG_LOG_CMD,
                "Failed to Take Screenshot, '%s' Directory Not Found\n", dir_screenshots);
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
            LOGDEBUG(FSL_FLAG_LOG_CMD,
                    "Screenshot: %s\n", file_name_full);
            stbi_write_png(file_name_full, render->size.x, render->size.y, FSL_COLOR_CHANNELS_RGB,
                    render->screen_buf, render->size.x * FSL_COLOR_CHANNELS_RGB);

            fsl_err = FSL_ERR_SUCCESS;
            return fsl_err;
        }
    }

    LOGERROR(FSL_ERR_SCREENSHOT_FAIL,
            FSL_FLAG_LOG_CMD,
            "%s\n", "Failed to Take Screenshot, Screenshot Rate Limit Exceeded");
    return fsl_err;
}

/* ---- section: meat ------------------------------------------------------- */

void fsl_attrib_vec3(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
}

void fsl_attrib_vec3_vec2(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

void fsl_attrib_vec3_vec3(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

void fsl_attrib_vec3_vec4(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

u32 fsl_fbo_init(fsl_fbo *fbo, fsl_mesh *mesh_fbo, b8 multisample, u32 samples)
{
    if (fbo == NULL)
        goto mesh_fbo_init;

    fsl_fbo_free(fbo);

    glGenFramebuffers(1, &fbo->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

    if (multisample)
    {
        /* ---- color buffer ------------------------------------------------ */

        glGenTextures(1, &fbo->color_buf);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA,
                render->size.x, render->size.y, GL_TRUE);

        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
                GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
                GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
                GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

        glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE,
                GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf, 0);

        /* ---- render buffer ----------------------------------------------- */

        glGenRenderbuffers(1, &fbo->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_DEPTH_COMPONENT24, render->size.x, render->size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->rbo);
    }
    else
    {
        /* ---- color buffer ------------------------------------------------ */

        glGenTextures(1, &fbo->color_buf);
        glBindTexture(GL_TEXTURE_2D, fbo->color_buf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                render->size.x, render->size.y, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, fbo->color_buf, 0);

        /* ---- render buffer ----------------------------------------------- */

        glGenRenderbuffers(1, &fbo->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, render->size.x, render->size.y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->rbo);
    }

    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        _LOGFATAL(FSL_ERR_FBO_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "FBO '%u': Status '%u' Not Complete, Process Aborted\n",
                fbo->fbo, status);
        return fsl_err;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

mesh_fbo_init:

    if (mesh_fbo == NULL || mesh_fbo->vbo_data != NULL)
        return 0;

    mesh_fbo->vbo_len = fsl_arr_len(vbo_data_unit_quad);
    if (fsl_mem_alloc((void*)&mesh_fbo->vbo_data, sizeof(GLfloat) * mesh_fbo->vbo_len,
                "fbo_init().mesh_fbo.vbo_data") != FSL_ERR_SUCCESS)
        goto cleanup;

    memcpy(mesh_fbo->vbo_data, vbo_data_unit_quad, sizeof(GLfloat) * mesh_fbo->vbo_len);

    /* ---- bind mesh ------------------------------------------------------- */

    glGenVertexArrays(1, &mesh_fbo->vao);
    glGenBuffers(1, &mesh_fbo->vbo);

    glBindVertexArray(mesh_fbo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_fbo->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_fbo->vbo_len * sizeof(GLfloat),
            mesh_fbo->vbo_data, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_fbo_free(fbo);
    fsl_mesh_free(mesh_fbo);
    return fsl_err;
}

u32 fsl_fbo_realloc(fsl_fbo *fbo, b8 multisample, u32 samples)
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

    if (multisample)
    {
        /* ---- color buffer ------------------------------------------------ */

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA,
                render->size.x, render->size.y, GL_TRUE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf, 0);

        /* ---- render buffer ----------------------------------------------- */

        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_DEPTH_COMPONENT24, render->size.x, render->size.y);
    }
    else
    {
        /* ---- color buffer ------------------------------------------------ */

        glBindTexture(GL_TEXTURE_2D, fbo->color_buf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                render->size.x, render->size.y, 0, GL_RGBA, GL_FLOAT, NULL);

        /* ---- render buffer ----------------------------------------------- */

        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                render->size.x, render->size.y);
    }

    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        _LOGFATAL(FSL_ERR_FBO_REALLOC_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "FBO '%u': Status '%u' Not Complete, Process Aborted\n", fbo->fbo, status);

        fsl_fbo_free(fbo);
        return fsl_err;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_fbo_free(fsl_fbo *fbo)
{
    if (!fbo) return;
    fbo->rbo ? glDeleteFramebuffers(1, &fbo->rbo) : 0;
    fbo->color_buf ? glDeleteTextures(1, &fbo->color_buf) : 0;
    fbo->fbo ? glDeleteFramebuffers(1, &fbo->fbo) : 0;
    *fbo = (fsl_fbo){0};
};

u32 fsl_texture_init(fsl_texture *texture, v2i32 size, const GLint format_internal, const GLint format,
        GLint filter, int channels, b8 grayscale, const str *file_name)
{
    if (!size.x || !size.y)
    {
        _LOGERROR(FSL_ERR_IMAGE_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Initialize Texture '%s', Image Size Too Small\n", file_name);
        return fsl_err;
    }

    if (strlen(file_name) >= PATH_MAX)
    {
        _LOGERROR(FSL_ERR_PATH_TOO_LONG,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Initialize Texture '%s', File Path Too Long\n", file_name);
        return fsl_err;
    }

    if (fsl_is_file_exists(file_name, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    texture->buf = (u8*)stbi_load(file_name, &texture->size.x, &texture->size.y,
            &texture->channels, channels);
    if (!texture->buf)
    {
        _LOGERROR(FSL_ERR_IMAGE_LOAD_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Initialize Texture '%s', 'stbi_load()' Failed\n", file_name);
        return fsl_err;
    }

    texture->format = format;
    texture->format_internal = format_internal;
    texture->filter = filter;
    texture->grayscale = grayscale;
    texture->loaded = TRUE;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_texture_generate(fsl_texture *texture, b8 bindless)
{
    _fsl_texture_generate(&texture->id, texture->format_internal, texture->format,
            texture->filter, texture->size.x, texture->size.y,
            texture->buf, texture->grayscale);
    texture->generated = TRUE;

    if (fsl_err == FSL_ERR_SUCCESS && bindless)
    {
        texture->handle = glGetTextureHandleARB(texture->id);
        glMakeTextureHandleResidentARB(texture->handle);
        _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                "Handle[%"PRIu64"] for Texture[%u] Created\n", texture->handle, texture->id);
        texture->bindless = TRUE;
    }

    if (texture->buf)
        stbi_image_free(texture->buf);

    return fsl_err;
}

u32 _fsl_texture_generate(GLuint *id, const GLint format_internal,  const GLint format,
        GLint filter, u32 width, u32 height, void *buf, b8 grayscale)
{
    if (!width || !height)
    {
        _LOGERROR(FSL_ERR_IMAGE_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Generate Texture [%u], Texture Size Too Small\n", *id);
        return fsl_err;
    }

    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);
    glTexImage2D(GL_TEXTURE_2D, 0, format_internal,
            width, height, 0, format, GL_UNSIGNED_BYTE, buf);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (grayscale)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glBindTexture(GL_TEXTURE_2D, 0);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_texture_free(fsl_texture *texture)
{
    if (!texture) return;

    if (texture->bindless)
    {
        glMakeTextureHandleNonResidentARB(texture->handle);
        _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                "Handle[%"PRIu64"] for Texture[%u] Destroyed\n",
                texture->handle, texture->id);
        texture->bindless = FALSE;
    }

    if (texture->generated)
    {
        glDeleteTextures(1, &texture->id);
        _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                "Texture[%u] Unloaded\n", texture->id);
        texture->generated = FALSE;
    }

    texture->loaded = FALSE;
}

u32 fsl_mesh_generate(fsl_mesh *mesh, void (*attrib)(), GLenum usage,
        GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data)
{
    if (fsl_mem_alloc((void*)&mesh->vbo_data, sizeof(GLfloat) * vbo_len,
                "mesh_generate().mesh.vbo_data") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (ebo_data)
        if (fsl_mem_alloc((void*)&mesh->ebo_data, sizeof(GLuint) * ebo_len,
                    "mesh_generate().mesh.vbo_data") != FSL_ERR_SUCCESS)
            goto cleanup;

    mesh->vbo_len = vbo_len;
    mesh->ebo_len = ebo_len;

    memcpy(mesh->vbo_data, vbo_data, sizeof(GLfloat) * vbo_len);
    memcpy(mesh->ebo_data, ebo_data, sizeof(GLuint) * ebo_len);

    /* ---- bind mesh ------------------------------------------------------- */

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

    glBufferData(GL_ARRAY_BUFFER, mesh->vbo_len * sizeof(GLfloat), mesh->vbo_data, usage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo_len * sizeof(GLuint), mesh->ebo_data, usage);

    if (attrib) attrib();

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mesh_free(mesh);
    fsl_err = FSL_ERR_MESH_GENERATION_FAIL;
    return fsl_err;
}

void fsl_mesh_free(fsl_mesh *mesh)
{
    if (!mesh)
        return;

    mesh->ebo ? glDeleteBuffers(1, &mesh->ebo) : 0;
    mesh->vbo ? glDeleteBuffers(1, &mesh->vbo) : 0;
    mesh->vao ? glDeleteVertexArrays(1, &mesh->vao) : 0;

    fsl_mem_free((void*)&mesh->vbo_data, sizeof(GLfloat) * mesh->vbo_len, "fsl_mesh_free().mesh.vbo_data");
    fsl_mem_free((void*)&mesh->ebo_data, sizeof(GLuint) * mesh->ebo_len, "fsl_mesh_free().mesh.vbo_data");
    *mesh = (fsl_mesh){0};
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

    f32 ratio = camera.ratio;
    f32 fovy = 1.0f / tanf((camera.fovy_smooth / 2.0f) * FSL_DEG2RAD);
    f32 far = camera.far;
    f32 near = camera.near;
    f32 clip = -(far + near) / (far - near);
    f32 offset = -(2.0f * far * near) / (far - near);

    /* ---- target ---------------------------------------------------------- */

    projection->target = (m4f32){
        1.0f,           0.0f,           0.0f,   0.0f,
        0.0f,           1.0f,           0.0f,   0.0f,
        0.0f,           0.0f,           1.0f,   0.0f,
        -CYAW * -CPCH,  SYAW * -CPCH,   -SPCH,  1.0f,
    };

    /* ---- translation ----------------------------------------------------- */

    projection->translation = (m4f32){
        1.0f,           0.0f,           0.0f,           0.0f,
        0.0f,           1.0f,           0.0f,           0.0f,
        0.0f,           0.0f,           1.0f,           0.0f,
        -camera.pos.x,  -camera.pos.y,  -camera.pos.z,  1.0f,
    };

    /* ---- rotation: yaw --------------------------------------------------- */

    projection->rotation = (m4f32){
            CYAW,   SYAW, 0.0f, 0.0f,
            -SYAW,  CYAW, 0.0f, 0.0f,
            0.0f,   0.0f, 1.0f, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
        };

    /* ---- rotation: pitch ------------------------------------------------- */

    projection->rotation = fsl_matrix_multiply(projection->rotation,
            (m4f32){
            CPCH,   0.0f, SPCH, 0.0f,
            0.0f,   1.0f, 0.0f, 0.0f,
            -SPCH,  0.0f, CPCH, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
            });

    /* ---- rotation: roll -------------------------------------------------- */

    if (roll)
        projection->rotation = fsl_matrix_multiply(projection->rotation,
                (m4f32){
                1.0f,   0.0f,   0.0f, 0.0f,
                0.0f,   CROL,   SROL, 0.0f,
                0.0f,   -SROL,  CROL, 0.0f,
                0.0f,   0.0f,   0.0f, 1.0f,
                });

    /* ---- orientation: z-up ----------------------------------------------- */

    projection->orientation = (m4f32){
        0.0f,   0.0f, -1.0f,    0.0f,
        -1.0f,  0.0f, 0.0f,     0.0f,
        0.0f,   1.0f, 0.0f,     0.0f,
        0.0f,   0.0f, 0.0f,     1.0f,
    };

    /* ---- view ------------------------------------------------------------ */

    projection->view = fsl_matrix_multiply(projection->translation,
                fsl_matrix_multiply(projection->rotation, projection->orientation));

    /* ---- projection ------------------------------------------------------ */

    projection->projection = (m4f32){
        fovy / ratio,   0.0f,   0.0f,   0.0f,
        0.0f,           fovy,   0.0f,   0.0f,
        0.0f,           0.0f,   clip,  -1.0f,
        0.0f,           0.0f,   offset, 0.0f,
    };

    projection->perspective = fsl_matrix_multiply(projection->view, projection->projection);
}

void fsl_get_camera_lookat_angles(v3f64 camera_pos, v3f64 target, f64 *pitch, f64 *yaw)
{
    v3f64 direction = fsl_normalize_v3f64((v3f64){
            camera_pos.x - target.x,
            camera_pos.y - target.y,
            camera_pos.z - target.z,
            });
    *pitch = atan2(direction.z, sqrt(direction.x * direction.x + direction.y * direction.y));
    *yaw = atan2(-direction.y, direction.x);
}
