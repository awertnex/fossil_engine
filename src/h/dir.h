#ifndef GAME_DIR_H
#define GAME_DIR_H

#include <engine/h/types.h>
#include <engine/h/dir.h>

#define DIR_MAX 128

enum Directories
{
    /* ---- root directories ------------------------------------------------ */

    DIR_ASSETS,
    DIR_AUDIO,
    DIR_FONTS,
    DIR_LOOKUPS,
    DIR_MODELS,
    DIR_SHADERS,
    DIR_TEXTURES,
    DIR_BLOCKS,
    DIR_ENTITIES,
    DIR_ENV,
    DIR_GUI,
    DIR_ITEMS,
    DIR_LOGO,
    DIR_CONFIG,
    DIR_LOGS,
    DIR_SCREENSHOTS,
    DIR_TEXT,
    DIR_WORLDS,
    DIR_ROOT_COUNT,

    /* ---- world directories ----------------------------------------------- */

    DIR_WORLD_CHUNKS = 0,
    DIR_WORLD_ENTITIES,
    DIR_WORLD_PLAYER,
    DIR_WORLD_COUNT,
}; /* Directories */

extern str PATH_ROOT[PATH_MAX];
extern str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX];
extern str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX];

/*! @brief initialize all paths related to the game.
 *
 *  1. load the binary's root path into the global array 'PATH_ROOT'.
 *  2. add it to the global arrays 'DIR_ROOT' and 'DIR_WORLD' entries.
 *
 *  @remark will create all the directories if not found next to the binary.
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 paths_init(void);

#endif /* GAME_DIR_H */
