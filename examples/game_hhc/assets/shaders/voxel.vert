#version 430 core

layout (location = 0) in uint a_data;
layout (location = 1) in uint a_pos;
layout (location = 2) in vec3 a_transform;

out uint vs_data;
out vec3 vs_pos;

void main()
{
    vs_data = a_data;
    vs_pos = a_transform + vec3(
            (a_pos >> 0) & 0xf,
            (a_pos >> 4) & 0xf,
            (a_pos >> 8) & 0xf);
}
