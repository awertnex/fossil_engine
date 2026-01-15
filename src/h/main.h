#ifndef GAME_H
#define GAME_H

#include <engine/h/core.h>
#include <engine/h/diagnostics.h>
#include <engine/h/types.h>

#include "common.h"

struct Core
{
    struct /* flag */
    {
        u64 active: 1;
        u64 paused: 1;
        u64 hud: 1;
        u64 debug: 1;
        u64 super_debug: 1;
        u64 fullscreen: 1;
        u64 menu_open: 1;
        u64 fps_cap: 1;
        u64 parse_target: 1;
        u64 world_loaded: 1;
        u64 chunk_buf_dirty: 1;
    } flag;
}; /* Core */

struct Settings
{
    /* ---- internal -------------------------------------------------------- */

    /*! @brief conversion from world-space to screen-space.
     *
     *  @remark read-only, updated internally in 'main.c/settings_update()'.
     */
    v2f32 ndc_scale;

    u32 fps;
    f32 lerp_speed;
    u32 chunk_buf_radius;
    u32 chunk_buf_diameter;
    u32 chunk_buf_layer;
    u32 chunk_buf_volume;
    u32 chunk_tab_center;

    f64 reach_distance; /* player reach (arm length) */

    /* ---- controls -------------------------------------------------------- */

    f32 mouse_sensitivity;

    /* ---- video ----------------------------------------------------------- */

    u32 gui_scale;
    f32 font_size;
    u64 target_fps; /* in nanoseconds */

    /* ---- graphics -------------------------------------------------------- */

    f32 fov;
    u32 render_distance;
    b8 anti_aliasing;
}; /* Settings */

struct Uniform
{
    struct /* defaults */
    {
        GLint offset;
        GLint scale;
        GLint mat_rotation;
        GLint mat_perspective;
        GLint sun_rotation;
        GLint sky_color;
    } defaults;

    struct /* skybox */
    {
        GLint texture_scale;
        GLint mat_translation;
        GLint mat_rotation;
        GLint mat_sun_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint texture_sky;
        GLint texture_horizon;
        GLint texture_stars;
        GLint texture_sun;
        GLint sun_rotation;
        GLint sky_color;
        GLint horizon_color;
        GLint render_layer;
    } skybox;

    struct /* gizmo */
    {
        GLint ndc_scale;
        GLint mat_translation;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint color;
    } gizmo;

    struct /* gizmo_chunk */
    {
        GLint gizmo_offset;
        GLint render_size;
        GLint chunk_buf_diameter;
        GLint mat_translation;
        GLint mat_rotation;
        GLint mat_orientation;
        GLint mat_projection;
        GLint camera_position;
        GLint time;
    } gizmo_chunk;

    struct /* post_processing */
    {
        GLint time;
    } post_processing;

    struct /* voxel */
    {
        GLint mat_perspective;
        GLint camera_position;
        GLint sun_rotation;
        GLint sky_light;
        GLint moon_light;
        GLint chunk_position;
        GLint color;
        GLint opacity;
        GLint flashlight_position;
        GLint toggle_flashlight;
        GLint render_distance;
    } voxel;

    struct /* bounding_box */
    {
        GLint mat_perspective;
        GLint position;
        GLint size;
        GLint color;
    } bounding_box;

}; /* Uniform */

/*! @brief global pointer to variable for game/engine-specific error codes.
 *
 *  @remark must be initialized globally, tho the pointed to variable itself can be modified.
 */
extern u32 *const GAME_ERR;

extern struct Core core;
extern struct Settings settings;
extern u8 debug_mode[DEBUG_MODE_COUNT];
extern Projection projection_world;
extern Projection projection_hud;
extern Font *font[FONT_COUNT];

#endif /* GAME_MAIN_H */
