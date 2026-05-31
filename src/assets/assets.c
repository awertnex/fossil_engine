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
 *  @file assets.c
 *
 *  @brief asset parsing, loading and unloading.
 */

#include "../common/config.h"
#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../common/types.h"
#include "../logger/logger.h"
#include "../logger/logger_messages_internal.h"
#include "../math/math.h"
#include "../math/matrix.h"
#include "../math/trigonometry.h"
#include "../math/vector.h"
#include "../memory/memory.h"

#include "../h/dir.h"

#include "assets.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#   define STB_TRUETYPE_IMPLEMENTATION
#   include "../external/stb_truetype.h"
#   define STB_IMAGE_IMPLEMENTATION
#   include "../external/stb_image.h"
#pragma GCC diagnostic pop /* ignored "-Wpedantic" */

#include <stdio.h>
#include <string.h>

static f32 vbo_data_unit_quad_internal[] =
{
    -1.0f, -1.0f, 0.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, -1.0f, 1.0f, 0.0f
};

/* ---- section: asset ------------------------------------------------------ */

fsl_asset_metadata fsl_asset_get_metadata(fsl_asset asset)
{
    fsl_asset_metadata metadata = {0};
    if (asset.name.arena)
        metadata.name = fsl_mem_handle_get(asset.name);
    if (asset.name_id.arena)
        metadata.name_id = fsl_mem_handle_get(asset.name_id);
    if (asset.file.arena)
        metadata.file = fsl_mem_handle_get(asset.file);
    if (asset.path.arena)
        metadata.path = fsl_mem_handle_get(asset.path);
    return metadata;
}

u32 fsl_asset_set_metadata(fsl_asset *asset, fsl_asset_type type,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path)
{
    fsl_name *name_p = NULL;
    fsl_name_id *name_id_p = NULL;
    fsl_file *file_p = NULL;
    fsl_path *path_p = NULL;

    if (asset == NULL)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, 0,
                MSG_ACTION_REASON_ERROR("Set Asset Metadata", "Pointer `NULL`"));
        return fsl_err;
    }

    asset->type = type;

    if (name)
    {
        if (fsl_mem_arena_push(&mem_arena_name_internal, &asset->name, FSL_ID_CAP,
                    "fsl_asset_set_metadata().asset->name") != FSL_ERR_SUCCESS)
            goto cleanup;
        name_p = fsl_mem_handle_get(asset->name),
        snprintf(name_p, FSL_ID_CAP, "%s", name);
    }

    if (name_id)
    {
        if (fsl_mem_arena_push(&mem_arena_name_id_internal, &asset->name_id, FSL_ID_CAP,
                    "fsl_asset_set_metadata().asset->name_id") != FSL_ERR_SUCCESS)
            goto cleanup;
        name_id_p = fsl_mem_handle_get(asset->name_id);
        snprintf(name_id_p, FSL_ID_CAP, "%s", name_id);
    }

    if (file)
    {
        if (fsl_mem_arena_push(&mem_arena_file_internal, &asset->file, FSL_ID_CAP,
                    "fsl_asset_set_metadata().asset->file") != FSL_ERR_SUCCESS)
            goto cleanup;
        file_p = fsl_mem_handle_get(asset->file);
        snprintf(file_p, FSL_ID_CAP, "%s", file);
    }

    if (path)
    {
        if (fsl_mem_arena_push(&mem_arena_path_internal, &asset->path, FSL_ID_CAP,
                    "fsl_asset_set_metadata().asset->path") != FSL_ERR_SUCCESS)
            goto cleanup;
        path_p = fsl_mem_handle_get(asset->path);
        snprintf(path_p, FSL_ID_CAP, "%s", path);
        fsl_check_slash(path_p);
        fsl_posix_slash(path_p);
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    /* TODO: use `fsl_mem_pop_arena()` when you make it */
    return fsl_err;
}

/* ---- section: vbo -------------------------------------------------------- */

FSLAPI u32 fsl_vbo_init(fsl_vbo *vbo, fsl_size size, fsl_len len, void *data,
        GLenum target, fsl_draw_type draw_type)
{
    GLenum draw_type_temp = 0;

    if (!vbo)
    {
        LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_POINTER_NULL_ACTION("Initialize VBO"));
    }

    if (vbo->initialized)
        return FSL_ERR_SUCCESS;

    if (!vbo->buf.arena && fsl_mem_arena_push(&mem_arena_sub_data_internal, &vbo->buf, size * len,
                "fsl_vbo_init().vbo->buf") != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_VBO_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Initialize VBO", "`fsl_mem_arena_push()` Failed"));
        return fsl_err;
    }

    switch(draw_type)
    {
        case FSL_DRAW_TYPE_STREAM:
            draw_type_temp = GL_STREAM_DRAW;
            break;
        case FSL_DRAW_TYPE_STATIC:
            draw_type_temp = GL_STATIC_DRAW;
            break;
        case FSL_DRAW_TYPE_DYNAMIC:
            draw_type_temp = GL_DYNAMIC_DRAW;
            break;
        default:
            draw_type_temp = GL_STATIC_DRAW;
    }

    glGenBuffers(1, &vbo->id);
    glBindBuffer(target, vbo->id);
    glBufferData(target, size * len, data, draw_type_temp);

    vbo->size = size;
    vbo->len = len;
    vbo->initialized = TRUE;
    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_VBO_INIT(vbo->id, size * len));
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_vbo_free(fsl_vbo *vbo)
{
    fsl_vbo novbo = {0};
    GLuint id = 0;

    if (!vbo)
        return;

    id = vbo->id;

    if (vbo->initialized)
    {
        vbo->initialized = FALSE;
        glDeleteBuffers(1, &vbo->id);
        fsl_mem_arena_pop(&vbo->buf, "fsl_vbo_free().vbo->buf");
    }

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_VBO_FREE(id, vbo->size * vbo->len));
    *vbo = novbo;
}

/* ---- section: fbo -------------------------------------------------------- */

u32 fsl_fbo_init(fsl_fbo *fbo, i32 size_x, i32 size_y, fsl_mesh *mesh_fbo,
        b8 multisample, u32 samples)
{
    GLuint status = 0;
    GLfloat *mesh_fbo_vbo_data = NULL;

    if (!fbo || fbo->asset.initialized)
        goto mesh_fbo_init;

    fbo->asset.type = FSL_ASSET_FBO;

    glGenFramebuffers(1, &fbo->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

    if (multisample)
    {
        /* ---- color buffer ------------------------------------------------ */

        glGenTextures(1, &fbo->color_buf);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA,
                size_x, size_y, GL_TRUE);

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
                GL_DEPTH_COMPONENT24, size_x, size_y);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->rbo);
    }
    else
    {
        /* ---- color buffer ------------------------------------------------ */

        glGenTextures(1, &fbo->color_buf);
        glBindTexture(GL_TEXTURE_2D, fbo->color_buf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                size_x, size_y, 0, GL_RGBA, GL_FLOAT, NULL);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D, fbo->color_buf, 0);

        /* ---- render buffer ----------------------------------------------- */

        glGenRenderbuffers(1, &fbo->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size_x, size_y);
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

    fbo->asset.initialized = TRUE;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

mesh_fbo_init:

    if (mesh_fbo == NULL || mesh_fbo->vbo_data.arena != NULL)
        return FSL_ERR_SUCCESS;

    mesh_fbo->vbo_len = fsl_arr_len(vbo_data_unit_quad_internal);
    if (fsl_mem_arena_push(&mem_arena_sub_data_internal, &mesh_fbo->vbo_data, sizeof(GLfloat) * mesh_fbo->vbo_len,
                "fsl_fbo_init().mesh_fbo->vbo_data") != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_asset_set_metadata(&mesh_fbo->asset, FSL_ASSET_MESH, "Unit Quad", "unit_quad", NULL, NULL);
    mesh_fbo_vbo_data = fsl_mem_handle_get(mesh_fbo->vbo_data);
    memcpy(mesh_fbo_vbo_data, vbo_data_unit_quad_internal, mesh_fbo->vbo_data.size);

    /* ---- bind mesh ------------------------------------------------------- */

    glGenVertexArrays(1, &mesh_fbo->vao);
    glGenBuffers(1, &mesh_fbo->vbo);

    glBindVertexArray(mesh_fbo->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh_fbo->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh_fbo->vbo_len * sizeof(GLfloat),
            fsl_mem_handle_get(mesh_fbo->vbo_data), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)(2 * sizeof(GLfloat)));

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh_fbo->asset.initialized = TRUE;
    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FBO_INIT(fbo->fbo, size_x, size_y));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_fbo_free(fbo);
    fsl_mesh_free(mesh_fbo);
    return fsl_err;
}

u32 fsl_fbo_realloc(fsl_fbo *fbo, i32 size_x, i32 size_y, b8 multisample, u32 samples)
{
    GLuint status = 0;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);

    if (multisample)
    {
        /* ---- color buffer ------------------------------------------------ */

        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_RGBA,
                size_x, size_y, GL_TRUE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                GL_TEXTURE_2D_MULTISAMPLE, fbo->color_buf, 0);

        /* ---- render buffer ----------------------------------------------- */

        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                GL_DEPTH_COMPONENT24, size_x, size_y);
    }
    else
    {
        /* ---- color buffer ------------------------------------------------ */

        glBindTexture(GL_TEXTURE_2D, fbo->color_buf);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                size_x, size_y, 0, GL_RGBA, GL_FLOAT, NULL);

        /* ---- render buffer ----------------------------------------------- */

        glBindRenderbuffer(GL_RENDERBUFFER, fbo->rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
                size_x, size_y);
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

    fbo->asset.initialized = TRUE;
    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FBO_REALLOC(fbo->fbo, size_x, size_y));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_fbo_free(fsl_fbo *fbo)
{
    fsl_fbo nofbo = {0};
    GLuint id = 0;

    if (!fbo)
        return;

    id = fbo->fbo;

    if (fbo->asset.initialized)
    {
        fbo->asset.initialized = FALSE;
        glDeleteFramebuffers(1, &fbo->rbo);
        glDeleteTextures(1, &fbo->color_buf);
        glDeleteFramebuffers(1, &fbo->fbo);
    }

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FBO_FREE(id));

    *fbo = nofbo;
}

/* ---- section: texture ---------------------------------------------------- */

u32 fsl_texture_init(fsl_texture *texture,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path,
        const GLint format, GLint filter, int channels, b8 grayscale, b8 bindless)
{
    str path_temp[FSL_PATH_CAP] = {0};
    u8 *buf = NULL;
    fsl_asset_metadata metadata = {0};

    if (fsl_asset_set_metadata(&texture->asset, FSL_ASSET_TEXTURE, name, name_id, file, path) != FSL_ERR_SUCCESS)
        goto cleanup;

    metadata = fsl_asset_get_metadata(texture->asset);

    snprintf(path_temp, FSL_PATH_CAP, "%s%s", metadata.path, metadata.file);
    if (fsl_is_file_exists(path_temp, TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    buf = (u8*)stbi_load(path_temp, &texture->size.x, &texture->size.y, &texture->channels, channels);
    if (buf == NULL)
    {
        LOGERROR(FSL_ERR_IMAGE_LOAD_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_TEXTURE_LOAD_REASON_FAIL(metadata.name_id, "`stbi_load()` Failed"));
        goto cleanup;
    }

    glGenTextures(1, &texture->asset.id);
    glBindTexture(GL_TEXTURE_2D, texture->asset.id);
    glTexImage2D(GL_TEXTURE_2D, 0, format,
            texture->size.x, texture->size.y, 0, format, GL_UNSIGNED_BYTE, buf);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    if (grayscale)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (bindless)
    {
        texture->bindless = TRUE;
        texture->bindless_handle = glGetTextureHandleARB(texture->asset.id);
        glMakeTextureHandleResidentARB(texture->bindless_handle);
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_TEXTURE_HANDLE_CREATE(texture->bindless_handle, metadata.name_id, texture->asset.id));
    }

    if (buf)
        stbi_image_free(buf);

    texture->asset.initialized = TRUE;
    texture->format = format;
    texture->filter = filter;
    texture->grayscale = grayscale;

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_TEXTURE_LOAD(metadata.name_id, texture->asset.id));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    if (buf)
        stbi_image_free(buf);
    fsl_texture_free(texture);
    return fsl_err;
}

void fsl_texture_free(fsl_texture *texture)
{
    fsl_texture notexture = {0};
    fsl_asset_metadata metadata = {0};

    if (texture == NULL)
        return;

    metadata = fsl_asset_get_metadata(texture->asset);

    if (texture->bindless)
    {
        texture->bindless = FALSE;
        glMakeTextureHandleNonResidentARB(texture->bindless_handle);
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_TEXTURE_HANDLE_DESTROY(texture->bindless_handle, metadata.name_id, texture->asset.id));
    }

    if (texture->asset.initialized)
    {
        texture->asset.initialized = FALSE;
        glDeleteTextures(1, &texture->asset.id);
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_TEXTURE_UNLOAD(metadata.name_id, texture->asset.id));
    }

    *texture = notexture;
}

/* ---- section: font ------------------------------------------------------- */

u32 fsl_font_init(fsl_font *font, u32 resolution,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path)
{
    stbtt_fontinfo *info = NULL;
    u32 i = 0;
    str path_temp[FSL_PATH_CAP] = {0};
    u8 *file_contents = NULL;
    u8 *bitmap = NULL;
    f32 scale = 0.0f;
    int glyph_index = 0;
    fsl_glyph *g = NULL;
    i32 x0 = 0, y0 = 0;
    i32 x1 = 0, y1 = 0;
    fsl_asset_metadata metadata = {0};

    if (fsl_asset_set_metadata(&font->asset, FSL_ASSET_FONT, name, name_id, file, path) != FSL_ERR_SUCCESS)
        return fsl_err;

    metadata = fsl_asset_get_metadata(font->asset);

    if (resolution <= 2)
    {
        LOGERROR(FSL_ERR_IMAGE_SIZE_TOO_SMALL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Initialize Font", metadata.name_id, "Size Too Small"));
        return fsl_err;
    }

    snprintf(path_temp, FSL_PATH_CAP, "%s%s", metadata.path, metadata.file);
    if (fsl_is_file_exists(path_temp, TRUE) != FSL_ERR_SUCCESS)
        return fsl_err;

    font->buf_len = fsl_get_file_contents(path_temp, (void*)&file_contents, TRUE);
    if (file_contents == NULL)
        return fsl_err;

    if (fsl_mem_arena_push(&mem_arena_internal, &font->info, sizeof(stbtt_fontinfo),
                "fsl_font_init().font->info") != FSL_ERR_SUCCESS)
        goto cleanup;
    info = fsl_mem_handle_get(font->info);

    if (!stbtt_InitFont(info, (const unsigned char*)file_contents, 0))
    {
        LOGERROR(FSL_ERR_FONT_INIT_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Initialize Font", metadata.name_id, "`stbtt_InitFont()` Failed"));
        goto cleanup;
    }

    if (fsl_mem_alloc((void*)&bitmap, FSL_GLYPH_MAX * resolution * resolution,
                "fsl_font_init().bitmap") != FSL_ERR_SUCCESS)
        goto cleanup;

    /* this line, stolen from 'stb_truetype.h' v1.26, function `stbtt_ScaleForPixelHeight()` */
    font->fheight = ttSHORT(info->data + info->hhea + 4) - ttSHORT(info->data + info->hhea + 6);

    stbtt_GetFontVMetrics(info, &font->ascent, &font->descent, &font->line_gap);
    font->resolution = resolution;
    font->line_height = font->ascent - font->descent + font->line_gap;
    font->size = resolution;
    scale = (f32)resolution / font->fheight;

    for (i = 0; i < FSL_GLYPH_MAX; ++i)
    {
        glyph_index = stbtt_FindGlyphIndex(info, i);
        if (!glyph_index)
            continue;

        g = &font->glyph[i];

        stbtt_GetGlyphHMetrics(info, glyph_index, &g->advance, &g->bearing.x);
        stbtt_GetGlyphBitmapBoxSubpixel(info, glyph_index,
                1.0f, 1.0f, 0.0f, 0.0f, &x0, &y0, &x1, &y1);

        g->bearing.y = y0;
        g->scale.x = x1 - x0;
        g->scale.y = y1 - y0;
        g->scale.x > font->scale.x ? font->scale.x = g->scale.x : 0;
        g->scale.y > font->scale.y ? font->scale.y = g->scale.y : 0;
        g->loaded = TRUE;

        if (!stbtt_IsGlyphEmpty(info, glyph_index))
            stbtt_MakeGlyphBitmapSubpixel(info, bitmap + i * resolution * resolution,
                    resolution, resolution, resolution, scale, scale, 0.0f, 0.0f, glyph_index);
    }

    /* ---- generate texture array ------------------------------------------ */

    glGenTextures(1, &font->asset.id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, font->asset.id);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RED, resolution, resolution, FSL_GLYPH_MAX);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, resolution, resolution, FSL_GLYPH_MAX, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    fsl_mem_free((void*)&bitmap, FSL_GLYPH_MAX * resolution * resolution,
            "fsl_font_init().bitmap");

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FONT_LOAD(metadata.name_id));

    font->asset.initialized = TRUE;
    font->buf = file_contents;
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mem_free((void*)&file_contents, font->buf_len, "fsl_font_init().file_contents");
    fsl_mem_free((void*)&bitmap, FSL_GLYPH_MAX * resolution * resolution, "fsl_font_init().bitmap");
    fsl_mem_arena_pop(&font->info, "fsl_font_init().font->info");
    fsl_font_free(font);
    return fsl_err;
}

void fsl_font_free(fsl_font *font)
{
    fsl_font nofont = {0};

    if (!font)
        return;

    if (font->asset.initialized)
    {
        font->asset.initialized = FALSE;
        glDeleteTextures(1, &font->asset.id);
    }

    if (font->buf)
        fsl_mem_free((void*)&font->buf, font->buf_len, "fsl_font_free().font->buf");
    if (font->info.arena)
        fsl_mem_arena_pop(&font->info, "fsl_font_free().font->info");

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_FONT_UNLOAD(fsl_mem_handle_get(font->asset.name_id)));
    *font = nofont;
}

/* ---- section: camera ----------------------------------------------------- */

void fsl_update_camera_movement(fsl_camera *camera, b8 roll)
{
    if (roll)
    {
        camera->roll.angle = fmod(camera->roll.angle, FSL_CAMERA_RANGE_MAX);
        if (camera->roll.angle < 0.0) camera->roll.angle += FSL_CAMERA_RANGE_MAX;
    }
    else camera->roll.angle = 0.0;

    camera->pitch.angle = fsl_clamp_f64(camera->pitch.angle, -FSL_CAMERA_ANGLE_MAX, FSL_CAMERA_ANGLE_MAX);
    camera->yaw.angle = fmod(camera->yaw.angle, FSL_CAMERA_RANGE_MAX);
    if (camera->yaw.angle < 0.0) camera->yaw.angle += FSL_CAMERA_RANGE_MAX;

    camera->roll.sin = sin(camera->roll.angle * FSL_DEG2RAD);
    camera->roll.cos = cos(camera->roll.angle * FSL_DEG2RAD);
    camera->pitch.sin = sin(camera->pitch.angle * FSL_DEG2RAD);
    camera->pitch.cos = cos(camera->pitch.angle * FSL_DEG2RAD);
    camera->yaw.sin = sin(camera->yaw.angle * FSL_DEG2RAD);
    camera->yaw.cos = cos(camera->yaw.angle * FSL_DEG2RAD);

    fsl_update_projection_perspective(*camera, &camera->projection, roll);
}

void fsl_update_projection_perspective(fsl_camera camera, fsl_projection *projection, b8 roll)
{
    const f32 SROL = camera.roll.sin;
    const f32 CROL = camera.roll.cos;
    const f32 SPCH = camera.pitch.sin;
    const f32 CPCH = camera.pitch.cos;
    const f32 SYAW = camera.yaw.sin;
    const f32 CYAW = camera.yaw.cos;
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

    /* TODO: try to fix 'roll' rotation in camera projection matrix */
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
