#version 430 core

layout (location = 0) in uint a_data;
layout (location = 1) in uint a_pos;

out uint vs_data;
out vec3 vs_position;
uniform vec3 chunk_position;

void main()
{
    vs_data = a_data;
    vs_position = chunk_position + vec3(
            (a_pos >> 0) & 0xf,
            (a_pos >> 4) & 0xf,
            (a_pos >> 8) & 0xf);
}
