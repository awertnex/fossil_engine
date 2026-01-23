#include "h/common.h"

#if FSL_PLATFORM_WIN
/* TODO: make time functions for windows */
#else
#   include <time.h>
#   include <sys/time.h>
#   include <unistd.h>

#   include "h/limits.h"
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

b8 fsl_get_timer(f64 *time_start, f32 interval)
{
    f64 _cur = fsl_get_time_nsecf();
    if (!*time_start || _cur - *time_start >= interval)
    {
        *time_start = _cur;
        return TRUE;
    }
    return FALSE;
}

void fsl_sleep_nsec(u64 nsec)
{
    u64 sec = nsec * FSL_NSEC2SEC;
    struct timespec ts = {.tv_sec = sec, .tv_nsec = nsec - sec * FSL_SEC2NSEC};
    clock_nanosleep(CLOCK_MONOTONIC, 0, &ts, NULL);
}
#endif /* FSL_PLATFORM */
