#include "deps/fossil/common/diagnostics.h"
#include "deps/fossil/input/input.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shaders.h"
#include "deps/fossil/shaders/shader_types.h"

#include "chunking/chunking.h"
#include "gui/gui.h"

#include "h/main.h"
#include "h/assets.h"
#include "h/input.h"
#include "h/player.h"
#include "h/world.h"

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
fsl_key_bind bind_build_or_use = {0};
fsl_key_bind bind_sample_block = {0};
fsl_key_bind bind_drop_item = {0};
fsl_key_bind bind_hotbar[2][CONTAINER_HOTBAR_SLOTS_MAX] = {0};
fsl_key_bind bind_inventory = {0};

/* ---- miscellaneous ------------------------------------------------------- */

fsl_key_bind bind_pause = {0};
fsl_key_bind bind_command_line = {0};
fsl_key_bind bind_toggle_hud = {0};
fsl_key_bind bind_take_screenshot = {0};
fsl_key_bind bind_toggle_debug = {0};
fsl_key_bind bind_toggle_cinematic_motion = {0};
fsl_key_bind bind_toggle_perspective = {0};
fsl_key_bind bind_toggle_fullscreen = {0};
fsl_key_bind bind_zoom = {0};
fsl_key_bind bind_toggle_flashlight = {0};
fsl_key_bind bind_reload_shaders = {0};

/* ---- debug & menu -------------------------------------------------------- */

/* TODO: navigate menus with arrow keys.
 */
fsl_key_bind bind_left = {0};
fsl_key_bind bind_right = {0};
fsl_key_bind bind_down = {0};
fsl_key_bind bind_up = {0};

u32 bind_debug_mod = FSL_ALT_LEFT;
fsl_key_bind bind_toggle_super_debug = {0};

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
    bind_toggle_cinematic_motion = fsl_key_bind_init(FSL_KEY_F4, 0, 0, 0, 0);
    bind_toggle_perspective = fsl_key_bind_init(FSL_KEY_F5, 0, 0, 0, 0);
    bind_toggle_fullscreen = fsl_key_bind_init(FSL_KEY_F11, 0, 0, 0, 0);
    bind_zoom = fsl_key_bind_init(FSL_KEY_Z, 0, 0, 0, 0);
    bind_toggle_flashlight = fsl_key_bind_init(FSL_KEY_F, 0, 0, 0, 0);
    bind_pause = fsl_key_bind_init(FSL_KEY_ESCAPE, 0, 0, 0, 0);
    bind_command_line = fsl_key_bind_init(FSL_KEY_SLASH, 0, 0, 0, 0);

    /* ---- debug & menu ---------------------------------------------------- */

    bind_left = fsl_key_bind_init(FSL_KEY_LEFT, 0, 0, 0, 0);
    bind_right = fsl_key_bind_init(FSL_KEY_RIGHT, 0, 0, 0, 0);
    bind_down = fsl_key_bind_init(FSL_KEY_DOWN, 0, 0, 0, 0);
    bind_up = fsl_key_bind_init(FSL_KEY_UP, 0, 0, 0, 0);
    bind_toggle_super_debug = fsl_key_bind_init(FSL_KEY_TAB, 0, 0, 0, 0);
    bind_reload_shaders = fsl_key_bind_init(FSL_KEY_L, 0, FSL_CONTROL_LEFT, 0, 0);
}

void input_update(hhc_player *p)
{
    fsl_shader_program *shader_p = fsl_mem_handle_get(shader);
    u32 shader_err = FSL_ERR_SUCCESS;
    u32 i = 0;
    f32 px = 0.0f;
    f32 nx = 0.0f;
    f32 py = 0.0f;
    f32 ny = 0.0f;
    f32 pz = 0.0f;
    f32 nz = 0.0f;
    f32 spch = sin(p->transform.rot.y * FSL_DEG2RAD);
    f32 cpch = cos(p->transform.rot.y * FSL_DEG2RAD);
    f32 syaw = sin(p->transform.rot.z * FSL_DEG2RAD);
    f32 cyaw = cos(p->transform.rot.z * FSL_DEG2RAD);

    p->input.x = 0.0f;
    p->input.y = 0.0f;
    p->input.z = 0.0f;

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
                (py - ny) * -cos(p->transform.rot.z * FSL_DEG2RAD + FSL_PI / 2.0) +
                (pz - nz) * cyaw * spch;
            p->input.y =
                (px - nx) * -syaw * cpch +
                (py - ny) * sin(p->transform.rot.z * FSL_DEG2RAD + FSL_PI / 2.0) +
                (pz - nz) * -syaw * spch;
            p->input.z =
                (px - nx) * -spch +
                (pz - nz) * cpch;
        }
        else
        {
            p->input.x =
                (px - nx) * cyaw +
                (py - ny) * -cos(p->transform.rot.z * FSL_DEG2RAD + FSL_PI / 2.0);
            p->input.y =
                (px - nx) * -syaw +
                (py - ny) * sin(p->transform.rot.z * FSL_DEG2RAD + FSL_PI / 2.0);
            p->input.z =
                pz - nz;
        }

        p->input = fsl_normalize_v3f32(p->input);

        /* ---- gameplay ---------------------------------------------------- */

        if (p->hit.hit && !p->menu_state && !core.flag.super_debug)
        {
            if (fsl_is_mouse_press(bind_attack_or_destroy))
            {
                block_break(p->hit);
            }

            if (fsl_is_mouse_press(bind_build_or_use))
            {
                block_place(p->hit, p->hotbar_slots[p->hotbar_slot_selected].id);
            }

            if (fsl_is_key_press(bind_sample_block))
            {
                p->hotbar_slots[p->hotbar_slot_selected].id = GET_BLOCK_ID(*p->hit.block);
            }
        }

        for (i = 0; i < CONTAINER_HOTBAR_SLOTS_MAX; ++i)
        {
            if (fsl_is_key_press(bind_hotbar[0][i]) || fsl_is_key_press(bind_hotbar[1][i]))
                player_hotbar_selected_set(p, fsl_mod_i32(i - 1, CONTAINER_HOTBAR_SLOTS_MAX));
        }

        if (fsl_is_key_press(bind_inventory))
        {
            if (p->menu_state == STATE_PLAYER_MENU_INVENTORY_SURVIVAL && state_menu_depth == 1)
            {
                state_menu_depth = 0;
                p->menu_state = 0;
                disable_cursor;
                center_cursor;
            }
            else if (p->menu_state != STATE_PLAYER_MENU_INVENTORY_SURVIVAL && !state_menu_depth)
            {
                state_menu_depth = 1;
                p->menu_state = STATE_PLAYER_MENU_INVENTORY_SURVIVAL;
                enable_cursor;
            }

            if (p->menu_state != STATE_PLAYER_MENU_INVENTORY_SURVIVAL && state_menu_depth)
                --state_menu_depth;
        }

        /* ---- miscellaneous ----------------------------------------------- */

        if (fsl_is_key_press(bind_pause))
        {
            core.request.menu_back = TRUE;

            if (!state_menu_depth)
            {
                ++state_menu_depth;
                menu_index_curr = MENU_GAME_PAUSE;
                p->menu_state = 0;
                enable_cursor;
            }
            else
                --state_menu_depth;

            if (!state_menu_depth)
            {
                disable_cursor;
                center_cursor;
            }
        }

        if (fsl_is_key_press(bind_toggle_hud))
            core.flag.hud ^= 1;

        if (fsl_is_key_press(bind_take_screenshot))
            fsl_request_screenshot();

        if (fsl_is_key_press(bind_toggle_debug))
            core.flag.debug ^= 1;

        if (fsl_is_key_press(bind_toggle_cinematic_motion))
        {
            p->flag ^= FLAG_PLAYER_CINEMATIC_MOTION;

            if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
                LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "Cinematic Motion Toggled On\n");
            else
                LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                        "Cinematic Motion Toggled Off\n");
        }

        if (fsl_is_key_press(bind_toggle_perspective))
            p->camera_mode = (p->camera_mode + 1) % PLAYER_CAMERA_MODE_COUNT;

        if (fsl_is_key_press(bind_zoom))
        {
            p->flag |= FLAG_PLAYER_ZOOMER;
            LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "Zoom Toggled On\n");
        }
        if (fsl_is_key_release(bind_zoom))
        {
            p->flag &= ~FLAG_PLAYER_ZOOMER;
            LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "Zoom Toggled Off\n");
        }
    }
    else
    {
        if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        {
            p->flag ^= FLAG_PLAYER_CINEMATIC_MOTION;

            LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "Cinematic Motion Toggled Off\n");
        }

        if (p->flag & FLAG_PLAYER_ZOOMER)
        {
            p->flag &= ~FLAG_PLAYER_ZOOMER;
            LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "Zoom Toggled Off\n");
        }
    }

    if (fsl_is_key_press(bind_toggle_flashlight))
    {
        p->flag ^= FLAG_PLAYER_FLASHLIGHT;

        if (p->flag & FLAG_PLAYER_FLASHLIGHT)
            LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "Flashlight Toggled On\n");
        else
            LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "Flashlight Toggled Off\n");
    }

    /* ---- debug ----------------------------------------------------------- */

    if (fsl_is_key_press(bind_reload_shaders))
    {
        for (i = 0; i < SHADER_COUNT; ++i)
        {
            if (fsl_shader_program_init(&shader_p[i]) != FSL_ERR_SUCCESS)
                shader_err = FSL_ERR_SHADER_COMPILE_FAIL;
        }

        if (shader_err == FSL_ERR_SUCCESS)
            LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "Shaders Reloaded!\n");
        else
            LOGERROR(shader_err, FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                    "Failed to Reload Shaders\n");
    }

    if (fsl_is_key_press(bind_toggle_super_debug))
    {
        core.flag.super_debug ^= 1;

        if (core.flag.super_debug)
            enable_cursor;
        else
        {
            disable_cursor;
            center_cursor;
        }
    }
}
