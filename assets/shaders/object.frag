#version 430 core

in vec4 pos;
in vec3 normal;
in vec2 uv;
out vec4 color;

vec3 base_color = vec3(0.19, 0.13, 0.01);

void main()
{
    /* reinhard tone mapping */
    base_color /= base_color + vec3(1.0);

    color = vec4(base_color, 1.0);
}
