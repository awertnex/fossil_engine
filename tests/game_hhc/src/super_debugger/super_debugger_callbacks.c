#include "deps/fossil/engine/engine_assets.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/ui/ui_types.h"

#include "../h/main.h"

#include "super_debugger_callbacks.h"

void super_debugger_panel_enter_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_ACTIVE]);
}

void super_debugger_panel_leave_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_INACTIVE]);
}

void super_debugger_button_enter_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_SELECTED]);
}

void super_debugger_button_leave_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE]);
}

void super_debugger_button_click_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_PRESSED]);
}

void super_debugger_button_release_func(fsl_ui_event event, void *data)
{
    super_debugger_button_enter_func(event, data);
}

void super_debugger_button_click_func_toggle_trans_blocks(fsl_ui_event event, void *data)
{
    super_debugger_button_click_func(event, data);

    core.debug.trans_blocks ^= 1;

    if (core.debug.trans_blocks)
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Transparent Blocks On\n");
    else
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Transparent Blocks Off\n");
}

void super_debugger_button_click_func_toggle_bounding_boxes(fsl_ui_event event, void *data)
{
    super_debugger_button_click_func(event, data);

    core.debug.bounding_boxes ^= 1;

    if (core.debug.bounding_boxes)
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Bounding Boxes On\n");
    else
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Bounding Boxes Off\n");
}

void super_debugger_button_click_func_toggle_chunk_bounds(fsl_ui_event event, void *data)
{
    super_debugger_button_click_func(event, data);

    core.debug.chunk_bounds ^= 1;

    if (core.debug.chunk_bounds)
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Chunk Boundaries On\n");
    else
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Chunk Boundaries Off\n");
}

void super_debugger_button_click_func_toggle_chunk_gizmo(fsl_ui_event event, void *data)
{
    super_debugger_button_click_func(event, data);

    core.debug.chunk_gizmo ^= 1;

    if (core.debug.chunk_gizmo)
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Chunk Gizmo On\n");
    else
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Chunk Gizmo Off\n");
}

void super_debugger_button_click_func_toggle_chunk_scheduler_visualizer(fsl_ui_event event, void *data)
{
    super_debugger_button_click_func(event, data);

    core.debug.chunk_scheduler_visualizer ^= 1;

    if (core.debug.chunk_scheduler_visualizer)
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Chunk Scheduler Visualizer On\n");
    else
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "View Chunk Scheduler Visualizer Off\n");
}
