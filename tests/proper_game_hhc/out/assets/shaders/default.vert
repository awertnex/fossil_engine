#version 430 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;

uniform vec3 offset;
uniform vec3 scale;
uniform mat4 mat_rotation;
uniform mat4 mat_perspective;
out vec4 position;
out vec3 normal;

void main()
{
    position = vec4(a_pos * scale, 1.0);
    position.xy -= (scale.xy / 2.0);
    position *= mat_rotation;
    position.xyz += offset;
    normal = a_normal;
    gl_Position = mat_perspective * position;
}
