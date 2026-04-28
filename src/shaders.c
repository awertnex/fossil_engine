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
#include "h/logger.h"
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

u32 fsl_shader_init(const str *shaders_dir, fsl_shader *shader, b8 *shader_created)
{
    str temp[PATH_MAX] = {0};
    char log[FSL_STRING_MAX] = {0};

    if (!shader->type)
    {
        fsl_err = FSL_ERR_SHADER_TYPE_NULL;
        return fsl_err;
    }

    snprintf(temp, PATH_MAX, "%s", shaders_dir);
    fsl_check_slash(temp);
    fsl_posix_slash(temp);
    strncat(temp, shader->file_name, PATH_MAX - strlen(temp));

    shader->source = fsl_shader_pre_process(temp, NULL);
    if (!shader->source)
    {
        _LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Shader Source '%s' NULL\n", shader->file_name));
        *shader_created = FALSE;
        return fsl_err;
    }

    shader->id = glCreateShader(shader->type);
    *shader_created = TRUE;

    glShaderSource(shader->id, 1, (const GLchar**)&shader->source, NULL);
    glCompileShader(shader->id);
    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &shader->loaded);
    if (!shader->loaded)
    {
        glGetShaderInfoLog(shader->id, FSL_STRING_MAX, NULL, log);
        _LOGERROR(FSL_ERR_SHADER_COMPILE_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Shader '%s':\n%s", shader->file_name, log));
        return fsl_err;
    }
    else
    {
        _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Shader %s[%u] Loaded\n", shader->file_name, shader->id));
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
        _LOGERROR(FSL_ERR_INCLUDE_RECURSION_LIMIT,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Failed to Pre-Process Shader, Include Recursion Limit Exceeded '%s'\n", path));
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
                _LOGERROR(FSL_ERR_SELF_INCLUDE,
                        FSL_FLAG_LOG_NO_VERBOSE,
                        fsl_logger_stringf("Failed to Pre-Process Shader, Self Include Detected '%s'\n", path));
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

    fsl_mem_free((void*)&buf_include, buf_include_len, "_fslshader_pre_process().buf_include");
    fsl_mem_free((void*)&buf_resolved, buf_resolved_len, "_fslshader_pre_process().buf_resolved");
    fsl_mem_free((void*)&buf, buf_len, "_fslshader_pre_process().buf");
    return NULL;
}

void fsl_shader_free(fsl_shader *shader)
{
    if (!shader || !shader->type || !shader->loaded)
        return;

    shader->loaded = 0;
    if (shader->source)
        fsl_mem_free((void*)&shader->source, strlen(shader->source),
                "fsl_shader_free().shader->source");

    _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("Shader %s[%u] Unloaded\n", shader->file_name, shader->id));
}

u32 fsl_shader_program_init(const str *shaders_dir, fsl_shader_program *program)
{
    char log[FSL_STRING_MAX] = {0};
    fsl_shader_program program_temp = *program;
    b8 vertex_created = FALSE;
    b8 geometry_created = FALSE;
    b8 fragment_created = FALSE;
    b8 program_created = FALSE;

    program_temp.vertex.source = NULL;
    program_temp.geometry.source = NULL;
    program_temp.fragment.source = NULL;

    fsl_shader_init(shaders_dir, &program_temp.vertex, &vertex_created);
    if (fsl_err != FSL_ERR_SUCCESS)
        goto cleanup;
    fsl_shader_init(shaders_dir, &program_temp.geometry, &geometry_created);
    if (fsl_err != FSL_ERR_SUCCESS && fsl_err != FSL_ERR_SHADER_TYPE_NULL)
        goto cleanup;
    fsl_shader_init(shaders_dir, &program_temp.fragment, &fragment_created);
    if (fsl_err != FSL_ERR_SUCCESS)
        goto cleanup;

    program_temp.id = glCreateProgram();
    program_created = TRUE;

    glAttachShader(program_temp.id, program_temp.vertex.id);
    glAttachShader(program_temp.id, program_temp.geometry.id);
    glAttachShader(program_temp.id, program_temp.fragment.id);
    glLinkProgram(program_temp.id);

    glGetProgramiv(program_temp.id, GL_LINK_STATUS, &program_temp.loaded);
    if (!program_temp.loaded)
    {
        glGetProgramInfoLog(program_temp.id, FSL_STRING_MAX, NULL, log);
        _LOGERROR(FSL_ERR_SHADER_PROGRAM_LINK_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Shader Program '%s':\n%s", program_temp.name, log));
        goto cleanup;
    }
    else
    {
        _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
                fsl_logger_stringf("Shader Program %s[%u] Loaded\n", program_temp.name, program->id));
    }

    if (program_temp.vertex.loaded)
        glDeleteShader(program_temp.vertex.id);
    if (program_temp.geometry.loaded)
        glDeleteShader(program_temp.geometry.id);
    if (program_temp.fragment.loaded)
        glDeleteShader(program_temp.fragment.id);

    if (program->loaded)
        glDeleteProgram(program->id);
    if (program->vertex.loaded)
        glDeleteShader(program->vertex.id);
    if (program->geometry.loaded)
        glDeleteShader(program->geometry.id);
    if (program->fragment.loaded)
        glDeleteShader(program->fragment.id);

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

    if (program_created)
        glDeleteProgram(program_temp.id);
    if (vertex_created)
        glDeleteShader(program_temp.vertex.id);
    if (geometry_created)
        glDeleteShader(program_temp.geometry.id);
    if (fragment_created)
        glDeleteShader(program_temp.fragment.id);

    if (program_temp.vertex.loaded)
        fsl_shader_free(&program_temp.vertex);
    if (program_temp.geometry.loaded)
        fsl_shader_free(&program_temp.geometry);
    if (program_temp.fragment.loaded)
        fsl_shader_free(&program_temp.fragment);
    return fsl_err;
}

void fsl_shader_program_free(fsl_shader_program *program)
{
    if (program == NULL || !program->loaded)
        return;

    program->loaded = 0;
    glDeleteProgram(program->id);

    fsl_shader_free(&program->vertex);
    fsl_shader_free(&program->geometry);
    fsl_shader_free(&program->fragment);

    _LOGTRACE(FSL_FLAG_LOG_NO_VERBOSE,
            fsl_logger_stringf("Shader Program %s[%u] Unloaded\n", program->name, program->id));
}
