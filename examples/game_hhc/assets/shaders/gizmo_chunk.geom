#version 430 core

#define VERTICES_MAX 36
#define GIZMO_SCALE 150.0
#define RENDER_DISTANCE_MAX 32.0
#define RENDER_DISTANCE_MIN 2.0
#define CAMERA_DISTANCE_PADDING 3.0
#define PULSE_TIME_MULTIPLIER 4.0
#define PULSE_FREQUENCY 0.2
#define PULSE_AMPLITUDE 0.04

layout(points) in;
layout(triangle_strip, max_vertices = VERTICES_MAX) out;

uniform ivec2 render_size;
uniform int chunk_buf_diameter;
uniform mat4 mat_translation;
uniform mat4 mat_rotation;
uniform mat4 mat_orientation;
uniform mat4 mat_projection;
uniform float time;
in vec3 vs_position[];
in vec4 vs_color[];
out vec3 vertex_position;
out vec4 chunk_color;
out float camera_distance;

void main()
{
    if (vs_color[0].a == 0.0) return;

    vec3 vbo[8] = vec3[](
            vec3(0.0, 0.0, 0.0),
            vec3(1.0, 0.0, 0.0),
            vec3(0.0, 1.0, 0.0),
            vec3(1.0, 1.0, 0.0),
            vec3(0.0, 0.0, 1.0),
            vec3(1.0, 0.0, 1.0),
            vec3(0.0, 1.0, 1.0),
            vec3(1.0, 1.0, 1.0));

    int ebo[VERTICES_MAX] = int[](
            1, 5, 7, 7, 3, 1,
            2, 6, 4, 4, 0, 2,
            3, 7, 6, 6, 2, 3,
            0, 4, 5, 5, 1, 0,
            4, 6, 7, 7, 5, 4,
            0, 1, 3, 3, 2, 0);

    float gizmo_scale = GIZMO_SCALE * (1.0 / chunk_buf_diameter) * (2.0 / render_size.y);

    mat4 mat_offset = mat4(
            1.0, 0.0, 0.0, 0.0,
            0.0, 1.0, 0.0, 0.0,
            0.0, 0.0, 1.0, 0.0,
            (render_size.x - GIZMO_SCALE * 2.0) / render_size.x,
            (render_size.y - GIZMO_SCALE * 2.0) / render_size.y,
            0.0, 1.0);

    float size = sin(vs_position[0].z * PULSE_FREQUENCY - time * PULSE_TIME_MULTIPLIER) *
        PULSE_AMPLITUDE + 1.0f - PULSE_AMPLITUDE;

    chunk_color = vs_color[0];
    camera_distance = float(chunk_buf_diameter) / 2.0 + CAMERA_DISTANCE_PADDING;

    for (int i = 0; i < VERTICES_MAX; ++i)
    {
        vertex_position = vbo[ebo[i]] * size + ((1.0 - size) / 2.0);
        vertex_position += vs_position[0];
        gl_Position =
            mat_offset *
            mat_projection *
            mat_orientation *
            mat_rotation *
            mat_translation *
            vec4(vertex_position * gizmo_scale, 1.0);
        EmitVertex();
        if ((i + 1) % 3 == 0) EndPrimitive();
    }
}
