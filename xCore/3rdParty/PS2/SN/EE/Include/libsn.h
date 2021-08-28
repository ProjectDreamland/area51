
#ifndef _LIBSN_H
#define _LIBSN_H

/* Version 1.20 */
/* 1.20 TC  16-10-2003 - Rebuilt to remove warnings about multiply defined symbols (SetGp)  */
/* 1.19 ADB 18-09-2003 - New func to allow debugger script to hook RPC from PS2 code        */
/* 1.18 TC  28-05-2003 - New func to return EE boot parameters: snGetEEBootParams()         */
/* 1.17 MND 27-05-2003 - DLL support: support for inserting/removing breakpoints as a DLL   */
/*                       loads/unloads + the ability for the target to tell the debugger to */
/*                       insert/remove breakpoints while the DLL is being patched.          */
/* 1.16 TC  29-04-2003 - DLL support: changed the dll unload code so that the DLL's symbol  */
/*                       table is not deactivated until after the global destructors have   */
/*                       been called.                                                       */
/*                     - Added 'const' to snDllGetFunctionAddress() prototype.              */
/* 1.15 TC  09-07-2002 - Added lib version symbols                                          */
/* 1.14 MND 25-04-2002 - DLL support has a new function to lookup name and return a pointer */
/* 1.13 MND 22-04-2002 - DLL support now returns NULL when no indexed functions are present */
/* 1.12 ADB 17-12-2001 - Removed DEBUG/VU modules because 1.72.21 PS2DBG does this now      */
/* 1.11 TC  VU0/1 MAC flags now preserved when doing Get/Set ACC                            */
/* 1.10 ADB irq handler now initialises own local stack space                               */
/* 1.9  ADB removed inter-function branch that upset ps2ld fn stripping                     */
/* 1.8  ADB removed dependency on 2.2.4 lib version of EI/DI                                */
/* 1.7  ADB paranoia... preserved other regs in context                                     */
/* 1.6  ADB fixed occasional spurious corruption of lo register                             */
/* 1.5  ADB samples written to uncached space                                               */
/* 1.4  ADB wrapped snProfEnableInt and snProfDisableInt with global EI/DI                  */
/* 1.3  ADB added snProfEnableInt and snProfDisableInt                                      */


#ifdef __cplusplus
extern "C" {
#endif

extern unsigned int __SN_Libsn_version[];	/* Declared as array so that compiler long refs this symbol */

/* Example
	printf("libsn version = %d\n",__SN_Libsn_version[0]);
*/

#ifndef	UINT32
#define UINT32 unsigned int
#define NEEDUNDEFUINT32
#endif

/**************************/
/* SN debugger extensions */

/* Routine to install additional debugger supprt functions */

extern int snDebugInit (void);	// this is now just a dummy Fn that returns 1 because the 1.72.21 debugger onwards does this automatically.
extern int __snIsDebuggerRunning;
extern int snIsDebuggerRunning(void);	/* fn or macro? leave this as a function for now */
extern void snNotifyDebugger(void);		/* Notify debugger after loading/unloading a dll/overlay */
extern void snAllBreaksOut(void);		/* Request debugger to remove all breakpoints from memory */
extern void snAllBreaksIn(void);		/* Request debugger to restore all removed breakpoints  */
extern unsigned int* snGetEEBootParams(void);	/* Returns pointer to: unsigned int nEEBootParams[2] */
extern int snCallDebugger(int n);		/* return -1 if it fails (debugger not running) */

#define EEPARAM_MEM_CONTROLLER_FIX_OFFSET 0	/* Legacy: Use EEPARAM_OFFSET instead */
#define EEPARAM_OFFSET 0
#define EEPARAM_MEM_CONTROLLER_FIX_MASK (1<<0)
#define EEPARAM_STARTUP_FROM_DISC_MASK (1<<1)
#define EEPARAM_RESERVED1_MASK (1<<2)
#define EEPARAM_RESERVED2_MASK (1<<3)
#define EEPARAM_YCrCb_VIDEO_OUTPUT_MASK (1<<4)	/* 0=RGB video output, 1=YCrCb video output */
#define EEPARAM_32MB_MASK (1<<5)		/* 0=128MB, 1=32MB */
#define EEPARAM_CHECK_STACK_MASK (1<<6)		/* 1=Check for stack overrun on thread switches */

/* Example:
	unsigned int* pnEEBootParams = snGetEEBootParams();
	if(pnEEBootParams)
	{
		if((pnEEBootParams[EEPARAM_OFFSET] & EEPARAM_MEM_CONTROLLER_FIX_MASK) == 0)
		{
			printf("EE Memory Controller Workaround *NOT* enabled\n");
		}
	}
	else
	{
		printf("Failed to get EE boot parameters!\n");
	}
*/

/*************************************/
/* SN Run time DLL handling routines */

/* call at the start of your code to initialise the dll system */

extern	int snInitDllSystem (void** index_pointer);

/* Call this routine after a dll has been loaded into memory to relocate and initialise it */

extern	int snDllLoaded (void* buffer, void** index_pointer);

/* Call this routine to close down a dll before freeing the memory it's in */

extern	int snDllUnload (void* buffer);

/* Call this routine to re-relocate a dll to a new address */
/* If your program has taken any pointers to the dll code or data your program will fail */
/* A dll cannot move itself or call a routine that moves it */

extern int snDllMove (void* destination, void* source, void** index_pointer);

/* Call this routine to look up the address of the specified function */
/* If it's not found 0 is returned */

extern unsigned int snDllGetFunctionAddress (const char* name);

#define SN_DLL_SUCCESS                        0	/* operation succeeded */

#define SN_DLL_NOT_A_DLL                      1	/* The buffer doesn't seem to contain a dll */
#define SN_DLL_BAD_VERSION                    2	/* The dll version is not support by this code */
#define SN_DLL_INVALID                        3	/* Some data in the dll header was invalid */
#define SN_DLL_BAD_ALIGN                      4	/* The dll is not aligned to the required boundary */
#define SN_DLL_NOT_LOADED                     5	/* The dll has not been loaded so can't be unloaded */
#define SN_DLL_TOO_MANY_MODULES               6	/* Too many modules loaded */
#define SN_DLL_INVALID_SYMTYPE                7 /* Symbol type invalid - dll file is corrupt or libsn.a needs updating  */
#define SN_DLL_INVALID_DEFINE_GLOBAL_FAILED   8 /* Failure defining a global symbol. Most likely cause is that there is a breakpoint on an address that needs to be patched */
#define SN_DLL_INVALID_DEFINE_ABS_FAILED      9 /* Failure defining an absolute symbol. Most likely cause is that there is a breakpoint on an address that needs to be patched */
#define SN_DLL_INVALID_PATCH_FAILED          10 /* Invalid patch type - dll file is corrupt or libsn.a needs updating */
#define SN_DLL_INVALID_REMOVE_RELMOD_FAILED  11 /* The dll has not been loaded so can't be moved */

/*============ PROFILER STUFF ====================*/

/* interval values to pass to snProfInit() and snProfSetInterval() */
#define	_1KHZ	300000
#define	_2KHZ	150000
#define	_4KHZ	 75000
#define	_10KHZ	 30000
#define	_20KHZ	 15000

struct _PROFHDR
{
	UINT32	interval;	/* interval count (cpu clocks) */
	UINT32	startpc;	/* profile lower limit for PC */
	UINT32	endpc;		/* profile upper limit for PC */
	UINT32	mask;		/* mask value for above flags */

	UINT32	flags;		/* flags */
	UINT32	buffptr;	/* ptr to two profile-sample-data buffers */
	UINT32	bufflen;	/* length of one-half sample buffer */
	UINT32	ptr;		/* actually an index but we just want to know if it is 0 or -1 */
/*	UINT32	buffnum;	not present on IOP side */
};
typedef struct _PROFHDR PROFHDR;

struct _PROFSAMPLE
{
	UINT32	pc;
/*	UINT32	flags; */
};
typedef struct _PROFSAMPLE PROFSAMPLE;

/* Export from profile.c */
int snProfInit(unsigned int rate, void* buffer, int buffersize );
void	snProfEnableInt();		
void	snProfDisableInt();		

/* and it's return values */
#define SN_PRF_SUCCESS          0	/* operation succeeded */
#define SN_PRF_TEQFAIL			1	/* IRQ hook failed to install */
#define SN_PRF_NOIOP			2	/* IOP sync failed */
#define SN_PRF_BUFFALIGN		3	/* The profile data buffer is not quadword aligned */
#define SN_PRF_BUFFSIZE			4	/* The buffer is too small or too large */
#define SN_PRF_RPCFAIL       	5   /* Cannot RPC the IOP module */

/* things in prof.s */
extern	int 	_snProfInit(unsigned int rate, void* buffer, int buffersize );
extern	void	_snProfEnableInt();
extern	void	_snProfDisableInt();
extern	int	snDIntr();
extern	int	snEIntr();
extern	void	snProfSetInterval(UINT32 interval);
extern	void	snProfSetRange(int profmask, void* startpc, void* endpc);
extern	int		snProfSetFlagValue(int value);
extern	int		snProfSetFlags(int flags);
extern	int		snProfClrFlags(int flags);

#ifdef NEEDUNDEFUINT32				/* only undefine it if *we* defined it */
#undef UINT32
#endif

#ifdef __cplusplus
}
#endif

#endif
