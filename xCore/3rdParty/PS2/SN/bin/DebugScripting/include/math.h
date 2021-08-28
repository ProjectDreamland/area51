#ifndef _MATH
#define _MATH

#pragma push_safeptr

#define HUGE_VAL 1.0e38

double acos(double a);
double asin(double a);
double atan(double a);
double atan2(double a, double b);
double cos(double a);
double sin(double a);
double tan(double a);
double cosh(double a);
double sinh(double a);
double tanh(double a);
double exp(double a);
double frexp(double a, int * b);
double ldexp(double a, int b);
double log(double a);
double log10(double a);
double modf(double a, double *b);
double pow(double a, double b);
double sqrt(double a);
double ceil(double a);
double fabs(double a);
double floor(double a);
double fmod(double a, double b);

#pragma pop_ptr

#endif /* _MATH */






