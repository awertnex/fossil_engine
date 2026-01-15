#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#include <engine/h/core.h>
#include <engine/h/platform.h>
#include <engine/h/dir.h>
#include <engine/h/logger.h>
#include <engine/h/math.h>
#include <engine/h/memory.h>
#include <engine/h/string.h>

#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/main.h"

str PATH_ROOT[PATH_MAX] = {0};
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

u32 paths_init(void)
{
    str *path_bin_root = NULL;
    u32 i;

    path_bin_root = get_path_bin_root();
    if (*GAME_ERR != ERR_SUCCESS)
        return *GAME_ERR;

    change_dir(path_bin_root);
    snprintf(PATH_ROOT, PATH_MAX, "%s", path_bin_root);
    mem_free((void*)&path_bin_root, strlen(path_bin_root),
            "paths_init().path_bin_root");

    LOGINFO(TRUE, "Creating Main Directories '%s'..\n", PATH_ROOT);

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (is_dir_exists(DIR_ROOT[i], FALSE) != ERR_SUCCESS)
            make_dir(DIR_ROOT[i]);

    LOGINFO(TRUE, "%s\n", "Checking Main Directories..");

    for (i = 0; i < DIR_ROOT_COUNT; ++i)
        if (!is_dir_exists(DIR_ROOT[i], TRUE) != ERR_SUCCESS)
            return *GAME_ERR;

    LOGINFO(TRUE, "Main Directory Created '%s'\n", PATH_ROOT);
    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;
}
