
#ifndef _CTIME_H
#define _CTIME_H

#define _DAY_SEC           (24UL * 60UL * 60UL) /* secs in a day */
#define _YEAR_SEC          (365L * _DAY_SEC)    /* secs in a year */
#define _FOUR_YEAR_SEC     (1461L * _DAY_SEC)   /* secs in a 4 year interval */
#define _DEC_SEC           315532800UL          /* secs in 1970-1979 */
#define _BASE_YEAR         70L                  /* 1970 is the base year */
#define _BASE_DOW          4                    /* 01-01-70 was a Thursday */
#define _LEAP_YEAR_ADJUST  17L                  /* Leap years 1900 - 1970 */
#define _MAX_YEAR          138L                 /* 2038 is the max year */

/*
 * in "gmtime.c"
 */
extern int _lpdays[];
extern int _days[];
extern struct tm _tb;

/*
 * in "tm_utils.c"
 */
extern long _dstbias;
extern int _isindst(struct tm *tb);

#endif // _CTIME_H

