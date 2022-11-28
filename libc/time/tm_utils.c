
#include <stdint.h>
#include <time.h>
#include "ctime.h"

//-------------------------------------------------------------------------------------------------

typedef struct
{
    int yr;                     /* year of interest */
    int yd;                     /* day of year */
    long ms;                    /* milli-seconds in the day */
} transitiondate;

/*
 * DST start and end structs.
 */
static transitiondate dststart = { -1, 0, 0L };
static transitiondate dstend   = { -1, 0, 0L };

int  _daylight = 1;                     /* daylight saving default enabled */
long _dstbias = -1 * 60L * 60L;         /* bias for daylight saving in seconds */
long _timezone = 5L * 60L * 60L;        /* default time zone is EST = -05:00:00 */

/*
 * The macro below is valid for years between 1901 and 2099, which easily
 * includes all years representable by the current implementation of time_t.
 */
#define IS_LEAP_YEAR(year)  ((year & 3) == 0)
#define DAY_MILLISEC        (24L * 60L * 60L * 1000L)

static void cvtdate(int trantype,
                    int datetype,
                    int year,
                    int month,
                    int week,
                    int dayofweek,
                    int date,
                    int hour,
                    int min,
                    int sec,
                    int msec)
{
    int yearday;
    int monthdow;

    if (datetype == 1)
    {
        /*
         * Transition day specified in day-in-month format.
         */

        /* Figure the year-day of the start of the month.
         */
        yearday = 1 + (IS_LEAP_YEAR(year) ? _lpdays[month - 1] : _days[month - 1]);

        /* Figure the day of the week of the start of the month.
         */
        monthdow = (yearday + ((year - 70) * 365) + ((year - 1) >> 2) - _LEAP_YEAR_ADJUST + _BASE_DOW) % 7;

        /* Figure the year-day of the transition date
         */
        if (monthdow <= dayofweek)
            yearday += (dayofweek - monthdow) + (week - 1) * 7;
        else
            yearday += (dayofweek - monthdow) + week * 7;

        /* May have to adjust the calculation above if week == 5 (meaning the last instance of
         * the day in the month). Check if year falls beyond after month and adjust accordingly.
         */
        if ((week == 5) && (yearday > (IS_LEAP_YEAR(year) ? _lpdays[month] : _days[month])))
        {
            yearday -= 7;
        }
    }
    else
    {
        /* Transition day specified as an absolute day
         */
        yearday = IS_LEAP_YEAR(year) ? _lpdays[month - 1] : _days[month - 1];
        yearday += date;
    }

    if (trantype == 1)
    {
        /* Converted date was for the start of DST
         */
        dststart.yd = yearday;
        dststart.ms = (long) msec + (1000L * (sec + 60L * (min + 60L * hour)));
        
        /* Set year field of dststart so that unnecessary calls to cvtdate() may be avoided.
         */
        dststart.yr = year;
    }
    else
    {
        /* Converted date was for the end of DST
         */
        dstend.yd = yearday;
        dstend.ms = (long) msec + (1000L * (sec + 60L * (min + 60L * hour)));
        
        /* The converted date is still a DST date. Must convert to a standard (local) date
         * while being careful the millisecond field does not overflow or underflow.
         */
        if ((dstend.ms += (_dstbias * 1000L)) < 0)
        {
            dstend.ms += DAY_MILLISEC;
            dstend.yd--;
        }
        else if (dstend.ms >= DAY_MILLISEC)
        {
            dstend.ms -= DAY_MILLISEC;
            dstend.yd++;
        }

        /* Set year field of dstend so that unnecessary calls to cvtdate() may be avoided.
         */
        dstend.yr = year;
    }

    return;
}

int _isindst(struct tm *tb)
{
    long ms;

    if (_daylight == 0)
        return 0;

    /*
     * Compute (recompute) the transition dates for daylight saving time
     * if necessary.The yr (year) fields of dststart and dstend is
     * compared to the year of interest to determine necessity.
     */
    if ((tb->tm_year != dststart.yr) || (tb->tm_year != dstend.yr))
    {
        cvtdate(1, 1, tb->tm_year, 3,   /* March */
                5,                      /* last... */
                0,                      /* ...Sunday */
                0, 2,                   /* 02:00 (2 AM) */
                0, 0, 0);

        cvtdate(0, 1, tb->tm_year, 10,  /* October */
                5,                      /* last... */
                0,                      /* ...Sunday */
                0, 2,                   /* 02:00 (2 AM) */
                0, 0, 0);
    }

    /* Handle simple cases first.
     */
    if (dststart.yd < dstend.yd)
    {
        /* Northern hemisphere ordering
         */
        if ((tb->tm_yday < dststart.yd) || (tb->tm_yday > dstend.yd))
            return 0;
        if ((tb->tm_yday > dststart.yd) && (tb->tm_yday < dstend.yd))
            return 1;
    }
    else
    {
        /* Southern hemisphere ordering
         */
        if ((tb->tm_yday < dstend.yd) || (tb->tm_yday > dststart.yd))
            return 1;
        if ((tb->tm_yday > dstend.yd) && (tb->tm_yday < dststart.yd))
            return 0;
    }

    ms = 1000L * (tb->tm_sec + 60L * tb->tm_min + 3600L * tb->tm_hour);

    if (tb->tm_yday == dststart.yd)
    {
        if (ms >= dststart.ms)
            return 1;
        else
            return 0;
    }
    else
    {
        if (ms < dstend.ms)
            return 1;
        else
            return 0;
    }
}


