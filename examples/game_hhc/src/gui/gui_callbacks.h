#ifndef HHC_GUI_CALLBACKS_H
#define HHC_GUI_CALLBACKS_H

#include "deps/fossil/ui/ui_types.h"

void ui_panel_enter_func(fsl_ui_event event, void *data);
void ui_panel_leave_func(fsl_ui_event event, void *data);
void ui_button_enter_func(fsl_ui_event event, void *data);
void ui_button_leave_func(fsl_ui_event event, void *data);
void ui_button_click_func(fsl_ui_event event, void *data);
void ui_button_release_func(fsl_ui_event event, void *data);

#endif /* HHC_GUI_CALLBACKS_H */
