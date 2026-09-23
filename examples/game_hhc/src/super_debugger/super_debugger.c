#include "deps/fossil/common/config.h"
#include "deps/fossil/assets/assets.h"
#include "deps/fossil/engine/engine_assets.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/string/string.h"
#include "deps/fossil/ui/ui.h"

#include "../settings/settings.h"

#include "../h/assets.h"
#include "../h/main.h"

#include "super_debugger.h"
#include "super_debugger_callbacks.h"

static i32 logger_scroll_pos = 0;
fsl_ui_element ui_element_sdb[UI_ELEMENT_SDB_COUNT] = {0};

void super_debugger_init(v2i32 render_size)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);

    fsl_ui_element_set_texture(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            &fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_INACTIVE]);
    fsl_ui_element_set_texture(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER],
            &fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_INACTIVE]);
    fsl_ui_element_set_texture(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE]);
    fsl_ui_element_set_texture(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE]);
    fsl_ui_element_set_texture(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE]);
    fsl_ui_element_set_texture(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE]);
    fsl_ui_element_set_texture(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE]);

    fsl_ui_element_set_uv(&ui_element_sdb[UI_ELEMENT_SDB_PANEL], 0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER], 0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS], 0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES], 0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS], 0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO], 0, 0, 16, 16);
    fsl_ui_element_set_uv(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            0, 0, 16, 16);

    fsl_ui_element_set_9_slice(&ui_element_sdb[UI_ELEMENT_SDB_PANEL], TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER], TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS], TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES], TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS], TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO], TRUE, 8);
    fsl_ui_element_set_9_slice(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            TRUE, 8);

    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_panel_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_panel_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_button_enter_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            FSL_UI_EVENT_TYPE_ENTER, super_debugger_button_enter_func, NULL);

    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_panel_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_panel_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_button_leave_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            FSL_UI_EVENT_TYPE_LEAVE, super_debugger_button_leave_func, NULL);

    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            FSL_UI_EVENT_TYPE_CLICK, super_debugger_button_click_func_toggle_trans_blocks, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            FSL_UI_EVENT_TYPE_CLICK, super_debugger_button_click_func_toggle_bounding_boxes, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            FSL_UI_EVENT_TYPE_CLICK, super_debugger_button_click_func_toggle_chunk_bounds, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            FSL_UI_EVENT_TYPE_CLICK, super_debugger_button_click_func_toggle_chunk_gizmo, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            FSL_UI_EVENT_TYPE_CLICK, super_debugger_button_click_func_toggle_chunk_scheduler_visualizer,
            NULL);

    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            FSL_UI_EVENT_TYPE_RELEASE, super_debugger_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            FSL_UI_EVENT_TYPE_RELEASE, super_debugger_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            FSL_UI_EVENT_TYPE_RELEASE, super_debugger_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            FSL_UI_EVENT_TYPE_RELEASE, super_debugger_button_release_func, NULL);
    fsl_ui_element_set_callback(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            FSL_UI_EVENT_TYPE_RELEASE, super_debugger_button_release_func, NULL);

    fsl_ui_element_attach(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            &ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS]);
    fsl_ui_element_attach(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            &ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES]);
    fsl_ui_element_attach(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            &ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS]);
    fsl_ui_element_attach(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            &ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO]);
    fsl_ui_element_attach(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            &ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER]);

    super_debugger_update(render_size);
}

void super_debugger_update(v2i32 render_size)
{
    fsl_ui_element_set_position(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            0, 0, SET_MARGIN, SET_MARGIN, 0, 0);
    fsl_ui_element_set_position(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER],
            0, render_size.y, SET_MARGIN, -SET_MARGIN, 0, 0);
    fsl_ui_element_set_size(&ui_element_sdb[UI_ELEMENT_SDB_PANEL],
            400, render_size.y - SET_MARGIN * 2, 0, 0);
    fsl_ui_element_set_size(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER],
            render_size.x - SET_MARGIN * 2, 400, 0, 0);

    fsl_ui_element_bake(&ui_element_sdb[UI_ELEMENT_SDB_PANEL]);

    fsl_ui_element_set_position(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            ui_element_sdb[UI_ELEMENT_SDB_PANEL].transform.size_baked.x,
            0, 0, 0, -SET_MARGIN, SET_MARGIN);
    fsl_ui_element_set_position(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            ui_element_sdb[UI_ELEMENT_SDB_PANEL].transform.size_baked.x,
            0, 0, 0, -SET_MARGIN, SET_MARGIN);
    fsl_ui_element_set_position(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            ui_element_sdb[UI_ELEMENT_SDB_PANEL].transform.size_baked.x,
            0, 0, 0, -SET_MARGIN, SET_MARGIN);
    fsl_ui_element_set_position(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            ui_element_sdb[UI_ELEMENT_SDB_PANEL].transform.size_baked.x,
            0, 0, 0, -SET_MARGIN, SET_MARGIN);
    fsl_ui_element_set_position(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            ui_element_sdb[UI_ELEMENT_SDB_PANEL].transform.size_baked.x,
            0, 0, 0, -SET_MARGIN, SET_MARGIN);

    fsl_ui_element_set_size(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS], 0, 0, 32, 16);
    fsl_ui_element_set_size(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES], 0, 0, 32, 16);
    fsl_ui_element_set_size(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS], 0, 0, 32, 16);
    fsl_ui_element_set_size(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO], 0, 0, 32, 16);
    fsl_ui_element_set_size(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            0, 0, 32, 16);

    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            settings.gui_scale, settings.gui_scale);

    fsl_ui_element_set_alignment(&ui_element_sdb[UI_ELEMENT_SDB_PANEL], -1, -1);
    fsl_ui_element_set_alignment(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER], -1, 1);
    fsl_ui_element_set_alignment(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS], 1, -1);
    fsl_ui_element_set_alignment(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES], 1, -4);
    fsl_ui_element_set_alignment(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS], 1, -7);
    fsl_ui_element_set_alignment(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO], 1, -10);
    fsl_ui_element_set_alignment(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            1, -13);
}

void super_debugger_draw(v2i32 render_size, GLuint fbo)
{
    i32 i = 0;
    u32 index = 0;
    i32 logger_panel_height =
        ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER].transform.size_baked.y - SET_MARGIN * 2;
    fsl_log_entry *log_entry = NULL;

    fsl_ui_start(FALSE);

    fsl_ui_element_draw(&ui_element_sdb[UI_ELEMENT_SDB_PANEL]);
    fsl_ui_element_draw(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS]);
    fsl_ui_element_draw(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES]);
    fsl_ui_element_draw(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS]);
    fsl_ui_element_draw(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO]);
    fsl_ui_element_draw(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER]);

    fsl_ui_element_draw(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER]);
    fsl_text_start(font[FONT_MONO_BOLD], settings.font_size, 0, FALSE);

    log_entry = fsl_mem_handle_get(logger_core.buf);
    for (i = logger_panel_height / settings.font_size; i > 0; --i)
    {
        index = fsl_mod_i32(logger_core.cursor - i - logger_scroll_pos, FSL_LOGGER_HISTORY_MAX);
        fsl_text_push(fsl_stringf("%s\n", log_entry[index].message),
                SET_MARGIN * 2, render_size.y - SET_MARGIN * 2,
                0, 0, render_size.x - SET_MARGIN * 4,
                log_entry[index].color);

        if ((i32)fsl_get_text_height() + SET_MARGIN * 2 >= logger_panel_height)
            break;
    }

    /* this "useless" function call aligns all the pushed strings correctly once
     * at the end of the loop, do not touch it. */
    fsl_text_push("", 0, 0, 0, FSL_TEXT_ALIGN_BOTTOM, 0, 0);
    fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
    fsl_fbo_blit(fbo);
    fsl_ui_stop();
}

void super_debugger_gui_scale_set(f32 scale)
{
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_PANEL], scale, scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_PANEL_LOGGER], scale, scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS],
            scale, scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES],
            scale, scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS],
            scale, scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO],
            scale, scale);
    fsl_ui_element_set_scale(&ui_element_sdb[UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER],
            scale, scale);
}

void super_debugger_logger_scroll(i32 delta)
{
    logger_scroll_pos =
        fsl_clamp_i32(logger_scroll_pos + delta * SET_CONSOLE_SCROLL_SPEED, 0, logger_core.cursor);
}
