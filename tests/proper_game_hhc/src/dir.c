#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/logger.h"
#include "h/main.h"

#include <src/h/common.h>
#include <src/h/core.h>
#include <src/h/dir.h>
#include <src/h/math.h>
#include <src/h/memory.h>
#include <src/h/string.h>

#include <stdio.h>
#include <string.h>
#include <dirent.h>

str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX] =
{
    [DIR_ASSETS] =      GAME_DIR_NAME_ASSETS,
    [DIR_AUDIO] =       GAME_DIR_NAME_AUDIO,
    [DIR_FONTS] =       GAME_DIR_NAME_FONTS,
    [DIR_LOOKUPS] =     GAME_DIR_NAME_LOOKUPS,
    [DIR_MODELS] =      GAME_DIR_NAME_MODELS,
    [DIR_SHADERS] =     GAME_DIR_NAME_SHADERS,
    [DIR_TEXTURES] =    GAME_DIR_NAME_TEXTURES,
    [DIR_BLOCKS] =      GAME_DIR_NAME_BLOCKS,
    [DIR_ENTITIES] =    GAME_DIR_NAME_ENTITIES,
    [DIR_ENV] =         GAME_DIR_NAME_ENV,
    [DIR_GUI] =         GAME_DIR_NAME_GUI,
    [DIR_ITEMS] =       GAME_DIR_NAME_ITEMS,
    [DIR_LOGO] =        GAME_DIR_NAME_LOGO,
    [DIR_CONFIG] =      GAME_DIR_NAME_CONFIG,
    [DIR_LOGS] =        GAME_DIR_NAME_LOGS,
    [DIR_SCREENSHOTS] = GAME_DIR_NAME_SCREENSHOTS,
    [DIR_TEXT] =        GAME_DIR_NAME_TEXT,
    [DIR_WORLDS] =      GAME_DIR_NAME_WORLDS,
};

str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX] =
{
    [DIR_WORLD_CHUNKS] =    GAME_DIR_WORLD_NAME_CHUNKS,
    [DIR_WORLD_ENTITIES] =  GAME_DIR_WORLD_NAME_ENTITIES,
    [DIR_WORLD_PLAYER] =    GAME_DIR_WORLD_NAME_PLAYER,
};

u32 game_init(void)
{
    u32 i = 0;

    HHC_LOGTRACE(FSL_FLAG_LOG_CMD,
            "Creating Main Directories '%s'..\n", FSL_DIR_PROC_ROOT);

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (fsl_is_dir_exists(DIR_ROOT[i], FALSE) != FSL_ERR_SUCCESS)
        {
            fsl_make_dir(DIR_ROOT[i]);
            if (*GAME_ERR != FSL_ERR_SUCCESS && *GAME_ERR != FSL_ERR_DIR_EXISTS)
                return *GAME_ERR;
        }

    HHC_LOGTRACE(FSL_FLAG_LOG_CMD,
            "Main Directory Created '%s'\n", FSL_DIR_PROC_ROOT);

    glfwSwapInterval(MODE_INTERNAL_VSYNC);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glFrontFace(GL_CCW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_MULTISAMPLE);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}
