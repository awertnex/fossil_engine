#version 430 core

uniform sampler2D texture_screen;
in vec2 uv;
out vec4 color;

void main()
{
    color = texture(texture_screen, uv);
}
