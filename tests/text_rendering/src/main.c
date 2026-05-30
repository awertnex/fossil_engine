#include "deps/fossil/fossil_engine.h"

#define MARGIN 10

i32 scrool = 0;
fsl_key_bind bind_enter = {0};
fsl_key_bind bind_quit = {0};
fsl_font *font = NULL;
fsl_render *render = NULL;

static void callback_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;

    scrool = fsl_clamp_i32(scrool + (i32)yoffset * 4, 0, logger_core.cursor);
}

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, NULL, 1920, 1080, 0) != FSL_ERR_SUCCESS)
        goto cleanup;

    render = fsl_render_get();

    bind_enter = fsl_key_bind_init(FSL_KEY_ENTER, 0, 0, 0, 0, 0);
    bind_quit = fsl_key_bind_init(FSL_KEY_Q, 0, 0, 0, 0, 0);

    font = fsl_mem_handle_get(fsl_font_buf);
    font = &font[FSL_FONT_INDEX_DEJAVU_SANS_MONO];

    glfwSetScrollCallback(render->window, callback_scroll);

page_1:

    while (fsl_engine_running(NULL))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        fsl_text_start(font, 22.0f, 0, TRUE);

        fsl_text_push(fsl_stringf(
                    "FPS [%u]\n"
                    "POS [%.0lf %.0lf]\n"
                    "ENTER: Switch Workspace",
                    (u32)(1 / ((f64)render->time_delta * FSL_NSEC2SEC)),
                    render->mouse_pos.x, render->mouse_pos.y),
                    MARGIN, MARGIN, 0, 0,
                    render->size.x, FSL_DIAGNOSTIC_COLOR_SUCCESS);

        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
        fsl_ui_stop();
        fsl_fbo_blit(0);

        if (fsl_is_key_press(bind_enter))
            goto page_2;

        if (fsl_is_key_press(bind_quit))
            fsl_request_engine_close();
    }

page_2:

    while (fsl_engine_running(NULL))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        fsl_text_start(font, 22.0f, 0, TRUE);

        fsl_text_push(fsl_stringf(
                    "FPS [%u]",
                    (u32)(1 / ((f64)render->time_delta * FSL_NSEC2SEC))),
                    MARGIN, MARGIN, 0, 0,
                    render->size.x, FSL_DIAGNOSTIC_COLOR_SUCCESS);

        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);

        /* ---- draw logger strings ----------------------------------------- */

        i32 i = 0;
        u32 index = 0;
        fsl_log_entry *log_entry = fsl_mem_handle_get(logger_core.buf);
        for (i = 46; i > 0; --i)
        {
            index = fsl_mod_i32(logger_core.cursor - i - scrool, FSL_LOGGER_HISTORY_MAX);
            fsl_text_push(fsl_stringf("%s\n", log_entry[index].message),
                    MARGIN, render->size.y - MARGIN,
                    0, 0,
                    render->size.x, log_entry->color);
        }

        /* align once after all the strings' heights have accumulated */
        fsl_text_push("", 0, 0, 0, FSL_TEXT_ALIGN_BOTTOM, render->size.x, 0x00000000);
        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);

        fsl_ui_stop();
        fsl_fbo_blit(0);

        if (fsl_is_key_press(bind_enter))
            goto page_1;

        if (fsl_is_key_press(bind_quit))
            fsl_request_engine_close();
    }

cleanup:

    fsl_engine_close();
    return 0;
}
