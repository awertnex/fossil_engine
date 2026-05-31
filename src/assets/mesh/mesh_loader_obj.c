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
 *  @file mesh_loader_obj.c
 *
 *  @brief Wavefront OBJ (.obj) mesh file parsing and loading.
 */

#include "../../common/diagnostics.h"
#include "../../common/limits.h"
#include "../../common/types.h"
#include "../../logger/logger.h"
#include "../../logger/logger_messages_internal.h"
#include "../../math/math.h"
#include "../../math/vector.h"
#include "../../memory/memory.h"
#include "../../string/string.h"

#include "mesh_loader_internal.h"

#include "../../external/glad/glad.h"

#include <stdio.h>
#include <string.h>

#define FSL_HASH_TABLE_MAX 8192

typedef struct fsl_hash_node fsl_hash_node;

struct fsl_hash_node
{
    u64 hash;
    u64 index;
    fsl_hash_node *next;
}; /* fsl_hash_node */

struct vertex_indices
{
    u32 pos;
    u32 normal;
    u32 uv;
}; /* vertex_indices */

struct triangle_indices
{
    u32 first;
    u32 curr;
    u32 prev;
}; /* triangle_indices */

/*!
 *  @internal
 *
 *  @brief extract vertex index data from face vertex string (e.g., "f 1/2/4", "f v1/vt2/vn4").
 *
 *  supports negative indices, unspecified indices, tagged indices (e.g., "f v1/vt1/vn1")
 *  and space-separated indices (e.g., "f 1 2 3").
 *
 *  @param pos_len number of elements in the raw vertex positions buffer.
 *  @param uv_len number of elements in the raw vertex UVs buffer.
 *  @param normal_len number of elements in the raw vertex normals buffer.
 *
 *  @remark result indices are 1-based so to separate 0 (first index) from 0 (no index).
 */
static void get_vertex_indices_internal(str *token, struct vertex_indices *vertex,
        fsl_len pos_len, fsl_len uv_len, fsl_len normal_len);

u32 mesh_load_obj_internal(const fsl_fs_path *path, fsl_array *vertex_dst, fsl_array *index_dst)
{
    FILE *file = NULL;
    str line[FSL_STRING_MAX] = {0};
    str *line_p = NULL;
    str *token = NULL;
    str *token_save[1] = {0};
    str delim[] = " ";
    u32 i = 0;

    struct mesh_vertex vertex = {0};
    struct mesh_vertex novertex = {0}; /* to zero-out `vertex` */
    struct vertex_indices vertex_indices = {0};
    struct triangle_indices triangle = {0};
    f32 vertex_w = 0.0f;
    u64 vertex_hash = 0;
    u64 vertex_hash_index = 0;

    fsl_array vertex_buf = {0};
    fsl_array uv_buf = {0};
    fsl_array normal_buf = {0};

    fsl_hash_node *node_curr = NULL;
    fsl_hash_node *node_new = NULL;
    fsl_hash_node *hash_table = NULL;
    u64 bucket_index = 0;
    b8 hash_found = FALSE;

    if (
            fsl_mem_map((void*)&hash_table, FSL_HASH_TABLE_MAX * sizeof(fsl_hash_node),
                "mesh_load_obj_internal().hash_table") != FSL_ERR_SUCCESS ||
            fsl_mem_array_init(&vertex_buf) != FSL_ERR_SUCCESS ||
            fsl_mem_array_init(&uv_buf) != FSL_ERR_SUCCESS ||
            fsl_mem_array_init(&normal_buf) != FSL_ERR_SUCCESS ||
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
        fsl_strip_non_printable(line);
        fsl_skip_spaces(&line_p);
        if (!line_p[0] || line_p[0] == '#')
            continue;

        token = strtok_r(line_p, delim, &token_save[0]);

        if (token[0] == 'v')
        {
            if (!token[1])
            {
                token = strtok_r(NULL, delim, &token_save[0]);
                fsl_convert_str_to_f32(token, &vertex.pos.x);

                token = strtok_r(NULL, delim, &token_save[0]);
                fsl_convert_str_to_f32(token, &vertex.pos.y);

                token = strtok_r(NULL, delim, &token_save[0]);
                fsl_convert_str_to_f32(token, &vertex.pos.z);

                token = strtok_r(NULL, delim, &token_save[0]);
                if (token)
                    fsl_convert_str_to_f32(token, &vertex_w);
                else
                    vertex_w = 1.0;

                if (vertex_w != 0.0)
                {
                    vertex.pos.x /= vertex_w;
                    vertex.pos.y /= vertex_w;
                    vertex.pos.z /= vertex_w;
                }

                fsl_mem_array_push(&vertex_buf, &vertex.pos, sizeof(v3f32));
            }
            else if (token[1] == 't' && !token[2])
            {
                token = strtok_r(NULL, delim, &token_save[0]);
                fsl_convert_str_to_f32(token, &vertex.uv.x);

                token = strtok_r(NULL, delim, &token_save[0]);
                if (token)
                    fsl_convert_str_to_f32(token, &vertex.uv.y);
                else
                    vertex.uv.y = 0.0;

                fsl_mem_array_push(&uv_buf, &vertex.uv, sizeof(v2f32));
            }
            else if (token[1] == 'n' && !token[2])
            {
                token = strtok_r(NULL, delim, &token_save[0]);
                fsl_convert_str_to_f32(token, &vertex.normal.x);

                token = strtok_r(NULL, delim, &token_save[0]);
                fsl_convert_str_to_f32(token, &vertex.normal.y);

                token = strtok_r(NULL, delim, &token_save[0]);
                fsl_convert_str_to_f32(token, &vertex.normal.z);

                fsl_mem_array_push(&normal_buf, &vertex.normal, sizeof(v3f32));
            }
        }
        else if (token[0] == 'f' && token[1] == '\0')
        {
            if (!index_dst->cap && fsl_mem_array_init(index_dst) != FSL_ERR_SUCCESS)
                goto cleanup;

            i = 0;
            while ((token = strtok_r(NULL, delim, &token_save[0])) != NULL)
            {
                get_vertex_indices_internal(token, &vertex_indices,
                        vertex_buf.cursor / sizeof(v3f32),
                        uv_buf.cursor / sizeof(v2f32),
                        normal_buf.cursor / sizeof(v3f32));
                vertex_hash = fsl_hash_djb2_u64(&vertex_indices, 3 * sizeof(u32));
                bucket_index = vertex_hash & (FSL_HASH_TABLE_MAX - 1);
                node_curr = &hash_table[bucket_index];

                while (node_curr)
                {
                    if (node_curr->hash == vertex_hash)
                    {
                        vertex_hash_index = node_curr->index;
                        hash_found = TRUE;
                        break;
                    }
                    node_curr = node_curr->next;
                }

                if (!hash_found)
                {
                    node_new = &hash_table[bucket_index];
                    node_new->hash = vertex_hash;
                    node_new->index = vertex_dst->cursor / sizeof(struct mesh_vertex);
                    node_new->next = node_curr;

                    vertex_hash_index = node_new->index;
                    vertex = novertex;
                    if (vertex_indices.pos)
                        vertex.pos = *((v3f32*)vertex_buf.buf + vertex_indices.pos - 1);
                    if (vertex_indices.uv)
                        vertex.uv = *((v2f32*)uv_buf.buf + vertex_indices.uv - 1);
                    if (vertex_indices.normal)
                        vertex.normal = *((v3f32*)normal_buf.buf + vertex_indices.normal - 1);
                    fsl_mem_array_push(vertex_dst, &vertex, sizeof(struct mesh_vertex));
                }

                triangle.curr = vertex_hash_index;
                if (i == 0)
                    triangle.first = triangle.curr;
                else if (i > 1)
                    fsl_mem_array_push(index_dst, &triangle, 3 * sizeof(u32));
                triangle.prev = triangle.curr;
                ++i;
            }
        }
    }

    fclose(file);
    fsl_mem_unmap((void*)&hash_table, FSL_HASH_TABLE_MAX * sizeof(fsl_hash_node*),
            "mesh_load_obj_internal().hash_table");
    fsl_mem_array_free(&vertex_buf);
    fsl_mem_array_free(&uv_buf);
    fsl_mem_array_free(&normal_buf);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    if (file)
        fclose(file);
    fsl_mem_unmap((void*)&hash_table, FSL_HASH_TABLE_MAX * sizeof(fsl_hash_node*),
            "mesh_load_obj_internal().hash_table");
    fsl_mem_array_free(vertex_dst);
    fsl_mem_array_free(index_dst);
    fsl_mem_array_free(&vertex_buf);
    fsl_mem_array_free(&uv_buf);
    fsl_mem_array_free(&normal_buf);
    return fsl_err;
}

static void get_vertex_indices_internal(str *token, struct vertex_indices *vertex,
        fsl_len pos_len, fsl_len uv_len, fsl_len normal_len)
{
    str *p = token;
    str temp[FSL_ID_CAP] = {0};
    u32 i = 0;
    u32 j = 0;
    i32 sign = 1;
    i64 index[3] = {0};
    fsl_len len[3] = {0};
    len[0] = pos_len;
    len[1] = uv_len;
    len[2] = normal_len;

    vertex->pos = 0;
    vertex->uv = 0;
    vertex->normal = 0;
    for (; i < 3; ++i, j = 0, sign = 1)
    {
        while(*p && *p != '/')
        {
            if (fsl_is_digit(*p) || *p == '-')
            {
                if (*p == '-')
                {
                    sign = -1;
                    ++p;
                    while (*p && !fsl_is_digit(*p) && *p != '/')
                        ++p;
                }

                while (*p && fsl_is_digit(*p))
                    temp[j++] = *p++;
                temp[j] = 0;
                fsl_convert_str_to_i64(temp, &index[i]);
                index[i] *= sign;
                if (len[i] && index[i] != (i64)len[i])
                    index[i] = fsl_mod_i64(index[i], len[i]);
                if (sign < 0)
                    ++index[i];
                break;
            }
            ++p;
        }

        while (*p && !fsl_is_digit(*p) && *p != '/')
            ++p;
        if (*p == '/')
            ++p;
    }
    vertex->pos = (u32)index[0];
    vertex->uv = (u32)index[1];
    vertex->normal = (u32)index[2];
}
