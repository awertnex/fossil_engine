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
 *  @file assets.h
 *
 *  @brief parsing, loading and unloading assets.
 */

#ifndef FSL_ASSETS_H
#define FSL_ASSETS_H

#include "../common/engine_info.h"
#include "../common/limits.h"
#include "../common/types.h"
#include "../memory/memory_types.h"

#include "asset_types.h"

#include <deps/glad/glad.h>

/* ---- section: declarations ----------------------------------------------- */

/*!
 *  @brief engine's default textures.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_assets_init().
 */
FSLAPI extern fsl_mem_handle fsl_texture_buf;

/*!
 *  @brief engine's default shaders.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_assets_init().
 */
FSLAPI extern fsl_mem_handle fsl_shader_buf;

/*!
 *  @brief engine's default fonts.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_assets_init().
 */
FSLAPI extern fsl_mem_handle fsl_font_buf;

/*!
 *  @brief engine's default unit quad, with texture coordinates.
 *
 *  @remark read-only, declared and initialized internally in @ref fsl_assets_init().
 */
FSLAPI extern fsl_mesh fsl_mesh_unit_quad;

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @remark get asset metadata.
 */
FSLAPI fsl_asset_metadata fsl_asset_get_metadata(const fsl_asset asset);

/*!
 *  @brief initialize a single asset.
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
 *  @remark does not modify @ref asset.initialized.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_asset_set_metadata(fsl_asset *asset, enum fsl_asset_type type,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path);

/*!
 *  @brief initialize engine's internal assets.
 *
 *  @remark called automatically from @ref fsl_engine_init().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_assets_init(void);

/*!
 *  @brief free all engine's internal assets.
 */
FSLAPI void fsl_assets_free(void);

/*!
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_fbo_init(fsl_fbo *fbo, fsl_mesh *mesh_fbo, b8 multisample, u32 samples);

/*!
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_fbo_realloc(fsl_fbo *fbo, b8 multisample, u32 samples);

FSLAPI void fsl_fbo_free(fsl_fbo *fbo);

/*!
 *  @brief load image data from disk into @ref texture->buf and set texture info.
 *
 *  @param name asset display name (optional) (@ref fsl_asset_set_metadata() parameter).
 *
 *  @param name_id asset stable, unique name for asset-search, and logging (optional) (@ref fsl_asset_set_metadata() parameter).
 *  naming convention: "[a-z_][a-z0-9_]*", or:
 *      - no leading digits.
 *      - only lowercase characters, digits 0 -> 9 and underscores.
 *
 *  @param file base file name (optional) (@ref fsl_asset_set_metadata() parameter).
 *  @param path path to asset file without file name (optional) (@ref fsl_asset_set_metadata() parameter).
 *
 *  @param bindless use 'OpenGL' extension 'GL_ARB_bindless_texture'
 *  (handle is in @ref texture->handle).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_texture_init(fsl_texture *texture,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path,
        const GLint format, GLint filter, int channels, b8 grayscale, b8 bindless);

FSLAPI void fsl_texture_free(fsl_texture *texture);

/*!
 *  @param attrib pointer to a function to set attribute arrays for `mesh->vao`
 *  (e.g., &attrib_vec3, set a single vec3 attribute array).
 *
 *  @param name_id asset stable, unique name for asset-search, and logging (optional) (@ref fsl_asset_set_metadata() parameter).
 *  naming convention: "[a-z_][a-z0-9_]*", or:
 *      - no leading digits.
 *      - only lowercase characters, digits 0 -> 9 and underscores.
 *
 *  @param file base file name (optional) (@ref fsl_asset_set_metadata() parameter).
 *  @param path path to asset file without file name (optional) (@ref fsl_asset_set_metadata() parameter).
 *  @param usage `GL_<x>_DRAW`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mesh_generate(fsl_mesh *mesh,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path,
        void (*attrib)(void), GLenum usage,
        GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data);

FSLAPI void fsl_mesh_free(fsl_mesh *mesh);

/*!
 *  @brief load font from file at `path`/`file`.
 *
 *  - generate square texture of diameter `resolution` * 16 and bake bitmap onto it.
 *
 *  @param resolution font size (font atlas cell diameter).
 *  @param name asset display name (optional) (@ref fsl_asset_set_metadata() parameter).
 *
 *  @param name_id asset stable, unique name for asset-search, and logging (optional) (@ref fsl_asset_set_metadata() parameter).
 *  naming convention: "[a-z_][a-z0-9_]*", or:
 *      - no leading digits.
 *      - only lowercase characters, digits 0 -> 9 and underscores.
 *
 *  @param file base file name (optional) (@ref fsl_asset_set_metadata() parameter).
 *  @param path path to asset file without file name (optional) (@ref fsl_asset_set_metadata() parameter).
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_font_init(fsl_font *font, u32 resolution,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path);

FSLAPI void fsl_font_free(fsl_font *font);

#endif /* FSL_ASSETS_H */
