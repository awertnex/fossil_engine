#ifndef GAME_LAUNCHER_H
#define GAME_LAUNCHER_H

#include <raylib.h>
#include <engine/h/defines.h>

#define WIDTH 1280
#define HEIGHT 720
#define MARGIN 20
#define color(r, g, b, v, a) \
    (Color){((f32)r/255)*v, ((f32)g/255)*v, ((f32)b/255)*v, a}

enum State
{
    FLAG_ACTIVE =           0x01,
}; /* State */

#define COL_BG              color(0x30, 0x30, 0x30, 0xFF, 0xFF)
#define COL_BUTTON          color(0x26, 0x26, 0x26, 0xFF, 0xFF)
#define COL_BUTTON_HOVER    color(0x2d, 0x2d, 0x2d, 0xFF, 0xFF)

extern u8 state;
extern v2i16 render_size;
extern Vector2 cursor;
#define detect_cursor cursor = GetMousePosition();

void init_launcher();
void update_launcher();
void close_launcher();
void update_launcher_input();

#endif /* GAME_LAUNCHER_H */
