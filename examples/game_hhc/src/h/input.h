#ifndef HHC_INPUT_H
#define HHC_INPUT_H

#include "deps/fossil/common/types.h"

#include "deps/fossil/input/input.h"

#include "container.h"
#include "player.h"

/* ---- movement ------------------------------------------------------------ */

extern fsl_key_bind bind_walk_forward;
extern fsl_key_bind bind_walk_backward;
extern fsl_key_bind bind_strafe_left;
extern fsl_key_bind bind_strafe_right;
extern fsl_key_bind bind_jump;
extern fsl_key_bind bind_sprint;
extern fsl_key_bind bind_sneak;

/* ---- gameplay ------------------------------------------------------------ */

extern fsl_key_bind bind_attack_or_destroy;
extern fsl_key_bind bind_build_or_use;
extern fsl_key_bind bind_sample_block;
/* extern fsl_key_bind bind_drop_item; */

/*!
 *  @remark two arrays for number keys and numberpad keys.
 */
extern fsl_key_bind bind_hotbar[2][CONTAINER_HOTBAR_SLOTS_MAX];

extern fsl_key_bind bind_inventory;

/* ---- misc ---------------------------------------------------------------- */

extern fsl_key_bind bind_pause;
extern fsl_key_bind bind_command_line;
extern fsl_key_bind bind_toggle_hud;
extern fsl_key_bind bind_take_screenshot;
extern fsl_key_bind bind_toggle_debug;
extern fsl_key_bind bind_toggle_cinematic_motion;
extern fsl_key_bind bind_toggle_perspective;
/* extern fsl_key_bind bind_toggle_fullscreen; */
extern fsl_key_bind bind_zoom;
extern fsl_key_bind bind_toggle_flashlight;
extern fsl_key_bind bind_reload_shaders;

/* ---- debug --------------------------------------------------------------- */

/* TODO: navigate menus with arrow keys.
 */
/* extern fsl_key_bind bind_left; */
/* extern fsl_key_bind bind_right; */
/* extern fsl_key_bind bind_down; */
/* extern fsl_key_bind bind_up; */

extern u32 bind_debug_mod;
extern fsl_key_bind bind_toggle_super_debug;

/*!
 *  @brief setup all key binding values and rules.
 */
void input_init(void);

/*!
 *  @brief update input from keyboard.
 *
 *  handle all key binds listed above and perform their logic on mostly `p->flag`.
 *
 *  @remark only updates player flags, no parameters are updated except for
 *  `p->input`.
 */
void input_update(hhc_player *p);

#endif /* HHC_INPUT_H */
