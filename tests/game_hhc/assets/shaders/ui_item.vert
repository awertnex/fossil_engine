#version 430 core

#define MASK_BLOCK_ID 0x3ff

layout (location = 0) in vec3 a_pos;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;
layout (location = 3) in mat4 a_transform;
layout (location = 7) in uint a_data;

layout(std430, binding = 1) readonly buffer ssbo_texture_indices
{
    uint texture_indices[];
};

uniform mat4 mat_perspective;
out vec4 pos;
out vec3 normal;
out vec2 uv;
out flat uint face_index;

void main()
{
    pos = vec4(a_pos, 1.0);
    normal = a_normal;
    uv = a_uv;
    face_index = texture_indices[(a_data & MASK_BLOCK_ID) * 6];

    gl_Position = a_transform * pos;
}
