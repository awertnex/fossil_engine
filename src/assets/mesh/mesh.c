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
 *  @file mesh.c
 *
 *  @brief mesh loading, generating, unloading and mesh file formats.
 */

#include "../../common/diagnostics.h"
#include "../../common/types.h"
#include "../assets.h"
#include "../../logger/logger.h"
#include "../../logger/logger_messages_internal.h"
#include "../../memory/memory.h"
#include "../../shaders/shaders.h"

#include "../../h/dir.h"
#include "../../h/string.h"

#include "mesh.h"
#include "mesh_loader_internal.h"

#include <stdio.h>
#include <string.h>

/*!
 *  @brief generate GPU-ready vertex arrays and transforms for a mesh.
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly on failure.
 */
static u32 mesh_generate_arrays(fsl_mesh *mesh, fsl_array vertex_buf, fsl_array index_buf);

u32 fsl_mesh_load(fsl_mesh *mesh,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path)
{
    fsl_mesh mesh_temp = {0};
    fsl_fs_path path_temp[FSL_PATH_CAP] = {0};
    fsl_mesh_format format = 0;
    fsl_array vertex_buf = {0};
    fsl_array index_buf = {0};

    if (!mesh)
    {
        LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Load Mesh", name_id, "Pointer `NULL`"));
        return fsl_err;
    }

    snprintf(path_temp, FSL_PATH_CAP, "%s%s", path, file);
    if (fsl_is_file_exists(path_temp, FALSE) != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_MESH_LOAD_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Load Mesh", path_temp, "File Not Found"));
        return fsl_err;
    }

    if (mesh_get_format_internal(file, &format) != FSL_ERR_SUCCESS)
        return fsl_err;

    switch(format)
    {
        case FSL_MESH_FORMAT_OBJ:
            if (mesh_load_obj_internal(path_temp, &vertex_buf, &index_buf) != FSL_ERR_SUCCESS)
                goto cleanup;
            break;
        case FSL_MESH_FORMAT_FBX:
            if (mesh_load_fbx_internal(path_temp, &vertex_buf, &index_buf) != FSL_ERR_SUCCESS)
                goto cleanup;
            break;
        case FSL_MESH_FORMAT_GLTF:
            if (mesh_load_gltf_internal(path_temp, &vertex_buf, &index_buf) != FSL_ERR_SUCCESS)
                goto cleanup;
            break;
        case FSL_MESH_FORMAT_GLB:
            if (mesh_load_glb_internal(path_temp, &vertex_buf, &index_buf) != FSL_ERR_SUCCESS)
                goto cleanup;
            break;
        default: /* error logging not needed here, already handled earlier */
            return fsl_err;
    }

    if (fsl_asset_set_metadata(&mesh_temp.asset, FSL_ASSET_MESH, name, name_id, file, path) != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_MESH_GENERATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Load Mesh", name_id, "`fsl_asset_set_metadata()` Failed"));
        goto cleanup;
    }

    if (mesh_generate_arrays(&mesh_temp, vertex_buf, index_buf) != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_mem_array_free(&vertex_buf);
    fsl_mem_array_free(&index_buf);

    mesh_temp.asset.initialized = TRUE;
    *mesh = mesh_temp;
    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_MESH_INIT(name_id));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mem_array_free(&vertex_buf);
    fsl_mem_array_free(&index_buf);
    fsl_mesh_free(&mesh_temp);
    return fsl_err;
}

void fsl_mesh_draw(fsl_mesh *mesh, f32 pos_x, f32 pos_y, f32 pos_z,
        f32 roll, f32 pitch, f32 yaw)
{
}

u32 fsl_mesh_generate(fsl_mesh *mesh,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path,
        void (*attrib)(void), GLenum usage,
        GLuint vbo_len, GLuint ebo_len, GLfloat *vbo_data, GLuint *ebo_data)
{
    GLfloat *vbo_data_p = NULL;
    GLuint *ebo_data_p = NULL;
    fsl_asset_metadata metadata = {0};

    if (fsl_mem_arena_push(&mem_arena_internal, &mesh->vbo_data, sizeof(GLfloat) * vbo_len,
                "fsl_mesh_generate().mesh->vbo_data") != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_MESH_GENERATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Generate Mesh", name_id, "`fsl_mem_arena_push()` Failed"));
        goto cleanup;
    }

    if (ebo_data)
    {
        if (fsl_mem_arena_push(&mem_arena_internal, &mesh->ebo_data, sizeof(GLuint) * ebo_len,
                    "fsl_mesh_generate().mesh->ebo_data") != FSL_ERR_SUCCESS)
        {
            LOGERROR(FSL_ERR_MESH_GENERATION_FAIL,
                    FSL_FLAG_LOG_NO_VERBOSE,
                    MSG_ACTION_SUBJECT_REASON_ERROR("Generate Mesh", name_id, "`fsl_mem_arena_push()` Failed"));
            goto cleanup;
        }
    }

    if (fsl_asset_set_metadata(&mesh->asset, FSL_ASSET_MESH, name, name_id, file, path) != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_MESH_GENERATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Generate Mesh", name_id, "`fsl_asset_set_metadata()` Failed"));
        goto cleanup;
    }

    metadata = fsl_asset_get_metadata(mesh->asset);

    vbo_data_p = fsl_mem_handle_get(mesh->vbo_data);
    ebo_data_p = fsl_mem_handle_get(mesh->ebo_data);
    mesh->vbo_len = vbo_len;
    mesh->ebo_len = ebo_len;
    memcpy(vbo_data_p, vbo_data, sizeof(GLfloat) * vbo_len);
    memcpy(ebo_data_p, ebo_data, sizeof(GLuint) * ebo_len);

    /* ---- bind mesh ------------------------------------------------------- */

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);

    glBufferData(GL_ARRAY_BUFFER, mesh->vbo_len * sizeof(GLfloat), vbo_data_p, usage);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo_len * sizeof(GLuint), ebo_data_p, usage);

    if (attrib) attrib();

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    mesh->asset.initialized = TRUE;
    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_MESH_INIT(metadata.name_id));

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mesh_free(mesh);
    fsl_err = FSL_ERR_MESH_GENERATION_FAIL;
    return fsl_err;
}

void fsl_mesh_free(fsl_mesh *mesh)
{
    fsl_mesh nomesh = {0};

    if (mesh == NULL)
        return;

    if (mesh->asset.initialized)
    {
        mesh->asset.initialized = FALSE;
        glDeleteBuffers(1, &mesh->ebo);
        glDeleteBuffers(1, &mesh->vbo);
        glDeleteVertexArrays(1, &mesh->vao);
    }

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_MESH_UNLOAD(fsl_mem_handle_get(mesh->asset.name_id)));

    /* TODO: use `fsl_mem_pop_arena()` when you make it */
    *mesh = nomesh;
}

static u32 mesh_generate_arrays(fsl_mesh *mesh, fsl_array vertex_buf, fsl_array index_buf)
{
    u64 stride_vertex = 2 * sizeof(v3f32) + sizeof(v2f32);
    u64 stride_instance = sizeof(m4f32);
    fsl_asset_metadata metadata = {0};

    metadata = fsl_asset_get_metadata(mesh->asset);

    if (
            fsl_mem_arena_push(&mem_arena_internal, &mesh->vertex_buf.buf, vertex_buf.cursor,
                fsl_stringf("mesh_generate_arrays().%s", metadata.name_id)) != FSL_ERR_SUCCESS ||
            fsl_mem_arena_push(&mem_arena_internal, &mesh->transform_buf.buf, stride_instance,
                fsl_stringf("mesh_generate_arrays().%s", metadata.name_id)) != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_MESH_GENERATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Generate Mesh Arrays", metadata.name_id, "`fsl_mem_arena_push()` Failed"));
        goto cleanup;
    }

    if (index_buf.cursor &&
            fsl_mem_arena_push(&mem_arena_internal, &mesh->index_buf.buf, index_buf.cursor,
                fsl_stringf("mesh_generate_arrays().%s", metadata.name_id)) != FSL_ERR_SUCCESS)
    {
        LOGERROR(FSL_ERR_MESH_GENERATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Generate Mesh Arrays", metadata.name_id, "`fsl_mem_arena_push()` Failed"));
        goto cleanup;
    }

    /* ---- bind mesh ------------------------------------------------------- */

    glGenVertexArrays(1, &mesh->vao);
    glBindVertexArray(mesh->vao);

    glGenBuffers(1, &mesh->vertex_buf.id);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vertex_buf.id);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertex_buf.len * sizeof(GLfloat),
            vertex_buf.buf, GL_STATIC_DRAW);

    mesh->vertex_buf.initialized = TRUE;
    mesh->transform_buf.initialized = TRUE;

    if (index_buf.cursor)
    {
        glGenBuffers(1, &mesh->index_buf.id);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buf.id);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->index_buf.len * sizeof(GLuint),
                index_buf.buf, GL_STATIC_DRAW);

        mesh->index_buf.initialized = TRUE;
    }

    /* position */
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride_vertex, (void*)0);

    /* normal */
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride_vertex, (void*)(3 * sizeof(GLfloat)));

    /* uv */
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride_vertex, (void*)(6 * sizeof(GLfloat)));

    mesh->transform_buf.len = 16;
    glGenBuffers(1, &mesh->transform_buf.id);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->transform_buf.id);
    glBufferData(GL_ARRAY_BUFFER, mesh->transform_buf.len * sizeof(GLfloat),
            NULL, GL_DYNAMIC_DRAW);

    /* transform, row 1 */
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, stride_instance, (void*)0);
    glVertexAttribDivisor(3, 1);

    /* transform, row 2 */
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, stride_instance, (void*)(4 * sizeof(GLfloat)));
    glVertexAttribDivisor(4, 1);

    /* transform, row 3 */
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, stride_instance, (void*)(8 * sizeof(GLfloat)));
    glVertexAttribDivisor(5, 1);

    /* transform, row 4 */
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, stride_instance, (void*)(12 * sizeof(GLfloat)));
    glVertexAttribDivisor(6, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mesh_free(mesh);
    fsl_err = FSL_ERR_MESH_GENERATION_FAIL;
    return fsl_err;
}
