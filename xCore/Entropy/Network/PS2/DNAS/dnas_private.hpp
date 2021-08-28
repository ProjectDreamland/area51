#ifndef __DNAS_PRIVATE_HPP
#define __DNAS_PRIVATE_HPP

#include "sifdev.h"
#include "sifcmd.h"
#include "sifrpc.h"
#include "sif.h"
#include "stdarg.h"
#include "libcdvd.h"

struct dnas_jump_table
{
    int             (*ifc_CreateThread)                     ( struct ThreadParam* );
    int             (*ifc_StartThread)                      ( int, void* );
    int             (*ifc_DeleteThread)                     ( int );
    int             (*ifc_TerminateThread)                  ( int );
    int             (*ifc_DelayThread)                      ( unsigned int );
    int             (*ifc_GetThreadId)                      ( void );
    int             (*ifc_ReferThreadStatus)                ( int, struct ThreadParam* );
    
    int             (*ifc_CreateSema)                       ( struct SemaParam* );
    int             (*ifc_DeleteSema)                       ( int );
    int             (*ifc_SignalSema)                       ( int );
    int             (*ifc_iSignalSema)                      ( int );
    int             (*ifc_WaitSema)                         ( int );
    int             (*ifc_PollSema)                         ( int );
    int             (*ifc_ReferSemaStatus)                  ( int, struct SemaParam* );

    int             (*ifc_sceSifInitIopHeap)                ( void );
    void*           (*ifc_sceSifAllocIopHeap)               ( unsigned int );
    int             (*ifc_sceSifFreeIopHeap)                ( void* );

    int             (*ifc_sceSifAddRebootNotifyHandler)     ( unsigned int, sceSifRebootNotifyHandler, void* );
    int             (*ifc_sceSifRemoveRebootNotifyHandler)  ( unsigned int );

    int             (*ifc_sceSifRebootIop)                  ( const char* );
    int             (*ifc_sceSifLoadModuleBuffer)           ( const void*, int, const char* );
    int             (*ifc_sceSifStopModule)                 ( int, int, const char*, int* );
    int             (*ifc_sceSifUnloadModule)               ( int );
    int             (*ifc_sceSifSearchModuleByName)         ( const char* );

    void            (*ifc_sceSifInitRpc)                    ( unsigned int );
    int             (*ifc_sceSifBindRpc)                    ( sceSifClientData*, unsigned int, unsigned int );
    int             (*ifc_sceSifCallRpc)                    ( sceSifClientData*, unsigned int, unsigned int, void*, int, void*, int, sceSifEndFunc, void* );

    void            (*ifc_sceSifAddCmdHandler)              ( unsigned int, sceSifCmdHandler, void* );
    void            (*ifc_sceSifRemoveCmdHandler)           ( unsigned int );
    unsigned int    (*ifc_sceSifSendCmd)                    ( unsigned int, void*, int, void*, void*, int );
    unsigned int    (*ifc_sceSifSetDma)                     ( sceSifDmaData*, int );
    int             (*ifc_sceSifDmaStat)                    ( unsigned int );
    void            (*ifc_sceSifWriteBackDCache)            ( const void*, int );

    unsigned int    (*ifc_sceSifGetSreg)                    ( int );
    unsigned int    (*ifc_sceSifSetSreg)                    ( int, unsigned int );

    int             (*ifc_sceOpen)                          ( const char *, int , unsigned int );
    int             (*ifc_sceClose)                         ( int );
    int             (*ifc_sceRead)                          ( int, void*, int );
    int             (*ifc_sceWrite)                         ( int, const void*, int );
    int             (*ifc_sceLseek)                         ( int, int, int );
    int             (*ifc_scePrintf)                        ( const char*, va_list args );

    int             (*ifc_SetTimerAlarm)                    ( unsigned long, unsigned long (*)(int, unsigned long, unsigned long, void*, void* ), void* );
    int             (*ifc_ReleaseTimerAlarm)                ( int );
    unsigned long   (*ifc_TimerUSec2BusClock)               ( unsigned int, unsigned int );

    int             (*ifc_EIntr)                            ( void );
    int             (*ifc_DIntr)                            ( void );
    void            (*ifc_FlushCache)                       ( int );

    int             (*ifc_sceCdReadClock)                   ( sceCdCLOCK* );
    int             (*ifc_sceVsnprintf)                     ( char*, size_t, const char*, va_list );

};


#endif // __DNAS_PRIVATE_HPP