#ifndef ENGINE_SHADERS_H
#define ENGINE_SHADERS_H

#include <engine/include/glad/glad.h>

#include "types.h"

typedef struct Shader
{
    str *file_name;
    GLuint id;          /* used by 'glCreateShader()' */
    GLuint type;        /* 'GL_<x>_SHADER' */
    GLchar *source;     /* shader file source code */
    GLint loaded;       /* used by 'glGetShaderiv()' */
} Shader;

typedef struct ShaderProgram
{
    str *name;          /* for debugging */
    GLuint id;          /* used by 'lCreateProgram()' */
    GLint loaded;       /* used by 'glGetProgramiv()' */
    Shader vertex;
    Shader geometry;
    Shader fragment;
} ShaderProgram;

typedef struct UBO
{
    GLuint index;
    GLuint buf;
} UBO;

/*! @brief default shaders.
 *
 *  @remark declared internally in 'core.h/engine_init()'.
 */
extern ShaderProgram engine_shader[ENGINE_SHADER_COUNT];

/*! @brief initialize single shader.
 *
 *  calls 'shader_pre_process()' on 'shader->file_name' before compiling
 *  shader, then compiles shader.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 shader_init(const str *shaders_dir, Shader *shader);

/*! @brief initialize shader program.
 *
 *  @param shaders_dir = path to shader files of 'program',
 *  shader file names must be pre-defined in 'program.shader.file_name'.
 *
 *  calls 'shader_init()' on all shaders in 'program' if 'shader->type' is set.
 *
 *  @return non-zero on failure and 'engine_err' is set accordingly.
 */
u32 shader_program_init(const str *shaders_dir, ShaderProgram *program);

void shader_program_free(ShaderProgram *program);

#endif /* ENGINE_SHADERS_H */
