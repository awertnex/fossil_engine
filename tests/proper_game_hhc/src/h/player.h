#ifndef HHC_PLAYER_H
#define HHC_PLAYER_H

#include "common.h"
#include "main.h"

#include "src/h/collision.h"

#define PLAYER_REACH_DISTANCE_MAX   5.0f
#define PLAYER_HOTBAR_SLOTS_MAX     10
#define PLAYER_INVENTORY_SLOTS_MAX  (PLAYER_HOTBAR_SLOTS_MAX * 4)

#define PLAYER_EYE_HEIGHT           1.55f
#define PLAYER_JUMP_HEIGHT          1.5f
#define PLAYER_ACCELERATION_SNEAK   2.5f
#define PLAYER_ACCELERATION_WALK    4.0f
#define PLAYER_ACCELERATION_SPRINT  6.0f
#define PLAYER_ACCELERATION_FLY     9.0f
#define PLAYER_ACCELERATION_FLY_FAST 20.0f
#define PLAYER_ACCELERATION_MAX     100.0f
#define PLAYER_FRICTION_DEFAULT     1.0f
#define PLAYER_FRICTION_FLY_NATURAL 1.0f
#define PLAYER_FRICTION_FLYING      4.0f
#define PLAYER_FRICTION_FLYING_V    13.0f
#define PLAYER_COLLISION_DAMAGE_THRESHOLD 15.0f

enum player_flag
{
    FLAG_PLAYER_CAN_JUMP =          0x00000001,
    FLAG_PLAYER_SNEAKING =          0x00000002,
    FLAG_PLAYER_SPRINTING =         0x00000004,
    FLAG_PLAYER_FLYING =            0x00000008,
    FLAG_PLAYER_SWIMMING =          0x00000010,
    FLAG_PLAYER_HUNGRY =            0x00000020,
    FLAG_PLAYER_DEAD =              0x00000040,
    FLAG_PLAYER_ZOOMER =            0x00000080,
    FLAG_PLAYER_CINEMATIC_MOTION =  0x00000100,
    FLAG_PLAYER_FLASHLIGHT =        0x00000200,

    FLAG_PLAYER_OVERFLOW_X =        0x00000400,
    FLAG_PLAYER_OVERFLOW_Y =        0x00000800,
    FLAG_PLAYER_OVERFLOW_Z =        0x00001000,

    /*! @brief positive overflow direction flags,
     *  @remark default is 0 for negative overflow (underflow).
     */
    FLAG_PLAYER_OVERFLOW_PX =       0x00002000,
    FLAG_PLAYER_OVERFLOW_PY =       0x00004000,
    FLAG_PLAYER_OVERFLOW_PZ =       0x00008000,
}; /* player_flag */

enum player_camera_mode
{
    PLAYER_CAMERA_MODE_1ST_PERSON,
    PLAYER_CAMERA_MODE_3RD_PERSON,
    PLAYER_CAMERA_MODE_3RD_PERSON_FRONT,
    PLAYER_CAMERA_MODE_STALKER,
    PLAYER_CAMERA_MODE_SPECTATOR,
    PLAYER_CAMERA_MODE_COUNT,
}; /* player_camera_mode */

enum player_menu_state
{
    STATE_PLAYER_MENU_CHEST,
    STATE_PLAYER_MENU_CRAFTING_TABLE,
    STATE_PLAYER_MENU_FURNACE,
    STATE_PLAYER_MENU_INVENTORY_SURVIVAL,
    STATE_PLAYER_MENU_INVENTORY_SANDBOX,
    STATE_PLAYER_MENU_COUNT,
}; /* player_menu_state */

typedef struct player
{
    str name[64];                   /* in-game name */
    u64 flag;                       /* enum: @ref player_flag */
    v3f64 pos;                      /* coordinates in world */
    v3f64 pos_last;                 /* coordinates in world of previous frame */
    v3f32 size;                     /* size (for collision detection) */
    v3f64 target;                   /* arm */
    v3f64 target_normal;

    f64 roll, pitch, yaw;
    f32 sin_roll, sin_pitch, sin_yaw;
    f32 cos_roll, cos_pitch, cos_yaw;
    f32 eye_height;                 /* eye-level (camera height) */

    v3f32 input;                    /* raw user input */
    v3f32 acceleration;
    v3f32 velocity;
    v3f32 friction;
    f32 acceleration_rate;          /* scalar for `acceleration` */
    f32 speed;                      /* derived from `velocity` */
    f32 health;

    fsl_camera camera;
    fsl_camera camera_hud;          /* for hud 3d elements */
    f32 camera_distance;            /* for camera collision detection */
    u8 camera_mode;                 /* enum: @ref player_camera_mode */

    /*! @brief player at world edge, enum: @ref player_flag.
     */
    u8 overflow;

    v3i32 ch;                       /* current chunk (named `ch` to avoid symbol clash with @ref chunk) */
    v3i32 ch_delta;                 /* previous chunk (named `ch` to avoid symbol clash with @ref chunk) */

    v3i64 spawn;                    /* spawn point */
    u64 menu_state;                 /* enum: @ref player_menu_state */

    /*! @remark signed instead of unsigned so it's possible to navigate `hotbar_slots
     *  when using mousewheel, used for wrapping around when out of range.
     */
    i32 hotbar_slot_selected;

    u32 hotbar_slots[PLAYER_HOTBAR_SLOTS_MAX];
    u32 inventory_slots[PLAYER_INVENTORY_SLOTS_MAX];

    fsl_bounding_box bbox;
    u32 death; /* enum: @ref player_death_reason */
} player;

/*! @brief update everything related to a player.
 *
 *  1. update kinematics and movement permissions based on `p->flag`.
 *  2. update player FOV based on speed and camera zoom.
 *  3. update player collision.
 *
 *  4. wrap player coordinates (teleport to the other side of the world if
 *    they cross a world edge).
 *
 *  5. update player current chunk.
 *  6. make entire chunk buffer dirty if player crosses a chunk boundary.
 */
void player_update(player *p, f64 dt);

void player_collision_update(player *p, f64 dt);
void player_bounding_box_update(player *p);

/*! @brief make a collision capsule for collision detection.
 *
 *  make a capsule encapsulating `b`, plus a padding of 1 in each direction.
 *  useful for limiting collision checks to only within the capsule.
 *
 *  @param ch = current chunk the player is in (named `ch` to avoid symbol clash with @ref chunk).
 *
 *  @remark collision capsule position is in chunk-relative coordinates, not world
 *  coordinates.
 */
fsl_bounding_box make_collision_capsule(fsl_bounding_box b, v3i32 ch, v3f32 velocity);

/*! @brief update player chunk deltas.
 *
 *  calculate current chunk and delta chunk, and determine whether
 *  @ref core.flag.chunk_buf_dirty should be set or not.
 */
void player_chunk_update(player *p);

/*! @brief calculate camera rotations and mechanics based on `player->camera_mode`.
 *
 *  @param use_mouse = let mouse delta move the camera, useful for interacting
 *  with UI instead of player.
 */
void player_camera_movement_update(player *p, v2f64 mouse_delta, b8 use_mouse);

void player_target_update(player *p);
void set_player_pos(player *p, f64 x, f64 y, f64 z);
void set_player_block(player *p, i64 x, i64 y, i64 z);

/*! @brief set player spawn point.
 */
void set_player_spawn(player *p, i64 x, i64 y, i64 z);

/*! @brief re-spawn player.
 *
 *  @param hard = `TRUE` will reset all player stats, `FALSE` will only teleport to spawn.
 */
void player_spawn(player *p, b8 hard);

void player_kill(player *p);

/*! @brief get random string from string buffers in @ref common.c for player death reason.
 *
 *  @return `NULL` on failure.
 */
str *get_death_str(player *p);

#endif /* HHC_PLAYER_H */
