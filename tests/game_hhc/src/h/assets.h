#ifndef HHC_ASSETS_H
#define HHC_ASSETS_H

#include "common.h"
#include "dir.h"

#include "src/h/core.h"
#include "src/h/limits.h"
#include "src/h/shaders.h"
#include "src/h/types.h"

#define FRICTION_BLOCK_SLIPPERY 0.02f
#define FRICTION_BLOCK_WET      0.1f
#define FRICTION_BLOCK_HARD     0.6f

enum /* shader_index */
{
    SHADER_DEFAULT,
    SHADER_SKYBOX,
    SHADER_GIZMO,
    SHADER_GIZMO_CHUNK,
    SHADER_POST_PROCESSING,
    SHADER_VOXEL,
    SHADER_BOUNDING_BOX,
    SHADER_COUNT
}; /* shader_index */

enum /* texture_index */
{
    TEXTURE_CROSSHAIR,
    TEXTURE_ITEM_BAR,
    TEXTURE_SKYBOX_VAL,
    TEXTURE_SKYBOX_HORIZON,
    TEXTURE_SKYBOX_STARS,
    TEXTURE_SUN,
    TEXTURE_MOON,
    TEXTURE_COUNT
}; /* texture_index */

enum /* texture_block_index */
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
    TEXTURE_BLOCK_COUNT
}; /* texture_block_index */

enum block_id
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
    BLOCK_COUNT
}; /* block_id */

enum block_state
{
    BLOCK_STATE_SOLID = 1,
}; /* block_state */

typedef struct block
{
    fsl_asset asset;
    u32 texture_index[6]; /* px, nx, py, ny, pz, nz */
    enum block_state state;
    f32 friction;
} block;

extern fsl_texture *texture;
extern fsl_fbo *fbo;
extern fsl_mesh *mesh;
extern fsl_shader_program *shader;

extern block *blocks;

/*! @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 assets_init(void);

void assets_free(void);

/*! @param index = index into @ref block_textures.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 block_texture_init(u32 index, const str *name, const str *name_id, const str *file);

/*! @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
void blocks_init(void);

#endif /* HHC_ASSETS_H */
