#version 430 core

uniform sampler2D texture_font_atlas;
in vec2 tex_coords;
in vec4 gs_color;
out vec4 color;

void main()
{
    color = gs_color * texture(texture_font_atlas, tex_coords).r;
}
