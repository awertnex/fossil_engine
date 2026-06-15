#ifndef HHC_PLAYER_H
#define HHC_PLAYER_H

#include "deps/fossil/assets/mesh/mesh.h"
#include "deps/fossil/math/trigonometry.h"
#include "deps/fossil/math/vector.h"
#include "deps/fossil/physics/collision.h"
#include "deps/fossil/physics/physics_types.h"

#include "raycast.h"
#include "container.h"

#define PLAYER_REACH_DISTANCE_MAX   5.0f

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
#define PLAYER_DEATH_STRING_CAP     128

/* ---- strings: death ------------------------------------------------------ */

#define DEATH_STRING_COLLISION_WALL_0           "died by headbutting a wall"
#define DEATH_STRING_COLLISION_WALL_1           "rammed a wall at high speed"
#define DEATH_STRING_COLLISION_WALL_2           "splat on a wall"
#define DEATH_STRING_COLLISION_WALL_COUNT       3

#define DEATH_STRING_COLLISION_FLOOR_0          "jumped off a cliff"
#define DEATH_STRING_COLLISION_FLOOR_1          "fell to their death"
#define DEATH_STRING_COLLISION_FLOOR_2          "splat on the ground"
#define DEATH_STRING_COLLISION_FLOOR_COUNT      3

#define DEATH_STRING_COLLISION_CEILING_0        "cracked their skull at a ceiling"
#define DEATH_STRING_COLLISION_CEILING_1        "flew into a ceiling"
#define DEATH_STRING_COLLISION_CEILING_2        "splat on a ceiling"
#define DEATH_STRING_COLLISION_CEILING_COUNT    3

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

    /*!
     *  @brief positive overflow direction flags.
     *
     *  @remark default is 0 for negative overflow (underflow).
     */
    FLAG_PLAYER_OVERFLOW_PX =       0x00002000,
    FLAG_PLAYER_OVERFLOW_PY =       0x00004000,
    FLAG_PLAYER_OVERFLOW_PZ =       0x00008000
}; /* player_flag */

typedef enum hhc_player_camera_mode
{
    PLAYER_CAMERA_MODE_1ST_PERSON,
    PLAYER_CAMERA_MODE_3RD_PERSON,
    PLAYER_CAMERA_MODE_3RD_PERSON_FRONT,
    PLAYER_CAMERA_MODE_STALKER,
    PLAYER_CAMERA_MODE_SPECTATOR,
    PLAYER_CAMERA_MODE_COUNT
} hhc_player_camera_mode;

typedef enum hhc_player_death_reason
{
    PLAYER_DEATH_REASON_NONE,
    PLAYER_DEATH_REASON_COLLISION_WALL,
    PLAYER_DEATH_REASON_COLLISION_FLOOR,
    PLAYER_DEATH_REASON_COLLISION_CEILING,
    PLAYER_DEATH_REASON_COUNT
} hhc_player_death_reason;

enum player_menu_state
{
    STATE_PLAYER_MENU_CHEST,
    STATE_PLAYER_MENU_CRAFTING_TABLE,
    STATE_PLAYER_MENU_FURNACE,
    STATE_PLAYER_MENU_INVENTORY_SURVIVAL,
    STATE_PLAYER_MENU_INVENTORY_SANDBOX,
    STATE_PLAYER_MENU_COUNT
}; /* player_menu_state */

typedef struct hhc_player
{
    str name[64];                   /* in-game name */
    u64 flag;                       /* enum @ref player_flag */
    transform_v3f64 transform;
    transform_v3f64 transform_last;
    v3f32 size;                     /* size (for collision detection) */
    v3f64 target;                   /* arm */

    angle_f64 roll, pitch, yaw;
    f32 eye_height;                 /* eye-level (camera height) */
    fsl_mesh mesh;

    v3f32 input;                    /* raw user input */
    v3f32 acceleration;
    v3f32 velocity;
    v3f32 friction;
    f32 acceleration_rate;          /* scalar for `acceleration` */
    f32 speed;                      /* derived from `velocity` */
    f32 health;

    fsl_camera camera;

    /*!
     *  @brief camera for HUD 3D elements responsive to player movement
     *  (e.g., 3D gizmo (<F3>), chunk gizmo (<Alt + G>).
     */
    fsl_camera camera_hud;

    /*!
     *  @brief camera for UI elements non-responsive to player movement
     *  (e.g., items in item slots).
     */
    fsl_camera camera_ui;

    f32 camera_distance;            /* for camera collision detection */
    hhc_player_camera_mode camera_mode;

    /*!
     *  @brief player at world edge, enum @ref player_flag.
     */
    u8 overflow;

    v3i32 ch;                       /* current chunk (named `ch` to avoid symbol clash with @ref chunk) */
    v3i32 ch_delta;                 /* previous chunk (named `ch` to avoid symbol clash with @ref chunk) */
    block_hit hit;                  /* information about the currently targeted block */

    v3i64 spawn;                    /* spawn point */
    u64 menu_state;                 /* enum @ref player_menu_state */

    /*!
     *  @remark signed instead of unsigned so it's possible to navigate `hotbar_slots`
     *  when using mousewheel, used for wrapping around when out of range.
     */
    i32 hotbar_slot_selected;

    hhc_container_slot hotbar_slots[CONTAINER_HOTBAR_SLOTS_MAX];
    hhc_container_slot inventory_slots[CONTAINER_INVENTORY_SLOTS_MAX];

    fsl_bounding_box bbox;
    hhc_player_death_reason death;
} hhc_player;

/*!
 *  @brief look-up table for @ref str_death_<x> buffer sizes.
 *
 *  @remark read-only, initialized internally in @ref common.c.
 */
extern u32 DEATH_STRINGS_MAX[PLAYER_DEATH_REASON_COUNT];

extern str str_death_collision_wall[DEATH_STRING_COLLISION_WALL_COUNT][PLAYER_DEATH_STRING_CAP];
extern str str_death_collision_floor[DEATH_STRING_COLLISION_FLOOR_COUNT][PLAYER_DEATH_STRING_CAP];
extern str str_death_collision_ceiling[DEATH_STRING_COLLISION_CEILING_COUNT][PLAYER_DEATH_STRING_CAP];

u32 player_init(hhc_player *p, const str *name);

/*!
 *  @brief update everything related to a player.
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
void player_update(hhc_player *p, f64 dt);

void player_hotbar_selected_set(hhc_player *p, u32 index);
void player_collision_update(hhc_player *p, f64 dt);
void player_bounding_box_update(hhc_player *p);

/*!
 *  @brief make a collision capsule for collision detection.
 *
 *  make a capsule encapsulating `b`, plus a padding of 1 in each direction.
 *  useful for limiting collision checks to only within the capsule.
 *
 *  @param ch current chunk the player is in (named `ch` to avoid symbol clash with @ref chunk).
 *
 *  @remark collision capsule position is in chunk-relative coordinates, not world
 *  coordinates.
 */
fsl_bounding_box make_collision_capsule(fsl_bounding_box b, v3i32 ch, v3f32 velocity);

/*!
 *  @brief calculate camera rotations and mechanics based on `p->camera_mode`.
 *
 *  @param use_mouse let mouse delta move the camera, useful for interacting
 *  with UI instead of player.
 */
void player_camera_movement_update(hhc_player *p, v2f64 mouse_delta, b8 use_mouse);

void player_target_update(hhc_player *p);
void player_set_pos(hhc_player *p, f64 x, f64 y, f64 z);
void player_set_block(hhc_player *p, i64 x, i64 y, i64 z);

/*!
 *  @brief set player spawn point.
 */
void player_set_spawn(hhc_player *p, i64 x, i64 y, i64 z);

/*!
 *  @brief re-spawn player.
 *
 *  @param hard `TRUE` will reset all player stats, `FALSE` will only teleport to spawn.
 */
void player_spawn(hhc_player *p, b8 hard);

void player_kill(hhc_player *p);

/*!
 *  @brief get random string from string buffers in @ref common.c for player death reason.
 *
 *  @return `NULL` on failure.
 */
str *get_death_str(hhc_player *p);

#endif /* HHC_PLAYER_H */
