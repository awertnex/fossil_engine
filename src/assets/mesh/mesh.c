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

#include "mesh.h"
#include "mesh_loader_internal.h"

#include <stdio.h>
#include <string.h>

u32 fsl_mesh_load(fsl_mesh *mesh,
        const fsl_name *name, const fsl_name_id *name_id, const fsl_file *file, const fsl_path *path)
{
    fsl_mesh mesh_temp = {0};
    fsl_fs_path path_temp[FSL_PATH_CAP] = {0};
    fsl_mesh_format format = 0;
    GLfloat *vbo_data_p = NULL;
    GLuint *ebo_data_p = NULL;

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

    if (fsl_mesh_get_format_internal(file, &format) != FSL_ERR_SUCCESS)
        return fsl_err;

    switch(format)
    {
        case FSL_MESH_FORMAT_OBJ:
            if (fsl_mesh_load_obj_internal(&mesh_temp, path_temp) != FSL_ERR_SUCCESS)
                goto cleanup;
            break;
        case FSL_MESH_FORMAT_FBX:
            if (fsl_mesh_load_fbx_internal(&mesh_temp, path_temp) != FSL_ERR_SUCCESS)
                goto cleanup;
            break;
        case FSL_MESH_FORMAT_GLTF:
            if (fsl_mesh_load_gltf_internal(&mesh_temp, path_temp) != FSL_ERR_SUCCESS)
                goto cleanup;
            break;
        case FSL_MESH_FORMAT_GLB:
            if (fsl_mesh_load_glb_internal(&mesh_temp, path_temp) != FSL_ERR_SUCCESS)
                goto cleanup;
            break;
        default: /* error logging not needed here, already handled earlier */
            return fsl_err;
    }

    vbo_data_p = fsl_mem_handle_get(mesh_temp.vbo_data);
    ebo_data_p = fsl_mem_handle_get(mesh_temp.ebo_data);
    mesh->vbo_data = mesh_temp.vbo_data;
    mesh->ebo_data = mesh_temp.ebo_data;

    if (fsl_mesh_generate(mesh, name, name_id, file, path, fsl_attrib_vec3_vec3, GL_STATIC_DRAW,
                mesh_temp.vbo_len, mesh_temp.ebo_len, vbo_data_p, ebo_data_p) != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    fsl_mesh_free(&mesh_temp);
    return fsl_err;
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
