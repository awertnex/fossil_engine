#version 430 core

layout (location = 0) in uvec2 a_data;

uniform float gizmo_offset;
out vec3 vs_position;
out vec4 vs_color;
float inv_255 = 1 / 255.0;

void main()
{
    uint alpha = a_data.y & 0xff;

    if (a_data.y == 0)
    {
        vs_color.a = 0.0;
        return;
    }

    vs_position = vec3(
            (a_data.x >> 0x18) & 0xff,
            (a_data.x >> 0x10) & 0xff,
            (a_data.x >> 0x08) & 0xff) - gizmo_offset;

    vs_color = vec4(
            (a_data.y >> 0x18) & 0xff,
            (a_data.y >> 0x10) & 0xff,
            (a_data.y >> 0x08) & 0xff,
            alpha) * (alpha * inv_255) * inv_255;
}
