/*
 * mktime.c
 *
 * created: 2021-08-04
 *  author:
 */

#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include "ctime.h"

/*
 * ChkAdd evaluates to TRUE if dest = src1 + src2 has overflowed
 */
#define ChkAdd(dest, src1, src2)   (((src1 >= 0L) && (src2 >= 0L) && \
                                     (dest < 0L)) || ((src1 < 0L) && \
                                     (src2 < 0L) && (dest >= 0L)))

/*
 * ChkMul evaluates to TRUE if dest = src1 * src2 has overflowed
 */
#define ChkMul(dest, src1, src2)   (src1 ? (dest/src1 != src2) : 0)

static time_t _make_time_t(struct tm *tb, int ultflag)
{
    long tmptm1, tmptm2, tmptm3;
    struct tm *tbtemp;

    if (NULL == tb)
        goto err_mktime;

    /* First, make sure tm_year is reasonably close to being in range.
     */
    if (((tmptm1 = tb->tm_year) < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1))
        goto err_mktime;

    /* Adjust month value so it is in the range 0 - 11.  This is because
     * we don't know how many days are in months 12, 13, 14, etc.
     */

    if ((tb->tm_mon < 0) || (tb->tm_mon > 11))
    {
        /* no danger of overflow because the range check above.
         */
        tmptm1 += (tb->tm_mon / 12);

        if ((tb->tm_mon %= 12) < 0)
        {
            tb->tm_mon += 12;
            tmptm1--;
        }

        /* Make sure year count is still in range.
         */
        if ((tmptm1 < _BASE_YEAR - 1) || (tmptm1 > _MAX_YEAR + 1))
            goto err_mktime;
    }

    /***** HERE: tmptm1 holds number of elapsed years *****/

    /* Calculate days elapsed minus one, in the given year, to the given
     * month. Check for leap year and adjust if necessary.
     */
    tmptm2 = _days[tb->tm_mon];
    if (!(tmptm1 & 3) && (tb->tm_mon > 1))
        tmptm2++;

    /* Calculate elapsed days since base date (midnight, 1/1/70, UTC)
     *
     * 365 days for each elapsed year since 1970, plus one more day for
     * each elapsed leap year. no danger of overflow because of the range
     * check (above) on tmptm1.
     */
    tmptm3 = (tmptm1 - _BASE_YEAR) * 365L + ((tmptm1 - 1L) >> 2) - _LEAP_YEAR_ADJUST;

    /* elapsed days to current month (still no possible overflow)
     */
    tmptm3 += tmptm2;

    /* elapsed days to current date. overflow is now possible.
     */
    tmptm1 = tmptm3 + (tmptm2 = (long) (tb->tm_mday));
    if (ChkAdd(tmptm1, tmptm3, tmptm2))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed days *****/

    /* Calculate elapsed hours since base date
     */
    tmptm2 = tmptm1 * 24L;
    if (ChkMul(tmptm2, tmptm1, 24L))
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long) tb->tm_hour);
    if (ChkAdd(tmptm1, tmptm2, tmptm3))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed hours *****/

    /* Calculate elapsed minutes since base date
     */
    tmptm2 = tmptm1 * 60L;
    if (ChkMul(tmptm2, tmptm1, 60L))
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long) tb->tm_min);
    if (ChkAdd(tmptm1, tmptm2, tmptm3))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed minutes *****/

    /* Calculate elapsed seconds since base date
     */
    tmptm2 = tmptm1 * 60L;
    if (ChkMul(tmptm2, tmptm1, 60L))
        goto err_mktime;

    tmptm1 = tmptm2 + (tmptm3 = (long) tb->tm_sec);
    if (ChkAdd(tmptm1, tmptm2, tmptm3))
        goto err_mktime;

    /***** HERE: tmptm1 holds number of elapsed seconds *****/

    if (ultflag)
    {
        /* Adjust for timezone. No need to check for overflow since
         * localtime() will check its arg value
         */
        tmptm1 += _timezone;

        /* Convert this second count back into a time block structure.
         * If localtime returns NULL, return an error.
         */
        if ((tbtemp = localtime(&tmptm1)) == NULL)
            goto err_mktime;

        /* Now must compensate for DST. The ANSI rules are to use the
         * passed-in tm_isdst flag if it is non-negative. Otherwise,
         * compute if DST applies. Recall that tbtemp has the time without
         * DST compensation, but has set tm_isdst correctly.
         */
        if ((tb->tm_isdst > 0) || ((tb->tm_isdst < 0) && (tbtemp->tm_isdst > 0)))
        {
            tmptm1 += _dstbias;
            tbtemp = localtime(&tmptm1);        /* reconvert, can't get NULL */
        }

    }
    else
    {
        if ((tbtemp = gmtime(&tmptm1)) == NULL)
            goto err_mktime;
    }

    /***** HERE: tmptm1 holds number of elapsed seconds, adjusted *****/
    /*****       for local time if requested                      *****/

    *tb = *tbtemp;
    return (time_t)tmptm1;

err_mktime:
    
    /* All errors come to here */
    return (time_t) (0);
}

time_t mktime(struct tm *timeptr)
{
    return (_make_time_t(timeptr, 1));
}


