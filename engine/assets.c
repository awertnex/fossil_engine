#include "h/common.h"
#include "h/core.h"
#include "h/shaders.h"

Mesh engine_mesh_unit = {0};
Texture engine_texture[ENGINE_TEXTURE_COUNT] = {0};

ShaderProgram engine_shader[ENGINE_SHADER_COUNT] =
{
    [ENGINE_SHADER_UNIT_QUAD] =
    {
        .name = "unit_quad",
        .vertex.file_name = "unit_quad.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "unit_quad.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [ENGINE_SHADER_TEXT] =
    {
        .name = "text",
        .vertex.file_name = "text.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .geometry.file_name = "text.geom",
        .geometry.type = GL_GEOMETRY_SHADER,
        .fragment.file_name = "text.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [ENGINE_SHADER_UI] =
    {
        .name = "ui",
        .vertex.file_name = "ui.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "ui.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [ENGINE_SHADER_UI_9_SLICE] =
    {
        .name = "ui_9_slice",
        .vertex.file_name = "ui_9_slice.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .geometry.file_name = "ui_9_slice.geom",
        .geometry.type = GL_GEOMETRY_SHADER,
        .fragment.file_name = "ui_9_slice.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },
};
