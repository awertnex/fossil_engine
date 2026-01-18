#include <stdio.h>
#include <string.h>

#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/limits.h"
#include "h/logger.h"
#include "h/memory.h"
#include "h/shaders.h"

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief process shader before compilation.
 *
 *  parse includes recursively.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
static str *shader_pre_process(const str *path, u64 *file_len);

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief process shader before compilation.
 *
 *  parse includes recursively.
 *
 *  @return NULL on failure and 'engine_err' is set accordingly.
 */
static str *_shader_pre_process(const str *path, u64 *file_len, u64 recursion_limit);

u32 shader_init(const str *shaders_dir, Shader *shader)
{
    if (!shader->type)
    {
        engine_err = ERR_SHADER_TYPE_NULL;
        return engine_err;
    }

    str str_reg[PATH_MAX] = {0};
    snprintf(str_reg, PATH_MAX, "%s", shaders_dir);
    check_slash(str_reg);
    posix_slash(str_reg);
    strncat(str_reg, shader->file_name, PATH_MAX - strlen(str_reg));

    shader->source = shader_pre_process(str_reg, NULL);
    if (!shader->source)
    {
        _LOGERROR(FALSE, ERR_POINTER_NULL, "Shader Source '%s' NULL\n", shader->file_name);
        return engine_err;
    }
    (shader->id) ? glDeleteShader(shader->id) : 0;

    shader->id = glCreateShader(shader->type);
    glShaderSource(shader->id, 1, (const GLchar**)&shader->source, NULL);
    glCompileShader(shader->id);
    mem_free((void*)&shader->source, strlen(shader->source), "shader_init().shader.source");
    shader->source = NULL;

    glGetShaderiv(shader->id, GL_COMPILE_STATUS, &shader->loaded);
    if (!shader->loaded)
    {
        char log[STRING_MAX];
        glGetShaderInfoLog(shader->id, STRING_MAX, NULL, log);
        _LOGERROR(FALSE, ERR_SHADER_COMPILE_FAIL, "Shader '%s':\n%s\n", shader->file_name, log);
        return engine_err;
    }
    else _LOGINFO(FALSE, "Shader %d '%s' Loaded\n", shader->id, shader->file_name);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

static str *shader_pre_process(const str *path, u64 *file_len)
{
    return _shader_pre_process(path, file_len, INCLUDE_RECURSION_MAX);
}

static str *_shader_pre_process(const str *path, u64 *file_len, u64 recursion_limit)
{
    if (!recursion_limit)
    {
        _LOGFATAL(FALSE, ERR_INCLUDE_RECURSION_LIMIT,
                "Include Recursion Limit Exceeded '%s', Process Aborted\n", path);
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
    u64 buf_len = get_file_contents(path, (void*)&buf, 1, TRUE);
    u64 buf_include_len = 0;
    u64 buf_resolved_len = 0;
    if (engine_err != ERR_SUCCESS)
        return NULL;

    if (mem_alloc((void*)&buf_resolved, buf_len + 1,
                "_shader_pre_process().buf_resolved") != ERR_SUCCESS)
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
            retract_path(string);
            snprintf(string + strlen(string), PATH_MAX - strlen(string),
                    "%.*s", (int)string_len, buf + i + strlen(token[0]));

            if (!strncmp(string, path, strlen(string)))
            {
                _LOGFATAL(FALSE, ERR_SELF_INCLUDE,
                        "Self Include Detected '%s', Process Aborted\n", path);
                goto cleanup;
            }
            buf_include = _shader_pre_process(string, &buf_include_len, recursion_limit - 1);
            if (engine_err != ERR_SUCCESS ||
                    mem_realloc((void*)&buf_resolved, buf_resolved_len + buf_include_len + 1,
                        "_shader_pre_process().buf_resolved") != ERR_SUCCESS)
                goto cleanup;
            buf_resolved_len += buf_include_len;

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%.*s", (int)(i - k), buf + k);

            cursor += snprintf(buf_resolved + cursor,
                    buf_resolved_len - cursor, "%s", buf_include);

            k = i + j;
            mem_free((void*)&buf_include, buf_include_len, "_shader_pre_process().buf_include");
        }
    }

    if (k < buf_len)
        snprintf(buf_resolved + cursor,
                buf_resolved_len - cursor, "%s", buf + k);

    mem_free((void*)&buf, buf_len, "_shader_pre_process().buf");
    if (file_len) *file_len = buf_resolved_len;
    engine_err = ERR_SUCCESS;
    return buf_resolved;

cleanup:

    mem_free((void*)&buf_include, buf_include_len, "_shader_pre_process().buf_include");
    mem_free((void*)&buf_resolved, buf_resolved_len, "_shader_pre_process().buf_resolved");
    mem_free((void*)&buf, buf_len, "_shader_pre_process().buf");
    return NULL;
}

u32 shader_program_init(const str *shaders_dir, ShaderProgram *program)
{
    shader_init(shaders_dir, &program->vertex);
    if (engine_err != ERR_SUCCESS && engine_err != ERR_SHADER_TYPE_NULL)
        return engine_err;
    shader_init(shaders_dir, &program->geometry);
    if (engine_err != ERR_SUCCESS && engine_err != ERR_SHADER_TYPE_NULL)
        return engine_err;
    shader_init(shaders_dir, &program->fragment);
    if (engine_err != ERR_SUCCESS && engine_err != ERR_SHADER_TYPE_NULL)
        return engine_err;

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
        char log[STRING_MAX];
        glGetProgramInfoLog(program->id, STRING_MAX, NULL, log);
        _LOGERROR(FALSE, ERR_SHADER_PROGRAM_LINK_FAIL,
                "Shader Program '%s':\n%s\n", program->name, log);
        return engine_err;
    }
    else _LOGINFO(FALSE,
            "Shader Program %d '%s' Loaded\n", program->id, program->name);

    if (program->vertex.loaded)
        glDeleteShader(program->vertex.id);
    if (program->geometry.loaded)
        glDeleteShader(program->geometry.id);
    if (program->fragment.loaded)
        glDeleteShader(program->fragment.id);

    engine_err = ERR_SUCCESS;
    return engine_err;
}

void shader_program_free(ShaderProgram *program)
{
    if (program == NULL || !program->id) return;
    glDeleteProgram(program->id);

    if (program->vertex.source)
        mem_free((void*)&program->vertex.source, strlen(program->vertex.source),
                "shader_program_free().vertex.source");

    if (program->fragment.source)
        mem_free((void*)&program->fragment.source, strlen(program->fragment.source),
                "shader_program_free().fragment.source");

    if (program->geometry.source)
        mem_free((void*)&program->geometry.source, strlen(program->geometry.source),
                "shader_program_free().geometry.source");
}
