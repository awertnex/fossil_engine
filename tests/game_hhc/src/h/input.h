#ifndef HHC_INPUT_H
#define HHC_INPUT_H

#include "main.h"
#include "player.h"

#include "src/h/core.h"
#include "src/h/input.h"
#include "src/h/types.h"

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
extern fsl_key_bind bind_sample_block;
extern fsl_key_bind bind_build_or_use;

/* ---- inventory ----------------------------------------------------------- */

/* extern fsl_key_bind bind_drop_item; */
extern fsl_key_bind bind_inventory;

/*! @remark two arrays for number keys and numberpad keys */
extern fsl_key_bind bind_hotbar[2][PLAYER_HOTBAR_SLOTS_MAX];

/* ---- misc ---------------------------------------------------------------- */

extern fsl_key_bind bind_toggle_hud;
extern fsl_key_bind bind_take_screenshot;
extern fsl_key_bind bind_toggle_debug;
extern fsl_key_bind bind_toggle_cinematic_motion;
extern fsl_key_bind bind_toggle_perspective;
/* extern fsl_key_bind bind_toggle_fullscreen; */
extern fsl_key_bind bind_zoom;
extern fsl_key_bind bind_toggle_flashlight;
/* extern fsl_key_bind bind_pause; */
/* extern fsl_key_bind bind_chat_or_command; */
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
extern fsl_key_bind bind_toggle_trans_blocks;
extern fsl_key_bind bind_toggle_chunk_bounds;
extern fsl_key_bind bind_toggle_bounding_boxes;
extern fsl_key_bind bind_toggle_chunk_gizmo;
extern fsl_key_bind bind_toggle_chunk_queue_visualizer;

/*! @brief setup all key binding values and rules.
 */
void input_init(void);

/*! @brief update input from keyboard.
 *
 *  handle all key binds listed above and perform their logic on mostly `p->flag`.
 *
 *  @remark only updates player flags, no parameters are updated except for
 *  `p->input`.
 */
void input_update(player *p);

#endif /* HHC_INPUT_H */
