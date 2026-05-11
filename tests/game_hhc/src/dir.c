#include "src/common/limits.h"
#include "src/common/session.h"
#include "src/logger/logger.h"
#include "src/memory/memory.h"

#include "src/h/core.h"
#include "src/h/dir.h"
#include "src/h/math.h"
#include "src/h/string.h"

#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/main.h"

#include <stdio.h>
#include <string.h>
#include <dirent.h>

str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX] = {0};
str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX] = {0};

u32 game_init(void)
{
    u32 i = 0;

    snprintf(DIR_ROOT[DIR_ASSETS], NAME_MAX, "%s", GAME_DIR_NAME_ASSETS);
    snprintf(DIR_ROOT[DIR_AUDIO], NAME_MAX, "%s", GAME_DIR_NAME_AUDIO);
    snprintf(DIR_ROOT[DIR_FONTS], NAME_MAX, "%s", GAME_DIR_NAME_FONTS);
    snprintf(DIR_ROOT[DIR_LOOKUPS], NAME_MAX, "%s", GAME_DIR_NAME_LOOKUPS);
    snprintf(DIR_ROOT[DIR_MODELS], NAME_MAX, "%s", GAME_DIR_NAME_MODELS);
    snprintf(DIR_ROOT[DIR_SHADERS], NAME_MAX, "%s", GAME_DIR_NAME_SHADERS);
    snprintf(DIR_ROOT[DIR_TEXTURES], NAME_MAX, "%s", GAME_DIR_NAME_TEXTURES);
    snprintf(DIR_ROOT[DIR_BLOCKS], NAME_MAX, "%s", GAME_DIR_NAME_BLOCKS);
    snprintf(DIR_ROOT[DIR_ENTITIES], NAME_MAX, "%s", GAME_DIR_NAME_ENTITIES);
    snprintf(DIR_ROOT[DIR_ENV], NAME_MAX, "%s", GAME_DIR_NAME_ENV);
    snprintf(DIR_ROOT[DIR_GUI], NAME_MAX, "%s", GAME_DIR_NAME_GUI);
    snprintf(DIR_ROOT[DIR_ITEMS], NAME_MAX, "%s", GAME_DIR_NAME_ITEMS);
    snprintf(DIR_ROOT[DIR_LOGO], NAME_MAX, "%s", GAME_DIR_NAME_LOGO);
    snprintf(DIR_ROOT[DIR_CONFIG], NAME_MAX, "%s", GAME_DIR_NAME_CONFIG);
    snprintf(DIR_ROOT[DIR_SCREENSHOTS], NAME_MAX, "%s", GAME_DIR_NAME_SCREENSHOTS);
    snprintf(DIR_ROOT[DIR_TEXT], NAME_MAX, "%s", GAME_DIR_NAME_TEXT);
    snprintf(DIR_ROOT[DIR_WORLDS], NAME_MAX, "%s", GAME_DIR_NAME_WORLDS);

    snprintf(DIR_WORLD[DIR_WORLD_CHUNKS], NAME_MAX, "%s", GAME_DIR_WORLD_NAME_CHUNKS);
    snprintf(DIR_WORLD[DIR_WORLD_ENTITIES], NAME_MAX, "%s", GAME_DIR_WORLD_NAME_ENTITIES);
    snprintf(DIR_WORLD[DIR_WORLD_PLAYER], NAME_MAX, "%s", GAME_DIR_WORLD_NAME_PLAYER);

    LOGTRACE(FSL_FLAG_LOG_CMD,
            fsl_logger_stringf("Creating Main Directories '%s'..\n", FSL_DIR_PROC_ROOT));

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (fsl_is_dir_exists(DIR_ROOT[i], FALSE) != FSL_ERR_SUCCESS)
        {
            fsl_make_dir(DIR_ROOT[i]);
            if (*GAME_ERR != FSL_ERR_SUCCESS && *GAME_ERR != FSL_ERR_DIR_EXISTS)
                return *GAME_ERR;
        }

    LOGTRACE(FSL_FLAG_LOG_CMD,
            fsl_logger_stringf("Main Directory Created '%s'\n", FSL_DIR_PROC_ROOT));

    if (fsl_mem_arena_init(&memory_arena_internal,
                "game_init().memory_arena_internal") != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}
