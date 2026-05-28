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
 *  @file mesh_loader.c
 *
 *  @brief mesh file parsing and loading.
 */

#include "../../common/diagnostics.h"
#include "../../common/limits.h"
#include "../../common/types.h"
#include "../../logger/logger.h"
#include "../../logger/logger_messages_internal.h"
#include "../../memory/memory.h"
#include "../../string/string.h"

#include "../../h/math.h"

#include "mesh.h"
#include "mesh_loader_internal.h"

#include "../../external/glad/glad.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

u32 mesh_get_format_internal(const str *file, fsl_mesh_format *format)
{
    str base_name[FSL_ID_CAP] = {0};
    str *extension = {0};
    u64 i = 0;
    u64 len = 0;

    if (!file || !format)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Get Mesh Format", "Pointer `NULL`"));
        return fsl_err;
    }

    snprintf(base_name, FSL_ID_CAP, "%s", file);
    extension = strrchr(base_name, '.');
    if (extension)
    {
        ++extension;
        len = strnlen(extension, FSL_ID_CAP - (extension - base_name));
        for (i = 0; i < len; ++i)
            extension[i] = tolower(extension[i]);
    }
    else
    {
        LOGERROR(FSL_ERR_MESH_FORMAT_UNKNOWN, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Get Mesh Format", "Format Unknown"));
        return fsl_err;
    }

    if (!strncmp(extension, "obj\0", 4))
    {
        *format = FSL_MESH_FORMAT_OBJ;
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }
    if (!strncmp(extension, "gltf\0", 5))
    {
        *format = FSL_MESH_FORMAT_GLTF;
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }
    if (!strncmp(extension, "glb\0", 4))
    {
        *format = FSL_MESH_FORMAT_GLB;
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }

    LOGERROR(FSL_ERR_MESH_FORMAT_UNKNOWN, FSL_FLAG_LOG_NO_VERBOSE,
            MSG_ACTION_SUBJECT_REASON_ERROR("Get Mesh Format", base_name, "Format Detection Failed"));
    return fsl_err;
}

/* TODO: make function `fsl_mesh_load_gltf_internal()` */
u32 mesh_load_gltf_internal(fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf)
{
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

/* TODO: make function `fsl_mesh_load_glb_internal()` */
u32 mesh_load_glb_internal(fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf)
{
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}
