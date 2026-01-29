#version 430 core

#define LIGHT_INTENSITY 9.0
#define LIGHT_SHARPNESS 1.2
#define WHITE_POINT 4.0

uniform vec3 camera_position;
in vec3 vertex_position;
in vec4 chunk_color;
in float camera_distance;
out vec4 color;

void main()
{
    vec3 light_position = camera_position.xyz * camera_distance;

    float distance = inversesqrt(length(vertex_position - light_position));
    distance = pow(distance, LIGHT_SHARPNESS) * LIGHT_INTENSITY;
    float light_influence = (chunk_color.r + chunk_color.g + chunk_color.b) / 3.0;
    light_influence *= chunk_color.a * distance * (1.0 - normalize(vec3(1.0, camera_distance, 0.0)).x);

    color.rgb = chunk_color.rgb * light_influence * chunk_color.a;
    color.a = chunk_color.a;

    /* reinhard tone mapping */
    color.rgb = (color.rgb * (1.0 + color.rgb / (WHITE_POINT * WHITE_POINT))) /
        (1.0 + color.rgb);
}
