#version 430 core

uniform sampler2D texture_font_atlas;
uniform vec4 text_color;
in vec2 tex_coords;
out vec4 color;

void main()
{
    color = text_color * texture(texture_font_atlas, tex_coords).r;
}
