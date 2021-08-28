#ifndef _STDLIBH
#define _STDLIBH

#pragma push_safeptr

#define _need_size_t
#define _need_NULL

#include "sys/stdtypes.h"

#undef _need_NULL
#undef _need_size_t

#include <sys/stdlib.h>


typedef struct {
    int quot, rem;
}div_t;

typedef struct {
    long quot, rem;
}ldiv_t;


/* declaration prototypes */

div_t div(int numer, int denom);
ldiv_t ldiv(long int numer, long int denom);

void * malloc(size_t n);
void * calloc(size_t x, size_t y);
void * realloc(void * ptr, size_t n);

#if defined(_EiC) && 0
#define  free(x)  (free)(&(x));
#endif

void  (free)(void *ptr);


int rand(void);
void srand(unsigned int seed);
double strtod(const char *str, char **endptr);
long strtol(const char *str, char **endptr,int base);

#ifndef _SUNOS
unsigned long strtoul(const char *str, char **endptr,int base);
#else
#define strtoul(x,y,z)         strtol(x,y,z)
#endif

int system(const char * str);
double atof(const char *str);
int atoi(const char *s);
long atol(const char *s);
char *getenv(const char *name);


void exit(int eval);
int atexit(void f(void));
void abort(void);


/* these two macros are unsafe */
#define abs(x)     ((x)>0? (x):-(x))
#define labs(x)    abs(x)

/* non-standard stuff EiC interpreter stuff*/
#ifdef _EiC
#define itoa(x,y,z)     _itoa(x,y,z,1)
#define utoa(x,y,z)     _itoa(x,y,z,2)
#define ltoa(x,y,z)     _ltoa(x,y,z,1)
#define ultoa(x,y,z)	_ltoa(x,y,z,2)
#else
char *utoa(unsigned int n, char *s, int radix);
char *itoa(int n, char *s, int radix);
char *ultoa(unsigned long n, char *s, int radix);
char *ltoa(long n, char *s, int radix);
#endif /* end EiC */

#if defined(_SUNOS) && !defined(_EiC)
#define realloc(x,z)   ((x) ? realloc(x,z) : malloc(z))  
#endif

char * _itoa(int n,char *str, int radix, int mod);
char * _ltoa(int n,char *str, int radix, int mod);


#ifdef _EiC

void (*_exit_FuNcN[32])(void);
int _exit_FuNcS_n = 0;

void eic_exit(int);

int (atexit)(void f(void))
{
	printf("atexit called\n");
    _exit_FuNcS_n;
    if(_exit_FuNcS_n == 32)
	return 1;
    _exit_FuNcN[_exit_FuNcS_n++]  = f;
    return 0;
}

void (exit)(int status)
{
    int i;

    for(i=0;i<_exit_FuNcS_n;++i)
	(*_exit_FuNcN[i])();
    _exit_FuNcS_n = 0;
    eic_exit(status);
}	



/* sort and search functions that can't be builtin.
 * Functions copied from The Standard C Library
 * by P.J. Plauger
 */

#define _QS_BS_ 256

void (qsort)(void *_base, size_t _n, size_t _size,
	     int (*_cmp)(const void *, const void *))
{
#ifndef _MEMCPY_
#define _MEMCPY_
void *memcpy(void * _dst, const void * _src, size_t _n);
#endif
    
    while(_n>1) {
	size_t _i = 0;
	size_t _j = _n - 1;
	char *_qi = (char*)_base;
	char *_qj = _qi + _size * _j;
	char * _qp = _qj;

	while(_i<_j) {
	    while(_i < _j && (*_cmp) (_qi, _qp) <= 0)
		++_i, _qi += _size;
	    while(_i < _j && (*_cmp)(_qp, _qj) <= 0)
		--_j, _qj -= _size;
	    if(_i<_j) {
		char _buf[_QS_BS_];
		char * _q1 = _qi;
		char * _q2 = _qj;
		size_t _m, _ms;
		for(_ms = _size; 0 < _ms; _ms -= _m, _q1 += _m,
			_q2 -= _m) {
		    _m = _ms < _QS_BS_ ? _ms : _QS_BS_;
		    memcpy(_buf, _q1, _m);
		    memcpy(_q1, _q2, _m);
		    memcpy(_q2,_buf,_m);
		}
		++_i, _qi += _size;
	    }
	}
	if(_qi != _qp) {
	    char _buf[_QS_BS_];
	    char * _q1 = _qi;
	    char * _q2 = _qp;
	    size_t _m, _ms;
	    for(_ms = _size; 0 < _ms; _ms -= _m, _q1 += _m,
		    _q2 -= _m) {
		_m = _ms < _QS_BS_ ? _ms : _QS_BS_;
		memcpy(_buf, _q1, _m);
		memcpy(_q1, _q2, _m);
		memcpy(_q2,_buf,_m);

	    }
	}
	_j = _n - _i - 1, _qi += _size;
	if(_j < _i) {
	    if(1 < _j) 
		qsort(_qi,_j,_size,_cmp);
	    _n = _i;
	} else {
	    if(1 < _i)
		qsort(_base,_i,_size,_cmp);
	    _base = _qi;
	    _n = _j;
	}
    }
}

#undef _QS_BS_

void * (bsearch)(const void * _key, const void * _base,
		 size_t _nelem,
		 size_t _size,
		 int _cmp(const void * keyval, const void * datum))
{
    const char *_p = _base;
    size_t _n;
    for(_n=_nelem;_n>0;) {
	const size_t _pivot = _n >> 1;
	const char *const _q = _p + _size * _pivot;
	const int _val = _cmp(_key,_q);
	if(_val < 0) 
	    _n = _pivot;
	else if(_val == 0)
	    return (void*)_q;
	else {
	    _p = _q + _size;
	   _n -= _pivot + 1;
	}
    }
    return NULL;
}

	
#endif

#pragma pop_ptr

#endif /* _STDLIBH */








