#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;

uniform vec2 offset;
uniform vec2 ndc_scale;
out vec2 vs_tex_coords;

void main()
{
    gl_Position =
        vec4(a_pos, 0.0, 1.0) +
        vec4(vec2(offset.x, -offset.y) * ndc_scale, 0.0, 0.0);
    vs_tex_coords = a_tex_coords;
}
