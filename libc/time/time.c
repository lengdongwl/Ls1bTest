/*
 * time.c
 */

#include "bsp.h"

#include <time.h>

time_t time(time_t *t)
{
#ifdef BSP_USE_RTC
#include "ls1x_rtc.h"
    struct tm dt;
    time_t cur_time;

    ls1x_rtc_get_datetime(&dt);
    cur_time = mktime(&dt);
    if (t)
        *t = cur_time;
    return cur_time;
    
#else
    time_t cur_time = (time_t)((get_clock_ticks() / 1000));
    if (t)
        *t = cur_time;
    return cur_time;
    
#endif
}



