#include "deps/fossil/engine/engine_assets.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/ui/ui_element.h"
#include "deps/fossil/ui/ui_types.h"

#include "gui_callbacks.h"

void ui_button_enter_func(fsl_ui_event event, void *data)
{
    fsl_ui_element_set_uv(event.caller, 0, 16, 16, 16);
}

void ui_button_leave_func(fsl_ui_event event, void *data)
{
    fsl_ui_element_set_uv(event.caller, 0, 0, 16, 16);
}

void ui_button_click_func(fsl_ui_event event, void *data)
{
    fsl_ui_element_set_uv(event.caller, 16, 0, 16, 16);
}

void ui_button_release_func(fsl_ui_event event, void *data)
{
    ui_button_enter_func(event, data);
}
