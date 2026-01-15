#include <stdio.h>
#include <string.h>
#include <math.h>

#include <engine/h/core.h>
#include <engine/h/diagnostics.h>
#include <engine/h/logger.h>
#include <engine/h/memory.h>
#include <engine/h/string.h>

#include "h/common.h"
#include "h/main.h"
#include "h/gui.h"
#include "h/dir.h"

u16 menu_index;
u16 menu_layer[5] = {0};
u8 state_menu_depth = 0;
b8 is_menu_ready;
u8 buttons[BTN_COUNT];

u32 gui_init(void)
{
    //game_menu_pos = setting.render_size.y / 3; /* TODO: figure this out */
    //menu_index = MENU_TITLE;
    //memset(buttons, 0, BTN_COUNT);
}

#ifdef FUCK /* TODO: undef FUCK */
/*jump*/
/* 
 * scale = (source.scale * scl);
 */
void draw_texture_a(Texture2D texture, Rectangle source, Rectangle dest, v2i16 pos, v2i16 scl, Color tint)
{
    if ((texture.id <= 0) || (scl.x <= 0.0f) || (scl.y <= 0.0f)
            || (source.width == 0.0f) || (source.height == 0.0f))
        return;
 
    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0.0f, 0.0f, 1.0f);
 
    i32 tile_width = source.width * scl.x;
    i32 tile_height = source.height * scl.y;
 
    /* top left */
    rlTexCoord2f(source.x / texture.width, source.y / texture.height);
    rlVertex2f(pos.x, pos.y);
 
    /* bottom left */
    rlTexCoord2f(source.x / texture.width, (source.y + source.height) / texture.height);
    rlVertex2f(pos.x, pos.y + tile_height);
 
    /* bottom right */
    rlTexCoord2f((source.x + source.width) / texture.width, (source.y + source.height) / texture.height);
    rlVertex2f(pos.x + tile_width, pos.y + tile_height);
 
    /* top right */
    rlTexCoord2f((source.x + source.width) / texture.width, source.y / texture.height);
    rlVertex2f(pos.x + tile_width, pos.y);
}

void update_menus(v2f32 render_size)
{
    if (!menu_index)
        return;

    switch (menu_index)
    {
        case MENU_TITLE:
            if (!is_menu_ready)
            {
                menu_layer[state_menu_depth] = MENU_TITLE;
                menu_index = MENU_TITLE;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_SINGLEPLAYER] = 1;
                buttons[BTN_MULTIPLAYER] = 1;
                buttons[BTN_SETTINGS] = 1;
                buttons[BTN_QUIT] = 1;
                is_menu_ready = 1;
            }

            /*jump*/
            draw_texture_a(texture_bg, rect_bg, (Rectangle){0.0f, 0.0f, 16.0f, 16.0f},
                    (v2i16){0, 0}, (v2i16){4, 4},
                    COL_TEXTURE_DEFAULT);
            draw_texture_a(texture_bg, rect_bg, (Rectangle){0.0f, 0.0f, 24.0f, 16.0f},
                    (v2i16){0, 64}, (v2i16){4, 4},
                    COL_TEXTURE_DEFAULT);
            draw_texture_a(texture_bg, rect_bg, (Rectangle){0.0f, 0.0f, 32.0f, 32.0f},
                    (v2i16){0, 128}, (v2i16){4, 4},
                    COL_TEXTURE_DEFAULT);

            draw_text(font, GAME_VERSION,
                    (v2i16){6, render_size.y - 3},
                    FONT_SIZE_DEFAULT, 2, 0, 2, COL_TEXT_DEFAULT);

            draw_text(font, GAME_AUTHOR,
                    (v2i16){render_size.x - 2, render_size.y - 3},
                    FONT_SIZE_DEFAULT, 2, 2, 2, COL_TEXT_DEFAULT);

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos},
                    1, 1,
                    BTN_SINGLEPLAYER,
                    &btn_func_singleplayer,
                    "Singleplayer");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + ((button.height + button_spacing_vertical) * setting.gui_scale)},
                    1, 1,
                    BTN_MULTIPLAYER,
                    &btn_func_multiplayer,
                    "Multiplayer");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 2) * setting.gui_scale)},
                    1, 1,
                    BTN_SETTINGS,
                    &btn_func_settings,
                    "Settings");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 3) * setting.gui_scale)},
                    1, 1,
                    BTN_QUIT,
                    &btn_func_quit,
                    "Quit Game");

            rlEnd();
            rlSetTexture(0);
            break;

        case MENU_SETTINGS:
            if (!is_menu_ready)
            {
                menu_layer[state_menu_depth] = MENU_SETTINGS;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_DONE] = 1;
                buttons[BTN_FOV] = 1;
                buttons[BTN_AUDIO] = 1;
                buttons[BTN_SETTINGS_VIDEO] = 1;
                buttons[BTN_CONTROLS] = 1;
                is_menu_ready = 1;
            }

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos},
                    1, 1,
                    BTN_DONE,
                    &btn_func_back,
                    "Done");

            rlEnd();
            rlSetTexture(0);
            break;

        case MENU_SETTINGS_GAME:
            if (!is_menu_ready)
            {
                menu_layer[state_menu_depth] = MENU_SETTINGS_GAME;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_DONE] = 1;
                buttons[BTN_FOV] = 1;
                buttons[BTN_AUDIO] = 1;
                buttons[BTN_SETTINGS_VIDEO] = 1;
                buttons[BTN_CONTROLS] = 1;
                is_menu_ready = 1;
            }

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos},
                    1, 1,
                    BTN_DONE,
                    &btn_func_back,
                    "Done");

            rlEnd();
            rlSetTexture(0);
            break;


        case MENU_GAME_PAUSE:
            if (!is_menu_ready)
            {
                menu_layer[state_menu_depth] = MENU_GAME_PAUSE;
                memset(buttons, 0, BTN_COUNT);
                buttons[BTN_UNPAUSE] = 1;
                buttons[BTN_SETTINGS] = 1;
                buttons[BTN_QUIT_WORLD] = 1;
                is_menu_ready = 1;
            }

            rlBegin(RL_QUADS);

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos},
                    1, 1,
                    BTN_UNPAUSE,
                    &btn_func_unpause,
                    "Unpause");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + ((button.height + button_spacing_vertical) * setting.gui_scale)},
                    1, 1,
                    BTN_SETTINGS,
                    &btn_func_settings,
                    "Settings");

            draw_button(texture_hud_widgets, button,
                    (v2i16){render_size.x / 2, game_menu_pos + (((button.height + button_spacing_vertical) * 4) * setting.gui_scale)},
                    1, 1,
                    BTN_QUIT,
                    &btn_func_quit_world,
                    "Quit World");

            rlEnd();
            rlSetTexture(0);
            break;
    }
}

void draw_hud()
{
    rlBegin(RL_QUADS);

    draw_texture(texture_hud_widgets, hotbar,
            hotbar_pos,
            (v2i16){setting.gui_scale, setting.gui_scale},
            1, 2, COL_TEXTURE_DEFAULT);

    draw_texture(texture_hud_widgets, hotbar_selected,
            (v2i16){
            hotbar_pos.x - 2 - ((hotbar.width / 2) * setting.gui_scale) + ((hotbar.height - 2) * setting.gui_scale * (hotbar_slot_selected - 1)),
            hotbar_pos.y + setting.gui_scale}, /* TODO: revise gui_scale mod of selected hotbar position Y */
            (v2i16){setting.gui_scale, setting.gui_scale},
            0, 2, COL_TEXTURE_DEFAULT);

    draw_texture(texture_hud_widgets, hotbar_offhand,
            (v2i16){
            hotbar_pos.x - ((hotbar.width / 2) * setting.gui_scale) - (hotbar.height * 2 * setting.gui_scale),
            hotbar_pos.y + setting.gui_scale}, 
            (v2i16){setting.gui_scale, setting.gui_scale},
            0, 2, COL_TEXTURE_DEFAULT);

    if (!(flag & FLAG_DEBUG))
        draw_texture(texture_hud_widgets, crosshair,
                crosshair_pos, 
                (v2i16){setting.gui_scale, setting.gui_scale},
                0, 0, COL_TEXTURE_DEFAULT);

    rlEnd();
    rlSetTexture(0);
}

float get_str_width(Font font, const str* str, f32 font_size, f32 spacing)
{
    f32 result = 0;
    f32 text_offset_x = 0.0f;
    f32 scale_factor = font_size / font.baseSize;
    for (u16 i = 0; i < TextLength(str);)
    {
        i32 codepoint_byte_count = 0;
        u16 codepoint = GetCodepointNext(&str[i], &codepoint_byte_count);
        u8 index = GetGlyphIndex(font, codepoint);

        if (codepoint == '\n')
            text_offset_x = 0.0f;
        else
        {
            if (font.glyphs[index].advanceX == 0)
            {
                result += font.recs[index].width * scale_factor + spacing;
                text_offset_x += ((f32)font.recs[index].width * scale_factor + spacing);
            }
            else
            {
                result += font.glyphs[index].advanceX * scale_factor + spacing;
                text_offset_x += ((f32)font.glyphs[index].advanceX * scale_factor + spacing);
            }
        }
        i += codepoint_byte_count;
    }
    return result + 4;
}

/* 
 * raylib/rtextures.c/DrawTexturePro refactored;
 * scale = (source.scale * scl);
 * align_x = (0 = left, 1 = center, 2 = right);
 * align_y = (0 = top, 1 = center, 2 = bottom);
 */
void draw_texture(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, u8 align_x, u8 align_y, Color tint)
{
    if ((texture.id <= 0) || (scl.x <= 0.0f) || (scl.y <= 0.0f)
            || (source.width == 0.0f) || (source.height == 0.0f))
        return;

    switch (align_x)
    {
        case 1:
            pos.x -= ((source.width * scl.x) / 2);
            break;

        case 2:
            pos.x -= (source.width * scl.x);
            break;
    };

    switch (align_y)
    {
        case 1:
            pos.y -= ((source.height * scl.y) / 2);
            break;

        case 2:
            pos.y -= (source.height * scl.y);
            break;
    };

    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0, 0, 1);

    i32 tile_width = source.width * scl.x;
    i32 tile_height = source.height * scl.y;

    /* top left */
    rlTexCoord2f(source.x / texture.width, source.y / texture.height);
    rlVertex2f(pos.x, pos.y);

    /* bottom left */
    rlTexCoord2f(source.x / texture.width, (source.y + source.height) / texture.height);
    rlVertex2f(pos.x, pos.y + tile_height);

    /* bottom right */
    rlTexCoord2f((source.x + source.width) / texture.width, (source.y + source.height) / texture.height);
    rlVertex2f(pos.x + tile_width, pos.y + tile_height);

    /* top right */
    rlTexCoord2f((source.x + source.width) / texture.width, source.y / texture.height);
    rlVertex2f(pos.x + tile_width, pos.y);
}

/*jump*/
/* TODO: make draw_texture_tiled() */
/* 
 * raylib/examples/textures/textures_draw_tiled.c/DrawTextureTiled refactored;
 */
void draw_texture_tiled(Texture2D texture, Rectangle source, Rectangle dest, v2i16 pos, v2i16 scl, Color tint)
{
    if ((texture.id <= 0) || (scl.x <= 0.0f) || (scl.y <= 0.0f)
            || (source.width == 0.0f) || (source.height == 0.0f))
        return;

    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0, 0, 1);

    i32 tile_width = source.width*scl.x;
    i32 tile_height = source.height*scl.y;

    // top left
    rlTexCoord2f(source.x/texture.width, source.y/texture.height);
    rlVertex2f(pos.x, pos.y);

    // bottom left
    rlTexCoord2f(source.x/texture.width, (source.y + source.height)/texture.height);
    rlVertex2f(pos.x, pos.y + tile_height);
    
    // bottom right
    rlTexCoord2f((source.x + source.width)/texture.width, (source.y + source.height)/texture.height);
    rlVertex2f(pos.x + tile_width, pos.y + tile_height);

    // top right
    rlTexCoord2f((source.x + source.width)/texture.width, source.y/texture.height);
    rlVertex2f(pos.x + tile_width, pos.y);


    if ((dest.width < tile_width) && (dest.height < tile_height))
    {
        DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tile_width)*source.width, ((float)dest.height/tile_height)*source.height},
                    (Rectangle){dest.x, dest.y, dest.width, dest.height}, origin, rotation, tint);
    }
    else if (dest.width <= tile_width)
    {
        // Tiled vertically (one column)
        int dy = 0;
        for (;dy+tile_height < dest.height; dy += tile_height)
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tile_width)*source.width, source.height}, (Rectangle){dest.x, dest.y + dy, dest.width, (float)tile_height}, origin, rotation, tint);

        // Fit last tile
        if (dy < dest.height)
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)dest.width/tile_width)*source.width, ((float)(dest.height - dy)/tile_height)*source.height},
                        (Rectangle){dest.x, dest.y + dy, dest.width, dest.height - dy}, origin, rotation, tint);

    }
    else if (dest.height <= tile_height)
    {
        // Tiled horizontally (one row)
        int dx = 0;
        for (;dx+tile_width < dest.width; dx += tile_width)
            DrawTexturePro(texture, (Rectangle){source.x, source.y, source.width, ((float)dest.height/tile_height)*source.height}, (Rectangle){dest.x + dx, dest.y, (float)tile_width, dest.height}, origin, rotation, tint);

        // Fit last tile
        if (dx < dest.width)
        {
            DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tile_width)*source.width, ((float)dest.height/tile_height)*source.height},
                        (Rectangle){dest.x + dx, dest.y, dest.width - dx, dest.height}, origin, rotation, tint);
        }
    }
    else
    {
        // Tiled both horizontally and vertically (rows and columns)
        int dx = 0;
        for (;dx+tile_width < dest.width; dx += tile_width)
        {
            int dy = 0;
            for (;dy+tile_height < dest.height; dy += tile_height)
                DrawTexturePro(texture, source, (Rectangle){dest.x + dx, dest.y + dy, (float)tile_width, (float)tile_height}, origin, rotation, tint);

            if (dy < dest.height)
                DrawTexturePro(texture, (Rectangle){source.x, source.y, source.width, ((float)(dest.height - dy)/tile_height)*source.height},
                    (Rectangle){dest.x + dx, dest.y + dy, (float)tile_width, dest.height - dy}, origin, rotation, tint);
        }

        // Fit last column of tiles
        if (dx < dest.width)
        {
            int dy = 0;
            for (;dy+tile_height < dest.height; dy += tile_height)
                DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tile_width)*source.width, source.height},
                        (Rectangle){dest.x + dx, dest.y + dy, dest.width - dx, (float)tile_height}, origin, rotation, tint);

            // Draw final tile in the bottom right corner
            if (dy < dest.height)
                DrawTexturePro(texture, (Rectangle){source.x, source.y, ((float)(dest.width - dx)/tile_width)*source.width, ((float)(dest.height - dy)/tile_height)*source.height},
                    (Rectangle){dest.x + dx, dest.y + dy, dest.width - dx, dest.height - dy}, origin, rotation, tint);
        }
    }
}
*/

/* raylib/rtextures.c/DrawTexturePro refactored;
   scale = (scl); */
void draw_texture_simple(Texture2D texture, Rectangle source, v2i16 pos, v2i16 scl, Color tint)
{
    if (texture.id <= 0) return;
    f32 width = (f32)texture.width;
    f32 height = (f32)texture.height;

    Vector2 top_left =       (Vector2){pos.x,            pos.y};
    Vector2 bottom_right =   (Vector2){pos.x + scl.x,    pos.y + scl.y};

    rlSetTexture(texture.id);
    rlColor4ub(tint.r, tint.g, tint.b, tint.a);
    rlNormal3f(0.0f, 0.0f, 1.0f);

    rlTexCoord2f(source.x / width, source.y / height);
    rlVertex2f(top_left.x, top_left.y);
    rlTexCoord2f(source.x / width, (source.y + source.height) / height);
    rlVertex2f(top_left.x, bottom_right.y);
    rlTexCoord2f((source.x + source.width) / width, (source.y + source.height) / height);
    rlVertex2f(bottom_right.x, bottom_right.y);
    rlTexCoord2f((source.x + source.width) / width, source.y / height);
    rlVertex2f(bottom_right.x, top_left.y);
}

/* align_x = (0 = left, 1 = center, 2 = right);
   align_y = (0 = top, 1 = center, 2 = bottom); */
void draw_button(Texture2D texture, Rectangle button, v2i16 pos, u8 align_x, u8 align_y, u8 btn_state, void (*func)(), const str *str)
{
    switch (align_x)
    {
        case 1:
            pos.x -= ((button.width * setting.gui_scale) / 2);
            break;

        case 2:
            pos.x -= (button.width * setting.gui_scale);
            break;
    };

    switch (align_y)
    {
        case 1:
            pos.y -= ((button.height * setting.gui_scale) / 2);
            break;

        case 2:
            pos.y -= (button.height * setting.gui_scale);
            break;
    };

    if (buttons[btn_state])
    {
        if (is_range_within_f(cursor.x,
                    pos.x, pos.x + (button.width * setting.gui_scale))
                && is_range_within_f(cursor.y,
                    pos.y, pos.y + (button.height * setting.gui_scale)))
        {
            draw_texture(texture, button, pos,
                    (v2i16){setting.gui_scale, setting.gui_scale},
                    0, 0, ColorTint(COL_TEXTURE_DEFAULT, TINT_BUTTON_HOVER));

            if (IsMouseButtonPressed(0))
                func();
        }
        else
            draw_texture(texture, button, pos, 
                    (v2i16){setting.gui_scale, setting.gui_scale},
                    0, 0, COL_TEXTURE_DEFAULT);

        if (str)
            draw_text(font, str,
                    (v2i16){pos.x + ((button.width * setting.gui_scale) / 2), pos.y + ((button.height * setting.gui_scale) / 2)},
                    FONT_SIZE_DEFAULT, 1, align_x, align_y, COL_TEXT_DEFAULT);

    } else draw_texture(texture, button_inactive, pos, 
            (v2i16){setting.gui_scale, setting.gui_scale},
            0, 0, COL_TEXTURE_DEFAULT);
}

void btn_func_singleplayer()
{
    menu_index = 0; /* TODO: set actual value (MENU_SINGLEPLAYER) */
    state_menu_depth = 0; /* TODO: set actual value (2) */
    is_menu_ready = 0;
    flag &= ~FLAG_PAUSED; /*temp*/

    init_world("Poop Consistency Tester"); /*temp*/
}

void btn_func_multiplayer()
{
    menu_index = MENU_MULTIPLAYER;
    state_menu_depth = 2;
    is_menu_ready = 0;
}

void btn_func_settings()
{
    menu_index = MENU_SETTINGS;
    state_menu_depth = 2;
    is_menu_ready = 0;
}

void btn_func_quit_game()
{
    flag &= ~FLAG_ACTIVE;
}

void btn_func_unpause()
{
    menu_index = 0;
    state_menu_depth = 0;
    is_menu_ready = 0;
    flag &= ~FLAG_PAUSED;
    lily.flag &= ~FLAG_MENU_OPEN;
    lily.container_state = 0;
}

void btn_func_quit_world()
{
    menu_index = MENU_TITLE;
    state_menu_depth = 1;
    is_menu_ready = 0;
    /* TODO: save and unload world */
    flag &= ~FLAG_WORLD_LOADED;
}

void btn_func_back()
{
    menu_layer[state_menu_depth] = 0;
    --state_menu_depth;
    menu_index = menu_layer[state_menu_depth];
    is_menu_ready = 0;
}
#endif /* TODO: undef FUCK */
