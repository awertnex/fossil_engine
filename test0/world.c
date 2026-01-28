#include <string.h>

#include <src/h/common.h>
#include <src/h/dir.h>
#include <src/h/limits.h>
#include <src/h/math.h>
#include <src/h/string.h>
#include <src/h/time.h>

#include "h/chunking.h"
#include "h/common.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/logger.h"
#include "h/main.h"
#include "h/world.h"

world_info world = {0};

u32 world_init(str *name, u64 seed, player *p)
{
    world_dir_init(name);
    if (*GAME_ERR != FSL_ERR_SUCCESS && *GAME_ERR != HHC_ERR_WORLD_EXISTS)
        return *GAME_ERR;

    world_load(&world, name, seed);
    if (*GAME_ERR != FSL_ERR_SUCCESS && *GAME_ERR != HHC_ERR_WORLD_EXISTS)
        return *GAME_ERR;

    if (chunking_init() != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    world.gravity = FSL_GRAVITY * 3.0f;

    set_player_spawn(p, 0, 0, 30);
    player_spawn(p, TRUE);
    player_chunk_update(p);

    core.flag.hud = 1;
    core.flag.world_loaded = 1;
    core.flag.chunk_buf_dirty = 1;
    disable_cursor;
    center_cursor;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

u32 world_dir_init(const str *world_name)
{
    str string[PATH_MAX] = {0};
    u32 i;

    if (!strlen(world_name))
    {
        HHC_LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "%s\n", "World Name Cannot Be Empty");
        return *GAME_ERR;
    }

    if (!strlen(world_name))
    {
        *GAME_ERR = FSL_ERR_POINTER_NULL;
        return *GAME_ERR;
    }

    if (fsl_is_dir_exists(FSL_DIR_PROC_ROOT, TRUE) != FSL_ERR_SUCCESS)
    {
        HHC_LOGERROR(HHC_ERR_WORLD_CREATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "Failed to Create World '%s', Root Directory Not Found\n", world_name);
        return *GAME_ERR;
    }

    if (fsl_is_dir_exists(GAME_DIR_NAME_WORLDS, TRUE) != FSL_ERR_SUCCESS)
    {
        HHC_LOGERROR(HHC_ERR_WORLD_CREATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "Failed to Create World '%s', World Directory Not Found\n", world_name);
        return *GAME_ERR;
    }

    snprintf(string, PATH_MAX, GAME_DIR_NAME_WORLDS"%s", world_name);
    fsl_check_slash(string);
    fsl_normalize_slash(string);

    if (fsl_is_dir_exists(string, FALSE) == FSL_ERR_SUCCESS)
    {
        snprintf(world.path, PATH_MAX, "%s", string);
        return *GAME_ERR;
    }

    fsl_make_dir(string);
    snprintf(world.path, PATH_MAX, "%s", string);

    HHC_LOGINFO(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "Creating World Directories '%s'..\n", world.path);

    for (i = 0; i < DIR_WORLD_COUNT; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", world.path, DIR_WORLD[i]);
        fsl_make_dir(string);
        if (*GAME_ERR != FSL_ERR_SUCCESS && *GAME_ERR != FSL_ERR_DIR_EXISTS)
            return *GAME_ERR;
    }

    HHC_LOGINFO(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "World Created '%s'\n", world_name);
    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

u32 world_load(world_info *world, const str *world_name, u64 seed)
{
    str string[2][PATH_MAX] = {0};
    str *file_contents = NULL;
    u64 file_len = 0;

    if (!strlen(world_name))
    {
        HHC_LOGERROR(FSL_ERR_POINTER_NULL,
                FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "%s\n", "Failed to Load World, World Name Empty");
        return *GAME_ERR;
    }

    if (fsl_is_dir_exists(FSL_DIR_PROC_ROOT, TRUE) != FSL_ERR_SUCCESS)
    {
        HHC_LOGERROR(HHC_ERR_WORLD_CREATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "Failed to Load World '%s', Root Directory Not Found\n", world_name);
        return *GAME_ERR;
    }

    if (fsl_is_dir_exists(GAME_DIR_NAME_WORLDS, TRUE) != FSL_ERR_SUCCESS)
    {
        HHC_LOGERROR(HHC_ERR_WORLD_CREATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "Failed to Load World '%s', '"GAME_DIR_NAME_WORLDS"' Directory Not Found\n", world_name);
        return *GAME_ERR;
    }

    snprintf(string[0], PATH_MAX, GAME_DIR_NAME_WORLDS"%s", world_name);
    if (fsl_is_dir_exists(string[0], TRUE) != FSL_ERR_SUCCESS)
    {
        HHC_LOGERROR(HHC_ERR_WORLD_CREATION_FAIL,
                FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "Failed to Load World '%s', World Not Found\n", world_name);
        return *GAME_ERR;
    }

    /* ---- metadata i'll get back to, TODO: load other world metadata ------ */

    world->id = 0;

    snprintf(world->name, NAME_MAX, "%s", world_name);

    world->type = 0;

    /* ---- world seed ------------------------------------------------------ */

    snprintf(string[0], PATH_MAX, GAME_DIR_NAME_WORLDS"%s/"GAME_FILE_NAME_WORLD_SEED, world_name);
    if (fsl_is_file_exists(string[0], FALSE) == FSL_ERR_SUCCESS)
    {
        file_len = fsl_get_file_contents(string[0], (void*)&file_contents, 1, TRUE);
        if (*GAME_ERR != FSL_ERR_SUCCESS || !file_contents)
            return *GAME_ERR;
        seed = (u64)strtoul(file_contents, NULL, 10);
        fsl_mem_free((void*)&file_contents, file_len, "world_load().file_contents");
    }
    else
    {
        if (!seed)
            seed = fsl_rand_u64(fsl_get_time_raw_nsec());

        fsl_convert_u64_to_str(string[1], NAME_MAX, seed);
        if (fsl_write_file(string[0], 1, strlen(string[1]),
                    &string[1], TRUE, TRUE) != FSL_ERR_SUCCESS)
            return *GAME_ERR;
    }

    world->seed = seed;

    /* ---- TODO: load the rest of world metadata --------------------------- */

    world->tick_start = 7000;
    world->days = 0;
    world->drag.x = WORLD_DRAG_AIR;
    world->drag.y = WORLD_DRAG_AIR;
    world->drag.z = WORLD_DRAG_AIR;

    /* ---- other stuff ----------------------------------------------------- */

    core.debug.chunk_gizmo = 1;

    HHC_LOGINFO(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "World Loaded '%s'\n", world_name);

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

void world_update(player *p)
{
    world.tick = world.tick_start + (u64)((f64)render->time * FSL_NSEC2SEC * WORLD_TICK_SPEED);
    world.days = world.tick / SET_DAY_TICKS_MAX;

    if (state_menu_depth || core.flag.super_debug)
        show_cursor;
    else disable_cursor;

    player_update(p, 1.0 - exp(-1.0 * (f64)render->time_delta * FSL_NSEC2SEC));
    b8 use_mouse = !state_menu_depth && !core.flag.super_debug && !(p->flag & FLAG_PLAYER_DEAD);
    player_camera_movement_update(p, render->mouse_delta, use_mouse);
    player_target_update(p);

    chunking_update(p->ch, &p->ch_delta);
    chunk_tab_index = get_chunk_index(p->ch, p->target);

    fsl_update_projection_perspective(p->camera, &projection_world, FALSE);
    fsl_update_projection_perspective(p->camera_hud, &projection_hud, FALSE);
}
