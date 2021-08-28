#ifndef __IOPMAIN_H
#define __IOPMAIN_H

#include "ioptypes.h"

//#define ENABLE_USB_DEVICES

#define MAX_IOP_THREADS 10
enum {
    IOPCMD_BASE = 0x00000000,
    IOPCMD_INIT,
    IOPCMD_KILL,
    IOPCMD_UPDATE,
    IOPCMD_FREEMEM,
};

s32     iop_CreateThread(void *EntryFunction,s32 priority,s32 StackSize, char *pName);
void    iop_DestroyThread(s32 ThreadId);
s32     iop_CreateMutex(void);
void    iop_DestroyMutex(s32 MutexId);
void    iop_EnterMutex(s32 mutex);
void    iop_ExitMutex(s32 mutex);
void    iop_ValidateMemory(void);
void	iop_WatchdogReset(void);

#define iop_DumpMemory() __iop_DumpMemory(__FUNCTION__,__LINE__)
void    __iop_DumpMemory( char* pFile,s32 Line );

s32     iop_GetThreadId(void);

void    *__iop_Malloc(s32 size,char *file,s32 line);

void    __iop_Free(void *Base,char *file,s32 line);
s32     iop_MemFree(void);
s32     iop_LargestFree(void);

void	iop_DebugMsg(const char *format,...);

#define iop_Malloc(size) __iop_Malloc(size,__FUNCTION__,__LINE__)
#define iop_Free(base) __iop_Free(base,__FUNCTION__,__LINE__);

typedef s32 compare_fn( void* pItem1, void* pItem2 );

void    iop_qsort( void*  apBase,     
              s32          NItems,    
              s32          ItemSize,  
              compare_fn*  pCompare )  ;

// returns the system time (1 tick = 0.1ms).
s32     iop_GetTime(void);

#endif
