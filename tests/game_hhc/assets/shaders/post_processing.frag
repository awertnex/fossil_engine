#version 430 core

#define ABERRATION_NARROWNESS 0.003
#define ABERRATION_INTENSITY 0.2

#define VIGNETTE_NARROWNESS 0.6
#define VIGNETTE_INTENSITY 0.2
#define VIGNETTE_RICHNESS 2.0

#define COLOR_RICHNESS 0.6
#define SATURATION 1.0

#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923

uniform sampler2D texture_skybox;
uniform sampler2D texture_world_pos;
uniform sampler2D texture_world_normal;
uniform sampler2D texture_world_albedo_specular;
uniform sampler2D texture_hud;
uniform uint time;
uniform mat4 mat_projection;
uniform float camera_far;
uniform float camera_near;
uniform vec3 ssao_sample[64];
in vec2 vs_pos;
in vec2 vs_uv;
out vec4 color;
const float kernel_scale = 1.0 / 64.0;

/* ---- settings ------------------------------------------------------------ */

float setting_grain_intensity = 0.2;

/* ---- implementation ------------------------------------------------------ */

vec3 saturation_get(vec3 color_src, float saturation)
{
    vec3 color_monochrome = vec3((color_src.r + color_src.g + color_src.b) / 3.0);
    return mix(color_monochrome, color_src, saturation);
}

vec4 aberration_get(sampler2D sampler, vec2 frag_pos, vec2 frag_uv,
        float narrowness, float intensity)
{
    float length = length(frag_pos);
    vec2 aberration = frag_pos * narrowness * length * length * length * intensity;
    vec4 final = texture(sampler, frag_uv);
    final.r = texture(sampler, frag_uv + aberration).r;
    final.b = texture(sampler, frag_uv - aberration).b;
    return final;
}

vec3 vignette_get(vec2 frag_pos, vec3 color_src, float narrowness, float intensity, float richness)
{
    float vignette = length(frag_pos) * narrowness;
    vignette = vignette * vignette * vignette;
    vignette = clamp(vignette, 0.0, 1.0) * intensity;
    vec3 color_rich = pow(color_src, vec3(richness));
    return mix(color_src, color_rich, vignette) - vignette;
}

vec3 color_enrich(vec3 color_src, float richness)
{
    vec3 color_rich = color_src * color_src * color_src * (color_src * (color_src * 6.0 - 15.0) + 10.0);
    return mix(color_src, color_rich, richness);
}

vec3 grain_get(vec2 frag_pos, uint seed, float intensity, float chroma)
{
    vec2 grain_pos = frag_pos * 0.5 + 0.5;
    float grain_size = 0.1;
    float base = fract(dot(grain_pos * seed * grain_size, vec2(-grain_pos.y, 78.233)));
    float drift = 43758.5453324;
    vec3 grain = vec3(
            sin(base),
            sin(base + HALF_PI),
            sin(base + PI)) * drift;
    grain = fract(grain) * 2.0 - 1.0;
    return mix(vec3(grain.r), grain, chroma) * intensity;
}

float depth_linearize(float depth, float far, float near)
{
    depth = depth * 2.0 - 1.0;
    return (2.0 * near * far) / (far + near - depth * (far - near));
}

float ambient_occlusion_get(sampler2D sampler_pos, sampler2D sampler_normal,
        vec2 frag_pos, vec2 frag_uv, float intensity)
{
    vec4 pos_vec = texture(sampler_pos, frag_uv);
    vec3 normal_vec = texture(sampler_normal, frag_uv).xyz;
    vec3 random_vec = grain_get(frag_pos, time, 1.0, 1.0);
    vec4 depth_vec = vec4(0.0);

    vec3 tangent = normalize(random_vec - normal_vec * dot(random_vec, normal_vec));
    vec3 bitangent = normalize(cross(normal_vec, tangent));
    mat3 tbn = mat3(tangent, bitangent, normal_vec);

    vec3 kernel_sample = vec3(0.0);
    float radius = 2.0;
    float bias = 0.025;
    vec4 offset = vec4(0.0);
    float depth_a = mix(camera_far, pos_vec.z, pos_vec.w);
    float depth_b = 0.0;
    float range_check = 0.0;
    float accumulate = 0.0;
    int i = 0;

    for(; i < 64; ++i)
    {
        kernel_sample = pos_vec.xyz + tbn * ssao_sample[i] * radius;
        offset = mat_projection * vec4(kernel_sample, 1.0);
        offset.xy /= offset.w;
        offset.xy = offset.xy * 0.5 + 0.5;
        depth_vec = texture(sampler_pos, offset.xy);
        depth_b = mix(camera_far, depth_vec.z, depth_vec.w);

        range_check = smoothstep(0.0, 1.0, radius / length(depth_a - depth_b));
        accumulate += (depth_b >= kernel_sample.z + bias ? kernel_scale : 0.0) * range_check;
    }

    return 1.0 - clamp(accumulate * intensity, 0.0, 1.0);
}

void main()
{
    /* ---- base ------------------------------------------------------------ */

    vec4 color_skybox = texture(texture_skybox, vs_uv);
    vec4 color_albedo = aberration_get(texture_world_albedo_specular, vs_pos, vs_uv,
            ABERRATION_NARROWNESS, ABERRATION_INTENSITY);
    vec4 color_ui = texture(texture_hud, vs_uv);

    /* ---- effects --------------------------------------------------------- */

    vec3 grain = grain_get(vs_pos, time, setting_grain_intensity, 1.0);
    float ambient_occlusion = ambient_occlusion_get(texture_world_pos, texture_world_normal, vs_pos, vs_uv, 0.4);

    /* ---- final ----------------------------------------------------------- */

    color.rgb = color_albedo.rgb * ambient_occlusion * color_albedo.a;
    color = mix(color_skybox, vec4(color.rgb, 1.0), color_albedo.a);

    color.rgb = vignette_get(vs_pos, color.rgb, VIGNETTE_NARROWNESS, VIGNETTE_INTENSITY, VIGNETTE_RICHNESS);
    color.rgb = color_enrich(color.rgb, COLOR_RICHNESS);

    color.rgb = mix(color.rgb, color_ui.rgb, color_ui.a);

    color.rgb *= 1.0 + grain;

    color.rgb = saturation_get(color.rgb, SATURATION);
}
