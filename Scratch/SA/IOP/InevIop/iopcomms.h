#ifndef __IOPCOMMS_HPP
#define __IOPCOMMS_HPP

#define IOP_BUFFER_SIZE 2048             // Buffer size in bytes

#define INEV_IOP_DEV  0xbeef0001      // Our device #

typedef struct s_iop_init
{
    s32 IopBufferSize;
} iop_init;

#endif // __IOPCOMMS_HPP