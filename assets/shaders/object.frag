#version 430 core

uniform sampler2D texture_0;
in vec4 pos;
in vec3 normal;
in vec2 uv;
out vec4 color;

vec3 base_color = vec3(0.19, 0.13, 0.01);

void main()
{
    /* reinhard tone mapping */
    base_color /= base_color + vec3(1.0);

    color = texture(texture_0, uv);
    color = vec4(color.rgb, color.a) * color.a;
}
