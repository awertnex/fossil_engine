#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_uv;

uniform vec2 uv_pos;
uniform vec2 uv_size;
uniform vec2 pos;
uniform vec2 size;
out vec2 uv;

void main()
{
    gl_Position = vec4(a_pos * size + pos, 0.0, 1.0);
    uv = a_uv * uv_size + uv_pos;
}
