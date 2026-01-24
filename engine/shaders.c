#include <stdio.h>
#include <string.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/shaders.h"

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

u32 fsl_shader_init(const str *shaders_dir, fsl_shader *shader)
{
    if (!shader->type)
    {
        fsl_err = FSL_ERR_SHADER_TYPE_NULL;
        return fsl_err;
    }

    str str_reg[PATH_MAX] = {0};
    snprintf(str_reg, PATH_MAX, "%s", shaders_dir);
    fsl_check_slash(str_reg);
    fsl_posix_slash(str_reg);
    strncat(str_reg, shader->file_name, PATH_MAX - strlen(str_reg));

    shader->source = fsl_shader_pre_process(str_reg, NULL);
    if (!shader->source)
    {
        _LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Shader Source '%s' NULL\n", shader->file_name);
        return fsl_err;
    }
    (shader->id) ? glDeleteShader(shader->id) : 0;

    shader->id = glCreateShader(shader->type);
    glShaderSource(shader->id, 1, (const GLchar**)&shader->source, NULL);
    glCompileShader(shader->id);
    fsl_mem_free((void*)&shader->source, strlen(shader->source), "fsl_shader_init().shader.source");
    shader->source = NULL;

    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &shader->loaded);
    if (!shader->loaded)
    {
        char log[FSL_STRING_MAX];
        glGetShaderInfoLog(shader->id, FSL_STRING_MAX, NULL, log);
        _LOGERROR(FSL_ERR_SHADER_COMPILE_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Shader '%s':\n%s\n", shader->file_name, log);
        return fsl_err;
    }
    else _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            "Shader %s[%u] Loaded\n", shader->file_name, shader->id);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

static str *_fsl_shader_pre_process(const str *path, u64 *file_len, u64 recursion_limit)
{
    if (!recursion_limit)
    {
        _LOGERROR(FSL_ERR_INCLUDE_RECURSION_LIMIT,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Failed to Pre-Process Shader, Include Recursion Limit Exceeded '%s'\n", path);
        return NULL;
    }

    static str token[2][256] =
    {
        "#include \"",
        "\"\n",
    };

    u64 i = 0, j = 0, k = 0, cursor = 0;
    str *buf = NULL;
    str *buf_include = NULL;
    str *buf_resolved = NULL;
    u64 buf_len = fsl_get_file_contents(path, (void*)&buf, 1, TRUE);
    u64 buf_include_len = 0;
    u64 buf_resolved_len = 0;
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
            u64 string_len = 0;
            for (j = strlen(token[0]); i + j < buf_len &&
                    strncmp(buf + i + j, token[1], strlen(token[1]));
                    ++string_len, ++j)
            {}
            j += strlen(token[1]);

            str string[PATH_MAX] = {0};
            snprintf(string, PATH_MAX, "%s", path);
            fsl_retract_path(string);
            snprintf(string + strlen(string), PATH_MAX - strlen(string),
                    "%.*s", (int)string_len, buf + i + strlen(token[0]));

            if (!strncmp(string, path, strlen(string)))
            {
                _LOGERROR(FSL_ERR_SELF_INCLUDE,
                        FSL_FLAG_LOG_NO_VERBOSE,
                        "Failed to Pre-Process Shader, Self Include Detected '%s'\n", path);
                goto cleanup;
            }
            buf_include = _fsl_shader_pre_process(string, &buf_include_len, recursion_limit - 1);
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

    if (shader->source)
        fsl_mem_free((void*)&shader->source, strlen(shader->source),
                "fsl_shader_free().shader.source");

    shader->loaded = FALSE;

    _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            "Shader %s[%u] Unloaded\n", shader->file_name, shader->id);
}

u32 fsl_shader_program_init(const str *shaders_dir, fsl_shader_program *program)
{
    fsl_shader_init(shaders_dir, &program->vertex);
    if (fsl_err != FSL_ERR_SUCCESS && fsl_err != FSL_ERR_SHADER_TYPE_NULL)
        return fsl_err;
    fsl_shader_init(shaders_dir, &program->geometry);
    if (fsl_err != FSL_ERR_SUCCESS && fsl_err != FSL_ERR_SHADER_TYPE_NULL)
        return fsl_err;
    fsl_shader_init(shaders_dir, &program->fragment);
    if (fsl_err != FSL_ERR_SUCCESS && fsl_err != FSL_ERR_SHADER_TYPE_NULL)
        return fsl_err;

    (program->id) ? glDeleteProgram(program->id) : 0;

    program->id = glCreateProgram();
    if (program->vertex.id)
        glAttachShader(program->id, program->vertex.id);
    if (program->geometry.id)
        glAttachShader(program->id, program->geometry.id);
    if (program->fragment.id)
        glAttachShader(program->id, program->fragment.id);
    glLinkProgram(program->id);

    glGetProgramiv(program->id, GL_LINK_STATUS, &program->loaded);
    if (!program->loaded)
    {
        char log[FSL_STRING_MAX];
        glGetProgramInfoLog(program->id, FSL_STRING_MAX, NULL, log);
        _LOGERROR(FSL_ERR_SHADER_PROGRAM_LINK_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE,
                "Shader Program '%s':\n%s\n", program->name, log);
        return fsl_err;
    }
    else _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            "Shader Program %s[%u] Loaded\n", program->name, program->id);

    if (program->vertex.loaded)
        glDeleteShader(program->vertex.id);
    if (program->geometry.loaded)
        glDeleteShader(program->geometry.id);
    if (program->fragment.loaded)
        glDeleteShader(program->fragment.id);

    fsl_err = FSL_ERR_SUCCESS;
    return fsl_err;
}

void fsl_shader_program_free(fsl_shader_program *program)
{
    if (program == NULL || !program->loaded)
        return;

    glDeleteProgram(program->id);

    fsl_shader_free(&program->vertex);
    fsl_shader_free(&program->geometry);
    fsl_shader_free(&program->fragment);

    program->loaded = FALSE;

    _LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE,
            "Shader Program %s[%u] Unloaded\n", program->name, program->id);
}
