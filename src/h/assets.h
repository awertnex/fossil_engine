#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include <engine/h/types.h>
#include <engine/h/limits.h>
#include "dir.h"

#define FRICTION_BLOCK_SLIPPERY 0.02f
#define FRICTION_BLOCK_WET      0.1f
#define FRICTION_BLOCK_HARD     0.6f

enum TextureBlockIndex
{
    TEXTURE_BLOCK_GRASS_SIDE,
    TEXTURE_BLOCK_GRASS_TOP,
    TEXTURE_BLOCK_DIRT,
    TEXTURE_BLOCK_DIRTUP,
    TEXTURE_BLOCK_STONE,
    TEXTURE_BLOCK_SAND,
    TEXTURE_BLOCK_GLASS,
    TEXTURE_BLOCK_WOOD_BIRCH_LOG_SIDE,
    TEXTURE_BLOCK_WOOD_BIRCH_LOG_TOP,
    TEXTURE_BLOCK_WOOD_BIRCH_PLANKS,
    TEXTURE_BLOCK_WOOD_CHERRY_LOG_SIDE,
    TEXTURE_BLOCK_WOOD_CHERRY_LOG_TOP,
    TEXTURE_BLOCK_WOOD_CHERRY_PLANKS,
    TEXTURE_BLOCK_WOOD_OAK_LOG_SIDE,
    TEXTURE_BLOCK_WOOD_OAK_LOG_TOP,
    TEXTURE_BLOCK_WOOD_OAK_PLANKS,
    TEXTURE_BLOCK_BLOOD,
    TEXTURE_BLOCK_COUNT,
}; /* TextureBlockIndex */

enum BlockID
{
    BLOCK_NONE,
    BLOCK_GRASS,
    BLOCK_DIRT,
    BLOCK_DIRTUP,
    BLOCK_STONE,
    BLOCK_SAND,
    BLOCK_GLASS,
    BLOCK_WOOD_BIRCH_LOG,
    BLOCK_WOOD_BIRCH_PLANKS,
    BLOCK_WOOD_CHERRY_LOG,
    BLOCK_WOOD_CHERRY_PLANKS,
    BLOCK_WOOD_OAK_LOG,
    BLOCK_WOOD_OAK_PLANKS,
    BLOCK_BLOOD,
    BLOCK_COUNT,
}; /* BlockID */

enum BlockState
{
    BLOCK_STATE_SOLID = 1,
}; /* BlockState */

typedef struct Block
{
    str name[NAME_MAX];
    enum BlockState state;
    u32 texture_index[6]; /* px, nx, py, ny, pz, nz */
    f32 friction;
} Block;

extern Block *blocks;

/*! @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 assets_init(void);

void assets_free(void);

/*! @param index = index into global array 'block_textures'.
 *
 *  @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
u32 block_texture_init(u32 index, v2i32 size, str *name);

/*! @return non-zero on failure and '*GAME_ERR' is set accordingly.
 */
void blocks_init(void);

#endif /* GAME_ASSETS_H */
