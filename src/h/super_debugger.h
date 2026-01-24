#ifndef HHC_SUPER_DEBUGGER_H
#define HHC_SUPER_DEBUGGER_H

#include <engine/h/types.h>
#include "gui.h"

#define SDB_ROW_HEIGHT  22
#define SDB_BASE_SIZE   5
#define SDB_BUTTON_SIZE 14

typedef struct DebugRect
{
    Rectangle corner_00;
    Rectangle corner_10;
    Rectangle corner_01;
    Rectangle corner_11;
    Rectangle edge_left;
    Rectangle edge_right;
    Rectangle edge_top;
    Rectangle edge_bottom;
    Rectangle rect_center;
    Vector2 pos;
    Vector2 scl;
} DebugRect;

extern Texture texture_super_debugger;
extern DebugRect debug_rectangle;

extern Rectangle debug_button_add;
extern Rectangle debug_button_sub;

void init_super_debugger(v2f32 render_size);
void free_super_debugger();
void draw_super_debugger(v2f32 render_size);

#endif /* HHC_SUPER_DEBUGGER_H */
