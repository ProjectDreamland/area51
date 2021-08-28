#ifdef __sysvnecv70_target
double EXFUN(fast_sin,(double));
double EXFUN(fast_cos,(double));
double EXFUN(fast_tan,(double));

double EXFUN(fast_asin,(double));
double EXFUN(fast_acos,(double));
double EXFUN(fast_atan,(double));

double EXFUN(fast_sinh,(double));
double EXFUN(fast_cosh,(double));
double EXFUN(fast_tanh,(double));

double EXFUN(fast_asinh,(double));
double EXFUN(fast_acosh,(double));
double EXFUN(fast_atanh,(double));

double EXFUN(fast_abs,(double));
double EXFUN(fast_sqrt,(double));
double EXFUN(fast_exp2,(double));
double EXFUN(fast_exp10,(double));
double EXFUN(fast_expe,(double));
double EXFUN(fast_log10,(double));
double EXFUN(fast_log2,(double));
double EXFUN(fast_loge,(double));


#define	sin(x)		fast_sin(x)
#define	cos(x)		fast_cos(x)
#define	tan(x)		fast_tan(x)
#define	asin(x)		fast_asin(x)
#define	acos(x)		fast_acos(x)
#define	atan(x)		fast_atan(x)
#define	sinh(x)		fast_sinh(x)
#define	cosh(x)		fast_cosh(x)
#define	tanh(x)		fast_tanh(x)
#define	asinh(x)	fast_asinh(x)
#define	acosh(x)	fast_acosh(x)
#define	atanh(x)	fast_atanh(x)
#define	abs(x)		fast_abs(x)
#define	sqrt(x)		fast_sqrt(x)
#define	exp2(x)		fast_exp2(x)
#define	exp10(x)	fast_exp10(x)
#define	expe(x)		fast_expe(x)
#define	log10(x)	fast_log10(x)
#define	log2(x)		fast_log2(x)
#define	loge(x)		fast_loge(x)

#ifdef _HAVE_STDC
/* These functions are in assembler, they really do take floats. This
   can only be used with a real ANSI compiler */

float EXFUN(fast_sinf,(float));
float EXFUN(fast_cosf,(float));
float EXFUN(fast_tanf,(float));

float EXFUN(fast_asinf,(float));
float EXFUN(fast_acosf,(float));
float EXFUN(fast_atanf,(float));

float EXFUN(fast_sinhf,(float));
float EXFUN(fast_coshf,(float));
float EXFUN(fast_tanhf,(float));

float EXFUN(fast_asinhf,(float));
float EXFUN(fast_acoshf,(float));
float EXFUN(fast_atanhf,(float));

float EXFUN(fast_absf,(float));
float EXFUN(fast_sqrtf,(float));
float EXFUN(fast_exp2f,(float));
float EXFUN(fast_exp10f,(float));
float EXFUN(fast_expef,(float));
float EXFUN(fast_log10f,(float));
float EXFUN(fast_log2f,(float));
float EXFUN(fast_logef,(float));
#define	sinf(x)		fast_sinf(x)
#define	cosf(x)		fast_cosf(x)
#define	tanf(x)		fast_tanf(x)
#define	asinf(x)	fast_asinf(x)
#define	acosf(x)	fast_acosf(x)
#define	atanf(x)	fast_atanf(x)
#define	sinhf(x)	fast_sinhf(x)
#define	coshf(x)	fast_coshf(x)
#define	tanhf(x)	fast_tanhf(x)
#define	asinhf(x)	fast_asinhf(x)
#define	acoshf(x)	fast_acoshf(x)
#define	atanhf(x)	fast_atanhf(x)
#define	absf(x)		fast_absf(x)
#define	sqrtf(x)	fast_sqrtf(x)
#define	exp2f(x)	fast_exp2f(x)
#define	exp10f(x)	fast_exp10f(x)
#define	expef(x)	fast_expef(x)
#define	log10f(x)	fast_log10f(x)
#define	log2f(x)	fast_log2f(x)
#define	logef(x)	fast_logef(x)
#endif
/* Override the functions defined in math.h */
#endif /* __sysvnecv70_target */


#ifdef __i386__
#if defined(__GNUC__) && __STDC__ - 0 > 0

#define __str1__(x) #x
#define __str2__(x) __str1__(x)
#define __U_L_PREFIX__ __str2__(__USER_LABEL_PREFIX__)

__extension__ double atan2(double, double)
  __asm__(__U_L_PREFIX__ "_f_atan2");
__extension__ double exp(double)
  __asm__(__U_L_PREFIX__ "_f_exp");
__extension__ double frexp(double, int*)
  __asm__(__U_L_PREFIX__ "_f_frexp");
__extension__ double ldexp(double, int)
  __asm__(__U_L_PREFIX__ "_f_ldexp");
__extension__ double log(double)
  __asm__(__U_L_PREFIX__ "_f_log");
__extension__ double log10(double)
  __asm__(__U_L_PREFIX__ "_f_log10");
__extension__ double pow(double, double)
  __asm__(__U_L_PREFIX__ "_f_pow");
__extension__ double tan(double)
  __asm__(__U_L_PREFIX__ "_f_tan");

#ifndef __STRICT_ANSI__
__extension__ float atan2f(float, float)
  __asm__(__U_L_PREFIX__ "_f_atan2f");
__extension__ float expf(float)
  __asm__(__U_L_PREFIX__ "_f_expf");
__extension__ float frexpf(float, int*)
  __asm__(__U_L_PREFIX__ "_f_frexpf");
__extension__ float ldexpf(float, int)
  __asm__(__U_L_PREFIX__ "_f_ldexpf");
__extension__ float logf(float)
  __asm__(__U_L_PREFIX__ "_f_logf");
__extension__ float log10f(float)
  __asm__(__U_L_PREFIX__ "_f_log10f");
__extension__ float powf(float, float)
  __asm__(__U_L_PREFIX__ "_f_powf");
__extension__ float tanf(float)
  __asm__(__U_L_PREFIX__ "_f_tanf");
#endif

#else

double EXFUN(_f_atan2,(double, double));
double EXFUN(_f_exp,(double));
double EXFUN(_f_frexp,(double, int*));
double EXFUN(_f_ldexp,(double, int));
double EXFUN(_f_log,(double));
double EXFUN(_f_log10,(double));
double EXFUN(_f_pow,(double, double));

float EXFUN(_f_atan2f,(float, float));
float EXFUN(_f_expf,(float));
float EXFUN(_f_frexpf,(float, int*));
float EXFUN(_f_ldexpf,(float, int));
float EXFUN(_f_logf,(float));
float EXFUN(_f_log10f,(float));
float EXFUN(_f_powf,(float, float));

#define atan2(y,x)    _f_atan2((y),(x))
#define exp(x)                _f_exp(x)
#define frexp(x,p)    _f_frexp((x),(p))
#define ldexp(x,e)    _f_ldexp((x),(e))
#define log(x)                _f_log(x)
#define log10(x)      _f_log10(x)
#define pow(x,y)      _f_pow((x),(y))

#ifndef __STRICT_ANSI__
#define atan2f(y,x)   _f_atan2f((y),(x))
#define expf(x)               _f_expf(x)
#define frexpf(x,p)   _f_frexpf((x),(p))
#define ldexpf(x,e)   _f_ldexpf((x),(e))
#define logf(x)               _f_logf(x)
#define log10f(x)     _f_log10f(x)
#define powf(x,y)     _f_powf((x),(y))
#endif

#endif /* GCC */
#endif /* __i386__ */

