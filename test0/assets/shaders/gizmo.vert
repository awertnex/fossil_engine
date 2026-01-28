#version 430 core

#define SCALE 12.0

layout (location = 0) in vec3 a_pos;

uniform vec2 ndc_scale;
uniform mat4 mat_translation;
uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;

void main()
{
    gl_Position =
        mat_projection *
        mat_orientation *
        mat_rotation *
        mat_translation *
        vec4((a_pos - 0.0025) * SCALE * ndc_scale.y, 1.0);
}
