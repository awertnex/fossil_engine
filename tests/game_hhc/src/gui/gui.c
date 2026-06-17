#include "deps/fossil/common/config.h"
#include "deps/fossil/common/diagnostics.h"
#include "deps/fossil/assets/asset_types.h"
#include "deps/fossil/assets/assets.h"
#include "deps/fossil/assets/mesh/mesh.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/shaders/shaders.h"

#include "../settings/settings.h"

#include "../h/common.h"
#include "../h/assets.h"
#include "../h/diagnostics.h"
#include "../h/main.h"

#include "gui.h"
#include "gui_menus.h"

#include <stdio.h>
#include <math.h>

#define UI_ITEM_PITCH -25.0f
#define UI_ITEM_YAW 50.0f
#define UI_ITEM_SCALE 16.0f

struct /* ui_item_data_internal */
{
    fsl_mesh mesh_unit_cube;
    fsl_vbo item_id_buf;
    fsl_shader_program shader;
    fsl_camera camera;
    f64 camera_distance;
} ui_item_data_internal = {0};

u32 menu_index_curr = 0;
u32 menu_layer[5] = {0};
u32 state_menu_depth = 0;
b8 is_menu_ready;
u32 buttons[BTN_COUNT];
fsl_ui_element ui_element[UI_ELEMENT_COUNT] = {0};

u32 gui_init(v2i32 render_size)
{
    u32 button_count = BTN_COUNT;

    if (fsl_mesh_load(&ui_item_data_internal.mesh_unit_cube,
                "Unit Cube", "unit_cube", "unit_cube.obj", GAME_DIR_NAME_MODELS) != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    glBindVertexArray(ui_item_data_internal.mesh_unit_cube.vao);

    if (fsl_vbo_init(&ui_item_data_internal.item_id_buf, 1, sizeof(GLuint), NULL,
                GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW) != FSL_ERR_SUCCESS)
        goto cleanup;

    glBindBuffer(GL_ARRAY_BUFFER, ui_item_data_internal.item_id_buf.id);

    glEnableVertexAttribArray(7);
    glVertexAttribIPointer(7, 1, GL_UNSIGNED_INT, sizeof(GLuint), (void*)0);
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (fsl_shader_program_init_ex(&ui_item_data_internal.shader, "UI Item", "ui_item",
                "ui_item.vert", NULL, "ui_item.frag", GAME_DIR_NAME_SHADERS) != FSL_ERR_SUCCESS)
        goto cleanup;

    ui_item_data_internal.camera.fovy = 35.0f;
    ui_item_data_internal.camera.fovy_smooth = 35.0f;
    ui_item_data_internal.camera.ratio = (f32)render_size.x / render_size.y;
    ui_item_data_internal.camera.far = FSL_CAMERA_CLIP_FAR_UI;
    ui_item_data_internal.camera.near = FSL_CAMERA_CLIP_NEAR_DEFAULT;

    ui_item_data_internal.camera_distance = 3.0;

    /*
    game_menu_pos = setting.render_size.y / 3; // TODO: figure this out
    menu_index_curr = MENU_TITLE;
     */

    while (button_count--)
        buttons[button_count] = 0;

    gui_update(render_size);
    gui_menus_init(render_size);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    gui_free();
    return *GAME_ERR;
}

void gui_update(v2i32 render_size)
{
    fsl_texture *texture_p = fsl_mem_handle_get(texture);

    /* element: crosshair */
    fsl_ui_element_set_texture(&ui_element[UI_ELEMENT_CROSSHAIR], &texture_p[TEXTURE_CROSSHAIR]);
    fsl_ui_element_set_uv(&ui_element[UI_ELEMENT_CROSSHAIR], 0, 0, 16, 16);
    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_CROSSHAIR],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, 0);
    fsl_ui_element_set_size(&ui_element[UI_ELEMENT_CROSSHAIR], 0, 0, 8, 8);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_CROSSHAIR],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_alignment(&ui_element[UI_ELEMENT_CROSSHAIR], 0, 0);

    /* element: hotbar */
    fsl_ui_element_set_texture(&ui_element[UI_ELEMENT_HOTBAR], &texture_p[TEXTURE_HOTBAR]);
    fsl_ui_element_set_uv(&ui_element[UI_ELEMENT_HOTBAR], 0, 0, 169, 16);
    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_HOTBAR],
            render_size.x / 2, render_size.y, 0, 0, 0, -4);
    fsl_ui_element_set_size(&ui_element[UI_ELEMENT_HOTBAR], 0, 0, 169, 16);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_HOTBAR],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_alignment(&ui_element[UI_ELEMENT_HOTBAR], 0, 1);

    /* element: hotbar selected */
    fsl_ui_element_set_texture(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], &texture_p[TEXTURE_HOTBAR]);
    fsl_ui_element_set_uv(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], 0, 16, 18, 18);
    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], 0, 0, 0, 0, -1, -1);
    fsl_ui_element_set_size(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], 0, 0, 18, 18);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_HOTBAR_SELECTED],
            settings.gui_scale, settings.gui_scale);
    fsl_ui_element_set_alignment(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], -1, -1);
    fsl_ui_element_attach(&ui_element[UI_ELEMENT_HOTBAR], &ui_element[UI_ELEMENT_HOTBAR_SELECTED]);

    /* element: container inventory survival */
    fsl_ui_element_set_texture(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL],
            &texture_p[TEXTURE_CONTAINER_INVENTORY_SURVIVAL]);
    fsl_ui_element_set_uv(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL], 0, 0, 177, 177);
    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL],
            render_size.x / 2, render_size.y / 2, 0, 0, 0, 0);
    fsl_ui_element_set_size(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL], 0, 0, 177, 177);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL],
            settings.gui_scale, settings.gui_scale);

    gui_menu_title_update(render_size);
}

void gui_free(void)
{
    fsl_mesh_free(&ui_item_data_internal.mesh_unit_cube);
    fsl_shader_program_free(&ui_item_data_internal.shader);
}

void gui_start_ui_items(v2i32 render_size)
{
    glUseProgram(ui_item_data_internal.shader.asset.id);

    ui_item_data_internal.camera.ratio = (f32)render_size.x / render_size.y;
    fsl_camera_movement_update(&ui_item_data_internal.camera,
            -ui_item_data_internal.camera_distance, 0.0, 0.0,
            0.0, 0.0, 0.0);
}

void gui_draw_ui_item(u32 item_id, f32 pos_x, f32 pos_y, v2i32 render_size)
{
    f32 pitch = UI_ITEM_PITCH;
    f32 yaw = UI_ITEM_YAW;
    f32 SPCH = 0.0f, CPCH = 0.0f, SYAW = 0.0f, CYAW = 0.0f;
    m4f32 transform = {0};
    m4f32 rotation_pitch = {0};
    m4f32 rotation_yaw = {0};
    m4f32 scale = {0};
    m4f32 offset = {0};

    SPCH = sinf(pitch * FSL_DEG2RAD);
    CPCH = cosf(pitch * FSL_DEG2RAD);
    SYAW = sinf(yaw * FSL_DEG2RAD);
    CYAW = cosf(yaw * FSL_DEG2RAD);

    rotation_pitch.a11 = CPCH;
    rotation_pitch.a13 = -SPCH;
    rotation_pitch.a22 = 1.0f;
    rotation_pitch.a31 = SPCH;
    rotation_pitch.a33 = CPCH;
    rotation_pitch.a44 = 1.0f;

    rotation_yaw.a11 = CYAW;
    rotation_yaw.a12 = -SYAW;
    rotation_yaw.a21 = SYAW;
    rotation_yaw.a22 = CYAW;
    rotation_yaw.a33 = 1.0f;
    rotation_yaw.a44 = 1.0f;

    scale.a11 = settings.gui_scale;
    scale.a22 = settings.gui_scale;
    scale.a33 = settings.gui_scale;
    scale.a44 = render_size.y / UI_ITEM_SCALE;

    offset.a11 = 1.0f;
    offset.a22 = 1.0f;
    offset.a33 = 1.0f;
    offset.a41 = ((f32)(-render_size.x + UI_ITEM_SCALE * settings.gui_scale) / 2.0 + pos_x) * render->ndc_scale.x;
    offset.a42 = ((f32)(-render_size.y + UI_ITEM_SCALE * settings.gui_scale) / 2.0 + pos_y) * render->ndc_scale.y;
    offset.a44 = 1.0f;

    /* 3D space */
    transform = fsl_matrix_multiply(rotation_yaw, rotation_pitch);
    transform = fsl_matrix_multiply(transform, ui_item_data_internal.camera.projection.perspective);

    /* UI space */
    transform = fsl_matrix_multiply(scale, transform);
    transform = fsl_matrix_multiply(transform, offset);

    glBindBuffer(GL_ARRAY_BUFFER, ui_item_data_internal.mesh_unit_cube.transform_buf.id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(m4f32), &transform, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, ui_item_data_internal.item_id_buf.id);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLuint), &item_id, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(ui_item_data_internal.mesh_unit_cube.vao);
    glDrawElementsInstanced(GL_TRIANGLES, ui_item_data_internal.mesh_unit_cube.index_buf.len,
        GL_UNSIGNED_INT, NULL, 1);
}

#ifdef FUCK /* TODO: undef FUCK */
void update_menus(v2f32 render_size)
{
    if (!menu_index_cur)
        return;

    switch (menu_index_cur)
    {
        case MENU_TITLE:
            if (!is_menu_ready)
            {
                menu_layer[state_menu_depth] = MENU_TITLE;
                menu_index_cur = MENU_TITLE;
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

/* align_x = (0 = left, 1 = center, 2 = right);
   align_y = (0 = top, 1 = center, 2 = bottom); */
void draw_button(Texture2D texture, Rectangle button, v2i16 pos,
        u8 align_x, u8 align_y, u8 btn_state, void (*func)(), const str *str)
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

void btn_func_unpause()
{
    menu_index_cur = 0;
    state_menu_depth = 0;
    is_menu_ready = 0;
    flag &= ~FLAG_PAUSED;
    lily.flag &= ~FLAG_MENU_OPEN;
    lily.container_state = 0;
}

void btn_func_quit_world()
{
    menu_index_cur = MENU_TITLE;
    state_menu_depth = 1;
    is_menu_ready = 0;
    /* TODO: save and unload world */
    flag &= ~FLAG_WORLD_LOADED;
}

void btn_func_back()
{
    menu_layer[state_menu_depth] = 0;
    --state_menu_depth;
    menu_index_cur = menu_layer[state_menu_depth];
    is_menu_ready = 0;
}
#endif /* TODO: undef FUCK */
