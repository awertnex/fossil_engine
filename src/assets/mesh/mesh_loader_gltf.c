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
 *  @file mesh_loader_gltf.c
 *
 *  @brief Khronos glTF/glB (.gltf/.glb) mesh file parsing and loading.
 */

#include "../../common/diagnostics.h"
#include "../../common/types.h"

#include "mesh_loader_internal.h"

/* TODO: make function `fsl_mesh_load_gltf_internal()` */
u32 mesh_load_gltf_internal(const fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf)
{
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

/* TODO: make function `fsl_mesh_load_glb_internal()` */
u32 mesh_load_glb_internal(const fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf)
{
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}
