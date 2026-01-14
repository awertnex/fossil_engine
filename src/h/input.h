#ifndef GAME_INPUT_H
#define GAME_INPUT_H

#include <engine/h/core.h>
#include <engine/h/types.h>

#include "main.h"
#include "player.h"

/* ---- movement ------------------------------------------------------------ */

extern u32 bind_walk_forward;
extern u32 bind_walk_backward;
extern u32 bind_strafe_left;
extern u32 bind_strafe_right;
extern u32 bind_jump;
extern u32 bind_sprint;
extern u32 bind_sneak;

/* ---- gameplay ------------------------------------------------------------ */

extern u32 bind_attack_or_destroy;
extern u32 bind_sample_block;
extern u32 bind_build_or_use;

/* ---- inventory ----------------------------------------------------------- */

/* extern u32 bind_drop_item; */

/* two arrays for number keys and numberpad keys */
extern u32 bind_hotbar[2][PLAYER_HOTBAR_SLOTS_MAX];

extern u32 bind_inventory;

/* ---- misc ---------------------------------------------------------------- */

extern u32 bind_toggle_hud;
/* extern u32 bind_take_screenshot; */
extern u32 bind_toggle_debug;
extern u32 bind_toggle_cinematic_motion;
extern u32 bind_toggle_perspective;
/* extern u32 bind_toggle_fullscreen; */
extern u32 bind_zoom;
extern u32 bind_toggle_flashlight;
/* extern u32 bind_pause; */
/* extern u32 bind_chat_or_command; */

/* ---- debug --------------------------------------------------------------- */

/* TODO: navigate menus with arrow keys */
/* extern u32 bind_left; */
/* extern u32 bind_right; */
/* extern u32 bind_down; */
/* extern u32 bind_up; */
extern u32 bind_debug_mod;
extern u32 bind_toggle_super_debug;
extern u32 bind_toggle_trans_blocks;
extern u32 bind_toggle_chunk_bounds;
extern u32 bind_toggle_bounding_boxes;
extern u32 bind_toggle_chunk_gizmo;
extern u32 bind_toggle_chunk_queue_visualizer;

/*! @brief update input from keyboard.
 *
 *  handle all key binds listed above and perform their logic on mostly 'p->flag'.
 *
 *  @remark only updates player flags, no parameters are updated except for
 *  'p->input'.
 */
void input_update(Player *p);

#endif /* GAME_INPUT_H */
