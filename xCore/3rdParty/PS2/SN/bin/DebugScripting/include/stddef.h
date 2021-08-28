#ifndef STDDEFH_
#define STDDEFH_

#pragma push_safeptr

#define _need_NULL
#define _need_ptrdiff_t
#define _need_size_t
#define _need_wchar_t

#include "sys/stdtypes.h"

#define offsetof(type, mem)   ((size_t)(&((type*)0)->mem)) 

#undef _need_ptrdiff_t
#undef _need_size_t
#undef _need_wchar_t
#undef _need_NULL

#pragma pop_ptr

#endif /* _STDDEFH */
