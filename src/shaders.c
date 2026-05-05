/*  @file shaders.c
 *
 *  @brief loading, pre-processing, parsing and unloading glsl shaders.
 *
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
 *  limitations under the License.OFTWARE.
 */

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "logger/log.h"
#include "h/memory.h"
#include "h/shaders.h"

#include <stdio.h>
#include <string.h>

#define fsl_shader_pre_process(path, file_len) \
    _fsl_shader_pre_process(path, file_len, FSL_INCLUDE_RECURSION_MAX)

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief process shader before compilation.
 *
 *  parse includes recursively.
 *
 *  @return `NULL` on failure and @ref fsl_err is set accordingly.
 */
static str *_fsl_shader_pre_process(const str *path, u64 *file_len, u64 recursion_limit);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief get shader type based on file name conventions.
 *
 *  examples:
 *      - file name "fbo.vert" -> shader type "GL_VERTEX_SHADER".
 *      - file name "frag.glsl" or "fragment.glsl" -> shader type "GL_FRAGMENT_SHADER".
 *
 *  @return non-zero on failure and @ref fsl_err is set accordingly.
 */
static u32 _fsl_shader_get_type(const str *file, GLenum *type);

u32 fsl_shader_init(fsl_shader *shader, b8 *shader_created)
{
    GLint status = 0;
    str temp[PATH_MAX] = {0};
    char log[FSL_STRING_MAX] = {0};
    GLenum type = 0;

    snprintf(temp, PATH_MAX, "%s%s", shader->asset.path, shader->asset.file);

    if (fsl_is_file_exists(temp, FALSE) != FSL_ERR_SUCCESS)
    {
        fsl_err = FSL_ERR_SHADER_TYPE_NULL;
        return fsl_err;
    }

    if (_fsl_shader_get_type(shader->asset.file, &type) != FSL_ERR_SUCCESS)
        return fsl_err;

    shader->source = fsl_shader_pre_process(temp, NULL);
    if (!shader->source)
    {
        LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_SUBJECT_REASON_ERROR("Initialize Shader", shader->asset.file, "`fsl_shader_pre_process()` Failed"));
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
                fsl_logger_stringf("Shader '%s':\n%s", shader->asset.file, log));
        return fsl_err;
    }
    else
    {
        shader->asset.loaded = TRUE;
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_SHADER_INIT(shader->asset.file, shader->asset.id));
    }

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

static str *_fsl_shader_pre_process(const str *path, u64 *file_len, u64 recursion_limit)
{
    static str token[2][256] =
    {
        "#include \"",
        "\"\n",
    };

    u64 i = 0;
    u64 j = 0;
    u64 k = 0;
    u64 cursor = 0;
    str *buf = NULL;
    str *buf_include = NULL;
    str *buf_resolved = NULL;
    u64 buf_len = 0;
    u64 buf_include_len = 0;
    u64 buf_resolved_len = 0;
    str temp[PATH_MAX] = {0};
    u64 temp_len = 0;

    if (!recursion_limit)
    {
        LOGERROR(FSL_ERR_INCLUDE_RECURSION_LIMIT,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_INCLUDE_RECURSION_LIMIT_EXCEED_ACTION_SUBJECT("Pre-Process Shader", path));
        return NULL;
    }

    buf_len = fsl_get_file_contents(path, (void*)&buf, 1, TRUE);
    if (fsl_err != FSL_ERR_SUCCESS)
        return NULL;

    if (fsl_mem_alloc((void*)&buf_resolved, buf_len + 1,
                "_fsl_shader_pre_process().buf_resolved") != FSL_ERR_SUCCESS)
        goto cleanup;

    buf_resolved_len = buf_len + 1;
    snprintf(buf_resolved, buf_resolved_len, "%s", buf);
    for (; i < buf_len; ++i)
    {
        if ((i == 0 || (buf[i - 1] == '\n')) &&
                (buf[i] == '#') &&
                !strncmp(buf + i, token[0], strlen(token[0])))
        {
            temp_len = 0;
            for (j = strlen(token[0]); i + j < buf_len &&
                    strncmp(buf + i + j, token[1], strlen(token[1]));
                    ++temp_len, ++j)
            {}
            j += strlen(token[1]);

            snprintf(temp, PATH_MAX, "%s", path);
            fsl_retract_path(temp);
            snprintf(temp + strlen(temp), PATH_MAX - strlen(temp),
                    "%.*s", (int)temp_len, buf + i + strlen(token[0]));

            if (!strncmp(temp, path, strlen(temp)))
            {
                LOGERROR(FSL_ERR_SELF_INCLUDE,
                        FSL_FLAG_LOG_NO_VERBOSE,
                        MSG_SELF_INCLUDE_DETECT_ACTION_SUBJECT("Pre-Process Shader", path));
                goto cleanup;
            }

            buf_include = _fsl_shader_pre_process(temp, &buf_include_len, recursion_limit - 1);
            if (fsl_err != FSL_ERR_SUCCESS ||
                    fsl_mem_realloc((void*)&buf_resolved, buf_resolved_len + buf_include_len + 1,
                        "_fsl_shader_pre_process().buf_resolved") != FSL_ERR_SUCCESS)
                goto cleanup;
            buf_resolved_len += buf_include_len;

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%.*s", (int)(i - k), buf + k);

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%s", buf_include);

            k = i + j;
            fsl_mem_free((void*)&buf_include, buf_include_len, "_fsl_shader_pre_process().buf_include");
        }
    }

    if (k < buf_len)
        snprintf(buf_resolved + cursor,
                buf_resolved_len - cursor, "%s", buf + k);

    fsl_mem_free((void*)&buf, buf_len, "_fsl_shader_pre_process().buf");
    if (file_len) *file_len = buf_resolved_len;

    fsl_err = FSL_ERR_SUCCESS;
    return buf_resolved;

cleanup:

    fsl_mem_free((void*)&buf_include, buf_include_len, "_fsl_shader_pre_process().buf_include");
    fsl_mem_free((void*)&buf_resolved, buf_resolved_len, "_fsl_shader_pre_process().buf_resolved");
    fsl_mem_free((void*)&buf, buf_len, "_fsl_shader_pre_process().buf");
    return NULL;
}

static u32 _fsl_shader_get_type(const str *file, GLenum *type)
{
    str base_name[NAME_MAX] = {0};
    str *extension = {0};

    if (!file || !type)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Get Shader Type", "Pointer `NULL`"));
        return fsl_err;
    }

    snprintf(base_name, NAME_MAX, "%s", file);
    extension = strrchr(base_name, '.');
    if (extension)
    {
        ++extension;

        if (!strncmp(extension, "glsl", 5))
            extension = base_name;
    }
    else extension = base_name;

    if (!strncmp(extension, "vertex", 6) || !strncmp(extension, "vert", 4))
    {
        *type = GL_VERTEX_SHADER;
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }
    if (!strncmp(extension, "geometry", 8) || !strncmp(extension, "geom", 4))
    {
        *type = GL_GEOMETRY_SHADER;
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }
    if (!strncmp(extension, "fragment", 8) || !strncmp(extension, "frag", 4))
    {
        *type = GL_FRAGMENT_SHADER;
        fsl_err = FSL_ERR_SUCCESS;
        return fsl_err;
    }

    LOGERROR(FSL_ERR_SHADER_TYPE_NULL, FSL_FLAG_LOG_NO_VERBOSE,
            MSG_ACTION_SUBJECT_REASON_ERROR("Get Shader Type", base_name, "Type Detection Failed"));
    return fsl_err;
}

void fsl_shader_free(fsl_shader *shader)
{
    fsl_shader noshader = {0};

    if (!shader || !shader->asset.loaded)
        return;

    shader->asset.loaded = FALSE;
    if (shader->source)
        fsl_mem_free((void*)&shader->source, strlen(shader->source),
                "fsl_shader_free().shader->source");

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_SHADER_UNLOAD(shader->asset.name, shader->asset.id));

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
                fsl_logger_stringf("Shader Program '%s':\n%s", program_temp.asset.name, log));
        goto cleanup;
    }
    else
    {
        program_temp.asset.loaded = TRUE;
        LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                MSG_SHADER_PROGRAM_LOAD(program_temp.asset.name, program_temp.asset.id));
    }

    if (program_temp.vertex.asset.loaded)
    {
        program_temp.vertex.asset.loaded = FALSE;
        glDeleteShader(program_temp.vertex.asset.id);
    }
    if (program_temp.geometry.asset.loaded)
    {
        program_temp.geometry.asset.loaded = FALSE;
        glDeleteShader(program_temp.geometry.asset.id);
    }
    if (program_temp.fragment.asset.loaded)
    {
        program_temp.fragment.asset.loaded = FALSE;
        glDeleteShader(program_temp.fragment.asset.id);
    }

    if (program->asset.loaded)
    {
        program->asset.loaded = FALSE;
        glDeleteProgram(program->asset.id);
    }
    if (program->vertex.asset.loaded)
    {
        program->vertex.asset.loaded = FALSE;
        glDeleteShader(program->vertex.asset.id);
    }
    if (program->geometry.asset.loaded)
    {
        program->geometry.asset.loaded = FALSE;
        glDeleteShader(program->geometry.asset.id);
    }
    if (program->fragment.asset.loaded)
    {
        program->fragment.asset.loaded = FALSE;
        glDeleteShader(program->fragment.asset.id);
    }

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

    if (program_temp.vertex.asset.loaded)
        fsl_shader_free(&program_temp.vertex);
    if (program_temp.geometry.asset.loaded)
        fsl_shader_free(&program_temp.geometry);
    if (program_temp.fragment.asset.loaded)
        fsl_shader_free(&program_temp.fragment);
    return fsl_err;
}

void fsl_shader_program_free(fsl_shader_program *program)
{
    fsl_shader_program noprogram = {0};

    if (!program || !program->asset.loaded)
        return;

    program->asset.loaded = FALSE;
    glDeleteProgram(program->asset.id);

    fsl_shader_free(&program->vertex);
    fsl_shader_free(&program->geometry);
    fsl_shader_free(&program->fragment);

    LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            MSG_SHADER_PROGRAM_UNLOAD(program->asset.name, program->asset.id));

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
