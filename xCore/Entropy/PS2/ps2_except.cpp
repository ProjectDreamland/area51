#include <eekernel.h>
#include <libdma.h>
#include <libgraph.h>
#include <libpkt.h>
#include <sifrpc.h>
#include <sifdev.h>
#include <libexcep.h>
#include "x_files.hpp"
#include "x_debug.hpp"
#include "entropy.hpp"
#include "ps2/ps2_except.hpp"
#include "ps2/ps2_exceptiondefs.hpp"
#include "ps2/iopmanager.hpp"
#include "../../Apps/GameApp/Config.hpp"


#if !defined(CONFIG_RETAIL) && (!CONFIG_IS_DEMO)
#define CATCHER_IS_ENABLED
#endif

#if defined( CATCHER_IS_ENABLED )

//#include "netlib/netlib.hpp"
//#include "../../demo1/specialversion.hpp"

#define SCREEN_WIDTH 384
#define SCREEN_HEIGHT 224
#define OFFX    (((4096-SCREEN_WIDTH)/2)<<4)
#define OFFY    (((4096-SCREEN_HEIGHT)/2)<<4)
#define MAXWIDTH 60
#define MAXHEIGHT 27
#define MAX_IGNORE_ENTRIES 20

#define ALLOW_CONTINUE
#define UNCACHED (0x20000000)

const char *ExceptionCode[]=
{
    "Interrupt",
    "TLB modification",
    "TLB on load or instr fetch",
    "TLB on store",
    "Address error on load or instr fetch",
    "Address error on store",
    "Bus error on load or store",
    "Syscall",
    "Breakpoint",
    "Reserved instruction",
    "Coprocessor unusable",
    "Arithmetic overflow",
    "Trap",
    "Unknown cause"
};

typedef u_long128 u128;
extern u32 __text_obj[];
extern u32 __text_objend[];
u32     gStackBase,gStackLimit;
u32     gCodeBase,gCodeLimit;
extern  xbool   PAL_Mode;
static s32 s_TimerTick;
//static s32 s_ExceptionSendState;

sceExcepIOPExceptionData    IopExceptionData    PS2_ALIGNMENT(64);
sceGsDBuff                  db                  PS2_ALIGNMENT(64);
u_long                      giftagAD[2]         PS2_ALIGNMENT(64);
long128                     Vifbuff[10]         PS2_ALIGNMENT(64);
sceVif1Packet               VifPkt              PS2_ALIGNMENT(64);

except_write    ExceptionBuffer;

#ifdef ALLOW_CONTINUE
static struct 
{
    const char  *pFilename;
    s32         Line;
} IgnoreList[MAX_IGNORE_ENTRIES];
static s32 IgnoreIndex;
#endif

void    *OldHandlers[14];

void    except_ee_Handler(u32 stat,u32 cause,u32 epc,u32 bva,u32 pba,u128 *gpr);
void    except_iop_Handler(void *p,void *d);
xbool   except_assert_Handler( const char* pFileName,
                      s32         LineNumber,
                      const char* pExprString,
                      const char* pMessageString );
//void except_Log_Handler(const char *pString);
//void exception_Send(void);


void except_Init(void)
{
    char    path[128];
    s32     i;

    // CJ: We are not loading the IOP exception handler
    //g_IopManager.LoadModule( "panicsys.irx" );

#ifdef TARGET_DEV
    x_sprintf(path,"host0:%s/3rdParty/PS2/Sony/IOP/Modules/panicsys.irx",XCORE_PATH);
#else
    x_sprintf(path,"cdrom0:\\MODULES\\PANICSYS.IRX;1");
#endif
    x_strtoupper(&path[5]);

    // Set the handlers
    for (i=1;i<14;i++)
    {
        if ( (i!=8) && (i!=9) )
            OldHandlers[i] = SetDebugHandler( i, except_ee_Handler );
    }

    // CJ: We are not loading the IOP exception handler
    //sceExcepSetDebugIOPHandler(path,except_iop_Handler, &IopExceptionData);

    gStackBase = (u32)__text_obj;
#ifdef TARGET_DEV
    gStackLimit = (u32)128 * 1048576;
#else
    gStackLimit = (u32)32 * 1048576;
#endif
    gCodeBase = (u32)__text_obj;
    gCodeLimit = (u32)__text_objend;
    //x_SetRTFHandler(except_assert_Handler);
//    x_SetLogHandler(except_Log_Handler);
#ifdef ALLOW_CONTINUE
    IgnoreIndex = 0;
#endif

}

void except_Kill(void)
{
    ASSERT(FALSE);
}


extern int vSyncCallback(int);

void InitGS(void)
{
    sceDmaEnv env;
    sceDmaChan *p1;
    s32 i;
    s32 width,height;

    // give the previous dma a chance to complete
    for (i=0;i<10;i++)
    {
        sceGsSyncV(0);
    }
    s_TimerTick = 0;
    sceGsResetPath();
    sceDmaReset(1);
    giftagAD[0] = SCE_GIF_SET_TAG(0, 1, 0, 0, 0, 1);
    giftagAD[1] = 0x000000000000000eL;

    sceVif1PkInit(&VifPkt, (u_long128 *)((u_int)Vifbuff | UNCACHED) );

    sceDmaGetEnv(&env);
    env.notify = 1<<SCE_DMA_VIF1;
    sceDmaPutEnv(&env);

    p1 = sceDmaGetChan(SCE_DMA_VIF1);
    p1->chcr.TTE = 1;

    eng_GetRes(width,height);
    if (height > 448)
        sceGsResetGraph(0, SCE_GS_INTERLACE, SCE_GS_PAL, SCE_GS_FRAME);
    else
        sceGsResetGraph(0, SCE_GS_INTERLACE, SCE_GS_NTSC, SCE_GS_FRAME);
    sceGsSetDefDBuff(&db, SCE_GS_PSMCT32, SCREEN_WIDTH, SCREEN_HEIGHT,
	       SCE_GS_ZGEQUAL, SCE_GS_PSMZ24, SCE_GS_CLEAR);

    sceGsSyncVCallback(NULL);

    sceVif1PkReset(&VifPkt);
    sceVif1PkCnt(&VifPkt, 0);
    sceVif1PkOpenDirectCode(&VifPkt, 0);
    sceVif1PkOpenGifTag(&VifPkt, *(u_long128*)&giftagAD);

    // add alpha environment packet
    sceVif1PkReserve(&VifPkt,
	       sceGsSetDefAlphaEnv((sceGsAlphaEnv *)VifPkt.pCurrent,0)
	       * 4);
    sceVif1PkCloseGifTag(&VifPkt);
    sceVif1PkCloseDirectCode(&VifPkt);
    sceVif1PkEnd(&VifPkt, 0);
    sceVif1PkTerminate(&VifPkt);

        iFlushCache( 0 );
    // kick Gs initialize packet
    sceDmaSend(p1,(long128*)(((u_int)(VifPkt.pBase)) & 0x8fffffff));
    // wait for Gs initialize packet end
    sceGsSyncPath(0,0);
    while(!sceGsSyncV(0));	// display next in odd field when interlace
}

void InitExceptionBuffer(exception_type type)
{
    u64 Hours,Minutes,Seconds, Milliseconds;

//    x_debug_crash_fn *pCrashFunction;

    Milliseconds = x_GetTime() / x_GetTicksPerMs();
    Seconds = Milliseconds / 1000;
    Minutes = Seconds / 60;
    Hours = Minutes / 60;

    Milliseconds = Milliseconds % 1000;
    Seconds = Seconds % 60;
    Minutes = Minutes % 60;

    ExceptionBuffer.Data.Type = type;
    ExceptionBuffer.Data.Time[0] = Hours;
    ExceptionBuffer.Data.Time[1] = Minutes;
    ExceptionBuffer.Data.Time[2] = Seconds;
    ExceptionBuffer.Data.Time[3] = Milliseconds;
    x_memset(ExceptionBuffer.Data.DebugVersion,0,sizeof(ExceptionBuffer.Data.DebugVersion));
    x_memset(ExceptionBuffer.Data.DebugMessage,0,sizeof(ExceptionBuffer.Data.DebugMessage));

    x_strncpy(ExceptionBuffer.Data.DebugVersion,x_DebugGetVersionString(),sizeof(ExceptionBuffer.Data.DebugVersion));
//    pCrashFunction = x_DebugGetCrashFunction();
//    if (pCrashFunction)
//    {
//        pCrashFunction(ExceptionBuffer.Data.DebugMessage,sizeof(ExceptionBuffer.Data.DebugMessage));
//    }
}

void    MakeRawStackDump(void *sp,u32 *pBuffer,u32 count)
{
    u32 i;
    u32 *stackptr;

    stackptr = (u32 *)sp;

    for (i=0;i<count;i++)
    {
        if ((u32)stackptr >= gStackLimit-4)
        {
            *pBuffer++ = 0xdeadbeef;
        }
        else
        {
            *pBuffer++ = *stackptr++;
        }
    }
}

void    MakeCookedStackDump(void *sp,u32 *pBuffer,u32 count)
{
    u32     i;
    u32     val;
    u32     *ptr;

    ptr = (u32 *)sp;

    for (i=0;i<count;i++)
    {
        while (1)
        {
            if ((u32)ptr >= gStackLimit)
            {
                val = 0xdeadbeef;
                break;
            }
            val = *ptr++;
            if ( (val >= gCodeBase) && (val <= gCodeLimit) )
                break;
        }
        *pBuffer++=val;
    }
}

void    DumpStack(void *sp,u32 *Raw,u32 *Cooked)
{
    s32 j,k;

    sceExcepConsPrintf("   *** RAW STACK DUMP FROM %08x ***\n",sp);
    for (j=0;j<4;j++)
    {
        for (k=0;k<4;k++)
        {
            sceExcepConsPrintf("0x%08x ",*Raw++);
        }
        sceExcepConsPrintf("\n");
    }

    sceExcepConsPrintf("\n  *** COOKED STACK DUMP FROM %08x ***\n",sp);

    for (j=0;j<3;j++)
    {
        for (k=0;k<4;k++)
        {
            sceExcepConsPrintf("0x%08x ",*Cooked++);
        }
        sceExcepConsPrintf("\n");
    }
}

extern xbool x_DebugGetCallStackPS2( s32& CallStackDepth, u32*& pCallStack, u32* ra, u32* sp );

void DumpCallStack( u32* ra, u32* sp )
{
    s32     CallStackDepth;
    u32*    pCallStack;
    x_DebugGetCallStackPS2( CallStackDepth, pCallStack, ra, sp );

    sceExcepConsPrintf( "  ** CALLSTACK **\n" );
    for( s32 j=0; j<14; j++ )
    {
        if( CallStackDepth-- > 0 )
            sceExcepConsPrintf( "0x%08x\n", *pCallStack++ );
    }
}

extern const char* x_DebugGetCause( void );

void    except_ee_Handler(u32 stat,u32 cause,u32 epc,u32 bva,u32 bpa,u128 *gpr)
{
    s32 j,i;

    // We need to do the "conversion" from 128 bit to 32 bit by using a union
    // because the compiler complains on the attempt to convert from 128 to 32 bit
    union
    {
        u128 u128bit;
        u32  u32bit[4];
    } split;

    InitExceptionBuffer(EXCEPT_EE);
    ExceptionBuffer.Data.EE.stat = stat;
    ExceptionBuffer.Data.EE.cause = cause;
    ExceptionBuffer.Data.EE.epc = epc;
    ExceptionBuffer.Data.EE.bva = bva;
    ExceptionBuffer.Data.EE.bpa = bpa;
    for (i=0;i<32;i++)
    {
        split.u128bit = gpr[i];
        ExceptionBuffer.Data.EE.gpr[i] = split.u32bit[0];
    }
    // Dump valid return addresses on stack (if stack in range)
    split.u128bit = gpr[GPR_sp];

    MakeRawStackDump((void *)split.u32bit[0],ExceptionBuffer.Data.EE.RawStack,32);
    MakeCookedStackDump((void *)split.u32bit[0],ExceptionBuffer.Data.EE.CookedStack,32);

    int frame=0;
    InitGS();

    db.clear0.rgbaq.R = 0;
    db.clear0.rgbaq.G = 25;
    db.clear0.rgbaq.B = 102;

    db.clear1.rgbaq.R = 0;
    db.clear1.rgbaq.G = 25;
    db.clear1.rgbaq.B = 102;
    sceExcepConsOpen(OFFX + (16<<4), OFFY + (16<<3),MAXWIDTH ,MAXHEIGHT );

    while(1)
    {

        j= (cause>>2)&31;
        if (j>13)
            j=13;

        sceExcepConsLocate( 0, 0 );
        // Dump cause information
        sceExcepConsPrintf("  ** %s **\n",ExceptionCode[j]);
        sceExcepConsPrintf(" TIME: %02d:%02d:%02d.%03d\n",ExceptionBuffer.Data.Time[0],ExceptionBuffer.Data.Time[1],ExceptionBuffer.Data.Time[2],ExceptionBuffer.Data.Time[3]);

        if( x_strlen(x_DebugGetCause()) > 0 )
        {
            sceExcepConsPrintf("\n");
            sceExcepConsPrintf("CAUSE: %s\n",x_DebugGetCause());
            sceExcepConsPrintf("\n");
        }
        else
        {
            extern s32 g_Changelist;
            sceExcepConsPrintf("Changelist# %d\n", g_Changelist );
            sceExcepConsPrintf("pc=0x%08x ra=0x%08x sp=0x%08x\n",epc, gpr[GPR_ra], gpr[GPR_sp]);
            sceExcepConsPrintf("gp=0x%08x fp=0x%08x sr=0x%08x\n",gpr[GPR_gp], gpr[GPR_fp], gpr[IOP_SR]);
            sceExcepConsPrintf("\n");
/*
            // Dump register list
            sceExcepConsPrintf("at=0x%08x v0=0x%08x v1=0x%08x\n",gpr[GPR_at],gpr[GPR_v0],gpr[GPR_v1]);
            sceExcepConsPrintf("a0=0x%08x a1=0x%08x a2=0x%08x\n",gpr[GPR_a0],gpr[GPR_a1],gpr[GPR_a2]);
            sceExcepConsPrintf("a3=0x%08x t0=0x%08x t1=0x%08x\n",gpr[GPR_a3],gpr[GPR_t0],gpr[GPR_t1]);
            sceExcepConsPrintf("t2=0x%08x t3=0x%08x t4=0x%08x\n",gpr[GPR_t2],gpr[GPR_t3],gpr[GPR_t4]);
            sceExcepConsPrintf("t5=0x%08x t6=0x%08x t7=0x%08x\n",gpr[GPR_t5],gpr[GPR_t6],gpr[GPR_t7]);
            sceExcepConsPrintf("s0=0x%08x s1=0x%08x s2=0x%08x\n",gpr[GPR_s0],gpr[GPR_s1],gpr[GPR_s2]);
            sceExcepConsPrintf("s3=0x%08x s4=0x%08x s5=0x%08x\n",gpr[GPR_s3],gpr[GPR_s4],gpr[GPR_s5]);
            sceExcepConsPrintf("s6=0x%08x s7=0x%08x t8=0x%08x\n",gpr[GPR_s6],gpr[GPR_s7],gpr[GPR_t8]);
            sceExcepConsPrintf("t9=0x%08x k0=0x%08x k1=0x%08x\n",gpr[GPR_t9],gpr[GPR_k0],gpr[GPR_k1]);
            sceExcepConsPrintf("\n");
*/
        }

//        DumpStack((void *)split.u32bit[0],ExceptionBuffer.Data.EE.RawStack,ExceptionBuffer.Data.EE.CookedStack);
        DumpCallStack( (u32*)epc, (u32*)gpr[GPR_sp] );
//        DumpCallStack( (u32*)gpr[GPR_ra], (u32*)gpr[GPR_sp] );

//        exception_Send();
        if( frame & 1 )
          {	// interrace half pixcel adjust
	    sceGsSetHalfOffset( &db.draw1, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
          } else {
	    sceGsSetHalfOffset( &db.draw0, 2048, 2048, sceGsSyncV( 0 )  ^ 0x01 );
          }
        iFlushCache( 0 );
        sceGsSyncPath( 0, 0 );
        sceGsSwapDBuff( &db, (frame++) & 0x01 );

    }
}


void    except_iop_Handler(void *p,void *d)
{
    (void)p;
    (void)d;
    int frame=0;

    InitExceptionBuffer(EXCEPT_IOP);

    x_memcpy(ExceptionBuffer.Data.Iop.gpr,IopExceptionData.reg,EXCEPT_REGISTER_COUNT * sizeof(u32));
    x_strcpy(ExceptionBuffer.Data.Iop.module,IopExceptionData.module);
    ExceptionBuffer.Data.Iop.offset = IopExceptionData.offset;
    ExceptionBuffer.Data.Iop.version = IopExceptionData.version;
    x_strcpy(ExceptionBuffer.Data.DebugVersion,x_DebugGetVersionString());

    InitGS();

    db.clear0.rgbaq.R = 0x00;
    db.clear0.rgbaq.G = 0x40;
    db.clear0.rgbaq.B = 0x00;

    db.clear1.rgbaq.R = 0x00;
    db.clear1.rgbaq.G = 0x40;
    db.clear1.rgbaq.B = 0x00;
    sceExcepConsOpen(OFFX + (16<<4), OFFY + (16<<3),MAXWIDTH ,MAXHEIGHT );

    while(1)
    {
        sceExcepConsLocate( 0, 0 );
        sceExcepConsPrintf("          *** IOP EXCEPTION ***\n\n");
        sceExcepConsPrintf("TIME: %02d:%02d:%02d.%03d\n",ExceptionBuffer.Data.Time[0],ExceptionBuffer.Data.Time[1],ExceptionBuffer.Data.Time[2],ExceptionBuffer.Data.Time[3]);
        sceExcepConsPrintf("cause = 0x%08x, %s\n",IopExceptionData.reg[IOP_CAUSE],ExceptionCode[(IopExceptionData.reg[IOP_CAUSE]>>2)&31]);
        sceExcepConsPrintf("Module %s Version %02x.%02x\n",IopExceptionData.module ,(IopExceptionData.version)>>8 ,(IopExceptionData.version)&0xf);
        sceExcepConsPrintf("Offset 0x%08x, badvaddr=0x%08x\n\n",IopExceptionData.offset,IopExceptionData.reg[IOP_BADVADDR]); 
        sceExcepConsPrintf("pc=0x%08x ra=0x%08x sp=0x%08x\n",IopExceptionData.reg[IOP_EPC], IopExceptionData.reg[GPR_ra], IopExceptionData.reg[GPR_sp]);
        sceExcepConsPrintf("gp=0x%08x fp=0x%08x sr=0x%08x\n",IopExceptionData.reg[GPR_gp], IopExceptionData.reg[GPR_fp], IopExceptionData.reg[IOP_SR]);
        sceExcepConsPrintf("\n");
        sceExcepConsPrintf("at=0x%08x v0=0x%08x v1=0x%08x\n",IopExceptionData.reg[GPR_at], IopExceptionData.reg[GPR_v0], IopExceptionData.reg[GPR_v1]);
        sceExcepConsPrintf("a0=0x%08x a1=0x%08x a2=0x%08x\n",IopExceptionData.reg[GPR_a0], IopExceptionData.reg[GPR_a1], IopExceptionData.reg[GPR_a2]);
        sceExcepConsPrintf("a3=0x%08x t0=0x%08x t1=0x%08x\n",IopExceptionData.reg[GPR_a3], IopExceptionData.reg[GPR_t0], IopExceptionData.reg[GPR_t1]);
        sceExcepConsPrintf("t2=0x%08x t3=0x%08x t4=0x%08x\n",IopExceptionData.reg[GPR_t2], IopExceptionData.reg[GPR_t3], IopExceptionData.reg[GPR_t4]);
        sceExcepConsPrintf("t5=0x%08x t6=0x%08x t7=0x%08x\n",IopExceptionData.reg[GPR_t5], IopExceptionData.reg[GPR_t6], IopExceptionData.reg[GPR_t7]);
        sceExcepConsPrintf("s0=0x%08x s1=0x%08x s2=0x%08x\n",IopExceptionData.reg[GPR_s0], IopExceptionData.reg[GPR_s1], IopExceptionData.reg[GPR_s2]);
        sceExcepConsPrintf("s3=0x%08x s4=0x%08x s5=0x%08x\n",IopExceptionData.reg[GPR_s3], IopExceptionData.reg[GPR_s4], IopExceptionData.reg[GPR_s5]);
        sceExcepConsPrintf("s6=0x%08x s7=0x%08x t8=0x%08x\n",IopExceptionData.reg[GPR_s6], IopExceptionData.reg[GPR_s7], IopExceptionData.reg[GPR_t8]);
        sceExcepConsPrintf("t9=0x%08x k0=0x%08x k1=0x%08x\n",IopExceptionData.reg[GPR_t9], IopExceptionData.reg[GPR_k0], IopExceptionData.reg[GPR_k1]);

//        exception_Send();
        if( frame & 1 )
          {	// interrace half pixcel adjust
	    sceGsSetHalfOffset( &db.draw1, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
          } else {
	    sceGsSetHalfOffset( &db.draw0, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
          }
        iFlushCache( 0 );
        sceGsSyncPath( 0, 0 );
        sceGsSwapDBuff( &db, frame++ );
    }
}

typedef	void (hndlr)(u_int, u_int, u_int, u_int, u_int, u_long128 *);

xbool   except_assert_Handler( const char* pFileName,
                      s32         LineNumber,
                      const char* pExprString,
                      const char* pMessageString )
{
    u32 frame=0;
    u32 stacktop;
    xbool retry;
    static char HourGlass[]=".o0O0o";

	if (pFileName==NULL)
	{
		pFileName = "<unknown>";
	}

    if (x_strlen(pFileName) > 37)
    {
        pFileName = pFileName+x_strlen(pFileName)-37;
    }

#ifdef ALLOW_CONTINUE
    s32 i;
    // If we're ignoring this assert, just return without doing anything
    for (i=0;i<IgnoreIndex;i++)
    {
        if ( (IgnoreList[i].pFilename == pFileName) && (IgnoreList[i].Line == LineNumber) )
        {
            return FALSE;
        }
    }
#endif

    InitExceptionBuffer(EXCEPT_ASSERT);

    ExceptionBuffer.Data.Assert.LineNumber = LineNumber;
    ExceptionBuffer.Data.Assert.StackPointer = (u32)(&stacktop)+0x24;
    x_strcpy(ExceptionBuffer.Data.Assert.Filename,pFileName);
	if (pExprString)
	{
		x_strcpy(ExceptionBuffer.Data.Assert.Expression,pExprString);
	}
	else
	{
		ExceptionBuffer.Data.Assert.Expression[0]=0x0;
	}
    MakeRawStackDump((void *)((u32)(&stacktop)+0x24),ExceptionBuffer.Data.Assert.RawStack,32);
    MakeCookedStackDump((void *)((u32)(&stacktop)+0x24),ExceptionBuffer.Data.Assert.CookedStack,32);

    if (pMessageString)
    {
        x_strcpy(ExceptionBuffer.Data.Assert.Message,pMessageString);
    }
    else
    {
        ExceptionBuffer.Data.Assert.Message[0]=0x0;
    }

    InitGS();

    db.clear0.rgbaq.R = 0x00;
    db.clear0.rgbaq.G = 0x00;
    db.clear0.rgbaq.B = 0x40;

    db.clear1.rgbaq.R = 0x00;
    db.clear1.rgbaq.G = 0x00;
    db.clear1.rgbaq.B = 0x40;
    sceExcepConsOpen(OFFX + (16<<4), OFFY + (16<<3),MAXWIDTH ,MAXHEIGHT );
    retry = 0;

    while(1)
    {
        sceExcepConsLocate( 0, 0 );
        sceExcepConsPrintf("          *** ASSERT FAILURE ***\n\n");
        sceExcepConsPrintf("TIME: %02d:%02d:%02d.%03d\n",ExceptionBuffer.Data.Time[0],ExceptionBuffer.Data.Time[1],ExceptionBuffer.Data.Time[2],ExceptionBuffer.Data.Time[3]);
    
        sceExcepConsPrintf("FILE: %s\n",pFileName);
        sceExcepConsPrintf("LINE: %d\n",LineNumber);
		if (pExprString)
		{
			sceExcepConsPrintf("EXPR: %s\n",pExprString);
		}

        if (pMessageString)
        {
            sceExcepConsPrintf("MESG: %s\n",pMessageString);
            
        }

        sceExcepConsPrintf("\n");
//        DumpStack((void *)((u32)(&stacktop)+0x24),ExceptionBuffer.Data.Assert.RawStack,ExceptionBuffer.Data.Assert.CookedStack);
#ifdef ALLOW_CONTINUE
        sceExcepConsPrintf("\n\n\n [] - Halt, X - Continue, O - Suppress\n");
#endif
        sceExcepConsPrintf("%c",HourGlass[(frame>>3) % 6]);
//        exception_Send();
        if( frame & 1 )
          {	// interrace half pixcel adjust
	    sceGsSetHalfOffset( &db.draw1, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
          } else {
	    sceGsSetHalfOffset( &db.draw0, 2048, 2048, sceGsSyncV( 0 ) ^ 0x01 );
          }
        iFlushCache( 0 );
        sceGsSyncPath( 0, 0 );
        sceGsSwapDBuff( &db, frame++ );

#ifdef ALLOW_CONTINUE
        input_UpdateState();

        if (input_WasPressed(INPUT_PS2_BTN_SQUARE))         // Debug, restore vectors then return
        {
            //SetDebugHandler(9,(hndlr *)NULL);
            return TRUE;
        }

        if (input_WasPressed(INPUT_PS2_BTN_CROSS))          // Continue, try assert again
        {
            return FALSE;
 }

        if (input_WasPressed(INPUT_PS2_BTN_CIRCLE))         // Suppress, forget about the assertion and continue
        {
            //
            // Add in to ignore list
            //
            if (IgnoreIndex < MAX_IGNORE_ENTRIES)
            {
                IgnoreList[IgnoreIndex].pFilename = pFileName;
                IgnoreList[IgnoreIndex].Line      = LineNumber;
                IgnoreIndex++;
            }
            return FALSE;
        }
#endif
    }
    sceExcepConsClose();
    return retry;
}

/*
enum
{
    SENDSTATE_INIT=0,
    SENDSTATE_OPEN_PORT,
    SENDSTATE_START_CONNECT,
    SENDSTATE_WAIT_CONNECT,
    SENDSTATE_SEND_PACKET,
    SENDSTATE_WAIT_RESPONSE,
    SENDSTATE_COMPLETE,
    SENDSTATE_UNABLE_TO_CONNECT,
    SENDSTATE_NONETWORK,
};

static net_address              s_ExceptServer;                       // IP/Port of the server
static net_address              s_ExceptClient;                       // IP/Port of the client (us)
static except_find              s_FindRequest;
static except_find_reply        s_FindReply;

void exception_Send(void)
{
    s32         status;
    static s32  Delay;
    static s32  Retries;
    s32         Length;
    interface_info info;

    sceExcepConsLocate( 0, 25 );
    if (s_TimerTick == 0)
    {
        s_ExceptionSendState = SENDSTATE_INIT;
    }

    if (ExceptionBuffer.Data.Type == EXCEPT_IOP)
    {
        sceExcepConsPrintf("Unable to send data if IOP crashes");
        return;
    }

    switch (s_ExceptionSendState)
    {
    case SENDSTATE_INIT:
        sceExcepConsPrintf("Connect delay");
        if (s_TimerTick > 2 * 60)
        {
            net_Init();
            s_ExceptionSendState = SENDSTATE_OPEN_PORT;
        }
        break;
    case SENDSTATE_OPEN_PORT:
        sceExcepConsPrintf("Attempting to open a network port");
        net_GetInterfaceInfo(-1,info);
        if (info.Address == 0)
        {
            s_ExceptionSendState = SENDSTATE_NONETWORK;
            break;
        }
        net_Bind(s_ExceptClient);
        if (s_ExceptClient.IP == 0)
        {
            s_ExceptionSendState = SENDSTATE_UNABLE_TO_CONNECT;
        }
        s_ExceptionSendState = SENDSTATE_WAIT_CONNECT; 
        Delay = 0;
        Retries = 3;
        break;
    case SENDSTATE_UNABLE_TO_CONNECT:
        sceExcepConsPrintf("Unable to establish a connection");
        break;
    case SENDSTATE_WAIT_CONNECT:
        sceExcepConsPrintf("Connecting to crash logging server");
        if (Delay==0)
        {
            s_ExceptServer.IP = net_StrToIP(EXCEPT_LOG_SERVER);
            s_ExceptServer.Port = EXCEPT_LOG_PORT;
            s_FindRequest.Type    = EXCEPT_REQ_FIND;
            s_FindRequest.Address = s_ExceptClient.IP;
            s_FindRequest.Port    = s_ExceptClient.Port;
            net_Send(s_ExceptClient,s_ExceptServer,&s_FindRequest,sizeof(s_FindRequest));
        }
        status = net_Receive(s_ExceptClient,s_ExceptServer,&s_FindReply,Length);

        if (status)
        {
            Delay = 0;
            Retries = 0;
            s_ExceptionSendState = SENDSTATE_SEND_PACKET;
        }
        else
        {
            Delay++;
            if (Delay > 2*60)
            {
                Retries++;
                if (Retries > 5)
                {
                    s_ExceptionSendState = SENDSTATE_UNABLE_TO_CONNECT;
                }
                Delay = 0;
            }
        }
        break;
    case SENDSTATE_SEND_PACKET:
        sceExcepConsPrintf("Attempting to send data to the server");
        if (Delay==0)
        {
            s_ExceptServer.IP = net_StrToIP(EXCEPT_LOG_SERVER);
            s_ExceptServer.Port = EXCEPT_LOG_PORT;
            ExceptionBuffer.Type = EXCEPT_REQ_WRITE;
            ExceptionBuffer.Data.LocalIP = s_ExceptClient.IP;
            net_Send(s_ExceptClient,s_ExceptServer,&ExceptionBuffer,sizeof(ExceptionBuffer));
        }

        status = net_Receive(s_ExceptClient,s_ExceptServer,&IopExceptionData,Length);
        if (status)
        {
            s_ExceptionSendState = SENDSTATE_COMPLETE;
        }

        Delay++;
        if (Delay > 2 * 60)
        {
            Delay = 0;
            Retries++;
            if (Retries>5)
            {
                s_ExceptionSendState = SENDSTATE_UNABLE_TO_CONNECT;
            }
        }
        break;
    case SENDSTATE_COMPLETE:
        sceExcepConsPrintf("Sent packet to inevitable.com");
        break;
    case SENDSTATE_NONETWORK:
        sceExcepConsPrintf("No network connection available");
        break;
    default:
        break;
    }

    s_TimerTick++;
}


void except_Log_Handler(const char *pString)
{
    interface_info info;
    static net_address Address;
    net_address Dest;

    net_GetInterfaceInfo(-1,info);
    if (info.Address)
    {
        if (Address.IP == 0)
        {
            net_Bind(Address);
        }

        if (Address.IP)
        {
            InitExceptionBuffer(EXCEPT_LOG);
            ExceptionBuffer.Type = EXCEPT_REQ_WRITE;
            x_strcpy(ExceptionBuffer.Data.Log.LogMessage,pString);
            Dest.IP = net_StrToIP(EXCEPT_LOG_SERVER);
            Dest.Port = EXCEPT_LOG_PORT;
            ExceptionBuffer.Data.LocalIP = Address.IP;
            net_Send(Address,Dest,&ExceptionBuffer,sizeof(ExceptionBuffer));

        }
    }
    else
    {
        x_DebugMsg("Unable to send log event: %s\n",pString);
    }
}
*/
#else // defined( CATCHER_IS_ENABLED )

void exception_Handler(u32 ,u32 ,u32 ,u32 ,u32 ,u128 *)
{
    // Reboot if we crash
    eng_Reboot( REBOOT_QUIT );
}

void except_Init(void)
{
    s32     i;

    // Set the handlers
    for( i=1; i<14; i++ )
    {
        if ( (i!=8) && (i!=9) )
        {
            SetDebugHandler( i, exception_Handler );
        }
    }
}

void except_Kill( void )
{
}

#endif // !defined( CATCHER_IS_ENABLED )
