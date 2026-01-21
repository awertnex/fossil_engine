#ifndef ENGINE_TIME_H
#define ENGINE_TIME_H

#include "common.h"
#include "types.h"

#define SEC2MSEC 1e3
#define MSEC2SEC 1e-3
#define SEC2USEC 1e6
#define USEC2SEC 1e-6
#define SEC2NSEC 1e9
#define NSEC2SEC 1e-9

/*! @brief get elapsed nanoseconds since the unix epoch, 1970-01-01 00:00:00 UTC.
 */
FSLAPI u64 get_time_raw_nsec(void);

/*! @brief get elapsed milliseconds since the unix epoch, 1970-01-01 00:00:00 UTC.
 *
 */
FSLAPI u64 get_time_raw_usec(void);

/*! @brief get elapsed nanoseconds since this function's first call in the process.
 *
 *  @remark called from @ref engine_init() to automatically initialize time.
 *
 *  @remark the macro `NSEC2SEC` can be used to convert from nanoseconds to
 *  seconds when multiplied by output.
 */
FSLAPI u64 get_time_nsec(void);

/*! @brief get elapsed seconds.nanoseconds since this function's first call in the process.
 *
 *  @remark called from @ref engine_init() to automatically initialize time.
 */
FSLAPI f64 get_time_nsecf(void);

/*! @brief get elapsed nanoseconds since this function's last call in the process.
 *
 *  @remark the macro `NSEC2SEC` can be used to convert from nanoseconds to
 *  seconds when multiplied by output.
 */
FSLAPI u64 get_time_delta_nsec(void);

/*! @brief get time string with max length of @ref TIME_STRING_MAX as per `format`
 *  and load into `dst`.
 */
FSLAPI void get_time_str(str *dst, const str *format);

FSLAPI b8 get_timer(f64 *time_start, f32 interval);

/*! @brief sleep for specified nanoseconds.
 *
 *  @remark the macros `SEC2NSEC`, `NSEC2SEC` can be used to convert from
 *  seconds to nanoseconds and vice-versa when multiplied by specified value.
 */
FSLAPI void sleep_nsec(u64 nsec);

#endif /* ENGINE_TIME_H */
