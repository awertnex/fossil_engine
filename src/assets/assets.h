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
 *  @brief asset parsing, loading and unloading.
 */

#ifndef FSL_ASSETS_H
#define FSL_ASSETS_H

#include "../common/api.h"
#include "../common/types.h"
#include "../math/vector.h"

#include "asset_types.h"
#include "mesh/mesh.h"

typedef enum fsl_draw_type
{
    FSL_DRAW_TYPE_NONE,
    FSL_DRAW_TYPE_STREAM,
    FSL_DRAW_TYPE_STATIC,
    FSL_DRAW_TYPE_DYNAMIC
} fsl_draw_type;

enum fsl_text_alignment
{
    FSL_TEXT_ALIGN_LEFT = 0,
    FSL_TEXT_ALIGN_CENTER = 1,
    FSL_TEXT_ALIGN_RIGHT = 2,
    FSL_TEXT_ALIGN_TOP = 0,
    FSL_TEXT_ALIGN_BOTTOM = 2
}; /* fsl_text_alignment */

/*!
 *  @brief get an asset's metadata (e.g., name, name_id, etc..).
 */
FSLAPI fsl_asset_metadata fsl_asset_get_metadata(fsl_asset asset);

/*!
 *  @brief set a single asset's metadata.
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
 *  @remark does not modify @ref asset->initialized.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_asset_set_metadata(fsl_asset *asset, fsl_asset_type type,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path);

/*!
 *  @param target 'OpenGL' buffer target (e.g., GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER).
 *  @param size size of each element in buffer.
 *  @param len number of elements in buffer.
 *
 *  @remark does not unbind buffer when done, @ref glBindBuffer() must be used
 *  to bind different buffer.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_vbo_init(fsl_vbo *vbo, fsl_size size, fsl_len len, void *data,
        GLenum target, fsl_draw_type draw_type);

void fsl_vbo_free(fsl_vbo *vbo);

/*!
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_fbo_init(fsl_fbo *fbo, i32 size_x, i32 size_y, fsl_mesh *mesh_fbo,
        b8 multisample, u32 samples);

/*!
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_fbo_realloc(fsl_fbo *fbo, i32 size_x, i32 size_y, b8 multisample, u32 samples);

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

/*!
 *  @brief update camera position and rotation (in degrees).
 *
 *  @remark rotation limits:
 *      roll:  [  0, 360].
 *      pitch: [-90,  90].
 *      yaw:   [  0, 360].
 */
FSLAPI void fsl_camera_movement_update(fsl_camera *camera,
        f64 pos_x, f64 pos_y, f64 pos_z, f64 roll, f64 pitch, f64 yaw);

/*!
 *  @brief update camera lookat angles looking from 'pos' at 'target'.
 *
 *  @remark rotation limits:
 *      roll:  [  0, 360].
 *      pitch: [-90,  90].
 *      yaw:   [  0, 360].
 */
FSLAPI void fsl_camera_lookat_update(fsl_camera *camera,
        f64 pos_x, f64 pos_y, f64 pos_z, f64 target_x, f64 target_y, f64 target_z);

/*!
 *  @brief make perspective projection matrices from camera parameters.
 *
 *  - setup camera matrices for Z-up, right-handed coordinates and vertical fov (fovy):
 *      - +X: forward.
 *      - +Y: left.
 *      - +Z: up.
 *
 *  @remark called automatically from @ref fsl_camera_movement_update().
 *
 *  @param roll enable/disable roll rotation.
 */
FSLAPI void fsl_projection_perspective_update(fsl_camera camera, fsl_projection *projection, b8 roll);

#endif /* FSL_ASSETS_H */
