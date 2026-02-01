#version 430 core

uniform sampler2D texture_font_atlas;
in vec2 vs_tex_coords;
in vec4 vs_color;
out vec4 color;

void main()
{
    color = vs_color * texture(texture_font_atlas, vs_tex_coords).r;
}
