#include "deps/fossil/common/config.h"
#include "deps/fossil/assets/assets.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/math/vector.h"
#include "deps/fossil/memory/memory.h"
#include "deps/fossil/physics/collision.h"
#include "deps/fossil/physics/transform.h"

#include "deps/fossil/h/time.h"

#include "chunking/chunking.h"
#include "gui/gui.h"
#include "settings/settings.h"

#include "h/main.h"
#include "h/config_internal.h"
#include "h/common.h"
#include "h/diagnostics.h"
#include "h/player.h"
#include "h/world.h"

#include <stdio.h>
#include <math.h>

/*!
 *  @internal
 *
 *  @brief handle player being near or past a world edge.
 *
 *  - teleport player to the other side of the world if they cross a world edge.
 *  - adjust player chunk delta so it looks natural to the chunking system,
 *    since if player chunk delta is too big, the chunk system will dump all
 *    chunks and parse them again.
 */
static void player_world_overflow_update(hhc_player *p);

u32 player_init(hhc_player *p, const str *name)
{
    hhc_player noplayer = {0};

    *p = noplayer;

    if (fsl_mesh_load(&p->mesh, "Player", "player", "player.obj", GAME_DIR_NAME_MODELS) != FSL_ERR_SUCCESS)
        return *GAME_ERR;

    snprintf(p->name, FSL_ID_CAP, "%s", name);
    p->size.x = 0.6f;
    p->size.y = 0.6f;
    p->size.z = 1.8f;
    p->transform.scale.x = 1.0;
    p->transform.scale.y = 1.0;
    p->transform.scale.z = 1.0;
    p->eye_height = PLAYER_EYE_HEIGHT;
    fsl_kinematics_mass_set(&p->kn, 4.0);
    p->camera_distance = SET_CAMERA_DISTANCE_MAX;
    p->camera_mode = PLAYER_CAMERA_MODE_1ST_PERSON;

    p->menu_state = 0;
    p->hotbar_slots[0].id = BLOCK_GRASS;
    p->hotbar_slots[1].id = BLOCK_DIRT;
    p->hotbar_slots[2].id = BLOCK_STONE;
    p->hotbar_slots[3].id = BLOCK_SAND;
    p->hotbar_slots[4].id = BLOCK_GLASS;
    p->hotbar_slots[5].id = BLOCK_WOOD_OAK_LOG;
    p->hotbar_slots[6].id = BLOCK_WOOD_BIRCH_LOG;
    p->hotbar_slots[7].id = BLOCK_WOOD_CHERRY_LOG;

    p->hotbar_slots[0].count = 1;
    p->hotbar_slots[1].count = 1;
    p->hotbar_slots[2].count = 1;
    p->hotbar_slots[3].count = 1;
    p->hotbar_slots[4].count = 1;
    p->hotbar_slots[5].count = 1;
    p->hotbar_slots[6].count = 1;
    p->hotbar_slots[7].count = 1;

    p->camera.fovy = settings.fov;
    p->camera.fovy_smooth = 0.0f;
    p->camera.ratio = (f32)render->size.x / render->size.y;
    p->camera.far = FSL_CAMERA_CLIP_FAR_OPTIMAL;
    p->camera.near = FSL_CAMERA_CLIP_NEAR_DEFAULT;

    p->camera_hud.fovy = (f32)SET_FOV_DEFAULT;
    p->camera_hud.fovy_smooth = (f32)SET_FOV_DEFAULT;
    p->camera_hud.ratio = (f32)render->size.x / render->size.y;
    p->camera_hud.far = FSL_CAMERA_CLIP_FAR_UI;
    p->camera_hud.near = FSL_CAMERA_CLIP_NEAR_DEFAULT;

    *GAME_ERR = FSL_ERR_SUCCESS;
    return *GAME_ERR;
}

void player_update(hhc_player *p, f64 dt)
{
    p->flag &= ~FLAG_PLAYER_CAN_JUMP;
    p->acceleration_rate = PLAYER_ACCELERATION_WALK;
    p->camera.fovy = settings.fov;

    /* ---- player flags ---------------------------------------------------- */

    if (p->flag & FLAG_PLAYER_FLYING)
    {
        p->kn_forces[ENTITY_KINEMATICS_ENV].acceleration.z = 0.0;
        p->kn_forces[ENTITY_KINEMATICS_ENV].velocity.z = 0.0;
        p->acceleration_rate = PLAYER_ACCELERATION_FLY;

        if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        {
            p->physics_material.drag.x = PLAYER_DRAG_FLY_NATURAL;
            p->physics_material.drag.y = PLAYER_DRAG_FLY_NATURAL;
            p->physics_material.drag.z = PLAYER_DRAG_FLY_NATURAL;
        }
        else
        {
            p->physics_material.drag.x = PLAYER_DRAG_FLYING;
            p->physics_material.drag.y = PLAYER_DRAG_FLYING;
            p->physics_material.drag.z = PLAYER_DRAG_FLYING_V;
        }

        p->camera.fovy += 10.0f;

        if (p->flag & FLAG_PLAYER_SPRINTING)
        {
            p->acceleration_rate = PLAYER_ACCELERATION_FLY_FAST;
            p->camera.fovy += 10.0f;
        }
    }
    else
    {
        p->kn_forces[ENTITY_KINEMATICS_ENV].acceleration.z = world.gravity.z * p->kn.mass;
        p->physics_material.drag.x = PLAYER_DRAG_DEFAULT;
        p->physics_material.drag.y = PLAYER_DRAG_DEFAULT;
        p->physics_material.drag.z = 0.0;

        if (p->flag & FLAG_PLAYER_SNEAKING)
            p->acceleration_rate = PLAYER_ACCELERATION_SNEAK;
        else if (p->flag & FLAG_PLAYER_SPRINTING)
        {
            p->acceleration_rate = PLAYER_ACCELERATION_SPRINT;
            p->camera.fovy += 5.0f;
        }
    }

    if (p->flag & FLAG_PLAYER_ZOOMER && p->camera.zoom)
        p->camera.fovy = settings.fov - p->camera.zoom;

    p->force.x *= p->acceleration_rate * p->physics_material.drag.x;
    p->force.y *= p->acceleration_rate * p->physics_material.drag.y;
    p->force.z *= p->acceleration_rate * p->physics_material.drag.z;

    fsl_kinematics_update_v3f64(&p->kn_forces[ENTITY_KINEMATICS_CONTROL], p->force,
            &p->physics_material, p->kn.mass_inv, dt);

    fsl_kinematics_update_v3f64(&p->kn_forces[ENTITY_KINEMATICS_ENV],
            p->kn_forces[ENTITY_KINEMATICS_ENV].acceleration, &world.physics_material,
            p->kn.mass_inv, dt);

    p->kn.velocity = fsl_kinematics_velocity_get(p->kn_forces, ENTITY_KINEMATICS_COUNT);
    p->transform.pos.x += p->kn.velocity.x * dt;
    p->transform.pos.y += p->kn.velocity.y * dt;
    p->transform.pos.z += p->kn.velocity.z * dt;

    p->kn.speed = sqrt(fsl_len_v3f64(p->kn.velocity));
    if (p->kn.speed > FSL_EPSILON)
        p->camera.fovy += p->kn.speed * 0.03f;
    p->camera.fovy = fsl_clamp_f32(p->camera.fovy, 1.0f, SET_FOV_MAX);
    p->camera.fovy_smooth = fsl_lerp_exp_f32(p->camera.fovy_smooth, p->camera.fovy,
                SET_LERP_SPEED_FOV_MODE, dt);

    player_bounding_box_update(p);
    if (MODE_INTERNAL_COLLIDE)
        player_collision_update(p, dt);
    player_world_overflow_update(p);
}

void player_hotbar_selected_set(hhc_player *p, u32 index)
{
    p->hotbar_slot_selected = index;

    fsl_ui_element_set_position(&ui_element[UI_ELEMENT_HOTBAR_SELECTED],
            0, 0, 0, 0, p->hotbar_slot_selected * 17 - 1, -1);
}

void player_hotbar_selected_advance(hhc_player *p, i32 delta)
{
    i32 index = p->hotbar_slot_selected + delta;

    if (index >= CONTAINER_HOTBAR_SLOTS_MAX)
        index = 0;
    else if (index < 0)
        index = CONTAINER_HOTBAR_SLOTS_MAX - 1;

    player_hotbar_selected_set(p, index);
}


void player_collision_update(hhc_player *p, f64 dt)
{
    hhc_chunk *ch = NULL;
    hhc_block *block_p = fsl_mem_handle_get(blocks);
    u32 *block = NULL;
    f64 speed;
    v3f64 velocity = {0};
    v3f64 displacement = {0};
    fsl_physics_material *block_physics_material = NULL;
    fsl_bounding_box block_box = {0};
    fsl_bounding_box collision_capsule = {0};
    i32 i = 0;
    i32 x = 0;
    i32 y = 0;
    i32 z = 0;
    v3i32 MIN = {0};
    v3i32 MAX = {0};
    v3i32 START = {0};
    v3i32 INCREMENT = {1, 1, 1};
    b8 resolved = TRUE;
    u32 max_axis = 0;

    velocity.x = p->kn.velocity.x;
    velocity.y = p->kn.velocity.y;
    velocity.z = p->kn.velocity.z;
    displacement.x = velocity.x * dt;
    displacement.y = velocity.y * dt;
    displacement.z = velocity.z * dt;
    collision_capsule = make_collision_capsule(p->bbox, p->ch, displacement);
    MIN.x = (i32)collision_capsule.pos.x;
    MIN.y = (i32)collision_capsule.pos.y;
    MIN.z = (i32)collision_capsule.pos.z;
    MAX.x = (i32)(collision_capsule.pos.x + collision_capsule.size.x);
    MAX.y = (i32)(collision_capsule.pos.y + collision_capsule.size.y);
    MAX.z = (i32)(collision_capsule.pos.z + collision_capsule.size.z);

    if (velocity.x < 0.0)
    {
        START.x = MAX.x - 1;
        INCREMENT.x = -1;
    }
    else START.x = MIN.x;

    if (velocity.y < 0.0)
    {
        START.y = MAX.y - 1;
        INCREMENT.y = -1;
    }
    else START.y = MIN.y;

    if (velocity.z < 0.0)
    {
        START.z = MAX.z - 1;
        INCREMENT.z = -1;
    }
    else START.z = MIN.z;

    for (i = 0; i < 2 && resolved; ++i)
    {
        resolved = FALSE;
        for (z = START.z; z >= MIN.z && z < MAX.z; z += INCREMENT.z)
        {
            for (y = START.y; y >= MIN.y && y < MAX.y; y += INCREMENT.y)
            {
                for (x = START.x; x >= MIN.x && x < MAX.x; x += INCREMENT.x)
                {
                    ch = get_chunk_resolved(settings.chunk_tab_center, x, y, z);
                    if (!ch || !(ch->flag & FLAG_CHUNK_GENERATED))
                        continue;

                    block = get_block_resolved(ch, x, y, z);
                    if (!block || !*block)
                        continue;

                    block_box.pos.x = (f64)(ch->pos_world.x * CHUNK_DIAMETER +
                            fsl_mod_i32(x, CHUNK_DIAMETER));
                    block_box.pos.y = (f64)(ch->pos_world.y * CHUNK_DIAMETER +
                            fsl_mod_i32(y, CHUNK_DIAMETER));
                    block_box.pos.z = (f64)(ch->pos_world.z * CHUNK_DIAMETER +
                            fsl_mod_i32(z, CHUNK_DIAMETER));
                    block_box.size.x = 1.0;
                    block_box.size.y = 1.0;
                    block_box.size.z = 1.0;

                    p->collision_info = fsl_get_swept_aabb(p->bbox, block_box, displacement);

                    if (p->collision_info.hit)
                    {
                        /* ---- resolution ---------------------------------- */

                        p->transform.pos.x += displacement.x * p->collision_info.entry_time +
                            p->collision_info.normal.x * FSL_COLLISION_EPSILON;

                        p->transform.pos.y += displacement.y * p->collision_info.entry_time +
                            p->collision_info.normal.y * FSL_COLLISION_EPSILON;

                        p->transform.pos.z += displacement.z * p->collision_info.entry_time +
                            p->collision_info.normal.z * FSL_COLLISION_EPSILON;

                        if (p->collision_info.slide)
                        {
                            displacement.x -= p->collision_info.dot * p->collision_info.normal.x;
                            displacement.y -= p->collision_info.dot * p->collision_info.normal.y;
                            displacement.z -= p->collision_info.dot * p->collision_info.normal.z;

                            p->collision_info.dot = fsl_dot_v3f64(p->kn_forces[0].velocity, p->collision_info.normal);
                            p->kn_forces[0].velocity.x -= p->collision_info.dot * p->collision_info.normal.x;
                            p->kn_forces[0].velocity.y -= p->collision_info.dot * p->collision_info.normal.y;
                            p->kn_forces[0].velocity.z -= p->collision_info.dot * p->collision_info.normal.z;

                            p->collision_info.dot = fsl_dot_v3f64(p->kn_forces[1].velocity, p->collision_info.normal);
                            p->kn_forces[1].velocity.x -= p->collision_info.dot * p->collision_info.normal.x;
                            p->kn_forces[1].velocity.y -= p->collision_info.dot * p->collision_info.normal.y;
                            p->kn_forces[1].velocity.z -= p->collision_info.dot * p->collision_info.normal.z;
                        }

                        p->transform.pos.x -= displacement.x * p->collision_info.entry_time;
                        p->transform.pos.y -= displacement.y * p->collision_info.entry_time;
                        p->transform.pos.z -= displacement.z * p->collision_info.entry_time;

                        p->kn.velocity = fsl_kinematics_velocity_get(p->kn_forces, ENTITY_KINEMATICS_COUNT);

                        if (p->collision_info.normal.z > 0.0)
                        {
                            if (!(p->flag & FLAG_PLAYER_CINEMATIC_MOTION))
                                p->flag &= ~FLAG_PLAYER_FLYING;
                            p->flag |= FLAG_PLAYER_CAN_JUMP;

                            block_physics_material = &block_p[GET_BLOCK_ID(*block)].physics_material;
                        }

                        player_bounding_box_update(p);

                        speed = p->kn.speed;
                        p->kn.speed = sqrt(fsl_len_v3f64(p->kn.velocity));
#if MODE_INTERNAL_DIE
                        speed -= p->kn.speed;
                        printf("DAMAGE: %f, HEALTH %f\n", speed, p->health);
                        if (speed > PLAYER_COLLISION_DAMAGE_THRESHOLD)
                        {
                            p->health -= speed;

                            if (p->health <= 0.0 && !(p->flag & FLAG_PLAYER_DEAD))
                            {
                                max_axis = fsl_max_axis_v3f64(p->collision_info.normal);
                                if (max_axis == 1 || max_axis == 2)
                                    p->death = PLAYER_DEATH_REASON_COLLISION_WALL;
                                else if (max_axis == 3)
                                {
                                    if (p->collision_info.normal.z > 0.0)
                                        p->death = PLAYER_DEATH_REASON_COLLISION_FLOOR;
                                    else
                                        p->death = PLAYER_DEATH_REASON_COLLISION_CEILING;
                                }

                                player_kill(p);
                            }
                        }
#endif /* MODE_INTERNAL_DIE */

                        resolved = TRUE;
                    }
                }
            }
        }
    }
}

void player_bounding_box_update(hhc_player *p)
{
    p->bbox.pos.x = p->transform.pos.x - p->size.x * 0.5f;
    p->bbox.pos.y = p->transform.pos.y - p->size.y * 0.5f;
    p->bbox.pos.z = p->transform.pos.z;
    p->bbox.size.x = p->size.x;
    p->bbox.size.y = p->size.y;
    p->bbox.size.z = p->size.z;
}

fsl_bounding_box make_collision_capsule(fsl_bounding_box b, v3i32 ch, v3f64 velocity)
{
    const f64 PADDING = 1.0f;
    v3f64 pos = {0};
    v3f64 size = {0};
    v3f64 delta = {0};
    fsl_bounding_box result = {0};

    if (velocity.x > 0.0f)
    {
        pos.x = b.pos.x - b.size.x * 0.5f - PADDING;
        delta.x = (pos.x - ceil(pos.x));
        size.x = b.size.x + velocity.x + delta.x + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.x = b.pos.x + velocity.x - b.size.x * 0.5f - PADDING;
        delta.x = (pos.x - ceil(pos.x));
        size.x = b.size.x - velocity.x + delta.x + PADDING * 2.0f + 1.0f;
    }

    if (velocity.y > 0.0f)
    {
        pos.y = b.pos.y - b.size.y * 0.5f - PADDING;
        delta.y = (pos.y - ceil(pos.y));
        size.y = b.size.y + velocity.y + delta.y + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.y = b.pos.y + velocity.y - b.size.y * 0.5f - PADDING;
        delta.y = (pos.y - ceil(pos.y));
        size.y = b.size.y - velocity.y + delta.y + PADDING * 2.0f + 1.0f;
    }

    if (velocity.z > 0.0f)
    {
        pos.z = b.pos.z - PADDING;
        delta.z = (pos.z - ceil(pos.z));
        size.z = b.size.z + velocity.z + delta.z + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.z = b.pos.z + velocity.z - PADDING;
        delta.z = (pos.z - ceil(pos.z));
        size.z = b.size.z - velocity.z + delta.z + PADDING * 2.0f + 1.0f;
    }

    result.pos.x = floor(pos.x) - ch.x * CHUNK_DIAMETER;
    result.pos.y = floor(pos.y) - ch.y * CHUNK_DIAMETER;
    result.pos.z = floor(pos.z) - ch.z * CHUNK_DIAMETER;
    result.size.x = ceil(size.x);
    result.size.y = ceil(size.y);
    result.size.z = ceil(size.z);
    return result;
}

static void player_world_overflow_update(hhc_player *p)
{
    i64 diameter = WORLD_DIAMETER * CHUNK_DIAMETER;
    i64 diamerer_v = WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER;
    i64 margin = WORLD_RADIUS * CHUNK_DIAMETER - WORLD_MARGIN * CHUNK_DIAMETER;
    i64 margin_v = WORLD_RADIUS_VERTICAL * CHUNK_DIAMETER - WORLD_MARGIN * CHUNK_DIAMETER;
    i64 edge = WORLD_RADIUS * CHUNK_DIAMETER + CHUNK_DIAMETER;
    i64 edge_v = WORLD_RADIUS_VERTICAL * CHUNK_DIAMETER + CHUNK_DIAMETER;
    v3f64 world_volume_min =
    {
        -(f64)(WORLD_DIAMETER * CHUNK_DIAMETER),
        -(f64)(WORLD_DIAMETER * CHUNK_DIAMETER),
        -(f64)(WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER)
    };
    v3f64 world_volume_max =
    {
        (f64)(WORLD_DIAMETER * CHUNK_DIAMETER),
        (f64)(WORLD_DIAMETER * CHUNK_DIAMETER),
        (f64)(WORLD_DIAMETER_VERTICAL * CHUNK_DIAMETER)
    };

    if (!fsl_is_in_bounds_v3f64(p->transform.pos, world_volume_min, world_volume_max))
        player_spawn(p, FALSE);

    /* ---- world margin tagging -------------------------------------------- */

    if (p->transform.pos.x > margin)
        p->flag |= FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX;
    else if (p->transform.pos.x < -margin)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_X;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PX;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX);

    if (p->transform.pos.y > margin)
        p->flag |= FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY;
    else if (p->transform.pos.y < -margin)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_Y;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PY;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY);

    if (p->transform.pos.z > margin_v)
        p->flag |= FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ;
    else if (p->transform.pos.z < -margin_v)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_Z;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PZ;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ);

    /* ---- overflow edge teleportation ------------------------------------- */

    if (p->transform.pos.x > edge)
    {
        p->transform.pos.x -= diameter;
        p->transform_last.pos.x -= diameter;
        p->ch_delta.x -= WORLD_DIAMETER;
    }
    if (p->transform.pos.x < -edge)
    {
        p->transform.pos.x += diameter;
        p->transform_last.pos.x += diameter;
        p->ch_delta.x += WORLD_DIAMETER;
    }
    if (p->transform.pos.y > edge)
    {
        p->transform.pos.y -= diameter;
        p->transform_last.pos.y -= diameter;
        p->ch_delta.y -= WORLD_DIAMETER;
    }
    if (p->transform.pos.y < -edge)
    {
        p->transform.pos.y += diameter;
        p->transform_last.pos.y += diameter;
        p->ch_delta.y += WORLD_DIAMETER;
    }
    if (p->transform.pos.z > edge_v)
    {
        p->transform.pos.z -= diamerer_v;
        p->transform_last.pos.z -= diamerer_v;
        p->ch_delta.z -= WORLD_DIAMETER_VERTICAL;
    }
    if (p->transform.pos.z < -edge_v)
    {
        p->transform.pos.z += diamerer_v;
        p->transform_last.pos.z += diamerer_v;
        p->ch_delta.z += WORLD_DIAMETER_VERTICAL;
    }

    p->ch.x = floorf((f32)p->transform.pos.x / CHUNK_DIAMETER);
    p->ch.y = floorf((f32)p->transform.pos.y / CHUNK_DIAMETER);
    p->ch.z = floorf((f32)p->transform.pos.z / CHUNK_DIAMETER);
}

void player_camera_movement_update(hhc_player *p, v2f64 mouse_delta, b8 use_mouse)
{
    f64 zoom = 0.0;
    f64 sensitivity = 0.0;
    f64 SROL = {0}, CROL = {0}, SPCH = {0}, CPCH = {0}, SYAW = {0}, CYAW = {0};
    v3f64 eye_pos = {0};

    sensitivity = settings.mouse_sensitivity;
    eye_pos.x = p->transform.pos.x;
    eye_pos.y = p->transform.pos.y;
    eye_pos.z = p->transform.pos.z + p->eye_height;

    if (use_mouse)
    {
        if (p->flag & FLAG_PLAYER_ZOOMER)
            zoom = p->camera.zoom;

        if (p->camera_mode != PLAYER_CAMERA_MODE_STALKER)
            sensitivity = settings.mouse_sensitivity / (zoom / FSL_CAMERA_ZOOM_SENSITIVITY + 1.0);

        p->transform.rot.y += mouse_delta.y * sensitivity;
        p->transform.rot.z += mouse_delta.x * sensitivity;

        p->transform.rot.y = fsl_clamp_f64(p->transform.rot.y, -FSL_CAMERA_ANGLE_MAX, FSL_CAMERA_ANGLE_MAX);
        p->transform.rot.z = fmod(p->transform.rot.z, FSL_CAMERA_RANGE_MAX);
        if (p->transform.rot.z < 0.0)
            p->transform.rot.z += FSL_CAMERA_RANGE_MAX;
    }

    SROL = sin(p->transform.rot.x * FSL_DEG2RAD);
    CROL = cos(p->transform.rot.x * FSL_DEG2RAD);
    SPCH = sin(p->transform.rot.y * FSL_DEG2RAD);
    CPCH = cos(p->transform.rot.y * FSL_DEG2RAD);
    SYAW = sin(p->transform.rot.z * FSL_DEG2RAD);
    CYAW = cos(p->transform.rot.z * FSL_DEG2RAD);
    p->roll.sin = SYAW;
    p->roll.cos = CYAW;
    p->pitch.sin = SPCH;
    p->pitch.cos = CPCH;
    p->yaw.sin = SYAW;
    p->yaw.cos = CYAW;
    p->camera.roll.sin = SROL;
    p->camera.roll.cos = CROL;
    p->camera.pitch.sin = SPCH;
    p->camera.pitch.cos = CPCH;
    p->camera.yaw.sin = SYAW;
    p->camera.yaw.cos = CYAW;

    switch (p->camera_mode)
    {
        case PLAYER_CAMERA_MODE_1ST_PERSON:
            fsl_camera_movement_update(&p->camera,
                    p->transform.pos.x,
                    p->transform.pos.y,
                    p->transform.pos.z + p->eye_height,
                    p->transform.rot.x,
                    p->transform.rot.y,
                    p->transform.rot.z);
            break;

        case PLAYER_CAMERA_MODE_3RD_PERSON:
            fsl_camera_movement_update(&p->camera,
                    p->transform.pos.x - CYAW * CPCH * p->camera_distance,
                    p->transform.pos.y + SYAW * CPCH * p->camera_distance,
                    p->transform.pos.z + p->eye_height + SPCH * p->camera_distance,
                    p->transform.rot.x,
                    p->transform.rot.y,
                    p->transform.rot.z);
            break;

        case PLAYER_CAMERA_MODE_3RD_PERSON_FRONT:
            fsl_camera_movement_update(&p->camera,
                    p->transform.pos.x + CYAW * CPCH * p->camera_distance,
                    p->transform.pos.y - SYAW * CPCH * p->camera_distance,
                    p->transform.pos.z + p->eye_height - SPCH * p->camera_distance,
                    p->transform.rot.x,
                    -p->transform.rot.y,
                    p->transform.rot.z + FSL_CAMERA_RANGE_MAX / 2.0);
            break;

        case PLAYER_CAMERA_MODE_STALKER:
            fsl_camera_lookat_update(&p->camera,
                    p->camera.pos.x,
                    p->camera.pos.y,
                    p->camera.pos.z,
                    eye_pos.x,
                    eye_pos.y,
                    eye_pos.z);
            break;

            /* TODO: make the spectator camera mode */
        case PLAYER_CAMERA_MODE_SPECTATOR:
            break;

        default:
            break;
    }

    p->camera_hud.pitch.sin = p->camera.pitch.sin;
    p->camera_hud.pitch.cos = p->camera.pitch.cos;
    p->camera_hud.yaw.sin = p->camera.yaw.sin;
    p->camera_hud.yaw.cos = p->camera.yaw.cos;
}

void player_target_update(hhc_player *p)
{
    v3f64 origin = {0};
    v3f64 start = {0};
    v3f64 end = {0};

    origin.x = p->transform.pos.x;
    origin.y = p->transform.pos.y;
    origin.z = p->transform.pos.z;
    start.x = p->transform.pos.x;
    start.y = p->transform.pos.y;
    start.z = p->transform.pos.z + p->eye_height;
    end.x = start.x + p->yaw.cos * p->pitch.cos;
    end.y = start.y - p->yaw.sin * p->pitch.cos;
    end.z = start.z - p->pitch.sin;

    p->hit = block_hit_get(origin,
            start.x, start.y, start.z, end.x, end.y, end.z,
            settings.reach_distance);
}

void player_set_pos(hhc_player *p, f64 x, f64 y, f64 z)
{
    p->transform.pos.x = x;
    p->transform.pos.y = y;
    p->transform.pos.z = z;
    p->kn.velocity.x = 0.0;
    p->kn.velocity.y = 0.0;
    p->kn.velocity.z = 0.0;
    p->kn_forces[0].velocity.x = 0.0;
    p->kn_forces[0].velocity.y = 0.0;
    p->kn_forces[0].velocity.z = 0.0;
    p->kn_forces[1].velocity.x = 0.0;
    p->kn_forces[1].velocity.y = 0.0;
    p->kn_forces[1].velocity.z = 0.0;
    p->transform_last.pos = p->transform.pos;
}

void player_set_block(hhc_player *p, i64 x, i64 y, i64 z)
{
    p->transform.pos.x = (f64)x + 0.5;
    p->transform.pos.y = (f64)y + 0.5;
    p->transform.pos.z = (f64)z + 0.5;
    p->transform_last.pos = p->transform.pos;
}

void player_set_spawn(hhc_player *p, i64 x, i64 y, i64 z)
{
    p->spawn.x = x;
    p->spawn.y = y;
    p->spawn.z = z;
}

void player_spawn(hhc_player *p, b8 hard)
{
    player_set_pos(p,
            p->spawn.x + 0.5,
            p->spawn.y + 0.5,
            p->spawn.z + 0.5);

    if (hard)
    {
        p->health = 100.0;
        p->flag &= ~(FLAG_PLAYER_FLYING | FLAG_PLAYER_HUNGRY | FLAG_PLAYER_DEAD);
    }
}

void player_toggle_flying(hhc_player *p)
{
    p->flag ^= FLAG_PLAYER_FLYING;

    if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
    {
        if (p->flag & FLAG_PLAYER_FLYING)
        {
            p->transform.pos.z += PLAYER_EYE_HEIGHT - p->size.x / 2.0;
            p->size.z = p->size.x;
            p->eye_height = p->size.x / 2.0f;
        }
        else
        {
            p->transform.pos.z -= PLAYER_EYE_HEIGHT - p->size.x / 2.0;
            p->size.z = 1.8f;
            p->eye_height = PLAYER_EYE_HEIGHT;
        }
        p->transform.scale.z = p->size.z / 1.8;
    }
}

void player_toggle_cinematic_motion(hhc_player *p)
{
    p->flag ^= FLAG_PLAYER_CINEMATIC_MOTION;

    if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "Cinematic Motion Toggled On\n");
    else
        LOGDEBUG(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
                "Cinematic Motion Toggled Off\n");

    if (p->flag & FLAG_PLAYER_FLYING)
    {
        if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        {
            p->transform.pos.z += PLAYER_EYE_HEIGHT - p->size.x / 2.0;
            p->size.z = p->size.x;
            p->eye_height = p->size.x / 2.0f;
        }
        else
        {
            p->transform.pos.z -= PLAYER_EYE_HEIGHT - p->size.x / 2.0;
            p->size.z = 1.8f;
            p->eye_height = PLAYER_EYE_HEIGHT;
        }
        p->transform.scale.z = p->size.z / 1.8;
    }
}

void player_kill(hhc_player *p)
{
    p->health = 0.0;
    p->flag |= FLAG_PLAYER_DEAD;
    p->flag &= ~FLAG_PLAYER_CAN_JUMP;

    if (p->flag & FLAG_PLAYER_FLYING)
        player_toggle_cinematic_motion(p);

    if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        player_toggle_cinematic_motion(p);

    player_spawn(p, TRUE);

    LOGINFO(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            fsl_logger_stringf("%s %s\n", p->name, get_death_str(p)));
}

str *get_death_str(hhc_player *p)
{
    u64 index = fsl_rand_u32(fsl_get_time_raw_usec());
    index %= DEATH_STRINGS_MAX[p->death];

    switch (p->death)
    {
        case PLAYER_DEATH_REASON_COLLISION_WALL:
            return str_death_collision_wall[index];
            break;

        case PLAYER_DEATH_REASON_COLLISION_FLOOR:
            return str_death_collision_floor[index];
            break;

        case PLAYER_DEATH_REASON_COLLISION_CEILING:
            return str_death_collision_ceiling[index];
            break;

        default:
            break;
    }

    return NULL;
}
