#include <engine/h/input.h>
#include <engine/h/math.h>

#include "h/assets.h"
#include "h/chunking.h"
#include "h/gui.h"
#include "h/input.h"
#include "h/player.h"
#include "h/world.h"

/* ---- movement ------------------------------------------------------------ */

u32 bind_walk_forward =             KEY_W;
u32 bind_walk_backward =            KEY_S;
u32 bind_strafe_left =              KEY_A;
u32 bind_strafe_right =             KEY_D;
u32 bind_jump =                     KEY_SPACE;
u32 bind_sprint =                   KEY_LEFT_SHIFT;
u32 bind_sneak =                    KEY_LEFT_CONTROL;

/* ---- gameplay ------------------------------------------------------------ */

u32 bind_attack_or_destroy =        GLFW_MOUSE_BUTTON_LEFT;
u32 bind_sample_block =             GLFW_MOUSE_BUTTON_MIDDLE;
u32 bind_build_or_use =             GLFW_MOUSE_BUTTON_RIGHT;

/* ---- inventory ----------------------------------------------------------- */

u32 bind_selected_item =            KEY_Q;
u32 bind_hotbar[2][PLAYER_HOTBAR_SLOTS_MAX] =
{
    {
        KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
        KEY_6, KEY_7, KEY_8, KEY_9, KEY_0,
    },
    {
        KEY_KP_1, KEY_KP_2, KEY_KP_3, KEY_KP_4, KEY_KP_5,
        KEY_KP_6, KEY_KP_7, KEY_KP_8, KEY_KP_9, KEY_KP_0,
    },
};
u32 bind_inventory =                KEY_E;

/* ---- miscellaneous ------------------------------------------------------- */

u32 bind_toggle_hud =               KEY_F1;
u32 bind_take_screenshot =          KEY_F2;
u32 bind_toggle_debug =             KEY_F3;
u32 bind_toggle_cinematic_camera =  KEY_F4;
u32 bind_toggle_perspective =       KEY_F5;
u32 bind_toggle_cinematic_motion =  KEY_F6;
u32 bind_toggle_fullscreen =        KEY_F11;
u32 bind_toggle_zoom =              KEY_Z;
u32 bind_toggle_flashlight =        KEY_F;
u32 bind_pause =                    KEY_ESCAPE;
u32 bind_chat_or_command =          KEY_SLASH;

/* ---- debug & menu -------------------------------------------------------- */

/* TODO: navigate menus with arrow keys */
u32 bind_left =                     KEY_LEFT;
u32 bind_right =                    KEY_RIGHT;
u32 bind_down =                     KEY_DOWN;
u32 bind_up =                       KEY_UP;
u32 bind_debug_mod =                KEY_LEFT_ALT;
u32 bind_toggle_super_debug =       KEY_TAB;
u32 bind_toggle_trans_blocks =      KEY_T;
u32 bind_toggle_chunk_bounds =      KEY_C;
u32 bind_toggle_bounding_boxes =    KEY_B;
u32 bind_toggle_chunk_gizmo =       KEY_G;
u32 bind_toggle_chunk_queue_visualizer = KEY_V;

void input_update(Render render, Player *p)
{
    u32 i;
    f32 px = 0.0f, nx = 0.0f,
        py = 0.0f, ny = 0.0f,
        pz = 0.0f, nz = 0.0f;

    p->input = (v3f32){0};

    /* ---- movement -------------------------------------------------------- */

    px += (f32)is_key_hold(bind_walk_forward);
    nx += (f32)is_key_hold(bind_walk_backward);
    py += (f32)is_key_hold(bind_strafe_left);
    ny += (f32)is_key_hold(bind_strafe_right);

    if (is_key_press_double(bind_walk_forward))
        p->flag |= FLAG_PLAYER_SPRINTING;

    /* ---- jumping --------------------------------------------------------- */

    if (is_key_hold(bind_jump))
    {
        if (p->flag & FLAG_PLAYER_FLYING)
            pz += 1.0f;
        else if (p->flag & FLAG_PLAYER_CAN_JUMP)
        {
            p->velocity.z += sqrtf(2.0f * world.gravity * PLAYER_JUMP_HEIGHT);
            p->flag &= ~FLAG_PLAYER_CAN_JUMP;
        }
    }

    if (is_key_press_double(bind_jump))
        p->flag ^= FLAG_PLAYER_FLYING;

    /* ---- sprinting ------------------------------------------------------- */

    if (is_key_hold(bind_sprint) && is_key_hold(bind_walk_forward))
        p->flag |= FLAG_PLAYER_SPRINTING;
    else if (is_key_release(bind_walk_forward))
        p->flag &= ~FLAG_PLAYER_SPRINTING;

    /* ---- sneaking -------------------------------------------------------- */

    if (is_key_hold(bind_sneak))
    {
        if (p->flag & FLAG_PLAYER_FLYING)
            nz += 1.0f;
        else p->flag |= FLAG_PLAYER_SNEAKING;
    }
    else p->flag &= ~FLAG_PLAYER_SNEAKING;

    /* ---- apply movement input -------------------------------------------- */

    if (p->flag & FLAG_PLAYER_DEAD)
    {}
    else if (p->flag & FLAG_PLAYER_FLYING && p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
    {
        p->input.x =
            (px - nx) * cos(p->yaw * DEG2RAD) * cos(p->pitch * DEG2RAD) +
            (py - ny) * -cos(p->yaw * DEG2RAD + PI * 0.5) +
            (pz - nz) * cos(p->yaw * DEG2RAD) * sin(p->pitch * DEG2RAD);
        p->input.y =
            (px - nx) * -sin(p->yaw * DEG2RAD) * cos(p->pitch * DEG2RAD) +
            (py - ny) * sin(p->yaw * DEG2RAD + PI * 0.5) +
            (pz - nz) * -sin(p->yaw * DEG2RAD) * sin(p->pitch * DEG2RAD);
        p->input.z =
            (px - nx) * -sin(p->pitch * DEG2RAD) +
            (pz - nz) * cos(p->pitch * DEG2RAD);
    }
    else
        p->input = (v3f32){
            (px - nx) * cos(p->yaw * DEG2RAD) + (py - ny) * -cos(p->yaw * DEG2RAD + PI * 0.5),
            (px - nx) * -sin(p->yaw * DEG2RAD) + (py - ny) * sin(p->yaw * DEG2RAD + PI * 0.5),
            pz - nz,
        };

    f32 input_len = sqrtf(len_v3f32(p->input));
    if (input_len > EPSILON)
    {
        p->input.x /= input_len;
        p->input.y /= input_len;
        p->input.z /= input_len;
    }

    /* ---- gameplay -------------------------------------------------------- */

    if (
            !(flag & FLAG_MAIN_CHUNK_BUF_DIRTY) &&
            (flag & FLAG_MAIN_PARSE_TARGET) &&
            chunk_tab[chunk_tab_index])
    {
        if (glfwGetMouseButton(render.window, bind_attack_or_destroy) == GLFW_PRESS)
        {
            block_break(chunk_tab_index,
                    (i64)p->target.x - chunk_tab[chunk_tab_index]->pos.x * CHUNK_DIAMETER,
                    (i64)p->target.y - chunk_tab[chunk_tab_index]->pos.y * CHUNK_DIAMETER,
                    (i64)p->target.z - chunk_tab[chunk_tab_index]->pos.z * CHUNK_DIAMETER);
        }
        if (glfwGetMouseButton(render.window, bind_build_or_use) == GLFW_PRESS)
        {
            block_place(chunk_tab_index,
                    (i64)p->target.x - chunk_tab[chunk_tab_index]->pos.x * CHUNK_DIAMETER,
                    (i64)p->target.y - chunk_tab[chunk_tab_index]->pos.y * CHUNK_DIAMETER,
                    (i64)p->target.z - chunk_tab[chunk_tab_index]->pos.z * CHUNK_DIAMETER,
                    p->target_normal, p->hotbar_slots[p->hotbar_slot_selected]);
        }

        if (is_key_press(bind_sample_block)) {}
    }

    /* ---- inventory ------------------------------------------------------- */

    for (i = 0; i < PLAYER_HOTBAR_SLOTS_MAX; ++i)
        if (is_key_press(bind_hotbar[0][i]) || is_key_press(bind_hotbar[1][i]))
            p->hotbar_slot_selected = i;

    if (is_key_press(bind_inventory))
    {
        if ((p->container_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && state_menu_depth)
        {
            state_menu_depth = 0;
            p->container_state &= ~STATE_PLAYER_MENU_INVENTORY_SURVIVAL;
        }
        else if (!(p->container_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && !state_menu_depth)
        {
            state_menu_depth = 1;
            p->container_state |= STATE_PLAYER_MENU_INVENTORY_SURVIVAL;
        }

        if (!(p->container_state & STATE_PLAYER_MENU_INVENTORY_SURVIVAL) && state_menu_depth)
            --state_menu_depth;
    }

    /* ---- miscellaneous --------------------------------------------------- */

    if (is_key_press(bind_toggle_hud))
        flag ^= FLAG_MAIN_HUD;

    if (is_key_press(bind_toggle_debug))
        flag ^= FLAG_MAIN_DEBUG;

    if (is_key_press(bind_toggle_perspective))
        p->camera_mode = (p->camera_mode + 1) % PLAYER_CAMERA_MODE_COUNT;

    if (is_key_press(bind_toggle_zoom))
        p->flag ^= FLAG_PLAYER_ZOOMER;

    if (is_key_press(bind_toggle_flashlight))
        p->flag ^= FLAG_PLAYER_FLASHLIGHT;

    if (is_key_press(bind_toggle_cinematic_motion))
        p->flag ^= FLAG_PLAYER_CINEMATIC_MOTION;

    /* ---- debug ----------------------------------------------------------- */

#if !GAME_RELEASE_BUILD
    if (is_key_press(bind_toggle_super_debug))
        flag ^= FLAG_MAIN_SUPER_DEBUG;
#endif /* GAME_RELEASE_BUILD */

    if (is_key_hold(bind_debug_mod))
    {
        if (is_key_press(bind_toggle_trans_blocks))
            debug_mode[DEBUG_MODE_TRANS_BLOCKS] ^= 1;

        if (is_key_press(bind_toggle_chunk_bounds))
            debug_mode[DEBUG_MODE_CHUNK_BOUNDS] ^= 1;

        if (is_key_press(bind_toggle_bounding_boxes))
            debug_mode[DEBUG_MODE_BOUNDING_BOXES] ^= 1;

        if (is_key_press(bind_toggle_chunk_gizmo))
            debug_mode[DEBUG_MODE_CHUNK_GIZMO] ^= 1;

        if (is_key_press(bind_toggle_chunk_queue_visualizer))
            debug_mode[DEBUG_MODE_CHUNK_QUEUE_VISUALIZER] ^= 1;
    }
}
