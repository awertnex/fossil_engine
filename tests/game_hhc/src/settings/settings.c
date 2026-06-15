#include "deps/fossil/common/diagnostics.h"
#include "deps/fossil/common/config.h"
#include "deps/fossil/memory/memory.h"

#include "deps/fossil/h/dir.h"

#include "../gui/gui.h"
#include "../super_debugger/super_debugger.h"

#include "../h/common.h"
#include "../h/diagnostics.h"
#include "../h/player.h"

#include "settings.h"

#include <stdio.h>
#include <string.h>

#define SETTINGS_FILE_SIZE_MAX (8 * 1024)

hhc_settings settings = {0};

u32 settings_init(void)
{
    str tokens[4][24] =
    {
        "mouse_sensitivity",
        "field_of_view",
        "render_distance",
        "target_fps",
    };
    str *file_contents_out = NULL;
    str *file_contents_in = NULL;

    if (fsl_mem_alloc((void*)&file_contents_out, SETTINGS_FILE_SIZE_MAX,
                "settings_init().file_contents_out") != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    if (fsl_is_dir_exists(GAME_DIR_NAME_CONFIG, TRUE) != FSL_ERR_SUCCESS)
        goto cleanup;

    snprintf(file_contents_out, SETTINGS_FILE_SIZE_MAX,
            "%s = %d\n"
            "%s = %d\n"
            "%s = %d\n"
            "%s = %d\n",
            tokens[0], SET_MOUSE_SENSITIVITY_DEFAULT,
            tokens[1], SET_FOV_DEFAULT,
            tokens[2], SET_RENDER_DISTANCE_DEFAULT,
            tokens[3], FSL_TARGET_FPS_DEFAULT);

    if (fsl_is_file_exists(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS, FALSE) != FSL_ERR_SUCCESS)
    {
        fsl_write_file(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS,
                strlen(file_contents_out),
                file_contents_out, TRUE, TRUE);
    }

    fsl_get_file_contents(GAME_DIR_NAME_CONFIG GAME_FILE_NAME_SETTINGS,
            (void*)&file_contents_in, TRUE);
    if (*GAME_ERR != FSL_ERR_SUCCESS)
        goto cleanup;

    settings.lerp_speed = SET_LERP_SPEED_DEFAULT;

    settings.render_distance = 8;
    settings.chunk_buf_radius = settings.render_distance;
    settings.chunk_buf_diameter = settings.chunk_buf_radius * 2 + 1;

    settings.chunk_buf_layer =
        settings.chunk_buf_diameter *
        settings.chunk_buf_diameter;

    settings.chunk_buf_volume =
        settings.chunk_buf_diameter *
        settings.chunk_buf_diameter *
        settings.chunk_buf_diameter;

    settings.chunk_tab_center =
        settings.chunk_buf_radius +
        settings.chunk_buf_radius * settings.chunk_buf_diameter +
        settings.chunk_buf_radius * settings.chunk_buf_layer;

    settings.reach_distance = PLAYER_REACH_DISTANCE_MAX;
    settings.mouse_sensitivity = SET_MOUSE_SENSITIVITY_DEFAULT * 0.004f;
    settings.font_size = 20.0f;
    settings.target_fps = 0;
    settings.fov = SET_FOV_DEFAULT;
    settings.anti_aliasing = TRUE;

    settings_gui_scale_set(SET_GUI_SCALE_DEFAULT);

    fsl_mem_free((void*)&file_contents_out, SETTINGS_FILE_SIZE_MAX,
            "settings_init().file_contents_out");
    if (file_contents_in)
        fsl_mem_free((void*)&file_contents_in, strlen(file_contents_in),
                "settings_init().file_contents_in");

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;

cleanup:

    fsl_mem_free((void*)&file_contents_out, SETTINGS_FILE_SIZE_MAX,
            "settings_init().file_contents_out");
    if (file_contents_in)
        fsl_mem_free((void*)&file_contents_in, strlen(file_contents_in),
                "settings_init().file_contents_in");
    return *GAME_ERR;
}

void settings_gui_scale_set(f32 scale)
{
    settings.gui_scale = scale;

    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_CROSSHAIR], scale, scale);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_HOTBAR], scale, scale);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_HOTBAR_SELECTED], scale, scale);
    fsl_ui_element_set_scale(&ui_element[UI_ELEMENT_CONTAINER_INVENTORY_SURVIVAL], scale, scale);

    super_debugger_gui_scale_set(scale);
}
