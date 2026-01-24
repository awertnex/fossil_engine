#include <engine/h/collision.h>
#include <engine/h/math.h>
#include <engine/h/time.h>

#include "h/chunking.h"
#include "h/common.h"
#include "h/logger.h"
#include "h/player.h"
#include "h/world.h"

/*! -- INTERNAL USE ONLY --;
 *
 *  @brief handle player being near or past world edges.
 *
 *  teleport player to the other side of the world if they cross a world edge.
 */
static void player_wrap_coordinates(player *p);

void player_update(player *p, f64 dt)
{
    v3f32 gravity = {0}, drag = {0},
          damping = {0}, air_control = {0};

    p->flag &= ~FLAG_PLAYER_CAN_JUMP;
    p->acceleration = (v3f32){0};
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

    p->pos.x += p->velocity.x * dt;
    p->pos.y += p->velocity.y * dt;
    p->pos.z += p->velocity.z * dt;

    p->friction.x = 0.0f;
    p->friction.y = 0.0f;

    p->speed = sqrtf(fsl_len_v3f32(p->velocity));
    if (p->speed > FSL_EPSILON)
        p->camera.fovy += p->speed * 0.03f;
    p->camera.fovy = fsl_clamp_f32(p->camera.fovy, 1.0f, SET_FOV_MAX);
    p->camera.fovy_smooth = fsl_lerp_exp_f32(p->camera.fovy_smooth, p->camera.fovy,
                SET_LERP_SPEED_FOV_MODE, dt);

    player_bounding_box_update(p);
    player_collision_update(p, dt);
    player_wrap_coordinates(p);
    player_chunk_update(p);
}

void player_collision_update(player *p, f64 dt)
{
    if (!MODE_INTERNAL_COLLIDE) return;

    chunk *ch = NULL;
    u32 *block = NULL;
    f32 speed;
    v3f32 displacement =
    {
        p->velocity.x * dt,
        p->velocity.y * dt,
        p->velocity.z * dt,
    };
    f32 time = 0.0f;
    v3f32 normal = {0};
    f32 dot = 0.0f;
    f32 friction = 0.0f;
    fsl_bounding_box block_box = {0};
    fsl_bounding_box collision_capsule;
    i32 i, x, y, z;
    v3i32 MIN, MAX, START = {0}, INCREMENT = {1, 1, 1};
    b8 resolved = TRUE;
    u32 max_axis = 0;

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
            for (y = START.y; y >= MIN.y && y < MAX.y; y += INCREMENT.y)
                for (x = START.x; x >= MIN.x && x < MAX.x; x += INCREMENT.x)
                {
                    ch = get_chunk_resolved(settings.chunk_tab_center, x, y, z);
                    if (!ch || !(ch->flag & FLAG_CHUNK_GENERATED)) continue;
                    block = get_block_resolved(ch, x, y, z);
                    if (!block || !*block) continue;

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

                        p->pos.x += displacement.x * time + normal.x * FSL_COLLISION_EPSILON;
                        p->pos.y += displacement.y * time + normal.y * FSL_COLLISION_EPSILON;
                        p->pos.z += displacement.z * time + normal.z * FSL_COLLISION_EPSILON;

                        dot = fsl_dot_v3f32(displacement, normal);
                        if (dot < 0.0f)
                        {
                            displacement.x -= dot * normal.x;
                            displacement.y -= dot * normal.y;
                            displacement.z -= dot * normal.z;
                        }

                        p->pos.x -= displacement.x * time;
                        p->pos.y -= displacement.y * time;
                        p->pos.z -= displacement.z * time;

                        p->velocity.x = displacement.x / dt;
                        p->velocity.y = displacement.y / dt;
                        p->velocity.z = displacement.z / dt;

                        if (normal.z > 0.0f)
                        {
                            if (!(p->flag & FLAG_PLAYER_CINEMATIC_MOTION))
                                p->flag &= ~FLAG_PLAYER_FLYING;
                            p->flag |= FLAG_PLAYER_CAN_JUMP;

                            friction = blocks[*block & MASK_BLOCK_ID].friction;
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

void player_bounding_box_update(player *p)
{
    if (p->flag & FLAG_PLAYER_FLYING && p->flag & FLAG_PLAYER_CINEMATIC_MOTION)
    {
        p->bbox.pos.x = p->pos.x - p->size.x * 0.5f;
        p->bbox.pos.y = p->pos.y - p->size.x * 0.5f;
        p->bbox.pos.z = p->pos.z + p->eye_height - p->size.x * 0.5f;

        /* size for all axes is 'x' intentionally because we want cube bbox */
        p->bbox.size.x = p->size.x;
        p->bbox.size.y = p->size.x;
        p->bbox.size.z = p->size.x;
    }
    else
    {
        p->bbox.pos.x = p->pos.x - p->size.x * 0.5f;
        p->bbox.pos.y = p->pos.y - p->size.y * 0.5f;
        p->bbox.pos.z = p->pos.z;
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

    return (fsl_bounding_box){
        .pos = (v3f64){
            floor(pos.x) - ch.x * CHUNK_DIAMETER,
            floor(pos.y) - ch.y * CHUNK_DIAMETER,
            floor(pos.z) - ch.z * CHUNK_DIAMETER,
        },
        .size = (v3f64){
            ceil(size.x),
            ceil(size.y),
            ceil(size.z),
        },
    };
}

void player_chunk_update(player *p)
{
    p->ch.x = floorf((f32)p->pos.x / CHUNK_DIAMETER);
    p->ch.y = floorf((f32)p->pos.y / CHUNK_DIAMETER);
    p->ch.z = floorf((f32)p->pos.z / CHUNK_DIAMETER);

    if (
            p->ch_delta.x - p->ch.x ||
            p->ch_delta.y - p->ch.y ||
            p->ch_delta.z - p->ch.z)
        core.flag.chunk_buf_dirty = 1;
}

static void player_wrap_coordinates(player *p)
{
    i64 DIAMETER = WORLD_DIAMETER,
        DIAMETER_V = WORLD_DIAMETER_VERTICAL,
        WORLD_MARGIN = WORLD_RADIUS - SET_RENDER_DISTANCE_MAX * CHUNK_DIAMETER,
        WORLD_MARGIN_V = WORLD_RADIUS_VERTICAL - SET_RENDER_DISTANCE_MAX * CHUNK_DIAMETER,
        OVERFLOW_EDGE = WORLD_RADIUS + CHUNK_DIAMETER,
        OVERFLOW_EDGE_V = WORLD_RADIUS_VERTICAL + CHUNK_DIAMETER;

    if (!fsl_is_in_volume_f64(p->pos,
                (v3f64){
                -(f64)(WORLD_DIAMETER + WORLD_RADIUS),
                -(f64)(WORLD_DIAMETER + WORLD_RADIUS),
                -(f64)(WORLD_DIAMETER_VERTICAL + WORLD_RADIUS_VERTICAL)},
                (v3f64){
                (f64)(WORLD_DIAMETER + WORLD_RADIUS),
                (f64)(WORLD_DIAMETER + WORLD_RADIUS),
                (f64)(WORLD_DIAMETER_VERTICAL + WORLD_RADIUS_VERTICAL)}))
        player_spawn(p, FALSE);

    /* ---- world margin ---------------------------------------------------- */

    if (p->pos.x > WORLD_MARGIN)
        p->flag |= FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX;
    else if (p->pos.x < -WORLD_MARGIN)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_X;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PX;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_X | FLAG_PLAYER_OVERFLOW_PX);

    if (p->pos.y > WORLD_MARGIN)
        p->flag |= FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY;
    else if (p->pos.y < -WORLD_MARGIN)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_Y;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PY;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_Y | FLAG_PLAYER_OVERFLOW_PY);

    if (p->pos.z > WORLD_MARGIN_V)
        p->flag |= FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ;
    else if (p->pos.z < -WORLD_MARGIN_V)
    {
        p->flag |= FLAG_PLAYER_OVERFLOW_Z;
        p->flag &= ~FLAG_PLAYER_OVERFLOW_PZ;
    }
    else p->flag &= ~(FLAG_PLAYER_OVERFLOW_Z | FLAG_PLAYER_OVERFLOW_PZ);

    /* ---- overflow edge --------------------------------------------------- */

    if (p->pos.x > OVERFLOW_EDGE)
    {
        p->pos.x -= DIAMETER;
        p->pos_last.x -= DIAMETER;
    }
    if (p->pos.x < -OVERFLOW_EDGE)
    {
        p->pos.x += DIAMETER;
        p->pos_last.x += DIAMETER;
    }
    if (p->pos.y > OVERFLOW_EDGE)
    {
        p->pos.y -= DIAMETER;
        p->pos_last.y -= DIAMETER;
    }
    if (p->pos.y < -OVERFLOW_EDGE)
    {
        p->pos.y += DIAMETER;
        p->pos_last.y += DIAMETER;
    }
    if (p->pos.z > OVERFLOW_EDGE_V)
    {
        p->pos.z -= DIAMETER_V;
        p->pos_last.z -= DIAMETER_V;
    }
    if (p->pos.z < -OVERFLOW_EDGE_V)
    {
        p->pos.z += DIAMETER_V;
        p->pos_last.z += DIAMETER_V;
    }
}

void player_camera_movement_update(player *p, v2f64 mouse_delta, b8 use_mouse)
{
    f64 zoom = 0.0, sensitivity = settings.mouse_sensitivity,
        SROL, CROL, SPCH, CPCH, SYAW, CYAW,
        lookat_pitch, lookat_yaw;
    v3f64 eye_pos =
    {
        p->pos.x,
        p->pos.y,
        p->pos.z + p->eye_height,
    };

    if (use_mouse)
    {
        if (p->flag & FLAG_PLAYER_ZOOMER)
            zoom = p->camera.zoom;

        if (p->camera_mode != PLAYER_CAMERA_MODE_STALKER)
            sensitivity = settings.mouse_sensitivity / (zoom / FSL_CAMERA_ZOOM_SENSITIVITY + 1.0);

        p->pitch += mouse_delta.y * sensitivity;
        p->yaw += mouse_delta.x * sensitivity;

        p->pitch = fsl_clamp_f64(p->pitch, -FSL_CAMERA_ANGLE_MAX, FSL_CAMERA_ANGLE_MAX);
        p->yaw = fmod(p->yaw, FSL_CAMERA_RANGE_MAX);
        if (p->yaw < 0.0)
            p->yaw += FSL_CAMERA_RANGE_MAX;
    }

    SROL = sin(p->roll * FSL_DEG2RAD);
    CROL = cos(p->roll * FSL_DEG2RAD);
    SPCH = sin(p->pitch * FSL_DEG2RAD);
    CPCH = cos(p->pitch * FSL_DEG2RAD);
    SYAW = sin(p->yaw * FSL_DEG2RAD);
    CYAW = cos(p->yaw * FSL_DEG2RAD);
    p->sin_roll = SYAW;
    p->cos_roll = CYAW;
    p->sin_pitch = SPCH;
    p->cos_pitch = CPCH;
    p->sin_yaw = SYAW;
    p->cos_yaw = CYAW;
    p->camera.sin_roll = SROL;
    p->camera.cos_roll = CROL;
    p->camera.sin_pitch = SPCH;
    p->camera.cos_pitch = CPCH;
    p->camera.sin_yaw = SYAW;
    p->camera.cos_yaw = CYAW;

    switch (p->camera_mode)
    {
        case PLAYER_CAMERA_MODE_1ST_PERSON:
            p->camera.pos.x = p->pos.x;
            p->camera.pos.y = p->pos.y;
            p->camera.pos.z = p->pos.z + p->eye_height;
            break;

        case PLAYER_CAMERA_MODE_3RD_PERSON:
            p->camera.pos.x = p->pos.x - CYAW * CPCH * p->camera_distance;
            p->camera.pos.y = p->pos.y + SYAW * CPCH * p->camera_distance;
            p->camera.pos.z = p->pos.z + p->eye_height + SPCH * p->camera_distance;
            break;

        case PLAYER_CAMERA_MODE_3RD_PERSON_FRONT:
            p->camera.pos.x = p->pos.x + CYAW * CPCH * p->camera_distance;
            p->camera.pos.y = p->pos.y - SYAW * CPCH * p->camera_distance;
            p->camera.pos.z = p->pos.z + p->eye_height - SPCH * p->camera_distance;

            p->camera.sin_pitch = -SPCH;
            p->camera.sin_yaw = sin((p->yaw + FSL_CAMERA_RANGE_MAX / 2.0) * FSL_DEG2RAD);
            p->camera.cos_yaw = cos((p->yaw + FSL_CAMERA_RANGE_MAX / 2.0) * FSL_DEG2RAD);
            break;

        case PLAYER_CAMERA_MODE_STALKER:
            fsl_get_camera_lookat_angles(p->camera.pos, eye_pos, &lookat_pitch, &lookat_yaw);

            p->camera.sin_pitch = sin(lookat_pitch);
            p->camera.cos_pitch = cos(lookat_pitch);
            p->camera.sin_yaw = sin(lookat_yaw + (FSL_CAMERA_RANGE_MAX / 2.0) * FSL_DEG2RAD);
            p->camera.cos_yaw = cos(lookat_yaw + (FSL_CAMERA_RANGE_MAX / 2.0) * FSL_DEG2RAD);
            break;

            /* TODO: make the spectator camera mode */
        case PLAYER_CAMERA_MODE_SPECTATOR:
            break;
    }

    p->camera_hud.sin_pitch = p->camera.sin_pitch;
    p->camera_hud.cos_pitch = p->camera.cos_pitch;
    p->camera_hud.sin_yaw = p->camera.sin_yaw;
    p->camera_hud.cos_yaw = p->camera.cos_yaw;
}

void player_target_update(player *p)
{
    f64 SPCH = p->sin_pitch;
    f64 CPCH = p->cos_pitch;
    f64 SYAW = p->sin_yaw;
    f64 CYAW = p->cos_yaw;
    v3f64 eye_pos =
    {
        p->pos.x,
        p->pos.y,
        p->pos.z + p->eye_height,
    };
    v3i64 block_pos =
    {
        (i64)floor(eye_pos.x),
        (i64)floor(eye_pos.y),
        (i64)floor(eye_pos.z),
    };
    v3f64 direction =
    {
        CYAW * CPCH,
        -SYAW * CPCH,
        -SPCH,
    };
    v3f64 delta =
    {
       direction.x == 0.0 ? INFINITY : fabs(1.0 / direction.x),
       direction.y == 0.0 ? INFINITY : fabs(1.0 / direction.y),
       direction.z == 0.0 ? INFINITY : fabs(1.0 / direction.z),
    };
    v3f64 distance = {0};
    v3f64 normal = {0};
    v3i32 step = {1, 1, 1};
    b8 hit = FALSE;
    chunk *ch = NULL;
    u32 *block = NULL;
    i32 x, y, z;

    if (direction.x < 0.0f)
    {
        distance.x = (eye_pos.x - block_pos.x) * delta.x;
        step.x = -1;
    }
    else distance.x = (block_pos.x + 1.0 - eye_pos.x) * delta.x;

    if (direction.y < 0.0f)
    {
        distance.y = (eye_pos.y - block_pos.y) * delta.y;
        step.y = -1;
    }
    else distance.y = (block_pos.y + 1.0 - eye_pos.y) * delta.y;

    if (direction.z < 0.0f)
    {
        distance.z = (eye_pos.z - block_pos.z) * delta.z;
        step.z = -1;
    }
    else distance.z = (block_pos.z + 1.0 - eye_pos.z) * delta.z;

    while (fsl_min_v3f32((v3f32){distance.x, distance.y, distance.z}) < settings.reach_distance)
    {
        switch (fsl_min_axis_v3f32((v3f32){distance.x, distance.y, distance.z}))
        {
            case 1:
                block_pos.x += step.x;
                distance.x += delta.x;
                normal = (v3f64){-step.x, 0.0, 0.0};
                break;

            case 2:
                block_pos.y += step.y;
                distance.y += delta.y;
                normal = (v3f64){0.0, -step.y, 0.0};
                break;

            case 3:
                block_pos.z += step.z;
                distance.z += delta.z;
                normal = (v3f64){0.0, 0.0, -step.z};
                break;
        }

        x = block_pos.x - p->ch.x * CHUNK_DIAMETER;
        y = block_pos.y - p->ch.y * CHUNK_DIAMETER;
        z = block_pos.z - p->ch.z * CHUNK_DIAMETER;
        ch = get_chunk_resolved(settings.chunk_tab_center, x, y, z);
        if (!ch || !(ch->flag & FLAG_CHUNK_GENERATED)) continue;
        block = get_block_resolved(ch, x, y, z);
        if (!block || !*block) continue;
        hit = TRUE;
        break;
    }

    if (hit && fsl_is_in_volume_i64((v3i64){(i64)p->target.x, (i64)p->target.y, (i64)p->target.z},
                (v3i64){-WORLD_DIAMETER, -WORLD_DIAMETER, -WORLD_DIAMETER_VERTICAL},
                (v3i64){WORLD_DIAMETER, WORLD_DIAMETER, WORLD_DIAMETER_VERTICAL}))
    {
        core.flag.parse_target = 1;
        p->target.x = (f64)block_pos.x;
        p->target.y = (f64)block_pos.y;
        p->target.z = (f64)block_pos.z;
        p->target_normal = normal;
    }
    else
    {
        core.flag.parse_target = 0;
        p->target_normal = (v3f64){0};
    }

}

void set_player_pos(player *p, f64 x, f64 y, f64 z)
{
    p->pos = (v3f64){x, y, z};
    p->velocity = (v3f32){0};
    p->pos_last = p->pos;
}

void set_player_block(player *p, i64 x, i64 y, i64 z)
{
    p->pos.x = (f64)x + 0.5f;
    p->pos.y = (f64)y + 0.5f;
    p->pos.z = (f64)z + 0.5f;
    p->pos_last = p->pos;
}

void set_player_spawn(player *p, i64 x, i64 y, i64 z)
{
    p->spawn = (v3i64){x, y, z};
}

void player_spawn(player *p, b8 hard)
{
    set_player_pos(p,
            p->spawn.x + 0.5f,
            p->spawn.y + 0.5f,
            p->spawn.z + 0.5f);
    p->health = 100.0f;
    if (!hard) return;
    p->flag &= ~(FLAG_PLAYER_FLYING | FLAG_PLAYER_HUNGRY | FLAG_PLAYER_DEAD);
}

void player_kill(player *p)
{
    p->velocity = (v3f32){0};
    p->health = 0.0f;
    p->flag |= FLAG_PLAYER_DEAD;

    HHC_LOGINFO(FSL_FLAG_LOG_NO_VERBOSE | FSL_FLAG_LOG_CMD,
            "%s %s\n", p->name, get_death_str(p));
}

str *get_death_str(player *p)
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
