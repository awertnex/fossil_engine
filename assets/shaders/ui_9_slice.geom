#version 430 core

#define MAX_SLICES 9
#define VERTICES_PER_QUAD 6
#define MAX_VERTICES 54

layout (points) in;
layout (triangle_strip, max_vertices = MAX_VERTICES) out;

uniform ivec2 size;
uniform ivec2 texture_size;
uniform ivec2 sprite_size;
uniform int use_nine_slice;
uniform float slice_size;
in vec2 vs_ndc_scale[];
out vec2 gs_tex_coords;

void main()
{
    vec2 vbo_quad[VERTICES_PER_QUAD] =
        vec2[](
                /* position */
                vec2(0.0, -1.0),
                vec2(0.0, 0.0),
                vec2(1.0, 0.0),
                vec2(1.0, 0.0),
                vec2(1.0, -1.0),
                vec2(0.0, -1.0));

    vec2 vbo_slices[MAX_SLICES * 2] =
        vec2[](
                /* position */
                vec2(0.0),
                vec2(slice_size, 0.0),
                vec2(size.x - slice_size, 0.0),
                vec2(0.0, slice_size),
                vec2(slice_size),
                vec2(size.x - slice_size, slice_size),
                vec2(0.0, size.y - slice_size),
                vec2(slice_size, size.y - slice_size),
                vec2(size.x - slice_size, size.y - slice_size),

                /* size */
                vec2(slice_size),
                vec2(size.x - (slice_size * 2.0), slice_size),
                vec2(slice_size),
                vec2(slice_size, size.y - (slice_size * 2.0)),
                vec2(size.x - (slice_size * 2.0), size.y - (slice_size * 2.0)),
                vec2(slice_size.x, size.y - (slice_size * 2.0)),
                vec2(slice_size),
                vec2(size.x - (slice_size * 2.0), slice_size),
                vec2(slice_size));

        vec2 tex_size = vec2(1.0 / texture_size);

        vec2 tex_0 = vec2(0.0, 1.0);

        vec2 tex_1 =
            vec2(
                    tex_size.x * slice_size,
                    1.0 - (tex_size.y * slice_size));

        vec2 tex_2 =
            vec2(
                    tex_size.x * (slice_size + 1.0),
                    1.0 - (tex_size.y * (slice_size + 1.0)));

        vec2 tex_3 =
            vec2(
                    tex_size.x * (sprite_size.x + 1.0),
                    1.0 - (tex_size.y * (sprite_size.y + 1.0)));

        vec2 vbo_tex_coords[MAX_SLICES * VERTICES_PER_QUAD] =
            vec2[](
                    vec2(tex_0.x, tex_1.y),
                    vec2(tex_0.x, tex_0.y),
                    vec2(tex_1.x, tex_0.y),
                    vec2(tex_1.x, tex_0.y),
                    vec2(tex_1.x, tex_1.y),
                    vec2(tex_0.x, tex_1.y),

                    vec2(tex_1.x, tex_1.y),
                    vec2(tex_1.x, tex_0.y),
                    vec2(tex_2.x, tex_0.y),
                    vec2(tex_2.x, tex_0.y),
                    vec2(tex_2.x, tex_1.y),
                    vec2(tex_1.x, tex_1.y),

                    vec2(tex_2.x, tex_1.y),
                    vec2(tex_2.x, tex_0.y),
                    vec2(tex_3.x, tex_0.y),
                    vec2(tex_3.x, tex_0.y),
                    vec2(tex_3.x, tex_1.y),
                    vec2(tex_2.x, tex_1.y),

                    vec2(tex_0.x, tex_2.y),
                    vec2(tex_0.x, tex_1.y),
                    vec2(tex_1.x, tex_1.y),
                    vec2(tex_1.x, tex_1.y),
                    vec2(tex_1.x, tex_2.y),
                    vec2(tex_0.x, tex_2.y),

                    vec2(tex_1.x, tex_2.y),
                    vec2(tex_1.x, tex_1.y),
                    vec2(tex_2.x, tex_1.y),
                    vec2(tex_2.x, tex_1.y),
                    vec2(tex_2.x, tex_2.y),
                    vec2(tex_1.x, tex_2.y),

                    vec2(tex_2.x, tex_2.y),
                    vec2(tex_2.x, tex_1.y),
                    vec2(tex_3.x, tex_1.y),
                    vec2(tex_3.x, tex_1.y),
                    vec2(tex_3.x, tex_2.y),
                    vec2(tex_2.x, tex_2.y),

                    vec2(tex_0.x, tex_3.y),
                    vec2(tex_0.x, tex_2.y),
                    vec2(tex_1.x, tex_2.y),
                    vec2(tex_1.x, tex_2.y),
                    vec2(tex_1.x, tex_3.y),
                    vec2(tex_0.x, tex_3.y),

                    vec2(tex_1.x, tex_3.y),
                    vec2(tex_1.x, tex_2.y),
                    vec2(tex_2.x, tex_2.y),
                    vec2(tex_2.x, tex_2.y),
                    vec2(tex_2.x, tex_3.y),
                    vec2(tex_1.x, tex_3.y),

                    vec2(tex_2.x, tex_3.y),
                    vec2(tex_2.x, tex_2.y),
                    vec2(tex_3.x, tex_2.y),
                    vec2(tex_3.x, tex_2.y),
                    vec2(tex_3.x, tex_3.y),
                    vec2(tex_2.x, tex_3.y));

            for (int i = 0; i < MAX_SLICES; ++i)
            {
                vbo_slices[i].y = -vbo_slices[i].y;
                for (int j = 0; j < VERTICES_PER_QUAD; ++j)
                {
                    gl_Position = gl_in[0].gl_Position +
                        vec4(vbo_slices[i] * vs_ndc_scale[0], 0.0, 0.0) +
                        vec4(vbo_slices[i + MAX_SLICES] * vbo_quad[j] *
                                vs_ndc_scale[0], 0.0, 0.0);

                    gs_tex_coords.x = 
                        vbo_tex_coords[(i * VERTICES_PER_QUAD) + j].x;
                    gs_tex_coords.y = 1.0 +
                        (vbo_tex_coords[(i * VERTICES_PER_QUAD) + j].y * -1.0);

                    EmitVertex();
                    if ((j + 1) % 3 == 0)
                        EndPrimitive();
                }
            }
}
