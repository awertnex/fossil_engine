#ifndef HHC_GUI_MENUS_H
#define HHC_GUI_MENUS_H

#include "deps/fossil/math/vector.h"

void gui_menus_init(v2i32 render_size);
void gui_menu_title_update(v2i32 render_size);
void gui_menu_pause_update(v2i32 render_size);
void gui_menu_title_draw(void);
void gui_menu_pause_draw(void);

#endif /* HHC_GUI_MENUS_H */
