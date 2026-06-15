#include "deps/fossil/engine/engine_assets.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/ui/ui_element.h"
#include "deps/fossil/ui/ui_types.h"

#include "gui_callbacks.h"

void ui_panel_enter_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_ACTIVE]);
}

void ui_panel_leave_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_PANEL_INACTIVE]);
}

void ui_button_enter_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_SELECTED]);
}

void ui_button_leave_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_ACTIVE]);
}

void ui_button_click_func(fsl_ui_event event, void *data)
{
    fsl_texture *fsl_texture_p = fsl_mem_handle_get(fsl_texture_buf);
    fsl_ui_element_set_texture(event.caller, &fsl_texture_p[FSL_TEXTURE_INDEX_BUTTON_PRESSED]);
}

void ui_button_release_func(fsl_ui_event event, void *data)
{
    ui_button_enter_func(event, data);
}
