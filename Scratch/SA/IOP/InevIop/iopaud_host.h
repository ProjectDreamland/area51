#ifndef __IOPAUD_HOST_H
#define __IOPAUD_HOST_H
//
// List of structs & defines common to the host and client side of the
// communication protocol.


#ifdef TARGET_PS2_IOP
#include "ioptypes.h"
#else
#include "x_types.hpp"
#endif

#include "..\..\support\audiomgr\audiodefs.hpp"
#define MAX_OUT_BUFFER_SIZE     (160*sizeof(u32))

#endif
