#ifndef HHC_WORLD_H
#define HHC_WORLD_H

#include "deps/fossil/common/types.h"
#include "deps/fossil/math/vector.h"

#include "../terrain/terrain.h"

#include "player.h"

#define WORLD_TICK_SPEED        20.0

#define WORLD_DRAG_AIR          0.2f
#define WORLD_DRAG_WATER        2.0f

typedef struct world_info
{
    u64 id;
    str name[FSL_ID_CAP];
    str path[FSL_PATH_CAP];
    u32 type; /* gamemode set at world creation */
    u64 seed;
    u64 tick;
    u64 tick_start;
    u64 days;
    hhc_terrain_noise_func terrain_noise_func;
    hhc_terrain_func terrain_func;

    f32 gravity;
    v3f32 drag;
} world_info;

/*!
 *  @brief info of current world loaded.
 *
 *  @remark declared internally in @ref world.c.
 */
extern world_info world;

/*!
 *  @brief initialize, create and load world.
 *
 *  @remark if `seed` is 0, a random seed will be generated.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 world_init(str *name, u64 seed, hhc_player *p);

/*!
 *  @brief initialize world directory structure.
 *
 *  create world directories needed on disk.
 *
 *  @remark called automatically from @ref world_init().
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 world_dir_init(const str *world_name);

/*!
 *  @brief load world and initialize files if not present.
 *
 *  load world data into memory if present and generate and write to disk if not:
 *  1. seed.txt, file containing world seed, can be changed,
 *     but changes only apply if world is loaded again.
 *
 *  @param seed if file 'seed.txt' not present, it will be created and `seed`
 *  will be written to it.
 *
 *  @remark if `seed` is 0, a random seed will be generated.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 world_load(world_info *world, const str *world_name, u64 seed);

void world_update(hhc_player *p);

/*!
 *  @brief write world state into disk on specific time intervals.
 *
 *  @return non-zero on failure and @ref *GAME_ERR is set accordingly.
 */
u32 world_save(hhc_player *p);

#endif /* HHC_WORLD_H */
