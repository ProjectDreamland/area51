#ifndef __THREADPRI_H
#define __THREADPRI_H

#define BASE_THREAD_PRIORITY 48

#define IOP_WATCHDOG_PRIORITY	32
#define IOP_DISP_PRIORITY   0           // Iop request dispatcher priority

#define NET_SEND_PRIORITY   2           // Iop network send update priority
#define NET_RECV_PRIORITY   1           // Iop network receive periodic update priority

#define AUD_UPDATE_PRIORITY 2          // Audio update thread priority
#define AUD_STREAM_PRIORITY 3           // Audio streaming thread priority

#endif