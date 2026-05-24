#version 430 core

in vec4 position;
in vec3 normal;
in vec2 tex_coords;
out vec4 color;
uniform vec3 sun_rotation;

float sun_direction = clamp(dot(normal, sun_rotation), 0.0, 1.0);
float moon_direction = clamp(dot(-normal, sun_rotation), 0.0, 1.0);

vec3 base_color = vec3(0.35, 0.58, 1.41);

void main()
{
    base_color += sun_direction * SUN_INFLUENCE;

    /* reinhard tone mapping */
    base_color /= base_color + vec3(1.0);

    color = vec4(base_color, 1.0);
}
