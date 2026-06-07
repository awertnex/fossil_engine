#version 430 core

layout (location = 0) in uint a_data;
layout (location = 1) in uint a_pos;
layout (location = 2) in vec3 a_location;

out uint vs_data;
out vec3 vs_position;

void main()
{
    vs_data = a_data;
    vs_position = a_location + vec3(
            (a_pos >> 0) & 0xf,
            (a_pos >> 4) & 0xf,
            (a_pos >> 8) & 0xf);
}
