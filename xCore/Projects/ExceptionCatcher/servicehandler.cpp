#include "x_types.hpp"
#include "entropy.hpp"
#include "e_threads.hpp"
#include <windows.h>
#include "stdio.h"

SERVICE_STATUS          MyServiceStatus; 
SERVICE_STATUS_HANDLE   MyServiceStatusHandle; 
s32 threadid;

VOID  WINAPI MyServiceStart (DWORD argc, LPTSTR *argv); 
VOID  WINAPI MyServiceCtrlHandler (DWORD opcode); 
DWORD MyServiceInitialization (DWORD argc, LPTSTR *argv, 
        DWORD *specificError); 
VOID SvcDebugOut(LPSTR String, DWORD Status) ;

int ExceptionMain( int argc, char** argv );
 
void main(s32,void **)
{ 
    ExceptionMain(0,NULL);
#if 0
    SERVICE_TABLE_ENTRY   DispatchTable[] = 
    { 
        { "MyService", MyServiceStart      }, 
        { NULL,              NULL          } 
    }; 
 
    if (!StartServiceCtrlDispatcher( DispatchTable)) 
    { 
       SvcDebugOut(" [MY_SERVICE] StartServiceCtrlDispatcher error = %d\n", GetLastError()); 
    }
#endif
} 
 
VOID SvcDebugOut(LPSTR String, DWORD Status) 
{ 
    CHAR  Buffer[1024]; 
    if (strlen(String) < 1000) 
    { 
        sprintf(Buffer, String, Status); 
        OutputDebugStringA(Buffer); 
    } 
} 

void WINAPI MyServiceStart (DWORD argc, LPTSTR *argv) 
{ 
    DWORD status; 
    DWORD specificError; 
 
    MyServiceStatus.dwServiceType        = SERVICE_WIN32; 
    MyServiceStatus.dwCurrentState       = SERVICE_START_PENDING; 
    MyServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | 
        SERVICE_ACCEPT_PAUSE_CONTINUE; 
    MyServiceStatus.dwWin32ExitCode      = 0; 
    MyServiceStatus.dwServiceSpecificExitCode = 0; 
    MyServiceStatus.dwCheckPoint         = 0; 
    MyServiceStatus.dwWaitHint           = 0; 
 
    MyServiceStatusHandle = RegisterServiceCtrlHandler( 
        "MyService", 
        MyServiceCtrlHandler); 
 
    if (MyServiceStatusHandle == (SERVICE_STATUS_HANDLE)0) 
    { 
        SvcDebugOut(" [MY_SERVICE] RegisterServiceCtrlHandler failed %d\n", GetLastError()); 
        return; 
    } 
 
    // Initialization code goes here. 
    status = MyServiceInitialization(argc,argv, &specificError); 
 
    // Handle error condition 
    if (status != NO_ERROR) 
    { 
        MyServiceStatus.dwCurrentState       = SERVICE_STOPPED; 
        MyServiceStatus.dwCheckPoint         = 0; 
        MyServiceStatus.dwWaitHint           = 0; 
        MyServiceStatus.dwWin32ExitCode      = status; 
        MyServiceStatus.dwServiceSpecificExitCode = specificError; 
 
        SetServiceStatus (MyServiceStatusHandle, &MyServiceStatus); 
        return; 
    } 
 
    // Initialization complete - report running status. 
    MyServiceStatus.dwCurrentState       = SERVICE_RUNNING; 
    MyServiceStatus.dwCheckPoint         = 0; 
    MyServiceStatus.dwWaitHint           = 0; 
 
    if (!SetServiceStatus (MyServiceStatusHandle, &MyServiceStatus)) 
    { 
        status = GetLastError(); 
        SvcDebugOut(" [MY_SERVICE] SetServiceStatus error %ld\n",status); 
    } 
 
    // This is where the service does its work. 
    SvcDebugOut(" [MY_SERVICE] Returning the Main Thread \n",0); 
 
    return; 
} 
 
// Stub initialization function. 
DWORD MyServiceInitialization(DWORD   argc, LPTSTR  *argv, 
    DWORD *specificError) 
{ 
//    threadid = eng_CreateThread(ServiceMain);
    argv; 
    argc; 
    specificError; 
    return(0); 
} 

VOID WINAPI MyServiceCtrlHandler (DWORD Opcode) 
{ 
    DWORD status; 
 
    switch(Opcode) 
    { 
        case SERVICE_CONTROL_PAUSE: 
        // Do whatever it takes to pause here. 
            MyServiceStatus.dwCurrentState = SERVICE_PAUSED; 
            break; 
 
        case SERVICE_CONTROL_CONTINUE: 
        // Do whatever it takes to continue here. 
            MyServiceStatus.dwCurrentState = SERVICE_RUNNING; 
            break; 
 
        case SERVICE_CONTROL_STOP: 
        // Do whatever it takes to stop here. 
            eng_DestroyThread(threadid);
            MyServiceStatus.dwWin32ExitCode = 0; 
            MyServiceStatus.dwCurrentState  = SERVICE_STOPPED; 
            MyServiceStatus.dwCheckPoint    = 0; 
            MyServiceStatus.dwWaitHint      = 0; 
 
            if (!SetServiceStatus (MyServiceStatusHandle, 
                &MyServiceStatus))
            { 
                status = GetLastError(); 
                SvcDebugOut(" [MY_SERVICE] SetServiceStatus error %ld\n",status); 
            } 
 
            SvcDebugOut(" [MY_SERVICE] Leaving MyService \n",0); 
            return; 
 
        case SERVICE_CONTROL_INTERROGATE: 
        // Fall through to send current status. 
            break; 
 
        default: 
            SvcDebugOut(" [MY_SERVICE] Unrecognized opcode %ld\n", 
                Opcode); 
    } 
 
    // Send current status. 
    if (!SetServiceStatus (MyServiceStatusHandle,  &MyServiceStatus)) 
    { 
        status = GetLastError(); 
        SvcDebugOut(" [MY_SERVICE] SetServiceStatus error %ld\n",status); 
    } 
    return; 
} 
