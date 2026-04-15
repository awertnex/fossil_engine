#version 430 core

uniform vec3 gizmo_color;
out vec4 color;

void main()
{
    color = vec4(gizmo_color, 1.0);
}
