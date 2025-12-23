#include "h/platform.h"

#if PLATFORM_LINUX

#include <time.h>
#include <sys/time.h>

#include "h/time.h"

u64 get_time_logic(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)(ts.tv_sec + ts.tv_nsec);
}

u64 get_time_u64(void)
{
    static u64 _time = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (!_time) _time = (u64)(ts.tv_sec * 1e9 + ts.tv_nsec);
    return (u64)(ts.tv_sec * 1e9 + ts.tv_nsec) - _time;
}

f64 get_time_f64(void)
{
    static u64 _time = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (!_time) _time = ts.tv_sec;
    return (f64)(ts.tv_sec - _time) + (f64)ts.tv_nsec / 1e9;
}

f64 get_time_delta_f64(void)
{
    static u64 _curr = 0;
    static u64 _last = 0;
    static u64 _delta = 0;
    f64 delta = 0.0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _curr = ts.tv_sec * 1e9 + ts.tv_nsec;
    _delta = _curr - _last;
    _last = _curr;
    delta = (f64)_delta * 1e-9;
    return delta > 0.1 ? 0.1 : delta;
}

b8 get_timer(f64 *time_start, f32 interval)
{
    f64 time_current = get_time_f64();
    if (!*time_start || time_current - *time_start >= interval)
    {
        *time_start = time_current;
        return TRUE;
    }
    return FALSE;
}

#elif PLATFORM_WIN /* TODO: make time functions for windows */

u64 get_time_logic(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (u64)(ts.tv_sec + ts.tv_nsec);
}

f64 get_time_f64(void)
{
    static u64 _time = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (!_time) _time = ts.tv_sec;
    return (f64)(ts.tv_sec - _time) + (f64)ts.tv_nsec / 1e9;
}

f64 get_time_delta_f64(void)
{
    static u64 _curr = 0;
    static u64 _last = 0;
    static u64 _delta = 0;
    f64 delta = 0.0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _curr = ts.tv_sec * 1e9 + ts.tv_nsec;
    _delta = _curr - _last;
    _last = _curr;
    delta = (f64)_delta * 1e-9;
    return delta > 0.1 ? 0.1 : delta;
}

b8 get_timer(f64 *time_start, f32 interval)
{
    f64 time_current = get_time_f64();
    if (time_current - *time_start >= interval)
    {
        *time_start = time_current;
        return TRUE;
    }
    return FALSE;
}

#endif /* PLATFORM */
