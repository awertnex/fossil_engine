#ifndef ENGINE_TIME_H
#define ENGINE_TIME_H

#include "types.h"

#define SEC2NANOSEC 1e9
#define NANOSEC2SEC 1e-9

/*! @brief get elapsed nanoseconds since the unix epoch, 1970-01-01 00:00:00 UTC.
 */
u64 get_time_raw_u64(void);

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
 *  in nanoseconds.
 *
 *  @remark the macro NANOSEC2SEC can be used to convert from nanoseconds to
 *  fractional seconds when multiplied by output.
 */
u64 get_time_delta_u64(void);

b8 get_timer(f64 *time_start, f32 interval);

/*! @brief sleep for specified nanoseconds.
 *
 *  @remark the macros SEC2NANOSEC, NANOSEC2SEC can be used to convert from
 *  seconds to nanoseconds and vice-versa when multiplied by specified value.
 */
void sleep_nsec(u64 nsec);

#endif /* ENGINE_TIME_H */
