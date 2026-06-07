#version 430 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in mat4 a_transform;

layout (std140, binding = 0) uniform ubo_ndc_scale
{
    vec2 ndc_scale;
};

void main()
{
    gl_Position = a_transform * vec4(a_pos * ndc_scale.y * 10.0, 1.0);
}
