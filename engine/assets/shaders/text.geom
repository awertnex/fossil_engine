#version 430 core

#define MAX_VERTICES 6

layout (points) in;
layout (triangle_strip, max_vertices = MAX_VERTICES) out;

uniform float char_size;
uniform vec2 font_size;
in vec2 vs_tex_coords[];
out vec2 tex_coords;

void main()
{
    vec4 vbo_quad[MAX_VERTICES] =
        vec4[](
                /* vertex_pos, tex_coords */
                vec4(0.0, -1.0, 0.0, 1.0),
                vec4(0.0, 0.0, 0.0, 0.0),
                vec4(1.0, 0.0, 1.0, 0.0),
                vec4(1.0, 0.0, 1.0, 0.0),
                vec4(1.0, -1.0, 1.0, 1.0),
                vec4(0.0, -1.0, 0.0, 1.0)
              );

    for (int i = 0; i < MAX_VERTICES; ++i)
    {
        gl_Position = vec4(-1.0, 1.0, 0.0, 0.0) +
            gl_in[0].gl_Position +
             vec4(vbo_quad[i].xy * font_size, 0.0, 0.0);

        tex_coords = vs_tex_coords[0] +
            (vbo_quad[i].zw * char_size);

        EmitVertex();
        if ((i + 1) % 3 == 0)
            EndPrimitive();
    }
}
