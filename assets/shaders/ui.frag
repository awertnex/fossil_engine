#version 430 core

uniform sampler2D texture_image;
uniform vec4 tint;
in vec2 vs_tex_coords;
out vec4 color;

void main()
{ 
    color = texture(texture_image, vs_tex_coords) * tint;
}
