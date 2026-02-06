#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;
layout (location = 2) in vec2 a_pos_offset;
layout (location = 3) in uint a_char_index;
layout (location = 4) in uint a_color;

layout (std140, binding = 0) uniform ubo_ndc_scale
{
    vec2 ndc_scale;
};

uniform vec2 font_size;
uniform int draw_shadow;
uniform vec4 shadow_color;
uniform vec2 shadow_offset;
out vec2 vs_tex_coords;
out flat uint vs_char_index;
out vec4 vs_color;
const float inv_255 = 1.0 / 255.0;

void main()
{
    if (!bool(draw_shadow))
    {
        gl_Position = vec4(
                vec2(-1.0, 1.0) +
                a_pos * font_size +
                a_pos_offset, 0.0, 1.0);

        vs_color = vec4(
                float((a_color >> 0x18) & 0xff),
                float((a_color >> 0x10) & 0xff),
                float((a_color >> 0x08) & 0xff),
                float((a_color >> 0x00) & 0xff)) * inv_255;
    }
    else
    {
        gl_Position = vec4(
                vec2(-1.0, 1.0) +
                a_pos * font_size +
                vec2(shadow_offset.x, -shadow_offset.y) * ndc_scale +
                a_pos_offset, 0.0, 1.0);

        vs_color = shadow_color;
    }

    vs_tex_coords = a_tex_coords;
    vs_char_index = a_char_index;
}
