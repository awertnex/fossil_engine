#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <inttypes.h>

#include "h/core.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/math.h"
#include "h/memory.h"
#include "h/string.h"
#include "h/time.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include <engine/include/stb_truetype.h>
#define STB_IMAGE_IMPLEMENTATION
#include <engine/include/stb_image.h>


static u64 engine_flag = 0; /* enum: EngineFlag */
u32 engine_err = ERR_SUCCESS;

/* ---- section: signatures ------------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 */
static void glfw_callback_error(int error, const char* message)
{
    (void)error;
    LOGERROR(TRUE, ERR_GLFW, "GLFW: %s\n", message);
}

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief process shader before compilation.
 *
 *  parse includes recursively.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
static str *shader_pre_process(const str *path, u64 *file_len);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief process shader before compilation.
 *
 *  parse includes recursively.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
static str *_shader_pre_process(const str *path, u64 *file_len, u64 recursion_limit);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief generate texture for opengl from 'buf'.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
static u32 _texture_generate(GLuint *id, const GLint format_internal,  const GLint format,
        GLint filter, u32 width, u32 height, void *buf, b8 grayscale);

/* ---- section: init ------------------------------------------------------- */

u32 engine_init(int argc, char **argv, Render *render, b8 multisample, b8 release_build)
{
    str *path = get_path_bin_root();

    get_time_f64(); /* initialize start time */
    glfwSetErrorCallback(glfw_callback_error);
    change_dir(path);

    if (
            logger_init(release_build, argc, argv) != ERR_SUCCESS ||
            glfw_init(multisample) != ERR_SUCCESS ||
            window_init(render) != ERR_SUCCESS ||
            glad_init() != ERR_SUCCESS)
        engine_close(render);

    return engine_err;
}

void engine_close(Render *render)
{
    logger_close();
    if (render->window)
        glfwDestroyWindow(render->window);
    if (engine_flag & FLAG_ENGINE_GLFW_INITIALIZED)
    {
        glfwTerminate();
        engine_flag &= ~FLAG_ENGINE_GLFW_INITIALIZED;
    }
}

u32 glfw_init(b8 multisample)
{
    if (!glfwInit())
    {
        LOGFATAL(FALSE, ERR_GLFW_INIT_FAIL,
                "%s\n", "Failed to Initialize GLFW, Process Aborted");
        return engine_err;
    }

    engine_flag |= FLAG_ENGINE_GLFW_INITIALIZED;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    if (multisample)
        glfwWindowHint(GLFW_SAMPLES, 4);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 window_init(Render *render)
{
    render->window = glfwCreateWindow(render->size.x, render->size.y, render->title, NULL, NULL);

    if (!render->window)
    {
        LOGFATAL(FALSE, ERR_WINDOW_INIT_FAIL,
                "%s\n", "Failed to Initialize Window or OpenGL Context, Process Aborted");
        return engine_err;
    }

    glfwMakeContextCurrent(render->window);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

u32 glad_init(void)
{
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        LOGFATAL(FALSE, ERR_GLAD_INIT_FAIL,
                "%s\n", "Failed to Initialize GLAD, Process Aborted");
        return engine_err;
    }

    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 3))
    {
        LOGFATAL(FALSE, ERR_GL_VERSION_NOT_SUPPORT,
                "OpenGL 4.3+ Required, Current Version '%d.%d', Process Aborted\n",
                GLVersion.major, GLVersion.minor);
        return engine_err;
    }

    LOGINFO(FALSE, "OpenGL:    %s\n", glGetString(GL_VERSION));
    LOGINFO(FALSE, "GLSL:      %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOGINFO(FALSE, "Vendor:    %s\n", glGetString(GL_VENDOR));
    LOGINFO(FALSE, "Renderer:  %s\n", glGetString(GL_RENDERER));

    engine_err = ERR_SUCCESS;
    return engine_err;
}

/* ---- section: shaders ---------------------------------------------------- */

u32 shader_init(const str *shaders_dir, Shader *shader)
{
    if (!shader->type)
        return ERR_SUCCESS;

    str str_reg[PATH_MAX] = {0};
    snprintf(str_reg, PATH_MAX, "%s%s", shaders_dir, shader->file_name);

    shader->source = shader_pre_process(str_reg, NULL);
    if (!shader->source)
    {
        LOGERROR(FALSE, ERR_POINTER_NULL, "Shader Source '%s' NULL\n", shader->file_name);
        return engine_err;
    }
    (shader->id) ? glDeleteShader(shader->id) : 0;

    shader->id = glCreateShader(shader->type);
    glShaderSource(shader->id, 1, (const GLchar**)&shader->source, NULL);
    glCompileShader(shader->id);
    mem_free((void*)&shader->source, strlen(shader->source), "shader_init().shader.source");
    shader->source = NULL;

    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &shader->loaded);
    if (!shader->loaded)
    {
        char log[STRING_MAX];
        glGetShaderInfoLog(shader->id, STRING_MAX, NULL, log);
        LOGERROR(FALSE, ERR_SHADER_COMPILE_FAIL, "Shader '%s':\n%s\n", shader->file_name, log);
        return engine_err;
    }
    else LOGINFO(FALSE, "Shader %d '%s' Loaded\n", shader->id, shader->file_name);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

static str *shader_pre_process(const str *path, u64 *file_len)
{
    return _shader_pre_process(path, file_len, INCLUDE_RECURSION_LIMIT_MAX);
}

static str *_shader_pre_process(const str *path, u64 *file_len, u64 recursion_limit)
{
    if (!recursion_limit)
    {
        LOGFATAL(FALSE, ERR_INCLUDE_RECURSION_LIMIT,
                "Include Recursion Limit Exceeded '%s', Process Aborted\n", path);
        return NULL;
    }

    static str token[2][256] =
    {
        "#include \"",
        "\"\n",
    };

    u64 i = 0, j = 0, k = 0, cursor = 0;
    str *buf = NULL;
    str *buf_include = NULL;
    str *buf_resolved = NULL;
    u64 buf_len = get_file_contents(path, (void*)&buf, 1, "rb", TRUE);
    u64 buf_include_len = 0;
    u64 buf_resolved_len = 0;
    if (engine_err != ERR_SUCCESS)
        return NULL;

    if (mem_alloc((void*)&buf_resolved, buf_len + 1,
                "_shader_pre_process().buf_resolved") != ERR_SUCCESS)
        goto cleanup;

    buf_resolved_len = buf_len + 1;
    snprintf(buf_resolved, buf_resolved_len, "%s", buf);
    for (; i < buf_len; ++i)
    {
        if ((i == 0 || (buf[i - 1] == '\n')) &&
                (buf[i] == '#') &&
                !strncmp(buf + i, token[0], strlen(token[0])))
        {
            u64 string_len = 0;
            for (j = strlen(token[0]); i + j < buf_len &&
                    strncmp(buf + i + j, token[1], strlen(token[1]));
                    ++string_len, ++j)
            {}
            j += strlen(token[1]);

            str string[PATH_MAX] = {0};
            snprintf(string, PATH_MAX, "%s", path);
            retract_path(string);
            snprintf(string + strlen(string), PATH_MAX - strlen(string),
                    "%.*s", (int)string_len, buf + i + strlen(token[0]));

            if (!strncmp(string, path, strlen(string)))
            {
                LOGFATAL(FALSE, ERR_SELF_INCLUDE,
                        "Self Include Detected '%s', Process Aborted\n", path);
                goto cleanup;
            }
            buf_include = _shader_pre_process(string, &buf_include_len, recursion_limit - 1);
            if (engine_err != ERR_SUCCESS ||
                    mem_realloc((void*)&buf_resolved, buf_resolved_len + buf_include_len + 1,
                        "_shader_pre_process().buf_resolved") != ERR_SUCCESS)
                goto cleanup;
            buf_resolved_len += buf_include_len;

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%.*s", (int)(i - k), buf + k);

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%s", buf_include);

            k = i + j;
            mem_free((void*)&buf_include, buf_include_len, "_shader_pre_process().buf_include");
        }
    }

    if (k < buf_len)
        snprintf(buf_resolved + cursor,
                buf_resolved_len - cursor, "%s", buf + k);

    mem_free((void*)&buf, buf_len, "_shader_pre_process().buf");
    if (file_len) *file_len = buf_resolved_len;
    engine_err = ERR_SUCCESS;
    return buf_resolved;

cleanup:

    mem_free((void*)&buf_include, buf_include_len, "_shader_pre_process().buf_include");
    mem_free((void*)&buf_resolved, buf_resolved_len, "_shader_pre_process().buf_resolved");
    mem_free((void*)&buf, buf_len, "_shader_pre_process().buf");
    return NULL;
}

u32 shader_program_init(const str *shaders_dir, ShaderProgram *program)
{
    if (shader_init(shaders_dir, &program->vertex) != 0)
        return engine_err;
    if (shader_init(shaders_dir, &program->geometry) != 0)
        return engine_err;
    if (shader_init(shaders_dir, &program->fragment) != 0)
        return engine_err;
    (program->id) ? glDeleteProgram(program->id) : 0;

    program->id = glCreateProgram();
    if (program->vertex.id)
        glAttachShader(program->id, program->vertex.id);
    if (program->geometry.id)
        glAttachShader(program->id, program->geometry.id);
    if (program->fragment.id)
        glAttachShader(program->id, program->fragment.id);
    glLinkProgram(program->id);

    glGetProgramiv(program->id, GL_LINK_STATUS, &program->loaded);
    if (!program->loaded)
    {
        char log[STRING_MAX];
        glGetProgramInfoLog(program->id, STRING_MAX, NULL, log);
        LOGERROR(FALSE, ERR_SHADER_PROGRAM_LINK_FAIL,
                "Shader Program '%s':\n%s\n", program->name, log);
        return engine_err;
    }
    else LOGINFO(FALSE,
            "Shader Program %d '%s' Loaded\n", program->id, program->name);

    if (program->vertex.loaded)
        glDeleteShader(program->vertex.id);
    if (program->geometry.loaded)
        glDeleteShader(program->geometry.id);
    if (program->fragment.loaded)
        glDeleteShader(program->fragment.id);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void shader_program_free(ShaderProgram *program)
{
    if (program == NULL || !program->id) return;
    glDeleteProgram(program->id);

    if (program->vertex.source)
        mem_free((void*)&program->vertex.source, strlen(program->vertex.source),
                "shader_program_free().vertex.source");

    if (program->fragment.source)
        mem_free((void*)&program->fragment.source, strlen(program->fragment.source),
                "shader_program_free().fragment.source");

    if (program->geometry.source)
        mem_free((void*)&program->geometry.source, strlen(program->geometry.source),
                "shader_program_free().geometry.source");
}

/* ---- section: meat ------------------------------------------------------- */

void attrib_vec3(void)
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);
}

void attrib_vec3_vec2(void)
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
}

void attrib_vec3_vec3(void)
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
}

void attrib_vec3_vec4(void)
{
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
}

u32 fbo_init(Render *render, FBO *fbo, Mesh *mesh_fbo,
        b8 multisample, u32 samples)
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
        LOGFATAL(FALSE, ERR_FBO_INIT_FAIL, "FBO '%d': Status '%d' Not Complete, Process Aborted\n",
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

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

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

u32 fbo_realloc(Render *render, FBO *fbo, b8 multisample, u32 samples)
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
        LOGFATAL(FALSE, ERR_FBO_REALLOC_FAIL,
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
        LOGERROR(FALSE, ERR_IMAGE_SIZE_TOO_SMALL,
                "Failed to Initialize Texture '%s', Image Size Too Small\n", file_name);
        return engine_err;
    }

    if (strlen(file_name) >= PATH_MAX)
    {
        LOGERROR(FALSE, ERR_PATH_TOO_LONG,
                "Failed to Initialize Texture '%s', File Path Too Long\n", file_name);
        return engine_err;
    }

    if (is_file_exists(file_name, TRUE) != ERR_SUCCESS)
        return engine_err;

    texture->buf = (u8*)stbi_load(file_name,
            &texture->size.x, &texture->size.y, &texture->channels, channels);
    if (!texture->buf)
    {
        LOGERROR(FALSE, ERR_IMAGE_LOAD_FAIL,
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
        LOGTRACE(FALSE, "Handle[%"PRIu64"] for Texture[%d] Created\n", texture->handle, texture->id);
    }

    (texture->buf) ? stbi_image_free(texture->buf) : 0;
    return engine_err;
}

static u32 _texture_generate(GLuint *id, const GLint format_internal,  const GLint format,
        GLint filter, u32 width, u32 height, void *buf, b8 grayscale)
{
    if (!width || !height)
    {
        LOGERROR(FALSE, ERR_IMAGE_SIZE_TOO_SMALL,
                "Failed to Generate Texture [%d], Size Too Small\n", *id);
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
        LOGTRACE(FALSE, "Handle[%"PRIu64"] for Texture[%d] Destroyed\n",
                texture->handle, texture->id);
    }

    if (texture->id)
    {
        glDeleteTextures(1, &texture->id);
        LOGTRACE(FALSE, "Texture[%d] Unloaded\n", texture->id);
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

/* ---- section: font ------------------------------------------------------- */

u32 font_init(Font *font, u32 resolution, const str *file_name)
{
    f32 scale;
    u32 i, x, y, col, row;
    u8 *canvas = NULL;
    Glyph *g = NULL;

    if (resolution <= 2)
    {
        LOGERROR(FALSE, ERR_IMAGE_SIZE_TOO_SMALL,
                "Failed to Initialize Font '%s', Font Size Too Small\n", file_name);
        return engine_err;
    }

    if (strlen(file_name) >= PATH_MAX)
    {
        LOGERROR(FALSE, ERR_PATH_TOO_LONG,
                "Failed to Initialize Font '%s', File Path Too Long\n", file_name);
        return engine_err;
    }

    if (is_file_exists(file_name, TRUE) != ERR_SUCCESS)
        return engine_err;

    font->buf_len = get_file_contents(file_name, (void*)&font->buf, 1, "rb", TRUE);
    if (!font->buf)
        return engine_err;

    if (!stbtt_InitFont(&font->info, (const unsigned char*)font->buf, 0))
    {
        LOGERROR(FALSE, ERR_FONT_INIT_FAIL,
                "Failed to Initialize Font '%s', 'stbtt_InitFont()' Failed\n", file_name);
        goto cleanup;
    }

    if (mem_alloc((void*)&font->bitmap, GLYPH_MAX * resolution * resolution,
                stringf("font_init().%s", file_name)) != ERR_SUCCESS)
        goto cleanup;

    if (mem_alloc((void*)&canvas, resolution * resolution,
                "font_init().font_glyph_canvas") != ERR_SUCCESS)
        goto cleanup;

    snprintf(font->path, PATH_MAX, "%s", file_name);
    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
    font->resolution = resolution;
    font->char_size = 1.0f / FONT_ATLAS_CELL_RESOLUTION;
    font->line_height = font->ascent - font->descent + font->line_gap;
    font->size = resolution;
    scale = stbtt_ScaleForPixelHeight(&font->info, resolution);

    for (i = 0; i < GLYPH_MAX; ++i)
    {
        int glyph_index = stbtt_FindGlyphIndex(&font->info, i);
        if (!glyph_index) continue;

        g = &font->glyph[i];

        stbtt_GetGlyphHMetrics(&font->info, glyph_index, &g->advance, &g->bearing.x);
        stbtt_GetGlyphBitmapBoxSubpixel(&font->info, glyph_index,
                1.0f, 1.0f, 0.0f, 0.0f, &g->x0, &g->y0, &g->x1, &g->y1);

        g->bearing.y = g->y0;
        g->scale.x = g->x1 - g->x0;
        g->scale.y = g->y1 - g->y0;
        g->scale.x > font->scale.x ? font->scale.x = g->scale.x : 0;
        g->scale.y > font->scale.y ? font->scale.y = g->scale.y : 0;

        col = i % FONT_ATLAS_CELL_RESOLUTION;
        row = i / FONT_ATLAS_CELL_RESOLUTION;
        if (!stbtt_IsGlyphEmpty(&font->info, glyph_index))
        {
            stbtt_MakeGlyphBitmapSubpixel(&font->info, canvas,
                    resolution, resolution, resolution, scale, scale, 0.0f, 0.0f, glyph_index);

            void *bitmap_offset = font->bitmap +
                col * resolution +
                row * resolution * resolution * FONT_ATLAS_CELL_RESOLUTION +
                1 + resolution * FONT_ATLAS_CELL_RESOLUTION;

            for (y = 0; y < resolution - 1; ++y)
                for (x = 0; x < resolution - 1; ++x)
                    memcpy(bitmap_offset + x +
                            y * resolution * FONT_ATLAS_CELL_RESOLUTION,
                            canvas + x + y * resolution, 1);

            mem_clear((void*)&canvas, resolution * resolution, "font_init().font_glyph_canvas");
        }

        g->texture_sample.x = col * font->char_size;
        g->texture_sample.y = row * font->char_size;
        font->glyph[i].loaded = TRUE;
    }

    if (_texture_generate(&font->id, GL_RED, GL_RED, GL_LINEAR,
                FONT_ATLAS_CELL_RESOLUTION * resolution,
                FONT_ATLAS_CELL_RESOLUTION * resolution,
                font->bitmap, TRUE) != ERR_SUCCESS)
        goto cleanup;

    mem_free((void*)&canvas, resolution * resolution, "font_init().font_glyph_canvas");

    engine_err = ERR_SUCCESS;
    return engine_err;

cleanup:

    mem_free((void*)&canvas, resolution * resolution, "font_init().font_glyph_canvas");
    font_free(font);
    return engine_err;
}

void font_free(Font *font)
{
    if (!font) return;
    mem_free((void*)&font->buf, font->buf_len, "font_free().file_contents");
    mem_free((void*)&font->bitmap, GLYPH_MAX * font->resolution * font->resolution, font->path);
    *font = (Font){0};
}
