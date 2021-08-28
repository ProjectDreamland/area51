#ifndef _STDARGH
#define _STDARGH

#pragma push_safeptr


typedef char *va_list;

int _get_AR_t_size(void);
va_list _StArT_Va(void *);
int _AR_t_SiZe;


#define va_arg(x,y)     (*(y*)(x -= _AR_t_SiZe))
#define va_start(x,y)   (x)=_StArT_Va(&(y))
#define va_end(x)   

_AR_t_SiZe = _get_AR_t_size();

#pragma pop_ptr

#endif

