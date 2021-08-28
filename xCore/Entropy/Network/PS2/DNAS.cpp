//==============================================================================
//
//  DNAS.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================

#include "x_files.hpp"
#include "Entropy.hpp"
#include "LibSN.h"
#include "e_Network.hpp"
#include "dnas.hpp"
#include "sif.h"
#include "sifcmd.h"
#include "libcdvd.h"
#include "ps2/iopmanager.hpp"
#include "../Support/NetworkMgr/Downloader/Archive.hpp"

#if defined(bwatson)
//#define ENABLE_DNAS
#endif
#if defined(X_RETAIL) && !defined(CONFIG_PROFILE)
#define ENABLE_DNAS
#endif

int OpenDummy( const char* n, int m, unsigned int p )
{
    return sceOpen( n, m, p );
}

int localvprintf( const char* pformat, va_list args )
{
    x_DebugMsg("%s",(const char*)xvfs(pformat,args));
    return 0;
}

extern char g_FullPath[];

#if defined( ENABLE_DNAS )

dnas_jump_table dnas_JumpTable=
{
    CreateThread,
    StartThread,
    DeleteThread,
    TerminateThread,
    DelayThread,
    GetThreadId,
    ReferThreadStatus,

    CreateSema,
    DeleteSema,
    SignalSema,
    iSignalSema,
    WaitSema,
    PollSema,
    ReferSemaStatus,

    sceSifInitIopHeap,
    sceSifAllocIopHeap,
    sceSifFreeIopHeap,

    sceSifAddRebootNotifyHandler,
    sceSifRemoveRebootNotifyHandler,

    sceSifRebootIop,                  
    sceSifLoadModuleBuffer,           
    sceSifStopModule,                 
    sceSifUnloadModule,               
    sceSifSearchModuleByName,         

    sceSifInitRpc,                    
    sceSifBindRpc,                    
    sceSifCallRpc,                    

    sceSifAddCmdHandler,              
    sceSifRemoveCmdHandler,           
    sceSifSendCmd,                    
    sceSifSetDma,                     
    sceSifDmaStat,                    
    sceSifWriteBackDCache,            

    sceSifGetSreg,                    
    sceSifSetSreg,                    

    OpenDummy,                          
    sceClose,                         
    sceRead,                          
    sceWrite,                         
    sceLseek,                         
    localvprintf,                        

    SetTimerAlarm,                    
    ReleaseTimerAlarm,                
    TimerUSec2BusClock,               

    EIntr,                            
    DIntr,                            
    FlushCache,                       

    sceCdReadClock,
    sceVsnprintf,

};

#endif // defined( ENABLE_DNAS )

//------------------------------------------------------------------------------
s32 dnas_authenticate::Init( const char* pFilename )
{
#if defined(ENABLE_DNAS)
    s32         Status;
    X_FILE*     Handle;
    s32         Length;
    byte*       pAlignedBuffer;
    byte*       pArchiveBuffer;

    // Load up the SN DLL
    Status = snInitDllSystem(0);

    ASSERT( Status == 0 );
    x_DebugMsg("Loading overlay...\n");

    
    Handle = x_fopen( pFilename, "rb" );
    ASSERT(Handle);

    Length = x_flength(Handle);

    pArchiveBuffer = (byte*)x_malloc(Length);
    ASSERT( pArchiveBuffer );

    x_DebugMsg("Reading overlay, length:%d, dest:0x%08x...\n",Length, pArchiveBuffer );
    Status = x_fread( pArchiveBuffer, 1, Length, Handle );
    ASSERT( Status == Length );
    x_fclose(Handle);

    archive* pArchive = new archive;

    pArchive->Init( pArchiveBuffer, Length );
    ASSERT( pArchive->GetMemberCount()==1 );

    m_OverlayLength = pArchive->GetMemberLength( 0 );
    m_pOverlay = x_malloc(m_OverlayLength+127);
    ASSERT( m_pOverlay );
    pAlignedBuffer = (byte*)(((u32)m_pOverlay+127)&~127);

    x_memcpy( pAlignedBuffer, pArchive->GetMemberData(0), pArchive->GetMemberLength(0) );
    pArchive->Kill();
    delete pArchive;
    x_free( pArchiveBuffer );

    x_DebugMsg("Relocating dll..\n");
    FlushCache(0);
    Status = snDllLoaded(pAlignedBuffer,NULL);
    ASSERT( Status == 0 );

    x_DebugMsg("DLL is loaded.\n");

    m_pInitFunction         = (init_prototype*)         snDllGetFunctionAddress("dnas_Init");
    m_pUpdateFunction       = (update_prototype*)       snDllGetFunctionAddress("dnas_Update");
    m_pKillFunction         = (kill_prototype*)         snDllGetFunctionAddress("dnas_Kill");
    m_pInitAuthFunction     = (init_auth_prototype*)    snDllGetFunctionAddress("dnas_InitAuthentication");
    m_pKillAuthFunction     = (kill_auth_prototype*)    snDllGetFunctionAddress("dnas_KillAuthentication");
    m_pErrorLabelFunction   = (errorlabel_prototype*)   snDllGetFunctionAddress("dnas_GetErrorLabel");
    m_pGetStateFunction     = (getstate_prototype*)     snDllGetFunctionAddress("dnas_GetState");
    m_pGetUniqueFunction    = (getunique_prototype*)    snDllGetFunctionAddress("dnas_GetUniqueId");
    m_pEncryptFunction      = (encrypt_prototype*)      snDllGetFunctionAddress("dnas_Encrypt" );
    m_pDecryptFunction      = (decrypt_prototype*)      snDllGetFunctionAddress("dnas_Decrypt" );
    m_pEncryptLengthFunction= (enc_length_prototype*)   snDllGetFunctionAddress("dnas_GetEncryptedLength" );
    m_pDecryptLengthFunction= (dec_length_prototype*)   snDllGetFunctionAddress("dnas_GetDecryptedLength" );
    m_pInitEncryptionFunction= (init_enc_prototype*)    snDllGetFunctionAddress("dnas_InitEncryption");
    m_pKillEncryptionFunction= (kill_enc_prototype*)    snDllGetFunctionAddress("dnas_KillEncryption");

    if( sceSifSearchModuleByName("IOP_MSIF_rpc_interface") < 0 )
    {
        m_MSifHandle = g_IopManager.LoadModule("msifrpc.irx");
    }
    m_LibnetHandle  = g_IopManager.LoadModule("libnet.irx");

    //
    // If any of these asserts fail, check the symbols.export file to make sure that
    // symbol has been properly exported from the overlay.
    //
    ASSERT( m_pInitFunction );
    ASSERT( m_pUpdateFunction );
    ASSERT( m_pKillFunction );
    ASSERT( m_pErrorLabelFunction );
    ASSERT( m_pEncryptFunction );
    ASSERT( m_pDecryptFunction );
    ASSERT( m_pDecryptLengthFunction );
    ASSERT( m_pEncryptLengthFunction );
    ASSERT( m_pInitAuthFunction );
    ASSERT( m_pKillAuthFunction );
    ASSERT( m_pInitEncryptionFunction );
    ASSERT( m_pKillEncryptionFunction );

    x_DebugMsg("Calling Init function...\n");
    Status = m_pInitFunction( &dnas_JumpTable );
    x_DebugMsg("Init complete.\n");
    return Status;
#else
    (void)pFilename;
    return 0;
#endif
}

//------------------------------------------------------------------------------
void dnas_authenticate::InitAuthentication( dnas_init* pInit )
{
#if defined(ENABLE_DNAS)
    ASSERT( m_pInitAuthFunction );
    m_pInitAuthFunction( pInit );
#else
    (void)pInit;
#endif
}

//------------------------------------------------------------------------------
void dnas_authenticate::KillAuthentication( void )
{
#if defined(ENABLE_DNAS)
    ASSERT( m_pKillAuthFunction );
    m_pKillAuthFunction();
#endif
}

//------------------------------------------------------------------------------
s32 dnas_authenticate::Update( f32 DeltaTime,s32& Progress )
{
#if defined(ENABLE_DNAS)
    ASSERT( m_pUpdateFunction );
    return m_pUpdateFunction(DeltaTime, Progress);
#else
    (void)DeltaTime;
    (void)Progress;
    return DNAS_STATUS_OK;
#endif
}

//------------------------------------------------------------------------------
void dnas_authenticate::Kill( void )
{
#if defined(ENABLE_DNAS)
    byte*       pAlignedBuffer;

    ASSERT( m_pKillFunction );
    m_pKillFunction();
    m_pInitFunction         = NULL;
    m_pUpdateFunction       = NULL;
    m_pKillFunction         = NULL;
    m_pErrorLabelFunction   = NULL;
    m_pDecryptFunction      = NULL;
    m_pEncryptFunction      = NULL;

    ASSERT(m_pOverlay);

    pAlignedBuffer = (byte*)(((u32)m_pOverlay+127)&~127);

    x_DebugMsg("Calling Kill function...\n");
    snDllUnload( pAlignedBuffer );
    g_IopManager.UnloadModule(m_LibnetHandle);
    x_memset( m_pOverlay, 0xc9, m_OverlayLength );
    x_free( m_pOverlay );
    x_DebugMsg("Kill Complete.\n");
    m_pOverlay = NULL;
#else
#endif
}

//------------------------------------------------------------------------------
const char* dnas_authenticate::GetErrorLabel( s32 ErrorCode )
{
#if defined(ENABLE_DNAS)
    ASSERT( m_pErrorLabelFunction );
    return m_pErrorLabelFunction( ErrorCode );
#else
    (void)ErrorCode;
    return "IDS_NULL";
#endif
}

//------------------------------------------------------------------------------
void dnas_authenticate::GetUniqueId( byte* pBuffer, s32& Length )
{
#if defined(ENABLE_DNAS)
    m_pGetUniqueFunction( pBuffer, Length );
#else
    x_sprintf( (char*)pBuffer, "0x%08x", net_GetSystemId() );
    Length = x_strlen((char*)pBuffer);
#endif
}

void dnas_authenticate::InitEncryption( void )
{
#if defined(ENABLE_DNAS)
    ASSERT( m_pInitEncryptionFunction );
    m_pInitEncryptionFunction();
#endif
}

void dnas_authenticate::KillEncryption( void )
{
#if defined(ENABLE_DNAS)
    ASSERT( m_pKillEncryptionFunction );
    m_pKillEncryptionFunction();
#endif
}


//------------------------------------------------------------------------------
xbool dnas_authenticate::Encrypt( void* pData, s32 EncryptedLength, s32 DecryptedLength )
{
    return m_pEncryptFunction( pData, EncryptedLength, DecryptedLength );
}

//------------------------------------------------------------------------------
xbool dnas_authenticate::Decrypt( void* pData, s32 EncryptedLength, s32 DecryptedLength )
{
    return m_pDecryptFunction( pData, EncryptedLength, DecryptedLength );

}

//------------------------------------------------------------------------------
s32 dnas_authenticate::GetDecryptedLength( void* pData, s32 EncryptedLength )
{
    return m_pDecryptLengthFunction( pData, EncryptedLength );
}

//------------------------------------------------------------------------------
s32 dnas_authenticate::GetEncryptedLength( void* pData, s32 DecryptedLength )
{
    return m_pEncryptLengthFunction( pData, DecryptedLength );
}