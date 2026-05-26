#version 430 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;

uniform vec3 location;
uniform vec3 scale;
uniform mat4 mat_rotation;
uniform mat4 mat_perspective;
out vec3 normal;
out vec2 uv;
vec4 pos;

void main()
{
    pos = vec4(a_pos * scale, 1.0);
    pos *= mat_rotation;
    pos.xyz += location;
    normal = a_normal;
    uv = a_uv;

    gl_Position = mat_perspective * pos;
}
