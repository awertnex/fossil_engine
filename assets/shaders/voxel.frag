#version 430 core

#extension GL_ARB_bindless_texture: require

layout(binding = 1, std430) readonly buffer ssbo_textures
{
    sampler2D textures[];
};

uniform sampler2D texture_block;
uniform float opacity;
uniform vec3 flashlight_position;
uniform float toggle_flashlight;
uniform int render_distance;
in vec3 vertex_position;
in vec2 tex_coords;
in vec3 normal;
in flat uint face_index;
in float block_light;
out vec4 color;

#define USE_SUN_DIRECTION
#define USE_MATH
#define USE_FOG
#define USE_TONE_MAPPING
#include "h/defaults.glsl"

void main()
{
    float distance = square_length(vertex_position - flashlight_position);
    float flashlight_intensity = (1.0 / distance * FLASHLIGHT_DISTANCE) *
        FLASHLIGHT_INTENSITY * toggle_flashlight;
    float sky_brightness = clamp(sun_rotation.z, 0.0, 1.0) *
        ((sky_color.r + sky_color.g + sky_color.b) / 3.0);

    vec4 texture_base = texture(textures[face_index], tex_coords);
    vec3 color_sky_influence = texture_base.rgb * sky_color *
        SKY_INFLUENCE * block_light * sky_brightness;
    vec3 color_sun_influence = texture_base.rgb * sun_direction *
        SUN_INFLUENCE * sky_brightness;
    vec3 color_block_light = block_light * color_sun_influence;
    vec3 color_ambient_light = texture_base.rgb * GLOBAL_ILLUMINATION;
    vec3 color_flashlight = FLASHLIGHT_COLOR * texture_base.rgb * flashlight_intensity;

    vec3 color_composite = 
        color_sky_influence +
        color_block_light +
        color_ambient_light +
        color_flashlight;

    vec3 color_final = mix(color_composite, sky_color,
            fog_linear(distance / float(render_distance * (1.0 - FOG_SOFTNESS / 1.5)),
                render_distance - render_distance * FOG_SOFTNESS,
                render_distance + render_distance * FOG_SOFTNESS));

    color_final = reinhard_tone_mapping(color_final, 30.0);

    color = vec4(color_final, 1.0) * texture_base.a * opacity;
}
