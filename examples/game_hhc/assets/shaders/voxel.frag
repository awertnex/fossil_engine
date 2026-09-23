#version 430 core

#define GLOBAL_ILLUMINATION 0.5
#define SKY_INFLUENCE 1.5
#define SUN_INFLUENCE 1.7
#define MOON_INFLUENCE 0.4
#define FLASHLIGHT_DISTANCE 0.8
#define FLASHLIGHT_COLOR vec3(1.0, 0.8, 0.5)
#define FOG_SOFTNESS 1.0
#define WHITE_POINT 30.0

#extension GL_ARB_bindless_texture: require

layout(location = 0) out vec4 g_pos;
layout(location = 1) out vec4 g_normal_depth;
layout(location = 2) out vec4 g_albedo_specular;

layout(std430, binding = 2) readonly buffer ssbo_textures
{
    sampler2D textures[];
};

struct hhc_spotlight
{
    vec3 pos;
    vec3 direction;
    float cutoff;
    float feather_factor;
    float intensity;
};

uniform sampler2D texture_block;
uniform float opacity;
uniform vec3 camera_position;
uniform float camera_far;
uniform float camera_near;
uniform int render_distance;
uniform vec3 sun_rotation;
uniform vec3 sky_light;
uniform vec3 moon_light;
uniform hhc_spotlight flashlight;
in vec4 pos;
in vec4 pos_view;
in vec2 uv;
in vec3 normal;
in vec3 normal_view;
in flat uint face_index;
in float block_light;

float sun_direction = clamp(dot(normal, sun_rotation), 0.0, 1.0);
float moon_direction = clamp(dot(-normal, sun_rotation), 0.0, 1.0);

/* ---- implementation ------------------------------------------------------ */

float square_length(vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

vec3 reinhard_tone_mapping(vec3 color, float white_point)
{
    white_point *= white_point;
    color = (color * (1.0 + color / white_point)) / (1.0 + color);
    return color * color;
}

vec3 fog_linear(vec3 color_a, vec3 color_b, float distance, float min, float max)
{
    float factor = clamp((max - distance) / (max - min), 0.0, 1.0);
    return mix(color_a, color_b, 1.0 - factor);
}

float spotlight_get(hhc_spotlight spotlight, vec3 normal)
{
    vec3 frag_pos = normalize(pos.xyz - spotlight.pos);
    float spot_factor = dot(frag_pos, flashlight.direction);
    float epsilon = spotlight.cutoff - (1.0 - spotlight.feather_factor) * spotlight.cutoff;
    float value = clamp((spot_factor - spotlight.cutoff) / epsilon, 0.0, 1.0);

    return value * value * value * (value * (value * 6.0 - 15.0) + 10.0) * spotlight.intensity;
}

float bimix(vec4 v, vec2 t)
{
    t = smoothstep(0.0, 1.0, t);
    vec2 w = 1.0 - t;
    return
        v.x * w.x * w.y +
        v.y * t.x * w.y +
        v.z * w.x * t.y +
        v.w * t.x * t.y;
}

void main()
{
    float distance = square_length(pos.xyz - flashlight.pos);
    float flashlight_attenuation = 1.0 / pow(sqrt(distance), 1.0 / FLASHLIGHT_DISTANCE);
    float sun_brightness = (sky_light.r + sky_light.g + sky_light.b) / 3.0;
    float moon_brightness = (moon_light.r + moon_light.g + moon_light.b) / 3.0;

    vec4 albedo = texture(textures[face_index], uv);
    vec3 global_illumination = albedo.rgb * GLOBAL_ILLUMINATION;
    vec3 color_sky = sky_light * SKY_INFLUENCE * (sun_brightness + moon_brightness);
    float color_sun = sun_direction * SUN_INFLUENCE * sun_brightness;
    float color_moon = moon_direction * MOON_INFLUENCE * moon_brightness;
    vec3 world_light = color_sky + color_sun + color_moon;
    vec3 color_block_light = block_light * world_light;
    vec3 color_flashlight = FLASHLIGHT_COLOR *
        spotlight_get(flashlight, normal) * flashlight_attenuation;

    vec3 color_composite = global_illumination + albedo.rgb *
        (color_block_light +
         color_flashlight);

    vec3 color_final = fog_linear(color_composite, sky_light,
            distance / (float(render_distance) * (1.0 - FOG_SOFTNESS / 1.5)),
            render_distance - render_distance * FOG_SOFTNESS,
            render_distance + render_distance * FOG_SOFTNESS);

    color_final = reinhard_tone_mapping(color_final, WHITE_POINT);

    float mix_factor = floor(albedo.a);
    g_pos = mix(g_pos, pos_view, mix_factor);
    g_normal_depth = mix(g_normal_depth, vec4(normalize(normal_view), 1.0), mix_factor);
    g_albedo_specular = vec4(color_final * albedo.a * opacity, albedo.a * opacity);
}
