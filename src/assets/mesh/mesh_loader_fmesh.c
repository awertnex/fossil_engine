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
 *  @file mesh_loader_fmesh.c
 *
 *  @brief engine's mesh format 'Fossil Mesh' (.fmesh) file parsing and loading.
 */

#include "../../common/config.h"
#include "../../common/diagnostics.h"
#include "../../common/limits.h"
#include "../../common/types.h"
#include "../../logger/logger.h"
#include "../../logger/logger_messages_internal.h"
#include "../../memory/memory.h"
#include "../../string/string.h"

#include "../../h/dir.h"

#include "../asset_types.h"

#include "mesh_loader_internal.h"

#include <stdio.h>
#include <string.h>

b8 mesh_is_cooked(const fsl_fs_path *path)
{
    fsl_fs_path path_temp[FSL_PATH_CAP] = {0};
    str *extension = NULL;

    snprintf(path_temp, FSL_PATH_CAP, "%s", path);
    extension = strrchr(path_temp, '.');
    if (extension)
    {
        ++extension;
        snprintf(extension, strlen(FSL_FILE_FORMAT_NAME_FOSSIL_MESH) + 1, "%s", FSL_FILE_FORMAT_NAME_FOSSIL_MESH);
    }

    if (fsl_is_file_exists(path_temp, FALSE) == FSL_ERR_SUCCESS)
        return TRUE;
    return FALSE;
}

u32 mesh_load_fmesh_internal(const fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf)
{
    FILE *file = NULL;
    fsl_file_format_fmesh file_data = {0};
    u64 cache[4] = {0};

    if ((file = fopen(path, "rb")) == NULL)
    {
        LOGERROR(FSL_ERR_FILE_OPEN_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_FILE_OPEN_FAIL(path));
        return fsl_err;
    }

    fread(&file_data.asset_type, 1, sizeof(u64), file);
    if (file_data.asset_type != FSL_ASSET_MESH)
    {
        LOGERROR(FSL_ERR_FILE_FORMAT_INVALID,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Load Mesh", path, "Wrong Asset Type"));
        goto cleanup;
    }

    fread(file_data.id, 1, strlen(MESH_FILE_ID), file);
    if (strncmp(file_data.id, MESH_FILE_ID, strlen(MESH_FILE_ID)))
    {
        LOGERROR(FSL_ERR_FILE_FORMAT_INVALID,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Load Mesh", path, "Format Invlid"));
        goto cleanup;
    }

    fread(cache, 1, 4 * sizeof(u64), file);
    fread(&file_data.hash, 1, sizeof(u64), file);

    if (file_data.hash != fsl_hash_fnv1a_u64(cache, 4 * sizeof(u64)))
    {
        LOGERROR(FSL_ERR_FILE_DATA_CORRUPT,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Load Mesh", path, "File Corrupted"));
        goto cleanup;
    }

    file_data.vertex_size = cache[0];
    file_data.vertex_len = cache[1];
    file_data.index_size = cache[2];
    file_data.index_len = cache[3];

    if (file_data.vertex_size && file_data.vertex_len &&
            fsl_mem_array_init(vertex_buf) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (file_data.index_size && file_data.index_len &&
            fsl_mem_array_init(index_buf) != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_mem_array_push(vertex_buf, NULL, file_data.vertex_size * file_data.vertex_len);
    fsl_mem_array_push(index_buf, NULL, file_data.index_size * file_data.index_len);

    fread(vertex_buf->buf, file_data.vertex_size, file_data.vertex_len, file);
    fread(index_buf->buf, file_data.index_size, file_data.index_len, file);

    fclose(file);
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    if (file)
        fclose(file);
    return fsl_err;
}

u32 mesh_export_fmesh_internal(const fsl_fs_path *path, fsl_array vertex_buf, fsl_array index_buf)
{
    fsl_fs_path path_temp[FSL_PATH_CAP] = {0};
    str *extension = NULL;
    FILE *file = NULL;
    u64 bytes_written = 0;
    fsl_file_format_fmesh file_data = {0};
    u64 cache[4] = {0};

    snprintf(path_temp, FSL_PATH_CAP, "%s", path);
    extension = strrchr(path_temp, '.');
    if (extension)
    {
        ++extension;
        snprintf(extension, strlen(FSL_FILE_FORMAT_NAME_FOSSIL_MESH) + 1, "%s", FSL_FILE_FORMAT_NAME_FOSSIL_MESH);
    }

    if ((file = fopen(path_temp, "wb")) == NULL)
    {
        LOGERROR(FSL_ERR_FILE_OPEN_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_FILE_OPEN_FAIL(path_temp));
        return fsl_err;
    }

    file_data.asset_type = FSL_ASSET_MESH;
    cache[0] = sizeof(struct mesh_vertex);
    cache[1] = vertex_buf.cursor / sizeof(struct mesh_vertex);
    cache[2] = sizeof(u64);
    cache[3] = index_buf.cursor / sizeof(u64);
    file_data.hash = fsl_hash_fnv1a_u64(cache, 4 * sizeof(u64));

    bytes_written += fwrite(&file_data.asset_type, 1, sizeof(u64), file);
    bytes_written += fwrite(MESH_FILE_ID, 1, strlen(MESH_FILE_ID), file);
    bytes_written += fwrite(cache, 1, 4 * sizeof(u64), file);
    bytes_written += fwrite(&file_data.hash, 1, sizeof(u64), file);
    bytes_written += fwrite(vertex_buf.buf, 1, vertex_buf.cursor, file);
    bytes_written += fwrite(index_buf.buf, 1, index_buf.cursor, file);

    LOGSUCCESS(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_MESH_EXPORT_FMESH(path_temp, bytes_written));
    fclose(file);
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

