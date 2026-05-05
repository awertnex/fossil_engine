/*  @file assets.c
 *
 *  @brief engine default assets.
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
 *  limitations under the License.OFTWARE.
 */

#include "h/common.h"
#include "h/assets.h"
#include "h/core.h"
#include "h/dir.h"
#include "logger/log.h"
#include "h/memory.h"
#include "h/shaders.h"
#include "h/string.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#   define STB_IMAGE_IMPLEMENTATION
#   include <deps/stb_image.h>
#pragma GCC diagnostic pop /* ignored "-Wpedantic" */

#include <stdio.h>
#include <string.h>

/* ---- section: declarations ----------------------------------------------- */

fsl_texture *fsl_texture_buf = NULL;
fsl_shader_program *fsl_shader_buf = NULL;
fsl_font *fsl_font_buf = NULL;

fsl_mesh fsl_mesh_unit_quad = {0};
static f32 vbo_data_unit_quad_internal[] =
{
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f,
};

/* ---- section: signatures ------------------------------------------------- */

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief generate texture for `OpenGL` and upload to `GPU` memory.
 *
 *  @param id where to store texture ID.
 *  @param buf texture data to upload to `gpu` memory.
 *
 *  @remark called automatically from @ref fsl_texture_generate() if texture data is
 *  already loaded into a texture by calling @ref fsl_texture_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
static u32 _fsl_texture_generate(GLuint *id, const GLint format_internal,  const GLint format,
        GLint filter, v2i32 size, void *buf, b8 grayscale);

/* ---- section: asset ------------------------------------------------------ */

u32 fsl_asset_init(fsl_asset *asset, enum fsl_asset_type type, const str *name, const str *file, const str *path)
{
    if (!asset)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, 0,
                MSG_ACTION_REASON_ERROR("Initialize Asset", "Pointer `NULL`"));
        return fsl_err;
    }

    if (name && name[0])
    {
        if (!asset->name && fsl_mem_push_arena(&_fsl_memory_arena_debug_internal, (void*)&asset->name,
                    NAME_MAX, "fsl_asset_init().asset->name") != FSL_ERR_SUCCESS)
            goto cleanup;
        if (!asset->name[0])
            snprintf(asset->name, NAME_MAX, "%s", name);
    }

    if (file && file[0])
    {
        if (!asset->file && fsl_mem_push_arena(&_fsl_memory_arena_debug_internal, (void*)&asset->file,
                    NAME_MAX, "fsl_asset_init().asset->file") != FSL_ERR_SUCCESS)
            goto cleanup;
        if (!asset->file[0])
        snprintf(asset->file, NAME_MAX, "%s", file);
    }

    if (path && path[0])
    {
        if (!asset->path && fsl_mem_push_arena(&_fsl_memory_arena_debug_internal, (void*)&asset->path,
                    PATH_MAX, "fsl_asset_init().asset->path") != FSL_ERR_SUCCESS)
            goto cleanup;
        if (!asset->path[0])
        {
            snprintf(asset->path, PATH_MAX, "%s", path);
            fsl_check_slash(asset->path);
            fsl_posix_slash(asset->path);
        }
    }

    asset->type = type;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    /* TODO: use `fsl_mem_pop_arena()` when you make it */
    return fsl_err;
}

u32 fsl_assets_init(void)
{
    i32 i = 0;

    /* ---- engine framebuffers --------------------------------------------- */

    if (
            fsl_fbo_init(&_fsl_core.fbo, &fsl_mesh_unit_quad, FALSE, 4) != FSL_ERR_SUCCESS ||
            fsl_fbo_init(&_fsl_core.fbo_msaa, NULL, TRUE, 4) != FSL_ERR_SUCCESS)
        return fsl_err;

    /* ---- engine textures ------------------------------------------------- */

    if (fsl_mem_push_arena(&_fsl_memory_arena_internal, (void*)&fsl_texture_buf,
                FSL_TEXTURE_INDEX_COUNT * sizeof(fsl_texture), "fsl_assets_init().fsl_texture_buf") != FSL_ERR_SUCCESS)
        return fsl_err;

    if (
            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_ACTIVE],
                "Panel Active", "panel_active.png", FSL_DIR_NAME_TEXTURES,
                GL_RGB, GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_INACTIVE],
                "Panel Inactive", "panel_inactive.png", FSL_DIR_NAME_TEXTURES,
                GL_RGB, GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_PANEL_DEBUG_NINE_SLICE],
                "Panel Debug 9-Slice", "panel_debug_nine_slice.png", FSL_DIR_NAME_TEXTURES,
                GL_RGB, GL_RGB, GL_NEAREST, FSL_COLOR_CHANNELS_RGB, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_BUTTON_SELECTED],
                "Button Selected", "button_selected.png", FSL_DIR_NAME_TEXTURES,
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_BUTTON_ACTIVE],
                "Button Active", "button_active.png", FSL_DIR_NAME_TEXTURES,
                GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE) != FSL_ERR_SUCCESS ||

            fsl_texture_init(&fsl_texture_buf[FSL_TEXTURE_INDEX_BUTTON_INACTIVE],
                    "Button Inactive", "button_inactive.png", FSL_DIR_NAME_TEXTURES,
                    GL_RGBA, GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE) != FSL_ERR_SUCCESS)
                        return fsl_err;

    for (i = 0; i < FSL_TEXTURE_INDEX_COUNT; ++i)
        if (fsl_texture_generate(&fsl_texture_buf[i], FALSE) != FSL_ERR_SUCCESS)
            return fsl_err;

    /* ---- engine meshes --------------------------------------------------- */

    if (fsl_asset_init(&fsl_mesh_unit_quad.asset, FSL_ASSET_MESH, "Unit Quad", NULL, NULL) != FSL_ERR_SUCCESS)
        return fsl_err;

    /* ---- engine shaders -------------------------------------------------- */

    if (fsl_mem_push_arena(&_fsl_memory_arena_internal, (void*)&fsl_shader_buf,
                FSL_SHADER_INDEX_COUNT * sizeof(fsl_shader_program), "fsl_assets_init().fsl_shader_buf") != FSL_ERR_SUCCESS)
        return fsl_err;

    if (
            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].asset, FSL_ASSET_SHADER_PROGRAM,
                "Unit Quad", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].vertex.asset, FSL_ASSET_SHADER,
                "Unit Quad", "unit_quad.vert", FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UNIT_QUAD].fragment.asset, FSL_ASSET_SHADER,
                "Unit Quad", "unit_quad.frag", FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        return fsl_err;

    if (
            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_TEXT].asset, FSL_ASSET_SHADER_PROGRAM,
                "Text", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_TEXT].vertex.asset, FSL_ASSET_SHADER,
                "Text", "text.vert", FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_TEXT].fragment.asset, FSL_ASSET_SHADER,
                "Text", "text.frag", FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        return fsl_err;

    if (
            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UI].asset, FSL_ASSET_SHADER_PROGRAM,
                "UI", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UI].vertex.asset, FSL_ASSET_SHADER,
                "UI", "ui.vert", FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UI].fragment.asset, FSL_ASSET_SHADER,
                "UI", "ui.frag", FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        return fsl_err;

    if (
            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].asset, FSL_ASSET_SHADER_PROGRAM,
                "UI 9-Slice", NULL, NULL) != FSL_ERR_SUCCESS ||

            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].vertex.asset, FSL_ASSET_SHADER,
                "UI 9-Slice", "ui_9_slice.vert", FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS ||

            fsl_asset_init(&fsl_shader_buf[FSL_SHADER_INDEX_UI_9_SLICE].fragment.asset, FSL_ASSET_SHADER,
                "UI 9-Slice", "ui_9_slice.frag", FSL_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        return fsl_err;

    /* ---- engine fonts ---------------------------------------------------- */

    if (fsl_mem_push_arena(&_fsl_memory_arena_internal, (void*)&fsl_font_buf,
                FSL_FONT_INDEX_COUNT * sizeof(fsl_font), "fsl_assets_init().fsl_font_buf") != FSL_ERR_SUCCESS)
        return fsl_err;

    if (
            fsl_font_init(&fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS], FSL_FONT_RESOLUTION_DEFAULT,
                "DejaVu Sans (ANSI)", "dejavu-fonts-ttf/dejavu_sans_ansi.ttf", FSL_DIR_NAME_FONTS) != FSL_ERR_SUCCESS ||

            fsl_font_init(&fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_BOLD], FSL_FONT_RESOLUTION_DEFAULT,
                "DejaVu Sans Bold (ANSI)", "dejavu-fonts-ttf/dejavu_sans_bold_ansi.ttf", FSL_DIR_NAME_FONTS) != FSL_ERR_SUCCESS ||

            fsl_font_init(&fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_MONO], FSL_FONT_RESOLUTION_DEFAULT,
                "DejaVu Sans Mono (ANSI)", "dejavu-fonts-ttf/dejavu_sans_mono_ansi.ttf", FSL_DIR_NAME_FONTS) != FSL_ERR_SUCCESS ||

            fsl_font_init(&fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_MONO_BOLD], FSL_FONT_RESOLUTION_DEFAULT,
                "DejaVu Sans Mono Bold (ANSI)", "dejavu-fonts-ttf/dejavu_sans_mono_bold_ansi.ttf", FSL_DIR_NAME_FONTS) != FSL_ERR_SUCCESS)
        return fsl_err;

    return fsl_err;
}

/* ---- section: fbo -------------------------------------------------------- */

u32 fsl_fbo_init(fsl_fbo *fbo, fsl_mesh *mesh_fbo, b8 multisample, u32 samples)
{
    GLuint status = 0;

    if (fbo == NULL)
        goto mesh_fbo_init;

    fbo->asset.type = FSL_ASSET_FBO;
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

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGFATAL(FSL_ERR_FBO_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_FBO_INIT_FAIL(fbo->fbo, status));
        return fsl_err;
    }

    fbo->asset.loaded = TRUE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

mesh_fbo_init:

    if (mesh_fbo == NULL || mesh_fbo->vbo_data != NULL)
        return FSL_ERR_SUCCESS;

    mesh_fbo->vbo_len = fsl_arr_len(vbo_data_unit_quad_internal);
    if (fsl_mem_push_arena(&_fsl_memory_arena_internal, (void*)&mesh_fbo->vbo_data, sizeof(GLfloat) * mesh_fbo->vbo_len,
                "fbo_init().mesh_fbo.vbo_data") != FSL_ERR_SUCCESS)
        goto cleanup;

    memcpy(mesh_fbo->vbo_data, vbo_data_unit_quad_internal, sizeof(GLfloat) * mesh_fbo->vbo_len);

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

    mesh_fbo->asset.loaded = TRUE;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_fbo_free(fbo);
    fsl_mesh_free(mesh_fbo);
    return fsl_err;
}

u32 fsl_fbo_realloc(fsl_fbo *fbo, b8 multisample, u32 samples)
{
    GLuint status = 0;

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

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        LOGFATAL(FSL_ERR_FBO_REALLOC_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_FBO_INIT_FAIL(fbo->fbo, status));

        fsl_fbo_free(fbo);
        return fsl_err;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    fbo->asset.loaded = TRUE;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_fbo_free(fsl_fbo *fbo)
{
    fsl_fbo nofbo = {0};

    if (!fbo)
        return;

    if (fbo->asset.loaded)
    {
        fbo->asset.loaded = FALSE;
        glDeleteFramebuffers(1, &fbo->rbo);
        glDeleteTextures(1, &fbo->color_buf);
        glDeleteFramebuffers(1, &fbo->fbo);
    }

    *fbo = nofbo;
}

/* ---- section: texture ---------------------------------------------------- */

u32 fsl_texture_init(fsl_texture *texture, const str *name, const str *file, const str *path,
        const GLint format_internal, const GLint format, GLint filter, int channels, b8 grayscale)
{
    str temp[PATH_MAX] = {0};

    snprintf(temp, PATH_MAX, "%s%s", path, file);
    if (fsl_is_file_exists(temp, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    fsl_asset_init(&texture->asset, FSL_ASSET_TEXTURE, name, file, path);

    texture->buf = (u8*)stbi_load(temp, &texture->size.x, &texture->size.y, &texture->channels, channels);
    if (texture->buf == NULL)
    {
        LOGERROR(FSL_ERR_IMAGE_LOAD_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_TEXTURE_LOAD_FAIL(texture->asset.name));
        return fsl_err;
    }

    texture->format = format;
    texture->format_internal = format_internal;
    texture->filter = filter;
    texture->grayscale = grayscale;
    texture->asset.loaded = TRUE;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

u32 fsl_texture_generate(fsl_texture *texture, b8 bindless)
{
    _fsl_texture_generate(&texture->asset.id, texture->format_internal, texture->format,
            texture->filter, texture->size, texture->buf, texture->grayscale);

    texture->generated = TRUE;

    if (fsl_err == FSL_ERR_SUCCESS && bindless)
    {
        texture->bindless = TRUE;
        texture->handle = glGetTextureHandleARB(texture->asset.id);
        glMakeTextureHandleResidentARB(texture->handle);
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_TEXTURE_HANDLE_CREATE(texture->handle, texture->asset.id));
    }

    if (texture->buf)
        stbi_image_free(texture->buf);

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_TEXTURE_GENERATE(texture->asset.name, texture->asset.id));

    return fsl_err;
}

static u32 _fsl_texture_generate(GLuint *id, const GLint format_internal,  const GLint format,
        GLint filter, v2i32 size, void *buf, b8 grayscale)
{
    glGenTextures(1, id);
    glBindTexture(GL_TEXTURE_2D, *id);
    glTexImage2D(GL_TEXTURE_2D, 0, format_internal,
            size.x, size.y, 0, format, GL_UNSIGNED_BYTE, buf);
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
    fsl_texture notexture = {0};

    if (!texture)
        return;

    if (texture->bindless)
    {
        texture->bindless = FALSE;
        glMakeTextureHandleNonResidentARB(texture->handle);
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_TEXTURE_HANDLE_DESTROY(texture->handle, texture->asset.id));
    }

    if (texture->generated)
    {
        texture->generated = FALSE;
        glDeleteTextures(1, &texture->asset.id);
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_TEXTURE_UNLOAD(texture->asset.name, texture->asset.id));
    }

    *texture = notexture;
}

/* ---- section: mesh ------------------------------------------------------- */

u32 fsl_mesh_generate(fsl_mesh *mesh,
        const str *name, const str *file, const str *path,
        void (*attrib)(void), GLenum usage,
        GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data)
{
    if (fsl_mem_push_arena(&_fsl_memory_arena_internal, (void*)&mesh->vbo_data, sizeof(GLfloat) * vbo_len,
                "fsl_mesh_generate().mesh->vbo_data") != FSL_ERR_SUCCESS)
        goto cleanup;

    if (ebo_data)
        if (fsl_mem_push_arena(&_fsl_memory_arena_internal, (void*)&mesh->ebo_data, sizeof(GLuint) * ebo_len,
                    "fsl_mesh_generate().mesh->ebo_data") != FSL_ERR_SUCCESS)
            goto cleanup;

    if (fsl_asset_init(&mesh->asset, FSL_ASSET_MESH, name, file, path) != FSL_ERR_SUCCESS)
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

    mesh->asset.loaded = TRUE;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mesh_free(mesh);
    fsl_err = FSL_ERR_MESH_GENERATION_FAIL;
    return fsl_err;
}

void fsl_mesh_free(fsl_mesh *mesh)
{
    fsl_mesh nomesh = {0};

    if (mesh == NULL)
        return;

    if (mesh->asset.loaded)
    {
        mesh->asset.loaded = FALSE;
        glDeleteBuffers(1, &mesh->ebo);
        glDeleteBuffers(1, &mesh->vbo);
        glDeleteVertexArrays(1, &mesh->vao);
    }

    /* TODO: use `fsl_mem_pop_arena()` when you make it */
    *mesh = nomesh;
}

/* ---- section: font ------------------------------------------------------- */

u32 fsl_font_init(fsl_font *font, u32 resolution,
        const str *name, const str *file, const str *path)
{
    f32 scale = 0.0f;
    u32 i = 0;
    fsl_glyph *g = NULL;
    i32 x0 = 0, y0 = 0;
    i32 x1 = 0, y1 = 0;
    int glyph_index = 0;
    str path_temp[PATH_MAX] = {0};

    if (fsl_asset_init(&font->asset, FSL_ASSET_FONT, name, file, path) != FSL_ERR_SUCCESS)
        return fsl_err;

    if (resolution <= 2)
    {
        LOGERROR(FSL_ERR_IMAGE_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Initialize Font", font->asset.name, "Size Too Small"));
        return fsl_err;
    }

    snprintf(path_temp, PATH_MAX, "%s%s", font->asset.path, font->asset.file);
    if (strlen(path_temp) >= PATH_MAX)
    {
        LOGERROR(FSL_ERR_PATH_TOO_LONG,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Initialize Font", font->asset.name, "Path Too Long"));
        return fsl_err;
    }

    if (fsl_is_file_exists(path_temp, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    font->buf_len = fsl_get_file_contents(path_temp, (void*)&font->buf, 1, TRUE);
    if (!font->buf)
        return fsl_err;

    if (!stbtt_InitFont(&font->info, (const unsigned char*)font->buf, 0))
    {
        LOGERROR(FSL_ERR_FONT_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Initialize Font", font->asset.name, "`stbtt_InitFont()` Failed"));
        goto cleanup;
    }

    if (fsl_mem_alloc((void*)&font->bitmap, FSL_GLYPH_MAX * resolution * resolution,
                fsl_stringf("fsl_font_init().%s", font->asset.name)) != FSL_ERR_SUCCESS)
        goto cleanup;

    stbtt_GetFontVMetrics(&font->info, &font->ascent, &font->descent, &font->line_gap);
    font->resolution = resolution;
    font->line_height = font->ascent - font->descent + font->line_gap;
    font->size = resolution;
    scale = stbtt_ScaleForPixelHeight(&font->info, resolution);

    for (i = 0; i < FSL_GLYPH_MAX; ++i)
    {
        glyph_index = stbtt_FindGlyphIndex(&font->info, i);
        if (!glyph_index) continue;

        g = &font->glyph[i];

        stbtt_GetGlyphHMetrics(&font->info, glyph_index, &g->advance, &g->bearing.x);
        stbtt_GetGlyphBitmapBoxSubpixel(&font->info, glyph_index,
                1.0f, 1.0f, 0.0f, 0.0f, &x0, &y0, &x1, &y1);

        g->bearing.y = y0;
        g->scale.x = x1 - x0;
        g->scale.y = y1 - y0;
        g->scale.x > font->scale.x ? font->scale.x = g->scale.x : 0;
        g->scale.y > font->scale.y ? font->scale.y = g->scale.y : 0;
        g->loaded = TRUE;

        if (!stbtt_IsGlyphEmpty(&font->info, glyph_index))
            stbtt_MakeGlyphBitmapSubpixel(&font->info, font->bitmap + i * resolution * resolution,
                    resolution, resolution, resolution, scale, scale, 0.0f, 0.0f, glyph_index);
    }

    /* ---- generate texture array ------------------------------------------ */

    glGenTextures(1, &font->asset.id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, font->asset.id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RED, resolution, resolution, FSL_GLYPH_MAX);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, resolution, resolution, FSL_GLYPH_MAX, 0, GL_RED, GL_UNSIGNED_BYTE, font->bitmap);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fsl_mem_free((void*)&font->bitmap, FSL_GLYPH_MAX * resolution * resolution,
            fsl_stringf("fsl_font_init().%s", font->asset.name));

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FONT_LOAD(font->asset.name));

    font->asset.loaded = TRUE;
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_font_free(font);
    return fsl_err;
}

void fsl_font_free(fsl_font *font)
{
    fsl_font nofont = {0};
    if (!font)
        return;

    if (font->asset.loaded)
    {
        font->asset.loaded = FALSE;
        glDeleteTextures(1, &font->asset.id);
    }
    fsl_mem_free((void*)&font->buf, font->buf_len, font->asset.name);
    fsl_mem_free((void*)&font->bitmap, FSL_GLYPH_MAX * font->resolution * font->resolution, font->asset.name);

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FONT_UNLOAD(font->asset.name));
    *font = nofont;
}
