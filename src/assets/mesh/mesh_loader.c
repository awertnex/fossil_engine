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

u32 fsl_mesh_get_format_internal(const str *file, fsl_mesh_format *format)
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

u32 fsl_mesh_load_obj_internal(fsl_mesh *mesh, fsl_fs_path *path)
{
    GLfloat *vbo_data_p = NULL;
    GLuint *ebo_data_p = NULL;
    FILE *file = NULL;
    str line[FSL_STRING_MAX] = {0};
    str line_copy[FSL_STRING_MAX] = {0};
    str *line_curr = NULL;
    str *token = NULL;
    str *token_save[2] = {0};
    str delim[] = " ";
    i32 i = 0;

    v4f32 vertex = {0};
    u64 vertex_count = 0;
    u64 vertex_cap = sizeof(v3f32);
    v3f32 *vertex_buf = NULL;

    i64 index = 0;
    u64 index_count = 0;
    u64 index_cap = sizeof(u64);
    u64 *index_buf = NULL;

    v3f32 normal = {0};
    u64 normal_count = 0;
    u64 normal_cap = sizeof(v3f32);
    v3f32 *normal_buf = NULL;

    v2f32 tex_coord = {0};
    u64 tex_coord_count = 0;
    u64 tex_coord_cap = sizeof(v2f32);
    v2f32 *tex_coord_buf = NULL;

    b8 has_vertex = FALSE;
    b8 has_index = FALSE;
    b8 has_normal = FALSE;
    b8 has_tex_coord = FALSE;
    u64 primitive_curr = 0;

    if (
            fsl_mem_alloc((void*)&vertex_buf, vertex_cap,
                "fsl_mesh_load_obj_internal().vertex_buf") != FSL_ERR_SUCCESS ||

            fsl_mem_alloc((void*)&index_buf, index_cap,
                "fsl_mesh_load_obj_internal().index_buf") != FSL_ERR_SUCCESS ||

            fsl_mem_alloc((void*)&normal_buf, normal_cap,
                "fsl_mesh_load_obj_internal().normal_buf") != FSL_ERR_SUCCESS ||

            fsl_mem_alloc((void*)&tex_coord_buf, tex_coord_cap,
                "fsl_mesh_load_obj_internal().tex_coord_buf") != FSL_ERR_SUCCESS)
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
        line_curr = line;
        fsl_skip_spaces(&line_curr);
        if (line_curr[0] == '#' || line_curr[0] == '\n')
            continue;

        snprintf(line_copy, FSL_STRING_MAX, "%s", line_curr);
        token = strtok_r(line_curr, delim, &token_save[0]);

        if (!strncmp(token, "v\0", 2))
        {
            has_vertex = TRUE;

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.x, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.y, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &vertex.z, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            if (token)
                fsl_convert_str_to_f32(token, &vertex.w, FSL_ID_CAP);
            else
                vertex.w = 1.0;

            if (vertex.w != 0.0)
            {
                vertex.x /= vertex.w;
                vertex.y /= vertex.w;
                vertex.z /= vertex.w;
            }

            vertex_buf[vertex_count].x = vertex.x;
            vertex_buf[vertex_count].y = vertex.y;
            vertex_buf[vertex_count].z = vertex.z;
            ++vertex_count;
            if (vertex_count * sizeof(v3f32) >= vertex_cap)
            {
                if (fsl_mem_realloc((void*)&vertex_buf, vertex_cap * 2,
                            "fsl_mesh_load_obj_internal().vertex_buf") != FSL_ERR_SUCCESS)
                    goto cleanup;
                vertex_cap *= 2;
            }
            continue;
        }

        if (!strncmp(token, "vn\0", 3))
        {
            has_normal = TRUE;

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &normal.x, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &normal.y, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &normal.z, FSL_ID_CAP);

            normal_buf[normal_count].x = normal.x;
            normal_buf[normal_count].y = normal.y;
            normal_buf[normal_count].z = normal.z;
            ++normal_count;
            if (normal_count * sizeof(v3f32) >= normal_cap)
            {
                if (fsl_mem_realloc((void*)&normal_buf, normal_cap * 2,
                            "fsl_mesh_load_obj_internal().normal_buf") != FSL_ERR_SUCCESS)
                    goto cleanup;
                normal_cap *= 2;
            }
            continue;
        }

        if (!strncmp(token, "vt\0", 3))
        {
            has_tex_coord = TRUE;

            token = strtok_r(NULL, delim, &token_save[0]);
            fsl_convert_str_to_f32(token, &tex_coord.x, FSL_ID_CAP);

            token = strtok_r(NULL, delim, &token_save[0]);
            if (token)
                fsl_convert_str_to_f32(token, &tex_coord.y, FSL_ID_CAP);
            else
                tex_coord.y = 0.0;

            tex_coord_buf[tex_coord_count].x = tex_coord.x;
            tex_coord_buf[tex_coord_count].y = tex_coord.y;
            ++tex_coord_count;
            if (tex_coord_count * sizeof(v2f32) >= tex_coord_cap)
            {
                if (fsl_mem_realloc((void*)&tex_coord_buf, tex_coord_cap * 2,
                            "fsl_mesh_load_obj_internal().tex_coord_buf") != FSL_ERR_SUCCESS)
                    goto cleanup;
                tex_coord_cap *= 2;
            }
            continue;
        }

        if (!strncmp(token, "f\0", 2))
        {
            has_index = TRUE;

            while ((token = strtok_r(NULL, delim, &token_save[0])) != NULL)
            {
                strtok_r(token, "/", &token_save[1]);
                fsl_convert_str_to_i64(token, &index, FSL_ID_CAP);

start_next_face:
                index_buf[index_count] = index;
                ++index_count;
                if (index_count * sizeof(v3f32) >= index_cap)
                {
                    if (fsl_mem_realloc((void*)&index_buf, index_cap * 2,
                                "fsl_mesh_load_obj_internal().index_buf") != FSL_ERR_SUCCESS)
                        goto cleanup;
                    index_cap *= 2;
                }
                ++i;
                if (i % 3 == 0)
                    goto start_next_face;
            }

            if ((i + 1) % 3 == 0)
            {
                snprintf(line_curr, FSL_STRING_MAX, "%s", line_copy);
                token = strtok_r(line_curr, delim, &token_save[0]);
                token = strtok_r(NULL, delim, &token_save[0]);
                token = strtok_r(token, "/", &token_save[1]);
                fsl_convert_str_to_i64(token, &index, FSL_ID_CAP);

                index_buf[index_count] = index;
                ++index_count;
                if (index_count * sizeof(v3f32) >= index_cap)
                {
                    if (fsl_mem_realloc((void*)&index_buf, index_cap * 2,
                                "fsl_mesh_load_obj_internal().index_buf") != FSL_ERR_SUCCESS)
                        goto cleanup;
                    index_cap *= 2;
                }
            }

            i = 0;
            continue;
        }
    }

    fclose(file);
    file = NULL;

    if (has_vertex)
    {
        if (fsl_mem_arena_push(&mem_arena_internal, &mesh->vbo_data, vertex_count * sizeof(v3f32),
                    "fsl_mesh_load_obj_internal().mesh->vbo_data") != FSL_ERR_SUCCESS)
            goto cleanup;

        vbo_data_p = fsl_mem_handle_get(mesh->vbo_data);
        memcpy(vbo_data_p, vertex_buf, vertex_count * sizeof(v3f32));
        mesh->vbo_len = vertex_count * 3;
    }

    if (has_index)
    {
        if (fsl_mem_arena_push(&mem_arena_internal, &mesh->ebo_data, index_count * sizeof(GLuint),
                    "fsl_mesh_load_obj_internal().mesh->vbo_data") != FSL_ERR_SUCCESS)
            goto cleanup;

        ebo_data_p = fsl_mem_handle_get(mesh->ebo_data);
        memcpy(ebo_data_p, index_buf, index_count * sizeof(GLuint));
        mesh->ebo_len = index_count;
    }

    fsl_mem_free((void*)&vertex_buf, vertex_cap, "fsl_mesh_load_obj_internal().vertex_buf");
    fsl_mem_free((void*)&index_buf, index_cap, "fsl_mesh_load_obj_internal().index_buf");
    fsl_mem_free((void*)&normal_buf, normal_cap, "fsl_mesh_load_obj_internal().normal_buf");
    fsl_mem_free((void*)&tex_coord_buf, tex_coord_cap, "fsl_mesh_load_obj_internal().tex_coord_buf");

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    if (file)
        fclose(file);
    fsl_mem_free((void*)&vertex_buf, vertex_cap, "fsl_mesh_load_obj_internal().vertex_buf");
    fsl_mem_free((void*)&index_buf, index_cap, "fsl_mesh_load_obj_internal().index_buf");
    fsl_mem_free((void*)&normal_buf, normal_cap, "fsl_mesh_load_obj_internal().normal_buf");
    fsl_mem_free((void*)&tex_coord_buf, tex_coord_cap, "fsl_mesh_load_obj_internal().tex_coord_buf");
    return fsl_err;
}

u32 fsl_mesh_load_fbx_internal(fsl_mesh *mesh, fsl_fs_path *path)
{
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

/* TODO: make function `fsl_mesh_load_gltf_internal()` */
u32 fsl_mesh_load_gltf_internal(fsl_mesh *mesh, fsl_fs_path *path)
{
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

/* TODO: make function `fsl_mesh_load_glb_internal()` */
u32 fsl_mesh_load_glb_internal(fsl_mesh *mesh, fsl_fs_path *path)
{
    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}
