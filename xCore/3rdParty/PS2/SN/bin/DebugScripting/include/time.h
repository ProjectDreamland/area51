#ifndef _TIMEH
#define _TIMEH

#pragma push_safeptr

#define _need_clock_t
#define _need_time_t
#define _need_size_t
#define _need_NULL

#include "sys/stdtypes.h"

#undef _need_clock_t
#undef _need_time_t
#undef _need_size_t
#undef _need_NULL

#include "sys/time.h"


#pragma pop_ptr

#endif /* _TIMEH */
