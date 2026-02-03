#include "h/assets.h"
#include "h/chunking.h"
#include "h/gui.h"
#include "h/input.h"
#include "h/logger.h"
#include "h/player.h"
#include "h/world.h"

#include "src/h/input.h"
#include "src/h/math.h"
#include "src/h/time.h"

#include <math.h>

/* ---- movement ------------------------------------------------------------ */

u32 bind_walk_forward =             FSL_KEY_W;
u32 bind_walk_backward =            FSL_KEY_S;
u32 bind_strafe_left =              FSL_KEY_A;
u32 bind_strafe_right =             FSL_KEY_D;
u32 bind_jump =                     FSL_KEY_SPACE;
u32 bind_sprint =                   FSL_KEY_LEFT_SHIFT;
u32 bind_sneak =                    FSL_KEY_LEFT_CONTROL;

/* ---- gameplay ------------------------------------------------------------ */

u32 bind_attack_or_destroy =        GLFW_MOUSE_BUTTON_LEFT;
u32 bind_sample_block =             GLFW_MOUSE_BUTTON_MIDDLE;
u32 bind_build_or_use =             GLFW_MOUSE_BUTTON_RIGHT;

/* ---- inventory ----------------------------------------------------------- */

u32 bind_selected_item =            FSL_KEY_Q;
u32 bind_hotbar[2][PLAYER_HOTBAR_SLOTS_MAX] =
{
    {
        FSL_KEY_1, FSL_KEY_2, FSL_KEY_3, FSL_KEY_4, FSL_KEY_5,
        FSL_KEY_6, FSL_KEY_7, FSL_KEY_8, FSL_KEY_9, FSL_KEY_0,
    },
    {
        FSL_KEY_KP_1, FSL_KEY_KP_2, FSL_KEY_KP_3, FSL_KEY_KP_4, FSL_KEY_KP_5,
        FSL_KEY_KP_6, FSL_KEY_KP_7, FSL_KEY_KP_8, FSL_KEY_KP_9, FSL_KEY_KP_0,
    },
};
u32 bind_inventory =                FSL_KEY_E;

/* ---- miscellaneous ------------------------------------------------------- */

u32 bind_toggle_hud =               FSL_KEY_F1;
u32 bind_take_screenshot =          FSL_KEY_F2;
u32 bind_toggle_debug =             FSL_KEY_F3;
u32 bind_toggle_cinematic_camera =  FSL_KEY_F4;
u32 bind_toggle_perspective =       FSL_KEY_F5;
u32 bind_toggle_cinematic_motion =  FSL_KEY_F6;
u32 bind_toggle_fullscreen =        FSL_KEY_F11;
u32 bind_zoom =                     FSL_KEY_Z;
u32 bind_toggle_flashlight =        FSL_KEY_F;
u32 bind_pause =                    FSL_KEY_ESCAPE;
u32 bind_chat_or_command =          FSL_KEY_SLASH;

/* ---- debug & menu -------------------------------------------------------- */

/* TODO: navigate menus with arrow keys */
u32 bind_left =                     FSL_KEY_LEFT;
u32 bind_right =                    FSL_KEY_RIGHT;
u32 bind_down =                     FSL_KEY_DOWN;
u32 bind_up =                       FSL_KEY_UP;
u32 bind_debug_mod =                FSL_KEY_LEFT_ALT;
u32 bind_toggle_super_debug =       FSL_KEY_TAB;
u32 bind_toggle_trans_blocks =      FSL_KEY_T;
u32 bind_toggle_chunk_bounds =      FSL_KEY_C;
u32 bind_toggle_bounding_boxes =    FSL_KEY_B;
u32 bind_toggle_chunk_gizmo =       FSL_KEY_G;
u32 bind_toggle_chunk_queue_visualizer = FSL_KEY_V;

void input_update(player *p)
{
    u32 i;
    f32 px = 0.0f, nx = 0.0f,
        py = 0.0f, ny = 0.0f,
        pz = 0.0f, nz = 0.0f,
        spch = sin(p->pitch * FSL_DEG2RAD),
        cpch = cos(p->pitch * FSL_DEG2RAD),
        syaw = sin(p->yaw * FSL_DEG2RAD),
        cyaw = cos(p->yaw * FSL_DEG2RAD);

    p->input = (v3f32){0};

    if (!(p->flag & FLAG_PLAYER_DEAD))
    {
        /* ---- movement ---------------------------------------------------- */

        px += (f32)fsl_is_key_hold(bind_walk_forward);
        nx += (f32)fsl_is_key_hold(bind_walk_backward);
        py += (f32)fsl_is_key_hold(bind_strafe_left);
        ny += (f32)fsl_is_key_hold(bind_strafe_right);

        if (fsl_is_key_press_double(bind_walk_forward))
            p->flag |= FLAG_PLAYER_SPRINTING;

        /* ---- jumping ----------------------------------------------------- */

        if (fsl_is_key_hold(bind_jump))
        {
            if (p->flag & FLAG_PLAYER_FLYING)
                pz += 1.0f;
            else if (p->flag & FLAG_PLAYER_CAN_JUMP)
            {
                p->velocity.z += sqrtf(2.0f * world.gravity * PLAYER_JUMP_HEIGHT);
                p->flag &= ~FLAG_PLAYER_CAN_JUMP;
            }
        }

        if (fsl_is_key_press_double(bind_jump))
            p->flag ^= FLAG_PLAYER_FLYING;

        /* ---- sprinting --------------------------------------------------- */

        if (fsl_is_key_hold(bind_sprint) && fsl_is_key_hold(bind_walk_forward))
            p->flag |= FLAG_PLAYER_SPRINTING;
        else if (fsl_is_key_release(bind_walk_forward))
            p->flag &= ~FLAG_PLAYER_SPRINTING;

        /* ---- sneaking ---------------------------------------------------- */

        if (fsl_is_key_hold(bind_sneak))
        {
            if (p->flag & FLAG_PLAYER_FLYING)
                nz += 1.0f;
            else p->flag |= FLAG_PLAYER_SNEAKING;
        }
        else p->flag &= ~FLAG_PLAYER_SNEAKING;

        /* ---- apply input ------------------------------------------------- */

        if (p->flag & FLAG_PLAYER_FLYING && p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        {
            p->input.x =
                (px - nx) * cyaw * cpch +
                (py - ny) * -cos(p->yaw * FSL_DEG2RAD + FSL_PI / 2.0) +
                (pz - nz) * cyaw * spch;
            p->input.y =
                (px - nx) * -syaw * cpch +
                (py - ny) * sin(p->yaw * FSL_DEG2RAD + FSL_PI / 2.0) +
                (pz - nz) * -syaw * spch;
            p->input.z =
                (px - nx) * -spch +
                (pz - nz) * cpch;
        }
        else
        {
            p->input.x =
                (px - nx) * cyaw +
                (py - ny) * -cos(p->yaw * FSL_DEG2RAD + FSL_PI / 2.0);
            p->input.y =
                (px - nx) * -syaw +
                (py - ny) * sin(p->yaw * FSL_DEG2RAD + FSL_PI / 2.0);
            p->input.z =
                pz - nz;
        }

        p->input = fsl_normalize_v3f32(p->input);

        /* ---- gameplay ---------------------------------------------------- */

        if (
                !core.flag.chunk_buf_dirty &&
                core.flag.parse_target &&
                chunk_tab[chunk_tab_index])
        {
            if (fsl_is_mouse_hold(bind_attack_or_destroy))
            {
                block_break(chunk_tab_index,
                        (i64)p->target.x - chunk_tab[chunk_tab_index]->pos.x * CHUNK_DIAMETER,
                        (i64)p->target.y - chunk_tab[chunk_tab_index]->pos.y * CHUNK_DIAMETER,
                        (i64)p->target.z - chunk_tab[chunk_tab_index]->pos.z * CHUNK_DIAMETER);
            }
            if (fsl_is_mouse_press(bind_build_or_use))
            {
                block_place(chunk_tab_index,
                        (i64)p->target.x - chunk_tab[chunk_tab_index]->pos.x * CHUNK_DIAMETER,
                        (i64)p->target.y - chunk_tab[chunk_tab_index]->pos.y * CHUNK_DIAMETER,
                        (i64)p->target.z - chunk_tab[chunk_tab_index]->pos.z * CHUNK_DIAMETER,
                        p->target_normal, p->hotbar_slots[p->hotbar_slot_selected]);
            }

            if (fsl_is_key_press(bind_sample_block)) {}
        }

        /* ---- inventory --------------------------------------------------- */

        for (i = 0; i < PLAYER_HOTBAR_SLOTS_MAX; ++i)
            if (fsl_is_key_press(bind_hotbar[0][i]) || fsl_is_key_press(bind_hotbar[1][i]))
                p->hotbar_slot_selected = i;

        if (fsl_is_key_press(bind_inventory))
        {
            if ((p->menu_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && state_menu_depth)
            {
                state_menu_depth = 0;
                p->menu_state &= ~STATE_PLAYER_MENU_INVENTORY_SURVIVAL;
            }
            else if (!(p->menu_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && !state_menu_depth)
            {
                state_menu_depth = 1;
                p->menu_state |= STATE_PLAYER_MENU_INVENTORY_SURVIVAL;
            }

            if (!(p->menu_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && state_menu_depth)
                --state_menu_depth;
        }

        /* ---- miscellaneous ----------------------------------------------- */

        if (fsl_is_key_press(bind_toggle_hud))
            core.flag.hud ^= 1;
        if (fsl_is_key_press(bind_take_screenshot))
            fsl_request_screenshot();

        if (fsl_is_key_press(bind_toggle_debug))
            core.flag.debug ^= 1;

        if (fsl_is_key_press(bind_toggle_perspective))
            p->camera_mode = (p->camera_mode + 1) % PLAYER_CAMERA_MODE_COUNT;

        if (fsl_is_key_press(bind_zoom))
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "%s\n", "Zoom Toggled On");
        if (fsl_is_key_hold(bind_zoom))
            p->flag |= FLAG_PLAYER_ZOOMER;
        if (fsl_is_key_release(bind_zoom))
        {
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "%s\n", "Zoom Toggled Off");
            p->flag &= ~FLAG_PLAYER_ZOOMER;
        }

        if (fsl_is_key_press(bind_toggle_flashlight))
        {
            p->flag ^= FLAG_PLAYER_FLASHLIGHT;

            if (p->flag & FLAG_PLAYER_FLASHLIGHT)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "Flashlight Toggled On");
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "Flashlight Toggled Off");
        }

        if (fsl_is_key_press(bind_toggle_cinematic_motion))
        {
            p->flag ^= FLAG_PLAYER_CINEMATIC_MOTION;

            if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "Cinematic Motion On");
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "Cinematic Motion Off");
        }
    }

    /* ---- debug ----------------------------------------------------------- */

#if !GAME_RELEASE_BUILD
    if (fsl_is_key_press(bind_toggle_super_debug))
        core.flag.super_debug ^= 1;
#endif /* GAME_RELEASE_BUILD */

    if (fsl_is_key_hold(bind_debug_mod))
    {
        if (fsl_is_key_press(bind_toggle_trans_blocks))
        {
            core.debug.trans_blocks ^= 1;

            if (core.debug.trans_blocks)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "View Transparent Blocks On");
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "View Transparent Blocks Off");
        }

        if (fsl_is_key_press(bind_toggle_chunk_bounds))
        {
            core.debug.chunk_bounds ^= 1;

            if (core.debug.chunk_bounds)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "View Chunk Boundaries On");
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "View Chunk Boundaries Off");
        }

        if (fsl_is_key_press(bind_toggle_bounding_boxes))
        {
            core.debug.bounding_boxes ^= 1;

            if (core.debug.bounding_boxes)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "View Bounding Boxes On");
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "View Bounding Boxes Off");
        }

        if (fsl_is_key_press(bind_toggle_chunk_gizmo))
            core.debug.chunk_gizmo ^= 1;

        if (fsl_is_key_press(bind_toggle_chunk_queue_visualizer))
        {
            core.debug.chunk_queue_visualizer ^= 1;

            if (core.debug.chunk_queue_visualizer)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "View Chunk Queue Visualizer On");
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "%s\n", "View Chunk Queue Visualizer Off");
        }
    }
}
