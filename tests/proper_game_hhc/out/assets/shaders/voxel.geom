#version 430 core

#define FACE_VERTICES   6
#define VERTICES_MAX    36

layout(points) in;
layout(triangle_strip, max_vertices = VERTICES_MAX) out;

layout(std430, binding = 1) readonly buffer ssbo_texture_indices
{
    uint texture_indices[];
};

/* from 'src/h/main.h' */
#define BLOCK_ID        0x000003ff
#define BLOCK_LIGHT     0x0000003f
#define POSITIVE_X      0x00010000
#define NEGATIVE_X      0x00020000
#define POSITIVE_Y      0x00040000
#define NEGATIVE_Y      0x00080000
#define POSITIVE_Z      0x00100000
#define NEGATIVE_Z      0x00200000

uniform mat4 mat_perspective;
in uint vs_data[];
in vec3 vs_position[];
out vec3 vertex_position;
out vec2 tex_coords;
out vec3 normal;
out flat uint face_index;
out float block_light;

void main()
{
    uint block_id = vs_data[0] & BLOCK_ID;
    block_light = ((vs_data[0] >> 0x18) & BLOCK_LIGHT) / float(BLOCK_LIGHT);

    vec3 vbo[8] =
        vec3[](
                vec3(0.0, 0.0, 0.0),
                vec3(1.0, 0.0, 0.0),
                vec3(0.0, 1.0, 0.0),
                vec3(1.0, 1.0, 0.0),
                vec3(0.0, 0.0, 1.0),
                vec3(1.0, 0.0, 1.0),
                vec3(0.0, 1.0, 1.0),
                vec3(1.0, 1.0, 1.0));

    int ebo[VERTICES_MAX] =
        int[](
                1, 5, 7, 7, 3, 1,
                2, 6, 4, 4, 0, 2,
                3, 7, 6, 6, 2, 3,
                0, 4, 5, 5, 1, 0,
                4, 6, 7, 7, 5, 4,
                0, 1, 3, 3, 2, 0);

    vec2 gs_tex_coords[4] =
        vec2[](
                vec2(0.0, 1.0),
                vec2(0.0, 0.0),
                vec2(1.0, 0.0),
                vec2(1.0, 1.0));

    int ebo_tex_coords[FACE_VERTICES] =
        int[](0, 1, 2, 2, 3, 0);

    int ebo_tex_coords_top[FACE_VERTICES] =
        int[](3, 0, 1, 1, 2, 3);

    if (bool(vs_data[0] & POSITIVE_X))
        for (int i = 0; i < FACE_VERTICES; ++i)
        {
            vertex_position = vs_position[0] +
                vbo[ebo[i]];
            tex_coords = gs_tex_coords[ebo_tex_coords[i]];
            face_index = texture_indices[block_id * 6 + 0];
            normal = vec3(1.0, 0.0, 0.0);

            gl_Position = mat_perspective * vec4(vertex_position, 1.0);
            EmitVertex();
            if ((i + 1) % 3 == 0) EndPrimitive();
        }

    if (bool(vs_data[0] & NEGATIVE_X))
        for (int i = 0; i < FACE_VERTICES; ++i)
        {
            vertex_position = vs_position[0] +
                vbo[ebo[i + FACE_VERTICES]];
            tex_coords = gs_tex_coords[ebo_tex_coords[i]];
            face_index = texture_indices[block_id * 6 + 1];
            normal = vec3(-1.0, 0.0, 0.0);

            gl_Position = mat_perspective * vec4(vertex_position, 1.0);
            EmitVertex();
            if ((i + 1) % 3 == 0) EndPrimitive();
        }

    if (bool(vs_data[0] & POSITIVE_Y))
        for (int i = 0; i < FACE_VERTICES; ++i)
        {
            vertex_position = vs_position[0] +
                vbo[ebo[i + (FACE_VERTICES * 2)]];
            tex_coords = gs_tex_coords[ebo_tex_coords[i]];
            face_index = texture_indices[block_id * 6 + 2];
            normal = vec3(0.0, 1.0, 0.0);

            gl_Position = mat_perspective * vec4(vertex_position, 1.0);
            EmitVertex();
            if ((i + 1) % 3 == 0) EndPrimitive();
        }

    if (bool(vs_data[0] & NEGATIVE_Y))
        for (int i = 0; i < FACE_VERTICES; ++i)
        {
            vertex_position = vs_position[0] +
                vbo[ebo[i + (FACE_VERTICES * 3)]];
            tex_coords = gs_tex_coords[ebo_tex_coords[i]];
            face_index = texture_indices[block_id * 6 + 3];
            normal = vec3(0.0, -1.0, 0.0);

            gl_Position = mat_perspective * vec4(vertex_position, 1.0);
            EmitVertex();
            if ((i + 1) % 3 == 0) EndPrimitive();
        }

    if (bool(vs_data[0] & POSITIVE_Z))
        for (int i = 0; i < FACE_VERTICES; ++i)
        {
            vertex_position = vs_position[0] +
                vbo[ebo[i + (FACE_VERTICES * 4)]];
            tex_coords = gs_tex_coords[ebo_tex_coords_top[i]];
            face_index = texture_indices[block_id * 6 + 4];
            normal = vec3(0.0, 0.0, 1.0);

            gl_Position = mat_perspective * vec4(vertex_position, 1.0);
            EmitVertex();
            if ((i + 1) % 3 == 0) EndPrimitive();
        }

    if (bool(vs_data[0] & NEGATIVE_Z))
        for (int i = 0; i < FACE_VERTICES; ++i)
        {
            vertex_position = vs_position[0] +
                vbo[ebo[i + (FACE_VERTICES * 5)]];
            tex_coords = gs_tex_coords[ebo_tex_coords[i]];
            face_index = texture_indices[block_id * 6 + 5];
            normal = vec3(0.0, 0.0, -1.0);

            gl_Position = mat_perspective * vec4(vertex_position, 1.0);
            EmitVertex();
            if ((i + 1) % 3 == 0) EndPrimitive();
        }
}
