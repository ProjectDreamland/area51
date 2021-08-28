/* float.h for R5900.  Differs from IEEE 754 in that:

   - NaNs and infinities are not supported; the exponent normally used
     for them can be used for normal numbers instead
   - Rounding is always towards 0 */

#ifndef _FLOAT_H_
#define _FLOAT_H_

   /* Radix of exponent representation */
#undef FLT_RADIX
#define FLT_RADIX 2
   /* Number of base-FLT_RADIX digits in the significand of a float */
#undef FLT_MANT_DIG
#define FLT_MANT_DIG 24
   /* Number of decimal digits of precision in a float */
#undef FLT_DIG
#define FLT_DIG 6
   /* Addition rounds to 0: zero, 1: nearest, 2: +inf, 3: -inf, -1: unknown */
#undef FLT_ROUNDS
#define FLT_ROUNDS 0
   /* Difference between 1.0 and the minimum float greater than 1.0 */
#undef FLT_EPSILON
#define FLT_EPSILON 0X1P-23f
   /* Minimum int x such that FLT_RADIX**(x-1) is a normalised float */
#undef FLT_MIN_EXP
#define FLT_MIN_EXP (-125)
   /* Minimum normalised float */
#undef FLT_MIN
#define FLT_MIN 0X1.000002P-126f
   /* Minimum int x such that 10**x is a normalised float */
#undef FLT_MIN_10_EXP
#define FLT_MIN_10_EXP (-37)
   /* Maximum int x such that FLT_RADIX**(x-1) is a representable float */
#undef FLT_MAX_EXP
#define FLT_MAX_EXP 129
   /* Maximum float */
#undef FLT_MAX
#define FLT_MAX 0X1.FFFFFEP128f
   /* Maximum int x such that 10**x is a representable float */
#undef FLT_MAX_10_EXP
#define FLT_MAX_10_EXP 38

   /* Number of base-FLT_RADIX digits in the significand of a double */
#undef DBL_MANT_DIG
#define DBL_MANT_DIG 53
   /* Number of decimal digits of precision in a double */
#undef DBL_DIG
#define DBL_DIG 15
   /* Difference between 1.0 and the minimum double greater than 1.0 */
#undef DBL_EPSILON
#define DBL_EPSILON 2.2204460492503131e-16
   /* Minimum int x such that FLT_RADIX**(x-1) is a normalised double */
#undef DBL_MIN_EXP
#define DBL_MIN_EXP (-1021)
   /* Minimum normalised double */
#undef DBL_MIN
#define DBL_MIN 2.2250738585072014e-308
   /* Minimum int x such that 10**x is a normalised double */
#undef DBL_MIN_10_EXP
#define DBL_MIN_10_EXP (-307)
   /* Maximum int x such that FLT_RADIX**(x-1) is a representable double */
#undef DBL_MAX_EXP
#define DBL_MAX_EXP 1024
   /* Maximum double */
#undef DBL_MAX
#define DBL_MAX 1.7976931348623157e+308
   /* Maximum int x such that 10**x is a representable double */
#undef DBL_MAX_10_EXP
#define DBL_MAX_10_EXP 308

   /* Maximum positive double value which can be convert to normal float value */
#define DBL_MAX_FLT_COMPAT 0x1.fffffffffffffp128
   /* Minimum positive double value which can be convert to normal float value */
#define DBL_MIN_FLT_COMPAT 0x1.0000000000000p-126

#endif /*  _FLOAT_H___ */
