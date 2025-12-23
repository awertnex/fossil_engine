#ifndef ENGINE_TIME_H
#define ENGINE_TIME_H

#include "types.h"

#define SEC2NANOSEC 1e9
#define NANOSEC2SEC 1e-9

/*! @brief get time for logic purposes, like for random number generators
 *  that require a seed.
 *
 *  get an unsigned 64-bit integer value based on unix time,
 *  but added nanoseconds of unix time.
 *
 *  @remark not accurate to actual time.
 */
u64 get_time_logic(void);

/*! @brief get elapsed nanoseconds since this function's first call in the process.
 */
u64 get_time_u64(void);

/*! @brief get elapsed time since this function's first call in the process,
 *  in seconds and fractional milliseconds.
 *
 *  @remark this function is called inside 'core.c/engine_init()' to automatically
 *  initialize time.
 */
f64 get_time_f64(void);

/*! @brief get elapsed time since this function's last call in the process,
 *  in seconds and fractional milliseconds.
 */
f64 get_time_delta_f64(void);

b8 get_timer(f64 *time_start, f32 interval);

#endif /* ENGINE_TIME_H */
