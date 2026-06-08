#version 430 core

#define SCALE 12.0

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in mat4 a_transform;

layout (std140, binding = 0) uniform ubo_ndc_scale
{
    vec2 ndc_scale;
};

out vec3 vertex_color;

void main()
{
    /* color is written in the place of normals, from the original file */
    vertex_color = a_normal;

    gl_Position = a_transform * vec4(a_pos * ndc_scale.y * SCALE, 1.0);
}
