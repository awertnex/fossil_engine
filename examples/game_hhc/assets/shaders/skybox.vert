#version 430 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in mat4 a_transform;

uniform mat4 mat_translation;
uniform mat4 mat_rotation;
uniform mat4 mat_sun_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;
uniform vec3 sun_rotation;
out vec2 uv;

void main()
{
    uv = a_uv;

    gl_Position =
        mat_projection *
        mat_orientation *
        mat_rotation *
        mat_translation *
        mat_sun_rotation *
        vec4(a_pos, 1.0);
}
