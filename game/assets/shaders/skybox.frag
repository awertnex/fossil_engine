#version 430 core

uniform sampler2D texture_sky;
uniform sampler2D texture_horizon;
uniform sampler2D texture_stars;
uniform sampler2D texture_sun;
uniform int render_layer;
uniform vec3 sky_color;
uniform vec3 horizon_color;
in vec2 tex_coords;
out vec4 color;
vec4 sun;

#define USE_TONE_MAPPING
#include "h/defaults.glsl"

void main()
{
    float sky_brightness = (sky_color.r + sky_color.g + sky_color.b +
            horizon_color.r + horizon_color.g + horizon_color.b) / 6.0;
    switch (render_layer)
    {
        case 0:
            float sky_val = texture(texture_sky, tex_coords).r;
            float horizon_val = texture(texture_horizon, tex_coords).r;
            vec4 stars = texture(texture_stars, tex_coords);

            color = vec4(sky_color * (1.0 - horizon_val), 1.0) +
                vec4(horizon_color * horizon_val, 1.0);
            color.rgb *= sky_val;
            color += stars * (1.0 - clamp(sky_val * sky_brightness, 0.0, 1.0));
            break;

        case 1:
            sun = texture(texture_sun, tex_coords);
            color = vec4(sun.rgb * sun.a, sun.a);
            break;

        case 2:
            sun = texture(texture_sun, tex_coords);
            color = vec4(sun.rgb * sun.a, sun.a) * (1.0 - sky_brightness);
            break;
    }
}
