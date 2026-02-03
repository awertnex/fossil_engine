#ifndef HHC_DIR_H
#define HHC_DIR_H

#include "src/h/types.h"
#include "src/h/dir.h"

#define DIR_MAX 128

enum directory_index
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
}; /* directory_index */

extern str DIR_ROOT[DIR_ROOT_COUNT][NAME_MAX];
extern str DIR_WORLD[DIR_WORLD_COUNT][NAME_MAX];

/*! @brief initialize game stuff.
 *
 *  @remark will create directories if not found next to the binary.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 game_init(void);

#endif /* HHC_DIR_H */
