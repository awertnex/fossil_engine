#include <dirent.h>

#include "launcher.h"
#include <engine/h/logger.h>
#include "../h/dir.h"

u8 state = 0;
v2i16 render_size = {500, 720};

DIR *dir;
struct dirent *drnt;

void init_launcher()
{
    state |= FLAG_ACTIVE;
    SetWindowState(FLAG_MSAA_4X_HINT);
    InitWindow(render_size.x, render_size.y, "Heaven-Hell Continuum Launcher");
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    if (LOGGING_DEBUG)
    {
        LOGDEBUG("Debugging Enabled");
        SetWindowState(FLAG_WINDOW_TOPMOST);
    }
}

int main(void)
{
    init_launcher();

    while (state & FLAG_ACTIVE)
    {
        update_launcher_input();
        update_launcher();
    }

    CloseWindow();
    return 0;
}

void update_launcher()
{
    if (IsWindowResized())
        render_size = (v2i16){GetRenderWidth(), GetRenderHeight()};

    BeginDrawing();
    ClearBackground(COL_BG);

    EndDrawing();
}

void update_launcher_input() /* TODO: evaluate instance */
{
    if (IsKeyPressed(KEY_Q))
        state &= ~FLAG_ACTIVE;
}

void evaluate_instance(str *path)
{
}
