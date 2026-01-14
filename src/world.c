#include <string.h>

#include <engine/h/logger.h>
#include <engine/h/math.h>
#include <engine/h/string.h>
#include <engine/h/time.h>

#include "h/chunking.h"
#include "h/diagnostics.h"
#include "h/dir.h"
#include "h/gui.h"
#include "h/main.h"
#include "h/world.h"

WorldInfo world = {0};

u32 world_init(str *name, u64 seed, Player *p)
{
    world_dir_init(name);
    if (*GAME_ERR != ERR_SUCCESS && *GAME_ERR != ERR_WORLD_EXISTS)
        return *GAME_ERR;

    world_load(&world, name, seed);
    if (*GAME_ERR != ERR_SUCCESS && *GAME_ERR != ERR_WORLD_EXISTS)
        return *GAME_ERR;

    if (chunking_init() != ERR_SUCCESS)
        return *GAME_ERR;

    world.gravity = GRAVITY * 3.0f;

    set_player_spawn(p, 0, 0, 30);
    player_spawn(p, TRUE);
    player_chunk_update(p);

    core.flag.hud = 1;
    core.flag.world_loaded = 1;
    core.flag.chunk_buf_dirty = 1;
    disable_cursor;
    center_cursor;

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;
}

u32 world_dir_init(const str *world_name)
{
    str string[PATH_MAX] = {0};
    u32 i;

    if (!strlen(world_name))
    {
        LOGERROR(FALSE, ERR_POINTER_NULL, "%s\n", "World Name Cannot Be Empty");
        return *GAME_ERR;
    }

    if (!strlen(world_name))
    {
        *GAME_ERR = ERR_POINTER_NULL;
        return *GAME_ERR;
    }

    if (is_dir_exists(PATH_ROOT, TRUE) != ERR_SUCCESS)
    {
        LOGERROR(FALSE, ERR_WORLD_CREATION_FAIL,
                "Failed to Create World '%s', Root Directory Not Found\n", world_name);
        return *GAME_ERR;
    }

    if (is_dir_exists(DIR_ROOT[DIR_WORLDS], TRUE) != ERR_SUCCESS)
    {
        LOGERROR(FALSE, ERR_WORLD_CREATION_FAIL,
                "Failed to Create World '%s', World Directory Not Found\n", world_name);
        return *GAME_ERR;
    }

    snprintf(string, PATH_MAX, "%s%s", DIR_ROOT[DIR_WORLDS], world_name);
    check_slash(string);
    normalize_slash(string);

    if (is_dir_exists(string, FALSE) == ERR_SUCCESS)
    {
        snprintf(world.path, PATH_MAX, "%s", string);
        return *GAME_ERR;
    }

    make_dir(string);
    snprintf(world.path, PATH_MAX, "%s", string);

    LOGINFO(FALSE, "Creating World Directories '%s'..\n", world.path);

    for (i = 0; i < DIR_WORLD_COUNT; ++i)
    {
        snprintf(string, PATH_MAX, "%s%s", world.path, DIR_WORLD[i]);
        check_slash(string);
        normalize_slash(string);
        snprintf(DIR_WORLD[i], PATH_MAX, "%s", string);

        make_dir(string);
    }

    LOGINFO(FALSE, "Checking World Directories '%s'..\n", world.path);

    for (i = 0; i < DIR_WORLD_COUNT; ++i)
        if (is_dir_exists(DIR_WORLD[i], TRUE) != ERR_SUCCESS)
            return *GAME_ERR;

    LOGINFO(FALSE, "World Created '%s'\n", world_name);
    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;
}

u32 world_load(WorldInfo *world, const str *world_name, u64 seed)
{
    str string[2][PATH_MAX] = {0};
    str *file_contents = NULL;
    u64 file_len = 0;

    if (!strlen(world_name))
    {
        LOGERROR(FALSE, ERR_POINTER_NULL, "%s\n", "World Name Cannot Be Empty");
        return *GAME_ERR;
    }

    if (is_dir_exists(PATH_ROOT, TRUE) != ERR_SUCCESS)
    {
        LOGERROR(FALSE, ERR_WORLD_CREATION_FAIL,
                "Failed to Load World '%s', Root Directory Not Found\n", world_name);
        return *GAME_ERR;
    }

    if (is_dir_exists(DIR_ROOT[DIR_WORLDS], TRUE) != ERR_SUCCESS)
    {
        LOGERROR(FALSE, ERR_WORLD_CREATION_FAIL,
                "Failed to Load World '%s', 'worlds/' Directory Not Found\n", world_name);
        return *GAME_ERR;
    }

    snprintf(string[0], PATH_MAX, "%s%s", DIR_ROOT[DIR_WORLDS], world_name);
    if (is_dir_exists(string[0], TRUE) != ERR_SUCCESS)
    {
        LOGERROR(FALSE, ERR_WORLD_CREATION_FAIL,
                "Failed to Load World '%s', World Not Found\n", world_name);
        return *GAME_ERR;
    }

    /* ---- metadata i'll get back to, TODO: load other world metadata ------ */

    world->id = 0;

    snprintf(world->name, NAME_MAX, "%s", world_name);

    world->type = 0;

    /* ---- world seed ------------------------------------------------------ */

    snprintf(string[0], PATH_MAX, "%s%s/"FILE_NAME_WORLD_SEED, DIR_ROOT[DIR_WORLDS], world_name);
    if (is_file_exists(string[0], FALSE) == ERR_SUCCESS)
    {
        file_len = get_file_contents(string[0], (void*)&file_contents, 1, "rb", TRUE);
        if (*GAME_ERR != ERR_SUCCESS || !file_contents)
            return *GAME_ERR;
        seed = (u64)strtoul(file_contents, NULL, 10);
        mem_free((void*)&file_contents, file_len, "world_init().file_contents");
    }
    else
    {
        if (!seed)
            seed = rand_u64(get_time_raw_u64());

        convert_u64_to_str(string[1], NAME_MAX, seed);
        if (write_file(string[0], 1, strlen(string[1]),
                    &string[1], "wb", TRUE, TRUE) != ERR_SUCCESS)
            return *GAME_ERR;
    }

    world->seed = seed;

    /* ---- TODO: load the rest of world metadata --------------------------- */

    world->tick_start = 0;
    world->days = 0;
    world->drag.x = WORLD_DRAG_AIR;
    world->drag.y = WORLD_DRAG_AIR;
    world->drag.z = WORLD_DRAG_AIR;

    /* ---- other stuff ----------------------------------------------------- */

    debug_mode[DEBUG_MODE_CHUNK_GIZMO] = 1;

    *GAME_ERR = ERR_SUCCESS;
    return *GAME_ERR;
}

void world_update(Player *p)
{
    world.tick = world.tick_start + (u64)((f64)render->time * NANOSEC2SEC * WORLD_TICK_SPEED);
    world.days = world.tick / SET_DAY_TICKS_MAX;

    if (state_menu_depth || core.flag.super_debug)
        show_cursor;
    else disable_cursor;

    player_update(p, 1.0 - exp(-1.0 * (f64)render->time_delta * NANOSEC2SEC));
    b8 use_mouse = !state_menu_depth && !core.flag.super_debug && !(p->flag & FLAG_PLAYER_DEAD);
    player_camera_movement_update(p, render->mouse_delta, use_mouse);
    player_target_update(p);

    chunking_update(p->chunk, &p->chunk_delta);
    chunk_tab_index = get_chunk_index(p->chunk, p->target);

    update_projection_perspective(p->camera, &projection_world, FALSE);
    update_projection_perspective(p->camera_hud, &projection_hud, FALSE);
}
