#define SKY_INFLUENCE 1.0
#define SUN_INFLUENCE 3.0
#define GLOBAL_ILLUMINATION 0.15
#define FLASHLIGHT_INTENSITY 5.0
#define FLASHLIGHT_DISTANCE 3.0
#define FLASHLIGHT_COLOR vec3(1.0, 0.9, 0.7)
#define FOG_SOFTNESS 1.0
#define WHITE_POINT 30.0

#define EPSILON 1e-3

uniform vec3 sun_rotation;
uniform vec3 sky_color;
uniform vec3 horizon_color;

#ifdef USE_SUN_DIRECTION

float sun_direction = clamp(dot(normal, sun_rotation), 0.0, SUN_INFLUENCE);

#endif /* USE_SUN_DIRECTION */

#ifdef USE_MATH

float square_length(vec3 v)
{
    return v.x * v.x + v.y * v.y + v.z * v.z;
}

#endif /* USE_MATH */

#ifdef USE_FOG

float fog_linear(float distance, float min, float max)
{
    return 1.0 - clamp((max - distance) / (max - min), 0.0, 1.0);
}

#endif /* USE_FOG */

#ifdef USE_TONE_MAPPING

/*! @param w = white point.
 */
vec3 reinhard_tone_mapping(vec3 color, float w)
{
    color = (color * (1.0 + color / (w * w))) / (1.0 + color);
    return pow(color, vec3(2.0));
}

#endif /* USE_TONE_MAPPING */
