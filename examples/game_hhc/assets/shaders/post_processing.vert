#version 430 core

out vec2 vs_pos;
out vec2 vs_uv;

const vec2 unit_quad[4] = vec2[4](
        vec2(-1.0, -1.0),
        vec2(-1.0, 1.0),
        vec2(1.0, 1.0),
        vec2(1.0, -1.0));

void main()
{
    vec4 pos_quad = vec4(unit_quad[gl_VertexID], 0.0, 1.0);
    vs_pos = pos_quad.xy;
    vs_uv = (pos_quad.xy + 1.0) * 0.5;
    gl_Position = pos_quad;
}
