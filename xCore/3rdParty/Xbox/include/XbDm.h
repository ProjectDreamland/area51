//
//
// Xbox debug interface functions
// Copyright Microsoft Corporation 2000 - 2001. All Rights Reserved.
//
//
#ifndef _XBDM_H
#define _XBDM_H


#ifdef __cplusplus
extern "C" {
#endif

#ifdef XBDBGS
#define DMAPI
#endif

#ifndef DMAPI
#ifdef _XBDM_
#define DMAPI
#else
#define DMAPI __declspec(dllimport)
#endif
#endif

#define DMHRAPI DMAPI HRESULT __stdcall

// Notification types
#define DM_NONE 0
#define DM_BREAK 1
#define DM_DEBUGSTR 2
#define DM_EXEC 3
#define DM_SINGLESTEP 4
#define DM_MODLOAD 5
#define DM_MODUNLOAD 6
#define DM_CREATETHREAD 7
#define DM_DESTROYTHREAD 8
#define DM_EXCEPTION 9
#define DM_CLOCKINT 10
#define DM_ASSERT 12
#define DM_DATABREAK 13
#define DM_RIP 14
#define DM_THREADSWITCH 15
#define DM_SECTIONLOAD 16
#define DM_SECTIONUNLOAD 17
#define DM_FIBER 18
#define DM_STACKTRACE 19
#define DM_BUGCHECK 20
#define DM_NOTIFYMAX 20

#define DM_NOTIFICATIONMASK 0xffffff
#define DM_STOPTHREAD 0x80000000

typedef DWORD (__stdcall *PDM_NOTIFY_FUNCTION)(ULONG dwNotification, DWORD dwParam);

// Break notification structure
typedef struct _DMN_BREAK {
    PVOID Address;
    DWORD ThreadId;
} DMN_BREAK, *PDMN_BREAK;

// Data breakpoint notification structure {
typedef struct _DMN_DATABREAK {
    PVOID Address;
    DWORD ThreadId;
    DWORD BreakType;
    PVOID DataAddress;
} DMN_DATABREAK, *PDMN_DATABREAK;

// Debugstr notification structure
typedef struct _DMN_DEBUGSTR {
    DWORD ThreadId;
    DWORD Length;
    LPCSTR String;
} DMN_DEBUGSTR, *PDMN_DEBUGSTR;

// execution notification values
#define DMN_EXEC_STOP 0
#define DMN_EXEC_START 1
#define DMN_EXEC_REBOOT 2
#define DMN_EXEC_PENDING 3

// Module load notification structure
typedef struct _DMN_MODLOAD {
    char Name[MAX_PATH];
    PVOID BaseAddress;
    ULONG Size;
    ULONG TimeStamp;
    ULONG CheckSum;
    ULONG Flags;
} DMN_MODLOAD, *PDMN_MODLOAD;

#define DMN_MODFLAG_XBE     0x0001
#define DMN_MODFLAG_TLS     0x0002

// Section load/unload notification structure
typedef struct _DMN_SECTIONLOAD {
    char Name[MAX_PATH];
    PVOID BaseAddress;
    ULONG Size;
    USHORT Index;
    USHORT Flags;
} DMN_SECTIONLOAD, *PDMN_SECTIONLOAD;

#define DMN_SECFLAG_LOADED        0x0001
#define DMN_SECFLAG_WRITEABLE     0x0002
#define DMN_SECFLAG_UNINITIALIZED 0x0004

// thread create notification structure
typedef struct _DMN_CREATETHREAD {
    DWORD ThreadId;
    PVOID StartAddress;
} DMN_CREATETHREAD, *PDMN_CREATETHREAD;

// fiber create/delete notification structure
typedef struct _DMN_FIBER {
    DWORD FiberId;
    BOOL Create;
    PVOID StartAddress;
} DMN_FIBER, *PDMN_FIBER;

// thread switch notification structure
typedef struct _DMN_THREADSWITCH {
    DWORD OldThreadId;
    DWORD NewThreadId;
} DMN_THREADSWITCH, *PDMN_THREADSWITCH;

// exception notification
typedef struct _DMN_EXCEPTION {
    DWORD ThreadId;
    DWORD Code;
    PVOID Address;
    DWORD Flags;
    DWORD Information[2];
} DMN_EXCEPTION, *PDMN_EXCEPTION;

// bugcheck notification
typedef struct _DMN_BUGCHECK {
    DWORD BugCheckData[5];
} DMN_BUGCHECK, *PDMN_BUGCHECK;

#define DM_EXCEPT_NONCONTINUABLE 1
#define DM_EXCEPT_FIRSTCHANCE 2

// profiling
typedef struct _DMN_PROFINT {
    DWORD Eip;
    DWORD EFlags;
    DWORD SegCs;
} DMN_PROFINT, *PDMN_PROFINT;

// Notification
#define DM_PERSISTENT 1
#define DM_DEBUGSESSION 2
#define DM_ASYNCSESSION 4
typedef struct _DMN_SESSION *PDMN_SESSION;
DMHRAPI DmOpenNotificationSession(DWORD dwFlags, PDMN_SESSION *pSession);
DMHRAPI DmCloseNotificationSession(PDMN_SESSION Session);
DMHRAPI DmNotify(PDMN_SESSION Session, DWORD dwNotification,
    PDM_NOTIFY_FUNCTION pfnHandler);

// notification extensions
typedef DWORD (__stdcall *PDM_EXT_NOTIFY_FUNCTION)(LPCSTR szNotification);
DMHRAPI DmRegisterNotificationProcessor(PDMN_SESSION Session, LPCSTR szType,
    PDM_EXT_NOTIFY_FUNCTION pfn);

// protocol
#define DEBUGGER_PORT 0x2db

// breakpoint types
#define DMBREAK_NONE 0
#define DMBREAK_WRITE 1
#define DMBREAK_READWRITE 2
#define DMBREAK_EXECUTE 3
#define DMBREAK_FIXED 4

// breakpoint routines
DMHRAPI DmSetBreakpoint(PVOID addr);
DMHRAPI DmRemoveBreakpoint(PVOID addr);
DMHRAPI DmSetInitialBreakpoint(void);
DMHRAPI DmSetDataBreakpoint(PVOID addr, DWORD dwType, DWORD dwSize);
DMHRAPI DmIsBreakpoint(PVOID addr, LPDWORD dwType);

// execution start and stop
DMHRAPI DmStop(void);
DMHRAPI DmGo(void);
DMHRAPI DmHaltThread(DWORD dwThreadId);
DMHRAPI DmContinueThread(DWORD dwThreadId, BOOL fException);
DMHRAPI DmSetupFunctionCall(DWORD dwThreadId);

// debugger
DMAPI BOOL __stdcall DmIsDebuggerPresent(void);

// event stop control
#define DMSTOP_CREATETHREAD 1
#define DMSTOP_FCE 2
#define DMSTOP_DEBUGSTR 4
#define DMSTOP_STACKTRACE 8
DMHRAPI DmStopOn(DWORD dwStopFlags, BOOL fStop);

// stack trace requests
DMHRAPI DmEnableStackTrace(BOOL fEnable);
#define DM_STACKTRACE_SERVICE 12
#define DmStackTrace(sz) \
    do { \
        LPCSTR _ = (sz); \
        _asm { mov ecx, _ } \
        _asm { mov eax, DM_STACKTRACE_SERVICE } \
        _asm { int 2dh } \
        _asm { int 3 } \
    } while(0)

// reboot
#define DMBOOT_WAIT 1
#define DMBOOT_WARM 2
#define DMBOOT_NODEBUG 4
#define DMBOOT_STOP 8
DMHRAPI DmReboot(DWORD dwFlags);

//crash dump
DMHRAPI DmCrashDump();

// memory
DMHRAPI DmGetMemory(LPCVOID lpbAddr, DWORD cb, LPVOID lpbBuf,
    LPDWORD pcbRet);

// In some circumstances DmGetMemory2 can be faster than DmGetMemory.
// Requirements:
//  lpbAddr must be aligned on a 16-byte boundary.
//  lpbAddr can't point to code
//  cb must be a multiple of 16 bytes.

DMHRAPI DmGetMemory2(LPCVOID lpbAddr, DWORD cb, LPVOID lpbBuf,
    LPDWORD pcbRet);

// The checksum is computed on the Xbox, without transferring the
// memory across the network. Each block of bytes will have its own
// 32-bit CRC calculated separately. (If you just want to checksum
// the whole chunk of data, pass the same value for both cb
// and cbBlockSize.)
// Requirements:
//  lpbAddr must be aligned on an 8-byte boundary.
//  lpbAddr can't point to code
//  cb must be a multiple of cbBlockSize
//  cbBlockSize must be a multiple of 8 bytes
//  lpbBuf will have sizeof(DWORD)*(cb/cbBlockSize) bytes written to it.

DMHRAPI DmGetMemoryChecksum(LPCVOID lpbAddr, DWORD cb,
    DWORD cbBlockSize, LPVOID lpbBuf, LPDWORD pcbRet);

DMHRAPI DmSetMemory(LPVOID lpbAddr, DWORD cb, LPCVOID lpbBuf,
    LPDWORD pcbRet);

typedef struct _DM_WALK_MEMORY *PDM_WALK_MEMORY;
typedef struct _DM_MEMORY_REGION
{
    PVOID BaseAddress;
    DWORD Size;
    DWORD Protect;
} DM_MEMORY_REGION, *PDM_MEMORY_REGION;
DMHRAPI DmWalkCommittedMemory(PDM_WALK_MEMORY *, PDM_MEMORY_REGION);
DMHRAPI DmCloseCommittedMemory(PDM_WALK_MEMORY);

// pool memory
DMAPI PVOID __stdcall DmAllocatePool(ULONG cb);
DMAPI PVOID __stdcall DmAllocatePoolWithTag(ULONG cb, ULONG tag);
DMAPI VOID __stdcall DmFreePool(PVOID p);

// profile interrupts
typedef void (__stdcall *PDMPROFILE_HANDLER)(PDMN_PROFINT);
DMHRAPI DmStartProfile(PHANDLE, ULONG, PDMPROFILE_HANDLER);
DMHRAPI DmStopProfile(HANDLE);

// thread stopped info
typedef struct _DM_THREADSTOP {
    DWORD NotifiedReason;
    union {
        DMN_BREAK Break;
        DMN_DATABREAK DataBreak;
        DMN_EXCEPTION Exception;
        DMN_DEBUGSTR DebugStr;
        DMN_BUGCHECK BugCheck;
    } u;
} DM_THREADSTOP, *PDM_THREADSTOP;

// thread general info
typedef struct _DM_THREADINFO {
    DWORD SuspendCount;
    DWORD Priority;
    LPVOID TlsBase;
} DM_THREADINFO, *PDM_THREADINFO;

typedef struct _DM_THREADINFOEX {
    DWORD    Size; //Size of this structure
    DWORD    SuspendCount;
    DWORD    Priority;
    LPVOID   TlsBase;
    LPVOID   StartAddress;
    LPVOID   StackBase;
    LPVOID   StackLimit;
    FILETIME CreateTime;
} DM_THREADINFOEX, *PDM_THREADINFOEX;

// thread routines
DMHRAPI DmGetThreadList(LPDWORD rgdwThreads, LPDWORD pcThreads);
DMHRAPI DmGetThreadContext(DWORD dwThreadId, PCONTEXT pdmcr);
DMHRAPI DmSetThreadContext(DWORD dwThreadId, PCONTEXT pdmcr);
DMHRAPI DmIsThreadStopped(DWORD dwThreadId, PDM_THREADSTOP pdmts);
DMHRAPI DmGetThreadInfo(DWORD dwThreadId, PDM_THREADINFO pdmti);
DMHRAPI DmGetThreadInfoEx(DWORD dwThreadId, PDM_THREADINFOEX pdmti);
DMHRAPI DmSuspendThread(DWORD dwThreadId);
DMHRAPI DmResumeThread(DWORD dwThreadId);

// XTL data
typedef struct _DM_XTLDATA {
    DWORD LastErrorOffset;
} DM_XTLDATA, *PDM_XTLDATA;

DMHRAPI DmGetXtlData(PDM_XTLDATA);

// loaded modules and sections
typedef struct _DM_WALK_MODULES *PDM_WALK_MODULES;
DMHRAPI DmWalkLoadedModules(PDM_WALK_MODULES *, PDMN_MODLOAD);
DMHRAPI DmCloseLoadedModules(PDM_WALK_MODULES);
typedef struct _DM_WALK_MODSECT *PDM_WALK_MODSECT;
DMHRAPI DmWalkModuleSections(PDM_WALK_MODSECT *, LPCSTR, PDMN_SECTIONLOAD);
DMHRAPI DmCloseModuleSections(PDM_WALK_MODSECT);
DMHRAPI DmGetModuleLongName(LPCSTR szShortName, LPSTR szLongName, LPDWORD pcch);

// XBE info
typedef struct _DM_XBE {
    char LaunchPath[MAX_PATH+1];
    DWORD TimeStamp;
    DWORD CheckSum;
    DWORD StackSize;
} DM_XBE, *PDM_XBE;
DMHRAPI DmGetXbeInfo(LPCSTR szName, PDM_XBE);

#define DM_XBEONDISKONLY  0x00000001
DMHRAPI DmGetXbeInfoEx(LPCSTR szName, PDM_XBE, DWORD dwFlags);

// command extension
typedef ULONG (__stdcall *PDM_ENTRYPROC)(ULONG, ULONG, ULONG);

typedef struct _DM_CMDCONT *PDM_CMDCONT;
typedef HRESULT (__stdcall *PDM_CMDCONTPROC)(PDM_CMDCONT pdmcc, LPSTR
    szResponse, DWORD cchResponse);
typedef struct _DM_CMDCONT {
    PDM_CMDCONTPROC HandlingFunction;
    DWORD DataSize;
    PVOID Buffer;
    DWORD BufferSize;
    PVOID CustomData;
    DWORD BytesRemaining;
} DM_CMDCONT;

typedef HRESULT (__stdcall *PDM_CMDPROC)(LPCSTR szCommand, LPSTR szResponse,
    DWORD cchResponse, PDM_CMDCONT pdmcc);
DMHRAPI DmRegisterCommandProcessor(LPCSTR szProcessor, PDM_CMDPROC pfn);
DMHRAPI DmRegisterCommandProcessorEx(LPCSTR szProcessor, PDM_CMDPROC pfn,
    PVOID pfnCreateThread);

#define DmRegisterThreadedCommandProcessor(sz, pfn) \
    DmRegisterCommandProcessorEx(sz, pfn, CreateThread)

DMHRAPI DmSendNotificationString(LPCSTR sz);

// per-thread data
DMHRAPI DmThreadUserData(DWORD tid, LPDWORD *ppdwData);
#define DM_CURRENT_THREAD -1

// Dynamic loading of debugger extensions
DMHRAPI DmLoadExtension(LPCSTR szName, PHANDLE phModule, PVOID *pvBase);
DMHRAPI DmUnloadExtension(HANDLE hModule);
DMHRAPI DmGetProcAddress(HANDLE hModule, LPCSTR szProcName, PVOID *ppvRet);
DMHRAPI DmGetFileAccessCount(LPDWORD lpdwFileAccessCount);

// name functions
DMHRAPI DmGetXboxName(LPSTR, LPDWORD);
DMHRAPI DmSetXboxName(LPCSTR);

// management functions
typedef struct _DM_USER {
    char UserName[256];
    DWORD AccessPrivileges;
} DM_USER, *PDM_USER;

#define DMPL_PRIV_READ           0x0001
#define DMPL_PRIV_WRITE          0x0002
#define DMPL_PRIV_CONTROL        0x0004
#define DMPL_PRIV_CONFIGURE      0x0008
#define DMPL_PRIV_MANAGE         0x0010

DMHRAPI DmAddUser(LPCSTR szUserName, DWORD dwAccess);
DMHRAPI DmRemoveUser(LPCSTR szUserName);
DMHRAPI DmSetUserAccess(LPCSTR szUserName, DWORD dwAccess);
DMHRAPI DmGetUserAccess(LPCSTR szUserName, LPDWORD lpdwAccess);
typedef struct _DM_WALK_USERS *PDM_WALK_USERS;
DMHRAPI DmWalkUserList(PDM_WALK_USERS *, PDM_USER);
DMHRAPI DmCloseUserList(PDM_WALK_USERS);
DMHRAPI DmEnableSecurity(BOOL fEnable);
DMHRAPI DmIsSecurityEnabled(LPBOOL pfEnabled);

// title to launch
#define DMTITLE_PERSIST    0x0001
#define DMTITLE_UNPERSIST  0x0002
DMHRAPI DmSetTitle(LPCSTR szDir, LPCSTR szTitle, LPCSTR szCmdLine);
DMHRAPI DmSetTitleEx(LPCSTR szDir, LPCSTR szTitle, LPCSTR szCmdLine, DWORD dwFlags);

typedef struct _DM_D3DSTATE
{
    DWORD   cbSize;
    DWORD   renderState[166];           // [D3DRS_MAX]
    DWORD   textureStageState[4*32];    // [D3DTSS_MAXSTAGES*D3DTSS_MAX]
} DM_D3DSTATE, *LPDM_D3DSTATE;
typedef const DM_D3DSTATE *LPCDM_D3DSTATE;


// start / stop profile data collection
DMHRAPI DmCAPControl(LPCSTR action);

// performance counters
#define DMCOUNT_FREQUENCY   0x000F
#define DMCOUNT_FREQ100MS   0x0001
#define DMCOUNT_FREQ1SEC    0x000A

#define DMCOUNT_COUNTTYPE   0x0030
#define DMCOUNT_EVENT       0x0010
#define DMCOUNT_VALUE       0x0000
#define DMCOUNT_PRATIO      0x0020
#define DMCOUNT_COUNTSUBTYPE 0x0FC0

// event rates
#define DMCOUNT_PERSEC      0x0040
#define DMCOUNT_PERMSEC     0x0080
#define DMCOUNT_PERFRAME    0x0100
#define DMCOUNT_PERTICK     0x0200

// value types
#define DMCOUNT_AVERAGE     0x0040

typedef struct _DM_COUNTDATA {
    LARGE_INTEGER CountValue;
    LARGE_INTEGER RateValue;
    DWORD CountType;
} DM_COUNTDATA, *PDM_COUNTDATA;

typedef struct _DM_COUNTINFO {
    char Name[256];
    DWORD Type;
} DM_COUNTINFO, *PDM_COUNTINFO;

DMHRAPI DmOpenPerformanceCounter(LPCSTR szName, HANDLE *phCounter);
DMHRAPI DmQueryPerformanceCounterHandle(HANDLE hCounter, DWORD dwType, PDM_COUNTDATA);
DMHRAPI DmClosePerformanceCounter(HANDLE hCounter);
typedef struct _DM_WALK_COUNTERS *PDM_WALK_COUNTERS;
DMHRAPI DmWalkPerformanceCounters(PDM_WALK_COUNTERS *, PDM_COUNTINFO);
DMHRAPI DmCloseCounters(PDM_WALK_COUNTERS);
DMHRAPI DmEnableGPUCounter(BOOL);


typedef struct _DM_UTILITY_DRIVE_INFO
{
    DWORD dwFlags;
    DWORD rgdwTitleId[3];
} DM_UTILITY_DRIVE_INFO, *PDM_UTILITY_DRIVE_INFO;
#define DM_UTILITY_DRIVE_0_NEVER_USED 0x00000001
#define DM_UTILITY_DRIVE_1_NEVER_USED 0x00000002
#define DM_UTILITY_DRIVE_2_NEVER_USED 0x00000004
#define DM_UTILITY_DRIVE_0_NOT_USED   0x00000010
#define DM_UTILITY_DRIVE_1_NOT_USED   0x00000020
#define DM_UTILITY_DRIVE_2_NOT_USED   0x00000040
DMHRAPI DmGetUtilityDriveInfo(PDM_UTILITY_DRIVE_INFO pdmUtilityDriveInfo);

typedef HRESULT (__stdcall *PDM_COUNTPROC)(PLARGE_INTEGER, PLARGE_INTEGER);
#define DMCOUNT_SYNC      0x00010000
#define DMCOUNT_ASYNC32   0x00020000
#define DMCOUNT_ASYNC64   0x00040000
#define DMCOUNT_ASYNC     0x00080000
DMHRAPI DmRegisterPerformanceCounter(LPCSTR szName, DWORD dwType, PVOID);
DMHRAPI DmUnregisterPerformanceCounter(LPCSTR szName);

// error codes
#define FACILITY_XBDM 0x2db
#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT)(((unsigned long)sev<<31)|((unsigned long)fac<<16)|((unsigned long)code)))
#endif
#define XBDM_HRESERR(code) MAKE_HRESULT(1, FACILITY_XBDM, code)
#define XBDM_HRESSUCC(code) MAKE_HRESULT(0, FACILITY_XBDM, code)

#define XBDM_NOERR XBDM_HRESSUCC(0)

#define XBDM_UNDEFINED XBDM_HRESERR(0)
#define XBDM_MAXCONNECT XBDM_HRESERR(1)
#define XBDM_NOSUCHFILE XBDM_HRESERR(2)
#define XBDM_NOMODULE XBDM_HRESERR(3)
#define XBDM_MEMUNMAPPED XBDM_HRESERR(4)
#define XBDM_NOTHREAD XBDM_HRESERR(5)
#define XBDM_CLOCKNOTSET XBDM_HRESERR(6)
#define XBDM_INVALIDCMD XBDM_HRESERR(7)
#define XBDM_NOTSTOPPED XBDM_HRESERR(8)
#define XBDM_MUSTCOPY XBDM_HRESERR(9)
#define XBDM_ALREADYEXISTS XBDM_HRESERR(10)
#define XBDM_DIRNOTEMPTY XBDM_HRESERR(11)
#define XBDM_BADFILENAME XBDM_HRESERR(12)
#define XBDM_CANNOTCREATE XBDM_HRESERR(13)
#define XBDM_CANNOTACCESS XBDM_HRESERR(14)
#define XBDM_DEVICEFULL XBDM_HRESERR(15)
#define XBDM_NOTDEBUGGABLE XBDM_HRESERR(16)
#define XBDM_BADCOUNTTYPE XBDM_HRESERR(17)
#define XBDM_COUNTUNAVAILABLE XBDM_HRESERR(18)
#define XBDM_NOTLOCKED XBDM_HRESERR(20)
#define XBDM_KEYXCHG XBDM_HRESERR(21)
#define XBDM_MUSTBEDEDICATED XBDM_HRESERR(22)
#define XBDM_INVALIDARG XBDM_HRESERR(23)
#define XBDM_PROFILENOTSTARTED XBDM_HRESERR(24)
#define XBDM_PROFILEALREADYSTARTED XBDM_HRESERR(25)
#define XBDM_D3D_DEBUG_COMMAND_NOT_IMPLEMENTED XBDM_HRESERR(0x50)
#define XBDM_D3D_INVALID_SURFACE XBDM_HRESERR(0x51)
#define XBDM_VX_TASK_PENDING XBDM_HRESERR(0x60)
#define XBDM_VX_TOO_MANY_SESSIONS XBDM_HRESERR(0x61)
#define XBDM_CANNOTCONNECT XBDM_HRESERR(0x100)
#define XBDM_CONNECTIONLOST XBDM_HRESERR(0x101)
#define XBDM_FILEERROR XBDM_HRESERR(0x103)
#define XBDM_ENDOFLIST XBDM_HRESERR(0x104)
#define XBDM_BUFFER_TOO_SMALL XBDM_HRESERR(0x105)
#define XBDM_NOTXBEFILE XBDM_HRESERR(0x106)
#define XBDM_MEMSETINCOMPLETE XBDM_HRESERR(0x107)
#define XBDM_NOXBOXNAME XBDM_HRESERR(0x108)
#define XBDM_NOERRORSTRING XBDM_HRESERR(0x109)

#define XBDM_CONNECTED XBDM_HRESSUCC(1)
#define XBDM_MULTIRESPONSE XBDM_HRESSUCC(2)
#define XBDM_BINRESPONSE XBDM_HRESSUCC(3)
#define XBDM_READYFORBIN XBDM_HRESSUCC(4)
#define XBDM_DEDICATED XBDM_HRESSUCC(5)
#define XBDM_PROFILERESTARTED XBDM_HRESSUCC(6)
#define XBDM_FASTCAPENABLED XBDM_HRESSUCC(7)

// Call-Attributed Profiler Support Function
DMHRAPI DmStartProfiling(LPCSTR szLogFileName, DWORD dwDataBufferSize);
DMHRAPI DmStopProfiling(VOID);
DMHRAPI DmIsFastCAPEnabled(VOID);



typedef struct _DM_MEMORY_STATISTICS {
    DWORD cbSize;
    DWORD TotalPages;
    DWORD AvailablePages;
    DWORD StackPages;
    DWORD VirtualPageTablePages;
    DWORD SystemPageTablePages;
    DWORD PoolPages;
    DWORD VirtualMappedPages;
    DWORD ImagePages;
    DWORD FileCachePages;
    DWORD ContiguousPages;
    DWORD DebuggerPages;
} DM_MEMORY_STATISTICS, *PDM_MEMORY_STATISTICS;

DMHRAPI DmQueryMemoryStatistics(PDM_MEMORY_STATISTICS MemStat);

//
// Allocation tracking types and definitions
//

#define DM_TRACK_HEAP                   0x0001
#define DM_TRACK_VIRTUAL_MEMORY         0x0002
#define DM_TRACK_CONTIGUOUS_MEMORY      0x0004
#define DM_TRACK_SYSTEM_MEMORY          0x0008
#define DM_TRACK_DEBUG_MEMORY           0x0010
#define DM_TRACK_KERNEL_POOL            0x0020
#define DM_TRACK_HANDLE                 0x0040
#define DM_TRACK_CUSTOM                 0x0080
#define DM_TRACK_TYPEMASK               0x00FF
#define DM_TRACK_ASSERT_ON_FAILURES     0x0100
#define DM_TRACK_MASK                   0x01FF

DMHRAPI DmQueryAllocationTypeName(USHORT AllocationType, LPSTR pszName, SIZE_T nSize);
DMHRAPI DmRegisterAllocationType(LPCSTR pszName, USHORT* AllocationnType);
DMHRAPI DmInsertAllocationEntry(PVOID AllocPtr, SIZE_T AllocSize, USHORT AllocType);
DMHRAPI DmRemoveAllocationEntry(PVOID AllocPtr, SIZE_T AllocSize, USHORT AllocType);

#define DM_ALLOCTYPE_HEAP               ((USHORT)0)
#define DM_ALLOCTYPE_VIRTUAL_MEMORY     ((USHORT)1)
#define DM_ALLOCTYPE_CONTIGUOUS_MEMORY  ((USHORT)2)
#define DM_ALLOCTYPE_SYSTEM_MEMORY      ((USHORT)3)
#define DM_ALLOCTYPE_DEBUG_MEMORY       ((USHORT)4)
#define DM_ALLOCTYPE_KERNEL_POOL        ((USHORT)5)
#define DM_ALLOCTYPE_DIRECTORY_OBJECT   ((USHORT)6)
#define DM_ALLOCTYPE_EVENT              ((USHORT)7)
#define DM_ALLOCTYPE_FILE               ((USHORT)8)
#define DM_ALLOCTYPE_IO_COMPLETION      ((USHORT)9)
#define DM_ALLOCTYPE_MUTANT             ((USHORT)10)
#define DM_ALLOCTYPE_SEMAPHORE          ((USHORT)11)
#define DM_ALLOCTYPE_TIMER              ((USHORT)12)
#define DM_ALLOCTYPE_THREAD             ((USHORT)13)
#define DM_ALLOCTYPE_INTERNAL_MAX       ((USHORT)14)

//
// Stack back trace helper function
//

#define DM_MAX_STACK_DEPTH  32

DMHRAPI DmCaptureStackBackTrace(ULONG FramesToCapture, PVOID *BackTrace);

//
// Internal PDB signature for verification
//

typedef struct _DM_PDB_SIGNATURE {
    DWORD Signature;    // NB10 or RSDS
    union {
        struct {
            DWORD Offset;
            ULONG Sig6;
            ULONG Age6;
        } nb10;
        struct {
            GUID Guid;
            DWORD Age7;
        } rsds;
    } u;
} DM_PDB_SIGNATURE, *PDM_PDB_SIGNATURE;

//
// DmFindPdbSignature
//
// This API locates and copies PDB signature (use in DmLoadSymbolsForModuleEx)
//

DMHRAPI
DmFindPdbSignature(
    IN  ULONG_PTR BaseAddress,
    OUT PDM_PDB_SIGNATURE PdbSignature
    );


#ifdef __cplusplus
}
#endif

#endif
