/*
 * strtod.c
 */

#include <stdlib.h>
#include <errno.h>
#include <ctype.h>

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

static int max_exponent = 511;	/* Largest possible base 10 exponent. Any exponent
                                 * larger than this will already produce underflow
                                 * or overflow, so there's no need to worry about
                                 * additional digits.
				                 */
static double powers_10[] =  	/* Table giving binary powers of 10.  Entry */
{
    10.,			            /* is 10^2^i.  Used to convert decimal */
    100.,			            /* exponents into floating-point numbers. */
    1.0e4,
    1.0e8,
    1.0e16,
    1.0e32,
    1.0e64,
    1.0e128,
    1.0e256
};

/* const char *string;	A decimal ASCII floating-point number, optionally preceded
 *                      by white space.
 *                      Must have form "-I.FE-X", where I is the integer part of the
 *                      mantissa, F is the fractional part of the mantissa, and X is
 *                      the exponent.  Either of the signs may be "+", "-", or omitted.
 *                      Either I or F may be omitted, or both.  The decimal point
 *                      isn't necessary unless F is present. The "E" may actually be
 *                      an "e".  E and X may both be omitted (but not just one).
 *
 * char **endptr;		If non-NULL, store terminating character's address here.
 */

double strtod(const char *string, char **endptr)
{
    int sign, exp_sign = FALSE;
    double fraction, dbl_exp, *d;
    register const char *p;
    register int c;
    int exp = 0;            /* Exponent read from "EX" field. */
    int frac_exp = 0;       /* Exponent that derives from the fractional part. Under
                               normal circumstatnces, it is the negative of the number
                               of digits in F. However, if I is very long, the last
                               digits of I get dropped (otherwise a long I with a large
				               negative exponent could cause an unnecessary overflow on
                               I alone).  In this case, frac_exp is incremented one for
                               each dropped digit. */
    int mant_size;          /* Number of digits in mantissa. */
    int dec_pt;			    /* Number of mantissa digits BEFORE decimal point. */
    const char *p_exp;		/* Temporarily holds location of exponent in string. */

    /*
     * Strip off leading blanks and check for a sign.
     */
    p = string;
    while (isspace(*p))
    {
        p += 1;
    }
    
    if (*p == '-')
    {
        sign = TRUE;
        p += 1;
    }
    else
    {
        if (*p == '+')
        {
            p += 1;
        }
        sign = FALSE;
    }

    /*
     * Count the number of digits in the mantissa (including the decimal
     * point), and also locate the decimal point.
     */
    dec_pt = -1;
    for (mant_size = 0; ; mant_size += 1)
    {
        c = *p;
        if (!isdigit(c))
        {
            if ((c != '.') || (dec_pt >= 0))
            {
                break;
            }
            dec_pt = mant_size;
        }
        p += 1;
    }

    /*
     * Now suck up the digits in the mantissa.  Use two integers to collect 9 digits
     * each (this is faster than using floating-point).
     * If the mantissa has more than 18 digits, ignore the extras, since they can't
     * affect the value anyway.
     */
    p_exp  = p;
    p -= mant_size;
    if (dec_pt < 0)
    {
        dec_pt = mant_size;
    }
    else
    {
        mant_size -= 1;			/* One of the digits was the point. */
    }
    
    if (mant_size > 18)
    {
        frac_exp = dec_pt - 18;
        mant_size = 18;
    }
    else
    {
        frac_exp = dec_pt - mant_size;
    }
    
    if (mant_size == 0)
    {
        fraction = 0.0;
        p = string;
        goto done;
    }
    else
    {
        int frac1, frac2;
        
        frac1 = 0;
        for ( ; mant_size > 9; mant_size -= 1)
        {
            c = *p;
            p += 1;
            if (c == '.')
            {
                c = *p;
                p += 1;
            }
            frac1 = 10*frac1 + (c - '0');
        }
        
        frac2 = 0;
        for (; mant_size > 0; mant_size -= 1)
        {
            c = *p;
            p += 1;
            if (c == '.')
            {
                c = *p;
                p += 1;
            }
            frac2 = 10*frac2 + (c - '0');
        }
        fraction = (1.0e9 * frac1) + frac2;
    }

    /*
     * Skim off the exponent.
     */
    p = p_exp;
    if ((*p == 'E') || (*p == 'e'))
    {
        p += 1;
        if (*p == '-')
        {
            exp_sign = TRUE;
            p += 1;
        }
        else
        {
            if (*p == '+')
            {
                p += 1;
            }
            exp_sign = FALSE;
        }
        
        while (isdigit(*p))
        {
            exp = exp * 10 + (*p - '0');
            p += 1;
        }
    }
    
    if (exp_sign)
    {
        exp = frac_exp - exp;
    }
    else
    {
        exp = frac_exp + exp;
    }

    /*
     * Generate a floating-point number that represents the exponent. Do this by
     * processing the exponent one bit at a time to combine many powers of 2 of 10.
     * Then combine the exponent with the fraction.
     */
    if (exp < 0)
    {
        exp_sign = TRUE;
        exp = -exp;
    }
    else
    {
        exp_sign = FALSE;
    }
    
    if (exp > max_exponent)
    {
        exp = max_exponent;
        errno = ERANGE;
    }
    
    dbl_exp = 1.0;
    for (d = powers_10; exp != 0; exp >>= 1, d += 1)
    {
        if (exp & 01)
        {
            dbl_exp *= *d;
        }
    }
    
    if (exp_sign)
    {
        fraction /= dbl_exp;
    }
    else
    {
        fraction *= dbl_exp;
    }

done:
    if (endptr != NULL)
    {
        *endptr = (char *) p;
    }

    if (sign)
    {
        return -fraction;
    }
    return fraction;
}


