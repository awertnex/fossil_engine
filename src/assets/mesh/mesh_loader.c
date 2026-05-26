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
 *  @file mesh_loader_internal.c
 *
 *  @brief mesh file parsing and loading.
 */

#include "../../common/diagnostics.h"
#include "../../common/limits.h"
#include "../../common/types.h"
#include "../../logger/logger.h"
#include "../../logger/logger_messages_internal.h"
#include "../../memory/memory.h"
#include "../../h/string.h"

#include "mesh.h"
#include "mesh_loader_internal.h"

#include "../../external/glad/glad.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

struct fsl_mesh_vertex
{
    v4f32 pos;
    v3f32 normal;
    v2f32 uv;
}; /* fsl_mesh_vertex */

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @internal
 *
 *  @brief extract vertex index data from obj string (e.g., "f 1/2/4", "f v1/vt2/vn4").
 *
 *  vertex index goes into `vertex->x`,
 *  uv index goes into `vertex->y` and
 *  normal index goes into `vertex->z`.
 */
static void mesh_obj_get_vertex_internal(const str *token, v3i64 *vertex);

/* ---- section: implementation --------------------------------------------- */

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
    if (!strncmp(extension, "fbx\0", 4))
    {
        *format = FSL_MESH_FORMAT_FBX;
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

u32 mesh_load_obj_internal(fsl_fs_path *path, fsl_array *vertex_dst, fsl_array *index_dst)
{
    FILE *file = NULL;
    str line[FSL_STRING_MAX] = {0};
    str *line_p = NULL;
    str *token = NULL;
    str *token_save[2] = {0};
    str delim[] = " ";
    i32 i = 0;

    u64 vertex_dst_alloc_size = 0;
    v3i64 vertex_indices_first = {0};
    v3i64 vertex_indices = {0};
    struct fsl_mesh_vertex vertex = {0};
    fsl_array vertex_buf = {0};
    fsl_array normal_buf = {0};
    fsl_array uv_buf = {0};

    b8 has_vertex = FALSE;
    b8 has_index = FALSE;
    b8 has_normal = FALSE;
    b8 has_uv = FALSE;

    if (
            fsl_mem_array_init(&vertex_buf) != FSL_ERR_SUCCESS ||
            fsl_mem_array_init(&normal_buf) != FSL_ERR_SUCCESS ||
            fsl_mem_array_init(&uv_buf) != FSL_ERR_SUCCESS ||
            fsl_mem_array_init(vertex_dst) != FSL_ERR_SUCCESS)
        goto cleanup;

    if ((file = fopen(path, "rb")) == NULL)
    {
        LOGERROR(FSL_ERR_FILE_OPEN_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_FILE_OPEN_FAIL(path));
        return fsl_err;
    }

    while (fgets(line, FSL_STRING_MAX, file) != NULL)
    {
        line_p = line;
        fsl_skip_spaces(&line_p);
        if (line_p[0] == '#' || line_p[0] == '\n')
            continue;

        token = strtok_r(line_p, delim, &token_save[0]);

        if (!strncmp(token, "v\0", 2))
        {
            has_vertex = TRUE;

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.pos.x, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.pos.y, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.pos.z, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            if (token)
                fsl_convert_str_to_f32(token, &vertex.pos.w, FSL_ID_CAP);
            else
                vertex.pos.w = 1.0;

            if (vertex.pos.w != 0.0)
            {
                vertex.pos.x /= vertex.pos.w;
                vertex.pos.y /= vertex.pos.w;
                vertex.pos.z /= vertex.pos.w;
            }

            fsl_mem_array_push(&vertex_buf, &vertex.pos, sizeof(v3f32));
            continue;
        }

        if (!strncmp(token, "vn\0", 3))
        {
            has_normal = TRUE;

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.normal.x, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.normal.y, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.normal.z, FSL_ID_CAP);

            fsl_mem_array_push(&normal_buf, &vertex.normal, sizeof(v3f32));
            continue;
        }

        if (!strncmp(token, "vt\0", 3))
        {
            has_uv = TRUE;

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.uv.x, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            if (token)
                fsl_convert_str_to_f32(token, &vertex.uv.y, FSL_ID_CAP);
            else
                vertex.uv.y = 0.0;

            fsl_mem_array_push(&uv_buf, &vertex.uv, sizeof(v2f32));
            continue;
        }

        if (!strncmp(token, "f\0", 2))
        {
            if (!has_index)
            {
                if (fsl_mem_array_init(index_dst) != FSL_ERR_SUCCESS)
                    goto cleanup;
                has_index = TRUE;
            }

            if ((token = strtok_r(NULL, delim, &token_save[0])) == NULL)
                continue;

            /* get first vertex */
            mesh_obj_get_vertex_internal(token, &vertex_indices_first);
            fsl_mem_array_push(index_dst, &vertex_indices_first.x, sizeof(u64));
            fsl_mem_array_push(vertex_dst, &*((u8*)vertex_buf.buf +
                        vertex_indices_first.x * sizeof(v3f32)), sizeof(v3f32));
            fsl_mem_array_push(vertex_dst, &*((u8*)normal_buf.buf +
                        vertex_indices_first.z * sizeof(v3f32)), sizeof(v3f32));
            fsl_mem_array_push(vertex_dst, &*((u8*)uv_buf.buf +
                        vertex_indices_first.y * sizeof(v2f32)), sizeof(v2f32));

            while ((token = strtok_r(NULL, delim, &token_save[0])) != NULL)
            {
                mesh_obj_get_vertex_internal(token, &vertex_indices);
                fsl_mem_array_push(index_dst, &vertex_indices.x, sizeof(u64));
                fsl_mem_array_push(vertex_dst, &*((u8*)vertex_buf.buf +
                            vertex_indices_first.x * sizeof(v3f32)), sizeof(v3f32));
                fsl_mem_array_push(vertex_dst, &*((u8*)normal_buf.buf +
                            vertex_indices_first.z * sizeof(v3f32)), sizeof(v3f32));
                fsl_mem_array_push(vertex_dst, &*((u8*)uv_buf.buf +
                            vertex_indices_first.y * sizeof(v2f32)), sizeof(v2f32));
                ++i;

                /* use first vertex to triangulate face */
                if (i % 3 == 0)
                {
                    fsl_mem_array_push(index_dst, &vertex_indices_first.x, sizeof(u64));
                    fsl_mem_array_push(vertex_dst, &*((u8*)vertex_buf.buf +
                                vertex_indices_first.x * sizeof(v3f32)), sizeof(v3f32));
                    fsl_mem_array_push(vertex_dst, &*((u8*)normal_buf.buf +
                                vertex_indices_first.z * sizeof(v3f32)), sizeof(v3f32));
                    fsl_mem_array_push(vertex_dst, &*((u8*)uv_buf.buf +
                                vertex_indices_first.y * sizeof(v2f32)), sizeof(v2f32));
                    ++i;
                }
            }

            /* triangulate final face */
            if ((i + 1) % 3 == 0)
            {
                fsl_mem_array_push(index_dst, &vertex_indices_first.x, sizeof(u64));
                fsl_mem_array_push(vertex_dst, &*((u8*)vertex_buf.buf +
                            vertex_indices_first.x * sizeof(v3f32)), sizeof(v3f32));
                fsl_mem_array_push(vertex_dst, &*((u8*)normal_buf.buf +
                        vertex_indices_first.z * sizeof(v3f32)), sizeof(v3f32));
                fsl_mem_array_push(vertex_dst, &*((u8*)uv_buf.buf +
                        vertex_indices_first.y * sizeof(v2f32)), sizeof(v2f32));
            }

            i = 0;
            continue;
        }
    }

    fclose(file);
    file = NULL;

    fsl_mem_array_free(&vertex_buf);
    fsl_mem_array_free(&normal_buf);
    fsl_mem_array_free(&uv_buf);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    if (file)
        fclose(file);
    fsl_mem_array_free(vertex_dst);
    fsl_mem_array_free(index_dst);
    fsl_mem_array_free(&vertex_buf);
    fsl_mem_array_free(&normal_buf);
    fsl_mem_array_free(&uv_buf);
    return fsl_err;
}

static void mesh_obj_get_vertex_internal(const str *token, v3i64 *vertex)
{
    str *p = token;
    str temp[FSL_ID_CAP] = {0};
    u32 i = 0;

    /* vertex index */
    while(p++)
    {
        if (fsl_is_digit(*p))
        {
            for (i = 0; p[i] && p[i] != '/'; ++i)
                temp[i] = p[i];
            temp[i] = 0;
            fsl_convert_str_to_i64(temp, &vertex->x, FSL_ID_CAP);
            --vertex->x;
            p += i;
            break;
        }
        else
            vertex->x = 0;
    }

    /* vertex uv */
    while(p++)
    {
        if (fsl_is_digit(*p))
        {
            for (i = 0; p[i] && p[i] != '/'; ++i)
                temp[i] = p[i];
            temp[i] = 0;
            fsl_convert_str_to_i64(temp, &vertex->y, FSL_ID_CAP);
            --vertex->y;
            p += i;
            break;
        }
        else
            vertex->y = 0;
    }

    /* vertex normal */
    while(p++)
    {
        if (fsl_is_digit(*p))
        {
            for (i = 0; p[i] && p[i] != '/'; ++i)
                temp[i] = p[i];
            temp[i] = 0;
            fsl_convert_str_to_i64(temp, &vertex->z, FSL_ID_CAP);
            --vertex->z;
            p += i;
            break;
        }
        else
            vertex->z = 0;
    }
}

u32 mesh_load_fbx_internal(fsl_fs_path *path, fsl_array *vertex_buf, fsl_array *index_buf)
{
    fsl_err = FSL_ERR_SUCCESS;
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
