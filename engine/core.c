#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "h/common.h"
#include "h/core.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/math.h"
#include "h/memory.h"
#include "h/shaders.h"
#include "h/string.h"
#include "h/time.h"
#include "h/text.h"
#include "h/ui.h"

#define STB_IMAGE_IMPLEMENTATION
#include <engine/include/stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <engine/include/stb_image_write.h>

u64 init_time = 0;
str *DIR_PROC_ROOT = NULL;
u32 engine_err = ERR_SUCCESS;

static struct /* flag */
{
    u64 active: 1;
    u64 glfw_initialized: 1;
    u64 request_screenshot: 1;
} flag;

static Render render_internal =
{
    .title = ENGINE_TITLE,
    .size = (v2i32){RENDER_WIDTH_DEFAULT, RENDER_HEIGHT_DEFAULT},
};

Render *render = &render_internal;

static struct /* ubo */
{
    UBO ndc_scale;
} ubo;

/* ---- section: signatures ------------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 */
static void glfw_callback_error(int error, const char* message)
{
    (void)error;
    _LOGERROR(TRUE, ERR_GLFW, "GLFW: %s\n", message);
}

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief take screenshot and save into dir at '_screenshot_dir'.
 *  
 *  save pixel data as 'RGB' into 'render->screen_buf'.
 *
 *  @param special_text = string appended to file name before extension.
 *
 *  @remark if directory not found, screenshot is still saved at 'render->screen_buf'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
static u32 take_screenshot(const str *_screenshot_dir, const str *special_text);

/* ---- section: init ------------------------------------------------------- */

u32 engine_init(int argc, char **argv, const str *_log_dir, const str *title,
        i32 size_x, i32 size_y, Render *_render, u64 flags)
{
    u32 i = 0;

    if (logger_init(argc, argv, flags & FLAG_ENGINE_RELEASE_BUILD, _log_dir,
                TRUE) != ERR_SUCCESS)
        goto cleanup;

    glfwSetErrorCallback(glfw_callback_error);

    if (_render)
        change_render(_render);

    if (
            glfw_init(flags & FLAG_ENGINE_MULTISAMPLE) != ERR_SUCCESS ||
            window_init(title, size_x, size_y) != ERR_SUCCESS ||
            glad_init() != ERR_SUCCESS)
        goto cleanup;

    if (flags & FLAG_ENGINE_LOAD_DEFAULT_SHADERS)
    {
        for (i = 0; i < ENGINE_SHADER_COUNT; ++i)
            if (shader_program_init(ENGINE_DIR_NAME_SHADERS, &engine_shader[i]) != ERR_SUCCESS)
                goto cleanup;

        glGenBuffers(1, &ubo.ndc_scale.buf);
        glBindBuffer(GL_UNIFORM_BUFFER, ubo.ndc_scale.buf);
        glBufferData(GL_UNIFORM_BUFFER, sizeof(v2f32), NULL, GL_STATIC_DRAW);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        glBindBufferBase(GL_UNIFORM_BUFFER, ENGINE_SHADER_BUFFER_BINDING_UBO_NDC_SCALE,
                ubo.ndc_scale.buf);
    }

    flag.active = 1;
    return engine_err;

cleanup:

    engine_close();
    return engine_err;
}

void engine_close(void)
{
    u32 i = 0;

    for (i = 0; i < ENGINE_FONT_COUNT; ++i)
        font_free(&engine_font[i]);
    for (i = 0; i < ENGINE_TEXTURE_COUNT; ++i)
        texture_free(&engine_texture[i]);
    for (i = 0; i < ENGINE_SHADER_COUNT; ++i)
        shader_program_free(&engine_shader[i]);

    mesh_free(&engine_mesh_unit);
    text_free();
    ui_free();

    if (render->window)
        glfwDestroyWindow(render->window);

    if (flag.glfw_initialized)
    {
        flag.glfw_initialized = 0;
        glfwTerminate();
    }

    mem_free((void*)&render->screen_buf, render->size.x * render->size.y * COLOR_CHANNELS_RGB,
                "engine_free().render.screen_buf");

    mem_free((void*)&DIR_PROC_ROOT, strlen(DIR_PROC_ROOT), "engine_close().DIR_PROC_ROOT");

    logger_close();
}

u32 glfw_init(b8 multisample)
{
    if (!glfwInit())
    {
        _LOGFATAL(FALSE, ERR_GLFW_INIT_FAIL,
                "%s\n", "Failed to Initialize GLFW, Process Aborted");
        return engine_err;
    }

    flag.glfw_initialized = 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (multisample)
        glfwWindowHint(GLFW_SAMPLES, 4);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 window_init(const str *title, i32 size_x, i32 size_y)
{
    if (title) snprintf(render->title, NAME_MAX, "%s", title);
    if (size_x) render->size.x = clamp_i32(size_x, RENDER_WIDTH_MIN, RENDER_WIDTH_MAX);
    if (size_y) render->size.y = clamp_i32(size_y, RENDER_HEIGHT_MIN, RENDER_HEIGHT_MAX);

    render->window = glfwCreateWindow(render->size.x, render->size.y, render->title, NULL, NULL);

    if (!render->window)
    {
        _LOGFATAL(FALSE, ERR_WINDOW_INIT_FAIL,
                "%s\n", "Failed to Initialize Window or OpenGL Context, Process Aborted");
        return engine_err;
    }

    glfwMakeContextCurrent(render->window);
    glfwSetWindowSizeLimits(render->window,
            RENDER_WIDTH_MIN, RENDER_HEIGHT_MIN,
            RENDER_WIDTH_MAX, RENDER_HEIGHT_MAX);

    if (mem_alloc((void*)&render->screen_buf, render->size.x * render->size.y * COLOR_CHANNELS_RGB,
                "engine_init().render.screen_buf") != ERR_SUCCESS)
        return engine_err;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 glad_init(void)
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        _LOGFATAL(FALSE, ERR_GLAD_INIT_FAIL,
                "%s\n", "Failed to Initialize GLAD, Process Aborted");
        return engine_err;
    }

    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 3))
    {
        _LOGFATAL(FALSE, ERR_GL_VERSION_NOT_SUPPORT,
                "OpenGL 4.3+ Required, Current Version '%d.%d', Process Aborted\n",
                GLVersion.major, GLVersion.minor);
        return engine_err;
    }

    _LOGINFO(FALSE, "OpenGL:    %s\n", glGetString(GL_VERSION));
    _LOGINFO(FALSE, "GLSL:      %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    _LOGINFO(FALSE, "Vendor:    %s\n", glGetString(GL_VENDOR));
    _LOGINFO(FALSE, "Renderer:  %s\n", glGetString(GL_RENDERER));

    engine_err = ERR_SUCCESS;
    return engine_err;
}

b8 engine_running(void)
{
    if (glfwWindowShouldClose(render->window) || !flag.active)
        return FALSE;

    /* order doesn't matter here, independent state */
    render->time = get_time_nsec();
    render->time_delta = get_time_delta_nsec();

    return TRUE;
}

u32 engine_update_render_settings(i32 size_x, i32 size_y)
{
    render->size.x = size_x;
    render->size.y = size_y;
    glViewport(0, 0, size_x, size_y);

    render->ndc_scale.x = 2.0f / size_x;
    render->ndc_scale.y = 2.0f / size_y;

    glBindBuffer(GL_UNIFORM_BUFFER, ubo.ndc_scale.buf);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(v2f32), &render->ndc_scale, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if (mem_realloc((void*)&render->screen_buf, size_x * size_y * COLOR_CHANNELS_RGB,
            "engine_update_render_settings().render.screen_buf") != ERR_SUCCESS)
        return engine_err;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 change_render(Render *_render)
{
    if (_render == NULL)
    {
        _LOGERROR(TRUE, ERR_POINTER_NULL, "%s\n", "Failed to Change Render, Pointer NULL");
        return engine_err;
    }

    render = _render;

    if (render->window)
        glfwMakeContextCurrent(render->window);
    else
    {
        _LOGWARNING(TRUE, ERR_WINDOW_NOT_FOUND, "%s\n", "No Window Found for Currently Bound Render");
        return engine_err;
    }
    
    engine_err = ERR_SUCCESS;
    return engine_err;
}

void request_screenshot(void)
{
    flag.request_screenshot = 1;
}

u32 process_screenshot_request(const str *_screenshot_dir, const str *special_text)
{
    if (flag.request_screenshot)
    {
        flag.request_screenshot = 0;
        return take_screenshot(_screenshot_dir, special_text);
    }
    return ERR_SUCCESS;
}

static u32 take_screenshot(const str *_screenshot_dir, const str *special_text)
{
    u64 i = 0;
    str str_time[TIME_STRING_MAX] = {0};
    str str_special_text[NAME_MAX] = {0};
    str file_name[NAME_MAX] = {0};
    str file_name_full[PATH_MAX] = {0};
    str screenshot_dir[PATH_MAX] = {0};

    if (is_dir_exists(_screenshot_dir, TRUE) != ERR_SUCCESS)
    {
        LOGERROR(FALSE, TRUE, ERR_SCREENSHOT_FAIL,
                "Failed to Take Screenshot, '%s' Directory Not Found\n", _screenshot_dir);
        return engine_err;
    }

    snprintf(screenshot_dir, PATH_MAX, "%s", _screenshot_dir);
    check_slash(screenshot_dir);
    posix_slash(screenshot_dir);

    get_time_str(str_time, "%F_%H-%M-%S");

    if (special_text[0])
        snprintf(str_special_text, NAME_MAX, "_%s", special_text);

    snprintf(file_name, NAME_MAX, "%s%s", screenshot_dir, str_time);

    for (i = 0; i < SCREENSHOT_RATE_MAX; ++i)
    {
        snprintf(file_name_full, PATH_MAX, "%s_%"PRIu64"%s.png", file_name, i, str_special_text);
        if (is_file_exists(file_name_full, FALSE) != ERR_SUCCESS)
        {
            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glReadPixels(0, 0, render->size.x, render->size.y, GL_RGB, GL_UNSIGNED_BYTE, render->screen_buf);

            stbi_flip_vertically_on_write(TRUE);
            LOGDEBUG(FALSE, TRUE, "Screenshot: %s\n", file_name_full);
            stbi_write_png(file_name_full, render->size.x, render->size.y, COLOR_CHANNELS_RGB,
                    render->screen_buf, render->size.x * COLOR_CHANNELS_RGB);

            engine_err = ERR_SUCCESS;
            return engine_err;
        }
    }

    LOGERROR(FALSE, TRUE, ERR_SCREENSHOT_FAIL, "%s\n",
            "Failed to Take Screenshot, Screenshot Rate Limit Exceeded");
    return engine_err;
}

/* ---- section: meat ------------------------------------------------------- */

void attrib_vec3(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
}

void attrib_vec3_vec2(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

void attrib_vec3_vec3(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

void attrib_vec3_vec4(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

u32 fbo_init(FBO *fbo, Mesh *mesh_fbo, b8 multisample, u32 samples)
{
    fbo_free(fbo);

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
        _LOGFATAL(FALSE, ERR_FBO_INIT_FAIL, "FBO '%d': Status '%d' Not Complete, Process Aborted\n",
                fbo->fbo, status);
        return engine_err;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    /* ---- mesh data ------------------------------------------------------- */

    if (mesh_fbo == NULL || mesh_fbo->vbo_data != NULL) return 0;

    mesh_fbo->vbo_len = 16;
    GLfloat vbo_data[] =
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f,
    };

    if (mem_alloc((void*)&mesh_fbo->vbo_data, sizeof(GLfloat) * mesh_fbo->vbo_len,
                "fbo_init().mesh_fbo.vbo_data") != ERR_SUCCESS)
        goto cleanup;

    memcpy(mesh_fbo->vbo_data, vbo_data, sizeof(GLfloat) * mesh_fbo->vbo_len);

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

    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:

    fbo_free(fbo);
    mesh_free(mesh_fbo);
    return engine_err;
}

u32 fbo_realloc(FBO *fbo, b8 multisample, u32 samples)
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
        _LOGFATAL(FALSE, ERR_FBO_REALLOC_FAIL,
                "FBO '%d': Status '%d' Not Complete, Process Aborted\n", fbo->fbo, status);

        fbo_free(fbo);
        return engine_err;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void fbo_free(FBO *fbo)
{
    if (!fbo) return;
    fbo->rbo ? glDeleteFramebuffers(1, &fbo->rbo) : 0;
    fbo->color_buf ? glDeleteTextures(1, &fbo->color_buf) : 0;
    fbo->fbo ? glDeleteFramebuffers(1, &fbo->fbo) : 0;
    *fbo = (FBO){0};
};

u32 texture_init(Texture *texture, v2i32 size, const GLint format_internal, const GLint format,
        GLint filter, int channels, b8 grayscale, const str *file_name)
{
    if (!size.x || !size.y)
    {
        _LOGERROR(FALSE, ERR_IMAGE_SIZE_TOO_SMALL,
                "Failed to Initialize Texture '%s', Image Size Too Small\n", file_name);
        return engine_err;
    }

    if (strlen(file_name) >= PATH_MAX)
    {
        _LOGERROR(FALSE, ERR_PATH_TOO_LONG,
                "Failed to Initialize Texture '%s', File Path Too Long\n", file_name);
        return engine_err;
    }

    if (is_file_exists(file_name, TRUE) != ERR_SUCCESS)
        return engine_err;

    texture->buf = (u8*)stbi_load(file_name, &texture->size.x, &texture->size.y,
            &texture->channels, channels);
    if (!texture->buf)
    {
        _LOGERROR(FALSE, ERR_IMAGE_LOAD_FAIL,
                "Failed to Initialize Texture '%s', 'stbi_load()' Failed\n", file_name);
        return engine_err;
    }

    texture->format = format;
    texture->format_internal = format_internal;
    texture->filter = filter;
    texture->grayscale = grayscale;

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 texture_generate(Texture *texture, b8 bindless)
{
    _texture_generate(&texture->id, texture->format_internal, texture->format,
            texture->filter, texture->size.x, texture->size.y,
            texture->buf, texture->grayscale);

    if (engine_err == ERR_SUCCESS && bindless)
    {
        texture->handle = glGetTextureHandleARB(texture->id);
        glMakeTextureHandleResidentARB(texture->handle);
        _LOGTRACE(FALSE, "Handle[%"PRIu64"] for Texture[%d] Created\n", texture->handle, texture->id);
    }

    (texture->buf) ? stbi_image_free(texture->buf) : 0;
    return engine_err;
}

u32 _texture_generate(GLuint *id, const GLint format_internal,  const GLint format,
        GLint filter, u32 width, u32 height, void *buf, b8 grayscale)
{
    if (!width || !height)
    {
        _LOGERROR(FALSE, ERR_IMAGE_SIZE_TOO_SMALL,
                "Failed to Generate Texture [%d], Texture Size Too Small\n", *id);
        return engine_err;
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

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void texture_free(Texture *texture)
{
    if (!texture) return;

    if (texture->handle)
    {
        glMakeTextureHandleNonResidentARB(texture->handle);
        _LOGTRACE(FALSE, "Handle[%"PRIu64"] for Texture[%d] Destroyed\n",
                texture->handle, texture->id);
    }

    if (texture->id)
    {
        glDeleteTextures(1, &texture->id);
        _LOGTRACE(FALSE, "Texture[%d] Unloaded\n", texture->id);
    }

    *texture = (Texture){0};
}

u32 mesh_generate(Mesh *mesh, void (*attrib)(), GLenum usage,
        GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data)
{
    if (mem_alloc((void*)&mesh->vbo_data, sizeof(GLfloat) * vbo_len,
                "mesh_generate().mesh.vbo_data") != ERR_SUCCESS)
        goto cleanup;

    if (ebo_data)
        if (mem_alloc((void*)&mesh->ebo_data, sizeof(GLuint) * ebo_len,
                    "mesh_generate().mesh.vbo_data") != ERR_SUCCESS)
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

    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:

    mesh_free(mesh);
    engine_err = ERR_MESH_GENERATION_FAIL;
    return engine_err;
}

void mesh_free(Mesh *mesh)
{
    if (!mesh) return;
    mesh->ebo ? glDeleteBuffers(1, &mesh->ebo) : 0;
    mesh->vbo ? glDeleteBuffers(1, &mesh->vbo) : 0;
    mesh->vao ? glDeleteVertexArrays(1, &mesh->vao) : 0;

    mem_free((void*)&mesh->vbo_data, sizeof(GLfloat) * mesh->vbo_len, "mesh_free().mesh.vbo_data");
    mem_free((void*)&mesh->ebo_data, sizeof(GLuint) * mesh->ebo_len, "mesh_free().mesh.vbo_data");
    *mesh = (Mesh){0};
}

/* ---- section: camera ----------------------------------------------------- */

void update_camera_movement(Camera *camera, b8 roll)
{
    if (roll)
    {
        camera->roll = fmod(camera->roll, CAMERA_RANGE_MAX);
        if (camera->roll < 0.0) camera->roll += CAMERA_RANGE_MAX;
    }
    else camera->roll = 0.0;

    camera->pitch = clamp_f64(camera->pitch, -CAMERA_ANGLE_MAX, CAMERA_ANGLE_MAX);
    camera->yaw = fmod(camera->yaw, CAMERA_RANGE_MAX);
    if (camera->yaw < 0.0) camera->yaw += CAMERA_RANGE_MAX;

    camera->sin_roll = sin(camera->roll * DEG2RAD);
    camera->cos_roll = cos(camera->roll * DEG2RAD);
    camera->sin_pitch = sin(camera->pitch * DEG2RAD);
    camera->cos_pitch = cos(camera->pitch * DEG2RAD);
    camera->sin_yaw = sin(camera->yaw * DEG2RAD);
    camera->cos_yaw = cos(camera->yaw * DEG2RAD);
}

void update_projection_perspective(Camera camera, Projection *projection, b8 roll)
{
    const f32 SROL = camera.sin_roll;
    const f32 CROL = camera.cos_roll;
    const f32 SPCH = camera.sin_pitch;
    const f32 CPCH = camera.cos_pitch;
    const f32 SYAW = camera.sin_yaw;
    const f32 CYAW = camera.cos_yaw;

    f32 ratio = camera.ratio;
    f32 fovy = 1.0f / tanf((camera.fovy_smooth / 2.0f) * DEG2RAD);
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

    projection->rotation = matrix_multiply(projection->rotation,
            (m4f32){
            CPCH,   0.0f, SPCH, 0.0f,
            0.0f,   1.0f, 0.0f, 0.0f,
            -SPCH,  0.0f, CPCH, 0.0f,
            0.0f,   0.0f, 0.0f, 1.0f,
            });

    /* ---- rotation: roll -------------------------------------------------- */

    if (roll)
        projection->rotation = matrix_multiply(projection->rotation,
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

    projection->view = matrix_multiply(projection->translation,
                matrix_multiply(projection->rotation, projection->orientation));

    /* ---- projection ------------------------------------------------------ */

    projection->projection = (m4f32){
        fovy / ratio,   0.0f,   0.0f,   0.0f,
        0.0f,           fovy,   0.0f,   0.0f,
        0.0f,           0.0f,   clip,  -1.0f,
        0.0f,           0.0f,   offset, 0.0f,
    };

    projection->perspective = matrix_multiply(projection->view, projection->projection);
}

void get_camera_lookat_angles(v3f64 camera_pos, v3f64 target, f64 *pitch, f64 *yaw)
{
    v3f64 direction = normalize_v3f64((v3f64){
            camera_pos.x - target.x,
            camera_pos.y - target.y,
            camera_pos.z - target.z,
            });
    *pitch = atan2(direction.z, sqrt(direction.x * direction.x + direction.y * direction.y));
    *yaw = atan2(-direction.y, direction.x);
}
