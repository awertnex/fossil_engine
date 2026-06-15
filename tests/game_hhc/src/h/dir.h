#ifndef HHC_DIR_H
#define HHC_DIR_H

#include "deps/fossil/common/types.h"

#define DIR_MAX 128

enum dir_root_index
{
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
    DIR_SCREENSHOTS,
    DIR_TEXT,
    DIR_WORLDS,
    DIR_ROOT_COUNT
}; /* dir_root_index */

enum dir_world_index
{
    DIR_WORLD_CHUNKS,
    DIR_WORLD_ENTITIES,
    DIR_WORLD_PLAYER,
    DIR_WORLD_COUNT
}; /* dir_world_index */

extern str DIR_ROOT[DIR_ROOT_COUNT][FSL_ID_CAP];
extern str DIR_WORLD[DIR_WORLD_COUNT][FSL_ID_CAP];

/*!
 *  @brief initialize game stuff.
 *
 *  @remark will create directories if not found next to the binary.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 game_init(void);

#endif /* HHC_DIR_H */
