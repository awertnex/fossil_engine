#include "deps/fossil/common/config.h"
#include "deps/fossil/assets/assets.h"
#include "deps/fossil/logger/logger.h"
#include "deps/fossil/math/math.h"
#include "deps/fossil/math/vector.h"
#include "deps/fossil/physics/collision.h"

#include "deps/fossil/h/time.h"

#include "h/main.h"

#include "chunking/chunking.h"

#include "h/common.h"
#include "h/diagnostics.h"
#include "h/player.h"
#include "h/world.h"

#include <stdio.h>
#include <math.h>

/*!
 *  @internal
 *
 *  @brief handle player being near or past world edges.
 *
 *  teleport player to the other side of the world if they cross a world edge.
 */
static void player_wrap_coordinates(hhc_player *p);

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
    p->transform.scale.x = 1.0f;
    p->transform.scale.y = 1.0f;
    p->transform.scale.z = 1.0f;
    p->eye_height = PLAYER_EYE_HEIGHT;
    p->camera_mode = PLAYER_CAMERA_MODE_1ST_PERSON;
    p->camera_distance = SET_CAMERA_DISTANCE_MAX;

    p->menu_state = 0;
    p->hotbar_slots[0] = BLOCK_GRASS;
    p->hotbar_slots[1] = BLOCK_DIRT;
    p->hotbar_slots[2] = BLOCK_STONE;
    p->hotbar_slots[3] = BLOCK_SAND;
    p->hotbar_slots[4] = BLOCK_GLASS;
    p->hotbar_slots[5] = BLOCK_WOOD_OAK_LOG;
    p->hotbar_slots[6] = BLOCK_WOOD_BIRCH_LOG;
    p->hotbar_slots[7] = BLOCK_WOOD_CHERRY_LOG;

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
    v3f32 gravity = {0};
    v3f32 drag = {0};
    v3f32 damping = {0};
    v3f32 air_control = {0};
    v3f32 nov3f32 = {0};

    p->flag &= ~FLAG_PLAYER_CAN_JUMP;
    p->acceleration = nov3f32;
    p->acceleration_rate = PLAYER_ACCELERATION_WALK;
    p->camera.fovy = settings.fov;

    /* ---- player flags ---------------------------------------------------- */

    if (p->flag & FLAG_PLAYER_FLYING)
    {
        p->acceleration_rate = PLAYER_ACCELERATION_FLY;

        if (p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
        {
            drag.x = PLAYER_FRICTION_FLY_NATURAL;
            drag.y = PLAYER_FRICTION_FLY_NATURAL;
            drag.z = PLAYER_FRICTION_FLY_NATURAL;
        }
        else
        {
            drag.x = PLAYER_FRICTION_FLYING;
            drag.y = PLAYER_FRICTION_FLYING;
            drag.z = PLAYER_FRICTION_FLYING_V;
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
        gravity.z -= world.gravity;
        drag.x = PLAYER_FRICTION_DEFAULT;
        drag.y = PLAYER_FRICTION_DEFAULT;
        drag.z = PLAYER_FRICTION_DEFAULT;

        air_control.x = p->input.x * p->acceleration_rate * drag.x * (1.0f - p->friction.x);
        air_control.y = p->input.y * p->acceleration_rate * drag.y * (1.0f - p->friction.y);

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

    /* ---- apply parameters ------------------------------------------------ */

    damping.x = -(world.drag.x) * p->velocity.x;
    damping.y = -(world.drag.y) * p->velocity.y;
    damping.z = -(world.drag.z) * p->velocity.z;

    p->acceleration.x = p->input.x * p->acceleration_rate * drag.x;
    p->acceleration.y = p->input.y * p->acceleration_rate * drag.y;
    p->acceleration.z = p->input.z * p->acceleration_rate * drag.z;

    p->velocity.x += (p->acceleration.x + damping.x + gravity.x) * dt;
    p->velocity.y += (p->acceleration.y + damping.y + gravity.y) * dt;
    p->velocity.z += (p->acceleration.z + damping.z + gravity.z) * dt;

    p->transform.pos.x += p->velocity.x * dt;
    p->transform.pos.y += p->velocity.y * dt;
    p->transform.pos.z += p->velocity.z * dt;

    p->friction.x = 0.0f;
    p->friction.y = 0.0f;

    p->speed = sqrtf(fsl_len_v3f32(p->velocity));
    if (p->speed > FSL_EPSILON)
        p->camera.fovy += p->speed * 0.03f;
    p->camera.fovy = fsl_clamp_f32(p->camera.fovy, 1.0f, SET_FOV_MAX);
    p->camera.fovy_smooth = fsl_lerp_exp_f32(p->camera.fovy_smooth, p->camera.fovy,
                SET_LERP_SPEED_FOV_MODE, dt);

    player_bounding_box_update(p);
    if (MODE_INTERNAL_COLLIDE)
        player_collision_update(p, dt);
    player_wrap_coordinates(p);
    p->ch.x = floorf((f32)p->transform.pos.x / CHUNK_DIAMETER);
    p->ch.y = floorf((f32)p->transform.pos.y / CHUNK_DIAMETER);
    p->ch.z = floorf((f32)p->transform.pos.z / CHUNK_DIAMETER);
}

void player_collision_update(hhc_player *p, f64 dt)
{
    hhc_chunk *ch = NULL;
    block *block_p = fsl_mem_handle_get(blocks);
    u32 *_block = NULL;
    f32 speed;
    v3f32 displacement = {0};
    f32 time = 0.0f;
    v3f32 normal = {0};
    f32 dot = 0.0f;
    f32 friction = 0.0f;
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

    displacement.x = p->velocity.x * dt;
    displacement.y = p->velocity.y * dt;
    displacement.z = p->velocity.z * dt;
    collision_capsule = make_collision_capsule(p->bbox, p->ch, displacement);
    MIN.x = (i32)collision_capsule.pos.x;
    MIN.y = (i32)collision_capsule.pos.y;
    MIN.z = (i32)collision_capsule.pos.z;
    MAX.x = (i32)(collision_capsule.pos.x + collision_capsule.size.x);
    MAX.y = (i32)(collision_capsule.pos.y + collision_capsule.size.y);
    MAX.z = (i32)(collision_capsule.pos.z + collision_capsule.size.z);

    if (p->velocity.x < 0.0f)
    {
        START.x = MAX.x - 1;
        INCREMENT.x = -1;
    }
    else START.x = MIN.x;

    if (p->velocity.y < 0.0f)
    {
        START.y = MAX.y - 1;
        INCREMENT.y = -1;
    }
    else START.y = MIN.y;

    if (p->velocity.z < 0.0f)
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

                    _block = get_block_resolved(ch, x, y, z);
                    if (!_block || !*_block)
                        continue;

                    block_box.pos.x = (f64)((i64)ch->pos.x * CHUNK_DIAMETER + fsl_mod_i32(x, CHUNK_DIAMETER));
                    block_box.pos.y = (f64)((i64)ch->pos.y * CHUNK_DIAMETER + fsl_mod_i32(y, CHUNK_DIAMETER));
                    block_box.pos.z = (f64)((i64)ch->pos.z * CHUNK_DIAMETER + fsl_mod_i32(z, CHUNK_DIAMETER));
                    block_box.size.x = 1.0;
                    block_box.size.y = 1.0;
                    block_box.size.z = 1.0;

                    time = fsl_get_swept_aabb(p->bbox, block_box, displacement, &normal);

                    if (fsl_is_in_range_f32(time, 0.0f, 1.0f) ||
                            fsl_is_intersect_aabb(p->bbox, block_box))
                    {
                        /* ---- resolution ---------------------------------- */

                        p->transform.pos.x += displacement.x * time + normal.x * FSL_COLLISION_EPSILON;
                        p->transform.pos.y += displacement.y * time + normal.y * FSL_COLLISION_EPSILON;
                        p->transform.pos.z += displacement.z * time + normal.z * FSL_COLLISION_EPSILON;

                        dot = fsl_dot_v3f32(displacement, normal);
                        if (dot < 0.0f)
                        {
                            displacement.x -= dot * normal.x;
                            displacement.y -= dot * normal.y;
                            displacement.z -= dot * normal.z;
                        }

                        p->transform.pos.x -= displacement.x * time;
                        p->transform.pos.y -= displacement.y * time;
                        p->transform.pos.z -= displacement.z * time;

                        p->velocity.x = displacement.x / dt;
                        p->velocity.y = displacement.y / dt;
                        p->velocity.z = displacement.z / dt;

                        if (normal.z > 0.0f)
                        {
                            if (!(p->flag & FLAG_PLAYER_CINEMATIC_MOTION))
                                p->flag &= ~FLAG_PLAYER_FLYING;
                            p->flag |= FLAG_PLAYER_CAN_JUMP;

                            friction = block_p[GET_BLOCK_ID(*_block)].friction;
                            p->friction.x = friction;
                            p->friction.y = friction;
                        }

                        player_bounding_box_update(p);

                        speed = p->speed;
                        p->speed = sqrtf(fsl_len_v3f32(p->velocity));
                        if (speed - p->speed > PLAYER_COLLISION_DAMAGE_THRESHOLD)
                        {
                            p->health -= (speed - p->speed);

                            if (p->health <= 0.0f && !(p->flag & FLAG_PLAYER_DEAD))
                            {
                                max_axis = fsl_max_axis_v3f32(normal);
                                if (max_axis == 1 || max_axis == 2)
                                    p->death = PLAYER_DEATH_REASON_COLLISION_WALL;
                                else if (max_axis == 3)
                                {
                                    if (normal.z > 0.0f)
                                        p->death = PLAYER_DEATH_REASON_COLLISION_FLOOR;
                                    else
                                        p->death = PLAYER_DEATH_REASON_COLLISION_CEILING;
                                }

                                player_kill(p);
                            }
                        }

                        resolved = TRUE;
                    }
                }
            }
        }
    }
}

void player_bounding_box_update(hhc_player *p)
{
    if (p->flag & FLAG_PLAYER_FLYING && p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
    {
        p->bbox.pos.x = p->transform.pos.x - p->size.x * 0.5f;
        p->bbox.pos.y = p->transform.pos.y - p->size.x * 0.5f; /* `size.x` here to link 'x' and 'y' to one size */
        p->bbox.pos.z = p->transform.pos.z + p->eye_height - p->size.x * 0.5f;

        /* size for all axes is `size.x` intentionally because we want cube bbox */
        p->bbox.size.x = p->size.x;
        p->bbox.size.y = p->size.x;
        p->bbox.size.z = p->size.x;
    }
    else
    {
        p->bbox.pos.x = p->transform.pos.x - p->size.x * 0.5f;
        p->bbox.pos.y = p->transform.pos.y - p->size.y * 0.5f;
        p->bbox.pos.z = p->transform.pos.z;
        p->bbox.size.x = p->size.x;
        p->bbox.size.y = p->size.y;
        p->bbox.size.z = p->size.z;
    }
}

fsl_bounding_box make_collision_capsule(fsl_bounding_box b, v3i32 ch, v3f32 velocity)
{
    const f32 PADDING = 1.0f;
    v3f64 pos = {0};
    v3f64 size = {0};
    v3f32 delta = {0};
    fsl_bounding_box result = {0};

    if (velocity.x > 0.0f)
    {
        pos.x = b.pos.x - b.size.x * 0.5f - PADDING;
        delta.x = (f32)(pos.x - ceil(pos.x));
        size.x = b.size.x + velocity.x + delta.x + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.x = b.pos.x + velocity.x - b.size.x * 0.5f - PADDING;
        delta.x = (f32)(pos.x - ceil(pos.x));
        size.x = b.size.x - velocity.x + delta.x + PADDING * 2.0f + 1.0f;
    }

    if (velocity.y > 0.0f)
    {
        pos.y = b.pos.y - b.size.y * 0.5f - PADDING;
        delta.y = (f32)(pos.y - ceil(pos.y));
        size.y = b.size.y + velocity.y + delta.y + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.y = b.pos.y + velocity.y - b.size.y * 0.5f - PADDING;
        delta.y = (f32)(pos.y - ceil(pos.y));
        size.y = b.size.y - velocity.y + delta.y + PADDING * 2.0f + 1.0f;
    }

    if (velocity.z > 0.0f)
    {
        pos.z = b.pos.z - PADDING;
        delta.z = (f32)(pos.z - ceil(pos.z));
        size.z = b.size.z + velocity.z + delta.z + PADDING * 2.0f + 1.0f;
    }
    else
    {
        pos.z = b.pos.z + velocity.z - PADDING;
        delta.z = (f32)(pos.z - ceil(pos.z));
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

static void player_wrap_coordinates(hhc_player *p)
{
    const i64 DIAMETER = WORLD_DIAMETER;
    const i64 DIAMETER_V = WORLD_DIAMETER_VERTICAL;
    const i64 WORLD_MARGIN = WORLD_RADIUS - SET_RENDER_DISTANCE_MAX * CHUNK_DIAMETER;
    const i64 WORLD_MARGIN_V = WORLD_RADIUS_VERTICAL - SET_RENDER_DISTANCE_MAX * CHUNK_DIAMETER;
    const i64 OVERFLOW_EDGE = WORLD_RADIUS + CHUNK_DIAMETER;
    const i64 OVERFLOW_EDGE_V = WORLD_RADIUS_VERTICAL + CHUNK_DIAMETER;
    const v3f64 WORLD_VOLUME_MIN =
    {
        -(f64)(WORLD_DIAMETER + WORLD_RADIUS),
        -(f64)(WORLD_DIAMETER + WORLD_RADIUS),
        -(f64)(WORLD_DIAMETER_VERTICAL + WORLD_RADIUS_VERTICAL)
    };
    const v3f64 WORLD_VOLUME_MAX =
    {
        (f64)(WORLD_DIAMETER + WORLD_RADIUS),
        (f64)(WORLD_DIAMETER + WORLD_RADIUS),
        (f64)(WORLD_DIAMETER_VERTICAL + WORLD_RADIUS_VERTICAL)
    };

    if (!fsl_is_in_volume_f64(p->transform.pos, WORLD_VOLUME_MIN, WORLD_VOLUME_MAX))
        player_spawn(p, FALSE);

    /* ---- world margin ---------------------------------------------------- */

    if (p->transform.pos.x > WORLD_MARGIN)
        p->flag |= FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX;
    else if (p->transform.pos.x < -WORLD_MARGIN)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_X;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PX;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX);

    if (p->transform.pos.y > WORLD_MARGIN)
        p->flag |= FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY;
    else if (p->transform.pos.y < -WORLD_MARGIN)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_Y;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PY;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY);

    if (p->transform.pos.z > WORLD_MARGIN_V)
        p->flag |= FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ;
    else if (p->transform.pos.z < -WORLD_MARGIN_V)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_Z;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PZ;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ);

    /* ---- overflow edge --------------------------------------------------- */

    if (p->transform.pos.x > OVERFLOW_EDGE)
    {
        p->transform.pos.x -= DIAMETER;
        p->transform_last.pos.x -= DIAMETER;
    }
    if (p->transform.pos.x < -OVERFLOW_EDGE)
    {
        p->transform.pos.x += DIAMETER;
        p->transform_last.pos.x += DIAMETER;
    }
    if (p->transform.pos.y > OVERFLOW_EDGE)
    {
        p->transform.pos.y -= DIAMETER;
        p->transform_last.pos.y -= DIAMETER;
    }
    if (p->transform.pos.y < -OVERFLOW_EDGE)
    {
        p->transform.pos.y += DIAMETER;
        p->transform_last.pos.y += DIAMETER;
    }
    if (p->transform.pos.z > OVERFLOW_EDGE_V)
    {
        p->transform.pos.z -= DIAMETER_V;
        p->transform_last.pos.z -= DIAMETER_V;
    }
    if (p->transform.pos.z < -OVERFLOW_EDGE_V)
    {
        p->transform.pos.z += DIAMETER_V;
        p->transform_last.pos.z += DIAMETER_V;
    }
}

void player_camera_movement_update(hhc_player *p, v2f64 mouse_delta, b8 use_mouse)
{
    f64 zoom = 0.0;
    f64 sensitivity = 0.0;
    f64 SROL = {0}, CROL = {0}, SPCH = {0}, CPCH = {0}, SYAW = {0}, CYAW = {0};
    f64 lookat_pitch = {0}, lookat_yaw = {0};
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
            fsl_update_camera_movement(&p->camera,
                    p->transform.pos.x,
                    p->transform.pos.y,
                    p->transform.pos.z + p->eye_height,
                    p->transform.rot.x,
                    p->transform.rot.y,
                    p->transform.rot.z);
            break;

        case PLAYER_CAMERA_MODE_3RD_PERSON:
            fsl_update_camera_movement(&p->camera,
                    p->transform.pos.x - CYAW * CPCH * p->camera_distance,
                    p->transform.pos.y + SYAW * CPCH * p->camera_distance,
                    p->transform.pos.z + p->eye_height + SPCH * p->camera_distance,
                    p->transform.rot.x,
                    p->transform.rot.y,
                    p->transform.rot.z);
            break;

        case PLAYER_CAMERA_MODE_3RD_PERSON_FRONT:
            fsl_update_camera_movement(&p->camera,
                    p->transform.pos.x + CYAW * CPCH * p->camera_distance,
                    p->transform.pos.y - SYAW * CPCH * p->camera_distance,
                    p->transform.pos.z + p->eye_height - SPCH * p->camera_distance,
                    p->transform.rot.x,
                    -p->transform.rot.y,
                    p->transform.rot.z + FSL_CAMERA_RANGE_MAX / 2.0);
            break;

        case PLAYER_CAMERA_MODE_STALKER:
            fsl_get_camera_lookat_angles(p->camera.pos, eye_pos, &lookat_pitch, &lookat_yaw);
            fsl_update_camera_movement(&p->camera,
                    p->camera.pos.x,
                    p->camera.pos.y,
                    p->camera.pos.z,
                    p->transform.rot.x,
                    lookat_pitch,
                    lookat_yaw);
            break;

            /* TODO: make the spectator camera mode */
        case PLAYER_CAMERA_MODE_SPECTATOR:
            break;
    }

    p->camera_hud.pitch.sin = p->camera.pitch.sin;
    p->camera_hud.pitch.cos = p->camera.pitch.cos;
    p->camera_hud.yaw.sin = p->camera.yaw.sin;
    p->camera_hud.yaw.cos = p->camera.yaw.cos;

    /* ---- camera_ui ------------------------------------------------------- */
}

void player_target_update(hhc_player *p)
{
    v3f64 start = {0};
    v3f64 end = {0};

    start.x = p->transform.pos.x;
    start.y = p->transform.pos.y;
    start.z = p->transform.pos.z + p->eye_height;
    end.x = start.x + p->yaw.cos * p->pitch.cos;
    end.y = start.y - p->yaw.sin * p->pitch.cos;
    end.z = start.z - p->pitch.sin;

    p->hit = block_hit_get(p->transform.pos,
            start.x, start.y, start.z, end.x, end.y, end.z,
            settings.reach_distance);
}

void set_player_pos(hhc_player *p, f64 x, f64 y, f64 z)
{
    p->transform.pos.x = x;
    p->transform.pos.y = y;
    p->transform.pos.z = z;
    p->velocity.x = 0.0;
    p->velocity.y = 0.0;
    p->velocity.z = 0.0;
    p->transform_last.pos = p->transform.pos;
}

void set_player_block(hhc_player *p, i64 x, i64 y, i64 z)
{
    p->transform.pos.x = (f64)x + 0.5f;
    p->transform.pos.y = (f64)y + 0.5f;
    p->transform.pos.z = (f64)z + 0.5f;
    p->transform_last.pos = p->transform.pos;
}

void set_player_spawn(hhc_player *p, i64 x, i64 y, i64 z)
{
    p->spawn.x = x;
    p->spawn.y = y;
    p->spawn.z = z;
}

void player_spawn(hhc_player *p, b8 hard)
{
    set_player_pos(p,
            p->spawn.x + 0.5f,
            p->spawn.y + 0.5f,
            p->spawn.z + 0.5f);
    p->health = 100.0f;
    if (!hard) return;
    p->flag &= ~(FLAG_PLAYER_FLYING | FLAG_PLAYER_HUNGRY | FLAG_PLAYER_DEAD);
}

void player_kill(hhc_player *p)
{
    p->velocity.x = 0.0;
    p->velocity.y = 0.0;
    p->velocity.z = 0.0;
    p->health = 0.0f;
    p->flag |= FLAG_PLAYER_DEAD;
    p->flag &= ~FLAG_PLAYER_CAN_JUMP;
    p->flag &= ~FLAG_PLAYER_FLYING;
    p->flag &= ~FLAG_PLAYER_CINEMATIC_MOTION;

    LOGINFO(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            fsl_logger_stringf("%s %s\n", p->name, get_death_str(p)));
}

str *get_death_str(hhc_player *p)
{
    u64 index = fsl_rand_u64(fsl_get_time_raw_usec()) % DEATH_STRINGS_MAX[p->death];

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
    }

    return NULL;
}
