#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

layout (std140, binding = 0) uniform ubo_ndc_scale
{
    vec2 ndc_scale;
};

uniform vec2 position;
uniform vec2 size;
uniform vec2 uv_pos;
uniform vec2 uv_size;
out vec2 vs_uv;

void main()
{
    gl_Position =
        vec4(-1.0, 1.0, 0.0, 0.0) +
        vec4(position.x * ndc_scale.x, -position.y * ndc_scale.y, 0.0, 0.0) +
        vec4(a_pos * size * ndc_scale, 0.0, 1.0);

    vs_uv = a_uv * uv_size + uv_pos;
}
