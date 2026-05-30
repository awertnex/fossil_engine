#ifndef HHC_DIAGNOSTICS_H
#define HHC_DIAGNOSTICS_H

#include "deps/fossil/common/diagnostics.h"
#include "deps/fossil/common/types.h"

#define HHC_ERR_WORLD_EXISTS        513
#define HHC_ERR_WORLD_CREATION_FAIL 514
#define HHC_ERR_COLLISIONS_DISABLED 515

/*!
 *  @brief global pointer to error variable.
 *
 *  @remark declared and initialized internally in @ref main.c.
 */
extern u32 *const GAME_ERR;

#endif /* HHC_DIAGNOSTICS_H */
