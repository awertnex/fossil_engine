#ifndef HHC_H
#define HHC_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/engine/engine.h"

struct hhc_core
{
    struct /* flag */
    {
        b8 world_load;
    } request;

    struct /* flag */
    {
        b8 paused;
        b8 hud;
        b8 debug;
        b8 super_debug;
        b8 fullscreen;
        b8 menu_open;
        b8 fps_cap;
        b8 world_loaded;
        b8 chunks_initialized;
    } flag;

    struct /* debug */
    {
        b8 trans_blocks;
        b8 bounding_boxes;
        b8 chunk_bounds;
        b8 chunk_gizmo;
        b8 chunk_scheduler_visualizer;
    } debug;
}; /* hhc_core */

struct hhc_uniform
{
    struct /* defaults */
    {
        GLint location;
        GLint scale;
        GLint mat_rotation;
        GLint mat_perspective;
        GLint sun_rotation;
        GLint sky_color;
    } defaults;

    struct /* skybox */
    {
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

}; /* hhc_uniform */

extern fsl_render *render;
extern struct hhc_core core;
extern struct hhc_uniform uniform;

#endif /* HHC_MAIN_H */
