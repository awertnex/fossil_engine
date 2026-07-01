#ifndef HHC_SETTINGS_H
#define HHC_SETTINGS_H

#include "deps/fossil/common/types.h"

typedef struct hhc_settings
{
    /* ---- internal -------------------------------------------------------- */

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

    f32 gui_scale;
    f32 font_size;
    u64 target_fps; /* in nanoseconds */

    /* ---- graphics -------------------------------------------------------- */

    f32 fov;
    u32 render_distance;
    b8 anti_aliasing;

    struct /* flag */
    {
        b8 render_distance_dirty;
        b8 gui_scale_dirty;
    } flag;

} hhc_settings;

extern hhc_settings settings;

/*!
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 settings_init(void);

void settings_gui_scale_set(f32 scale);
void settings_render_distance_set(u32 distance);

#endif /* HHC_SETTINGS_H */
