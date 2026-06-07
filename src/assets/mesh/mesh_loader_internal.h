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
#include "../../math/vector.h"

#define MESH_FILE_ID "fsl_mesh"

typedef struct fsl_file_format_fmesh fsl_file_format_fmesh;

typedef enum fsl_mesh_format
{
    FSL_MESH_FORMAT_NONE,
    FSL_MESH_FORMAT_FMESH, /* engine's binary mesh format */
    FSL_MESH_FORMAT_OBJ,
    FSL_MESH_FORMAT_GLTF,
    FSL_MESH_FORMAT_GLB,
    FSL_MESH_FORMAT_COUNT
} fsl_mesh_format;

struct fsl_file_format_fmesh
{
    u64 asset_type;
    str id[8]; /* @ref MESH_FILE_ID */

    fsl_size vertex_size;
    fsl_len vertex_len;
    fsl_size index_size;
    fsl_len index_len;
    u64 hash; /* hash of `vertex_size`, `vertex_len`, `index_size` and `index_len`, in that order */

    u8* vertex_data;
    u8* index_data;
}; /* fsl_file_format_fmesh */

struct mesh_vertex
{
    v3f32 pos;
    v3f32 normal;
    v2f32 uv;
}; /* mesh_vertex */

/*!
 *  @internal
 *
 *  @brief check whether a cooked version of the mesh file has been found or not.
 *
 *  @return FALSE on failure.
 */
b8 mesh_is_cooked(const fsl_fs_path *path);

/*!
 *  @internal
 *
 *  @brief load a 'Fossil Mesh' (.fmesh) file from disk into specified buffers.
 *
 *  @remark buffers must be `NULL`, will be allocated via @ref fsl_mem_array_init()
 *  and should be copied and freed using @ref fsl_mem_array_free().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_load_fmesh_internal(const fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf);

/*!
 *  @internal
 *
 *  @brief export a 'Fossil Mesh' (.fmesh) mesh from RAM into `path`.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_export_fmesh_internal(const fsl_fs_path *path, fsl_array vertex_buf, fsl_array index_buf);

/*!
 *  @internal
 *
 *  @brief load a 'Wavefront Obj' (.obj) mesh from disk into specified buffers.
 *
 *  @remark buffers must be `NULL`, will be allocated via @ref fsl_mem_array_init()
 *  and should be copied and freed using @ref fsl_mem_array_free().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_load_obj_internal(const fsl_fs_path *path, fsl_array *vertex_dst, fsl_array *index_dst);

/*!
 *  @internal
 *
 *  @brief load a 'Khronos GLTF' (.gltf) mesh from disk into specified buffers.
 *
 *  @remark buffers must be `NULL`, will be allocated via @ref fsl_mem_array_init()
 *  and should be copied and freed using @ref fsl_mem_array_free().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_load_gltf_internal(const fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf);

/*!
 *  @internal
 *
 *  @brief load a 'Khronos GLB' (.glb) mesh from disk into specified buffers.
 *
 *  @remark buffers must be `NULL`, will be allocated via @ref fsl_mem_array_init()
 *  and should be copied and freed using @ref fsl_mem_array_free().
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
u32 mesh_load_glb_internal(const fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf);

#endif /* FSL_MESH_LOADER_INTERNAL_H */
