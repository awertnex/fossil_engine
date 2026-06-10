#include "../../../fossil/deps/fossil/fossil_engine.h"
#include "../../../fossil/deps/fossil/ui/ui_core.h"

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
fsl_texture texture = {0};
fsl_ui_element element = {0};
v2f32 scale = {0};
v2i32 align = {0};

void ui_update(void)
{
    fsl_ui_start(FALSE, TRUE);
    ui_element_set_position(&element, render->mouse_pos.x, render->mouse_pos.y, 0, 0, 0, 0);
    ui_element_set_scale(&element, scale.x, scale.y);
    ui_element_set_alignment(&element, align.x, align.y);

    ui_element_draw(&element);
    fsl_ui_stop();

    fsl_fbo_blit(0);
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

    scale.x = 1.0f;
    scale.y = 1.0f;
    align.x = -1;
    align.y = -1;

    ui_element_set_texture(&element, &texture);
    ui_element_set_uv(&element, 0, 0, 177, 177);
    ui_element_set_position(&element, 0, 0, 0, 0, 0, 0);
    ui_element_set_size(&element, 0, 0, texture.size.x, texture.size.x);
    ui_element_set_scale(&element, scale.x, scale.y);
    ui_element_set_alignment(&element, align.x, align.y);

    while (fsl_engine_running(NULL))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        ui_update();

        if (fsl_is_key_hold(bind_bigger_x))
            scale.x += 0.1f;

        if (fsl_is_key_hold(bind_smaller_x))
            scale.x -= 0.1f;

        if (fsl_is_key_hold(bind_bigger_y))
            scale.y += 0.1f;

        if (fsl_is_key_hold(bind_smaller_y))
            scale.y -= 0.1f;

        if (fsl_is_key_press(bind_align_left))
            align.x += 1;
        if (fsl_is_key_press(bind_align_right))
            align.x -= 1;
        if (fsl_is_key_press(bind_align_up))
            align.y += 1;
        if (fsl_is_key_press(bind_align_down))
            align.y -= 1;

        if (fsl_is_key_press(bind_quit))
            fsl_request_engine_close();

        fsl_limit_framerate(60, render->time);
    }

cleanup:

    fsl_texture_free(&texture);
    fsl_engine_close();
    return 0;
}
