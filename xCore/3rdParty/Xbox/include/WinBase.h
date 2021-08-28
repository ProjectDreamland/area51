/************************************************************************
*                                                                       *
*   winbase.h -- This module defines the 32-Bit Windows Base APIs       *
*                                                                       *
*   Copyright (c) 1990-2001, Microsoft Corp. All rights reserved.       *
*                                                                       *
************************************************************************/
#ifndef _WINBASE_
#define _WINBASE_


//
// Define API decoration for direct importing of DLL references.
//


#define WINBASEAPI

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Compatibility macros
 */

#define GetCurrentTime()                GetTickCount()

#define Yield()

#define INVALID_HANDLE_VALUE ((HANDLE)-1)
#define INVALID_FILE_SIZE ((DWORD)0xFFFFFFFF)
#define INVALID_SET_FILE_POINTER ((DWORD)-1)

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

#define TIME_ZONE_ID_INVALID ((DWORD)0xFFFFFFFF)

#define WAIT_FAILED ((DWORD)0xFFFFFFFF)
#define WAIT_OBJECT_0       ((STATUS_WAIT_0 ) + 0 )

#define WAIT_ABANDONED         ((STATUS_ABANDONED_WAIT_0 ) + 0 )
#define WAIT_ABANDONED_0       ((STATUS_ABANDONED_WAIT_0 ) + 0 )

#define WAIT_IO_COMPLETION                  STATUS_USER_APC
#define STILL_ACTIVE                        STATUS_PENDING
#define EXCEPTION_ACCESS_VIOLATION          STATUS_ACCESS_VIOLATION
#define EXCEPTION_DATATYPE_MISALIGNMENT     STATUS_DATATYPE_MISALIGNMENT
#define EXCEPTION_BREAKPOINT                STATUS_BREAKPOINT
#define EXCEPTION_SINGLE_STEP               STATUS_SINGLE_STEP
#define EXCEPTION_ARRAY_BOUNDS_EXCEEDED     STATUS_ARRAY_BOUNDS_EXCEEDED
#define EXCEPTION_FLT_DENORMAL_OPERAND      STATUS_FLOAT_DENORMAL_OPERAND
#define EXCEPTION_FLT_DIVIDE_BY_ZERO        STATUS_FLOAT_DIVIDE_BY_ZERO
#define EXCEPTION_FLT_INEXACT_RESULT        STATUS_FLOAT_INEXACT_RESULT
#define EXCEPTION_FLT_INVALID_OPERATION     STATUS_FLOAT_INVALID_OPERATION
#define EXCEPTION_FLT_OVERFLOW              STATUS_FLOAT_OVERFLOW
#define EXCEPTION_FLT_STACK_CHECK           STATUS_FLOAT_STACK_CHECK
#define EXCEPTION_FLT_UNDERFLOW             STATUS_FLOAT_UNDERFLOW
#define EXCEPTION_INT_DIVIDE_BY_ZERO        STATUS_INTEGER_DIVIDE_BY_ZERO
#define EXCEPTION_INT_OVERFLOW              STATUS_INTEGER_OVERFLOW
#define EXCEPTION_PRIV_INSTRUCTION          STATUS_PRIVILEGED_INSTRUCTION
#define EXCEPTION_IN_PAGE_ERROR             STATUS_IN_PAGE_ERROR
#define EXCEPTION_ILLEGAL_INSTRUCTION       STATUS_ILLEGAL_INSTRUCTION
#define EXCEPTION_NONCONTINUABLE_EXCEPTION  STATUS_NONCONTINUABLE_EXCEPTION
#define EXCEPTION_STACK_OVERFLOW            STATUS_STACK_OVERFLOW
#define EXCEPTION_INVALID_DISPOSITION       STATUS_INVALID_DISPOSITION
#define EXCEPTION_GUARD_PAGE                STATUS_GUARD_PAGE_VIOLATION
#define EXCEPTION_INVALID_HANDLE            STATUS_INVALID_HANDLE
#define CONTROL_C_EXIT                      STATUS_CONTROL_C_EXIT
#define MoveMemory RtlMoveMemory
#define CopyMemory RtlCopyMemory
#define FillMemory RtlFillMemory
#define ZeroMemory RtlZeroMemory
#define HeapAlloc   RtlAllocateHeap
#define HeapReAlloc RtlReAllocateHeap
#define HeapSize    RtlSizeHeap
#define InitializeCriticalSection RtlInitializeCriticalSection
#define DeleteCriticalSection     RtlDeleteCriticalSection
#define EnterCriticalSection      RtlEnterCriticalSection
#define LeaveCriticalSection      RtlLeaveCriticalSection
#define TryEnterCriticalSection   RtlTryEnterCriticalSection
#if !defined(_NTOS_)
#define InterlockedCompareExchange _InterlockedCompareExchange
#define InterlockedDecrement       _InterlockedDecrement
#define InterlockedExchange        _InterlockedExchange
#define InterlockedExchangeAdd     _InterlockedExchangeAdd
#define InterlockedIncrement       _InterlockedIncrement
#endif // !defined(_NTOS_)

//
// File creation flags must start at the high end since they
// are combined with the attributes
//

#define FILE_FLAG_WRITE_THROUGH         0x80000000
#define FILE_FLAG_OVERLAPPED            0x40000000
#define FILE_FLAG_NO_BUFFERING          0x20000000
#define FILE_FLAG_RANDOM_ACCESS         0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS      0x02000000

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

//
// Define possible return codes from the CopyFileEx callback routine
//

#define PROGRESS_CONTINUE   0
#define PROGRESS_CANCEL     1
#define PROGRESS_QUIET      3

//
// Define CopyFileEx callback routine state change values
//

#define CALLBACK_CHUNK_FINISHED         0x00000000
#define CALLBACK_STREAM_SWITCH          0x00000001

//
// Define CopyFileEx option flags
//

#define COPY_FILE_FAIL_IF_EXISTS        0x00000001
#define COPY_FILE_OPEN_SOURCE_FOR_WRITE 0x00000004


//
//  File structures
//

typedef struct _OVERLAPPED {
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    DWORD   Offset;
    DWORD   OffsetHigh;
    HANDLE  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

typedef LPVOID PSECURITY_ATTRIBUTES;
typedef LPVOID LPSECURITY_ATTRIBUTES;

//
//  File System time stamps are represented with the following structure:
//

typedef struct _FILETIME {
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;

//
// System time is represented with the following structure:
//


typedef struct _SYSTEMTIME {
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;


typedef DWORD (WINAPI *PTHREAD_START_ROUTINE)(
    LPVOID lpThreadParameter
    );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

typedef VOID (WINAPI *PFIBER_START_ROUTINE)(
    LPVOID lpFiberParameter
    );
typedef PFIBER_START_ROUTINE LPFIBER_START_ROUTINE;

typedef RTL_CRITICAL_SECTION CRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION PCRITICAL_SECTION;
typedef PRTL_CRITICAL_SECTION LPCRITICAL_SECTION;


#define MUTEX_MODIFY_STATE MUTANT_QUERY_STATE
#define MUTEX_ALL_ACCESS MUTANT_ALL_ACCESS


/* Global Memory Flags */
#define GMEM_FIXED          0x0000
#define GMEM_MOVEABLE       0x0002
#define GMEM_NOCOMPACT      0x0010
#define GMEM_NODISCARD      0x0020
#define GMEM_ZEROINIT       0x0040
#define GMEM_MODIFY         0x0080
#define GMEM_DISCARDABLE    0x0100
#define GMEM_NOT_BANKED     0x1000
#define GMEM_SHARE          0x2000
#define GMEM_DDESHARE       0x2000
#define GMEM_NOTIFY         0x4000
#define GMEM_LOWER          GMEM_NOT_BANKED
#define GMEM_VALID_FLAGS    0x7F72
#define GMEM_INVALID_HANDLE 0x8000

#define GHND                (GMEM_MOVEABLE | GMEM_ZEROINIT)
#define GPTR                (GMEM_FIXED | GMEM_ZEROINIT)

#define GlobalDiscard( h )      GlobalReAlloc( (h), 0, GMEM_MOVEABLE )

/* Flags returned by GlobalFlags (in addition to GMEM_DISCARDABLE) */
#define GMEM_DISCARDED      0x4000
#define GMEM_LOCKCOUNT      0x00FF

typedef struct _MEMORYSTATUS {
    DWORD dwLength;
    DWORD dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
} MEMORYSTATUS, *LPMEMORYSTATUS;

/* Local Memory Flags */
#define LMEM_FIXED          0x0000
#define LMEM_MOVEABLE       0x0002
#define LMEM_NOCOMPACT      0x0010
#define LMEM_NODISCARD      0x0020
#define LMEM_ZEROINIT       0x0040
#define LMEM_MODIFY         0x0080
#define LMEM_DISCARDABLE    0x0F00
#define LMEM_VALID_FLAGS    0x0F72
#define LMEM_INVALID_HANDLE 0x8000

#define LHND                (LMEM_MOVEABLE | LMEM_ZEROINIT)
#define LPTR                (LMEM_FIXED | LMEM_ZEROINIT)

#define NONZEROLHND         (LMEM_MOVEABLE)
#define NONZEROLPTR         (LMEM_FIXED)

#define LocalDiscard( h )   LocalReAlloc( (h), 0, LMEM_MOVEABLE )

/* Flags returned by LocalFlags (in addition to LMEM_DISCARDABLE) */
#define LMEM_DISCARDED      0x4000
#define LMEM_LOCKCOUNT      0x00FF

//
// dwCreationFlag values
//


#define CREATE_SUSPENDED            0x00000004

#define THREAD_PRIORITY_LOWEST          THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL    (THREAD_PRIORITY_LOWEST+1)
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST-1)
#define THREAD_PRIORITY_ERROR_RETURN    (MAXLONG)

#define THREAD_PRIORITY_TIME_CRITICAL   THREAD_BASE_PRIORITY_LOWRT
#define THREAD_PRIORITY_IDLE            THREAD_BASE_PRIORITY_IDLE


#if !defined(MIDL_PASS)
typedef PCONTEXT LPCONTEXT;
typedef PEXCEPTION_RECORD LPEXCEPTION_RECORD;
typedef PEXCEPTION_POINTERS LPEXCEPTION_POINTERS;
#endif


#define IGNORE              0       // Ignore signal
#define INFINITE            0xFFFFFFFF  // Infinite timeout


#ifndef _NTOS_

WINBASEAPI
LONG
WINAPI
InterlockedIncrement(
    IN OUT LPLONG lpAddend
    );

WINBASEAPI
LONG
WINAPI
InterlockedDecrement(
    IN OUT LPLONG lpAddend
    );

WINBASEAPI
LONG
WINAPI
InterlockedExchange(
    IN OUT LPLONG Target,
    IN LONG Value
    );

#define InterlockedExchangePointer(Target, Value) \
    (PVOID)InterlockedExchange((PLONG)(Target), (LONG)(Value))

WINBASEAPI
LONG
WINAPI
InterlockedExchangeAdd(
    IN OUT LPLONG Addend,
    IN LONG Value
    );

WINBASEAPI
LONG
WINAPI
InterlockedCompareExchange (
    IN OUT LPLONG Destination,
    IN LONG Exchange,
    IN LONG Comperand
    );

#ifdef __cplusplus
// Use a function for C++ so X86 will generate the same errors as RISC.
__inline
PVOID
__cdecl
__InlineInterlockedCompareExchangePointer (
    IN OUT PVOID *Destination,
    IN PVOID ExChange,
    IN PVOID Comperand
    )
{
    return((PVOID)InterlockedCompareExchange((PLONG)Destination, (LONG)ExChange, (LONG)Comperand));
}
#define InterlockedCompareExchangePointer __InlineInterlockedCompareExchangePointer

#else

#define InterlockedCompareExchangePointer(Destination, ExChange, Comperand) \
    (PVOID)InterlockedCompareExchange((PLONG)(Destination), (LONG)(ExChange), (LONG)(Comperand))

#endif

#endif /* NT_INCLUDED */


WINBASEAPI
HGLOBAL
WINAPI
GlobalAlloc(
    IN UINT uFlags,
    IN SIZE_T dwBytes
    );

WINBASEAPI
HGLOBAL
WINAPI
GlobalReAlloc(
    IN HGLOBAL hMem,
    IN SIZE_T dwBytes,
    IN UINT uFlags
    );

WINBASEAPI
UINT
WINAPI
GlobalFlags(
    IN HGLOBAL hMem
    );

#define GlobalSize LocalSize
#define GlobalLock LocalLock
#define GlobalHandle LocalHandle
#define GlobalUnlock LocalUnlock
#define GlobalFree LocalFree


WINBASEAPI
VOID
WINAPI
GlobalMemoryStatus(
    IN OUT LPMEMORYSTATUS lpBuffer
    );


WINBASEAPI
HLOCAL
WINAPI
LocalAlloc(
    IN UINT uFlags,
    IN SIZE_T uBytes
    );

WINBASEAPI
HLOCAL
WINAPI
LocalReAlloc(
    IN HLOCAL hMem,
    IN SIZE_T uBytes,
    IN UINT uFlags
    );

WINBASEAPI
LPVOID
WINAPI
LocalLock(
    IN HLOCAL hMem
    );

WINBASEAPI
HLOCAL
WINAPI
LocalHandle(
    IN LPCVOID pMem
    );

WINBASEAPI
BOOL
WINAPI
LocalUnlock(
    IN HLOCAL hMem
    );

WINBASEAPI
SIZE_T
WINAPI
LocalSize(
    IN HLOCAL hMem
    );

WINBASEAPI
UINT
WINAPI
LocalFlags(
    IN HLOCAL hMem
    );

WINBASEAPI
HLOCAL
WINAPI
LocalFree(
    IN HLOCAL hMem
    );


WINBASEAPI
LPVOID
WINAPI
VirtualAlloc(
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD flAllocationType,
    IN DWORD flProtect
    );

WINBASEAPI
BOOL
WINAPI
VirtualFree(
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD dwFreeType
    );

WINBASEAPI
BOOL
WINAPI
VirtualProtect(
    IN  LPVOID lpAddress,
    IN  SIZE_T dwSize,
    IN  DWORD flNewProtect,
    OUT PDWORD lpflOldProtect
    );

WINBASEAPI
DWORD
WINAPI
VirtualQuery(
    IN LPCVOID lpAddress,
    OUT PMEMORY_BASIC_INFORMATION lpBuffer,
    IN DWORD dwLength
    );

WINBASEAPI
LPVOID
WINAPI
VirtualAllocEx(
    IN HANDLE hProcess,
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD flAllocationType,
    IN DWORD flProtect
    );

WINBASEAPI
BOOL
WINAPI
VirtualFreeEx(
    IN HANDLE hProcess,
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD dwFreeType
    );

WINBASEAPI
BOOL
WINAPI
VirtualProtectEx(
    IN  HANDLE hProcess,
    IN  LPVOID lpAddress,
    IN  SIZE_T dwSize,
    IN  DWORD flNewProtect,
    OUT PDWORD lpflOldProtect
    );

WINBASEAPI
DWORD
WINAPI
VirtualQueryEx(
    IN HANDLE hProcess,
    IN LPCVOID lpAddress,
    OUT PMEMORY_BASIC_INFORMATION lpBuffer,
    IN DWORD dwLength
    );

WINBASEAPI
HANDLE
WINAPI
HeapCreate(
    IN DWORD flOptions,
    IN SIZE_T dwInitialSize,
    IN SIZE_T dwMaximumSize
    );

WINBASEAPI
BOOL
WINAPI
HeapDestroy(
    IN OUT HANDLE hHeap
    );


#if !defined(_NTURTL_)

WINBASEAPI
LPVOID
WINAPI
HeapAlloc(
    IN HANDLE hHeap,
    IN DWORD dwFlags,
    IN SIZE_T dwBytes
    );

WINBASEAPI
LPVOID
WINAPI
HeapReAlloc(
    IN HANDLE hHeap,
    IN DWORD dwFlags,
    IN LPVOID lpMem,
    IN SIZE_T dwBytes
    );

WINBASEAPI
SIZE_T
WINAPI
HeapSize(
    IN HANDLE hHeap,
    IN DWORD dwFlags,
    IN LPCVOID lpMem
    );

#endif // !defined(_NTURTL_)

WINBASEAPI
BOOL
WINAPI
HeapFree(
    IN HANDLE hHeap,
    IN DWORD dwFlags,
    IN LPVOID lpMem
    );

WINBASEAPI
BOOL
WINAPI
HeapValidate(
    IN HANDLE hHeap,
    IN DWORD dwFlags,
    IN LPCVOID lpMem
    );

WINBASEAPI
SIZE_T
WINAPI
HeapCompact(
    IN HANDLE hHeap,
    IN DWORD dwFlags
    );

WINBASEAPI
HANDLE
WINAPI
GetProcessHeap( VOID );


typedef struct _PROCESS_HEAP_ENTRY {
    PVOID lpData;
    DWORD cbData;
    BYTE cbOverhead;
    BYTE iRegionIndex;
    WORD wFlags;
    union {
        struct {
            HANDLE hMem;
            DWORD dwReserved[ 3 ];
        } Block;
        struct {
            DWORD dwCommittedSize;
            DWORD dwUnCommittedSize;
            LPVOID lpFirstBlock;
            LPVOID lpLastBlock;
        } Region;
    };
} PROCESS_HEAP_ENTRY, *LPPROCESS_HEAP_ENTRY, *PPROCESS_HEAP_ENTRY;

#define PROCESS_HEAP_REGION             0x0001
#define PROCESS_HEAP_UNCOMMITTED_RANGE  0x0002
#define PROCESS_HEAP_ENTRY_BUSY         0x0004
#define PROCESS_HEAP_ENTRY_MOVEABLE     0x0010
#define PROCESS_HEAP_ENTRY_DDESHARE     0x0020

WINBASEAPI
BOOL
WINAPI
HeapLock(
    IN HANDLE hHeap
    );

WINBASEAPI
BOOL
WINAPI
HeapUnlock(
    IN HANDLE hHeap
    );


WINBASEAPI
BOOL
WINAPI
HeapWalk(
    IN HANDLE hHeap,
    IN OUT LPPROCESS_HEAP_ENTRY lpEntry
    );


#ifndef GetCurrentProcess
#define GetCurrentProcess() NtCurrentProcess()
#endif

WINBASEAPI
VOID
WINAPI
RaiseException(
    IN DWORD dwExceptionCode,
    IN DWORD dwExceptionFlags,
    IN DWORD nNumberOfArguments,
    IN CONST ULONG_PTR *lpArguments
    );

WINBASEAPI
LONG
WINAPI
UnhandledExceptionFilter(
    IN struct _EXCEPTION_POINTERS *ExceptionInfo
    );

typedef LONG (WINAPI *PTOP_LEVEL_EXCEPTION_FILTER)(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );
typedef PTOP_LEVEL_EXCEPTION_FILTER LPTOP_LEVEL_EXCEPTION_FILTER;

WINBASEAPI
LPTOP_LEVEL_EXCEPTION_FILTER
WINAPI
SetUnhandledExceptionFilter(
    IN LPTOP_LEVEL_EXCEPTION_FILTER lpTopLevelExceptionFilter
    );

extern __declspec(thread) PVOID XapiCurrentFiber;
#define GetCurrentFiber() XapiCurrentFiber
#define GetFiberData() (*(PVOID *)(GetCurrentFiber()))

WINBASEAPI
LPVOID
WINAPI
CreateFiber(
    IN DWORD dwStackSize,
    IN LPFIBER_START_ROUTINE lpStartAddress,
    IN LPVOID lpParameter
    );

WINBASEAPI
VOID
WINAPI
DeleteFiber(
    IN LPVOID lpFiber
    );

WINBASEAPI
LPVOID
WINAPI
ConvertThreadToFiber(
    IN LPVOID lpParameter
    );

WINBASEAPI
VOID
WINAPI
SwitchToFiber(
    IN LPVOID lpFiber
    );

WINBASEAPI
BOOL
WINAPI
SwitchToThread(
    VOID
    );

WINBASEAPI
HANDLE
WINAPI
CreateThread(
    IN LPSECURITY_ATTRIBUTES lpThreadAttributes,
    IN DWORD dwStackSize,
    IN LPTHREAD_START_ROUTINE lpStartAddress,
    IN LPVOID lpParameter,
    IN DWORD dwCreationFlags,
    OUT LPDWORD lpThreadId
    );


#ifndef GetCurrentThread
#define GetCurrentThread() NtCurrentThread()
#endif

WINBASEAPI
DWORD
WINAPI
GetCurrentThreadId(
    VOID
    );


WINBASEAPI
BOOL
WINAPI
SetThreadPriority(
    IN HANDLE hThread,
    IN int nPriority
    );

WINBASEAPI
BOOL
WINAPI
SetThreadPriorityBoost(
    IN HANDLE hThread,
    IN BOOL bDisablePriorityBoost
    );

WINBASEAPI
BOOL
WINAPI
GetThreadPriorityBoost(
    IN HANDLE hThread,
    OUT PBOOL pDisablePriorityBoost
    );

WINBASEAPI
int
WINAPI
GetThreadPriority(
    IN HANDLE hThread
    );

WINBASEAPI
BOOL
WINAPI
GetThreadTimes(
    IN HANDLE hThread,
    OUT LPFILETIME lpCreationTime,
    OUT LPFILETIME lpExitTime,
    OUT LPFILETIME lpKernelTime,
    OUT LPFILETIME lpUserTime
    );

WINBASEAPI
DECLSPEC_NORETURN
VOID
WINAPI
ExitThread(
    IN DWORD dwExitCode
    );


WINBASEAPI
BOOL
WINAPI
GetExitCodeThread(
    IN HANDLE hThread,
    OUT LPDWORD lpExitCode
    );

WINBASEAPI
DWORD
WINAPI
GetLastError(
    VOID
    );

WINBASEAPI
VOID
WINAPI
SetLastError(
    IN DWORD dwErrCode
    );

#define HasOverlappedIoCompleted(lpOverlapped) ((lpOverlapped)->Internal != STATUS_PENDING)

WINBASEAPI
BOOL
WINAPI
GetOverlappedResult(
    IN HANDLE hFile,
    IN LPOVERLAPPED lpOverlapped,
    OUT LPDWORD lpNumberOfBytesTransferred,
    IN BOOL bWait
    );

WINBASEAPI
HANDLE
WINAPI
CreateIoCompletionPort(
    IN HANDLE FileHandle,
    IN HANDLE ExistingCompletionPort,
    IN ULONG_PTR CompletionKey,
    IN DWORD NumberOfConcurrentThreads
    );

WINBASEAPI
BOOL
WINAPI
GetQueuedCompletionStatus(
    IN  HANDLE CompletionPort,
    OUT LPDWORD lpNumberOfBytesTransferred,
    OUT PULONG_PTR lpCompletionKey,
    OUT LPOVERLAPPED *lpOverlapped,
    IN  DWORD dwMilliseconds
    );

WINBASEAPI
BOOL
WINAPI
PostQueuedCompletionStatus(
    IN HANDLE CompletionPort,
    IN DWORD dwNumberOfBytesTransferred,
    IN ULONG_PTR dwCompletionKey,
    IN LPOVERLAPPED lpOverlapped
    );


WINBASEAPI
DWORD
WINAPI
SuspendThread(
    IN HANDLE hThread
    );

WINBASEAPI
DWORD
WINAPI
ResumeThread(
    IN HANDLE hThread
    );


typedef
VOID
(APIENTRY *PAPCFUNC)(
    ULONG_PTR dwParam
    );

WINBASEAPI
DWORD
WINAPI
QueueUserAPC(
    IN PAPCFUNC pfnAPC,
    IN HANDLE hThread,
    IN ULONG_PTR dwData
    );



WINBASEAPI
VOID
WINAPI
DebugBreak(
    VOID
    );


WINBASEAPI
BOOL
WINAPI
SetEvent(
    IN HANDLE hEvent
    );

WINBASEAPI
BOOL
WINAPI
ResetEvent(
    IN HANDLE hEvent
    );

WINBASEAPI
BOOL
WINAPI
PulseEvent(
    IN HANDLE hEvent
    );

WINBASEAPI
BOOL
WINAPI
ReleaseSemaphore(
    IN HANDLE hSemaphore,
    IN LONG lReleaseCount,
    OUT LPLONG lpPreviousCount
    );

WINBASEAPI
BOOL
WINAPI
ReleaseMutex(
    IN HANDLE hMutex
    );

WINBASEAPI
DWORD
WINAPI
WaitForSingleObject(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds
    );

WINBASEAPI
DWORD
WINAPI
WaitForMultipleObjects(
    IN DWORD nCount,
    IN CONST HANDLE *lpHandles,
    IN BOOL bWaitAll,
    IN DWORD dwMilliseconds
    );

WINBASEAPI
VOID
WINAPI
Sleep(
    IN DWORD dwMilliseconds
    );


typedef struct _BY_HANDLE_FILE_INFORMATION {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD dwVolumeSerialNumber;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD nNumberOfLinks;
    DWORD nFileIndexHigh;
    DWORD nFileIndexLow;
} BY_HANDLE_FILE_INFORMATION, *PBY_HANDLE_FILE_INFORMATION, *LPBY_HANDLE_FILE_INFORMATION;

WINBASEAPI
BOOL
WINAPI
GetFileInformationByHandle(
    IN HANDLE hFile,
    OUT LPBY_HANDLE_FILE_INFORMATION lpFileInformation
    );


WINBASEAPI
DWORD
WINAPI
GetFileSize(
    IN HANDLE hFile,
    OUT LPDWORD lpFileSizeHigh
    );

WINBASEAPI
BOOL
WINAPI
GetFileSizeEx(
    HANDLE hFile,
    PLARGE_INTEGER lpFileSize
    );

WINBASEAPI
BOOL
WINAPI
WriteFile(
    IN HANDLE hFile,
    IN LPCVOID lpBuffer,
    IN DWORD nNumberOfBytesToWrite,
    OUT LPDWORD lpNumberOfBytesWritten,
    IN LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
ReadFile(
    IN HANDLE hFile,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfBytesToRead,
    OUT LPDWORD lpNumberOfBytesRead,
    IN LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
FlushFileBuffers(
    IN HANDLE hFile
    );

WINBASEAPI
BOOL
WINAPI
DeviceIoControl(
    IN HANDLE hDevice,
    IN DWORD dwIoControlCode,
    IN LPVOID lpInBuffer,
    IN DWORD nInBufferSize,
    OUT LPVOID lpOutBuffer,
    IN DWORD nOutBufferSize,
    OUT LPDWORD lpBytesReturned,
    IN LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
SetEndOfFile(
    IN HANDLE hFile
    );

WINBASEAPI
DWORD
WINAPI
SetFilePointer(
    IN HANDLE hFile,
    IN LONG lDistanceToMove,
    IN PLONG lpDistanceToMoveHigh,
    IN DWORD dwMoveMethod
    );

WINBASEAPI
BOOL
WINAPI
SetFilePointerEx(
    HANDLE hFile,
    LARGE_INTEGER liDistanceToMove,
    PLARGE_INTEGER lpNewFilePointer,
    DWORD dwMoveMethod
    );

#define FindClose(hFindFile) CloseHandle(hFindFile)

WINBASEAPI
BOOL
WINAPI
GetFileTime(
    IN HANDLE hFile,
    OUT LPFILETIME lpCreationTime,
    OUT LPFILETIME lpLastAccessTime,
    OUT LPFILETIME lpLastWriteTime
    );

WINBASEAPI
BOOL
WINAPI
SetFileTime(
    IN HANDLE hFile,
    IN CONST FILETIME *lpCreationTime,
    IN CONST FILETIME *lpLastAccessTime,
    IN CONST FILETIME *lpLastWriteTime
    );

WINBASEAPI
BOOL
WINAPI
CloseHandle(
    IN OUT HANDLE hObject
    );

WINBASEAPI
BOOL
WINAPI
DuplicateHandle(
    IN HANDLE hSourceProcessHandle,
    IN HANDLE hSourceHandle,
    IN HANDLE hTargetProcessHandle,
    OUT LPHANDLE lpTargetHandle,
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN DWORD dwOptions
    );


WINBASEAPI
int
WINAPI
MulDiv(
    IN int nNumber,
    IN int nNumerator,
    IN int nDenominator
    );

WINBASEAPI
VOID
WINAPI
GetSystemTime(
    OUT LPSYSTEMTIME lpSystemTime
    );

WINBASEAPI
VOID
WINAPI
GetSystemTimeAsFileTime(
    OUT LPFILETIME lpSystemTimeAsFileTime
    );


WINBASEAPI
VOID
WINAPI
GetLocalTime(
    OUT LPSYSTEMTIME lpSystemTime
    );


typedef struct _TIME_ZONE_INFORMATION {
    LONG Bias;
    WCHAR StandardName[ 32 ];
    SYSTEMTIME StandardDate;
    LONG StandardBias;
    WCHAR DaylightName[ 32 ];
    SYSTEMTIME DaylightDate;
    LONG DaylightBias;
} TIME_ZONE_INFORMATION, *PTIME_ZONE_INFORMATION, *LPTIME_ZONE_INFORMATION;


WINBASEAPI
DWORD
WINAPI
GetTimeZoneInformation(
    OUT LPTIME_ZONE_INFORMATION lpTimeZoneInformation
    );


//
// Routines to convert back and forth between system time and file time
//

WINBASEAPI
BOOL
WINAPI
SystemTimeToFileTime(
    IN CONST SYSTEMTIME *lpSystemTime,
    OUT LPFILETIME lpFileTime
    );

WINBASEAPI
BOOL
WINAPI
FileTimeToLocalFileTime(
    IN CONST FILETIME *lpFileTime,
    OUT LPFILETIME lpLocalFileTime
    );

WINBASEAPI
BOOL
WINAPI
LocalFileTimeToFileTime(
    IN CONST FILETIME *lpLocalFileTime,
    OUT LPFILETIME lpFileTime
    );

WINBASEAPI
BOOL
WINAPI
FileTimeToSystemTime(
    IN CONST FILETIME *lpFileTime,
    OUT LPSYSTEMTIME lpSystemTime
    );

WINBASEAPI
LONG
WINAPI
CompareFileTime(
    IN CONST FILETIME *lpFileTime1,
    IN CONST FILETIME *lpFileTime2
    );


WINBASEAPI
DWORD
WINAPI
GetTickCount(
    VOID
    );


//
// _l Compat Functions
//

WINBASEAPI
int
WINAPI
lstrcmpA(
    IN LPCSTR lpString1,
    IN LPCSTR lpString2
    );
WINBASEAPI
int
WINAPI
lstrcmpW(
    IN LPCWSTR lpString1,
    IN LPCWSTR lpString2
    );
#ifdef UNICODE
#define lstrcmp  lstrcmpW
#else
#define lstrcmp  lstrcmpA
#endif // !UNICODE

WINBASEAPI
int
WINAPI
lstrcmpiA(
    IN LPCSTR lpString1,
    IN LPCSTR lpString2
    );
WINBASEAPI
int
WINAPI
lstrcmpiW(
    IN LPCWSTR lpString1,
    IN LPCWSTR lpString2
    );
#ifdef UNICODE
#define lstrcmpi  lstrcmpiW
#else
#define lstrcmpi  lstrcmpiA
#endif // !UNICODE

WINBASEAPI
LPSTR
WINAPI
lstrcpynA(
    OUT LPSTR lpString1,
    IN LPCSTR lpString2,
    IN int iMaxLength
    );
WINBASEAPI
LPWSTR
WINAPI
lstrcpynW(
    OUT LPWSTR lpString1,
    IN LPCWSTR lpString2,
    IN int iMaxLength
    );
#ifdef UNICODE
#define lstrcpyn  lstrcpynW
#else
#define lstrcpyn  lstrcpynA
#endif // !UNICODE

WINBASEAPI
LPSTR
WINAPI
lstrcpyA(
    OUT LPSTR lpString1,
    IN LPCSTR lpString2
    );
WINBASEAPI
LPWSTR
WINAPI
lstrcpyW(
    OUT LPWSTR lpString1,
    IN LPCWSTR lpString2
    );
#ifdef UNICODE
#define lstrcpy  lstrcpyW
#else
#define lstrcpy  lstrcpyA
#endif // !UNICODE

WINBASEAPI
LPSTR
WINAPI
lstrcatA(
    IN OUT LPSTR lpString1,
    IN LPCSTR lpString2
    );
WINBASEAPI
LPWSTR
WINAPI
lstrcatW(
    IN OUT LPWSTR lpString1,
    IN LPCWSTR lpString2
    );
#ifdef UNICODE
#define lstrcat  lstrcatW
#else
#define lstrcat  lstrcatA
#endif // !UNICODE

WINBASEAPI
int
WINAPI
lstrlenA(
    IN LPCSTR lpString
    );
WINBASEAPI
int
WINAPI
lstrlenW(
    IN LPCWSTR lpString
    );
#ifdef UNICODE
#define lstrlen  lstrlenW
#else
#define lstrlen  lstrlenA
#endif // !UNICODE


WINBASEAPI
DWORD
WINAPI
TlsAlloc(
    VOID
    );

#define TLS_OUT_OF_INDEXES (DWORD)0xFFFFFFFF

WINBASEAPI
LPVOID
WINAPI
TlsGetValue(
    IN DWORD dwTlsIndex
    );

WINBASEAPI
BOOL
WINAPI
TlsSetValue(
    IN DWORD dwTlsIndex,
    IN LPVOID lpTlsValue
    );

WINBASEAPI
BOOL
WINAPI
TlsFree(
    IN DWORD dwTlsIndex
    );

typedef
VOID
(WINAPI *LPOVERLAPPED_COMPLETION_ROUTINE)(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
DWORD
WINAPI
SleepEx(
    IN DWORD dwMilliseconds,
    IN BOOL bAlertable
    );

WINBASEAPI
DWORD
WINAPI
WaitForSingleObjectEx(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds,
    IN BOOL bAlertable
    );

WINBASEAPI
DWORD
WINAPI
WaitForMultipleObjectsEx(
    IN DWORD nCount,
    IN CONST HANDLE *lpHandles,
    IN BOOL bWaitAll,
    IN DWORD dwMilliseconds,
    IN BOOL bAlertable
    );

WINBASEAPI
DWORD
WINAPI
SignalObjectAndWait(
    IN HANDLE hObjectToSignal,
    IN HANDLE hObjectToWaitOn,
    IN DWORD dwMilliseconds,
    IN BOOL bAlertable
    );

WINBASEAPI
BOOL
WINAPI
ReadFileEx(
    IN HANDLE hFile,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfBytesToRead,
    IN LPOVERLAPPED lpOverlapped,
    IN LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

WINBASEAPI
BOOL
WINAPI
WriteFileEx(
    IN HANDLE hFile,
    IN LPCVOID lpBuffer,
    IN DWORD nNumberOfBytesToWrite,
    IN LPOVERLAPPED lpOverlapped,
    IN LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

WINBASEAPI
BOOL
WINAPI
ReadFileScatter(
    IN HANDLE hFile,
    IN FILE_SEGMENT_ELEMENT aSegmentArray[],
    IN DWORD nNumberOfBytesToRead,
    OUT LPDWORD lpNumberOfBytesRead,
    IN LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL
WINAPI
WriteFileGather(
    IN HANDLE hFile,
    OUT FILE_SEGMENT_ELEMENT aSegmentArray[],
    IN DWORD nNumberOfBytesToWrite,
    OUT LPDWORD lpNumberOfBytesWritten,
    IN LPOVERLAPPED lpOverlapped
    );

//
// Dual Mode API below this line. Dual Mode Structures also included.
//

typedef struct _WIN32_FIND_DATAA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
    DWORD dwReserved0;
    DWORD dwReserved1;
    CHAR cFileName[ MAX_PATH ];
    CHAR cAlternateFileName[ 14 ];
} WIN32_FIND_DATAA, *PWIN32_FIND_DATAA, *LPWIN32_FIND_DATAA;
typedef WIN32_FIND_DATAA WIN32_FIND_DATA;
typedef PWIN32_FIND_DATAA PWIN32_FIND_DATA;
typedef LPWIN32_FIND_DATAA LPWIN32_FIND_DATA;

typedef struct _WIN32_FILE_ATTRIBUTE_DATA {
    DWORD dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD nFileSizeHigh;
    DWORD nFileSizeLow;
} WIN32_FILE_ATTRIBUTE_DATA, *LPWIN32_FILE_ATTRIBUTE_DATA;

WINBASEAPI
HANDLE
WINAPI
CreateMutexA(
    IN LPSECURITY_ATTRIBUTES lpMutexAttributes,
    IN BOOL bInitialOwner,
    IN LPCSTR lpName
    );
#define CreateMutex CreateMutexA

WINBASEAPI
HANDLE
WINAPI
OpenMutexA(
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN LPCSTR lpName
    );
#define OpenMutex OpenMutexA

WINBASEAPI
HANDLE
WINAPI
CreateEventA(
    IN LPSECURITY_ATTRIBUTES lpEventAttributes,
    IN BOOL bManualReset,
    IN BOOL bInitialState,
    IN LPCSTR lpName
    );
#define CreateEvent CreateEventA

WINBASEAPI
HANDLE
WINAPI
OpenEventA(
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN LPCSTR lpName
    );
#define OpenEvent OpenEventA

WINBASEAPI
HANDLE
WINAPI
CreateSemaphoreA(
    IN LPSECURITY_ATTRIBUTES lpSemaphoreAttributes,
    IN LONG lInitialCount,
    IN LONG lMaximumCount,
    IN LPCSTR lpName
    );
#define CreateSemaphore CreateSemaphoreA

WINBASEAPI
HANDLE
WINAPI
OpenSemaphoreA(
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN LPCSTR lpName
    );
#define OpenSemaphore OpenSemaphoreA

typedef
VOID
(APIENTRY *PTIMERAPCROUTINE)(
    LPVOID lpArgToCompletionRoutine,
    DWORD dwTimerLowValue,
    DWORD dwTimerHighValue
    );

WINBASEAPI
HANDLE
WINAPI
CreateWaitableTimerA(
    IN LPSECURITY_ATTRIBUTES lpTimerAttributes,
    IN BOOL bManualReset,
    IN LPCSTR lpTimerName
    );
#define CreateWaitableTimer CreateWaitableTimerA

WINBASEAPI
HANDLE
WINAPI
OpenWaitableTimerA(
    IN DWORD dwDesiredAccess,
    IN BOOL bInheritHandle,
    IN LPCSTR lpTimerName
    );
#define OpenWaitableTimer OpenWaitableTimerA

WINBASEAPI
BOOL
WINAPI
SetWaitableTimer(
    IN HANDLE hTimer,
    IN const LARGE_INTEGER *lpDueTime,
    IN LONG lPeriod,
    IN PTIMERAPCROUTINE pfnCompletionRoutine,
    IN LPVOID lpArgToCompletionRoutine,
    IN BOOL fResume
    );

WINBASEAPI
BOOL
WINAPI
CancelWaitableTimer(
    IN HANDLE hTimer
    );


WINBASEAPI
VOID
WINAPI
OutputDebugStringA(
    IN LPCSTR lpOutputString
    );
WINBASEAPI
VOID
WINAPI
OutputDebugStringW(
    IN LPCWSTR lpOutputString
    );
#ifdef UNICODE
#define OutputDebugString  OutputDebugStringW
#else
#define OutputDebugString  OutputDebugStringA
#endif // !UNICODE


WINBASEAPI
BOOL
WINAPI
GetDiskFreeSpaceExA(
    IN LPCSTR lpDirectoryName,
    OUT PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    OUT PULARGE_INTEGER lpTotalNumberOfBytes,
    OUT PULARGE_INTEGER lpTotalNumberOfFreeBytes
    );
#define GetDiskFreeSpaceEx GetDiskFreeSpaceExA

WINBASEAPI
BOOL
WINAPI
CreateDirectoryA(
    IN LPCSTR lpPathName,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );
#define CreateDirectory CreateDirectoryA


WINBASEAPI
BOOL
WINAPI
RemoveDirectoryA(
    IN LPCSTR lpPathName
    );
#define RemoveDirectory RemoveDirectoryA


WINBASEAPI
HANDLE
WINAPI
CreateFileA(
    IN LPCSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile
    );
#define CreateFile CreateFileA

WINBASEAPI
BOOL
WINAPI
SetFileAttributesA(
    IN LPCSTR lpFileName,
    IN DWORD dwFileAttributes
    );
#define SetFileAttributes SetFileAttributesA

WINBASEAPI
DWORD
WINAPI
GetFileAttributesA(
    IN LPCSTR lpFileName
    );
#define GetFileAttributes GetFileAttributesA

typedef enum _GET_FILEEX_INFO_LEVELS {
    GetFileExInfoStandard,
    GetFileExMaxInfoLevel
} GET_FILEEX_INFO_LEVELS;

WINBASEAPI
BOOL
WINAPI
GetFileAttributesExA(
    IN LPCSTR lpFileName,
    IN GET_FILEEX_INFO_LEVELS fInfoLevelId,
    OUT LPVOID lpFileInformation
    );
#define GetFileAttributesEx GetFileAttributesExA

WINBASEAPI
BOOL
WINAPI
DeleteFileA(
    IN LPCSTR lpFileName
    );
#define DeleteFile DeleteFileA

WINBASEAPI
HANDLE
WINAPI
FindFirstFileA(
    IN LPCSTR lpFileName,
    OUT LPWIN32_FIND_DATA lpFindFileData
    );
#define FindFirstFile FindFirstFileA

WINBASEAPI
BOOL
WINAPI
FindNextFileA(
    IN HANDLE hFindFile,
    OUT LPWIN32_FIND_DATAA lpFindFileData
    );
#define FindNextFile FindNextFileA


WINBASEAPI
BOOL
WINAPI
CopyFileA(
    IN LPCSTR lpExistingFileName,
    IN LPCSTR lpNewFileName,
    IN BOOL bFailIfExists
    );
#define CopyFile CopyFileA

typedef
DWORD
(WINAPI *LPPROGRESS_ROUTINE)(
    LARGE_INTEGER TotalFileSize,
    LARGE_INTEGER TotalBytesTransferred,
    LARGE_INTEGER StreamSize,
    LARGE_INTEGER StreamBytesTransferred,
    DWORD dwStreamNumber,
    DWORD dwCallbackReason,
    HANDLE hSourceFile,
    HANDLE hDestinationFile,
    LPVOID lpData OPTIONAL
    );

WINBASEAPI
BOOL
WINAPI
CopyFileExA(
    IN LPCSTR lpExistingFileName,
    IN LPCSTR lpNewFileName,
    IN LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    IN LPVOID lpData OPTIONAL,
    IN LPBOOL pbCancel OPTIONAL,
    IN DWORD dwCopyFlags
    );
#define CopyFileEx CopyFileExA

WINBASEAPI
BOOL
WINAPI
MoveFileA(
    IN LPCSTR lpExistingFileName,
    IN LPCSTR lpNewFileName
    );
#define MoveFile MoveFileA

WINBASEAPI
BOOL
WINAPI
MoveFileExA(
    IN LPCSTR lpExistingFileName,
    IN LPCSTR lpNewFileName,
    IN DWORD dwFlags
    );
#define MoveFileEx MoveFileExA

WINBASEAPI
BOOL
WINAPI
MoveFileWithProgressA(
    IN LPCSTR lpExistingFileName,
    IN LPCSTR lpNewFileName,
    IN LPPROGRESS_ROUTINE lpProgressRoutine OPTIONAL,
    IN LPVOID lpData OPTIONAL,
    IN DWORD dwFlags
    );
#define MoveFileWithProgress MoveFileWithProgressA

#define MOVEFILE_REPLACE_EXISTING       0x00000001
#define MOVEFILE_COPY_ALLOWED           0x00000002
#define MOVEFILE_WRITE_THROUGH          0x00000008


WINBASEAPI
BOOL
WINAPI
GetVolumeInformationA(
    IN LPCSTR lpRootPathName,
    OUT LPSTR lpVolumeNameBuffer,
    IN DWORD nVolumeNameSize,
    OUT LPDWORD lpVolumeSerialNumber,
    OUT LPDWORD lpMaximumComponentLength,
    OUT LPDWORD lpFileSystemFlags,
    OUT LPSTR lpFileSystemNameBuffer,
    IN DWORD nFileSystemNameSize
    );
#define GetVolumeInformation GetVolumeInformationA

WINBASEAPI
BOOL
WINAPI
CancelIo(
    IN HANDLE hFile
    );


WINBASEAPI
BOOL
WINAPI
IsBadReadPtr(
    IN CONST VOID *lp,
    IN UINT_PTR ucb
    );

WINBASEAPI
BOOL
WINAPI
IsBadWritePtr(
    IN LPVOID lp,
    IN UINT_PTR ucb
    );


WINBASEAPI
BOOL
WINAPI
IsBadCodePtr(
    IN FARPROC lpfn
    );

WINBASEAPI
BOOL
WINAPI
IsBadStringPtrA(
    IN LPCSTR lpsz,
    IN UINT_PTR ucchMax
    );
WINBASEAPI
BOOL
WINAPI
IsBadStringPtrW(
    IN LPCWSTR lpsz,
    IN UINT_PTR ucchMax
    );
#ifdef UNICODE
#define IsBadStringPtr  IsBadStringPtrW
#else
#define IsBadStringPtr  IsBadStringPtrA
#endif // !UNICODE


//
// Performance counter API's
//

WINBASEAPI
BOOL
WINAPI
QueryPerformanceCounter(
    OUT LARGE_INTEGER *lpPerformanceCount
    );

WINBASEAPI
BOOL
WINAPI
QueryPerformanceFrequency(
    OUT LARGE_INTEGER *lpFrequency
    );

// DOS and OS/2 Compatible Error Code definitions returned by the Win32 Base
// API functions.
//

#include <winerror.h>

/* Abnormal termination codes */

#define TC_NORMAL       0
#define TC_HARDERR      1
#define TC_GP_TRAP      2
#define TC_SIGNAL       3


WINBASEAPI
int
WINAPI
wvsprintfA(
    OUT LPSTR,
    IN LPCSTR,
    IN va_list arglist);
WINBASEAPI
int
WINAPI
wvsprintfW(
    OUT LPWSTR,
    IN LPCWSTR,
    IN va_list arglist);
#ifdef UNICODE
#define wvsprintf  wvsprintfW
#else
#define wvsprintf  wvsprintfA
#endif // !UNICODE

WINBASEAPI
int
WINAPIV
wsprintfA(
    OUT LPSTR,
    IN LPCSTR,
    ...);
WINBASEAPI
int
WINAPIV
wsprintfW(
    OUT LPWSTR,
    IN LPCWSTR,
    ...);
#ifdef UNICODE
#define wsprintf  wsprintfW
#else
#define wsprintf  wsprintfA
#endif // !UNICODE

#define CP_ACP                    0           // default to ANSI code page
#define CP_UTF8                   65001       // UTF-8 translation

WINBASEAPI
int
WINAPI
MultiByteToWideChar(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    OUT LPWSTR  lpWideCharStr,
    IN int      cchWideChar);

WINBASEAPI
int
WINAPI
WideCharToMultiByte(
    IN UINT     CodePage,
    IN DWORD    dwFlags,
    IN LPCWSTR  lpWideCharStr,
    IN int      cchWideChar,
    OUT LPSTR   lpMultiByteStr,
    IN int      cbMultiByte,
    IN LPCSTR   lpDefaultChar,
    OUT LPBOOL  lpUsedDefaultChar);


WINBASEAPI
LPSTR
WINAPI
CharUpperA(
    IN OUT LPSTR lpsz);
WINBASEAPI
LPWSTR
WINAPI
CharUpperW(
    IN OUT LPWSTR lpsz);
#ifdef UNICODE
#define CharUpper  CharUpperW
#else
#define CharUpper  CharUpperA
#endif // !UNICODE

WINBASEAPI
LPSTR
WINAPI
CharLowerA(
    IN OUT LPSTR lpsz);
WINBASEAPI
LPWSTR
WINAPI
CharLowerW(
    IN OUT LPWSTR lpsz);
#ifdef UNICODE
#define CharLower  CharLowerW
#else
#define CharLower  CharLowerA
#endif // !UNICODE

WINBASEAPI
BOOL
WINAPI
SetRect(
    OUT LPRECT lprc,
    IN int xLeft,
    IN int yTop,
    IN int xRight,
    IN int yBottom);

WINBASEAPI
BOOL
WINAPI
SetRectEmpty(
    OUT LPRECT lprc);

WINBASEAPI
BOOL
WINAPI
CopyRect(
    OUT LPRECT lprcDst,
    IN CONST RECT *lprcSrc);

WINBASEAPI
BOOL
WINAPI
InflateRect(
    IN OUT LPRECT lprc,
    IN int dx,
    IN int dy);

WINBASEAPI
BOOL
WINAPI
IntersectRect(
    OUT LPRECT lprcDst,
    IN CONST RECT *lprcSrc1,
    IN CONST RECT *lprcSrc2);

WINBASEAPI
BOOL
WINAPI
UnionRect(
    OUT LPRECT lprcDst,
    IN CONST RECT *lprcSrc1,
    IN CONST RECT *lprcSrc2);

WINBASEAPI
BOOL
WINAPI
SubtractRect(
    OUT LPRECT lprcDst,
    IN CONST RECT *lprcSrc1,
    IN CONST RECT *lprcSrc2);

WINBASEAPI
BOOL
WINAPI
OffsetRect(
    IN OUT LPRECT lprc,
    IN int dx,
    IN int dy);

WINBASEAPI
BOOL
WINAPI
IsRectEmpty(
    IN CONST RECT *lprc);

WINBASEAPI
BOOL
WINAPI
EqualRect(
    IN CONST RECT *lprc1,
    IN CONST RECT *lprc2);

WINBASEAPI
BOOL
WINAPI
PtInRect(
    IN CONST RECT *lprc,
    IN POINT pt);


#ifdef __cplusplus
}
#endif


#endif // _WINBASE_

