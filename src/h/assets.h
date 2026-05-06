/*  @file assets.h
 *
 *  @brief asset types, textures, meshes, etc..
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

#ifndef FSL_ASSETS_H
#define FSL_ASSETS_H

#include "limits.h"
#include "types.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#   include <deps/glad/glad.h>
#pragma GCC diagnostic pop /* ignored "-Wpedantic" */

#include <deps/stb_truetype.h>

enum fsl_asset_type
{
    FSL_ASSET_CUSTOM, /* user defined asset type */
    FSL_ASSET_FBO,
    FSL_ASSET_TEXTURE,
    FSL_ASSET_MESH,
    FSL_ASSET_SHADER,
    FSL_ASSET_SHADER_PROGRAM,
    FSL_ASSET_FONT,
    FSL_ASSET_TYPE_COUNT
}; /* fsl_asset_type */

typedef struct fsl_asset
{
    /*! @remark used by @ref glGenTextures() for textures, @ref glCreateShader() for shaders etc..
     */
    GLuint id;

    enum fsl_asset_type type;

    /*! @brief display name, can be used in asset-search (optional).
     */
    str *name;

    /*! @brief stable, unique name for asset-search, and logging (optional).
     *
     *  naming convention: "[a-z_][a-z0-9_]*", or:
     *      - no leading digits.
     *      - only lowercase characters, digits 0 -> 9 and underscores.
     */
    str *name_id;

    /*! @brief base file name (optional).
     */
    str *file;

    /*! @brief path to asset file without file name (optional).
     */
    str *path;

    /*! @remark `TRUE` does not mean `name`, `name_internal`, `file` and `path` are filled out,
     *  it means whatever the asset requires to be fully initialized and usable is fulfilled
     *  (e.g. texture is generated and uploaded to VRAM).
     */
    b8 loaded;
} fsl_asset;

typedef struct fsl_fbo
{
    fsl_asset asset;
    GLuint fbo;
    GLuint color_buf;
    GLuint rbo;
} fsl_fbo;

typedef struct fsl_texture
{
    fsl_asset asset;
    v2i32 size;

    /*! @brief used by `OpenGL` extension @ref GL_ARB_bindless_texture.
     */
    u64 handle;

    GLint format;           /* used by @ref glTexImage2D() */
    GLint filter;           /* used by @ref glTexParameteri() */

    /*! @brief number of color channels, used by @ref stbi_load().
     */
    int channels;

    b8 grayscale;
    b8 bindless;
} fsl_texture;

typedef struct fsl_mesh
{
    fsl_asset asset;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint vbo_len;
    GLuint ebo_len;
    GLfloat *vbo_data;
    GLuint *ebo_data;
} fsl_mesh;

typedef struct fsl_shader
{
    fsl_asset asset;
    GLint status;       /* used by @ref glGetShaderiv() */
    GLchar *source;     /* shader file source code */
} fsl_shader;

typedef struct fsl_shader_program
{
    fsl_asset asset;
    GLint status;       /* used by @ref glGetProgramiv() */
    fsl_shader vertex;
    fsl_shader geometry;
    fsl_shader fragment;
} fsl_shader_program;

typedef struct fsl_glyph
{
    v2i32 scale;
    v2i32 bearing;
    i32 advance;
    b8 loaded;
} fsl_glyph;

typedef struct fsl_font
{
    fsl_asset asset;
    u32 resolution;         /* glyph bitmap diameter in bytes */
    i32 ascent;             /* glyphs highest points' deviation from baseline */
    i32 descent;            /* glyphs lowest points' deviation from baseline */
    i32 line_gap;
    i32 line_height;
    f32 size;               /* global font size, for text uniformity */
    v2i32 scale;            /* biggest glyph bounding box size in font units */
    u8 *buf;
    u64 buf_len;
    stbtt_fontinfo info;    /* used by @ref stbtt_InitFont() */
    fsl_glyph glyph[FSL_GLYPH_MAX];
} fsl_font;

/* ---- section: declarations ----------------------------------------------- */

/*! @brief engine's default textures.
 *
 *  @remark declared and initialized internally.
 */
FSLAPI extern fsl_texture *fsl_texture_buf;

/*! @brief engine's default unit quad, with texture coordinates.
 *
 *  @remark declared and initialized internally.
 */
FSLAPI extern fsl_mesh fsl_mesh_unit_quad;

/*! @brief engine's default shaders.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_init().
 */
FSLAPI extern fsl_shader_program *fsl_shader_buf;

/*! @brief engine's default fonts.
 *
 *  @remark declared and initialized internally in @ref fsl_ui_init().
 */
FSLAPI extern fsl_font *fsl_font_buf;

/* ---- section: signatures ------------------------------------------------- */

/*! @brief initialize a single asset.
 *
 *  @param type asset type (enum @ref fsl_asset_type).
 *  @param name asset display name (optional).
 *
 *  @param name_id asset stable, unique name for asset-search, and logging (optional).
 *  naming convention: "[a-z_][a-z0-9_]*", or:
 *      - no leading digits.
 *      - only lowercase characters, digits 0 -> 9 and underscores.
 *
 *  @param file base file name (optional).
 *  @param path path to asset file without file name (optional).
 *
 *  @remark does not modify @ref asset.loaded.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_asset_init(fsl_asset *asset, enum fsl_asset_type type,
        const str *name, const str *name_id, const str *file, const str *path);

/*! @brief initialize engine's internal assets.
 *
 *  @remark called automatically from @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_assets_init(void);

/*! @brief free all engine's internal assets.
 */
FSLAPI void fsl_assets_free(void);

/*! @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_fbo_init(fsl_fbo *fbo, fsl_mesh *mesh_fbo, b8 multisample, u32 samples);

/*! @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_fbo_realloc(fsl_fbo *fbo, b8 multisample, u32 samples);

FSLAPI void fsl_fbo_free(fsl_fbo *fbo);

/*! @brief load image data from disk into @ref texture->buf and set texture info.
 *
 *  @param name asset display name (optional) (@ref fsl_asset_init() parameter).
 *
 *  @param name_id asset stable, unique name for asset-search, and logging (optional) (@ref fsl_asset_init() parameter).
 *  naming convention: "[a-z_][a-z0-9_]*", or:
 *      - no leading digits.
 *      - only lowercase characters, digits 0 -> 9 and underscores.
 *
 *  @param file base file name (optional) (@ref fsl_asset_init() parameter).
 *  @param path path to asset file without file name (optional) (@ref fsl_asset_init() parameter).
 *
 *  @param bindless use 'OpenGL' extension 'GL_ARB_bindless_texture'
 *  (handle is in @ref texture->handle).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_texture_init(fsl_texture *texture,
        const str *name, const str *name_id, const str *file, const str *path,
        const GLint format, GLint filter, int channels, b8 grayscale, b8 bindless);

FSLAPI void fsl_texture_free(fsl_texture *texture);

/*! @param attrib pointer to a function to set attribute arrays for `mesh->vao`
 *  (e.g. &attrib_vec3, set a single vec3 attribute array).
 *
 *  @param name_id asset stable, unique name for asset-search, and logging (optional) (@ref fsl_asset_init() parameter).
 *  naming convention: "[a-z_][a-z0-9_]*", or:
 *      - no leading digits.
 *      - only lowercase characters, digits 0 -> 9 and underscores.
 *
 *  @param file base file name (optional) (@ref fsl_asset_init() parameter).
 *  @param path path to asset file without file name (optional) (@ref fsl_asset_init() parameter).
 *  @param usage `GL_<x>_DRAW`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mesh_generate(fsl_mesh *mesh,
        const str *name, const str *name_id, const str *file, const str *path,
        void (*attrib)(void), GLenum usage,
        GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data);

FSLAPI void fsl_mesh_free(fsl_mesh *mesh);

/*! @brief load font from file at `file_name` or at `font->path`.
 *
 *  1. allocate memory for `font->buf` and load file contents into it in binary format.
 *  2. allocate memory for `font->bitmap` and render glyphs onto it.
 *  3. generate square texture of diameter `resolution` * 16 and bake bitmap onto it.
 *
 *  @param resolution font size (font atlas cell diameter).
 *  @param name asset display name (optional) (@ref fsl_asset_init() parameter).
 *
 *  @param name_id asset stable, unique name for asset-search, and logging (optional) (@ref fsl_asset_init() parameter).
 *  naming convention: "[a-z_][a-z0-9_]*", or:
 *      - no leading digits.
 *      - only lowercase characters, digits 0 -> 9 and underscores.
 *
 *  @param file base file name (optional) (@ref fsl_asset_init() parameter).
 *  @param path path to asset file without file name (optional) (@ref fsl_asset_init() parameter).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_font_init(fsl_font *font, u32 resolution,
        const str *name, const str *name_id, const str *file, const str *path);

FSLAPI void fsl_font_free(fsl_font *font);

#endif /* FSL_ASSETS_H */
