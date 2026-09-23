#ifndef HHC_SUPER_DEBUGGER_H
#define HHC_SUPER_DEBUGGER_H

#include "deps/fossil/math/vector.h"
#include "deps/fossil/ui/ui_element.h"

enum ui_element_sdb_index
{
    UI_ELEMENT_SDB_PANEL,
    UI_ELEMENT_SDB_PANEL_LOGGER,
    UI_ELEMENT_SDB_TOGGLE_TRANS_BLOCKS,
    UI_ELEMENT_SDB_TOGGLE_BOUNDING_BOXES,
    UI_ELEMENT_SDB_TOGGLE_CHUNK_BOUNDS,
    UI_ELEMENT_SDB_TOGGLE_CHUNK_GIZMO,
    UI_ELEMENT_SDB_TOGGLE_CHUNK_SCHEDULER_VISUALIZER,
    UI_ELEMENT_SDB_COUNT
}; /* ui_element_sdb_index */

extern fsl_ui_element ui_element_sdb[UI_ELEMENT_SDB_COUNT];

void super_debugger_init(v2i32 render_size);
void super_debugger_update(v2i32 render_size);
void super_debugger_draw(v2i32 render_size, GLuint fbo);
void super_debugger_logger_scroll(i32 delta);
void super_debugger_gui_scale_set(f32 scale);

#endif /* HHC_SUPER_DEBUGGER_H */
