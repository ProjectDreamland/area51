#include "x_target.hpp"
#if !defined(TARGET_PS2)
#error This is only for TARGET_PS2 builds. Please check your dependancies.
#endif
#include "ps2/IopManager.hpp"
#include "entropy.hpp"
#include "e_Memcard.hpp"
#include "..\..\3rdparty\PS2\Sony\EE\include\libmc2.h"
#include "..\..\3rdparty\PS2\Sony\EE\include\libmc.h"
#include "..\..\3rdparty\PS2\Sony\EE\include\libdbc.h"
#include "..\..\3rdparty\PS2\Sony\EE\include\libscf.h"

#if defined(TARGET_PS2)
#include "ps2\ps2_misc.hpp"
#endif

//==============================================================================
//=================================== Locals ===================================
//==============================================================================
static xbool s_Initialized      = FALSE; // FLAG: Has the Init function been called?
static xbool s_InitHardware     = TRUE;  // FLAG: Initialize hardware inside the Init funtcion?
static xbool s_DispatcherActive = FALSE; // FLAG: Is the dispatcher active? (prevents re-entry).


// Icon creation locals
static sceMcIconSys s_Icon;

extern char g_FullPath[];

// icon filename
static char* s_IconName     = "gray.icn";
#define SWITCH_LABEL(x) case x: return #x;

//==============================================================================
//=================== Hardware Specific Defines, Enums, etc ====================
//==============================================================================

// define directory name
//#ifdef X_PAL
//#define DIR_NAME	            "/BESLES-00000AREA51"
//#else
//#define DIR_NAME	            "/BASLUS-00000AREA51"
//#endif


// Maximum number of memory cards the system supports.
#define MAX_MEMCARDS            2 

// Size of the message queue.
#define MAX_MESSAGES            6 

// maximum number of files in a directory (sony limit)
//#define MEMCARD_DIR_ENTRIES     18
// this is now the maximum number of dirs that can be read from the card (not max in one directory)
#define MEMCARD_DIR_ENTRIES     64


// Valid messages for the Dispatcher.
enum memcard_message 
{
    MSG_INITIATE = 0,
    MSG_PROCESS,
    MSG_COMPLETE,
    MSG_RETRY,
    MSG_ERROR,
};

// Make DAMN sure that error_extensions do not 
// collide with the systems native error codes.
// On PS2, all error codes are <= 0.
enum error_extensions 
{
    EXT_CARD_RESULT_INCOMPATIBLE = 1,
    EXT_CARD_RESULT_REMOVED,
};

enum memcard_file_flags 
{
    MEMCARD_FILE_EXISTS       = (1<<0),
    MEMCARD_FILE_NOPERMISSION = (1<<1),
};

// Error mapping structure.
struct error_map 
{
    memcard_error AbstractError;
    s32           HardwareError;
};

#define SAVE_GAME_MAX_DATA_SIZE (1024 * 32)

#define CARD_NAME_LENGTH    32


typedef struct s_card_info
{
    s32     Location;
    s32     Capacity;
    s32     Used;
    s32     Free;
    s32     Type;
} card_info;

static xbool CheckError  ( s32 HardwareError, xbool bProcessOnNoError = FALSE );

static void  AsciiToSJIS ( const char *pTitleName, s16 *pSJISBuffer );
//static s32   MakeIcon    ( const char *pTitlename,  u32 socketID );

//==============================================================================
//========================== Hardware Specific Data ============================
//==============================================================================

static SceMc2DirParam           s_DirList[MEMCARD_DIR_ENTRIES];
static SceMc2DirParam           s_CardDir[MAX_MEMCARDS];
static SceMc2SocketParam        s_CardSocket[MAX_MEMCARDS];
static SceMc2InfoParam          s_CardInfo[MAX_MEMCARDS];
static s32                      s_nFiles;
static xarray<mc_file_info>     s_FileInfo;
static xarray<s32>              s_IgnoreList;
static volatile xbool           s_WasIgnored   = FALSE;
memcard_error                   s_IgnoredError = MEMCARD_SUCCESS;

static xbool                    s_IsConnected[MAX_MEMCARDS];
static s32                      s_CardSocketID[MAX_MEMCARDS];
static u_long128                s_CardDmaBuf[SCE_MC2_DMA_BUFFER_MAX] __attribute__((aligned (64)));



#define CARD_SEARCH_RESET       0
#define CARD_SEARCH_NEXT        1

#define CARD_SEEK_SET           0
#define CARD_SEEK_CUR           1
#define CARD_SEEK_END           2


#define CARD_ERR_OK             0  
#define CARD_ERR_NOTFOUND       -1 
#define CARD_ERR_FULL           -2 
#define CARD_ERR_DIRFULL        -3 
#define CARD_ERR_UNFORMATTED    -4 
#define CARD_ERR_UNPLUGGED      -5 
#define CARD_ERR_CHANGED        -6 
#define CARD_ERR_EXISTS         -7 
#define CARD_ERR_PROTECTED      -8 
#define CARD_ERR_WRONGTYPE      -9 
#define CARD_ERR_FATAL_ERROR    -10
#define CARD_ERR_BUSY           -11
#define CARD_ERR_BROKEN         -12
#define CARD_ERR_LIMIT          -13
#define CARD_ERR_NAMETOOLONG    -14
#define CARD_ERR_CANCELED       -15
#define CARD_ERR_INCOMPATIBLE   -16
#define CARD_ERR_NOSPACE        -17

static error_map s_ErrorMap[] = {
    { MEMCARD_SUCCESS,             CARD_ERR_OK               },   // Ready to start the next operation.
    { MEMCARD_FATAL_ERROR,         CARD_ERR_FATAL_ERROR      },   // Error due to program design (e.g., parameter range error, etc.)
    { MEMCARD_BUSY,                CARD_ERR_BUSY             },   // Busy
    { MEMCARD_NOT_A_MEMCARD,       CARD_ERR_WRONGTYPE        },   // A device is detected, but it is not a memory card. 
    { MEMCARD_NO_CARD,             CARD_ERR_UNPLUGGED        },   // Memory card is not detected (or not mounted yet).
    { MEMCARD_DAMAGED,             CARD_ERR_BROKEN           },   // File system is broken.
    { MEMCARD_FILE_NOT_FOUND,      CARD_ERR_NOTFOUND         },   // Specified file was not found.
    { MEMCARD_FILE_ALREADY_EXISTS, CARD_ERR_EXISTS           },   // The filename about to be created/renamed already exists. 
    { MEMCARD_NO_FILES_AVAILABLE,  CARD_ERR_DIRFULL          },   // No more free directory entries. 
    { MEMCARD_NOT_ENOUGH_SPACE,    CARD_ERR_NOSPACE          },   // Insufficient free space in data blocks. 
    { MEMCARD_FULL,                CARD_ERR_FULL             },   // The memory card is completely full
    { MEMCARD_ACCESS_DENIED,       CARD_ERR_PROTECTED        },   // No file access permission. 
    { MEMCARD_PAST_END_OF_FILE,    CARD_ERR_LIMIT            },   // Tried to read/write over the file size limit. 
    { MEMCARD_FILENAME_TOO_LONG,   CARD_ERR_NAMETOOLONG      },   // The filename about to be created/renamed is too long. 
    { MEMCARD_IO_CANCELED,         CARD_ERR_CANCELED         },   // The read/write operation is canceled.
    { MEMCARD_INCOMPATIBLE,        CARD_ERR_INCOMPATIBLE     },   // Extension - Nintendo frowns on non-8k sector cards.
    { MEMCARD_CARD_CHANGED,        CARD_ERR_CHANGED          },   // Memory card previously in slot was changed
    { MEMCARD_UNFORMATTED,         CARD_ERR_UNFORMATTED      },   // Card is unformatted
    { MEMCARD_END_OF_LIST,         0                         },
};




//==============================================================================
//============================= Hardware Functions =============================
//==============================================================================

//------------------------------------------------------------------------------
static s32 SonyToInternalError ( s32 errorCode )
{
    s32 internalError = 0;
    s32 sceError = 0;

    // check error code > 0 
    if ( errorCode > 0 )
    {
        // all ok
        return CARD_ERR_OK;
    }

    // convert error code
    sceError = SCE_ERROR_ERRNO(errorCode);
    
    switch ( sceError )
    {
    
    case SCE_OK:
        // Its all good
        internalError = CARD_ERR_OK;
        break;

    case SCE_ENEW_DEVICE:       
        // Memory card was swapped
        internalError = CARD_ERR_CHANGED;
        break;

    case SCE_ECONNREFUSED:      
        // Memory card access failed
        internalError = CARD_ERR_UNPLUGGED;
        break;

    case SCE_ENODEV:            
        // Memory card could not be detected
        internalError = CARD_ERR_UNPLUGGED;
        break;

    case SCE_EFORMAT:           
        // Memory card has not been formatted
        internalError = CARD_ERR_UNFORMATTED;
        break;            

    case SCE_EDEVICE_BROKEN:    // Memory card is damaged
        internalError = CARD_ERR_BROKEN;
        break;

    case SCE_EFILE_BROKEN:      // specified directory is corrupted    
        internalError = CARD_ERR_BROKEN;
        break;

    case SCE_EBUSY:           
        // busy
        internalError = CARD_ERR_BUSY;
        break;

    case SCE_ENOSPC:
        internalError = CARD_ERR_NOSPACE;
        break;

    case SCE_EACCES:   
        // file is write protected
        internalError = CARD_ERR_PROTECTED;
        break;

    case SCE_ENOTSUP:
        // memcard is not supported
        internalError = CARD_ERR_INCOMPATIBLE;
        break;

    case SCE_EID:             
        // socket number invalid
        internalError = CARD_ERR_FATAL_ERROR;
        break;

    case SCE_ENOENT:            // path does not exist
        internalError = CARD_ERR_NOTFOUND;   
        break;

    case SCE_ENOTDIR:           // path is not a directory
        internalError = CARD_ERR_NOTFOUND;   
        break;

    case SCE_EEXIST:            // directory or path name already exists
        internalError = CARD_ERR_EXISTS;
        break;

    case SCE_ENFILE:            // socket not created or no more files could be created
        internalError = CARD_ERR_DIRFULL;
        break;
        
    case SCE_ENAMETOOLONG:      // path name too long
        internalError = CARD_ERR_NAMETOOLONG;
        break;

    case SCE_ESEMA:             // semaphore creation failed
        // memcard system uninitialized
        internalError = CARD_ERR_FATAL_ERROR;
        break;

    case SCE_ETHREAD:           // thread creation failed
        // memcard system uninitialized
        internalError = CARD_ERR_FATAL_ERROR;
        break;

    case SCE_EMDEPTH:           // directory nesting is too deep            
        // memcard system uninitialized
        internalError = CARD_ERR_FATAL_ERROR;
        break;

    case SCE_ENOTEMPTY:         // directory not empty
        // memcard system uninitialized
        internalError = CARD_ERR_FATAL_ERROR;
        break;

    case SCE_EINIT:             // memory card system uninitialized
        // memcard system uninitialized
        internalError = CARD_ERR_FATAL_ERROR;
        break;

    case SCE_EINVAL:            // path name is invalid
        // memcard system uninitialized
        internalError = CARD_ERR_FATAL_ERROR;
        break;

    default:
        // unexpected error!
        internalError = CARD_ERR_FATAL_ERROR;
        ASSERT(0);        
        break;
    }

    return internalError;
}

//------------------------------------------------------------------------------

static void AddErrorToIgnoreList( s32 HardwareError )
{
    s_IgnoreList.Append();
    s_IgnoreList[ s_IgnoreList.GetCount()-1 ] = HardwareError;
}

//------------------------------------------------------------------------------

static void ClearIgnoreList( void )
{
    s_IgnoreList.Clear();
    s_WasIgnored = FALSE;
}

//------------------------------------------------------------------------------

static xbool ErrorIsIgnored( s32 HardwareError )
{
    return (s_IgnoreList.Find( HardwareError ) != -1);
}

//------------------------------------------------------------------------------

//static xbool ErrorWasIgnored( void )
//{
//    return s_WasIgnored;
//}

//------------------------------------------------------------------------------

//static void BailOnIgnoredError( void )
//{
//    // Was an error ignored?
//    if( s_WasIgnored )
//    {
//        // Set the error code.
//        g_MemcardHardware.SetState( s_IgnoredError );
//        
//        // Dear John...
//        g_MemcardHardware.SendMessage( MSG_ERROR );
//    }
//}

//------------------------------------------------------------------------------

static xbool CheckError( s32 HardwareError, xbool bProcessOnNoError )
{
    error_map* pTable = s_ErrorMap;

    // Search entire table.
    while( pTable->AbstractError != MEMCARD_END_OF_LIST )
    {
        // Find a match?
        if( pTable->HardwareError == HardwareError )
        {
            // MEMCARD_SUCCESS == no error occured.
            if( (pTable->HardwareError == MEMCARD_SUCCESS) || ErrorIsIgnored( pTable->HardwareError ) )
            {
                // Was error ignored?
                if( pTable->HardwareError != MEMCARD_SUCCESS )
                {
                    s_WasIgnored   = TRUE; 
                    s_IgnoredError = pTable->AbstractError;
                }
                
                // Process if no error?
                if( bProcessOnNoError )
                {
                    // Next!
                    g_MemcardHardware.SendMessage( MSG_PROCESS );
                }
                
                // Woot!
                return FALSE;
            }
            else if (pTable->HardwareError == MEMCARD_BUSY)
            {
                // retry same operation
                g_MemcardHardware.SendMessage( MSG_RETRY );

                return FALSE;
            }
            else
            {
                // Set the error code.
                g_MemcardHardware.SetState( pTable->AbstractError );
                
                // Dear John...
                g_MemcardHardware.SendMessage( MSG_ERROR );
                
                // Drats!
                return TRUE;
            }
        }
        
        // Next!
        pTable++;
    }
    
    // Should NEVER get here.
    ASSERT( 0 );
    return TRUE;
}

//------------------------------------------------------------------------------

u16 sjis_table[]=
{
    0x8140,0x8149,0x8168,0x8194,0x8190,0x8193,0x8195,0x8166,0x8169,0x816a,0x8196,0x817b,0x8143,0x817c,0x8144,0x815e,    //  !"#$%&'()*+,-./
        0x824f,0x8250,0x8251,0x8252,0x8253,0x8254,0x8255,0x8256,0x8257,0x8258,0x8146,0x8147,0x8171,0x8181,0x8172,0x8148,    // 0123456789:;<=>?
        0x8197,0x8260,0x8261,0x8262,0x8263,0x8264,0x8265,0x8266,0x8267,0x8268,0x8269,0x826a,0x826b,0x826c,0x826d,0x826e,    // @ABCDEFGHIJKLMNO
        0x826f,0x8270,0x8271,0x8272,0x8273,0x8274,0x8275,0x8276,0x8277,0x8278,0x8279,0x816d,0x815f,0x816e,0x814f,0x8151,    // PQRSTUVWXYZ[\]^_
        0x814d,0x8281,0x8282,0x8283,0x8284,0x8285,0x8286,0x8287,0x8288,0x8289,0x828a,0x828b,0x828c,0x828d,0x828e,0x828f,    // `abcdefghijklmno
        0x8290,0x8291,0x8292,0x8293,0x8294,0x8295,0x8296,0x8297,0x8298,0x8299,0x829a,0x816f,0x8162,0x8170,0x8160,0x829f,    // pqrstuvwxyz{|}~
        
};

static void AsciiToSJIS(const char *pTitleName,s16 *pSJISBuffer)
{
    s32 count;
    
    for (count=0;count<34;count++)
    {
        if (*pTitleName==0x0)
        {
            pSJISBuffer[count]=0x0;
        }
        else
        {
            pSJISBuffer[count]=(sjis_table[*pTitleName-32]>>8)|(sjis_table[*pTitleName-32]<<8);
            pTitleName++;
        }
    }
    // Make sure it's zero delimited
    pSJISBuffer[33]=0x0;
}

//------------------------------------------------------------------------------


//==============================================================================
//================================= Dispatcher =================================
//==============================================================================

//------------------------------------------------------------------------------
const char* GetStateName( memcard_message Message )
{
    switch( Message )
    {
        SWITCH_LABEL(MSG_INITIATE);
        SWITCH_LABEL(MSG_PROCESS);
        SWITCH_LABEL(MSG_COMPLETE);
        SWITCH_LABEL(MSG_RETRY);
        SWITCH_LABEL(MSG_ERROR);
        default:                    return "<unknown>";
    }
}

void MemcardDispatcher( void )
{
    // Error check.
    ASSERT( !s_DispatcherActive );
    
    // Set semaphore.
    s_DispatcherActive = TRUE;
    
    // Do this a whole lot....
    while( x_GetCurrentThread()->IsActive() )
    {   
        // So lonely...waiting by the phone for someone to call!!!!
        memcard_message Message = (memcard_message)((s32)g_MemcardHardware.m_pDispatcherMQ->Recv( MQ_BLOCK ));
        if( x_GetCurrentThread()->IsActive()==FALSE )
        {
            break;
        }
        
    //  LOG_MESSAGE( "MemcardDispatcher", "Message received. Message:%s", GetStateName(Message) );
        // Someone called! OMG! What should I do?
        switch( Message )
        {
        case MSG_INITIATE:
            // We are in progress now!
            g_MemcardHardware.SetState( MEMCARD_IN_PROGRESS );
            
            // Lets get started! (0 means start the process)
            g_MemcardHardware.SetSubState( 0 );
            
            // Ok process the bad boy.
            g_MemcardHardware.Process();
            break;
            
        case MSG_PROCESS:
            // Next state.
            g_MemcardHardware.SetSubState( g_MemcardHardware.GetSubState() + 1 );
            
            // Ok process the bad boy.
            g_MemcardHardware.Process();
            break;
            
        case MSG_RETRY:
            // retry the same process - don't increment sub state
            g_MemcardHardware.Process();
            break;

        case MSG_COMPLETE:
            // Woot! All good!
            g_MemcardHardware.SetState( MEMCARD_SUCCESS );
            g_MemcardHardware.SetOperation( MEMCARD_OP_IDLE );
            break;
            
        case MSG_ERROR:
            // Opps...something bad.
            g_MemcardHardware.SetOperation( MEMCARD_OP_IDLE );
            break;
            
        default:
            // Should never get here.
            ASSERT( 0 );
        }
    }
}

//==============================================================================
//=============================== Class Functions ==============================
//==============================================================================

//------------------------------------------------------------------------------

memcard_hardware::memcard_hardware( void )
{
}

//------------------------------------------------------------------------------

memcard_hardware::~memcard_hardware( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::Init( void )
{
    sceVu0IVECTOR s_bgcolor[4] = {	// 0xff ‚ªÅ‘å‹P“x 
	    { 0x80,    0,    0, 0 },		// ”wŒiF(¶ã) 
	    {    0, 0x80,    0, 0 },		// ”wŒiF(‰Eã) 
	    {    0,    0, 0x80, 0 },		// ”wŒiF(¶‰º) 
	    { 0x80, 0x80, 0x80, 0 },		// ”wŒiF(‰Eã) 
    };
    sceVu0FVECTOR s_lightdir[3] = {
	    { 0.5, 0.5, 0.5, 0.0 },
	    { 0.0,-0.4,-0.1, 0.0 },
	    {-0.5,-0.5, 0.5, 0.0 },
    };
    sceVu0FVECTOR s_lightcol[3] = {
	    { 0.48, 0.48, 0.03, 0.00 },
	    { 0.50, 0.33, 0.20, 0.00 },
	    { 0.14, 0.14, 0.38, 0.00 },
    };
    sceVu0FVECTOR s_ambient = { 0.50, 0.50, 0.50, 0.00 };

    char* s_IconTitle = "Area 51Profile";    
    s16 SJISBuffer[34];
    s32 i;

    // Error check.
    ASSERT( !s_Initialized );
    
    // Can only init the hardware once on the GameCube.
    if( s_InitHardware )
    {
#if 0
        s32 result;
        //*INEV* BW - this has been moved to IOP boot.

        //g_MemcardHardware.m_MC2Handle = g_IopManager.LoadModule("mc2\\mc2_d.irx");

        //result = sceDbcInit();
        ASSERT(result==1);                
#endif
        
        // Don't do it again...
        s_InitHardware = FALSE;
    }

#if 0
    //*INEV*BW this has been moved to IOP boot.
    //result = sceMc2Init(0);
    //ASSERT(SCE_ERROR_ERRNO(result) == SCE_OK);
#endif
    
    // clear the socket IDs
    s_CardSocketID[0] = -1;
    s_CardSocketID[1] = -1;

    // clear the connected flag
    s_IsConnected[0]  = FALSE;
    s_IsConnected[1]  = FALSE;
    
    // create virtual sockets for the memory card(s)
    s_CardSocket[0].option = SCE_MC2_SPECIFIC_PORT;
    s_CardSocket[0].port   = SCE_MC2_PORT_1;
    s_CardSocket[0].slot   = 0; // always 0 - multitap not supported in libmc2
    s_CardSocketID[0] = sceMc2CreateSocket(&s_CardSocket[0], s_CardDmaBuf);
    ASSERT(s_CardSocketID[0] >= 0);

    s_CardSocket[1].option = SCE_MC2_SPECIFIC_PORT;
    s_CardSocket[1].port   = SCE_MC2_PORT_2;
    s_CardSocket[1].slot   = 0; // always 0 - multitap not supported in libmc2
    s_CardSocketID[1] = sceMc2CreateSocket(&s_CardSocket[1], s_CardDmaBuf);
    ASSERT(s_CardSocketID[1] >= 0);

    // initialize internal states
    m_pDispatcherMQ      = new xmesgq(MAX_MESSAGES);
    m_Error              = MEMCARD_SUCCESS;
    m_Operation          = MEMCARD_OP_IDLE;
    m_RequestedCard      = NO_CARD;
    m_pRequestedBuffer   = NULL;
    m_nRequestedBytes    = 0;
    m_RequestedOffset    = 0;
    m_MountedCard        = NO_CARD;
    m_FileLength         = -1;
    
    // Don't ignore any errors.
    ClearIgnoreList();
       
    // Nuke the fileinfo.
    s_FileInfo.Clear();

    // Create the memcard_hardware thread.
    m_pThread = new xthread( MemcardDispatcher, "memcard_mgr dispatcher", 8192, 1 );
    
    // setup icon data
    x_memset(&s_Icon, 0, sizeof(s_Icon));
	x_strcpy((char *)s_Icon.Head, "PS2D");
    AsciiToSJIS(s_IconTitle, SJISBuffer);
    x_memcpy(s_Icon.TitleName, SJISBuffer, 34*sizeof(s16));

#if 0
    // Find a good place to do a linebreak in the title name as close to the
    // middle as possible.
    i = x_strlen(s_IconTitle) / 2 ;
    while (s_IconTitle[i]!=' ')
    {
        i--;
        if (i<0)
        {
            i=x_strlen(s_IconTitle) / 2 ;
            break;
        }
    }
#else
    // force the CR for A51
    i = 7;
#endif
	s_Icon.OffsLF = i*2;
	s_Icon.TransRate = 0x60;
	x_memcpy(s_Icon.BgColor, s_bgcolor, sizeof(s_bgcolor));
	x_memcpy(s_Icon.LightDir, s_lightdir, sizeof(s_lightdir));
	x_memcpy(s_Icon.LightColor, s_lightcol, sizeof(s_lightcol));
	x_memcpy(s_Icon.Ambient, s_ambient, sizeof(s_ambient));

	x_strcpy((char *)s_Icon.FnameView, s_IconName);
    x_strcpy((char *)s_Icon.FnameCopy, s_IconName);
    x_strcpy((char *)s_Icon.FnameDel,  s_IconName);
        
    // Ok all good now!
    s_Initialized = TRUE;
}

//------------------------------------------------------------------------------

void memcard_hardware::Kill( void )
{
    s32 result;
    
    // Error check.
    ASSERT( s_Initialized );
    
    // Nuke the thread.
    delete m_pThread;
    delete m_pDispatcherMQ;
    
    s_DispatcherActive = FALSE;
    
    // Nuke the fileinfo.
    s_FileInfo.Clear();
    s_FileInfo.FreeExtra();

    // delete the sockets
    result = sceMc2DeleteSocket( s_CardSocketID[0] );
    ASSERT(SCE_ERROR_ERRNO(result) == SCE_OK);
    result = sceMc2DeleteSocket( s_CardSocketID[1] );
    ASSERT(SCE_ERROR_ERRNO(result) == SCE_OK);
    
    // clear the socket ID
    s_CardSocketID[0] = -1;
    s_CardSocketID[1] = -1;
    
    // kill the ps2 libs``
    sceMc2End();

    // All bad now...
    s_Initialized = FALSE;
}

//------------------------------------------------------------------------------

s32 g_MemCardSleepMS = 1;

void memcard_hardware::ProcessHardwareCallback( void )
{
	// Sleep for a bit and give PS2 memcard some time.
	x_DelayThread(g_MemCardSleepMS);

    // check results of async calls to mem card routines
    s32 result;
    s32 status, cmnd;
    
    status = sceMc2CheckAsync(&cmnd, &result);

    // **BW** This is such a hack! We need some way of indicating how much
    // data has actually been read or written from the memory card. So, I'm
    // just doing a guesstimate of how long it will take to do the operation.
    if( cmnd == SCE_MC2_FUNC_WRITEFILE )
    {
        if( m_FilePosition < m_FileLength )
        {
            m_FilePosition += (s32)(1.6*1024);
        }
    }
    else if( cmnd == SCE_MC2_FUNC_READFILE )
    {
        if( m_FilePosition < m_FileLength )
        {
            m_FilePosition += 6*1024;
        }
    }
    else
    {
        m_FilePosition = 0;
    }

    // check if our operation completed
    if (status == SCE_MC2_STAT_EXEC_FINISH)
    {
        // do any command specific processing
        switch (cmnd)
        {
     /*
            SCE_MC2_FUNC_GETINFO
                SCE_OK              A memory card that remains inserted was detected
                SCE_ENEW_DEVICE     Memory card was swapped
                SCE_ECONNREFUSED    Memory card access failed
                SCE_ENODEV          Memory card could not be detected
                SCE_EDEVICE_BROKEN  Memory card is damaged
                SCE_EFORMAT         Memory card has not been formatted


            SCE_MC2_FUNC_FORMAT
                SCE_OK              Success
                SCE_ECONNREFUSED    Memory card access failed
                SCE_ENODEV          Memory card could not be detected


            SCE_MC2_FUNC_READFILE
                >=0                 Success (value is number of bytes read)
                SCE_ECONNREFUSED    Memory card access failed
                SCE_EINVAL          Path name or parameter is invalid
                SCE_EACCES          File is read-prohibited
                SCE_ENOENT          Specified path does not exist
                SCE_ENODEV          Memory card could not be detected


            SCE_MC2_FUNC_WRITEFILE
                >=0                 Success (value is number of bytes written)
                SCE_ECONNREFUSED    Memory card access failed
                SCE_EINVAL          Path name is invalid
                SCE_EACCES          File is write-prohibited.
                SCE_ENOSPC          Insufficient free space
                SCE_ENOENT          Specified path does not exist
                SCE_ENODEV          Memory card could not be detected


            SCE_MC2_FUNC_CREATEFILE
                SCE_OK              Success
                SCE_ECONNREFUSED    Memory card access failed
                SCE_EINVAL          Path name is invalid
                SCE_EEXIST          A file or directory having the same name already exists
                SCE_ENOSPC          Insufficient free space
                SCE_ENFILE          No more files can be created
                SCE_ENODEV          Memory card could not be detected


            SCE_MC2_FUNC_DELETE
                SCE_OK              Success
                SCE_ECONNREFUSED    Memory card access failed
                SCE_EINVAL          Path name is invalid
                SCE_ENOENT          Specified path does not exist
                SCE_ENOTEMPTY       Directory is not empty
                SCE_ENODEV          Memory card could not be detected


            SCE_MC2_FUNC_GETDIR
                SCE_OK              Success
                SCE_ECONNREFUSED    Memory card access failed
                SCE_EINVAL          Path name is invalid
                SCE_EFILE_BROKEN    Specified directory is corrupted
                SCE_ENOENT          Specified path does not exist
                SCE_ENOTDIR         Specified path is not a directory
                SCE_ENODEV          Memory card could not be detected


            SCE_MC2_FUNC_MKDIR
                SCE_OK              Success
                SCE_ECONNREFUSED    Memory card access failed
                SCE_EINVAL          Path name is invalid
                SCE_EEXIST          A file or directory having the same name already exists
                SCE_EMDEPTH         Directory nesting is too deep
                SCE_ENOSPC          Insufficient free space
                SCE_ENODEV          Memory card could not be detected


            SCE_MC2_FUNC_CHDIR
                SCE_OK              Success
                SCE_ENOENT          Specified path does not exist
                SCE_ENOTDIR         Specified path is not a directory
                SCE_EINVAL          Path name is invalid
                SCE_ECONNREFUSED    Memory card access failed
                SCE_ENODEV          Memory card could not be detected


            SCE_MC2_FUNC_SEARCHFILE
                SCE_OK              Success
                SCE_ECONNREFUSED    Memory card access failed
                SCE_EINVAL          Path name is invalid
                SCE_ENOENT          Specified path does not exist
                SCE_ENODEV          Memory card could not be detected

  
            SCE_MC2_GETENTSPC
                >=0                 Success (number of free entries is returned)
                SCE_ECONNREFUSED    Memory card access failed
                SCE_EINVAL          Path name is invalid
                SCE_ENOENT          Specified path does not exist
                SCE_ENOTDIR         Specified path is not a directory
                SCE_ENODEV          Memory card could not be detected
     */
        default:
            // nothing to do!
            break;
        }

        
        // If no error, then continue.
        CheckError( SonyToInternalError(result), TRUE );    
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::SetIconDisplayName( const char* pName )
{
    // set the display name of the memcard icon
    s16 SJISBuffer[34];
    char IconTitle[32];

    x_memset( IconTitle, 0, sizeof(IconTitle) );
    x_strcpy( IconTitle, "Area 51" );
    x_strcat( IconTitle, pName );

    AsciiToSJIS( IconTitle, SJISBuffer );
    x_memcpy( s_Icon.TitleName, SJISBuffer, 34*sizeof(s16) );
}

//------------------------------------------------------------------------------

void memcard_hardware::SendMessage( s32 Message )
{
    // Error check.
    ASSERT( s_Initialized );
    
    // A message in a bottle...send it to the dispatcher.
    s32 MessageStatus = m_pDispatcherMQ->Send( (void*)Message, MQ_NOBLOCK );
    
    // Make sure it was sent!
    ASSERT( MessageStatus );
    
    // No nasty warnings in release.
    (void)MessageStatus;
}

//------------------------------------------------------------------------------

void memcard_hardware::SetIOParams( const char* FileName, byte* pBuffer, s32 Offset, s32 nBytes )
{
    // Error check.
    ASSERT( s_Initialized );
    ASSERT( x_strlen( FileName ) < (s32)sizeof(m_pRequestedFileName) );

    ASSERT( FileName );
    x_strcpy( m_pRequestedFileName,FileName );

    m_pRequestedBuffer   = pBuffer;
    m_nRequestedBytes    = nBytes;
    m_RequestedOffset    = Offset;
}

//------------------------------------------------------------------------------

const char* GetStateName( memcard_op Operation )
{
    switch( Operation )
    {
        SWITCH_LABEL(MEMCARD_OP_IDLE);
        SWITCH_LABEL(MEMCARD_OP_MOUNT);
        SWITCH_LABEL(MEMCARD_OP_UNMOUNT);
        SWITCH_LABEL(MEMCARD_OP_READ_FILE);
        SWITCH_LABEL(MEMCARD_OP_WRITE_FILE);
        SWITCH_LABEL(MEMCARD_OP_DELETE_FILE);
        SWITCH_LABEL(MEMCARD_OP_FORMAT);
        SWITCH_LABEL(MEMCARD_OP_REPAIR);
        SWITCH_LABEL(MEMCARD_OP_READ_FILE_LIST);
        SWITCH_LABEL(MEMCARD_OP_PURGE_FILE_LIST);
        SWITCH_LABEL(MEMCARD_OP_GET_FILE_LENGTH);
        SWITCH_LABEL(MEMCARD_OP_READ);
        SWITCH_LABEL(MEMCARD_OP_WRITE);
        SWITCH_LABEL(MEMCARD_OP_CREATE_FILE);
        SWITCH_LABEL(MEMCARD_OP_CREATE_DIR);
        SWITCH_LABEL(MEMCARD_OP_SET_DIR);
        SWITCH_LABEL(MEMCARD_OP_DELETE_DIR);
    default: return "<unknown>";
    }
}
void memcard_hardware::SetOperation( memcard_op Operation )
{
    // Error check.
    ASSERT( s_Initialized );
    
    if( m_Operation != Operation )
    {
    //  LOG_MESSAGE( "memcard_hardware::SetOperation", "Operation set from %s to %s", GetStateName(m_Operation), GetStateName(Operation) );
    }
    // Set the operation.
    m_Operation = Operation;
}

//------------------------------------------------------------------------------

void memcard_hardware::InitiateOperation( void )
{
    // Error check.
    ASSERT( s_Initialized );
    
    // Can't have 2 concurrent operations.
    ASSERT( GetState() != MEMCARD_IN_PROGRESS ); 
    
    // Don't ignore any errors.
    ClearIgnoreList();
    
    // We have ignition...er or something like that...
    SendMessage( MSG_INITIATE );
}

//------------------------------------------------------------------------------

s32 memcard_hardware::GetFileList( xarray<mc_file_info>& FileList )
{
    FileList.Clear();
    
    if( m_bIsFileListValid )
    {
        FileList.SetCapacity( m_nFileCount );
        
        for( s32 i=0 ; i<m_nFileCount ; i++ )
        {
            FileList.Append() = s_FileInfo[i];
        }
        
        return m_nFileCount;
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::InvalidateFileList( void )
{
    // Invalid the file list.
    m_bIsFileListValid = FALSE;
    m_nFileCount       = 0;
    
    // Clear the file info.
    s_FileInfo.Clear();
}

//------------------------------------------------------------------------------

void memcard_hardware::Process( void )
{
    // Error check.
    ASSERT( s_Initialized );
    
    // Lets figure out what to do!
    switch( GetOperation() )
    {
    case MEMCARD_OP_MOUNT:
        ProcessMount();
        break;
        
    case MEMCARD_OP_UNMOUNT:
        ProcessUnmount();
        break;
        
    case MEMCARD_OP_READ_FILE:
        ProcessReadFile();
        break;
        
    case MEMCARD_OP_WRITE_FILE:
        ProcessWriteFile();
        break;
        
    case MEMCARD_OP_DELETE_FILE:
        ProcessDeleteFile();
        break;
        
    case MEMCARD_OP_FORMAT:
        ProcessFormat();
        break;
        
    case MEMCARD_OP_REPAIR:
        ProcessRepair();
        break;
        
    case MEMCARD_OP_READ_FILE_LIST:
        ProcessReadFileList();
        break;
        
    case MEMCARD_OP_PURGE_FILE_LIST:
        ProcessPurgeFileList();
        break;
        
    case MEMCARD_OP_GET_FILE_LENGTH:
        ProcessGetFileLength();
        break;

    case MEMCARD_OP_READ:
        ProcessRead();
        break;
    
    case MEMCARD_OP_WRITE:
        ProcessWrite();
        break;
    
    case MEMCARD_OP_CREATE_FILE:
        ProcessCreateFile();
        break;

    case MEMCARD_OP_CREATE_DIR:
        ProcessCreateDir();
        break;

    case MEMCARD_OP_SET_DIR:
        ProcessSetDir();
        break;

    case MEMCARD_OP_DELETE_DIR:
        ProcessDeleteDir();
        break;

    case MEMCARD_OP_IDLE:
    default:
        // Um, how did this happen?
        ASSERT( 0 );
        break;
    }
}

enum mount_state
{
    STATE_MOUNT_START = 0,
    STATE_MOUNT_COMPLETED,
};

f32 MountTime;

void memcard_hardware::ProcessMount( void )
{
    s32 Result;
    
    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_MOUNT_START:
        // No file length.
        SetFileLength( -1 );
        
        // Nuke the file list
        //InvalidateFileList();
        
        // Set the mounted card. 
        SetMountedCard( m_RequestedCard );
        
        // Set sector size (8k - could be anything, but keep same as NGC for simplicity)
        m_SectorSize = 8192;

        // Attempt to mount the card.
        {
            xtimer t;
            t.Start();
            Result = sceMc2GetInfoAsync( s_CardSocketID[m_RequestedCard], &s_CardInfo[m_RequestedCard] );
            t.Stop();
            MountTime = t.ReadMs();
        }
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;              
        
    case STATE_MOUNT_COMPLETED:            
        // Done!
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------

enum unmount_state
{
    STATE_UNMOUNT_START = 0,
    STATE_UNMOUNT_COMPLETED,
};

void memcard_hardware::ProcessUnmount( void )
{   
    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_UNMOUNT_START:
         if( m_MountedCard == NO_CARD )
        {
            // No card is mounted...
            CheckError( CARD_ERR_UNPLUGGED );
        }
        else
        {
            // No file length.
            SetFileLength( -1 );
            
            // Nuke the file list.
            //InvalidateFileList();
                        
            // No card is mounted.
            m_MountedCard = NO_CARD;

            // Check error and process.
            CheckError( CARD_ERR_OK, TRUE );
        }
        break;
        
    case STATE_UNMOUNT_COMPLETED:
        // Done!
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------

enum readfile_state
{
    STATE_READFILE_START = 0,
    STATE_READFILE_READ,
    STATE_READFILE_COMPLETED,
};

void memcard_hardware::ProcessReadFile( void )
{
    s32 Result;
    
    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {       
        // Oops...bad.
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }
    
    // These need to be valid.
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes );
    
    
    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_READFILE_START:
        // check directory exists and change to it 
        ClearIgnoreList();
        AddErrorToIgnoreList( CARD_ERR_NOTFOUND );

    //  LOG_MESSAGE( "memcard_hardware::ProcessRead", "Setting directory:%s.", m_pRequestedDirName );
        Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);        
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;
        
    case STATE_READFILE_READ:
        // check if the directory was found
        if (s_WasIgnored)
        {
            // not found
            ClearIgnoreList();
            CheckError(CARD_ERR_NOTFOUND, FALSE);
        }
        else
        {
            // Read the data
        //  LOG_MESSAGE( "memcard_hardware::ProcessRead", "Reading file:%s, length:%d.", m_pRequestedFileName, m_nRequestedBytes );
            Result = sceMc2ReadFileAsync(s_CardSocketID[m_MountedCard], m_pRequestedFileName, m_pRequestedBuffer, 0, m_nRequestedBytes);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        }
        break;
        
    case STATE_READFILE_COMPLETED:
        // its all good
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
    

}

//------------------------------------------------------------------------------

enum writefile_state
{
    STATE_WRITEFILE_START = 0,
    STATE_WRITEFILE_CHECKDIR,
    STATE_WRITEFILE_CHANGEDIR,
    STATE_WRITEFILE_CREATEICONFILE,
    STATE_WRITEFILE_WRITEICONFILE,
    STATE_WRITEFILE_FREEICONBUFFER,
    STATE_WRITEFILE_CREATE,
    STATE_WRITEFILE_WRITE,
    STATE_WRITEFILE_CREATEICONSYS,
    STATE_WRITEFILE_WRITEICONSYS,
    STATE_WRITEFILE_COMPLETED,
};

void memcard_hardware::ProcessWriteFile( void )
{
    static X_FILE  *pFile;
    static s32     length=0;
    static u8      *pBuffer=NULL;
    s32             Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Opps...
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes );

    m_FileLength    = m_nRequestedBytes;
    m_FilePosition  = 0;
    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_WRITEFILE_START:
        // check if the directory exists on the card
        ClearIgnoreList();
        AddErrorToIgnoreList( CARD_ERR_NOTFOUND );
        
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Set Directory:%s.", m_pRequestedDirName );
        Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;

    case STATE_WRITEFILE_CHECKDIR:

        // check if the directory was found
        if (s_WasIgnored)
        {
            // not found - create it
            // doesn't exist - create it!
            ClearIgnoreList();
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Create Directory:%s.", m_pRequestedDirName );
            Result = sceMc2MkdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        }
        else
        {
            // we found it, so skip to the write state
            m_SubState = STATE_WRITEFILE_CREATE-1;
            // send process msg and increment the state
            ClearIgnoreList();
            CheckError(CARD_ERR_OK, TRUE);
        }
        break;

    case STATE_WRITEFILE_CHANGEDIR:
        // change to it
    //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Set Directory:%s.", m_pRequestedDirName );
        Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;
        
    case STATE_WRITEFILE_CREATEICONFILE:
        pFile=x_fopen(xfs("%s\\%s",g_FullPath,s_IconName),"rb");
        if (!pFile)
        {
            ASSERT(FALSE);
        }
        x_fseek(pFile,0,X_SEEK_END);
        length = x_ftell(pFile);
        x_fseek(pFile,0,X_SEEK_SET);
        pBuffer = (u8 *)x_malloc(length);
        ASSERT(pBuffer);
        x_fread(pBuffer,length,1,pFile);
        x_fclose(pFile);
    
    //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Create icon file:%s.", s_IconName );
        Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], s_IconName);
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;

    case STATE_WRITEFILE_WRITEICONFILE:
    //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Write icon file:%s, length:%d.", s_IconName, length );
        Result = sceMc2WriteFileAsync (s_CardSocketID[m_MountedCard], s_IconName, pBuffer, 0, length);
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;

    case STATE_WRITEFILE_FREEICONBUFFER:
        x_free(pBuffer);
        CheckError(CARD_ERR_OK, TRUE);
        break;

    case STATE_WRITEFILE_CREATE:
        ClearIgnoreList();
        AddErrorToIgnoreList( CARD_ERR_EXISTS );
    //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Create file:%s.", m_pRequestedFileName );
        Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], m_pRequestedFileName);
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                
        break;
            
    case STATE_WRITEFILE_WRITE:
        // Nuke the file list.
        //InvalidateFileList();
         
        // Write data to the memcard.
    //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Write file:%s, length:%d.", m_pRequestedFileName, m_nRequestedBytes );
        Result = sceMc2WriteFileAsync (s_CardSocketID[m_MountedCard], m_pRequestedFileName, m_pRequestedBuffer, 0, m_nRequestedBytes);
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;

    case STATE_WRITEFILE_CREATEICONSYS:
        // create and write icon file
        ClearIgnoreList();
        AddErrorToIgnoreList( CARD_ERR_EXISTS );
    //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Create icon.sys." );
        Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], "icon.sys");
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;

    case STATE_WRITEFILE_WRITEICONSYS:
    //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Write icon.sys, size:%d.", sizeof(s_Icon) );
        Result = sceMc2WriteFileAsync(s_CardSocketID[m_MountedCard], "icon.sys", &s_Icon, 0, sizeof(s_Icon));
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;

    case STATE_WRITEFILE_COMPLETED:
        // Done!
        ClearIgnoreList();
        SendMessage( MSG_COMPLETE );
        break;

    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------

enum deletefile_state
{
    STATE_DELETEFILE_START = 0,
    STATE_DELETEFILE_COMPLETED,
};

void memcard_hardware::ProcessDeleteFile( void )
{
    s32 Result;
    
    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }
    
    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_DELETEFILE_START:
        // Wave bye bye...
    //  LOG_MESSAGE( "memcard_hardware::ProcessDeleteFile", "Deleting file:%s", m_pRequestedFileName );
        Result = sceMc2DeleteAsync(s_CardSocketID[m_MountedCard], m_pRequestedFileName);
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                
        break;
        
    case STATE_DELETEFILE_COMPLETED:
        // Nuke the file list.
        //InvalidateFileList();
        
        // Done!
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }    
}

//------------------------------------------------------------------------------

enum format_state
{
    STATE_FORMAT_START = 0,
    STATE_FORMAT_COMPLETED,
};

void memcard_hardware::ProcessFormat( void )
{
    s32 Result;
    
    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }
    
    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_FORMAT_START:
        // No file length.
        SetFileLength( -1 );
        
        // Invalidate the file list.
        //InvalidateFileList();
        
        // Wave bye bye...
        Result = sceMc2FormatAsync( s_CardSocketID[m_MountedCard] );
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                               
        break;
        
    case STATE_FORMAT_COMPLETED:
        // Done!
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------
enum readfilelist_state
{
    STATE_READFILELIST_START = 0,
    STATE_READFILELIST_CHECK,
};

void memcard_hardware::ProcessReadFileList( void )
{
    s32 Result;
    s32 i;
    
    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }
    
    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_READFILELIST_START:
        // Init file list
        m_bIsFileListValid = FALSE;
        m_nFileCount       = 0;
        s_nFiles           = 0; 
        
        // Clear the file list.
        s_FileInfo.Clear();

        // read the file list from the card (within our game directory only)        
        Result = sceMc2GetDirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, 0, MEMCARD_DIR_ENTRIES, s_DirList, &s_nFiles);
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);            
        break;

    case STATE_READFILELIST_CHECK:
        // Make space for the files.
        s_FileInfo.SetCapacity( s_nFiles );
        
        // Now fill in the file info data structures
        for (i=0; i<s_nFiles; i++)
        {
            s_FileInfo.Append();
            x_strcpy( s_FileInfo[i].FileName, s_DirList[i].name);
            s_FileInfo[i].Length       = s_DirList[i].size;
            s_FileInfo[i].Index        = i; 
            s_FileInfo[i].CreationDate = *(u64*)(&s_DirList[i].creation);
            s_FileInfo[i].ModifiedDate = *(u64*)(&s_DirList[i].modification);
        }

        // Init file list
        m_nFileCount       = s_nFiles;
        m_bIsFileListValid = TRUE;
        
        // All done!
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------

enum purgefilelist_state
{
    STATE_PURGEFILELIST_START = 0,
};

void memcard_hardware::ProcessPurgeFileList( void )
{
    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_PURGEFILELIST_START:
        // Nuke it.
        InvalidateFileList();
        
        // Done!
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------

enum getfilelength_state
{
    STATE_GETFILELENGTHLIST_START = 0,
    STATE_GETFILELENGTHLIST_SET,
};

void memcard_hardware::ProcessGetFileLength( void )
{
    s32      Result;
    
    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }
    
    switch( m_SubState )
    {
    case STATE_GETFILELENGTHLIST_START:
        // No file length.
        SetFileLength( -1 );
        
        // make sure we are looking in the correct directory
    //  LOG_MESSAGE( "memcard_hardware::ProcessGetFileLength", "Setting directory:%s.", m_pRequestedDirName );
        Result = sceMc2Chdir(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);

        switch (SCE_ERROR_ERRNO(Result))
        {
        case SCE_ENOENT:
        case SCE_ENOTDIR:
            // doesn't exist - bail out
            SetFileLength( 0 );
            
            // Done!
            SendMessage( MSG_COMPLETE );
            return;
        
        default:
            // error check as normal
            CheckError(SonyToInternalError(Result));        
            break;
        }
            
        // search on the card for the file in question
    //  LOG_MESSAGE( "memcard_hardware::ProcessGetFileLength", "Search directory:%s.", m_pRequestedDirName );
        Result = sceMc2SearchFile(s_CardSocketID[m_MountedCard], m_pRequestedFileName, &s_CardDir[m_MountedCard]);

        if (SCE_ERROR_ERRNO(Result) == SCE_ENOENT)
        {
            // file doesn't exist
            SetFileLength( 0 );
            
            // Done!
            SendMessage( MSG_COMPLETE );
        }
        else
        {
            CheckError(SonyToInternalError(Result), TRUE);
        }
        break;
        
    case STATE_GETFILELENGTHLIST_SET:
        // Set the length 
        SetFileLength( s_CardDir[m_MountedCard].size );
        
        // Done!
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::ProcessRepair( void )
{
    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }

    // do nothing!
    SendMessage( MSG_COMPLETE );
}

//------------------------------------------------------------------------------

s32 memcard_hardware::GetMaxCards( void )
{
    // Error check.
    ASSERT( s_Initialized );
    
    // Tell the world.
    return MAX_MEMCARDS;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Return raw status information about the memcard.
memcard_error memcard_hardware::GetCardStatus( s32 CardID )
{
    // Error check.
    ASSERT( s_Initialized );

    // check socket is initialized
    if (s_CardSocketID[CardID] == -1)
    {
        ASSERT( FALSE );
        return MEMCARD_NO_CARD;
    }


    // Is a card connected?
    s32 Result = sceMc2GetInfo( s_CardSocketID[CardID], &s_CardInfo[CardID] );
    xbool IsFormatted = s_CardInfo[CardID].isFormat;

    s32 ErrorCode = SCE_ERROR_ERRNO(Result);
    switch ( ErrorCode )
    {
    case SCE_OK:
    case SCE_ENEW_DEVICE:               
        if( IsFormatted == FALSE )
        {
            return MEMCARD_UNFORMATTED;
        }
        return MEMCARD_SUCCESS;

    case SCE_EFORMAT:
        return MEMCARD_UNFORMATTED;

    case SCE_ENOTSUP:
        return MEMCARD_NOT_A_MEMCARD;

    case SCE_EDEVICE_BROKEN:
        return MEMCARD_DAMAGED;

    case SCE_ENODEV:
    case SCE_ECONNREFUSED:
        return MEMCARD_NO_CARD;
    default:
        ASSERT( FALSE );
        break;
    }

    return MEMCARD_NO_CARD;
}


xbool memcard_hardware::IsCardConnected( s32 CardID )
{
    xbool IsConnected = FALSE;
    
    // Error check.
    ASSERT( s_Initialized );
    
    // check socket is initialized
    if (s_CardSocketID[CardID] == -1)
        return FALSE;


    // Is a card connected?
    s32 Result = sceMc2GetInfo( s_CardSocketID[CardID], &s_CardInfo[CardID] );
      
    switch ( SCE_ERROR_ERRNO(Result) )
    {
    case SCE_OK:
    case SCE_ENEW_DEVICE:               
    case SCE_EFORMAT:           
    case SCE_EDEVICE_BROKEN:    
    case SCE_ENOSPC:
    case SCE_EACCES:           
        // found memory card
        IsConnected = TRUE;
        break;

    case SCE_EID:
        // oops socket invalid - shouldn't ever happen!
        ASSERT( FALSE );
        break;

    default:
        break;
    }

  
    // Tell the world.
//    return s_IsConnected[CardID];
    return IsConnected;
}


//------------------------------------------------------------------------------

enum read_state
{
    STATE_READ_START = 0,
    STATE_READ_READ,
    STATE_READ_COMPLETED,
};

void memcard_hardware::ProcessRead( void )
{
    s32 Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Oops...bad.
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes > 0 );
    ASSERT( m_RequestedOffset >= 0 );


    // Decide what to do!
    switch( m_SubState )
    {
    case STATE_READ_START:
        // check directory exists and change to it 
        ClearIgnoreList();
        AddErrorToIgnoreList( CARD_ERR_NOTFOUND );

    //  LOG_MESSAGE( "memcard_hardware::ProcessRead", "Setting directory:%s.", m_pRequestedDirName );
        Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);        
        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        break;
        
    case STATE_READ_READ:
        // check if the directory was found
        if (s_WasIgnored)
        {
            // enough room, but not found, so create..
            ClearIgnoreList();
            CheckError(CARD_ERR_NOTFOUND, FALSE);
        }
        else
        {
            // Read the data
        //  LOG_MESSAGE( "memcard_hardware::ProcessRead", "Reading file:%s, length:%d.", m_pRequestedFileName, m_nRequestedBytes );

            Result = sceMc2ReadFileAsync(s_CardSocketID[m_MountedCard], m_pRequestedFileName, m_pRequestedBuffer, m_RequestedOffset, m_nRequestedBytes);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
        }
        break;
        
    case STATE_READ_COMPLETED:
        // Done!
        SendMessage( MSG_COMPLETE );
        break;
        
    default:
        // Should never get here.
        ASSERT( 0 );
        break;
    }
}

//------------------------------------------------------------------------------

enum write_state
{
    STATE_WRITE_START = 0,
    STATE_WRITE_MKDIR,
    STATE_WRITE_CHDIR,
    STATE_WRITE_CREATEICONFILE,
    STATE_WRITE_WRITEICONFILE,
    STATE_WRITE_FREEICONBUFFER,
    STATE_WRITE_FIND_FILE,
    STATE_WRITE_CREATE,
    STATE_WRITE_WRITE,
    STATE_WRITE_CREATEICONSYS,
    STATE_WRITE_WRITEICONSYS,
    STATE_WRITE_COMPLETED,
};

void memcard_hardware::ProcessWrite( void )
{
    static X_FILE  *pFile;
    static s32     length=0;
    static u8      *pBuffer=NULL;
    s32             Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Opps...
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes > 0 );
    ASSERT( m_RequestedOffset >= 0 );


    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_WRITE_START:
            // check if the directory exists on the card
            ClearIgnoreList();
            AddErrorToIgnoreList( CARD_ERR_NOTFOUND );
            Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Directory set to %s.", m_pRequestedDirName );
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_WRITE_MKDIR:
            // check if the directory was found
            if (s_WasIgnored)
            {
                // not found - create it
                // doesn't exist - create it!
                ClearIgnoreList();
                Result = sceMc2MkdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName);
            //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Directory created: %s.", m_pRequestedDirName );
                ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            }
            else
            {
                // we found it, so skip to the write state
                m_SubState = STATE_WRITE_FIND_FILE-1;
                // send process msg and increment the state
                ClearIgnoreList();
                CheckError(CARD_ERR_OK, TRUE);
            }
            break;

        case STATE_WRITE_CHDIR:
            // change to it
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Directory set to %s.", m_pRequestedDirName );
            Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                       
            break;

        case STATE_WRITE_CREATEICONFILE:
            pFile=x_fopen(xfs("%s\\%s",g_FullPath,s_IconName),"rb");
            if (!pFile)
            {
                ASSERT(FALSE);
            }
            x_fseek(pFile,0,X_SEEK_END);
            length = x_ftell(pFile);
            x_fseek(pFile,0,X_SEEK_SET);
            pBuffer = (u8 *)x_malloc(length);
            ASSERT(pBuffer);
            x_fread(pBuffer,length,1,pFile);
            x_fclose(pFile);
    
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Creating icon file:%s.", s_IconName );
            Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], s_IconName);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_WRITE_WRITEICONFILE:
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Writing icon file:%s, length:%d.", s_IconName, length );
            Result = sceMc2WriteFileAsync (s_CardSocketID[m_MountedCard], s_IconName, pBuffer, 0, length);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_WRITE_FREEICONBUFFER:
            x_free(pBuffer);
            CheckError(CARD_ERR_OK, TRUE);
            break;            

        case STATE_WRITE_FIND_FILE:
            ClearIgnoreList();
            AddErrorToIgnoreList( CARD_ERR_NOTFOUND );
            Result = sceMc2SearchFileAsync(s_CardSocketID[m_MountedCard], m_pRequestedFileName, &s_CardDir[m_MountedCard]);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                        
            break;

        case STATE_WRITE_CREATE:                        
            if (s_WasIgnored)
            {
                // Could not find the file, so create it!
            //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Creating file:%s.", m_pRequestedFileName );
                Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], m_pRequestedFileName);
                ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                        
            }
            else
            {
                // found it, now write to it
                CheckError(CARD_ERR_OK, TRUE);
            }
            break;
            
        case STATE_WRITE_WRITE:
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Writing file:%s, length:%d.", m_pRequestedFileName, m_nRequestedBytes );
            Result = sceMc2WriteFileAsync (s_CardSocketID[m_MountedCard], m_pRequestedFileName, m_pRequestedBuffer, m_RequestedOffset, m_nRequestedBytes);                    
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                       
            break;

        case STATE_WRITE_CREATEICONSYS:
            // create and write icon file
            ClearIgnoreList();
            AddErrorToIgnoreList( CARD_ERR_EXISTS );
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "icon.sys created." );
            Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], "icon.sys");
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_WRITE_WRITEICONSYS:
        //  LOG_MESSAGE( "memcard_hardware::ProcessWrite", "Writing icon.sys file." );
            Result = sceMc2WriteFileAsync(s_CardSocketID[m_MountedCard], "icon.sys", &s_Icon, 0, sizeof(s_Icon));
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_WRITE_COMPLETED:
            // Done!
            ClearIgnoreList();
            SendMessage( MSG_COMPLETE );
            break;

        default:
            // Should never get here.
            ASSERT( 0 );
            break;
    }
}

//------------------------------------------------------------------------------

enum createfile_state
{
    STATE_CREATEFILE_START = 0,
    STATE_CREATEFILE_MKDIR,
    STATE_CREATEFILE_CHDIR,
    STATE_CREATEFILE_CREATEICONFILE,
    STATE_CREATEFILE_WRITEICONFILE,
    STATE_CREATEFILE_FREEICONBUFFER,
    STATE_CREATEFILE_WRITE,
    STATE_CREATEFILE_CREATEICONSYS,
    STATE_CREATEFILE_WRITEICONSYS,
    STATE_CREATEFILE_COMPLETED,
};

void memcard_hardware::ProcessCreateFile( void )
{
    static X_FILE  *pFile;
    static s32     length=0;
    static u8      *pBuffer=NULL;
    s32             Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Opps...
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes );

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_CREATEFILE_START:
            // check if the directory exists on the card
            ClearIgnoreList();
            AddErrorToIgnoreList( CARD_ERR_NOTFOUND );
            Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);
        //  LOG_MESSAGE( "memcard_hardware::ProcessCreateFile", "Set directory: %s.", m_pRequestedDirName );
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEFILE_MKDIR:
            // check if the directory was found
            if (s_WasIgnored)
            {
                // not found - create it
                // doesn't exist - create it!
                ClearIgnoreList();
            //  LOG_MESSAGE( "memcard_hardware::ProcessCreateFile", "Create directory: %s.", m_pRequestedDirName );
                Result = sceMc2MkdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName);
                ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            }
            else
            {
                // we found it, so skip to the write state
                m_SubState = STATE_CREATEFILE_WRITE-1;
                // send process msg and increment the state
                ClearIgnoreList();
                CheckError(CARD_ERR_OK, TRUE);
            }
            break;

        case STATE_CREATEFILE_CHDIR:
            // change to it
        //  LOG_MESSAGE( "memcard_hardware::ProcessCreateFile", "Set directory: %s.", m_pRequestedDirName );
            Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                       
            break;
        
        case STATE_CREATEFILE_CREATEICONFILE:
            pFile=x_fopen(xfs("%s\\%s",g_FullPath,s_IconName),"rb");
            if (!pFile)
            {
                ASSERT(FALSE);
            }
            x_fseek(pFile,0,X_SEEK_END);
            length = x_ftell(pFile);
            x_fseek(pFile,0,X_SEEK_SET);
            pBuffer = (u8 *)x_malloc(length);
            ASSERT(pBuffer);
            x_fread(pBuffer,length,1,pFile);
            x_fclose(pFile);
    
            Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], s_IconName);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEFILE_WRITEICONFILE:
        //  LOG_MESSAGE( "memcard_hardware::ProcessCreateFile", "Write icon file:%s, length:%d", s_IconName, length );
            Result = sceMc2WriteFileAsync (s_CardSocketID[m_MountedCard], s_IconName, pBuffer, 0, length);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEFILE_FREEICONBUFFER:
            x_free(pBuffer);
            CheckError(CARD_ERR_OK, TRUE);
            break;

        case STATE_CREATEFILE_WRITE:                        
            ClearIgnoreList();
            AddErrorToIgnoreList(CARD_ERR_EXISTS);
        //  LOG_MESSAGE( "memcard_hardware::ProcessCreateFile", "Create file:%s.", m_pRequestedFileName );
            Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], m_pRequestedFileName);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;
                  
        case STATE_CREATEFILE_CREATEICONSYS:
            // create and write icon file
            ClearIgnoreList();
            AddErrorToIgnoreList(CARD_ERR_EXISTS);
        //  LOG_MESSAGE( "memcard_hardware::ProcessCreateFile", "Create icon.sys" );
            Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], "icon.sys");
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEFILE_WRITEICONSYS:
        //  LOG_MESSAGE( "memcard_hardware::ProcessCreateFile", "Write icon.sys, length:%d", sizeof(s_Icon) );
            Result = sceMc2WriteFileAsync(s_CardSocketID[m_MountedCard], "icon.sys", &s_Icon, 0, sizeof(s_Icon));
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEFILE_COMPLETED:
            // Done!
            ClearIgnoreList();
            SendMessage( MSG_COMPLETE );
            break;

        default:
            // Should never get here.
            ASSERT( 0 );
            break;
    }
}

//------------------------------------------------------------------------------
enum createdir_state
{
    STATE_CREATEDIR_START = 0,
    STATE_CREATEDIR_CHDIR,
    STATE_CREATEDIR_CREATEICONFILE,
    STATE_CREATEDIR_WRITEICONFILE,
    STATE_CREATEDIR_FREEICONBUFFER,
    STATE_CREATEDIR_CREATEICONSYS,
    STATE_CREATEDIR_WRITEICONSYS,
    STATE_CREATEDIR_COMPLETED,
};

void memcard_hardware::ProcessCreateDir( void )
{
    static X_FILE  *pFile;
    static s32     length=0;
    static u8      *pBuffer=NULL;
    s32             Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Opps...
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_CREATEDIR_START:
            // Get the requested directory name
            x_strcpy( m_pRequestedDirName, m_pRequestedFileName );

            // attempt to create the directory
            ClearIgnoreList();
            Result = sceMc2MkdirAsync( s_CardSocketID[m_MountedCard], m_pRequestedDirName );
        //  LOG_MESSAGE( "memcard_hardware::ProcessCreateDir", "Directory created:%s.", m_pRequestedDirName );
            ASSERT( SCE_ERROR_ERRNO( Result ) == SCE_OK );        
            break;

        case STATE_CREATEDIR_CHDIR:
            // change to it
            Result = sceMc2ChdirAsync( s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL );
            ASSERT( SCE_ERROR_ERRNO ( Result ) == SCE_OK );                       
            break;
        
        case STATE_CREATEDIR_CREATEICONFILE:
            pFile=x_fopen( xfs("%s\\%s",g_FullPath,s_IconName),"rb");
            if( ! pFile )
            {
                ASSERT(FALSE);
            }

            x_fseek(pFile,0,X_SEEK_END);
            length = x_ftell(pFile);
            x_fseek(pFile,0,X_SEEK_SET);

            pBuffer = (u8 *)x_malloc(length);
            ASSERT(pBuffer);

            x_fread(pBuffer,length,1,pFile);
            x_fclose(pFile);
    
            Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], s_IconName);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEDIR_WRITEICONFILE:
            Result = sceMc2WriteFileAsync (s_CardSocketID[m_MountedCard], s_IconName, pBuffer, 0, length);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEDIR_FREEICONBUFFER:
            x_free(pBuffer);
            CheckError(CARD_ERR_OK, TRUE);
            break;
                 
        case STATE_CREATEDIR_CREATEICONSYS:
            // create and write icon file
            ClearIgnoreList();
            AddErrorToIgnoreList(CARD_ERR_EXISTS);
            Result = sceMc2CreateFileAsync(s_CardSocketID[m_MountedCard], "icon.sys");
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEDIR_WRITEICONSYS:
            Result = sceMc2WriteFileAsync(s_CardSocketID[m_MountedCard], "icon.sys", &s_Icon, 0, sizeof(s_Icon));
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_CREATEDIR_COMPLETED:
            // Done!
            ClearIgnoreList();
            SendMessage( MSG_COMPLETE );
            break;

        default:
            // Should never get here.
            ASSERT( 0 );
            break;
    }
}

//------------------------------------------------------------------------------
enum changedir_state
{
    STATE_CHANGEDIR_START = 0,
    STATE_CHANGEDIR_COMPLETED,
};

void memcard_hardware::ProcessSetDir( void )
{
    s32             Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Opps...
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_CHANGEDIR_START:
            // These need to be valid.
            x_strcpy( m_pRequestedDirName, m_pRequestedFileName );
            // attempt to change to requested directory
            Result = sceMc2ChdirAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL);
        //  LOG_MESSAGE( "memcard_hardware::ProcessSetDir", "Directory set to %s.", m_pRequestedDirName );
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);                       
            break;
        
        case STATE_CHANGEDIR_COMPLETED:
            // Done!
            ClearIgnoreList();
            SendMessage( MSG_COMPLETE );
            break;

        default:
            // Should never get here.
            ASSERT( 0 );
            break;
    }
}

//------------------------------------------------------------------------------
enum deletedir_state
{
    STATE_DELETEDIR_START = 0,
    STATE_DELETEDIR_READ_FILE_LIST,
    STATE_DELETEDIR_DELETE_FILE,
    STATE_DELETEDIR_CHECK_DELETE_FILE,
    STATE_DELETEDIR_DELETE_DIR,
    STATE_DELETEDIR_COMPLETED,
};

void memcard_hardware::ProcessDeleteDir( void )
{
    s32             Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Opps...
        CheckError( CARD_ERR_UNPLUGGED );
        return;
    }

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_DELETEDIR_START:
            // Get the requested directory name
            x_strcpy( m_pRequestedDirName, m_pRequestedFileName );

            // change to the requested directory
            ClearIgnoreList();
        //  LOG_MESSAGE( "memcard_hardware::ProcessDeleteDir", "Directory set to:%s.", m_pRequestedDirName );
            Result = sceMc2ChdirAsync( s_CardSocketID[m_MountedCard], m_pRequestedDirName, NULL );
            ASSERT( SCE_ERROR_ERRNO ( Result ) == SCE_OK );                       
            break;

        case STATE_DELETEDIR_READ_FILE_LIST:
            // Init file list
            m_bIsFileListValid = FALSE;
            m_nFileCount       = 0;
            s_nFiles           = 0; 
            
            // Clear the file list.
            s_FileInfo.Clear();

            ClearIgnoreList();
            AddErrorToIgnoreList( CARD_ERR_BROKEN );

            // read the directory file list from the card        
        //  LOG_MESSAGE( "memcard_hardware::ProcessSetDir", "Get Directory:*" );
            Result = sceMc2GetDirAsync(s_CardSocketID[m_MountedCard], xstring("*"), 0, MEMCARD_DIR_ENTRIES, s_DirList, &s_nFiles);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);            
            break;

        case STATE_DELETEDIR_DELETE_FILE:
            // check if broken
            if( s_WasIgnored )
            {
                // file system is damaged, attempt to delete the directory
                m_SubState = STATE_DELETEDIR_DELETE_DIR - 1;
                ClearIgnoreList();
                CheckError(CARD_ERR_OK, TRUE);
            }
            else
            {
                // is there anything to delete?
                if( s_nFiles )
                {
                    // decrement the file count
                    s_nFiles--;

                    // check for dirs
                    if( s_DirList[s_nFiles].name[0] == '.' )
                    {
                        // skip this one
                        ClearIgnoreList();
                        CheckError(CARD_ERR_OK, TRUE);
                    }
                    else
                    {
                        // delete file from directory
                    //  LOG_MESSAGE( "memcard_hardware::ProcessSetDir", "Delete file:%s.", s_DirList[s_nFiles].name );
                        Result = sceMc2DeleteAsync(s_CardSocketID[m_MountedCard], s_DirList[s_nFiles].name ); 
                        ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
                    }
                }
                else
                {
                    // nothing to delete - send process msg and increment the state
                    ClearIgnoreList();
                    CheckError(CARD_ERR_OK, TRUE);
                }
            }
            break;                        

        case STATE_DELETEDIR_CHECK_DELETE_FILE:
            // are there any more files to delete?
            if( s_nFiles )
            {
                // go back to delete another file
                m_SubState = STATE_DELETEDIR_DELETE_FILE - 1;
            }           
            // send process msg and increment the state
            ClearIgnoreList();
            CheckError(CARD_ERR_OK, TRUE);
            break;


        case STATE_DELETEDIR_DELETE_DIR:
            // delete the directory
            ClearIgnoreList();
        //  LOG_MESSAGE( "memcard_hardware::ProcessSetDir", "Delete file:%s.", m_pRequestedDirName );
            Result = sceMc2DeleteAsync(s_CardSocketID[m_MountedCard], m_pRequestedDirName);
            ASSERT(SCE_ERROR_ERRNO(Result)==SCE_OK);        
            break;

        case STATE_DELETEDIR_COMPLETED:
            // Nuke the file list.
            //InvalidateFileList();
            // Done!
            ClearIgnoreList();
            SendMessage( MSG_COMPLETE );
            break;

        default:
            // Should never get here.
            ASSERT( 0 );
            break;
    }
}

//------------------------------------------------------------------------------

void memcard_hardware::FreeIOBuffer( void )
{
}

//------------------------------------------------------------------------------

void memcard_hardware::AllocIOBuffer( void )
{
}

//------------------------------------------------------------------------------

u32 memcard_hardware::GetFreeSpace( void )
{
    return s_CardInfo[m_RequestedCard].freeClust*1024;
}
