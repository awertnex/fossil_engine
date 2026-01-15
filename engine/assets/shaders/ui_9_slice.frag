#version 430 core

uniform sampler2D texture_screen;
uniform vec4 tint;
in vec2 gs_tex_coords;
out vec4 color;

void main()
{ 
    color = texture(texture_screen, gs_tex_coords) * tint;
}
