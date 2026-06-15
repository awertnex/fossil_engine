#include "deps/fossil/assets/asset_types.h"
#include "deps/fossil/engine/engine.h"
#include "deps/fossil/engine/engine_assets.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shader_types.h"
#include "deps/fossil/ui/ui.h"

#include "../settings/settings.h"

#include "../h/main.h"
#include "../h/assets.h"

#include "gui.h"
#include "gui_callbacks.h"
#include "gui_menus.h"

enum ui_element_menu_title_index
{
    UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER,
    UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER,
    UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS,
    UI_ELEMENT_MENU_TITLE_BUTTON_QUIT,
    UI_ELEMENT_MENU_COUNT
}; /* ui_element_menu_title_index */

static fsl_ui_element ui_element_menu[UI_ELEMENT_MENU_COUNT];

static void ui_button_menu_title_singleplayer_click_func(fsl_ui_event event, void *data);
static void ui_button_menu_title_multiplayer_click_func(fsl_ui_event event, void *data);
static void ui_button_menu_title_settings_click_func(fsl_ui_event event, void *data);
static void ui_button_menu_title_quit_click_func(fsl_ui_event event, void *data);

void gui_menus_init(v2i32 render_size)
{
    fsl_texture *texture_p = fsl_mem_handle_get(texture);

    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            &texture_p[TEXTURE_BUTTON]);

    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            0, 0, 16, 16);

    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            TRUE, 8);

    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);

    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);

    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_menu_title_singleplayer_click_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_menu_title_multiplayer_click_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_menu_title_settings_click_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_menu_title_quit_click_func, NULL);

    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);

    gui_menu_title_update(render_size);
}

void gui_menu_title_update(v2i32 render_size)
{
    v2i32 button_size = {256, 32};
    i32 button_spacing = 4;

    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, -button_spacing);
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, 0);
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            render_size.x / 2, render_size.y / 2, 0, 0, -button_spacing / 2, button_spacing);
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            render_size.x / 2, render_size.y / 2, 0, 0, button_spacing / 2, button_spacing);

    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER], 0, 0,
            button_size.x, button_size.y);
    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER], 0, 0,
            button_size.x, button_size.y);
    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS], 0, 0,
            button_size.x / 2 - button_spacing / 2, button_size.y);
    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT], 0, 0,
            button_size.x / 2 - button_spacing / 2, button_size.y);

    fsl_ui_element_set_scale(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            settings.gui_scale, settings.gui_scale);

    fsl_ui_element_set_alignment(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            0, 2);
    fsl_ui_element_set_alignment(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            0, 0);
    fsl_ui_element_set_alignment(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            1, -2);
    fsl_ui_element_set_alignment(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            -1, -2);
}

void gui_menu_title_draw(v2i32 render_size)
{
    fsl_ui_start(TRUE);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER]);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER]);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS]);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT]);
    fsl_ui_stop();
    fsl_fbo_blit(0);
}

void ui_button_menu_title_singleplayer_click_func(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    menu_index_cur = 0; /* TODO: set actual value (MENU_SINGLEPLAYER) */
    state_menu_depth = 0; /* TODO: set actual value (2) */
    is_menu_ready = 0;
    core.flag.paused = FALSE;
    core.request.world_load = TRUE;
}

void ui_button_menu_title_multiplayer_click_func(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    menu_index_cur = MENU_MULTIPLAYER;
    state_menu_depth = 2;
    is_menu_ready = 0;
}

void ui_button_menu_title_settings_click_func(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    menu_index_cur = MENU_SETTINGS;
    state_menu_depth = 2;
    is_menu_ready = 0;
}

void ui_button_menu_title_quit_click_func(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    fsl_request_engine_close();
}
