#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;
layout (location = 2) in uint a_color;

layout (std140, binding = 0) uniform ubo_ndc_scale
{
    vec2 ndc_scale;
};

uniform int draw_shadow;
uniform vec4 shadow_color;
uniform vec2 shadow_offset;
out vec2 vs_pos;
out vec2 vs_tex_coords;
out vec4 vs_color;
const float inv_255 = 1.0 / 255.0;

void main()
{
    vs_tex_coords = a_tex_coords;

    if (!bool(draw_shadow))
    {
        vs_pos = a_pos;
        vs_color = vec4(
                float((a_color >> 0x18) & 0xff),
                float((a_color >> 0x10) & 0xff),
                float((a_color >> 0x08) & 0xff),
                float((a_color >> 0x00) & 0xff)) * inv_255;
    }
    else
    {
        vs_pos = a_pos + vec2(shadow_offset.x, -shadow_offset.y) * ndc_scale;
        vs_color = shadow_color;
    }
}
