#ifndef GAME_DIAGNOSTICS_H
#define GAME_DIAGNOSTICS_H

#include <engine/h/types.h>
#include <engine/h/diagnostics.h>
#include <engine/h/limits.h>

#define GAME_ERR_OFFSET 512

enum GameErrorCodes
{
    ERR_MODE_INTERNAL_DEBUG_DISABLE = GAME_ERR_OFFSET,
    ERR_MODE_INTERNAL_COLLIDE_DISABLE,
    ERR_WORLD_EXISTS,
    ERR_WORLD_CREATION_FAIL,
}; /* GameErrorCodes */

/*! @brief global pointer to error variable.
 *
 *  @remark declared and initialized internally in 'src/main.c'.
 */
extern u32 *const GAME_ERR;

#endif /* GAME_DIAGNOSTICS_H */
