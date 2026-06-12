#version 430 core

layout (location = 0) in vec2 a_quad_pos;
layout (location = 1) in vec2 a_quad_uv;
layout (location = 2) in vec2 a_uv_pos;
layout (location = 3) in vec2 a_uv_size;
layout (location = 4) in vec2 a_pos;
layout (location = 5) in vec2 a_size;

out vec2 uv;

void main()
{
    gl_Position = vec4(a_quad_pos * a_size + a_pos, 0.0, 1.0);
    uv = a_quad_uv * a_uv_size + a_uv_pos;
}
