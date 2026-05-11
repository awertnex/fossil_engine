/*!
 *  Copyright 2026 Lily Awertnex
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

/*!
 *  @file time.h
 *
 *  @brief get time, limit framerate, set timer and get time window.
 */

#ifndef FSL_TIME_H
#define FSL_TIME_H

#include "common.h"
#include "types.h"

#define FSL_SEC2MSEC 1000UL
#define FSL_MSEC2SEC 0.001
#define FSL_SEC2USEC 1000000UL
#define FSL_USEC2SEC 0.000001
#define FSL_SEC2NSEC 1000000000UL
#define FSL_NSEC2SEC 0.000000001

/*!
 *  @brief get elapsed nanoseconds since the unix epoch, 1970-01-01 00:00:00 UTC.
 */
FSLAPI u64 fsl_get_time_raw_nsec(void);

/*!
 *  @brief get elapsed milliseconds since the unix epoch, 1970-01-01 00:00:00 UTC.
 *
 */
FSLAPI u64 fsl_get_time_raw_usec(void);

/*!
 *  @brief get elapsed nanoseconds since this function's first call in the process.
 *
 *  @remark called from @ref fsl_engine_init() to automatically initialize time.
 *
 *  @remark the macro @ref FSL_NSEC2SEC can be used to convert from nanoseconds to
 *  seconds when multiplied by output.
 */
FSLAPI u64 fsl_get_time_nsec(void);

/*!
 *  @brief get elapsed seconds.nanoseconds since this function's first call in the process.
 *
 *  @remark called from @ref fsl_engine_init() to automatically initialize time.
 */
FSLAPI f64 fsl_get_time_nsecf(void);

/*!
 *  @brief get elapsed nanoseconds since this function's last call in the process.
 *
 *  @remark the macro @ref FSL_NSEC2SEC can be used to convert from nanoseconds to
 *  seconds when multiplied by output.
 */
FSLAPI u64 fsl_get_time_delta_nsec(void);

/*!
 *  @brief get time string with max length of @ref FSL_TIME_STRING_MAX as per `format`
 *  and write into `dst`.
 */
FSLAPI void fsl_get_time_str(str *dst, const str *format);

/*!
 *  @brief sleep for `nsec` nanoseconds.
 *
 *  @remark the macro @ref FSL_SEC2NSEC can be used to convert from seconds to
 *  nanoseconds when multiplied by specified value.
 */
FSLAPI void fsl_sleep_nsec(u64 nsec);

/*!
 *  @brief check if current time is within `*t` and `interval`, and reset
 *  `*t` to `curr` if not.
 */
FSLAPI b8 fsl_is_in_time_window(u64 *t, u64 interval, u64 curr);

/*!
 *  @brief check if `*t` + `interval` has been reached by `curr`
 *  (useful for things like enforcing a set framerate without invoking a syscall,
 *  as oppose to @ref fsl_limit_framerate()).
 *
 *  @remark no syscalls used, time is queried from `curr`, usually passed from
 *  @ref fsl_render.time of the currently bound `fsl_render`.
 */
FSLAPI b8 fsl_on_time_interval(u64 *t, u64 interval, u64 curr);

/*!
 *  @brief limit framerate to `target_fps`.
 *
 *  @param curr current time, in nanoseconds.
 *
 *  @remark not thread-safe.
 */
FSLAPI void fsl_limit_framerate(u64 target_fps, u64 curr);

#endif /* FSL_TIME_H */
