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
 *  @file mesh_loader_internal.h
 *
 *  @brief mesh file parsing and loading.
 */

#ifndef FSL_MESH_LOADER_INTERNAL_H
#define FSL_MESH_LOADER_INTERNAL_H

#include "../../common/types.h"
#include "mesh.h"

typedef struct fsl_mesh_vertex fsl_mesh_vertex;

typedef enum fsl_mesh_format
{
    FSL_MESH_FORMAT_NONE,
    FSL_MESH_FORMAT_OBJ,
    FSL_MESH_FORMAT_FBX,
    FSL_MESH_FORMAT_GLTF,
    FSL_MESH_FORMAT_GLB,
    FSL_MESH_FORMAT_COUNT
} fsl_mesh_format;

struct fsl_mesh_vertex
{
    v3f32 pos;
    v3f32 normal;
    v2f32 uv;
}; /* fsl_mesh_vertex */

/*!
 *  @internal
 *
 *  @brief get mesh type based on file name conventions.
 *
 *  examples:
 *      - file name "player.obj" -> mesh format "FSL_MESH_FORMAT_OBJ".
 *      - file name "player.glb" -> mesh format "FSL_MESH_FORMAT_GLB".
 *
 *  @remark case insensitive.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 fsl_mesh_get_format_internal(const str *file, fsl_mesh_format *format);

/*!
 *  @internal
 *
 *  @brief load a 'Wavefront Obj' (.obj) mesh from disk into `mesh`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 fsl_mesh_load_obj_internal(fsl_mesh *mesh, fsl_fs_path *path);

/*!
 *  @internal
 *
 *  @brief load an 'Autodesk FBX' (.fbx) mesh from disk into `mesh`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 fsl_mesh_load_fbx_internal(fsl_mesh *mesh, fsl_fs_path *path);

/*!
 *  @internal
 *
 *  @brief load a 'Khronos GLTF' (.gltf) mesh from disk into `mesh`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 fsl_mesh_load_gltf_internal(fsl_mesh *mesh, fsl_fs_path *path);

/*!
 *  @internal
 *
 *  @brief load a 'Khronos GLB' (.glb) mesh from disk into `mesh`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 fsl_mesh_load_glb_internal(fsl_mesh *mesh, fsl_fs_path *path);

#endif /* FSL_MESH_LOADER_INTERNAL_H */
