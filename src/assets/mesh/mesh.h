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
 *  @file mesh.h
 *
 *  @brief mesh loading, generating, unloading and mesh file formats.
 */

#ifndef FSL_MESH_H
#define FSL_MESH_H

#include "../../common/api.h"
#include "../../common/types.h"
#include "../../memory/memory_types.h"

#include "../asset_types.h"

#include "../../external/glad/glad.h"

typedef struct fsl_mesh fsl_mesh;

struct fsl_mesh
{
    fsl_asset asset;
    GLuint vao;

    /*!
     *  @brief mesh vertex data.
     */
    fsl_vbo vertex_buf;

    /*!
     *  @brief mesh indices of vertex data.
     */
    fsl_ebo index_buf;

    /*!
     *  @brief model transform data (location, rotation and scale).
     */
    fsl_vbo transform_buf;
}; /* fsl_mesh */

/*!
 *  @brief load mesh from file into VRAM.
 *
 *  additionally, convert file into a cooked asset for easier loading next time,
 *  in 'Fossil Mesh' (.fmesh) format.
 *
 *  - if cooked version of file found, it will be loaded regardless of file
 *    extension specified, even if original file not found.
 *
 *  @param mesh pointer to `fsl_mesh` to store mesh data into.
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
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mesh_load(fsl_mesh *mesh,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path);

/*!
 *  @brief draw mesh through the lens of `camera` at specified transforms.
 */
FSLAPI void fsl_mesh_draw(const fsl_mesh *mesh, const fsl_camera *camera,
        GLuint texture_id,
        f32 pos_x, f32 pos_y, f32 pos_z,
        f32 roll, f32 pitch, f32 yaw,
        f32 scale_x, f32 scale_y, f32 scale_z);

/*!
 *  @param attrib pointer to a function to set attribute arrays for `mesh->vao`
 *  (e.g., &attrib_vec3, set a single vec3 attribute array).
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
 *  @param usage `GL_<x>_DRAW`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
FSLAPI u32 fsl_mesh_generate(fsl_mesh *mesh,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path,
        void (*attrib)(void), GLenum usage,
        GLuint vertex_buf_len, GLuint index_buf_len, GLfloat *vertex_data, GLuint *index_data);

FSLAPI void fsl_mesh_free(fsl_mesh *mesh);

#endif /* FSL_MESH_H */
