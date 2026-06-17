#include "deps/fossil/assets/asset_types.h"
#include "deps/fossil/engine/engine.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/ui/ui.h"

#include "../settings/settings.h"

#include "../h/main.h"
#include "../h/assets.h"

#include "gui.h"
#include "gui_callbacks.h"
#include "gui_menus.h"

static v2i32 menu_button_size = {256, 32};
static i32 menu_button_spacing = 4;

enum ui_element_menu_title_index
{
    UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER,
    UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER,
    UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS,
    UI_ELEMENT_MENU_TITLE_BUTTON_QUIT,
    UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME,
    UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS,
    UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE,
    UI_ELEMENT_MENU_COUNT
}; /* ui_element_menu_title_index */

static fsl_ui_element ui_element_menu[UI_ELEMENT_MENU_COUNT];

static void ui_button_click_func_menu_title_singleplayer(fsl_ui_event event, void *data);
static void ui_button_click_func_menu_title_multiplayer(fsl_ui_event event, void *data);
static void ui_button_click_func_menu_title_settings(fsl_ui_event event, void *data);
static void ui_button_click_func_menu_title_quit(fsl_ui_event event, void *data);
static void ui_button_click_func_menu_pause_resume(fsl_ui_event event, void *data);
static void ui_button_click_func_menu_pause_settings(fsl_ui_event event, void *data);
static void ui_button_click_func_menu_pause_quit_to_title(fsl_ui_event event, void *data);

void gui_menus_init(v2i32 render_size)
{
    fsl_texture *texture_p = fsl_mem_handle_get(texture);
    u32 i = 0;

    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            &texture_p[TEXTURE_BUTTON]);
    fsl_ui_element_set_texture(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            &texture_p[TEXTURE_BUTTON]);

    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            0, 0, 16, 16);

    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            TRUE, 8);

    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            FSL_UI_EVENT_TYPE_ENTER, ui_button_enter_func, NULL);

    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            FSL_UI_EVENT_TYPE_LEAVE, ui_button_leave_func, NULL);

    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_click_func_menu_title_singleplayer, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_click_func_menu_title_multiplayer, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_click_func_menu_title_settings, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_click_func_menu_title_quit, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_click_func_menu_pause_resume, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_click_func_menu_pause_settings, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            FSL_UI_EVENT_TYPE_CLICK, ui_button_click_func_menu_pause_quit_to_title, NULL);

    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            FSL_UI_EVENT_TYPE_RELEASE, ui_button_release_func, NULL);

    gui_menu_title_update(render_size);
    gui_menu_pause_update(render_size);
}

void gui_menu_title_update(v2i32 render_size)
{
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, -menu_button_spacing);
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, 0);
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS],
            render_size.x / 2, render_size.y / 2, 0, 0,
            -menu_button_spacing / 2, menu_button_spacing);
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT],
            render_size.x / 2, render_size.y / 2, 0, 0,
            menu_button_spacing / 2, menu_button_spacing);

    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER], 0, 0,
            menu_button_size.x, menu_button_size.y);
    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER], 0, 0,
            menu_button_size.x, menu_button_size.y);
    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS], 0, 0,
            menu_button_size.x / 2 - menu_button_spacing / 2, menu_button_size.y);
    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT], 0, 0,
            menu_button_size.x / 2 - menu_button_spacing / 2, menu_button_size.y);

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

void gui_menu_pause_update(v2i32 render_size)
{
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, -menu_button_spacing);
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, 0);
    fsl_ui_element_set_position(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, menu_button_spacing);

    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME], 0, 0,
            menu_button_size.x, menu_button_size.y);
    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS], 0, 0,
            menu_button_size.x, menu_button_size.y);
    fsl_ui_element_set_size(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE], 0, 0,
            menu_button_size.x, menu_button_size.y);

    fsl_ui_element_set_scale(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            settings.gui_scale, settings.gui_scale);

    fsl_ui_element_set_alignment(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME],
            0, 2);
    fsl_ui_element_set_alignment(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS],
            0, 0);
    fsl_ui_element_set_alignment(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE],
            0, -2);
}

void gui_menu_title_draw(void)
{
    fsl_ui_start(FALSE);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SINGLEPLAYER]);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_MULTIPLAYER]);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_SETTINGS]);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_TITLE_BUTTON_QUIT]);
    fsl_ui_stop();
    fsl_fbo_blit(0);
}

void gui_menu_pause_draw(void)
{
    fsl_ui_start(FALSE);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_RESUME]);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_SETTINGS]);
    fsl_ui_element_draw(&ui_element_menu[UI_ELEMENT_MENU_PAUSE_BUTTON_QUIT_TO_TITLE]);
    fsl_ui_stop();
    fsl_fbo_blit(0);
}

static void ui_button_click_func_menu_title_singleplayer(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    menu_index_curr = 0; /* TODO: set actual value (MENU_SINGLEPLAYER) */
    state_menu_depth = 0; /* TODO: set actual value (2) */
    is_menu_ready = 0;
    core.flag.paused = FALSE;
    core.request.world_load = TRUE; /* temp */
}

static void ui_button_click_func_menu_title_multiplayer(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    menu_index_curr = MENU_MULTIPLAYER;
    state_menu_depth = 2;
    is_menu_ready = 0;
}

static void ui_button_click_func_menu_title_settings(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    menu_index_curr = MENU_SETTINGS;
    state_menu_depth = 2;
    is_menu_ready = 0;

    core.request.menu_settings_enter = TRUE;
}

static void ui_button_click_func_menu_title_quit(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    fsl_request_engine_close();
}

static void ui_button_click_func_menu_pause_resume(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    core.request.menu_back = TRUE;

    if (!state_menu_depth)
    {
        ++state_menu_depth;
        menu_index_curr = MENU_GAME_PAUSE;
        enable_cursor;
    }
    else
        --state_menu_depth;
}

static void ui_button_click_func_menu_pause_settings(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    core.request.menu_settings_enter = TRUE;
}

static void ui_button_click_func_menu_pause_quit_to_title(fsl_ui_event event, void *data)
{
    ui_button_click_func(event, data);

    core.request.menu_title_enter = TRUE;
}
