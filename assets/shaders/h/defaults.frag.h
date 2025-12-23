#define SKY_INFLUENCE 1.0
#define SUN_INFLUENCE 3.0
#define AMBIENT_LIGHT_INTENSITY 0.11
#define FLASHLIGHT_INTENSITY 5.0
#define FLASHLIGHT_DISTANCE 3.0
#define FLASHLIGHT_COLOR vec3(1.0, 0.9, 0.7)
#define FOG_SOFTNESS 1.0

uniform vec3 sun_rotation;
uniform vec3 sky_color;

float sun_direction = clamp(dot(normal, sun_rotation), 0.0, SUN_INFLUENCE);
