#include "x_files.hpp"
#include "x_threads.hpp"
#include "iopmanager.hpp"
#include "e_input.hpp"

#include "sifrpc.h"
#include "sifdev.h"
#include "sifcmd.h"
#include "libcdvd.h"
#include "libpad.h"
#include "libdbc.h"
#include "libmc2.h"
#include "Implementation/x_tool_private.hpp"
#include "../../Apps/GameApp/Config.hpp"

iop_manager g_IopManager;

#ifdef TARGET_DEV
//#define SULPHA_ENABLED
#endif

//#define ENABLE_HDD_SUPPORT

static const char* s_SearchPath[]=
{
#ifdef TARGET_DEV
#ifdef X_DEBUG
    "host:ps2-iop-debug/%s",
#else
    "host:ps2-iop-release/%s",
#endif
    "host:"XCORE_PATH"/3rdparty/ps2/Sony/iop/modules/%s",
    "host:"XCORE_PATH"/3rdparty/ps2/sn/iop/modules/%s",
    NULL
#else
    "cdrom0:\\MODULES\\%s;1",
    NULL
#endif
};

//-----------------------------------------------------------------------------
void iop_manager::Init(void)
{
    m_pPoweroffThread = NULL;
    Reboot( TRUE, TRUE, IOP_IMAGE_FILE );
}

//-----------------------------------------------------------------------------
void iop_manager::Kill(void)
{
    Reboot( FALSE );
}

//-----------------------------------------------------------------------------
void iop_manager::FindFile( char* pResult, const char* pFilename )
{
    const char**    pSearch;
    s32             Handle;

    pSearch = s_SearchPath;

    while( *pSearch )
    {
        x_sprintf( pResult, *pSearch, pFilename );
#ifdef TARGET_DVD
        x_strtoupper( &pResult[5] );
#endif
        Handle = sceOpen( pResult, SCE_RDONLY );
        if( Handle >= 0 )
        {
            sceClose( Handle );
            return;
        }
        pSearch++;
    }
    ASSERTS( FALSE, xfs("Unable to find file %s in IOP search paths.",pFilename) );
}

#if (!CONFIG_IS_DEMO)
xsema   *pPoweroffSemaphore;

static void PoweroffCallback(void*)
{
    // we should use sema_Signal but we can't since it's in the interrupt context
    // so we just directly signal it.
    pPoweroffSemaphore->Release(X_TH_INTERRUPT|X_TH_NOBLOCK);
}

xbool g_RebootPending;

static void PoweroffThread(void)
{
    s32 stat=0;
    xsema PoweroffSemaphore(1,0);

    pPoweroffSemaphore = &PoweroffSemaphore;

    sceCdPOffCallback( PoweroffCallback, NULL );
    // Lock the mutex for now, the interrupt callback routine will release it

    while( x_GetCurrentThread()->IsActive() )
    {
        pPoweroffSemaphore->Acquire();
        if( x_GetCurrentThread()->IsActive() == FALSE )
        {
            break;
        }
        scePrintf("<<<<POWEROFF>>>> START\n");
        xtimer t;

        t.Start();
        while( sceDevctl("dev9x:", DDIOC_OFF, NULL, 0, NULL, 0) < 0 )
        {
            if( t.ReadSec() > 5.0f )
                break;
        }

        while( !sceCdPowerOff(&stat)||stat );
    }
    sceCdPOffCallback( NULL, NULL );
}
#endif
//-----------------------------------------------------------------------------
extern void ShowSplash(void);

void iop_manager::Reboot( xbool LoadModules, xbool bShowSplash, const char* pKernelName )
{
    xtimer  Timeout;
    char    ImageName[128];

    // Reboot the IO processor
    // First, using the search paths, find the irx module to load. We have hardcoded this
    // at the moment
    if( pKernelName == NULL )
    {
        pKernelName = IOP_IMAGE_FILE;
    }

    sceDbcEnd();
    sceSifInitRpc( 0 );

    FindFile( ImageName, pKernelName );

    Timeout.Reset();
    Timeout.Start();

    if( m_pPoweroffThread )
    {
        delete m_pPoweroffThread;
        m_pPoweroffThread = NULL;
    }
#if !defined(TARGET_DEV)
    sceCdInit( SCECdEXIT );
    sceSifExitCmd();
    while( !sceSifRebootIop( ImageName ) )
    {
        if( Timeout.ReadSec() > 5.0f )
        {
            ASSERTS( Timeout.ReadSec() < 5.0f, "Unable to reboot the IOP" );
        }
    }

    Timeout.Reset();
    Timeout.Start();
    while( !sceSifSyncIop() )
    {
        if( Timeout.ReadSec() > 5.0f )
        {
            ASSERTS( Timeout.ReadSec() < 5.0f, "Unable to reboot the IOP" );
        }
    }
#endif

    sceSifInitRpc( 0 );

    sceCdMmode(SCECdDVD);
#if !defined(TARGET_DEV)
    sceFsReset();
    sceSifLoadFileReset();
#endif
    sceSifInitIopHeap();
    sceCdDiskReady(0);

    if( bShowSplash )
    {
        ShowSplash();
    }

    if( LoadModules )
    {
        // Sulpha is a app used to debug SPU memory, register, etc.. therefore it should only be enabled in debug mode.
#ifdef SULPHA_ENABLED    
        LoadModule("SulphaSound.irx");
        LoadModule("SulphaComm.irx");
#else
        LoadModule("libsd.irx");
#endif
        LoadModule("sio2man.irx");
        LoadModule("dbcman.irx");
        LoadModule("sio2d.irx");
        LoadModule("iopdrivr.irx");
    
#if (!CONFIG_IS_DEMO)
        // Load all the modules required for HDD support ahead of time. As soon as dev9.irx is loaded,
        // we MUST re-vector the poweroff callback.
        m_pPoweroffThread = new xthread(PoweroffThread,"Power off Handler",8192,8);
        xbool Dev9Present;
        Dev9Present = LoadModule("dev9.irx",NULL,0,TRUE);
#if defined(ENABLE_HDD_SUPPORT)
        if( Dev9Present )
        {
            Dev9Present = LoadModule("atad.irx",NULL,0,TRUE);
            if( Dev9Present )
            {
                const char pfsarg[] = "-m" "\0" "4" "\0" "-o" "\0" "10" "\0" "-n" "\0" "24";
                LoadModule("pfs.irx",pfsarg,sizeof(pfsarg),TRUE);
            }
        }
#endif

#endif
        // this module is for the new controller library - don't load if using padman.irx
        //LoadModule("pad2\\ds2u_d.irx");
    
        LoadModule("mc2\\mc2_d.irx");

        sceDbcInit();
        sceMc2Init(0);

        // this module should only be loaded for the old controller library
        LoadModule("padman.irx");
    }

}

//-----------------------------------------------------------------------------
s32 iop_manager::LoadModule(const char* pFilename,const char* Params, s32 ParamLength, xbool AllowFail)
{
    char            path[256];
    int             retcode;

    m_Lock.Acquire();
    FindFile(path, pFilename );

    retcode = sceSifLoadModule( path, ParamLength, Params );
    m_Lock.Release();

    if( (retcode<0) && AllowFail )
    {
        return FALSE;
    }

    ASSERTS(retcode >= 0, xfs("Unable to load module %s for reason %d(%s)",pFilename, retcode, KernelError(retcode)) );
    return retcode;
}

//-----------------------------------------------------------------------------
void iop_manager::UnloadModule(s32 id)
{
    if( id >= 0 )
    {
        s32     result;
        s32     status      = 0;
        xtimer  StopTimer;

        (void)status;

        x_DebugMsg("Attempting to stop module id %d\n",id);
        
        StopTimer.Start();

        m_Lock.Acquire();
        while( StopTimer.ReadMs() < 15000.0f )
        {
            status = sceSifStopModule(id,0,0,&result);
            if( status>=0 )
                break;
            x_DelayThread(100);
        }
        ASSERTS(status>=0,"Unable to stop module");
        status = sceSifUnloadModule(id);
        ASSERTS(status>=0,"Unable to unload module");
        m_Lock.Release();
    }

    LOG_MESSAGE("iop_manager::UnloadModule","Id:%d, Free:%d, Largest:%d", id, sceSifQueryTotalFreeMemSize(), sceSifQueryMaxFreeMemSize() );
}

//-----------------------------------------------------------------------------
const char* iop_manager::KernelError(s32 ErrorCode)
{
    switch( ErrorCode )
    {
    case -200:
        return "KE_LINKERR";
        break;
    case -201:
        return "KE_ILLEGAL_OBJECT";
        break;
    case -202:
        return "KE_UNKNOWN_MODULE";
        break;
    case -203:
        return "KE_NOFILE";
        break;
    case -204:
        return "KE_FILEERR";
        break;
    case -205:
        return "KE_NO_MEMORY";
        break;
    case -400:
        return "KE_NOMEM";
    case -65540:
        return "Wrong Devkit Flash version";
        break;
    default:
        break;
    }
    return "<unknown>";
}

