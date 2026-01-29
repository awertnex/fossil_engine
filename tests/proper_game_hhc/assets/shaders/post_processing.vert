#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;

out vec2 vs_pos;
out vec2 vs_tex_coords;

void main()
{
    gl_Position = vec4(a_pos, 0.0, 1.0); 
    vs_pos = a_pos;
    vs_tex_coords = a_tex_coords;
}
