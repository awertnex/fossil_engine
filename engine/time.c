#include "h/common.h"

#if PLATFORM_LINUX

#include <time.h>
#include <sys/time.h>
#include <unistd.h>

#include "h/limits.h"
#include "h/time.h"

u64 get_time_raw_nsec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (u64)(ts.tv_sec * SEC2NSEC + ts.tv_nsec);
}

u64 get_time_raw_usec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return (u64)(ts.tv_sec * SEC2USEC + ts.tv_nsec * NSEC2USEC);
}

u64 get_time_nsec(void)
{
    static u64 _time = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (!_time) _time = (u64)(ts.tv_sec * SEC2NSEC + ts.tv_nsec);
    return (u64)(ts.tv_sec * SEC2NSEC + ts.tv_nsec) - _time;
}

f64 get_time_nsecf(void)
{
    static u64 _time = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    if (!_time) _time = ts.tv_sec;
    return (f64)(ts.tv_sec - _time) + (f64)ts.tv_nsec * NSEC2SEC;
}

u64 get_time_delta_nsec(void)
{
    static u64 _curr = 0;
    static u64 _last = 0;
    static u64 _delta = 0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _curr = ts.tv_sec * SEC2NSEC + ts.tv_nsec;
    if (!_last) _last = _curr;
    _delta = _curr - _last;
    _last = _curr;
    return _delta;
}

void get_time_str(str *buf, const str *format)
{
    u64 _time_nsec = get_time_raw_nsec();
    time_t _time = _time_nsec * NSEC2SEC;
    struct tm *_tm = localtime(&_time);
    strftime(buf, TIME_STRING_MAX, format, _tm);
}

b8 get_timer(f64 *time_start, f32 interval)
{
    f64 _cur = get_time_nsecf();
    if (!*time_start || _cur - *time_start >= interval)
    {
        *time_start = _cur;
        return TRUE;
    }
    return FALSE;
}

void sleep_nsec(u64 nsec)
{
    u64 sec = nsec * NSEC2SEC;
    struct timespec ts = {.tv_sec = sec, .tv_nsec = nsec - sec * SEC2NSEC};
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
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
    return (f64)(ts.tv_sec - _time) + (f64)ts.tv_nsec * NSEC2SEC;
}

f64 get_time_delta_f64(void)
{
    static u64 _curr = 0;
    static u64 _last = 0;
    static u64 _delta = 0;
    f64 delta = 0.0;
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    _curr = ts.tv_sec * SEC2NSEC + ts.tv_nsec;
    _delta = _curr - _last;
    _last = _curr;
    delta = (f64)_delta * NSEC2SEC;
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

void sleep_ns(u64 ns)
{
    u64 sec = nsec * NSEC2SEC;
    struct timespec ts = {.tv_sec = sec, .tv_nsec = nsec - sec * SEC2NSEC};
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}

#endif /* PLATFORM */
