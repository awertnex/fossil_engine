#ifndef HHC_GUI_H
#define HHC_GUI_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/math/vector.h"
#include "deps/fossil/ui/ui_element.h"

#define enable_cursor   glfwSetInputMode(render->window, \
        GLFW_CURSOR, GLFW_CURSOR_NORMAL)

#define disable_cursor  glfwSetInputMode(render->window, \
        GLFW_CURSOR, GLFW_CURSOR_DISABLED)

#define center_cursor \
    do { \
        glfwSetCursorPos(render->window, render->size.x / 2.0f, render->size.y / 2.0f); \
        fsl_request_skip_mouse_delta(); \
    } while (0)

#define BTN_COUNT 110

#define COL_TEXT_DEFAULT    0xbcbcbcff
#define COL_TEXT_HOVER      0xa4ed74ff

enum ui_element_index
{
    UI_ELEMENT_CROSSHAIR,
    UI_ELEMENT_HOTBAR,
    UI_ELEMENT_HOTBAR_SELECTED,
    UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL,
    UI_ELEMENT_COUNT
}; /* ui_element_index */

enum menu_index
{
    MENU_NONE,
    MENU_TITLE,
    MENU_SINGLEPLAYER,
    MENU_MULTIPLAYER,
    MENU_SETTINGS,
    MENU_SETTINGS_AUDIO,
    MENU_SETTINGS_VIDEO,
    MENU_GAME_PAUSE,
    MENU_DEATH
}; /* menu_index */

enum button_index
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
    BTN_ITEM_OUT_3
}; /* button_index */

extern u32 menu_index_curr;
extern u32 menu_layer[5];
extern u32 state_menu_depth;
extern b8 is_menu_ready;
extern u32 buttons[BTN_COUNT];
extern fsl_ui_element ui_element[UI_ELEMENT_COUNT];

/*!
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 gui_init(v2i32 render_size);

void gui_update(v2i32 render_size);
void gui_free(void);

/*!
 *  @brief bind correct resources to start drawing UI items.
 */
void gui_start_ui_items(v2i32 render_size);

/*!
 *  @brief draw UI item (e.g., hotbar items, container items).
 */
void gui_draw_ui_item(u32 item_id, f32 pos_x, f32 pos_y, v2i32 render_size);

#if 0 /* TODO: undef */
void update_menus(v2f32 render_size);
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

#endif /* HHC_GUI_H */
