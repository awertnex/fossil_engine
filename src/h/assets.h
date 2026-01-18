#ifndef GAME_ASSETS_H
#define GAME_ASSETS_H

#include <engine/h/core.h>
#include <engine/h/limits.h>
#include <engine/h/shaders.h>
#include <engine/h/types.h>

#include "common.h"
#include "dir.h"

#define FRICTION_BLOCK_SLIPPERY 0.02f
#define FRICTION_BLOCK_WET      0.1f
#define FRICTION_BLOCK_HARD     0.6f

enum /* ShaderIndex */
{
    SHADER_DEFAULT,
    SHADER_SKYBOX,
    SHADER_GIZMO,
    SHADER_GIZMO_CHUNK,
    SHADER_POST_PROCESSING,
    SHADER_VOXEL,
    SHADER_BOUNDING_BOX,
    SHADER_COUNT,
}; /* ShaderIndex */

enum /* TextureIndex */
{
    TEXTURE_CROSSHAIR,
    TEXTURE_ITEM_BAR,
    TEXTURE_SKYBOX_VAL,
    TEXTURE_SKYBOX_HORIZON,
    TEXTURE_SKYBOX_STARS,
    TEXTURE_SUN,
    TEXTURE_MOON,
    TEXTURE_COUNT,
}; /* TextureIndex */

enum /* TextureBlockIndex */
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

extern ShaderProgram shader[SHADER_COUNT];
extern Texture texture[TEXTURE_COUNT];

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
