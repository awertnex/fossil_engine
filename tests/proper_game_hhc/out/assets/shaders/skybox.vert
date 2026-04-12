#version 430 core

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec2 a_tex_coords;

uniform float texture_scale;
uniform mat4 mat_translation;
uniform mat4 mat_rotation;
uniform mat4 mat_sun_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;
uniform vec3 sun_rotation;
out vec2 tex_coords;

void main()
{
    tex_coords = a_tex_coords * texture_scale;

    gl_Position =
        mat_projection *
        mat_orientation *
        mat_rotation *
        mat_translation *
        mat_sun_rotation *
        vec4(a_pos, 1.0);
}
