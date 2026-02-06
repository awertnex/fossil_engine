#include <src/h/fossil_engine.h>

#define MARGIN 10

i32 scrool = 0;

static void callback_scroll(GLFWwindow *window, double xoffset, double yoffset);

static void callback_scroll(GLFWwindow *window, double xoffset, double yoffset)
{
    (void)window;
    (void)xoffset;

    scrool = fsl_clamp_i32(scrool + (i32)yoffset * 4, 0, fsl_logger_tab_index);
}

int main(int argc, char **argv)
{
    if (fsl_engine_init(argc, argv, NULL, NULL, 1920, 1080, NULL, 0) != FSL_ERR_SUCCESS)
        goto cleanup;

    fsl_ui_init(FALSE);

    glfwSetScrollCallback(render->window, callback_scroll);

page_1:

    while (fsl_engine_running())
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        fsl_text_start(&fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_MONO], 22.0f, 0, NULL, TRUE);

        fsl_text_push(fsl_stringf(
                    "FPS [%u]\n"
                    "POS [%.0lf %.0lf]\n"
                    "ENTER: Switch Workspace",
                    (u32)(1 / ((f64)render->time_delta * FSL_NSEC2SEC)),
                    render->mouse_pos.x, render->mouse_pos.y),
                    MARGIN, MARGIN, 0, 0,
                    FSL_DIAGNOSTIC_COLOR_SUCCESS);

        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
        fsl_text_stop();
        fsl_text_fbo_blit(0);

        if (fsl_is_key_press(FSL_KEY_ENTER))
            goto page_2;

        if (fsl_is_key_press(FSL_KEY_Q))
            fsl_request_engine_close();
    }

page_2:

    while (fsl_engine_running())
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        fsl_text_start(&fsl_font_buf[FSL_FONT_INDEX_DEJAVU_SANS_MONO], 22.0f, 0, NULL, TRUE);

        fsl_text_push(fsl_stringf(
                    "FPS [%u]",
                    (u32)(1 / ((f64)render->time_delta * FSL_NSEC2SEC))),
                    MARGIN, MARGIN, 0, 0,
                    FSL_DIAGNOSTIC_COLOR_SUCCESS);

        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);

        /* ---- draw logger strings ----------------------------------------- */

        i32 i = 0;
        u32 index = 0;
        for (i = 46; i > 0; --i)
        {
            index = fsl_mod_i32(fsl_logger_tab_index - i - scrool, FSL_LOGGER_HISTORY_MAX);
            fsl_text_push(fsl_stringf("%s\n", fsl_logger_tab[index]),
                    MARGIN, render->size.y - MARGIN,
                    0, 0,
                    fsl_logger_color[index]);
        }

        /* align once after all the strings' heights have accumulated */
        fsl_text_push("", 0, 0, 0, FSL_TEXT_ALIGN_BOTTOM, 0x00000000);
        fsl_text_render(TRUE, FSL_TEXT_COLOR_SHADOW);
        fsl_text_stop();

        fsl_ui_fbo_blit(0);

        if (fsl_is_key_press(FSL_KEY_ENTER))
            goto page_1;

        if (fsl_is_key_press(FSL_KEY_Q))
            fsl_request_engine_close();
    }

cleanup:

    fsl_engine_close();
    return 0;
}
