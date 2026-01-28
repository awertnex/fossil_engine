#version 430 core

#define LINE_OFFSET 0.00001

layout (location = 0) in vec3 a_pos;

uniform mat4 mat_perspective;
uniform vec3 position;
uniform vec3 size;

void main()
{
    gl_Position = mat_perspective * vec4((a_pos * size) + position, 1.0);
    gl_Position.z -= LINE_OFFSET * gl_Position.w;
}
