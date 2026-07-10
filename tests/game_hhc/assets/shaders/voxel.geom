#version 430 core

#define FACE_VERTICES 6
#define VERTICES_MAX 36
#define FACES_MAX 6

layout(points) in;
layout(triangle_strip, max_vertices = VERTICES_MAX) out;

layout(std430, binding = 1) readonly buffer ssbo_texture_indices
{
    uint texture_buf[];
};

#define MASK_BLOCK_ID   0x000003ff
#define MASK_BLOCK_LIGHT 0x0000003f
#define FLAG_POSITIVE_X 0x00010000
#define FLAG_NEGATIVE_X 0x00020000
#define FLAG_POSITIVE_Y 0x00040000
#define FLAG_NEGATIVE_Y 0x00080000
#define FLAG_POSITIVE_Z 0x00100000
#define FLAG_NEGATIVE_Z 0x00200000

uniform mat4 mat_view;
uniform mat4 mat_perspective;
in uint vs_data[];
in vec3 vs_pos[];
out vec4 pos;
out vec4 pos_view;
out vec2 uv;
out vec3 normal;
out vec3 normal_view;
out flat uint face_index;
out float block_light;

void voxel_make()
{
    int i = 0;
    int j = 0;
    uint block_id = vs_data[0] & MASK_BLOCK_ID;
    uint block_faces = vs_data[0] >> 16;
    block_light = ((vs_data[0] >> 0x18) & MASK_BLOCK_LIGHT) / float(MASK_BLOCK_LIGHT);

    vec3 vertex_buf[8] =
        vec3[](
                vec3(0.0, 0.0, 0.0),
                vec3(1.0, 0.0, 0.0),
                vec3(0.0, 1.0, 0.0),
                vec3(1.0, 1.0, 0.0),
                vec3(0.0, 0.0, 1.0),
                vec3(1.0, 0.0, 1.0),
                vec3(0.0, 1.0, 1.0),
                vec3(1.0, 1.0, 1.0));

    vec3 normal_buf[6] =
        vec3[](
                vec3(1.0, 0.0, 0.0),
                vec3(-1.0, 0.0, 0.0),
                vec3(0.0, 1.0, 0.0),
                vec3(0.0, -1.0, 0.0),
                vec3(0.0, 0.0, 1.0),
                vec3(0.0, 0.0, -1.0));

    int index_buf[VERTICES_MAX] =
        int[](
                1, 5, 7, 7, 3, 1,
                2, 6, 4, 4, 0, 2,
                3, 7, 6, 6, 2, 3,
                0, 4, 5, 5, 1, 0,
                4, 6, 7, 7, 5, 4,
                0, 1, 3, 3, 2, 0);

    vec2 gs_uv[4] =
        vec2[](
                vec2(0.0, 1.0),
                vec2(0.0, 0.0),
                vec2(1.0, 0.0),
                vec2(1.0, 1.0));

    int index_buf_uv[FACE_VERTICES] =
        int[](0, 1, 2, 2, 3, 0);

    int ebo_uv_top[FACE_VERTICES] =
        int[](3, 0, 1, 1, 2, 3);

    mat3 mat_view_transpose = transpose(inverse(mat3(mat_view)));

    for (i = 0; i < FACES_MAX; ++i)
    {
        if (bool(block_faces & (1 << i)))
            for (j = 0; j < FACE_VERTICES; ++j)
            {
                pos = vec4(vs_pos[0] + vertex_buf[index_buf[j + FACE_VERTICES * i]], 1.0);
                pos_view = mat_view * pos;
                uv = gs_uv[index_buf_uv[j]];
                face_index = texture_buf[block_id * 6 + i];
                normal = normal_buf[i];
                normal_view = mat_view_transpose * normal;

                gl_Position = mat_perspective * pos;
                EmitVertex();
                if ((j + 1) % 3 == 0) EndPrimitive();
            }
    }
}

void main()
{
    voxel_make();
}
