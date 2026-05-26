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

typedef enum fsl_mesh_format
{
    FSL_MESH_FORMAT_NONE,
    FSL_MESH_FORMAT_OBJ,
    FSL_MESH_FORMAT_FBX,
    FSL_MESH_FORMAT_GLTF,
    FSL_MESH_FORMAT_GLB,
    FSL_MESH_FORMAT_COUNT
} fsl_mesh_format;

/*!
 *  @internal
 *
 *  @brief get mesh file format based on file name conventions.
 *
 *  examples:
 *      - file name "player.obj" -> mesh format @ref FSL_MESH_FORMAT_OBJ.
 *      - file name "player.glb" -> mesh format @ref FSL_MESH_FORMAT_GLB.
 *
 *  @remark case insensitive.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_get_format_internal(const str *file, fsl_mesh_format *format);

/*!
 *  @internal
 *
 *  @brief load a 'Wavefront Obj' (.obj) mesh from disk into specified buffers.
 *
 *  @remark buffers must be `NULL`, will be allocated via @ref fsl_mem_alloc()
 *  and should be copied and freed using @ref fsl_mem_free().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_load_obj_internal(fsl_fs_path *path, fsl_array *vertex_dst, fsl_array *index_dst);

/*!
 *  @internal
 *
 *  @brief load an 'Autodesk FBX' (.fbx) mesh from disk into specified buffers.
 *
 *  @remark buffers must be `NULL`, will be allocated via @ref fsl_mem_alloc()
 *  and should be copied and freed using @ref fsl_mem_free().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_load_fbx_internal(fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf);

/*!
 *  @internal
 *
 *  @brief load a 'Khronos GLTF' (.gltf) mesh from disk into specified buffers.
 *
 *  @remark buffers must be `NULL`, will be allocated via @ref fsl_mem_alloc()
 *  and should be copied and freed using @ref fsl_mem_free().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_load_gltf_internal(fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf);

/*!
 *  @internal
 *
 *  @brief load a 'Khronos GLB' (.glb) mesh from disk into specified buffers.
 *
 *  @remark buffers must be `NULL`, will be allocated via @ref fsl_mem_alloc()
 *  and should be copied and freed using @ref fsl_mem_free().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_load_glb_internal(fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf);

#endif /* FSL_MESH_LOADER_INTERNAL_H */
