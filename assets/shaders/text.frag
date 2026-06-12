#version 430 core

uniform sampler2DArray texture_characters;
in vec2 uv;
in flat uint char_index;
in vec4 vs_color;
out vec4 color;

void main()
{
    color = vs_color * texture(texture_characters, vec3(uv, float(char_index))).r;
}
