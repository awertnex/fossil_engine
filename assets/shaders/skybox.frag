#version 430 core

uniform sampler2D texture_sky;
uniform sampler2D texture_horizon;
uniform sampler2D texture_stars;
uniform vec3 sky_color;
in vec2 tex_coords;
out vec4 color;

void main()
{
    vec4 sky = texture(texture_sky, tex_coords);
    vec4 horizon = texture(texture_horizon, tex_coords);
    vec4 stars = texture(texture_stars, tex_coords);
    float alpha = (horizon.r + horizon.g + horizon.b) / 3.0;

    horizon.g *= 0.5;
    horizon.b *= 0.3;

    color = vec4(sky_color, 1.0) * (sky + horizon) + stars * (1.0 - clamp(sky + horizon, 0.0, 1.0));
    color.rgb *= sky.a;
    color.a = sky.a;
}
