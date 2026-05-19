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
 *  @file shader_pre_processor.c
 *
 *  @brief pre-processing glsl shaders.
 */

#include "../common/diagnostics.h"
#include "../common/limits.h"
#include "../common/types.h"
#include "../logger/logger.h"
#include "../logger/logger_messages_internal.h"
#include "../memory/memory.h"

#include "../h/dir.h"
#include "shader_pre_processor_internal.h"

#include <stdio.h>
#include <string.h>

/* ---- section: signatures ------------------------------------------------- */

/*!
 *  @internal
 *
 *  @brief process shader before compilation.
 *
 *  parse includes recursively.
 *
 *  @return `NULL` on failure and @ref fsl_err is set accordingly.
 */
static str *fsl_shader_pre_process_includes_internal(const str *path, u64 *file_len, u64 recursion_limit);

/* ---- section: implementation --------------------------------------------- */

str *fsl_shader_pre_process_internal(const str *path, u64 *file_len)
{
    return fsl_shader_pre_process_includes_internal(path, file_len, FSL_SHADER_PRE_PROCESS_INCLUDE_RECURSION_MAX);
}

static str *fsl_shader_pre_process_includes_internal(const str *path, u64 *file_len, u64 recursion_limit)
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
    str temp[FSL_PATH_CAP] = {0};
    u64 temp_len = 0;

    if (!recursion_limit)
    {
        LOGERROR(FSL_ERR_INCLUDE_RECURSION_LIMIT,
                FSL_FLAG_LOG_NO_VERBOSE,
                MSG_INCLUDE_RECURSION_LIMIT_EXCEED_ACTION_SUBJECT("Pre-Process Shader", path));
        return NULL;
    }

    buf_len = fsl_get_file_contents(path, (void*)&buf, TRUE);
    if (fsl_err != FSL_ERR_SUCCESS)
        return NULL;

    if (fsl_mem_alloc((void*)&buf_resolved, buf_len + 1,
                "fsl_shader_pre_process_includes_internal().buf_resolved") != FSL_ERR_SUCCESS)
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

            snprintf(temp, FSL_PATH_CAP, "%s", path);
            fsl_retract_path(temp);
            snprintf(temp + strlen(temp), FSL_PATH_CAP - strlen(temp),
                    "%.*s", (int)temp_len, buf + i + strlen(token[0]));

            if (!strncmp(temp, path, strlen(temp)))
            {
                LOGERROR(FSL_ERR_SELF_INCLUDE,
                        FSL_FLAG_LOG_NO_VERBOSE,
                        MSG_SELF_INCLUDE_DETECT_ACTION_SUBJECT("Pre-Process Shader", path));
                goto cleanup;
            }

            buf_include = fsl_shader_pre_process_includes_internal(temp, &buf_include_len, recursion_limit - 1);
            if (fsl_err != FSL_ERR_SUCCESS ||
                    fsl_mem_realloc((void*)&buf_resolved, buf_resolved_len + buf_include_len + 1,
                        "fsl_shader_pre_process_includes_internal().buf_resolved") != FSL_ERR_SUCCESS)
                goto cleanup;
            buf_resolved_len += buf_include_len;

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%.*s", (int)(i - k), buf + k);

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%s", buf_include);

            k = i + j;
            fsl_mem_free((void*)&buf_include, buf_include_len, "fsl_shader_pre_process_includes_internal().buf_include");
        }
    }

    if (k < buf_len)
        snprintf(buf_resolved + cursor,
                buf_resolved_len - cursor, "%s", buf + k);

    fsl_mem_free((void*)&buf, buf_len, "fsl_shader_pre_process_includes_internal().buf");
    if (file_len) *file_len = buf_resolved_len;

    fsl_err = FSL_ERR_SUCCESS;
    return buf_resolved;

cleanup:

    fsl_mem_free((void*)&buf_include, buf_include_len, "fsl_shader_pre_process_includes_internal().buf_include");
    fsl_mem_free((void*)&buf_resolved, buf_resolved_len, "fsl_shader_pre_process_includes_internal().buf_resolved");
    fsl_mem_free((void*)&buf, buf_len, "fsl_shader_pre_process_includes_internal().buf");
    return NULL;
}

u32 fsl_shader_get_type_internal(const str *file, GLenum *type)
{
    str base_name[FSL_ID_CAP] = {0};
    str *extension = {0};

    if (!file || !type)
    {
        LOGERROR(FSL_ERR_POINTER_NULL, FSL_FLAG_LOG_NO_VERBOSE,
                MSG_ACTION_REASON_ERROR("Get Shader Type", "Pointer `NULL`"));
        return fsl_err;
    }

    snprintf(base_name, FSL_ID_CAP, "%s", file);
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
