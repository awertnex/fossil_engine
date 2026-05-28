#version 430 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in mat4 a_transform;

out vec4 pos;
out vec3 normal;

void main()
{
    pos = vec4(a_pos, 1.0);
    normal = a_normal;

    gl_Position = a_transform * pos;
}
