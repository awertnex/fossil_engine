#include "deps/fossil/common/limits.h"
#include "deps/fossil/common/session.h"
#include "deps/fossil/engine/engine.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/memory/memory.h"

#include "deps/fossil/h/dir.h"

#include "h/main.h"
#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"

#include <stdio.h>

str DIR_ROOT[DIR_ROOT_COUNT][FSL_ID_CAP] = {0};
str DIR_WORLD[DIR_WORLD_COUNT][FSL_ID_CAP] = {0};

u32 game_init(void)
{
    u32 i = 0;

    snprintf(DIR_ROOT[DIR_ASSETS], FSL_ID_CAP, "%s", GAME_DIR_NAME_ASSETS);
    snprintf(DIR_ROOT[DIR_AUDIO], FSL_ID_CAP, "%s", GAME_DIR_NAME_AUDIO);
    snprintf(DIR_ROOT[DIR_FONTS], FSL_ID_CAP, "%s", GAME_DIR_NAME_FONTS);
    snprintf(DIR_ROOT[DIR_LOOKUPS], FSL_ID_CAP, "%s", GAME_DIR_NAME_LOOKUPS);
    snprintf(DIR_ROOT[DIR_MODELS], FSL_ID_CAP, "%s", GAME_DIR_NAME_MODELS);
    snprintf(DIR_ROOT[DIR_SHADERS], FSL_ID_CAP, "%s", GAME_DIR_NAME_SHADERS);
    snprintf(DIR_ROOT[DIR_TEXTURES], FSL_ID_CAP, "%s", GAME_DIR_NAME_TEXTURES);
    snprintf(DIR_ROOT[DIR_BLOCKS], FSL_ID_CAP, "%s", GAME_DIR_NAME_BLOCKS);
    snprintf(DIR_ROOT[DIR_ENTITIES], FSL_ID_CAP, "%s", GAME_DIR_NAME_ENTITIES);
    snprintf(DIR_ROOT[DIR_ENV], FSL_ID_CAP, "%s", GAME_DIR_NAME_ENV);
    snprintf(DIR_ROOT[DIR_GUI], FSL_ID_CAP, "%s", GAME_DIR_NAME_GUI);
    snprintf(DIR_ROOT[DIR_ITEMS], FSL_ID_CAP, "%s", GAME_DIR_NAME_ITEMS);
    snprintf(DIR_ROOT[DIR_LOGO], FSL_ID_CAP, "%s", GAME_DIR_NAME_LOGO);
    snprintf(DIR_ROOT[DIR_CONFIG], FSL_ID_CAP, "%s", GAME_DIR_NAME_CONFIG);
    snprintf(DIR_ROOT[DIR_SCREENSHOTS], FSL_ID_CAP, "%s", GAME_DIR_NAME_SCREENSHOTS);
    snprintf(DIR_ROOT[DIR_TEXT], FSL_ID_CAP, "%s", GAME_DIR_NAME_TEXT);
    snprintf(DIR_ROOT[DIR_WORLDS], FSL_ID_CAP, "%s", GAME_DIR_NAME_WORLDS);

    snprintf(DIR_WORLD[DIR_WORLD_CHUNKS], FSL_ID_CAP, "%s", GAME_DIR_WORLD_NAME_CHUNKS);
    snprintf(DIR_WORLD[DIR_WORLD_ENTITIES], FSL_ID_CAP, "%s", GAME_DIR_WORLD_NAME_ENTITIES);
    snprintf(DIR_WORLD[DIR_WORLD_PLAYER], FSL_ID_CAP, "%s", GAME_DIR_WORLD_NAME_PLAYER);

    LOGTRACE(FSL_FLAG_LOG_CMD,
            fsl_logger_stringf("Creating Main Directories '%s'..\n", FSL_SESSION.bin_root));

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (fsl_is_dir_exists(DIR_ROOT[i], FALSE) != FSL_ERR_SUCCESS)
        {
            fsl_make_dir(DIR_ROOT[i]);
            if (*GAME_ERR != FSL_ERR_SUCCESS && *GAME_ERR != FSL_ERR_DIR_EXISTS)
                return *GAME_ERR;
        }

    LOGTRACE(FSL_FLAG_LOG_CMD,
            fsl_logger_stringf("Main Directory Created '%s'\n", FSL_SESSION.bin_root));

    if (fsl_mem_arena_init(&memory_arena_internal,
                "game_init().memory_arena_internal") != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    render = fsl_render_get();

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}
