#version 430 core

uniform vec2 ndc_scale;
uniform ivec2 position;
uniform ivec2 alignment;
out vec2 vs_ndc_scale;

void main()
{
    vec2 vs_alignment = vec2(alignment.x, -alignment.y);
    vec2 vs_position = vec2(position.x, -position.y) * ndc_scale;
    vs_ndc_scale = ndc_scale;
    gl_Position = vec4(vs_position + vs_alignment, 0.0, 1.0);
}
