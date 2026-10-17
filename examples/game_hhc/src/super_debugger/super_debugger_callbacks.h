#ifndef HHC_SUPER_DEBUGGER_CALLBACKS_H
#define HHC_SUPER_DEBUGGER_CALLBACKS_H

#include "deps/fossil/ui/ui_types.h"

void super_debugger_panel_enter_func(fsl_ui_event event, void *data);
void super_debugger_panel_leave_func(fsl_ui_event event, void *data);
void super_debugger_button_enter_func(fsl_ui_event event, void *data);
void super_debugger_button_leave_func(fsl_ui_event event, void *data);
void super_debugger_button_click_func(fsl_ui_event event, void *data);
void super_debugger_button_release_func(fsl_ui_event event, void *data);

void super_debugger_button_click_func_toggle_trans_blocks(fsl_ui_event event, void *data);
void super_debugger_button_click_func_toggle_bounding_boxes(fsl_ui_event event, void *data);
void super_debugger_button_click_func_toggle_chunk_bounds(fsl_ui_event event, void *data);
void super_debugger_button_click_func_toggle_chunk_gizmo(fsl_ui_event event, void *data);
void super_debugger_button_click_func_toggle_chunk_scheduler_visualizer(fsl_ui_event event, void *data);

#endif /* HHC_SUPER_DEBUGGER_CALLBACKS_H */
