#include "../../../fossil/deps/fossil/fossil_engine.h"

fsl_render *render = {0};
fsl_key_bind bind_quit = {0};
fsl_key_bind bind_bigger_x = {0};
fsl_key_bind bind_smaller_x = {0};
fsl_key_bind bind_bigger_y = {0};
fsl_key_bind bind_smaller_y = {0};
fsl_key_bind bind_align_left = {0};
fsl_key_bind bind_align_right = {0};
fsl_key_bind bind_align_up = {0};
fsl_key_bind bind_align_down = {0};
fsl_key_bind bind_element_attach = {0};
fsl_texture texture = {0};
fsl_ui_element element_parent = {0};
fsl_ui_element element_child = {0};
v2f32 scale = {0};
v2i32 align = {0};

void ui_update(void)
{
    fsl_ui_element_set_position(&element_parent, render->mouse_pos.x, render->mouse_pos.y, 0, 0, 0, 0);
    fsl_ui_element_set_scale(&element_parent, scale.x, scale.y);
    fsl_ui_element_set_alignment(&element_parent, align.x, align.y);
}

void ui_draw(void)
{
    fsl_ui_start(FALSE, TRUE);
    fsl_ui_element_draw(&element_parent);
    fsl_ui_element_draw(&element_child);
    fsl_ui_stop();
    fsl_fbo_blit(0);
}

void resize(fsl_ui_event event, void *data)
{
    (void)data;
    fsl_ui_element_set_scale(event.caller, scale.x, scale.y);
    LOGSUCCESS(0, "ENTER\n");
}

void move(fsl_ui_event event, void *data)
{
    static i32 i = 0;

    (void)data;
    fsl_ui_element_set_position(event.caller, i, 0, 0, 0, 0, 0);
    ++i;

    LOGSUCCESS(0, fsl_logger_stringf("HOVER: %d\n", i));
}

void reset(fsl_ui_event event, void *data)
{
    (void)data;
    fsl_ui_element_set_position(event.caller, 0, 0, 0, 0, 0, 0);

    LOGSUCCESS(0, "LEAVE: 0\n");
}

void fuck(fsl_ui_event event, void *data)
{
    v2f64 pos = *(v2f64*)data;
    fsl_ui_element_set_position(event.caller, pos.x, pos.y, 0, 0, 0, 0);

    LOGSUCCESS(0, "CLICK\n");
}

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, NULL, 1280, 720, 0) != FSL_ERR_SUCCESS)
        goto cleanup;

    if (fsl_texture_init(&texture, "Panel", "panel", "panel.png", "assets/",
                GL_RGBA, GL_NEAREST, FSL_COLOR_CHANNELS_RGBA, FALSE, FALSE) != FSL_ERR_SUCCESS)
        goto cleanup;

    render = fsl_render_get();

    bind_quit = fsl_key_bind_init(FSL_KEY_Q, 0, 0, 0, 0, 0);
    bind_bigger_x = fsl_key_bind_init(FSL_KEY_A, 0, 0, 0, 0, 0);
    bind_smaller_x = fsl_key_bind_init(FSL_KEY_D, 0, 0, 0, 0, 0);
    bind_bigger_y = fsl_key_bind_init(FSL_KEY_W, 0, 0, 0, 0, 0);
    bind_smaller_y = fsl_key_bind_init(FSL_KEY_S, 0, 0, 0, 0, 0);
    bind_align_left = fsl_key_bind_init(FSL_KEY_LEFT, 0, 0, 0, 0, 0);
    bind_align_right = fsl_key_bind_init(FSL_KEY_RIGHT, 0, 0, 0, 0, 0);
    bind_align_up = fsl_key_bind_init(FSL_KEY_UP, 0, 0, 0, 0, 0);
    bind_align_down = fsl_key_bind_init(FSL_KEY_DOWN, 0, 0, 0, 0, 0);
    bind_element_attach = fsl_key_bind_init(FSL_KEY_ENTER, 0, 0, 0, 0, 0);

    scale.x = 1.0f;
    scale.y = 1.0f;
    align.x = -1;
    align.y = -1;

    fsl_ui_element_set_texture(&element_parent, &texture);
    fsl_ui_element_set_uv(&element_parent, 0, 0, 177, 177);
    fsl_ui_element_set_position(&element_parent, 0, 0, 0, 0, 0, 0);
    fsl_ui_element_set_size(&element_parent, 0, 0, 177, 177);
    fsl_ui_element_set_scale(&element_parent, scale.x, scale.y);
    fsl_ui_element_set_alignment(&element_parent, align.x, align.y);

    fsl_ui_element_set_texture(&element_child, &texture);
    fsl_ui_element_set_uv(&element_child, 0, 0, 177, 177);
    fsl_ui_element_set_position(&element_child, 0, 0, 0, 0, 0, 0);
    fsl_ui_element_set_size(&element_child, 0, 0, texture.size.x, texture.size.y);
    fsl_ui_element_set_scale(&element_child, 1.0f, 1.0f);
    fsl_ui_element_set_alignment(&element_child, -1, -1);
    fsl_ui_element_set_callback(&element_child, FSL_UI_EVENT_TYPE_ENTER, &resize, NULL);
    fsl_ui_element_set_callback(&element_child, FSL_UI_EVENT_TYPE_HOVER, &move, NULL);
    fsl_ui_element_set_callback(&element_child, FSL_UI_EVENT_TYPE_LEAVE, &reset, NULL);
    fsl_ui_element_set_callback(&element_child, FSL_UI_EVENT_TYPE_CLICK, &fuck, &render->mouse_pos);

    while (fsl_engine_running(NULL))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        if (render->mouse_delta.x || render->mouse_delta.y)
            ui_update();

        if (fsl_is_key_hold(bind_bigger_x))
        {
            scale.x += 0.1f;
            ui_update();
        }

        if (fsl_is_key_hold(bind_smaller_x))
        {
            scale.x -= 0.1f;
            ui_update();
        }

        if (fsl_is_key_hold(bind_bigger_y))
        {
            scale.y += 0.1f;
            ui_update();
        }

        if (fsl_is_key_hold(bind_smaller_y))
        {
            scale.y -= 0.1f;
            ui_update();
        }

        if (fsl_is_key_press(bind_align_left))
        {
            align.x += 1;
            ui_update();
        }

        if (fsl_is_key_press(bind_align_right))
        {
            align.x -= 1;
            ui_update();
        }

        if (fsl_is_key_press(bind_align_up))
        {
            align.y += 1;
            ui_update();
        }

        if (fsl_is_key_press(bind_align_down))
        {
            align.y -= 1;
            ui_update();
        }

        if (fsl_is_key_press(bind_element_attach))
        {
            if (element_child.parent)
                fsl_ui_element_detach(&element_child);
            else
                fsl_ui_element_attach(&element_parent, &element_child);

            ui_update();
        }

        ui_draw();

        if (fsl_is_key_press(bind_quit))
            fsl_request_engine_close();

        fsl_limit_framerate(60, render->time);
    }

cleanup:

    fsl_texture_free(&texture);
    fsl_engine_close();
    return 0;
}
