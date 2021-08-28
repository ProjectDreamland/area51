#ifndef ERRNOH_
#define ERRNOH_

#pragma push_safeptr


#include <sys/errno.h>

#ifdef _EiC

extern int *_ErRnNo_;
#define errno   (*_ErRnNo_)
int * _get_errno(void);
_ErRnNo_ = _get_errno();

#else

extern int errno;

#endif



#pragma pop_ptr

#endif /* ERRNOH_ */
