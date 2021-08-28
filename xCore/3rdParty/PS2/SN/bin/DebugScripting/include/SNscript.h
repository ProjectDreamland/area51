#ifndef _SNSCRIPT_H
#define _SNSCRIPT_H

//
// ADB: SNscript.h
//
// This header file defines and declares the ProDG debugger builtin functions
// that are available to scripting as well as the types that these functions
// make use of.
// You must include this file if you wish to make use of the debugger specific
// scripting extensions.
//

// ---- 1) Things that are defined in both EiC and MS VC++
//
// Events which can be hooked by a pane callback
//
#define	SM_TARGETATN	 0	// target CPU changed state (run, stop, breakpoint etc
#define SM_CHAR			 1	// script pane: character key (translated keypress) pressed
#define SM_KEYDOWN		 2	// script pane: key pressed
#define SM_TIMER		 3	// custom timer tick
#define SM_MOUSECLICK	 4	// mouse clicked in this script pane
#define SM_DBLCLICK		 5	// mouse double clicked in this script pane
#define	SM_MOUSEACTIVATE 6	// script pane activated
#define	SM_LBDOWN		 7
#define SM_LBDBLCLICK	 8
#define SM_MENU			 9	// context menu set with SNAddMenu was selected
#define SM_CLOSE		10  // the script pane is closing
#define SM_RPCCALL		11	// code on the target system has called the debugger

typedef unsigned int     SNPARAM;	// parameters to SN callback

// Keybinding "KeyState" flags word
#define KS_SHIFT	0x0001	// bit 0 => shift key
#define KS_CONTROL	0x0002	// bit 1 => control key
#define KS_ALT		0x0004	// bit 2 => alt key

typedef unsigned int	SNADDR;	// a target address

//
// Types returned/used by the SN debugger's expression eval (see SNevaluate() call below)
//
struct SNint64 {unsigned int word[2];};
struct SNint128 {unsigned int word[4];};

typedef signed char      SN_SIGNED_8BIT;
typedef unsigned char    SN_UNSIGNED_8BIT;
typedef signed short     SN_SIGNED_16BIT;
typedef unsigned short   SN_UNSIGNED_16BIT;
typedef signed int       SN_SIGNED_32BIT;
typedef unsigned int     SN_UNSIGNED_32BIT;
typedef struct SNint64   SN_SIGNED_64BIT;
typedef struct SNint64	 SN_UNSIGNED_64BIT;
typedef struct SNint128	 SN_SIGNED_128BIT;
typedef struct SNint128	 SN_UNSIGNED_128BIT;
typedef float            SN_FP_32BIT;
typedef double           SN_FP_64BIT;
typedef long double      SN_FP_80BIT;

enum SN_script_valtype
{
	SNVAL_NONE,
	SNVAL_SI8BIT,
	SNVAL_UI8BIT,
	SNVAL_SI16BIT,
	SNVAL_UI16BIT,
	SNVAL_SI32BIT,
	SNVAL_UI32BIT,
	SNVAL_SI64BIT,
	SNVAL_UI64BIT,
	SNVAL_SI128BIT,
	SNVAL_UI128BIT,
	SNVAL_FP32BIT,
	SNVAL_FP64BIT,
	SNVAL_FP80BIT,
	SNVAL_STRING,
	SNVAL_WSTRING,
	SNVAL_ADDRESS,
	SNVAL_TYPE,
	SNVAL_DERIVED
};

union SNdata
{
	SN_SIGNED_8BIT     s8;
	SN_UNSIGNED_8BIT   u8;
	SN_SIGNED_16BIT    s16;
	SN_UNSIGNED_16BIT  u16;
	SN_SIGNED_32BIT    s32;
	SN_UNSIGNED_32BIT  u32;
	SN_SIGNED_64BIT    s64;
	SN_UNSIGNED_64BIT  u64;
	SN_SIGNED_128BIT   s128;
	SN_UNSIGNED_128BIT u128;
	SN_FP_32BIT        fp32;
	SN_FP_64BIT        fp64;
	SN_FP_80BIT        fp80;
	char*              string;
/*	wchar_t*           wstring; */
};

struct SNvalue
{
	enum SN_script_valtype	type;
	SNADDR					address;
	union SNdata			val;
};

typedef struct SNvalue snval_t;

//
// Paneinfo structure returned by SNGetPaneInfo (see below)
//
struct paneinfo
{
	unsigned int	handle;		// hwnd
	unsigned int	panetype;	// pane type
	SNADDR			cursaddr;	// address the cursor is over
	SNADDR			panestart;	// address the pane starts drawing at
};
typedef struct paneinfo PANEINFO;

// ---- 2) Things that are defined only under EiC

#ifdef _EiC

// flags used by SNLoadELF ()
#define	LFLOAD			(1<<0)	// load code
#define	LFSYM			(1<<1)	// load symbols
#define	LFEXEC			(1<<2)	// execute after code loaded
#define	LFSTARTUP		(1<<3)	// load and execute but breakpoint at main/start (PS2 EE & IOP)
#define	LFRESET			(1<<4)	// reset before loading code
#define	LFRESETFS		(1<<5)	// reset FileServer root directory
#define LFIMPBPS		(1<<6)	// import BPs from MS Visual Studio  (PS2 EE & IOP)
#define LFSLOWSYM		(1<<7)	// thorough (not shortcut) symbol loading
#define LFCLRBPS		(1<<8)	// clear BreakPoints
#define	LFRESETHOME		(1<<9)	// reset fileserver home directory
#define LFCLRTTY		(1<<10)	// set if loading the ELF should clear TTY channels first
#define LFUSEDVDEMU		(1<<11)
#define LFRESETFS_USE_ELF_DIR (1<<12)	// RESET THE FILE SERVER ROOT DIRECTORY WITH THE ELF DIRECTORY - MUST BE SPECIFIED WITH LFRESETFS
#define LFRESETHOME_USE_ELF_DIR (1<<13)	// RESET THE HOME DIRECTORY WITH THE ELF DIRECTORY - MUST BE SPECIFIED WITH LFRESETHOME
#define LFNOKPATCH		(1<<14)			// NO KERNEL PATCHING
#define	LFKEEPOLDSYM	(1<<15)			// set => don't clear symbols before loading new ones

// TTY channel indexes (these examples are PS2)
#define ALLTTY		0			// buffer 0 gets MAINCPU (in black) plus IOPTTY (in blue)
#define	DBGTTY		1			// buffer 1 gets Debugger's internal diagnostic text
#define	TMTTY		2			// buffer 2 gets Target Manager
#define MAINTTY		3			// buffer 3 gets main CPU
#define IOPTTY		4			// buffer 4 gets IOP
#define LOGTTY		5			// buffer 5 gets LOG stream
#define NUMTTY		6

//
// Debugger Window types (see parameters to SNUpdateWindows)
//
#define	WT_REGISTERS	0
#define	WT_MEMORY		1
#define	WT_DISASM		2
#define	WT_SOURCE		3
#define	WT_WATCH		4
#define WT_LOCALS		5
#define WT_BREAK		6
#define WT_STACK		7
// ps2 specific
#define WT_PS2TTY		8
#define WT_PS2IOPMODS	9
#define WT_PS2DMA		10
#define	WT_PS2EEPROFILE	11

// agb specific
#define WT_AGBTTY		12

// ngc specific
#define WT_NGCTTY		13

// back to non-target specific
#define WT_WORKSPACE	14

#define WT_AGBPALETTE	15
#define WT_SCRIPT		16

#define	WT_NGCPROFILE	17
#define WT_KERNELPANE	18	// THREADING
#define	WT_AGBPROFILE	19
#define WT_NGCDL		20	// NGC Display List Pane

#define	WT_FORCEUPDATE	31	// not really a window type, it's set if immediate pane update is essential

#define M_REGISTERS		(1<<WT_REGISTERS)
#define M_MEMORY		(1<<WT_MEMORY)
#define M_DISASM		(1<<WT_DISASM)
#define M_SOURCE		(1<<WT_SOURCE)
#define M_WATCH			(1<<WT_WATCH)
#define M_LOCALS		(1<<WT_LOCALS)
#define M_BREAK			(1<<WT_BREAK)
#define M_STACK			(1<<WT_STACK)

#define M_PS2TTY		(1<<WT_PS2TTY)
#define M_PS2IOP		(1<<WT_PS2IOPMODS)
#define M_PS2DMA		(1<<WT_PS2DMA)
#define M_PS2EEPROFILE	(1<<WT_PS2EEPROFILE)

#define M_AGBTTY		(1<<WT_AGBTTY)
#define M_NGCTTY		(1<<WT_NGCTTY)

#define M_WORKSPACE		(1<<WT_WORKSPACE)
//
// RGO PALETTE
#define M_AGBPALETTE	(1<<WT_AGBPALETTE)
#define M_SCRIPT		(1<<WT_SCRIPT)

#define M_NGCPROFILE	(1<<WT_NGCPROFILE)
#define M_KERNELPANE	(1<<WT_KERNELPANE) // THREADING
#define M_AGBPROFILE	(1<<WT_AGBPROFILE)
#define	M_NGCDL			(1<<WT_NGCDL)

#define	M_FORCEUPDATE	(1<<WT_FORCEUPDATE)

#define M_ALL			0x7FFFFFFF	// note FORCEUPDATE bit is OFF by default.
#define	M_NONE			0

//
// SN added builtin functions
//
int   SNSetCallback(char* pFuncName);
int   SNHookMessage(int message);
int   SNUnhookMessage(int message);
int   SNBindKey(int keycode, int keydata, char* pstatement);
int   SNSetstdout(int ttychannel);

int   SNTxtSetPos(int xpos, int ypos);
int   SNTxtHome(void);
int   SNTxtClrEol(void);
int   SNTxtClrEop(void);
int   SNTxtSetConsole(int width, int height);
int   SNTxtSetTTY(int buffersize);

char* SNEvaluate(snval_t* result, int unitnum, char* pexpstr);
int   SNGetMemory(void* buffaddr, int unitnum, SNADDR addr, unsigned int count);
int   SNSetMemory(void* buffaddr, int unitnum, SNADDR addr, unsigned int count);
int   SNLoadElf(int unit, char* pfilename, char* pcmdline, int flags);
int   SNLoadBin(int unit, char* pfname, SNADDR address);
int   SNSaveBin(int unit, char* pfname, SNADDR address, unsigned int count);
int   SNStart(int unit);
int   SNStop(int unit);
int   SNSetBP(int unit, SNADDR addr);
int   SNClrBP(int unit, SNADDR addr);
int   SNIsRunning(int unit);
void  SNUpdateWindows(int mask, int unitnum);
void  SNRefreshAll(void);
int   SNSetTypeDisplay(char* ptypestring, char* ptemplate);
int   SNGetTypeDisplay(char* pdest, char* ptype, int index);
int   SNDelTypeHandlers(char* ptype);
int   SNSetTimer(int interval);

// GDI functions
int   SNGetWindowSize(int* width, int* height);
int   SNCreateBmp(int width, int height);
int   SNClearBmp(void);
int   SNSetPixel(int x, int y);
int   SNSetLine(int x0, int y0, int x1, int y1);
int   SNSetColor(int red, int green, int blue);
int	  SNRectangle(int x0, int y0, int x1, int y1);
int	  SNEllipse(int x0, int y0, int x1, int y1);
int	  SNPolygon(int count, int* points);
int	  SNTextOut(int x, int y, char* pstr);
int   SNSetFont(int size, int flags, char* pname);
int   SNEnumFonts(void);
int   SNSetGraphMode(int flags, int mode);

char* getcwd( char* buffer, int maxlen);
int   chdir( char* dirname);
int   ShellExecute(char* pverb, char* pfname, char* pparms, int showcmd);
unsigned int _bswapw(unsigned int num);	// byte swap a word
void  _bswap(void* ptr, int len);		// byte swap an arbitrary length item in a buffer

int   SNStep(int unit, int flags);
int   SNStepOver(int unit, int flags);
int   SNAddr2Line(int unit, SNADDR addr, char** pnamebuff, unsigned int* pline);
int   SNLine2Addr(int unit, SNADDR* paddr, char* pname, unsigned int line);

int   SNAddMenu(int panetype, char* menutext, char* submenutext, char* script);
int   SNGetMenuInfo(int panetype, int command, char* menutext, char* submenutext);
int   SNGetPaneInfo(unsigned int hwnd, struct paneinfo * pout);
int   SNGetPaneTitle(unsigned int hwnd, char* pout);
int   SNExit(int retval);			// abort the debugger (parameter currently ignored)
int   SNAddr2Label(int unit, SNADDR addr, char* pout, int* plen);
int   SNRun2Addr(int unit, SNADDR addr);
int   SNDelMenu(int panetype, char* menutext, char* submenutext);
int   SNFlushMemCache(int unitnum);
void  SNHaltRPC(int reason);
int   SNMessageBox( char* ptext, char* ptitle, unsigned int flags);
int   SNAddr2Func(int unit, unsigned int addr, char** pnamebuff, unsigned int* pstart, unsigned int* pend);
int   SNFindNearestLabel(int unit, unsigned int addr, char** pnamebuff, unsigned int* pprevaddr);
int   SNGetMenuID(int panetype, char* menutext, char* submenutext);
int   SNSetMenuItem(int panetype, int idx, char* menutext, int enable, int check );
//
// ShowWindow() Commands used by ShellExecute above
//
#define SW_HIDE             0
#define SW_NORMAL           1
#define SW_SHOWMINIMIZED    2
#define SW_SHOWMAXIMIZED    3
#define SW_MAXIMIZE         3
#define SW_SHOWNOACTIVATE   4
#define SW_SHOW             5
#define SW_MINIMIZE         6
#define SW_SHOWMINNOACTIVE  7
#define SW_SHOWNA           8
#define SW_RESTORE          9
#define SW_SHOWDEFAULT      10
#define SW_FORCEMINIMIZE    11

//
// SNMessageBox() Flags
//
#define MB_OK                       0x00000000L
#define MB_OKCANCEL                 0x00000001L
#define MB_ABORTRETRYIGNORE         0x00000002L
#define MB_YESNOCANCEL              0x00000003L
#define MB_YESNO                    0x00000004L
#define MB_RETRYCANCEL              0x00000005L
#define MB_ICONHAND                 0x00000010L
#define MB_ICONQUESTION             0x00000020L
#define MB_ICONEXCLAMATION          0x00000030L
#define MB_ICONASTERISK             0x00000040L


/* flags values for SNSetGraphMode */
#define STRETCHON	0x01
#define GRAPHON		0x02
#define TEXTON		0x04

/* SNSetGraphMode: combine modes for graph + text in script pane */
/* dest = text pane, src = bitmap                                */
#define SRCCOPY		0		/* dest = source                     */
#define SRCPAINT	1		/* dest = source OR dest             */
#define SRCAND		2		/* dest = source AND dest            */
#define SRCINVERT	3		/* dest = source XOR dest            */
#define SRCERASE	4		/* dest = source AND (NOT dest )     */
#define NOTSRCCOPY	5		/* dest = (NOT source)               */
#define NOTSRCERASE	6		/* dest = (NOT src) AND (NOT dest)   */
#define MERGECOPY	7		/* dest = (source AND pattern)       */
#define MERGEPAINT	8		/* dest = (NOT source) OR dest       */
#define PATCOPY		9		/* dest = pattern                    */
#define PATPAINT	10		/* dest = DPSnoo                     */
#define PATINVERT	11		/* dest = pattern XOR dest           */
#define DSTINVERT	12		/* dest = (NOT dest)                 */
#define BLACKNESS	13		/* dest = BLACK                      */
#define WHITENESS	14		/* dest = WHITE                      */

#endif


#endif	// #ifdef _SNSCRIPT_H