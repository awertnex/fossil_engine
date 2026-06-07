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
 *  @file asset_types.h
 *
 *  @brief asset types.
 */

#ifndef FSL_ASSET_TYPES_H
#define FSL_ASSET_TYPES_H

#include "../common/limits.h"
#include "../common/types.h"
#include "../math/matrix.h"
#include "../math/trigonometry.h"
#include "../math/vector.h"
#include "../memory/memory_types.h"

#include "../external/glad/glad.h"

/*!
 *  @brief an asset's display name (optional).
 */
typedef str fsl_name;

/*!
 *  @brief an asset's stable, unique name for asset-search, and logging.
 *  naming convention: "[a-z_][a-z0-9_]*", or:
 *      - no leading digits.
 *      - only lowercase characters, digits 0 -> 9 and underscores.
 */
typedef str fsl_name_id;

/*!
 *  @brief an asset's file base name.
*/
typedef str fsl_file;

/*!
 *  @brief an asset file's parent directory path.
 */
typedef str fsl_path;

typedef struct fsl_asset            fsl_asset;
typedef struct fsl_asset_metadata   fsl_asset_metadata;
typedef struct fsl_vbo              fsl_vbo;
typedef struct fsl_vbo              fsl_ebo;
typedef struct fsl_fbo              fsl_fbo;
typedef struct fsl_texture          fsl_texture;
typedef struct fsl_glyph            fsl_glyph;
typedef struct fsl_font             fsl_font;
typedef struct fsl_projection       fsl_projection;
typedef struct fsl_camera           fsl_camera;

typedef enum fsl_asset_type
{
    FSL_ASSET_CUSTOM = 0, /* user defined asset types */
    FSL_ASSET_FBO = 1,
    FSL_ASSET_TEXTURE = 2,
    FSL_ASSET_MESH = 3,
    FSL_ASSET_SHADER = 4,
    FSL_ASSET_SHADER_PROGRAM = 5,
    FSL_ASSET_FONT = 6,
    FSL_ASSET_TYPE_COUNT
} fsl_asset_type;

/*!
 *  @remark this struct should be filled using the function @ref fsl_set_asset_metadata().
 */
struct fsl_asset
{
    /*!
     *  @remark used by @ref glGenTextures() for textures, @ref glCreateShader() for shaders etc..
     */
    GLuint id;

    fsl_asset_type type;

    /*!
     *  @brief display name, can be used in asset-search (optional).
     */
    fsl_mem_handle name;

    /*!
     *  @brief stable, unique name for asset-search, and logging (optional).
     *
     *  naming convention: "[a-z_][a-z0-9_]*", or:
     *      - no leading digits.
     *      - only lowercase characters, digits 0 -> 9 and underscores.
     */
    fsl_mem_handle name_id;

    /*!
     *  @brief base file name (optional).
     */
    fsl_mem_handle file;

    /*!
     *  @brief path to asset file without file name (optional).
     */
    fsl_mem_handle path;

    /*!
     *  @remark `TRUE` does not mean 'has metadata', it means whatever the asset
     *  requires to be fully initialized and usable is fulfilled
     *  (e.g., texture generated and uploaded to VRAM).
     */
    b8 initialized;
}; /* fsl_asset */

/*!
 *  @remark this struct can be filled using the function @ref fsl_get_asset_metadata().
 */
struct fsl_asset_metadata
{
    fsl_name *name;
    fsl_name_id *name_id;
    fsl_file *file;
    fsl_path *path;
}; /* fsl_asset_metadata */

struct fsl_vbo
{
    GLuint id;
    fsl_size size;      /* size of each element in `buf` */
    fsl_size len;       /* number of elements in `buf` */
    fsl_mem_handle buf;
    b8 initialized;
}; /* fsl_vbo */

struct fsl_fbo
{
    fsl_asset asset;
    GLuint fbo;
    GLuint color_buf;
    GLuint rbo;
}; /* fsl_fbo */

struct fsl_texture
{
    fsl_asset asset;
    v2i32 size;

    /*!
     *  @brief used by 'OpenGL' extension @ref GL_ARB_bindless_texture.
     */
    u64 bindless_handle;

    GLint format; /* used by @ref glTexImage2D() */
    GLint filter; /* used by @ref glTexParameteri() */

    /*!
     *  @brief number of color channels, used by @ref stbi_load().
     */
    int channels;

    b8 grayscale;
    b8 bindless;
}; /* fsl_texture */

struct fsl_glyph
{
    v2i32 scale;
    v2i32 bearing;
    i32 advance;
    b8 loaded;
}; /* fsl_glyph */

struct fsl_font
{
    fsl_asset asset;
    u32 resolution;         /* glyph bitmap diameter, in bytes */
    i32 ascent;             /* glyphs highest points' deviation from baseline */
    i32 descent;            /* glyphs lowest points' deviation from baseline */
    i32 line_gap;
    i32 line_height;

    /*
     *  divisor for converting 'font units' to 'pixel height'.
     */
    i32 fheight;

    f32 size;               /* global font size, for text uniformity */
    v2i32 scale;            /* biggest glyph bounding box size, in font units */
    u8* buf;                /* font file contents (used in runtime) */
    u64 buf_len;            /* size allocated for `buf`, in bytes */
    fsl_mem_handle info;    /* used by @ref stbtt_InitFont() */
    fsl_glyph glyph[FSL_GLYPH_MAX];
}; /* fsl_font */

struct fsl_projection
{
    m4f32 target;
    m4f32 translation;
    m4f32 rotation;
    m4f32 orientation;
    m4f32 view;
    m4f32 projection;
    m4f32 perspective;
}; /* fsl_projection */

struct fsl_camera
{
    v3f64 pos;
    angle_f64 roll;
    angle_f64 pitch;
    angle_f64 yaw;
    f32 fovy;
    f32 fovy_smooth;
    f32 ratio;
    f32 far;
    f32 near;
    f32 zoom;
    fsl_projection projection;
}; /* fsl_camera */

#endif /* FSL_ASSET_TYPES_H */
