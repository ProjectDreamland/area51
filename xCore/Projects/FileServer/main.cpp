#include "x_files.hpp"
#include "e_Network.hpp"
#include "iomanager/device_net/io_device_net.hpp"

#include "windows.h"

u8  ReceiveBuffer[MAX_PACKET_SIZE];

typedef struct s_open_file
{
    struct      s_open_file *pNext;
    xtimer      ReadTimer;
    s32         BytesRead;
    s32         BytesWritten;
    s32         LastSequence;

    char        Filename[128];
    HANDLE      Handle;
} open_file;

open_file *FileList=NULL;


static const char* MaxLength( const char* pString, s32 Length )
{
    if( x_strlen(pString) > Length )
    {
        return pString+x_strlen(pString)-32;
    }
    else
    {
        return pString;
    }
}

void AddOpenFile(open_file *pOpenFile)
{
    pOpenFile->pNext = FileList;
    pOpenFile->ReadTimer.Reset();
    pOpenFile->ReadTimer.Start();
    pOpenFile->BytesRead    = 0;
    pOpenFile->BytesWritten = 0;
    pOpenFile->LastSequence = -1;
    FileList = pOpenFile;
}

void DeleteOpenFile (open_file *pOpenFile)
{
    if (pOpenFile == FileList)
    {
        FileList = pOpenFile->pNext;
    }
    else
    {
        open_file *pPrev;
        pPrev = FileList;
        while (pPrev->pNext != pOpenFile)
        {
            pPrev = pPrev->pNext;
            ASSERT(pPrev);
        }
        pPrev->pNext = pOpenFile->pNext;
    }
}

open_file *FindOpenFile(s32 Handle)
{
    open_file *pOpenFile;

    pOpenFile = FileList;

    while (pOpenFile)
    {
        if (pOpenFile->Handle == (X_FILE *)Handle)
            return pOpenFile;
        pOpenFile = pOpenFile->pNext;
    }
    return NULL;
}

int main( int argc, char** argv )
{

    net_socket          Server;
    net_address         Client;
    net_address         ClientAddress;
    interface_info      info;
    s32                 Length;
    xbool               status;
    fileserver_request  Request;
    fileserver_reply    Reply;
    open_file           *pOpenFile;
    s32                 count;
    char                *pMode;
    char                ClientName[128];
    xbool               ReadEnabled;
    xbool               WriteEnabled;
    xbool               LogReads;
    xbool               LogWrites;
    const char*         pArgs[32];
    s32                 ArgCount;
    net_address         LastRespondent;
    s32                 LookupsFromLastRespondent;

    x_Init(argc,argv);
    net_Init();

    x_DebugMsg("server_manager: Waiting for IP address to be assigned...\n");
    while( TRUE )
    {
        net_GetInterfaceInfo(0,info);
        if( info.Address != 0 )
        {
            break;
        }
        x_DelayThread(10);
    }
    x_DebugMsg("server_manager: Interface attached, IP = %d\n",info.Address);

    ReadEnabled     = TRUE;
    WriteEnabled    = TRUE;
    LogReads        = FALSE;
    LogWrites       = FALSE;
    
    ArgCount = 0;

    // Get rid of app name from arglist.
    argc--;
    argv++;

    while( argc )
    {
        const char* pArg;

        pArg = argv[0];

        if( *pArg=='-' )
        {
            pArg++;
            if( x_stricmp(pArg,"disablewrite")==0 )
            {
                WriteEnabled = FALSE;
                x_printf("**** WRITE DISABLED ****\n");
            }
            else if( x_stricmp(pArg,"disableread")==0 )
            {
                ReadEnabled = FALSE;
                x_printf("**** READ DISABLED ****\n");
            }
            else if( x_stricmp( pArg, "logread" ) == 0 )
            {
                LogReads = TRUE;
                x_printf( "**** LOGGING READS ****\n" );
            }
            else if( x_stricmp( pArg, "logwrite" ) == 0 )
            {
                LogWrites = TRUE;
                x_printf( "**** LOGGING WRITES ****\n" );
            }
            else
            {
                x_printf( "Unknown command line option -'%s'\n", pArg );
                x_printf( "Usage: <ip of target> [-disableread|-disablewrite|-logread|-logwrite]\n" );
            }
        }
        else
        {
            pArgs[ArgCount++]=argv[0];
        }
        argc--;
        argv++;
    }

    if( ArgCount )
    {
        x_strcpy(ClientName,pArgs[0]);
        ClientAddress = net_address(ClientName, 0);
        if( ClientAddress.IsEmpty() )
        {
            x_DebugMsg("Warning: Cannot resolve IP address of client in command line args, will respond to anyone.\n");
        }
    }
    else
    {
        BYTE    Data[128];
        DWORD   Length;
        HKEY    hKey;
        s32     status;

        status = RegOpenKeyEx(  HKEY_CURRENT_USER,
                                "Software\\Microsoft\\XBoxSDK",
                                NULL,
                                KEY_QUERY_VALUE,
                                &hKey);
        if( status==0 )
        {
            Length = sizeof(Data);
            RegQueryValueEx(hKey,
                            "XBoxName",
                            NULL,
                            NULL,
                            Data,
                            &Length);
            x_DebugMsg("RegQueryValueEx returned %s\n",Data);
            RegCloseKey(hKey);
        }
        else
        {
            x_DebugMsg("Unable to open registry key");
        }

        ClientAddress.Clear();
        x_strcpy(ClientName,"anyone");
    }
    Server.Bind(FILESERVER_PORT,NET_FLAGS_BROADCAST|NET_FLAGS_BLOCKING);
    DWORD   AccessMode;
    DWORD   ShareMode;
    DWORD   CreateMode;

    x_printf("Local address %s, waiting for connections from %s.\n",Server.GetStrAddress(),ClientName);
    LastRespondent.Clear();
    LookupsFromLastRespondent = 0;
    while( TRUE )
    {
        status = Server.Receive(Client,&Request,Length);
        if( status )
        {
            //x_DebugMsg("Received: Type=%d, client=%s,Length=%d\n",Request.Header.Type, Client.GetStrIP(), Length);
            switch( Request.Header.Type )
            {
            case FSRV_REQ_FIND:
                x_printf("A client at address %s is looking for a server.\n",Client.GetStrIP());
                Length = 0;

                // If we have specified a client, respond immediately.
                if( Client.GetIP() == ClientAddress.GetIP() )
                {
                    Reply.Find.Address  = Server.GetIP();
                    Reply.Find.Port     = Server.GetPort();
                    Length = sizeof(Reply.Find);
                }

                // If no client has been specified, then we respond to that client performing a lookup request
                // but only if it has sent 3 lookup requests. This will reduce the number of false responses
                // should more than one file server be running on the local network (since, you should really
                // always specify which console you are serving.
                if( ClientAddress.IsEmpty() )
                {
                    if( LastRespondent.GetIP() == ClientAddress.GetIP() )
                    {
                        LookupsFromLastRespondent++;
                        if( LookupsFromLastRespondent >= 3 )
                        {
                            Reply.Find.Address  = Server.GetIP();
                            Reply.Find.Port     = Server.GetPort();
                            Length              = sizeof(Reply.Find);
                        }
                    }
                    else
                    {
                        LastRespondent            = ClientAddress;
                        LookupsFromLastRespondent = 0;
                    }
                }
                break;
//----------------------------------------------------------------------------
            case FSRV_REQ_RESET:
                count = 0;
                while( FileList )
                {
                    CloseHandle(FileList->Handle);
                    DeleteOpenFile(FileList);
                    count++;
                }
                x_printf("Fileserver reset, closed %d files.\n",count);
                Length = sizeof(Reply.Reset);
                break;
//----------------------------------------------------------------------------
            case FSRV_REQ_OPEN:
                xbool skip;

                pOpenFile = (open_file *)x_malloc(sizeof(open_file));
                ASSERT(pOpenFile);
                x_printf("Open : %s, mode:%s,",MaxLength(Request.Open.Filename,24),Request.Open.Mode);
                x_strcpy(pOpenFile->Filename,Request.Open.Filename);

                pMode = Request.Open.Mode;
                skip = FALSE;
                CreateMode = 0;
                AccessMode = 0;
                ShareMode  = 0;

                while (*pMode)
                {
                    switch (*pMode)
                    {
                    case 'R':
                    case 'r':
                        if (!ReadEnabled)
                        {
                            skip = TRUE;
                        }
                        CreateMode   = OPEN_EXISTING;
                        ShareMode   |= FILE_SHARE_READ;
                        AccessMode  |= GENERIC_READ;  
                        break;
                    case 'W':
                    case 'w':
                        CreateMode   = CREATE_ALWAYS;
                        ShareMode   |= FILE_SHARE_WRITE;
                        AccessMode  |= GENERIC_WRITE;
                        if (!WriteEnabled)
                        {
                            skip = TRUE;
                        }
                        break;
                    case 'A':
                    case 'a':
                        CreateMode   = OPEN_ALWAYS;
                        ShareMode   |= FILE_SHARE_WRITE;
                        AccessMode  |= GENERIC_WRITE;
                        break;
                    default:
                        break;
                    }
                    pMode++;
                }

                if( skip )
                {
                    pOpenFile->Handle = NULL;
                }
                else
                {
                    pOpenFile->Handle = CreateFile( Request.Open.Filename, 
                                                    AccessMode, 
                                                    ShareMode,
                                                    0,
                                                    CreateMode,
                                                    FILE_ATTRIBUTE_NORMAL,
                                                    NULL );
                }

                if( pOpenFile->Handle == INVALID_HANDLE_VALUE )
                {
                    Reply.Open.Status = FSRV_ERR_NOTFOUND;
                    x_free(pOpenFile);
                    x_printf("Not found.\n");
                }
                else
                {
                    AddOpenFile(pOpenFile);
                    Reply.Open.Handle = (s32)pOpenFile->Handle;
                    Reply.Open.Length = GetFileSize( pOpenFile->Handle, NULL );
                    Reply.Open.Status = FSRV_ERR_OK;
                    x_printf("Handle: 0x%x,length:%d\n",pOpenFile->Handle,Reply.Open.Length);
                }
                Length = sizeof(Reply.Open);
                break;
//----------------------------------------------------------------------------
            case FSRV_REQ_CLOSE:
                pOpenFile = FindOpenFile(Request.Close.Handle);
                if( pOpenFile )
                {
                    f32         BytesPerSec;

                    pOpenFile->ReadTimer.Stop();
                    BytesPerSec = (f32)(pOpenFile->BytesRead / pOpenFile->ReadTimer.ReadSec());

                    x_printf("Close: %s, %2.2fKB/sec\n",MaxLength(pOpenFile->Filename,24), BytesPerSec / 1024.0f);
                    CloseHandle( pOpenFile->Handle );
                    DeleteOpenFile(pOpenFile);
                    x_free(pOpenFile);
                    Reply.Close.Status = FSRV_ERR_OK;
                }
                else
                {
                    x_printf("Close handle 0x%x - not open\n",Request.Close.Handle);
                    Reply.Close.Status = FSRV_ERR_NOTFOUND;
                }
                Length = sizeof(Reply.Close);
                break;
//----------------------------------------------------------------------------
            case FSRV_REQ_READ:
                pOpenFile = FindOpenFile(Request.Read.Handle);
                if( pOpenFile )
                {
                    DWORD Length;

                    SetFilePointer( pOpenFile->Handle, Request.Read.Position, NULL, FILE_BEGIN );
                    ReadFile( pOpenFile->Handle, Reply.Read.Data, Request.Read.Length, &Length, NULL );
                    Reply.Read.Length = (s32)Length;
                    pOpenFile->BytesRead += Request.Read.Length;
                    Reply.Read.Status = FSRV_ERR_OK;
                    if( LogReads )
                    {
                        x_printf("Read handle 0x%x, offset %d, length %d(%d)\n",Request.Read.Handle,Request.Read.Position,Request.Read.Length,Reply.Read.Length);
                    }
                }
                else
                {
                    x_printf("Read handle 0x%x, ---- Not open\n",Request.Read.Handle);
                    Reply.Read.Length = 0;
                    Reply.Read.Status = FSRV_ERR_NOTOPEN;
                }
                Length = sizeof(Reply.Read);
                break;
//----------------------------------------------------------------------------
            case FSRV_REQ_WRITE:
                pOpenFile = FindOpenFile(Request.Write.Handle);
                if( pOpenFile )
                {
                    DWORD   Length;
                    s32     Result;

                    if( Request.Header.Sequence == 0 )
                    {
                        pOpenFile->LastSequence = -1;
                    }
                    if( Request.Write.Position != pOpenFile->BytesWritten )
                    {
                        Result = TRUE;
                        Length = Request.Write.Length;
                    }
                    else
                    {
                        Result = WriteFile( pOpenFile->Handle, Request.Write.Data, Request.Write.Length, &Length, NULL );
                        if( Result )
                        {
                            pOpenFile->BytesWritten += Request.Write.Length;
                        }
                    }
                    pOpenFile->LastSequence = Request.Header.Sequence;

                    if( LogWrites )
                    {
                        if( Result == FALSE )
                        {
                            Result = GetLastError();
                        }
                        x_printf("Write handle 0x%x, offset %d, length %d, result:%d, written:%d\n",Request.Write.Handle,Request.Write.Position,Request.Write.Length, Result, Length);
                    }
                    if( Result )
                    {
                        Reply.Write.Length = (s32)Length;
                        Reply.Write.Status = FSRV_ERR_OK;
                    }
                    else
                    {
                        Reply.Write.Length = (s32)Length;
                        Reply.Write.Status = FSRV_ERR_NOTOPEN;
                    }
                }
                else
                {
                    x_printf("Write handle 0x%x, ---- Not open\n",Request.Write.Handle);
                    Reply.Write.Length = 0;
                    Reply.Write.Status = FSRV_ERR_NOTOPEN;
                }
                Length = sizeof(Reply.Write);
                break;
//----------------------------------------------------------------------------
             default:
                Reply.Close.Status = FSRV_ERR_BADCOMMAND;
                Length = sizeof(Reply.Close);
                x_DebugMsg("Invalid request type %d\n",Request.Header.Type);
                break;
            }
            if( Length )
            {
                Length = Length+sizeof(Reply.Header);
                Reply.Header.Sequence = Request.Header.Sequence;
                Server.Send(Client,&Reply,Length);
            }
//            x_DebugMsg("Sent: Client=%08x:%d, Length=%d,sequence = \n",Client.IP,Client.Port,Length,Reply.Header.Sequence);
        }
    }

    return 0;
}