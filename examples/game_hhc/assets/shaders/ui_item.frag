#version 430 core

#extension GL_ARB_bindless_texture: require

layout(std430, binding = 2) readonly buffer ssbo_textures
{
    sampler2D textures[];
};

uniform sampler2D texture_block;
in vec4 pos;
in vec3 normal;
in vec2 uv;
in flat uint face_index;
out vec4 color;

vec3 base_color = vec3(0.89, 0.83, 0.81);

void main()
{
    vec4 texture_base = texture(textures[face_index], uv);
    base_color = texture_base.rgb;

    /* reinhard tone mapping */
    base_color /= base_color + vec3(0.3);

    color = vec4(base_color * texture_base.a, 1.0) * texture_base.a;
}
