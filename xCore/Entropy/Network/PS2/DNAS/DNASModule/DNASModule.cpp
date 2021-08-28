//==============================================================================
//
//  DNASModule.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
//
// We need to avoid using any x-files calls as we want to make this module as generic and
// independant as possible.
//
#include "x_types.hpp"
#include "x_debug.hpp"
#include "IOPUtilities.hpp"
#include "Network/ps2/dnas.hpp"
#include "dnas2.h"
#include "dnas2_inst_mc.h"
#include "com_error.h"
#include "eekernel.h"
#include "libcdvd.h"
#include "libmrpc.h"
#include "sifdev.h"
#include "sifcmd.h"
#include "sifrpc.h"
#include "stdio.h"
#include "libnet.h"

#define REQUEST_UNIQUEID

extern dnas_jump_table  JumpTable;
dnas_state              s_State;
char                    s_UniqueId[64] PS2_ALIGNMENT(64);
sceDNAS2UniqueID_t      s_Unique PS2_ALIGNMENT(64);
s32                     s_UniqueIdSize;
s32                     s_CategoryID;
xbool                   s_AuthenticationRunning = FALSE;

void dnas_memset(void* ptr, s32 Value, s32 Length )
{
    s32 i;
    byte* pData = (byte*)ptr;
    for( i=0; i<Length; i++ )
    {
        *pData++ = Value;
    }
}

//==============================================================================
extern "C" s32 dnas_Init( dnas_jump_table* pTable )
{
    JumpTable = *pTable;

    sceSifMInitRpc(0);

    s_State = IDLE;
    return 0;
}

extern "C" s32 dnas_InitAuthentication( dnas_init* pInit )
{
    sceDNAS2TitleAuthInfo_t AuthInfo;
    sceDNAS2TimeoutInfo_t   Timeout;
    s32                     Error;

#if defined(X_DEBUG)
    scePrintf( "******************** WARNING ******************\n" );
    scePrintf( "** THIS IS A DEBUG BUILD OF THE DNAS OVERLAY **\n" );
    scePrintf( "** THIS SHOULD NOT BE USED ON A FINAL RELEASE**\n" );
    scePrintf( "** BUILD FOR ANY REASON. PLEASE CHECK WITH   **\n" );
    scePrintf( "** BISCUIT IF YOU SEE THIS MESSAGE. HE NEEDS **\n" );
    scePrintf( "** TO BE SLAPPED AROUND FOR CHECKING THIS IN **\n" );
    scePrintf( "********* THIS IS VERY IMPORTANT! *************\n" );

#endif
    AuthInfo.auth_data.ptr      = (void*)pInit->pAuthData;
    AuthInfo.auth_data.size     = pInit->AuthLength;
    AuthInfo.pass_phrase.ptr    = (void*)pInit->pPassPhrase;
    AuthInfo.pass_phrase.size   = pInit->PassPhraseLength;
    s_CategoryID                = pInit->Category;
    Timeout.timeout     = 60;
    Timeout.priority    = 31;

    AuthInfo.line_type          = sceDNAS2_T_INET;
    sceDNAS2InitNoHDD( &AuthInfo, 32, 0, 0, &Timeout );
    s_UniqueIdSize  = sizeof(s_UniqueId);
    s_Unique.category = s_CategoryID;
    s_Unique.ptr      = s_UniqueId;
    s_Unique.sizep    = &s_UniqueIdSize;
    Error = sceDNAS2AuthGetUniqueID( &s_Unique );

    s_State = WAITING_AUTHENTICATION;
    s_AuthenticationRunning = TRUE;
    return Error;
}

sceDNAS2Status_t    Status PS2_ALIGNMENT(64);
//==============================================================================
extern "C" s32 dnas_Update( f32 DeltaTime, s32& Progress )
{
    s32                 Error;

    (void)DeltaTime;

    Progress = 0;
    Error = sceDNAS2GetStatus(&Status);
#if defined(X_DEBUG)
    static sceDNAS2Status_t OldStatus;
    if( OldStatus.progress != Status.progress )
    {
        scePrintf("dnas_Update: Error:%d, Progress:%d, Code:%d, SubCode:%d\n", Error, Status.progress, Status.code, Status.sub_code );
        OldStatus = Status;
    }
#endif
    if( Error < 0 )
    {
        return Error;
    }

    switch( s_State )
    {
        //---------------------------------------
    case IDLE:
        break;

        //---------------------------------------
    case WAITING_AUTHENTICATION:
        Progress = Status.progress;

        switch( Status.code )
        {
        case sceDNAS2_S_NETSTART:
            break;
        case sceDNAS2_S_END:
                s_State = IDLE;
            break;
        case sceDNAS2_S_NG:
        case sceDNAS2_S_COM_ERROR:
            s_State = IDLE;
            return Status.sub_code;
        default:
            break;
        }
        //
        // Still in progress. Just return current status.
        //
        return Status.code;
        break;
        //---------------------------------------
    default:
        break;
    }

    return Error;
}

//==============================================================================
extern "C" void dnas_KillAuthentication( void )
{
    sceDNAS2Status_t NewStatus;

    NewStatus.progress = 0;
    NewStatus.code = 0;
    sceDNAS2GetStatus( &NewStatus );
    s32 loopcount=0;
    while( NewStatus.progress == sceDNAS2_P_CONNECT )
    {
        DelayThread(32000);
        NewStatus.progress = 0;
        NewStatus.code = 0;
        sceDNAS2GetStatus( &NewStatus );
        loopcount++;
    }

    s32 result = sceDNAS2Abort();

    s32 idleloopcount=0;
    if( result != sceDNAS2_NOT_IN_COMM )
    {
        while( NewStatus.code != sceDNAS2_S_INIT ) 
        {
            DelayThread(1000);
            NewStatus.progress = 0;
            NewStatus.code = 0;
            sceDNAS2GetStatus( &NewStatus );
            idleloopcount++;
        }
    }
    s_State = IDLE;
    s32 shutdownresult;

    shutdownresult = sceDNAS2Shutdown();
#if defined(X_DEBUG)
    scePrintf("\ndnas_KillAuthentication: Delay loopcount:%d, idleloopcount:%d, sceDNAS2Abort returned %d, Shutdown returned %d\n\n", loopcount, idleloopcount, result, shutdownresult );
#endif
    s_AuthenticationRunning = FALSE;
}

//==============================================================================
extern "C" void dnas_Kill( void )
{
    if( s_AuthenticationRunning )
    {
        dnas_KillAuthentication();
    }
    sceSifMExitRpc();
    // --- THIS IS NOT UNLOADABLE --- iop_UnloadModule(msifrpc_handle);
}

//==============================================================================
extern "C" void dnas_GetUniqueId(byte* pBuffer, s32& Length )
{
    s32         IdLength;
    u8*         pUniqueId;
    byte*       pOut;
    const byte  HexChars[64+1]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_~";

    IdLength = s_UniqueIdSize;
    pUniqueId = (byte*)s_UniqueId;
    pOut = pBuffer;

    Length = 0;
    while( IdLength > 0 )
    {
        u32 Value;

        // Make a 24 bit value out of 3 bytes
        Value = (pUniqueId[0]<<16)|(pUniqueId[1]<<8)|pUniqueId[2];


        // Make 4 characters out of the 24 bit value using base64 style encoding.
        *pOut++ = HexChars[(Value>>18)&0x3f];
        *pOut++ = HexChars[(Value>>12)&0x3f];
        *pOut++ = HexChars[(Value>>6 )&0x3f];
        *pOut++ = HexChars[(Value>>0 )&0x3f];
        Length = Length + 4;
        IdLength  -= 3;
        pUniqueId += 3;
    }
}
//==============================================================================
extern "C" s32 dnas_GetDownloadedContent( void* pBase, s32& Length )
{
    (void)pBase;
    (void)Length;
    BREAK;
    return 0;
}

//==============================================================================
extern "C" s32 dnas_SetDownloadedContent( s32 ContentId )
{
    (void)ContentId;
    BREAK;
    return 0;
}

//==============================================================================
extern "C" const char* dnas_GetErrorLabel( s32 StatusCode )
{
    switch( StatusCode )
    {
    case sceDNAS2_OK:                       return "DNAS_OK";
    case sceDNAS2_SS_SERVER_BUSY:           return "IDS_ONLINE_DNAS_SERVER_BUSY";
    case sceDNAS2_SS_BEFORE_SERVICE:        return "IDS_ONLINE_DNAS_NOT_STARTED";
    case sceDNAS2_SS_OUT_OF_SERVICE:        return "IDS_ONLINE_DNAS_FINISHED";
    case sceDNAS2_SS_END_OF_SERVICE:        return "IDS_ONLINE_DNAS_END_OF_SERVICE";
    case sceDNAS2_SS_SESSION_TIME_OUT:      return "IDS_ONLINE_DNAS_TIMEOUT";
    case sceDNAS2_SS_INVALID_SERVER:        return "IDS_ONLINE_DNAS_INVALID_SERVER";
    case sceDNAS2_SS_INTERNAL_ERROR:        return "IDS_ONLINE_DNAS_INTERNAL_ERROR";
    case sceDNAS2_SS_EXTERNAL_ERROR:        return "IDS_ONLINE_DNAS_EXTERNAL_ERROR";
    case sceDNAS2_SS_INVALID_PS2:           return "IDS_ONLINE_DNAS_HARDWARE_ERROR";
    case sceDNAS2_SS_INVALID_MEDIA:         return "IDS_ONLINE_DNAS_DISC_ERROR";
    case sceDNAS2_SS_INVALID_AUTHDATA:      return "IDS_ONLINE_DNAS_AUTH_ERROR";
    case sceDNAS2_SS_INVALID_HDD_BINDING:   return "IDS_ONLINE_DNAS_MACHINE_ERROR";
    case sceDNAS2_ERR_GLUE_ABORT:           return "IDS_ONLINE_DNAS_CONNECTION_ABORTED";
    case sceDNAS2_ERR_NET_PROXY:            return "IDS_ONLINE_DNAS_PROXY_SETTING";
    case sceDNAS2_ERR_NET_TIMEOUT:          return "IDS_ONLINE_DNAS_NETWORK_TIMEOUT";
        //---------------------------------
    case sceDNAS2_ERR_NET_DNS_HOST_NOT_FOUND:
    case sceDNAS2_ERR_NET_DNS_NO_RECOVERY:
    case sceDNAS2_ERR_NET_DNS_NO_DATA:      return "IDS_ONLINE_DNAS_DNS_UNRESOLVED";
        //---------------------------------
    case sceDNAS2_ERR_NET_DNS_TRY_AGAIN:    return "IDS_ONLINE_DNAS_DNS_NO_RESPONSE";
    case sceDNAS2_ERR_NET_DNS_OTHERS:       return "IDS_ONLINE_DNAS_DNS_OTHER";
    case sceDNAS2_ERR_NET_ETIMEOUT:         return "IDS_ONLINE_DNAS_CONNECTION_TIMEOUT";
        //---------------------------------
    case sceDNAS2_ERR_NET_EISCONN:
    case sceDNAS2_ERR_NET_ECONNREFUSED:
    case sceDNAS2_ERR_NET_ENETUNREACH:
    case sceDNAS2_ERR_NET_ENOTCONN:
    case sceDNAS2_ERR_NET_ECONNRESET:
    case sceDNAS2_ERR_NET_SSL:              return "IDS_ONLINE_DNAS_SERVER_ERROR";
        //---------------------------------
    case sceDNAS2_ERR_NET_ENOBUFS:
    case sceDNAS2_ERR_NET_EMFILE:
    case sceDNAS2_ERR_NET_EBADF:
    case sceDNAS2_ERR_NET_EINVAL:
    case sceDNAS2_ERR_NET_OTHERS:           return "IDS_ONLINE_DNAS_CONNECTION_ERROR";

    default:                                return "IDS_ONLINE_DNAS_UNEXPECTED_ERROR";
    }
}

//==============================================================================
extern "C" dnas_state dnas_GetState( void )
{
    return s_State;
}

//==============================================================================
extern "C" s32  dnas_InitEncryption( void )
{
    return sceDNAS2InstInit_mc();
}

//==============================================================================
extern "C" void  dnas_KillEncryption( void )
{
    sceDNAS2InstShutdown_mc();
}

//==============================================================================
extern "C" s32   dnas_GetEncryptedLength( void* pData, s32 Length )
{
    u32 ReturnLength;
    s32 Result;

    Result = sceDNAS2InstPersonalizeDataLength_mc( Length, (byte*)pData, &ReturnLength );
    if( Result != 0 )
    {
        return Result;
    }
    return (s32)ReturnLength;
}

//==============================================================================
extern "C" s32   dnas_GetDecryptedLength( void* pData, s32 Length )
{
    u32 ReturnLength;
    s32 Result;

    Result = sceDNAS2InstExtractDataLength_mc( Length, (byte*)pData, &ReturnLength );
    if( Result != 0 )
    {
        return Result;
    }
    return (s32)ReturnLength;
}

//==============================================================================
extern "C" s32 dnas_Encrypt( void* pData, s32 SourceLength, s32 DestLength )
{
    s32 Result;

    Result = sceDNAS2InstPersonalizeData_mc( SourceLength, DestLength, (byte*)pData );

    return (Result==0);
}

//==============================================================================
extern "C" s32 dnas_Decrypt( void* pData, s32 SourceLength, s32 DestLength )
{
    s32 Result;

    Result = sceDNAS2InstExtractData_mc( SourceLength, DestLength, (byte*)pData );

    return (Result==0);
}
