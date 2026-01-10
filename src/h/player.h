#ifndef GAME_PLAYER_H
#define GAME_PLAYER_H

#include <engine/h/collision.h>

#include "main.h"

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
#define PLAYER_FRICTION_FLY_NATURAL 1.0f
#define PLAYER_FRICTION_FLYING      4.0f
#define PLAYER_FRICTION_FLYING_V    13.0f
#define PLAYER_FRICTION_DEFAULT     20.0f
#define PLAYER_COLLISION_DAMAGE_THRESHOLD 15.0f

enum PlayerFlag
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
}; /* PlayerFlag */

enum PlayerCameraMode
{
    PLAYER_CAMERA_MODE_1ST_PERSON,
    PLAYER_CAMERA_MODE_3RD_PERSON,
    PLAYER_CAMERA_MODE_3RD_PERSON_FRONT,
    PLAYER_CAMERA_MODE_STALKER,
    PLAYER_CAMERA_MODE_SPECTATOR,
    PLAYER_CAMERA_MODE_COUNT,
}; /* PlayerCameraMode */

enum PlayerMenuState
{
    STATE_PLAYER_MENU_CHEST,
    STATE_PLAYER_MENU_CRAFTING_TABLE,
    STATE_PLAYER_MENU_FURNACE,
    STATE_PLAYER_MENU_INVENTORY_SURVIVAL,
    STATE_PLAYER_MENU_INVENTORY_SANDBOX,
    STATE_PLAYER_MENU_COUNT,
}; /* PlayerMenuState */

typedef struct Player
{
    str name[64];                   /* in-game name */
    u64 flag;                       /* enum: PlayerFlag */
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
    f32 acceleration_rate;          /* scalar for 'acceleration' */
    f32 speed;                      /* derived from 'velocity' */
    f32 health;

    Camera camera;
    Camera camera_hud;              /* for hud 3d elements */
    f32 camera_distance;            /* for camera collision detection */
    u8 camera_mode;                 /* enum: CameraModes */

    /*! @brief player at world edge, enum: PlayerFlag.
     */
    u8 overflow;

    v3i32 chunk;                    /* current chunk */
    v3i32 chunk_delta;              /* previous chunk */

    v3i64 spawn;                    /* spawn point */
    u64 container_state;            /* enum: ContainerFlag */

    /*! @remark signed instead of unsigned so it's possible to navigate
     *  'hotbar_slots' when using mousewheel, used for wrapping around
     *  when out of range.
     */
    i32 hotbar_slot_selected;

    u32 hotbar_slots[PLAYER_HOTBAR_SLOTS_MAX];
    u32 inventory_slots[PLAYER_INVENTORY_SLOTS_MAX];

    BoundingBox bbox;
} Player;

/*! @brief update everything related to a player.
 *
 *  1. update kinematics and movement permissions based on 'p->flag'.
 *  2. update player FOV based on speed and camera zoom.
 *  3. update player collision.
 *  4. wrap player coordinates (teleport to the other side of the world if
 *    they cross a world edge).
 *  5. update player current chunk.
 *  6. make entire chunk buffer dirty if player crosses a chunk boundary.
 */
void player_update(Player *p, f64 dt);

void player_collision_update(Player *p, f64 dt);
void player_bounding_box_update(Player *p);

/*! @brief make a collision capsule for collision detection.
 *
 *  make a capsule encapsulating the bounding box 'b', plus a padding of 1 in each direction.
 *  useful for limiting collision checks to only within the capsule.
 *
 *  @param chunk = current chunk the player is in.
 *
 *  @remark collision capsule position is in chunk-relative coordinates, not world
 *  coordinates.
 */
BoundingBox make_collision_capsule(BoundingBox b, v3i32 chunk, v3f32 velocity);

/*! @brief update player chunk deltas.
 *
 *  calculate current chunk and delta chunk, and determine whether
 *  'FLAG_MAIN_CHUNK_BUF_DIRTY' should be set or not.
 */
void player_chunk_update(Player *p);

/*! @brief calculate camera rotations and mechanics based on 'player->camera_mode'.
 *
 *  @param use_mouse = let mouse delta move the camera, useful for interacting
 *  with UI instead of player.
 */
void player_camera_movement_update(Player *p, v2f64 mouse_delta, b8 use_mouse);

void player_target_update(Player *p);
void set_player_pos(Player *p, f64 x, f64 y, f64 z);
void set_player_block(Player *p, i64 x, i64 y, i64 z);

/*! @brief set player spawn point.
 */
void set_player_spawn(Player *p, i64 x, i64 y, i64 z);

/*! @brief re-spawn player.
 *
 *  @param hard = TRUE will reset all player stats, FALSE will only teleport to spawn.
 */
void player_spawn(Player *p, b8 hard);

void player_kill(Player *p);

#endif /* GAME_PLAYER_H */
