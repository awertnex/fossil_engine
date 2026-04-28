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

fsl_key_bind bind_walk_forward = {0};
fsl_key_bind bind_walk_backward = {0};
fsl_key_bind bind_strafe_left = {0};
fsl_key_bind bind_strafe_right = {0};
fsl_key_bind bind_jump = {0};
fsl_key_bind bind_sprint = {0};
fsl_key_bind bind_sneak = {0};

/* ---- gameplay ------------------------------------------------------------ */

fsl_key_bind bind_attack_or_destroy = {0};
fsl_key_bind bind_sample_block = {0};
fsl_key_bind bind_build_or_use = {0};

/* ---- inventory ----------------------------------------------------------- */

fsl_key_bind bind_drop_item = {0};
fsl_key_bind bind_inventory = {0};
fsl_key_bind bind_hotbar[2][PLAYER_HOTBAR_SLOTS_MAX] = {0};

/* ---- miscellaneous ------------------------------------------------------- */

fsl_key_bind bind_toggle_hud = {0};
fsl_key_bind bind_take_screenshot = {0};
fsl_key_bind bind_toggle_debug = {0};
fsl_key_bind bind_toggle_cinematic_camera = {0};
fsl_key_bind bind_toggle_perspective = {0};
fsl_key_bind bind_toggle_cinematic_motion = {0};
fsl_key_bind bind_toggle_fullscreen = {0};
fsl_key_bind bind_zoom = {0};
fsl_key_bind bind_toggle_flashlight = {0};
fsl_key_bind bind_pause = {0};
fsl_key_bind bind_chat_or_command = {0};

/* ---- debug & menu -------------------------------------------------------- */

/* TODO: navigate menus with arrow keys.
 */
fsl_key_bind bind_left = {0};
fsl_key_bind bind_right = {0};
fsl_key_bind bind_down = {0};
fsl_key_bind bind_up = {0};

u32 bind_debug_mod = FSL_ALT_LEFT;
fsl_key_bind bind_toggle_super_debug = {0};
fsl_key_bind bind_toggle_trans_blocks = {0};
fsl_key_bind bind_toggle_chunk_bounds = {0};
fsl_key_bind bind_toggle_bounding_boxes = {0};
fsl_key_bind bind_toggle_chunk_gizmo = {0};
fsl_key_bind bind_toggle_chunk_queue_visualizer = {0};

void input_init(void)
{
    /* ---- movement -------------------------------------------------------- */

    bind_walk_forward = fsl_key_bind_init(FSL_KEY_W, 0, 0, 0, 0);
    bind_walk_backward = fsl_key_bind_init(FSL_KEY_S, 0, 0, 0, 0);
    bind_strafe_left = fsl_key_bind_init(FSL_KEY_A, 0, 0, 0, 0);
    bind_strafe_right = fsl_key_bind_init(FSL_KEY_D, 0, 0, 0, 0);
    bind_jump = fsl_key_bind_init(FSL_KEY_SPACE, 0, 0, 0, 0);
    bind_sprint = fsl_key_bind_init(FSL_KEY_LEFT_SHIFT, 0, 0, 0, 0);
    bind_sneak = fsl_key_bind_init(FSL_KEY_LEFT_CONTROL, 0, 0, 0, 0);

    /* ---- gameplay -------------------------------------------------------- */

    bind_attack_or_destroy = fsl_key_bind_init(GLFW_MOUSE_BUTTON_LEFT, 0, 0, 0, 0);
    bind_sample_block = fsl_key_bind_init(GLFW_MOUSE_BUTTON_MIDDLE, 0, 0, 0, 0);
    bind_build_or_use = fsl_key_bind_init(GLFW_MOUSE_BUTTON_RIGHT, 0, 0, 0, 0);

    /* ---- inventory ------------------------------------------------------- */

    bind_drop_item = fsl_key_bind_init(FSL_KEY_Q, 0, 0, 0, 0);
    bind_inventory = fsl_key_bind_init(FSL_KEY_E, 0, 0, 0, 0);

    bind_hotbar[0][1] = fsl_key_bind_init(FSL_KEY_1, 0, 0, 0, 0);
    bind_hotbar[0][2] = fsl_key_bind_init(FSL_KEY_2, 0, 0, 0, 0);
    bind_hotbar[0][3] = fsl_key_bind_init(FSL_KEY_3, 0, 0, 0, 0);
    bind_hotbar[0][4] = fsl_key_bind_init(FSL_KEY_4, 0, 0, 0, 0);
    bind_hotbar[0][5] = fsl_key_bind_init(FSL_KEY_5, 0, 0, 0, 0);
    bind_hotbar[0][6] = fsl_key_bind_init(FSL_KEY_6, 0, 0, 0, 0);
    bind_hotbar[0][7] = fsl_key_bind_init(FSL_KEY_7, 0, 0, 0, 0);
    bind_hotbar[0][8] = fsl_key_bind_init(FSL_KEY_8, 0, 0, 0, 0);
    bind_hotbar[0][9] = fsl_key_bind_init(FSL_KEY_9, 0, 0, 0, 0);
    bind_hotbar[0][0] = fsl_key_bind_init(FSL_KEY_0, 0, 0, 0, 0);
    bind_hotbar[1][1] = fsl_key_bind_init(FSL_KEY_KP_1, 0, 0, 0, 0);
    bind_hotbar[1][2] = fsl_key_bind_init(FSL_KEY_KP_2, 0, 0, 0, 0);
    bind_hotbar[1][3] = fsl_key_bind_init(FSL_KEY_KP_3, 0, 0, 0, 0);
    bind_hotbar[1][4] = fsl_key_bind_init(FSL_KEY_KP_4, 0, 0, 0, 0);
    bind_hotbar[1][5] = fsl_key_bind_init(FSL_KEY_KP_5, 0, 0, 0, 0);
    bind_hotbar[1][6] = fsl_key_bind_init(FSL_KEY_KP_6, 0, 0, 0, 0);
    bind_hotbar[1][7] = fsl_key_bind_init(FSL_KEY_KP_7, 0, 0, 0, 0);
    bind_hotbar[1][8] = fsl_key_bind_init(FSL_KEY_KP_8, 0, 0, 0, 0);
    bind_hotbar[1][9] = fsl_key_bind_init(FSL_KEY_KP_9, 0, 0, 0, 0);
    bind_hotbar[1][0] = fsl_key_bind_init(FSL_KEY_KP_0, 0, 0, 0, 0);

    /* ---- miscellaneous --------------------------------------------------- */

    bind_toggle_hud = fsl_key_bind_init(FSL_KEY_F1, 0, 0, 0, 0);
    bind_take_screenshot = fsl_key_bind_init(FSL_KEY_F2, 0, 0, 0, 0);
    bind_toggle_debug = fsl_key_bind_init(FSL_KEY_F3, 0, 0, 0, 0);
    bind_toggle_cinematic_camera = fsl_key_bind_init(FSL_KEY_F4, 0, 0, 0, 0);
    bind_toggle_perspective = fsl_key_bind_init(FSL_KEY_F5, 0, 0, 0, 0);
    bind_toggle_cinematic_motion = fsl_key_bind_init(FSL_KEY_F6, 0, 0, 0, 0);
    bind_toggle_fullscreen = fsl_key_bind_init(FSL_KEY_F11, 0, 0, 0, 0);
    bind_zoom = fsl_key_bind_init(FSL_KEY_Z, 0, 0, 0, 0);
    bind_toggle_flashlight = fsl_key_bind_init(FSL_KEY_F, 0, 0, 0, 0);
    bind_pause = fsl_key_bind_init(FSL_KEY_ESCAPE, 0, 0, 0, 0);
    bind_chat_or_command = fsl_key_bind_init(FSL_KEY_SLASH, 0, 0, 0, 0);

    /* ---- debug & menu ---------------------------------------------------- */

    bind_left = fsl_key_bind_init(FSL_KEY_LEFT, 0, 0, 0, 0);
    bind_right = fsl_key_bind_init(FSL_KEY_RIGHT, 0, 0, 0, 0);
    bind_down = fsl_key_bind_init(FSL_KEY_DOWN, 0, 0, 0, 0);
    bind_up = fsl_key_bind_init(FSL_KEY_UP, 0, 0, 0, 0);
    bind_toggle_super_debug = fsl_key_bind_init(FSL_KEY_TAB, 0, 0, 0, 0);
    bind_toggle_trans_blocks = fsl_key_bind_init(FSL_KEY_T, 0, 0, bind_debug_mod, 0);
    bind_toggle_chunk_bounds = fsl_key_bind_init(FSL_KEY_C, 0, 0, bind_debug_mod, 0);
    bind_toggle_bounding_boxes = fsl_key_bind_init(FSL_KEY_B, 0, 0, bind_debug_mod, 0);
    bind_toggle_chunk_gizmo = fsl_key_bind_init(FSL_KEY_G, 0, 0, bind_debug_mod, 0);
    bind_toggle_chunk_queue_visualizer = fsl_key_bind_init(FSL_KEY_V, 0, 0, bind_debug_mod, 0);
}

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
                    fsl_logger_stringf("%s\n", "Zoom Toggled On"));
        if (fsl_is_key_hold(bind_zoom))
            p->flag |= FLAG_PLAYER_ZOOMER;
        if (fsl_is_key_release(bind_zoom))
        {
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "Zoom Toggled Off"));
            p->flag &= ~FLAG_PLAYER_ZOOMER;
        }

        if (fsl_is_key_press(bind_toggle_flashlight))
        {
            p->flag ^= FLAG_PLAYER_FLASHLIGHT;

            if (p->flag & FLAG_PLAYER_FLASHLIGHT)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Flashlight Toggled On"));
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Flashlight Toggled Off"));
        }

        if (fsl_is_key_press(bind_toggle_cinematic_motion))
        {
            p->flag ^= FLAG_PLAYER_CINEMATIC_MOTION;

            if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Cinematic Motion On"));
            else
                HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        fsl_logger_stringf("%s\n", "Cinematic Motion Off"));
        }
    }

    /* ---- debug ----------------------------------------------------------- */

#if !GAME_RELEASE_BUILD
    if (fsl_is_key_press(bind_toggle_super_debug))
        core.flag.super_debug ^= 1;
#endif /* GAME_RELEASE_BUILD */

    if (fsl_is_key_press(bind_toggle_trans_blocks))
    {
        core.debug.trans_blocks ^= 1;

        if (core.debug.trans_blocks)
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "View Transparent Blocks On"));
        else
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "View Transparent Blocks Off"));
    }

    if (fsl_is_key_press(bind_toggle_chunk_bounds))
    {
        core.debug.chunk_bounds ^= 1;

        if (core.debug.chunk_bounds)
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "View Chunk Boundaries On"));
        else
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "View Chunk Boundaries Off"));
    }

    if (fsl_is_key_press(bind_toggle_bounding_boxes))
    {
        core.debug.bounding_boxes ^= 1;

        if (core.debug.bounding_boxes)
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "View Bounding Boxes On"));
        else
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "View Bounding Boxes Off"));
    }

    if (fsl_is_key_press(bind_toggle_chunk_gizmo))
        core.debug.chunk_gizmo ^= 1;

    if (fsl_is_key_press(bind_toggle_chunk_queue_visualizer))
    {
        core.debug.chunk_queue_visualizer ^= 1;

        if (core.debug.chunk_queue_visualizer)
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "View Chunk Queue Visualizer On"));
        else
            HHC_LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    fsl_logger_stringf("%s\n", "View Chunk Queue Visualizer Off"));
    }
}
