#include <kernel.h>
#include <sifrpc.h>
#include <sif.h>

#include "ioptypes.h"
#include "iopcomms.h"
#include "iopaudio.h"
#include "iopmain.h"
#include "iopthreadpri.h"
#include "stdarg.h"
#include "stdio.h"

ModuleInfo Module = {"inev_iop_driver", 0xf101};

#ifdef X_DEBUG
#define TRACK_ALLOCATIONS
#endif

void    *rpc_arg;
static volatile void *ret;


typedef struct
{
    char *pName;
    s32  tid;
    struct ThreadInfo Status;
} thread_info;

thread_info s_iop_threads[MAX_IOP_THREADS];

s32 IopMemUsed=0;

#ifdef TRACK_ALLOCATIONS
char MEM_SIGNATURE[16] = "~~~MEM HEADER~~~";

typedef struct s_mem_header
{
    char                    Signature[16];
    struct s_mem_header*    pNext;
    char *                  pFunction;
    s32                     Line;
    s32                     Length;
} mem_header;

mem_header *s_MemoryList=NULL;
s32         s_AllocCount;

#endif

// in command.c
extern s32 iop_loop (void);
extern s32 iop_idle (void);
extern s32 iop_watchdog(void);

// Two thresholds are used to optimize the algorithm.  These values are 
// currently tuned for items having an average size of 48 bytes.

#define RECURSION_THRESH  4     //  Items needed to invoke PQS.
#define PARTITION_THRESH  6     //  Items needed to seek partition key.

// Anounce the PQS function.

static void PseudoQuickSort( byte*        pBase, 
                             byte*        pMax, 
                             s32          ItemSize,
                             s32          RecursionThresh, 
                             s32          PartitionThresh, 
                             compare_fn*  pCompare );

//------------------------------------------------------------------------------
void iop_DebugMsg(const char *format,...)
{
#ifdef X_DEBUG
	char buf[128];
	s32 timesec,timems;
	va_list anArgPtr;
	va_start(anArgPtr, format);
	
	timesec = iop_GetTime();
	timems = timesec % 10000;
	timesec = timesec / 10000;
	timems /= 100;
	if(format)
		vsprintf(buf, format, anArgPtr);

	Kprintf("%d.%02d: %s",timesec,timems, buf);
#else
	(void)format;
#endif

}

void iop_qsort( void*  apBase,     
              s32          NItems,    
              s32          ItemSize,  
              compare_fn*  pCompare )  
{
    byte  c;
    byte* i;
    byte* j;
    byte* lo;
    byte* hi;
    byte* min;
    byte* pMax;
    byte* pBase = (byte*)apBase;
    s32   RecursionThresh;          // Recursion threshold in bytes
    s32   PartitionThresh;          // Partition threshold in bytes

    ASSERT( pBase    );
    ASSERT( pCompare );
    ASSERT( NItems   > 0 );
    ASSERT( ItemSize > 0 );

    // Easy out?
    if( NItems <= 1 )
        return;

    // Set up some useful values.
    RecursionThresh = ItemSize * RECURSION_THRESH;
    PartitionThresh = ItemSize * PARTITION_THRESH;
    pMax            = pBase + (NItems * ItemSize);

    //
    // Set the 'hi' value.
    // Also, if there are enough values, call the PseudoQuickSort function.
    //
    if( NItems >= RECURSION_THRESH )
    {
        PseudoQuickSort( pBase, 
                         pMax, 
                         ItemSize, 
                         RecursionThresh, 
                         PartitionThresh, 
                         pCompare );
        hi = pBase + RecursionThresh;
    }
    else
    {
        hi = pMax;
    }

    //
    // Find the smallest element in the first "MIN(RECURSION_THRESH,NItems)"
    // items.  At this point, the smallest element in the entire list is 
    // guaranteed to be present in this sublist.
    //
    for( j = lo = pBase; (lo += ItemSize) < hi;  )
    {
        if( pCompare( j, lo ) > 0 )
            j = lo;
    }

    // 
    // Now put the smallest item in the first position to prime the next 
    // loop.
    //
    if( j != pBase )
    {
        for( i = pBase, hi = pBase + ItemSize; i < hi;  )
        {
            c    = *j;
            *j++ = *i;
            *i++ = c;
        }
    }

    //
    // Smallest item is in place.  Now we run the following hyper-fast
    // insertion sort.  For each remaining element, min, from [1] to [n-1],
    // set hi to the index of the element AFTER which this one goes.  Then,
    // do the standard insertion sort shift on a byte at a time basis.
    //
    for( min = pBase; (hi = min += ItemSize) < pMax;  )
    {
        while( pCompare( hi -= ItemSize, min ) > 0 )
        {
            // No body in this loop.
        }        

        if( (hi += ItemSize) != min )
        {
            for( lo = min + ItemSize; --lo >= min;  )
            {
                c = *lo;
                for( i = j = lo; (j -= ItemSize) >= hi; i = j )
                {
                    *i = *j;
                }
                *i = c;
            }
        }
    }
}

//------------------------------------------------------------------------------
//
//  What the hell is a PseudoQuickSort?  Simple!  Its "sorta quick sort".
//
//  Find a partition element and put it in the first position of the list.  The 
//  partition value is the median value of the first, middle, and last items.  
//  (Using the median value of these three items rather than just using the 
//  first item is a big win.)
//
//  Then, the usual partitioning and swapping, followed by swapping the 
//  partition element into place.
//
//  Then, of the two portions of the list which remain on either side of the 
//  partition, sort the smaller portion recursively, and sort the larger 
//  portion via another iteration of the same code.
//
//  Do not bother "quick sorting" lists (or sub lists) which have fewer than
//  RECURSION_THRESH items.  All final sorting is handled with an insertion sort
//  which is executed by the caller (x_qsort).  This is another huge win.  This 
//  means that this function does not actually completely sort the input list.  
//  It mostly sorts it.  That is, each item will be within RECURSION_THRESH 
//  positions of its correct position.  Thus, an insert sort will be able to 
//  finish off the sort process without a serious performance hit.
//
//  All data swaps are done in-line, which trades some code space for better
//  performance.  There are only three swap points, anyway.
//
//------------------------------------------------------------------------------

static 
void PseudoQuickSort( byte*        pBase,          
                      byte*        pMax,           
                      s32          ItemSize,        
                      s32          RecursionThresh,
                      s32          PartitionThresh,
                      compare_fn*  pCompare )
{
    byte* i;
    byte* j;
    byte* jj;
    byte* mid;
    byte* tmp;
    byte  c;
    s32   ii;
    s32   lo;
    s32   hi;

    lo = (s32)(pMax - pBase);   // Total data to sort in bytes.

    // Deep breath now...
    do
    {
        //
        // At this point, lo is the number of bytes in the items in the current
        // partition.  We want to find the median value item of the first, 
        // middle, and last items.  This median item will become the middle 
        // item.  Set j to the "greater of first and middle".  If last is larger
        // than j, then j is the median.  Otherwise, compare the last item to 
        // "the lesser of the first and middle" and take the larger.  The code 
        // is biased to prefer the middle over the first in the event of a tie.
        //

        mid = i = pBase + ItemSize * ((u32)(lo / ItemSize) >> 1);

        if( lo >= PartitionThresh )
        {
            j = (pCompare( (jj = pBase), i ) > 0)  ?  jj : i;

            if( pCompare( j, (tmp = pMax - ItemSize) ) > 0 )
            {
                // Use lesser of first and middle.  (First loser.)
                j = (j == jj ? i : jj);
                if( pCompare( j, tmp ) < 0 )
                    j = tmp;
            }

            if( j != i )
            {
                // Swap!
                ii = ItemSize;
                do
                {
                    c    = *i;
                    *i++ = *j;
                    *j++ = c;
                } while( --ii );
            }
        }
                
        //
        // Semi-standard quicksort partitioning / swapping...
        //

        for( i = pBase, j = pMax - ItemSize;  ;  )
        {
            while( (i < mid) && (pCompare( i, mid ) <= 0) )
            {
                i += ItemSize;
            }

            while( j > mid )
            {
                if( pCompare( mid, j ) <= 0 )
                {
                    j -= ItemSize;
                    continue;
                }

                tmp = i + ItemSize;     // Value of i after swap.

                if( i == mid )
                {
                    // j <-> mid, new mid is j.
                    mid = jj = j;       
                }
                else
                {
                    // i <-> j
                    jj = j;
                    j -= ItemSize;
                }

                goto SWAP;
            }

            if( i == mid )
            {
                break;
            }
            else
            {
                jj  = mid;
                tmp = mid = i;
                j  -= ItemSize;
            }
SWAP:
            ii = ItemSize;
            do
            {
                c     = *i;
                *i++  = *jj;
                *jj++ = c;
            } while( --ii );

            i = tmp;
        }

        //
        // Consider the sizes of the two partitions.  Process the smaller
        // partition first via recursion.  Then process the larger partition by
        // iterating through the above code again.  (Update variables as needed
        // to make it work.)
        //
        // NOTE:  Do not bother sorting a given partition, either recursively or
        // by another iteration, if the size of the partition is less than 
        // RECURSION_THRESH items.
        //

        j = mid;
        i = mid + ItemSize;

        if( (lo = (j-pBase)) <= (hi = (pMax-i)) )
        {
            if( lo >= RecursionThresh )
                PseudoQuickSort( pBase, j, 
                                 ItemSize,        
                                 RecursionThresh,
                                 PartitionThresh,
                                 pCompare );
            pBase = i;
            lo    = hi;
        }
        else
        {
            if( hi >= RecursionThresh )
                PseudoQuickSort( i, pMax,
                                 ItemSize,        
                                 RecursionThresh,
                                 PartitionThresh,
                                 pCompare );
            pMax = j;
        }

    } while( lo >= RecursionThresh );
}

//-----------------------------------------------------------------------------
s32 iop_CreateThread(void *EntryFunction,s32 priority,s32 StackSize, char *pName)
{
    struct ThreadParam param;
    s32 th;
    s32 index;


    for (index=0;index<MAX_IOP_THREADS;index++)
    {
        if (s_iop_threads[index].tid==0)
            break;
    }

    ASSERT(index < MAX_IOP_THREADS);

    s_iop_threads[index].pName = pName;

    param.attr         = TH_C;
    param.entry        = EntryFunction;
    param.initPriority = BASE_THREAD_PRIORITY-priority;
    param.stackSize    = StackSize;
    param.option       = 0;
    th = CreateThread (&param);
    s_iop_threads[index].tid=th;
    if (th > 0) 
    {
	    StartThread (th, 0);
    }
    else
    {
        return FALSE;
    }
    s_iop_threads[index].tid=th;
    return index;
}

//-----------------------------------------------------------------------------
void iop_DestroyThread(s32 ThreadId)
{
    TerminateThread(s_iop_threads[ThreadId].tid);
    DeleteThread(s_iop_threads[ThreadId].tid);
    s_iop_threads[ThreadId].tid = 0;
}

//-----------------------------------------------------------------------------
s32 iop_GetThreadId(void)
{
    s32 tid;
    s32 i;

    tid=GetThreadId();
    for (i=0;i<MAX_IOP_THREADS;i++)
    {
        if (s_iop_threads[i].tid==tid)
            return i;
    }
    ASSERT(FALSE);
    return 0;
}

//-----------------------------------------------------------------------------
s32 iop_CreateMutex(void)
{
    struct SemaParam sema;

    sema.initCount = 0;
    sema.maxCount  = 1;
    sema.attr      = AT_THFIFO;
    return (CreateSema (&sema));
}

//-----------------------------------------------------------------------------
void iop_DestroyMutex(s32 MutexId)
{
    DeleteSema(MutexId);
}

//-----------------------------------------------------------------------------
void iop_EnterMutex(s32 mutex)
{
    WaitSema(mutex);
}

//-----------------------------------------------------------------------------
void iop_ExitMutex(s32 mutex)
{
    SignalSema(mutex);
}

//-----------------------------------------------------------------------------
s32 iop_GetTime(void)
{
    struct SysClock sysclock;
    s32 sec;
    s32 usec;

    GetSystemTime(&sysclock);

    SysClock2USec(&sysclock,&sec,&usec);

    usec /= 100;
    // returns the system time (1 tick = 0.1ms).
    return sec*10000+usec;
}

void    *__iop_Malloc(s32 size,char *file,s32 line)
{
    void *pMem;
    s32 oldstate;

    (void)file;
    (void)line;

    CpuSuspendIntr(&oldstate);
#ifdef TRACK_ALLOCATIONS
    pMem = AllocSysMemory(1,size+sizeof(mem_header),NULL);
#else
    pMem = AllocSysMemory(1,size,NULL);
#endif

    if (pMem)
    {
#ifdef TRACK_ALLOCATIONS
        mem_header *pHeader;

        pHeader = (mem_header*)pMem;
        memcpy(pHeader->Signature,MEM_SIGNATURE,16);
        pHeader->pFunction  = file;
        pHeader->Line       = line;
        pHeader->Length     = size;
        pHeader->pNext      = s_MemoryList;      
        pMem = (void*)(pHeader+1);
        s_MemoryList    = pHeader;
        s_AllocCount++;

#endif
        IopMemUsed+=size;       
        memset(pMem,0,size);
    }
    else
    {
        iop_DebugMsg("Allocation failed attempting to allocate %d bytes\n",size);
        iop_DumpMemory();
        ASSERT(FALSE);
    }
    CpuResumeIntr(oldstate);

    return pMem;
}

void    __iop_Free(void *Base,char *file,s32 line)
{
    s32 oldstate;

    (void)file;
    (void)line;

    ASSERT(Base);
    CpuSuspendIntr(&oldstate);
#ifdef TRACK_ALLOCATIONS
    {
        mem_header *pHeader;
        mem_header *pLast;
        pHeader = s_MemoryList;
        pLast = NULL;
        while (pHeader)
        {
            ASSERT(memcmp(pHeader->Signature,MEM_SIGNATURE,16)==0);
            if (pHeader == (mem_header*)Base-1)
            {
                break;
            }
            pLast = pHeader;
            pHeader = pHeader->pNext;
        }
        ASSERT(pHeader);
        s_AllocCount--;
        if (pLast)
            pLast->pNext = pHeader->pNext;
        else
            s_MemoryList = pHeader->pNext;
        Base = (void*)pHeader;
    }
#endif
    FreeSysMemory(Base);
    CpuResumeIntr(oldstate);
}

void    iop_ValidateMemory(void)
{
#ifdef TRACK_ALLOCATIONS
    mem_header* pHeader;
    s32 count;

    pHeader = s_MemoryList;
    count = 0;
    while (pHeader)
    {
        ASSERT(memcmp(pHeader->Signature,MEM_SIGNATURE,16)==0);
        pHeader = pHeader->pNext;
        count++;
    }
    ASSERT(count==s_AllocCount);
#endif
}

void    __iop_DumpMemory( char* pFile, s32 Line )
{
#ifdef TRACK_ALLOCATIONS
    mem_header* pList;
    s32         Count;
    s32         Amount;

    pList = s_MemoryList;

    Count=0;
    Amount=0;
    iop_DebugMsg("======================= MEMORY DUMP START ===================\n");
    iop_DebugMsg("Dump requested by %s:%d\n",pFile,Line);
    iop_DebugMsg("Address    Length   Function\n");
    while (pList)
    {
        iop_DebugMsg("0x%06x %8d - %s:%d\n",pList+1,pList->Length,pList->pFunction,pList->Line);
        Count++;
        Amount+=pList->Length;
        pList = pList->pNext;
    }
    ASSERT(Count==s_AllocCount);
    iop_DebugMsg("Total of %d allocations using %d bytes\n",Count,Amount);
    iop_DebugMsg("System reports %d bytes free, largest=%d bytes\n",iop_MemFree(),iop_LargestFree());
    iop_DebugMsg("======================= MEMORY DUMP END =====================\n");
#endif
    (void)pFile;
    (void)Line;
}
s32     iop_MemFree(void)
{
    s32 memfree;

    memfree = QueryTotalFreeMemSize();
    return memfree;
}

s32     iop_LargestFree(void)
{
    s32 memfree;
    void *ptr;
    s32 jumpsize;

    memfree = QueryTotalFreeMemSize();
    jumpsize = 4096;

    while (jumpsize > 64)
    {
        ptr = AllocSysMemory(1,memfree,NULL);
        if (ptr)
        {
            FreeSysMemory(ptr);
            jumpsize /= 2;
            memfree += jumpsize;
        }
        else
        {
            memfree -= jumpsize;
        }
    }
    return memfree;
}
//-----------------------------------------------------------------------------
int start (int argc,char *argv[])
{
    iop_init Init;
    s32 status;

    iop_DebugMsg ("Inevitable Generic Iop driver\n");
    iop_DebugMsg("Compiled %s,%s\n",__DATE__,__TIME__);
    
    (void)argc;

#ifdef TRACK_ALLOCATIONS
    s_MemoryList = NULL;
    s_AllocCount = 0;
#endif
    memset(&s_iop_threads,0,sizeof(s_iop_threads));
    s_iop_threads[0].pName = "Inevitable Startup";
    s_iop_threads[0].tid=GetThreadId();
    memcpy(&Init,argv[1],sizeof(iop_init));

    if (! sceSifCheckInit ())
    {
    	sceSifInit ();
    }
    sceSifInitRpc (0);

    iop_DebugMsg("IOP Buffer size = %d bytes,%d Bytes free, %d Largest.\n",Init.IopBufferSize,iop_MemFree(),iop_LargestFree());
    rpc_arg = (void *)iop_Malloc(Init.IopBufferSize);
    ASSERT(rpc_arg);
#ifdef X_DEBUG
	iop_CreateThread(iop_watchdog,IOP_WATCHDOG_PRIORITY,2048,"IOP Watchdog");
#endif
    status = iop_CreateThread(iop_loop,IOP_DISP_PRIORITY,2048,"iop_loop (dispatch)");


    {
/*
        f32 Float;
	    s32 Time,Deltatimesec,Deltatimems;
        s32 i;
    BREAK;
        Float = 1.0f;
        for (i=0;i<1000;i++)
        {
        Float += (f32)iop_GetTime() / 1000.0f;
        }

	    // Add
        Time = iop_GetTime();

        for( i = 0; i < 1; i++ )
            Float += 0.001f;
    
        Deltatimesec = iop_GetTime();
        Deltatimesec = Deltatimesec - Time;
	    Deltatimems = Deltatimesec % 10000;
	    Deltatimesec = Deltatimesec / 10000;
	    Deltatimems /= 100;

	    iop_DebugMsg("Add %d.%02d: %s",Deltatimesec,Deltatimems);

	    // Sub
        Time = iop_GetTime();

        for( i = 0; i < 1000; i++ )
            Float -= 0.001f;
    
        Deltatimesec = iop_GetTime();
        Deltatimesec = Deltatimesec - Time;
	    Deltatimems = Deltatimesec % 10000;
	    Deltatimesec = Deltatimesec / 10000;
	    Deltatimems /= 100;

	    Kprintf("Sub %d.%02d: %s",Deltatimesec,Deltatimems);

	    // Mult
        Time = iop_GetTime();

        for( i = 0; i < 1000; i++ )
            Float *= 1.0f;
    
        Deltatimesec = iop_GetTime();
        Deltatimesec = Deltatimesec - Time;
	    Deltatimems = Deltatimesec % 10000;
	    Deltatimesec = Deltatimesec / 10000;
	    Deltatimems /= 100;

	    Kprintf("Mult %d.%02d: %s",Deltatimesec,Deltatimems);

	    // Divid
        Time = iop_GetTime();

        for( i = 0; i < 1000; i++ )
            Float /= 1.0f;
    
        Deltatimesec = iop_GetTime();
        Deltatimesec = Deltatimesec - Time;
	    Deltatimems = Deltatimesec % 10000;
	    Deltatimesec = Deltatimesec / 10000;
	    Deltatimems /= 100;

	    Kprintf("Divid %d.%02d: %s",Deltatimesec,Deltatimems);
*/
    }

    if (status) 
    {
#ifdef IOP_DEBUG
        iop_DebugMsg ("ineviop: Thread created. Loader finished.\n");
#endif
        return 0;
    }
    else
    {
        iop_DebugMsg("ineviop: Error occurred while creating thread\n");
	    return 1;
    }
}

s32 CurrentCommand;

//-----------------------------------------------------------------------------
void *iop_dispatch (u32 command, void *data, s32 size)
{ 
    CurrentCommand = command;
#if 0
    {
        static s32 countdown=50;
        countdown--;
        if (countdown==0)
        {
            *(u32 *)0x01 = 0xdeadbeef;
        }
    }
#endif
	iop_WatchdogReset();
    switch (command >> 16)
    {
    case 0: // Internal IOP command function
            switch (command)
            {
            case IOPCMD_FREEMEM:
                *(u32 *)data = iop_MemFree();
                ret = data;
                break;
            default:
                ret = data;
            }
        break;
    case 1: // Audio subsystem
        ret = audio_Dispatch(command,data,size);
        break;
    default:
        iop_DebugMsg("WARNING: Invalid IOP command code %08x\n",command);
    }
    CurrentCommand = -1;
    if (ret==0)
    {
        return &ret;
    }
    return (void*)ret;
}

//-----------------------------------------------------------------------------
s32 iop_loop(void)
{
#ifdef X_DEBUG
#endif
    sceSifQueueData QueueData;
    sceSifServeData ServerData;


    sceSifInitRpc (0);
    sceSifSetRpcQueue (&QueueData, GetThreadId ());
    sceSifRegisterRpc (&ServerData, INEV_IOP_DEV, iop_dispatch, rpc_arg, NULL, NULL, &QueueData);
    
    sceSifRpcLoop (&QueueData);

    return 0;
}

s32 idlecount;
s32 idlemax;
s32 cpu_utilization;

s32 iop_idle(void)
{
    s32 curtime,deltatime,count;

    idlemax=0;
    idlecount=0;
    curtime = iop_GetTime();
    count = 0;
    while (1)
    {
        count++;

        deltatime = iop_GetTime() - curtime;
        // Every 1/10th second, reset the idlecount
        if ( (deltatime > 1000) || (deltatime < 0) )
        {
            curtime = iop_GetTime();
            idlecount = count;
            if (count > idlemax)
            {
                idlemax = count;
            }
            count = 0;

            if (idlemax)
            {
                cpu_utilization = 100 - (idlecount * 100 / idlemax);
            }
            {
                s32 i;
                for (i=0;i<MAX_IOP_THREADS;i++)
                {
                    if (s_iop_threads[i].tid)
                        ReferThreadStatus(s_iop_threads[i].tid,&s_iop_threads[i].Status);
                }
            }
        }
    }
}

#ifdef X_DEBUG
s32 s_WatchdogTimer;

s32 iop_watchdog(void)
{
	s32 i;

	iop_WatchdogReset();

	while(1)
	{
		DelayThread(10000);		// Delay 10ms (timer based on usec)
		s_WatchdogTimer--;
		if (s_WatchdogTimer < 0)
		{
			s_WatchdogTimer = 1000;
			iop_DebugMsg("***** IOP THREAD STALL *****\n");
            for (i=0;i<MAX_IOP_THREADS;i++)
            {
                if (s_iop_threads[i].tid)
				{
                    ReferThreadStatus(s_iop_threads[i].tid,&s_iop_threads[i].Status);
					iop_DebugMsg("Thread id 0x%08x, name '%s', current pc = 0x%08x\n",
									s_iop_threads[i].tid,
									s_iop_threads[i].pName,
									s_iop_threads[i].Status.entry);
				}
            }
		}
	}
}

void iop_WatchdogReset(void)
{
	s_WatchdogTimer = 100;
}
#else
void iop_WatchdogReset(void)
{
}
#endif