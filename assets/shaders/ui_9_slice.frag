#version 430 core

uniform sampler2D texture_image;
uniform vec4 tint;
in vec2 tex_coords;
out vec4 color;

void main()
{ 
    color = vec4(texture(texture_image, tex_coords).rgb, 1.0) * tint;
}
