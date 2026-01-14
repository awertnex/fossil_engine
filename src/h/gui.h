#ifndef GAME_GUI_H
#define GAME_GUI_H

#include "main.h"

#define show_cursor     glfwSetInputMode(render->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL)
#define disable_cursor  glfwSetInputMode(render->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED)
#define center_cursor   glfwSetCursorPos(render->window, render->size.x / 2.0f, render->size.y / 2.0f)

#define BTN_COUNT 110

#define COL_TEXT_DEFAULT    0xbcbcbcff
#define COL_TEXT_HOVER      0xa4ed74ff

extern u16 menu_index;
extern u16 menu_layer[5];
extern u8 state_menu_depth;
extern b8 is_menu_ready;
extern u8 buttons[BTN_COUNT];

enum MenuNames
{
    MENU_TITLE = 1,
    MENU_SINGLEPLAYER,
    MENU_MULTIPLAYER,
    MENU_SETTINGS,
    MENU_SETTINGS_AUDIO,
    MENU_SETTINGS_VIDEO,
    MENU_GAME_PAUSE,
    MENU_DEATH,

}; /* MenuNames */

enum ButtonNames
{
    /* ---- title screen ---------------------------------------------------- */

    BTN_SINGLEPLAYER = 0,
    BTN_MULTIPLAYER,
    BTN_SETTINGS,
    BTN_QUIT,

    /* ---- world menu ------------------------------------------------------ */

    BTN_UNPAUSE = 0,
    BTN_ENABLE_LAN_CONNECTION,

    /* ---- settings -------------------------------------------------------- */

    BTN_DONE = 0,
    BTN_FOV,
    BTN_SETTINGS_AUDIO,
    BTN_SETTINGS_VIDEO,
    BTN_CONTROLS,

    /* ---- hotbar slots ---------------------------------------------------- */

    BTN_HOTBAR_1,
    BTN_HOTBAR_2,
    BTN_HOTBAR_3,
    BTN_HOTBAR_4,
    BTN_HOTBAR_5,
    BTN_HOTBAR_6,
    BTN_HOTBAR_7,
    BTN_HOTBAR_8,
    BTN_HOTBAR_9,

    /* ---- functional ------------------------------------------------------ */

    BTN_ITEM_IN_1,
    BTN_ITEM_IN_2,
    BTN_ITEM_IN_3,
    BTN_ITEM_IN_4,
    BTN_ITEM_IN_5,
    BTN_ITEM_IN_6,
    BTN_ITEM_IN_7,
    BTN_ITEM_IN_8,
    BTN_ITEM_IN_9,
    BTN_ITEM_OUT_1,
    BTN_ITEM_OUT_2,
    BTN_ITEM_OUT_3,

    /* ---- super debugger (SDB) -------------------------------------------- */

    BTN_SDB_ADD,
    BTN_SDB_SUB,
}; /* ButtonNames */

/*! @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 gui_init(void);

void gui_free(void);

#if 0 /* TODO: undef */
void update_menus(v2f32 render_size);
void draw_hud();

float get_str_width(Font font, const str *str, f32 font_size, f32 spacing);
void draw_texture(Texture2D texture, Rectangle source,
        v2i16 pos, v2i16 scl, u8 align_x, u8 align_y, Color tint);
void draw_texture_tiled(Texture2D texture, Rectangle source, Rectangle dest,
        v2i16 pos, v2i16 scl, Color tint);
void draw_texture_simple(Texture2D texture, Rectangle source,
        v2i16 pos, v2i16 scl, Color tint);
void draw_button(Texture2D texture, Rectangle button,
        v2i16 pos, u8 align_x, u8 align_y, u8 btn_state,
        void (*func)(), const str *str);
#endif /* TODO: undef */

void btn_func_singleplayer();
void btn_func_multiplayer();
void btn_func_settings();
void btn_func_back();
void btn_func_unpause();
void btn_func_quit_game();
void btn_func_quit_world();

#endif /* GAME_GUI_H */
