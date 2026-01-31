#version 430 core

uniform sampler2D texture_image;
in vec2 tex_coords;
out vec4 color;

void main(void)
{
    color = vec4(texture(texture_image, tex_coords).rgb, 1.0);
}
