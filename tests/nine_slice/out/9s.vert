#version 430 core

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec2 a_tex_coords;
layout (location = 2) in vec2 a_slice_pos;
layout (location = 3) in vec2 a_slice_size;
layout (location = 4) in vec2 a_slice_tex_coords_pos;
layout (location = 5) in vec2 a_slice_tex_coords_size;

layout (std140, binding = 0) uniform ubo_ndc_scale
{
    vec2 ndc_scale;
};

out vec2 tex_coords;

void main(void)
{
    vec2 slice_pos =
        vec2(a_pos * a_slice_size) +
        vec2(a_slice_pos.x, -a_slice_pos.y);

    gl_Position = vec4(-1.0, 1.0, 0.0, 1.0) +
            vec4(slice_pos * ndc_scale, 0.0, 0.0);

    tex_coords = a_tex_coords * a_slice_tex_coords_size + a_slice_tex_coords_pos;
}
