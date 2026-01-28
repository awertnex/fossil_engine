/*  Copyright 2026 Lily Awertnex
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
 *  limitations under the License.OFTWARE.
 */

/*  time.h - get time, limit framerate, set timer and get time window
 */

#include "h/common.h"

#if FSL_PLATFORM_WIN
/* TODO: make time functions for windows */
#else
#   include <time.h>
#   include <sys/time.h>
#   include <unistd.h>

#   include "h/limits.h"
#   include "h/math.h"
#   include "h/time.h"

u64 fsl_get_time_raw_nsec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (u64)(ts.tv_sec * FSL_SEC2NSEC + ts.tv_nsec);
}

u64 fsl_get_time_raw_usec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (u64)(ts.tv_sec * FSL_SEC2USEC + ts.tv_nsec * FSL_MSEC2SEC);
}

u64 fsl_get_time_nsec(void)
{
    static u64 _time = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (!_time) _time = (u64)(ts.tv_sec * FSL_SEC2NSEC + ts.tv_nsec);
    return (u64)(ts.tv_sec * FSL_SEC2NSEC + ts.tv_nsec) - _time;
}

f64 fsl_get_time_nsecf(void)
{
    static u64 _time = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (!_time) _time = ts.tv_sec;
    return (f64)(ts.tv_sec - _time) + (f64)ts.tv_nsec * FSL_NSEC2SEC;
}

u64 fsl_get_time_delta_nsec(void)
{
    static u64 _curr = 0;
    static u64 _last = 0;
    static u64 _delta = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _curr = ts.tv_sec * FSL_SEC2NSEC + ts.tv_nsec;
    if (!_last) _last = _curr;
    _delta = _curr - _last;
    _last = _curr;
    return _delta;
}

void fsl_get_time_str(str *dst, const str *format)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    struct tm *_tm = localtime(&ts.tv_sec);
    strftime(dst, FSL_TIME_STRING_MAX, format, _tm);
}

b8 fsl_is_in_time_window(u64 *_time, u64 interval, u64 _curr)
{
    if (!*_time || _curr - *_time >= interval)
    {
        *_time = _curr;
        return TRUE;
    }
    return FALSE;
}

b8 fsl_on_time_interval(u64 *_time, u64 interval, u64 _curr)
{
    if (!interval)
        return TRUE;

    if (_curr < *_time + interval)
        return FALSE;

    *_time += interval;
    return TRUE;
}

void fsl_sleep_nsec(u64 nsec)
{
    u64 sec = nsec * FSL_NSEC2SEC;
    struct timespec ts = {.tv_sec = sec, .tv_nsec = nsec - sec * FSL_SEC2NSEC};
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}

void fsl_limit_framerate(u64 target_fps, u64 _curr)
{
    static u64 _time = 0;

    if (!target_fps)
    {
        _time = _curr;
        return;
    }

    _time += FSL_SEC2NSEC / fsl_clamp_u64(target_fps, FSL_TARGET_FPS_MIN, FSL_TARGET_FPS_MAX);

    if (_curr < _time)
        fsl_sleep_nsec(_time - _curr);
    else _time = _curr;
}
#endif /* FSL_PLATFORM */
