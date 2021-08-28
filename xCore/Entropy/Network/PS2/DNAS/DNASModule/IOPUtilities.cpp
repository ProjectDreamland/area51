//==============================================================================
//
//  IOPUtilities.cpp
//
//  Copyright (c) 2002-2004 Inevitable Entertainment Inc.  All rights reserved.
//
//==============================================================================
// This contains the interface functions to call the equivalent in the main code
// DNAS library body. This will allow us to NOT use any system libraries in the
// overlay.
//
#include "x_types.hpp"
#include "x_debug.hpp"
#include "IOPUtilities.hpp"

#include "eekernel.h"
#include "sifrpc.h"
#include "sifdev.h"
#include "sifcmd.h"
#include "libcdvd.h"
#include "libpad.h"
#include "stdio.h"
#include "sys/stat.h"
#include "Network/ps2/dnas/dnas_private.hpp"

u32 __sce_sema_id=0;

dnas_jump_table JumpTable;

extern "C"
{

byte s_MemBuffer[32768];
s32  s_MemIndex = 0;

void* malloc ( size_t size )
{
    void* pData;
    size = (size + 15) &~15;

    pData = s_MemBuffer+s_MemIndex;
    s_MemIndex += size;
    if( s_MemIndex > (s32)sizeof(s_MemBuffer) )
    {
        BREAK;
    }

    return pData;
}

void free( void* ptr )
{
    (void)ptr;
}

int CreateThread( struct ThreadParam* p)
{
    return JumpTable.ifc_CreateThread( p );
}

int StartThread( int t, void* a )
{
    return JumpTable.ifc_StartThread( t, a );
}

int DeleteThread( int t )
{
    return JumpTable.ifc_DeleteThread( t );
}

int TerminateThread( int t )
{
    return JumpTable.ifc_TerminateThread( t );
}

int DelayThread( unsigned int t )
{
    return JumpTable.ifc_DelayThread( t );
}

int GetThreadId( void )
{
    return JumpTable.ifc_GetThreadId();
}

int ReferThreadStatus( int t, ThreadParam* p )
{
    return JumpTable.ifc_ReferThreadStatus( t, p );
}

int CreateSema( struct SemaParam* p )
{
    return JumpTable.ifc_CreateSema( p );
}

int SignalSema( int s )
{
    return JumpTable.ifc_SignalSema( s );
}

int iSignalSema( int s )
{
    return JumpTable.ifc_iSignalSema( s );
}

int WaitSema( int s )
{
    return JumpTable.ifc_WaitSema( s );
}

int PollSema( int s )
{
    return JumpTable.ifc_PollSema( s );
}

int DeleteSema( int s )
{
    return JumpTable.ifc_DeleteSema( s );
}

int sceSifInitIopHeap( void )
{
    return JumpTable.ifc_sceSifInitIopHeap();
}

void* sceSifAllocIopHeap( unsigned int s )
{
    return JumpTable.ifc_sceSifAllocIopHeap( s );
}

int sceSifFreeIopHeap( void* ptr )
{
    return JumpTable.ifc_sceSifFreeIopHeap( ptr );
}

int sceSifAddRebootNotifyHandler( unsigned int n, sceSifRebootNotifyHandler h, void* p )
{
    return JumpTable.ifc_sceSifAddRebootNotifyHandler( n, h, p );
}

int sceSifRemoveRebootNotifyHandler( unsigned int n )
{
    return JumpTable.ifc_sceSifRemoveRebootNotifyHandler( n );
}

int sceSifRebootIop( const char* i )
{
    return JumpTable.ifc_sceSifRebootIop( i );
}

int sceSifLoadModuleBuffer( const void* b, int a, const char* p )
{
    return JumpTable.ifc_sceSifLoadModuleBuffer( b, a, p );
}

int sceSifStopModule( int m, int a, const char* p, int* r )
{
    return JumpTable.ifc_sceSifStopModule( m, a, p, r );
}

int sceSifUnloadModule( int m )
{
    return JumpTable.ifc_sceSifUnloadModule( m );
}

int sceSifSearchModuleByName( const char* n )
{
    return JumpTable.ifc_sceSifSearchModuleByName( n );
}

void sceSifInitRpc( unsigned int p )
{
    JumpTable.ifc_sceSifInitRpc( p );
}

int sceSifBindRpc( sceSifClientData* c, unsigned int a, unsigned int b )
{
    return JumpTable.ifc_sceSifBindRpc( c, a, b );
}

int sceSifCallRpc( sceSifClientData* c, unsigned int f1, unsigned int f2, void* ind, int ins, void* outd, int outs, sceSifEndFunc f, void* pd )
{
    return JumpTable.ifc_sceSifCallRpc( c, f1, f2, ind, ins, outd, outs, f, pd );
}

void sceSifAddCmdHandler( unsigned int a, sceSifCmdHandler b, void* c )
{
    return JumpTable.ifc_sceSifAddCmdHandler( a, b, c );
}

void sceSifRemoveCmdHandler( unsigned int h )
{
    JumpTable.ifc_sceSifRemoveCmdHandler( h );
}

unsigned int sceSifSendCmd( unsigned int a, void* b, int c, void* d, void* e, int f )
{
    return JumpTable.ifc_sceSifSendCmd( a, b, c, d, e, f );
}

unsigned int sceSifSetDma( sceSifDmaData* d, int l )
{
    return JumpTable.ifc_sceSifSetDma( d, l );
}

int sceSifDmaStat( unsigned int i )
{
    return JumpTable.ifc_sceSifDmaStat( i );
}

void sceSifWriteBackDCache( const void* b, int l )
{
    JumpTable.ifc_sceSifWriteBackDCache( b, l );
}

unsigned int sceSifGetSreg( int r )
{
    return JumpTable.ifc_sceSifGetSreg( r );
}

unsigned int sceSifSetSreg( int r, unsigned int v )
{
    return JumpTable.ifc_sceSifSetSreg( r, v );
}

int sceOpen( const char* n, int f, ... )
{
    return JumpTable.ifc_sceOpen( n, f, 0 );
} 

int sceClose( int h )
{
    return JumpTable.ifc_sceClose( h );
}

int sceRead( int f, void* d, int n )
{
    return JumpTable.ifc_sceRead( f, d, n );
}

int sceWrite( int f, const void* d, int n )
{
    return JumpTable.ifc_sceWrite( f, d, n );
}

int sceLseek( int f, int o, int w )
{
    return JumpTable.ifc_sceLseek( f, o, w );
}

void scePrintf( const char* f, ... )
{
    va_list a;

    va_start( a, f );

    JumpTable.ifc_scePrintf( f, a );
}

int open( const char* pName, int m, ... )
{
    va_list a;

    va_start( a, m );
    return JumpTable.ifc_sceOpen( pName, m, va_arg( a, int ) );
}

int close( int f )
{
    return JumpTable.ifc_sceClose( f );
}

int read( int f, void* d, unsigned int l )
{
    return JumpTable.ifc_sceRead( f, d, l );
}

int write( int f, const void* d, unsigned int l )
{
    return JumpTable.ifc_sceWrite( f, d, l );
}

int lseek( int f, long p, int w )
{
    return JumpTable.ifc_sceLseek( f, p, w );
}

int fstat( int, struct stat* )
{
    BREAK;
    return 0;
}

int stat( const char*, struct stat* )
{
    BREAK;
    return 0;
}

void* sbrk( unsigned int )
{
    BREAK;
    return NULL;
}

int getpid( void )
{
    return JumpTable.ifc_GetThreadId();
}

int SetTimerAlarm( unsigned long c, unsigned long(*h) (int, unsigned long, unsigned long, void*, void* ), void* d)
{
    return JumpTable.ifc_SetTimerAlarm( c, h, d );
}

int ReleaseTimerAlarm( int t )
{
    return JumpTable.ifc_ReleaseTimerAlarm( t );
}

unsigned long TimerUSec2BusClock( unsigned int a, unsigned int b )
{
    return JumpTable.ifc_TimerUSec2BusClock( a, b );
}

int EIntr( void )
{
    return JumpTable.ifc_EIntr();
}

int DIntr( void )
{
    return JumpTable.ifc_DIntr();
}

void FlushCache( int i )
{
    return JumpTable.ifc_FlushCache( i );
}

int sceCdReadClock( sceCdCLOCK* r )
{
    return JumpTable.ifc_sceCdReadClock( r );
}


} // end of extern "C"

int ReferSemaStatus( int SemaId, struct SemaParam* pParam )
{
    return JumpTable.ifc_ReferSemaStatus(SemaId, pParam);
}

int sceSnprintf(char *buffer, size_t count, const char *fmt, ...)
{
    va_list   Args;
    va_start( Args, fmt );

    return JumpTable.ifc_sceVsnprintf( buffer, count, fmt, Args );
}

