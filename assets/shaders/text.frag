#version 430 core

uniform sampler2DArray texture_characters;
in vec2 vs_tex_coords;
in flat uint vs_char_index;
in vec4 vs_color;
out vec4 color;

void main()
{
    color = vs_color * texture(texture_characters, vec3(vs_tex_coords, float(vs_char_index))).r;
}
