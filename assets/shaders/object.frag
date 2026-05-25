#version 430 core

in vec3 normal;
in vec2 uv;
out vec4 color;

vec3 base_color = vec3(0.35, 0.58, 1.41);

void main()
{
    /* reinhard tone mapping */
    base_color /= base_color + vec3(1.0);

    color = vec4(base_color, 1.0);
}
