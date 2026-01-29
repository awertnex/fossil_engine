#version 430 core

in vec4 position;
in vec3 normal;
out vec4 color;

#define USE_SUN_DIRECTION
#include "h/defaults.glsl"

vec3 base_color = vec3(0.35, 0.58, 1.41);

void main()
{
    base_color += sun_direction * SUN_INFLUENCE;

    /* reinhard tone mapping */
    base_color /= base_color + vec3(1.0);

    color = vec4(base_color, 1.0);
}
