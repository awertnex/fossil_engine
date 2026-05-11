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
 *  @file shaders.c
 *
 *  @brief loading and unloading glsl shaders.
 */

#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../logger/logger.h"
#include "../memory/memory.h"

#include "../h/dir.h"
#include "../h/assets.h"
#include "shader_pre_processor.h"
#include "shaders.h"

#include <stdio.h>
#include <string.h>

u32 fsl_shader_init(fsl_shader *shader, b8 *shader_created)
{
    GLint status = 0;
    str temp[PATH_MAX] = {0};
    char log[FSL_STRING_MAX] = {0};
    GLenum type = 0;
    fsl_asset_metadata metadata = {0};

    metadata = fsl_asset_get_metadata(shader->asset);
    if (!metadata.file || !metadata.path)
    {
        fsl_err = FSL_ERR_SHADER_TYPE_NULL;
        return fsl_err;
    }

    snprintf(temp, PATH_MAX, "%s%s", metadata.path, metadata.file);
    if (fsl_is_file_exists(temp, FALSE) != FSL_ERR_SUCCESS)
    {
        fsl_err = FSL_ERR_SHADER_TYPE_NULL;
        return fsl_err;
    }

    if (fsl_shader_get_type_internal(metadata.file, &type) != FSL_ERR_SUCCESS)
        return fsl_err;

    shader->source = fsl_shader_pre_process_internal(temp, NULL);
    if (!shader->source)
    {
        LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Initialize Shader", metadata.name_id, "`fsl_shader_pre_process_internal()` Failed"));
        if (shader_created)
            *shader_created = FALSE;
        return fsl_err;
    }

    shader->asset.id = glCreateShader(type);
    if (shader_created)
        *shader_created = TRUE;

    glShaderSource(shader->asset.id, 1, (const GLchar**)&shader->source, NULL);
    glCompileShader(shader->asset.id);
    glGetShaderiv(shader->asset.id, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        glGetShaderInfoLog(shader->asset.id, FSL_STRING_MAX, NULL, log);
        LOGERROR(FSL_ERR_SHADER_COMPILE_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Shader '%s':\n%s", metadata.name_id, log));
        return fsl_err;
    }
    else
    {
        shader->asset.initialized = TRUE;
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_SHADER_INIT(metadata.name_id, shader->asset.id));
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_shader_free(fsl_shader *shader)
{
    fsl_shader noshader = {0};

    if (!shader || !shader->asset.initialized)
        return;

    shader->asset.initialized = FALSE;
    if (shader->source)
        fsl_mem_free((void*)&shader->source, strlen(shader->source),
                "fsl_shader_free().shader->source");

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_SHADER_UNLOAD(fsl_mem_handle_get(str, shader->asset.name_id), shader->asset.id));

    *shader = noshader;
}

u32 fsl_shader_program_init(fsl_shader_program *program)
{
    GLint status = 0;
    char log[FSL_STRING_MAX] = {0};
    fsl_shader_program program_temp = *program;
    b8 created_vertex = FALSE;
    b8 created_geometry = FALSE;
    b8 created_fragment = FALSE;
    b8 created_program = FALSE;

    program_temp.vertex.source = NULL;
    program_temp.geometry.source = NULL;
    program_temp.fragment.source = NULL;

    fsl_shader_init(&program_temp.vertex, &created_vertex);
    if (fsl_err != FSL_ERR_SUCCESS)
        goto cleanup;
    fsl_shader_init(&program_temp.geometry, &created_geometry);
    if (fsl_err != FSL_ERR_SUCCESS && fsl_err != FSL_ERR_SHADER_TYPE_NULL)
        goto cleanup;
    fsl_shader_init(&program_temp.fragment, &created_fragment);
    if (fsl_err != FSL_ERR_SUCCESS)
        goto cleanup;

    program_temp.asset.id = glCreateProgram();
    created_program = TRUE;

    glAttachShader(program_temp.asset.id, program_temp.vertex.asset.id);
    if (created_geometry)
        glAttachShader(program_temp.asset.id, program_temp.geometry.asset.id);
    glAttachShader(program_temp.asset.id, program_temp.fragment.asset.id);
    glLinkProgram(program_temp.asset.id);

    glGetProgramiv(program_temp.asset.id, GL_LINK_STATUS, &status);
    if (!status)
    {
        glGetProgramInfoLog(program_temp.asset.id, FSL_STRING_MAX, NULL, log);
        LOGERROR(FSL_ERR_SHADER_PROGRAM_LINK_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Shader Program '%s':\n%s", fsl_mem_handle_get(str, program_temp.asset.name_id), log));
        goto cleanup;
    }
    else
    {
        program_temp.asset.initialized = TRUE;
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_SHADER_PROGRAM_LOAD(fsl_mem_handle_get(str, program_temp.asset.name_id), program_temp.asset.id));
    }

    if (program_temp.vertex.asset.initialized)
    {
        program_temp.vertex.asset.initialized = FALSE;
        glDeleteShader(program_temp.vertex.asset.id);
    }
    if (program_temp.geometry.asset.initialized)
    {
        program_temp.geometry.asset.initialized = FALSE;
        glDeleteShader(program_temp.geometry.asset.id);
    }
    if (program_temp.fragment.asset.initialized)
    {
        program_temp.fragment.asset.initialized = FALSE;
        glDeleteShader(program_temp.fragment.asset.id);
    }

    if (program->asset.initialized)
        glDeleteProgram(program->asset.id);
    if (program->vertex.asset.initialized)
        glDeleteShader(program->vertex.asset.id);
    if (program->geometry.asset.initialized)
        glDeleteShader(program->geometry.asset.id);
    if (program->fragment.asset.initialized)
        glDeleteShader(program->fragment.asset.id);

    if (program->vertex.source) /* we make this check so we don't pass `NULL` into `strlen` */
        fsl_mem_free((void*)&program->vertex.source, strlen(program->vertex.source), "fsl_shader_program_init().program->vertex.source");
    if (program->geometry.source) /* we make this check so we don't pass `NULL` into `strlen` */
        fsl_mem_free((void*)&program->geometry.source, strlen(program->geometry.source), "fsl_shader_program_init().program->geometry.source");
    if (program->fragment.source) /* we make this check so we don't pass `NULL` into `strlen` */
        fsl_mem_free((void*)&program->fragment.source, strlen(program->fragment.source), "fsl_shader_program_init().program->fragment.source");

    *program = program_temp;

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;

cleanup:

    if (created_program)
        glDeleteProgram(program_temp.asset.id);
    if (created_vertex)
        glDeleteShader(program_temp.vertex.asset.id);
    if (created_geometry)
        glDeleteShader(program_temp.geometry.asset.id);
    if (created_fragment)
        glDeleteShader(program_temp.fragment.asset.id);

    if (program_temp.vertex.asset.initialized)
        fsl_shader_free(&program_temp.vertex);
    if (program_temp.geometry.asset.initialized)
        fsl_shader_free(&program_temp.geometry);
    if (program_temp.fragment.asset.initialized)
        fsl_shader_free(&program_temp.fragment);
    return fsl_err;
}

void fsl_shader_program_free(fsl_shader_program *program)
{
    fsl_shader_program noprogram = {0};

    if (!program || !program->asset.initialized)
        return;

    program->asset.initialized = FALSE;
    glDeleteProgram(program->asset.id);

    fsl_shader_free(&program->vertex);
    fsl_shader_free(&program->geometry);
    fsl_shader_free(&program->fragment);

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_SHADER_PROGRAM_UNLOAD(fsl_mem_handle_get(str, program->asset.name_id), program->asset.id));

    *program = noprogram;
}

/* ---- section: attrib ----------------------------------------------------- */

void fsl_attrib_vec3(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0);
}

void fsl_attrib_vec3_vec2(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

void fsl_attrib_vec3_vec3(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}

void fsl_attrib_vec3_vec4(void)
{
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
}
