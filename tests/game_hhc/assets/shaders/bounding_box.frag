#version 430 core

uniform vec4 box_color;
out vec4 color;

void main()
{
    color = vec4(box_color.rgb, 1.0) * box_color.a;
}
