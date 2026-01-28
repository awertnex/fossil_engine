#include "h/common.h"
#include "h/core.h"
#include "h/shaders.h"

fsl_mesh fsl_mesh_unit_quad = {0};
fsl_texture fsl_texture_buf[FSL_TEXTURE_INDEX_COUNT] = {0};

fsl_shader_program fsl_shader_buf[FSL_SHADER_INDEX_COUNT] =
{
    [FSL_SHADER_INDEX_UNIT_QUAD] =
    {
        .name = "unit_quad",
        .vertex.file_name = "unit_quad.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "unit_quad.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [FSL_SHADER_INDEX_TEXT] =
    {
        .name = "text",
        .vertex.file_name = "text.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .geometry.file_name = "text.geom",
        .geometry.type = GL_GEOMETRY_SHADER,
        .fragment.file_name = "text.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [FSL_SHADER_INDEX_UI] =
    {
        .name = "ui",
        .vertex.file_name = "ui.vert",
        .vertex.type = GL_VERTEX_SHADER,
        .fragment.file_name = "ui.frag",
        .fragment.type = GL_FRAGMENT_SHADER,
    },

    [FSL_SHADER_INDEX_UI_9_SLICE] =
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
