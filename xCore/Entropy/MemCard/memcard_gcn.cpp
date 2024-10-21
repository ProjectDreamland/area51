#include "entropy.hpp"
#include "C:\Projects\Meridian\xCore\entropy\e_Memcard.hpp"
#include "\Projects\Meridian\support\frontend\frontend_mgr.hpp"
#include <dolphin.h>

//==============================================================================
//=================================== Locals ===================================
//==============================================================================

static xbool s_Initialized      = FALSE; // FLAG: Has the Init function been called?
static xbool s_InitHardware     = TRUE;  // FLAG: Initialize hardware inside the Init funtcion?
static xbool s_DispatcherActive = FALSE; // FLAG: Is the dispatcher active? (prevents re-entry).

//==============================================================================
//=================== Hardware Specific Defines, Enums, etc ====================
//==============================================================================

// Maximum number of memory cards the system supports.
#define MAX_MEMCARDS (2) 

// Size of the message queue.
#define MAX_MESSAGES (6) 

// Maximum number of files (nintendo limit).
#define MEMCARD_MAX_FILES (127)

// Valid messages for the Dispatcher.
enum memcard_message 
{
    MSG_INITIATE = 0,
    MSG_PROCESS,
    MSG_COMPLETE,
    MSG_ERROR,
};

// Make DAMN sure that error_extensions do not 
// collide with the systems native error codes.
// On GCN, all error codes are <= 0.
enum error_extensions 
{
    EXT_CARD_RESULT_INCOMPATIBLE = 1,
    EXT_CARD_RESULT_REMOVED,
};

enum memcard_file_flags 
{
    MEMCARD_FILE_EXISTS =       (1<<0),
    MEMCARD_FILE_NOPERMISSION = (1<<1),
};

// Error mapping structure.
struct error_map 
{
    memcard_error AbstractError;
    s32           HardwareError;
};

struct gcn_save_file
{
    s32     Checksum;
	char	Comment[128];
    char    Preferences[128];
	u8		Icon[2048];
};

static gcn_save_file* s_pFile = NULL; 

static xbool CheckError( s32 HardwareError, xbool bProcessOnNoError = FALSE );

//==============================================================================
//========================== Hardware Specific Data ============================
//==============================================================================

//static CARDFileInfo s_FileInfo;
static byte s_WorkArea[CARD_WORKAREA_SIZE] __attribute__((aligned(32)));

static xarray<s32> s_IgnoreList;

static CARDFileInfo s_CardFileInfo;

static CARDStat* s_FileStats;

static u32* s_FileFlags;

static xarray<mc_file_info> s_FileInfo;

static volatile xbool s_WasIgnored   = FALSE;
memcard_error         s_IgnoredError = MEMCARD_SUCCESS;

static error_map s_ErrorMap[] = {
{ MEMCARD_SUCCESS,             CARD_RESULT_READY            },   // Ready to start the next operation.
{ MEMCARD_FATAL_ERROR,         CARD_RESULT_FATAL_ERROR      },   // Error due to program design (e.g., parameter range error, etc.)
{ MEMCARD_BUSY,                CARD_RESULT_BUSY             },   // Busy
{ MEMCARD_NOT_A_MEMCARD,       CARD_RESULT_WRONGDEVICE      },   // A device is detected, but it is not a memory card. 
{ MEMCARD_NO_CARD,             CARD_RESULT_NOCARD           },   // Memory card is not detected (or not mounted yet).
{ MEMCARD_WORN_OUT,            CARD_RESULT_IOERROR          },   // Memory card has reached limit of useable life. 
{ MEMCARD_WRONG_REGION,        CARD_RESULT_ENCODING         },   // Character set encoding is mismatched.     
{ MEMCARD_DAMAGED,             CARD_RESULT_BROKEN           },   // File system is broken.
{ MEMCARD_FILE_NOT_FOUND,      CARD_RESULT_NOFILE           },   // Specified file was not found.
{ MEMCARD_FILE_ALREADY_EXISTS, CARD_RESULT_EXIST            },   // The filename about to be created/renamed already exists. 
{ MEMCARD_NO_FILES_AVAILABLE,  CARD_RESULT_NOENT            },   // No more free directory entries. 
{ MEMCARD_NOT_ENOUGH_SPACE,    CARD_RESULT_INSSPACE         },   // Insufficient free space in data blocks. 
{ MEMCARD_ACCESS_DENIED,       CARD_RESULT_NOPERM           },   // No file access permission. 
{ MEMCARD_PAST_END_OF_FILE,    CARD_RESULT_LIMIT            },   // Tried to read/write over the file size limit. 
{ MEMCARD_FILENAME_TOO_LONG,   CARD_RESULT_NAMETOOLONG      },   // The filename about to be created/renamed is too long. 
{ MEMCARD_IO_CANCELED,         CARD_RESULT_CANCELED         },   // The read/write operation is canceled.
{ MEMCARD_INCOMPATIBLE,        EXT_CARD_RESULT_INCOMPATIBLE },   // Extension - Nintendo frowns on non-8k sector cards.
{ MEMCARD_END_OF_LIST,         0                            },
};

static u8 s_CardIcon[2048] = 
{
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x84, 0x21, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x98, 0x80, 
0x80, 0x00, 0x84, 0x00, 0xb1, 0x01, 0xe2, 0x65, 0x80, 0x00, 0xb5, 0x23, 0xeb, 0x56, 0xfb, 0xb4, 
0x84, 0x00, 0x90, 0x60, 0xa0, 0xa0, 0xb1, 0x00, 0xc9, 0xa1, 0xde, 0x41, 0xe6, 0x80, 0xe2, 0x40, 
0xf7, 0x23, 0xfb, 0x23, 0xf7, 0x02, 0xf2, 0xe2, 0xfb, 0x46, 0xfb, 0x46, 0xfb, 0x46, 0xf7, 0x47, 
0xb5, 0x00, 0xb5, 0x00, 0xb1, 0x00, 0xa8, 0xc0, 0xde, 0x20, 0xde, 0x20, 0xda, 0x21, 0xcd, 0xc3, 
0xee, 0xc2, 0xee, 0xc3, 0xda, 0x03, 0xb5, 0x20, 0xf7, 0x4e, 0xe2, 0x65, 0xb0, 0xe0, 0xa8, 0xa0, 
0x98, 0x60, 0x84, 0x20, 0x80, 0x00, 0x80, 0x00, 0xbd, 0x41, 0xb5, 0x20, 0x8c, 0x20, 0x80, 0x00, 
0xa8, 0xc0, 0xac, 0xe0, 0xb1, 0x00, 0x84, 0x20, 0xa4, 0xa0, 0xa0, 0x80, 0xa0, 0xa0, 0x9c, 0x80, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x88, 0x42, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x88, 0x42, 0x84, 0x21, 0x88, 0x20, 
0x80, 0x00, 0x88, 0x42, 0x94, 0xa5, 0xad, 0x22, 0x80, 0x00, 0x84, 0x21, 0xad, 0x4a, 0xde, 0xcd, 
0xa0, 0xa0, 0xe3, 0x34, 0xe7, 0xfe, 0xfb, 0xb4, 0xd6, 0x6a, 0xf3, 0xff, 0xf3, 0xfc, 0xfb, 0xb1, 
0xef, 0xb7, 0xfb, 0xd7, 0xff, 0x8e, 0xff, 0x8e, 0xf3, 0xfc, 0xfb, 0xb1, 0xfb, 0x66, 0xfb, 0x66, 
0xfb, 0x4b, 0xfb, 0x4b, 0xfb, 0x4b, 0xfb, 0xb4, 0xfb, 0xb1, 0xfb, 0xb4, 0xf3, 0xb7, 0xf3, 0xff, 
0xff, 0x8e, 0xfb, 0xb4, 0xf3, 0xff, 0xe7, 0xfe, 0xfb, 0x67, 0xfb, 0x87, 0xf7, 0xfa, 0xd6, 0xb0, 
0xf3, 0x54, 0xb9, 0x21, 0xa8, 0xa0, 0x98, 0x60, 0xd2, 0x2a, 0xa8, 0xa0, 0x9c, 0x60, 0x98, 0x20, 
0xb9, 0x23, 0x9c, 0x60, 0x98, 0x20, 0xa0, 0x40, 0xa8, 0x80, 0x90, 0x40, 0x98, 0x20, 0xa0, 0x40, 
0x94, 0x40, 0x94, 0x40, 0x98, 0x60, 0xa0, 0xa0, 0x98, 0x20, 0x98, 0x20, 0x98, 0x40, 0x98, 0x80, 
0x9c, 0x40, 0xa0, 0x40, 0xa0, 0x40, 0x9c, 0x60, 0xa0, 0x40, 0xa0, 0x40, 0xa4, 0x40, 0xa0, 0x60, 
0x90, 0x40, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xa0, 0xa0, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0xa0, 0xa0, 0x8c, 0x40, 0x80, 0x00, 0x80, 0x00, 0xa4, 0xc0, 0x98, 0x60, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x84, 0x21, 
0x80, 0x00, 0x80, 0x00, 0xc1, 0xec, 0xfb, 0xdb, 0x80, 0x00, 0x80, 0x00, 0xc1, 0xa6, 0xfb, 0xff, 
0x84, 0x21, 0x94, 0xa5, 0xeb, 0x56, 0xfb, 0xff, 0x88, 0x42, 0xad, 0x4a, 0xef, 0x33, 0xfb, 0xdb, 
0xfb, 0xdb, 0xfb, 0xb1, 0xfb, 0x64, 0xfb, 0x66, 0xfb, 0xff, 0xff, 0xb7, 0xff, 0x8e, 0xfb, 0x67, 
0xf3, 0xff, 0xf7, 0x4e, 0xf7, 0x47, 0xf7, 0x43, 0xfb, 0xdb, 0xf7, 0x4e, 0xf3, 0x01, 0xf3, 0x01, 
0xfb, 0x64, 0xfb, 0x87, 0xf7, 0xfa, 0xb9, 0x23, 0xfb, 0x64, 0xfb, 0x8a, 0xe3, 0x34, 0xb0, 0xe0, 
0xfb, 0x41, 0xfb, 0xb1, 0xd6, 0xb0, 0xa8, 0xa0, 0xf3, 0x01, 0xf3, 0xb2, 0xd2, 0x4b, 0xa8, 0x80, 
0x9c, 0x60, 0x88, 0x20, 0x98, 0x40, 0xa0, 0x40, 0x90, 0x40, 0x80, 0x00, 0x98, 0x40, 0xa8, 0x81, 
0x8c, 0x40, 0x80, 0x00, 0x8c, 0x40, 0xb4, 0xe2, 0x84, 0x00, 0x80, 0x00, 0x88, 0x20, 0xb9, 0x23, 
0xa4, 0x60, 0xa4, 0x60, 0xa4, 0x60, 0xa4, 0x60, 0xb0, 0xa0, 0xb0, 0xc0, 0xb0, 0xa0, 0xac, 0xa0, 
0xb9, 0x00, 0xbd, 0x00, 0xbd, 0x00, 0xb8, 0xe0, 0xc5, 0x62, 0xc5, 0x40, 0xc5, 0x40, 0xc1, 0x20, 
0xac, 0xc1, 0xa0, 0xa0, 0x80, 0x00, 0x80, 0x00, 0xb5, 0x23, 0xa8, 0xe1, 0x84, 0x00, 0x80, 0x00, 
0xbd, 0x43, 0xb1, 0x02, 0x8c, 0x40, 0x80, 0x00, 0xc5, 0x62, 0xbd, 0x84, 0x94, 0x60, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x84, 0x21, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x88, 0x42, 0x90, 0x60, 0xeb, 0x0c, 0xf3, 0x72, 0x80, 0x00, 0x94, 0x60, 0xeb, 0x0c, 0xeb, 0x0c, 
0x80, 0x00, 0xa0, 0xa0, 0xe2, 0xec, 0xea, 0xc7, 0x80, 0x00, 0xa4, 0xc0, 0xe2, 0xec, 0xe6, 0x84, 
0xf7, 0x4e, 0xf7, 0x4e, 0xee, 0xc2, 0xee, 0xc1, 0xea, 0xa1, 0xef, 0x09, 0xee, 0xe4, 0xee, 0xc1, 
0xe6, 0x80, 0xea, 0xa2, 0xea, 0xc4, 0xe6, 0x80, 0xe2, 0x40, 0xe2, 0x40, 0xe6, 0x82, 0xe6, 0x80, 
0xf3, 0x02, 0xef, 0x91, 0xc9, 0xc5, 0xa4, 0x80, 0xee, 0xe3, 0xef, 0x4d, 0xc5, 0x82, 0xa0, 0x80, 
0xea, 0xc4, 0xeb, 0x0c, 0xc5, 0x62, 0xa0, 0x80, 0xe6, 0x84, 0xe7, 0x0d, 0xbd, 0x41, 0x9c, 0x60, 
0x80, 0x00, 0x80, 0x00, 0x84, 0x00, 0xb5, 0x23, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xac, 0xe3, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xa4, 0xe2, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xa4, 0xe2, 
0xcd, 0x83, 0xc9, 0x60, 0xc9, 0x60, 0xc9, 0x60, 0xd1, 0xe5, 0xc9, 0x60, 0xcd, 0x80, 0xcd, 0x80, 
0xd1, 0xe5, 0xd1, 0xa0, 0xd1, 0xa0, 0xd1, 0xa0, 0xda, 0x47, 0xd5, 0xc0, 0xd5, 0xc0, 0xd5, 0xc0, 
0xcd, 0x83, 0xc9, 0xe7, 0x98, 0x60, 0x80, 0x00, 0xd1, 0xa2, 0xd2, 0x29, 0xa0, 0xa0, 0x80, 0x00, 
0xd1, 0xe3, 0xd6, 0x6a, 0xa4, 0xc0, 0x80, 0x00, 0xda, 0x03, 0xda, 0x8b, 0xa8, 0xc0, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0xa0, 0xa1, 0xde, 0xcd, 0xde, 0x43, 0x80, 0x00, 0x98, 0x80, 0xda, 0x8b, 0xd6, 0x02, 
0x80, 0x00, 0x90, 0x60, 0xd6, 0x6b, 0xcd, 0xc3, 0x80, 0x00, 0x8c, 0x40, 0xd2, 0x4b, 0xcd, 0xc3, 
0xda, 0x20, 0xde, 0x20, 0xe2, 0x40, 0xde, 0x20, 0xd5, 0xe0, 0xd5, 0xe0, 0xda, 0x00, 0xda, 0x00, 
0xcd, 0xc1, 0xcd, 0xc1, 0xcd, 0xc1, 0xcd, 0xc1, 0xc9, 0x80, 0xc9, 0x80, 0xc9, 0x80, 0xc9, 0x80, 
0xe6, 0x84, 0xe2, 0xec, 0xb5, 0x20, 0x9c, 0x60, 0xde, 0x43, 0xe2, 0xec, 0xb9, 0x42, 0x9c, 0x60, 
0xd6, 0x02, 0xde, 0xcd, 0xb9, 0x42, 0xa0, 0x80, 0xcd, 0xc3, 0xde, 0xcd, 0xb9, 0x42, 0xa4, 0x80, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xa4, 0xe2, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xac, 0xe3, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xad, 0x24, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xb5, 0x65, 
0xda, 0x48, 0xd9, 0xe0, 0xd9, 0xe0, 0xd9, 0xe0, 0xda, 0x48, 0xde, 0x00, 0xde, 0x01, 0xde, 0x00, 
0xda, 0x47, 0xde, 0x01, 0xde, 0x21, 0xde, 0x20, 0xda, 0x47, 0xde, 0x21, 0xde, 0x41, 0xe2, 0x21, 
0xda, 0x03, 0xda, 0x8b, 0xa8, 0xe0, 0x80, 0x00, 0xde, 0x43, 0xde, 0x89, 0xa8, 0xc0, 0x80, 0x00, 
0xe2, 0x65, 0xde, 0x89, 0xa0, 0xa0, 0x80, 0x00, 0xe2, 0x65, 0xda, 0x48, 0x98, 0x80, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x88, 0x20, 0xc1, 0xc8, 0xc9, 0xc5, 0x80, 0x00, 0x84, 0x00, 0xb9, 0x66, 0xc9, 0xe7, 
0x80, 0x00, 0x80, 0x00, 0xa4, 0xe2, 0xc9, 0xe9, 0x80, 0x00, 0x80, 0x00, 0x94, 0x40, 0xc9, 0xe9, 
0xc1, 0x60, 0xc5, 0x60, 0xc1, 0x60, 0xc1, 0x60, 0xc1, 0x40, 0xc1, 0x40, 0xbd, 0x20, 0xbd, 0x20, 
0xbd, 0x20, 0xbd, 0x20, 0xb9, 0x21, 0xb9, 0x00, 0xbd, 0x21, 0xb8, 0xe0, 0xb4, 0xe0, 0xb4, 0xe0, 
0xc9, 0x81, 0xde, 0xcd, 0xbd, 0x43, 0xa8, 0x80, 0xc5, 0x61, 0xd6, 0x6b, 0xbd, 0x84, 0xa8, 0x80, 
0xbd, 0x20, 0xd2, 0x29, 0xc1, 0xa7, 0xa8, 0x80, 0xb4, 0xe0, 0xc9, 0xe7, 0xc9, 0xe9, 0xa8, 0x80, 
0x84, 0x00, 0x80, 0x00, 0x84, 0x20, 0xc1, 0xc8, 0x8c, 0x20, 0x80, 0x00, 0x8c, 0x40, 0xd2, 0x6b, 
0x90, 0x40, 0x80, 0x00, 0x9c, 0xa0, 0xde, 0xcd, 0x9c, 0x60, 0x80, 0x00, 0xc1, 0xc8, 0xeb, 0x0c, 
0xe2, 0x65, 0xde, 0x41, 0xe2, 0x62, 0xe2, 0x21, 0xe2, 0x65, 0xe2, 0x62, 0xe2, 0x62, 0xe2, 0x62, 
0xe6, 0x84, 0xea, 0xa2, 0xea, 0xa2, 0xea, 0xa2, 0xea, 0xc2, 0xee, 0xe3, 0xee, 0xc2, 0xea, 0xc4, 
0xe6, 0xa7, 0xd2, 0x27, 0x94, 0x60, 0x80, 0x00, 0xe6, 0xa7, 0xbd, 0x84, 0x90, 0x40, 0x80, 0x00, 
0xde, 0x89, 0xb1, 0x01, 0x88, 0x20, 0x80, 0x00, 0xd2, 0x27, 0xa4, 0xa0, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x88, 0x00, 0xbd, 0xaa, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0xa4, 0xe5, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x8c, 0x40, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0xc1, 0x64, 0xb4, 0xc0, 0xb4, 0xc0, 0xb0, 0xc0, 0xc5, 0xa8, 0xb4, 0xc0, 0xb0, 0xc0, 0xb0, 0xc0, 
0xbd, 0xaa, 0xb8, 0xe2, 0xb0, 0xa0, 0xac, 0xa0, 0xa4, 0xe5, 0xc1, 0xa7, 0xac, 0x80, 0xa8, 0x80, 
0xb4, 0xc0, 0xc1, 0x64, 0xd2, 0x4b, 0xa8, 0xa1, 0xb0, 0xc0, 0xb8, 0xe0, 0xd2, 0x2a, 0xb1, 0x24, 
0xac, 0xa0, 0xb0, 0xa0, 0xc1, 0xa6, 0xc1, 0xc8, 0xa8, 0x80, 0xa8, 0x80, 0xac, 0xc0, 0xc1, 0xa7, 
0xa4, 0x60, 0xa4, 0xe5, 0xeb, 0xba, 0xef, 0x05, 0xa0, 0x40, 0xc1, 0xa7, 0xef, 0x09, 0xf3, 0x03, 
0x9c, 0x60, 0xa8, 0xa1, 0xcd, 0xc1, 0xea, 0xc4, 0xb1, 0x02, 0x9c, 0x60, 0xa8, 0xa1, 0xbd, 0x41, 
0xf3, 0x02, 0xf3, 0x03, 0xef, 0x05, 0xf3, 0x72, 0xf3, 0x23, 0xf3, 0x04, 0xeb, 0xba, 0xe3, 0x7a, 
0xee, 0xc3, 0xea, 0xc7, 0xeb, 0xba, 0xbd, 0xaa, 0xc5, 0x82, 0xbd, 0x43, 0xac, 0xe3, 0x9c, 0x60, 
0xc1, 0xa7, 0x9c, 0x60, 0x80, 0x00, 0x80, 0x00, 0xa8, 0xa1, 0x90, 0x40, 0x80, 0x00, 0x80, 0x00, 
0x9c, 0x60, 0x84, 0x00, 0x80, 0x00, 0x80, 0x00, 0x94, 0x40, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x84, 0x00, 0xb1, 0x47, 0xb9, 0x23, 0xa4, 0x60, 0x80, 0x00, 0x88, 0x20, 0xb1, 0x24, 0xb8, 0xe1, 
0x80, 0x00, 0x80, 0x00, 0x84, 0x00, 0xa0, 0x81, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0xa4, 0x60, 0xa4, 0x60, 0xa0, 0x40, 0xa4, 0xa1, 0xac, 0x80, 0xa4, 0x40, 0xa0, 0x40, 0xa0, 0x40, 
0xb8, 0xe0, 0xc0, 0xe0, 0xb0, 0xa0, 0xac, 0x80, 0x88, 0x00, 0xa0, 0x80, 0xb8, 0xe0, 0xb8, 0xe0, 
0xb1, 0x02, 0xa4, 0xa1, 0xa0, 0x60, 0xa0, 0x60, 0xa0, 0x60, 0xa4, 0xa1, 0xa4, 0xa1, 0xa4, 0x80, 
0xa8, 0x80, 0xa4, 0x60, 0xa8, 0x80, 0xa8, 0x80, 0xb8, 0xe0, 0xb9, 0x00, 0xbd, 0x20, 0xbd, 0x00, 
0xa0, 0x80, 0xa0, 0x60, 0xa0, 0x40, 0xa0, 0x60, 0xa0, 0x60, 0xa4, 0x80, 0xac, 0xc0, 0x8c, 0x20, 
0xac, 0xc0, 0xb4, 0xe0, 0x90, 0x40, 0x80, 0x00, 0xac, 0xc0, 0x88, 0x20, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 
0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00, 0x80, 0x00
};

char s_CardComment1[32]= "The Hobbit";
char s_CardComment2[32]= "Savegames and Settings";
char s_CardComment1_GER[32]= "The Hobbit";
char s_CardComment2_GER[32]= "Savegames and Settings";
char s_CardComment1_FRE[32]= "The Hobbit";
char s_CardComment2_FRE[32]= "Savegames and Settings";
char s_CardComment1_ITA[32]= "The Hobbit";
char s_CardComment2_ITA[32]= "Savegames and Settings";
char s_CardComment1_SPA[32]= "The Hobbit";
char s_CardComment2_SPA[32]= "Savegames and Settings";

//==============================================================================
//============================= Hardware Functions =============================
//==============================================================================

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

static xbool ErrorWasIgnored( void )
{
    return s_WasIgnored;
}

//------------------------------------------------------------------------------

static void BailOnIgnoredError( void )
{
    // Was an error ignored?
    if( s_WasIgnored )
    {
        // Set the error code.
        g_MemcardHardware.SetState( s_IgnoredError );

        // Dear John...
        g_MemcardHardware.SendMessage( MSG_ERROR );
    }
}

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
            // CARD_RESULT_READY == no error occured.
            if( (pTable->HardwareError == CARD_RESULT_READY) || ErrorIsIgnored( pTable->HardwareError ) )
            {
                // Was error ignored?
                if( pTable->HardwareError != CARD_RESULT_READY )
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

static void gcn_MountDetachCallback( s32 Channel, s32 Result )
{
    // No warnings!
    (void)Channel;
    (void)Result;
    
    // It's not mounted anymore...
    g_MemcardHardware.SetMountedCard( NO_CARD );

    // No file length
    g_MemcardHardware.SetFileLength( -1 );

    // Invalidate the file list.
    g_MemcardHardware.InvalidateFileList();
}

//------------------------------------------------------------------------------

static void gcn_DefaultCallback( s32 Channel, s32 Result )
{
    // No warnings!
    (void)Channel;

    // If no error, then continue.
    CheckError( Result, TRUE );
}

//------------------------------------------------------------------------------

static void gcn_WriteFileCallback( s32 Channel, s32 Result )
{
    // No warnings!
    (void)Channel;

    // Nuke the file.
    if( s_pFile )
    {
        x_free( s_pFile );
        s_pFile = NULL;
    }

    // If no error, then continue.
    CheckError( Result, TRUE );
}

//------------------------------------------------------------------------------

static void gcn_ReadFileCallback( s32 Channel, s32 Result )
{
    // No warnings!
    (void)Channel;

    // All good?
    if( Result != CARD_RESULT_READY )
    {
        // Nuke the file.
        if( s_pFile )
        {
            x_free( s_pFile );
            s_pFile = NULL;
        }
    }

    // If no error, then continue.
    CheckError( Result, TRUE );
}

//==============================================================================
//================================= Dispatcher =================================
//==============================================================================

//------------------------------------------------------------------------------

void MemcardDispatcher( void )
{
    // Error check.
    ASSERT( !s_DispatcherActive );

    // Set semaphore.
    s_DispatcherActive = TRUE;

    // Do this a whole lot....
    while( 1 )
    {   
        // So lonely...waiting by the phone for someone to call!!!!
        memcard_message Message = (memcard_message)((s32)g_MemcardHardware.m_pDispatcherMQ->Recv( MQ_BLOCK ));

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
    // Error check.
    ASSERT( !s_Initialized );

    // Can only init the hardware once on the GameCube.
    if( s_InitHardware )
    {
        // Initialize the card system.
        CARDInit();

        // Don't do it again...
        s_InitHardware = FALSE;
    }

    m_pDispatcherMQ      = new xmesgq(MAX_MESSAGES);
    m_Error              = MEMCARD_SUCCESS;
    m_Operation          = MEMCARD_OP_IDLE;
    m_RequestedCard      = NO_CARD;
    m_pRequestedFileName = NULL;
    m_pRequestedBuffer   = NULL;
    m_nRequestedBytes    = 0;
    m_RequestedOffset    = 0;
    m_MountedCard        = NO_CARD;
    m_SectorSize         = 0;
    m_FileLength         = -1;

    // Don't ignore any errors.
    ClearIgnoreList();

    // Allocate file stat buffer
    s_FileStats = (CARDStat*)x_malloc( MEMCARD_MAX_FILES * sizeof(CARDStat) );
    ASSERT( s_FileStats );
    x_memset( s_FileStats, 0, MEMCARD_MAX_FILES * sizeof(CARDStat) );

    // Allocate file flag buffer
    s_FileFlags = (u32*)x_malloc( MEMCARD_MAX_FILES * sizeof(u32) );
    ASSERT( s_FileFlags );
    x_memset( s_FileFlags, 0, MEMCARD_MAX_FILES * sizeof(u32) );

    // Nuke the fileinfo.
    s_FileInfo.Clear();

    // TODO: Allocate the work area (currently static).
/*
    {
        X_FILE* f = x_fopen( "icon1.tpl", "rb" );
        if( f )
        {
            x_fseek( f, 64, X_SEEK_SET );
            x_fread( s_CardIcon, 2048, 1, f );
            x_fclose( f );

            f = x_fopen( "icon.txt", "w+t" );
            if( f )
            {
                for( s32 i=0 ; i<2048 ; i++ )
                {
                    if( (i & 15) == 0 )
                    {
                        x_fprintf( f, "\n" );
                    }
                    x_fprintf( f, "0x%02x, ", s_CardIcon[i] );
                }

                x_fclose( f );
            }
        }
    }
*/

    // Create the memcard_hardware thread.
    m_pThread = new xthread( MemcardDispatcher, "memcard_mgr dispatcher", 8192, 1 );


    // Ok all good now!
    s_Initialized = TRUE;
}

//------------------------------------------------------------------------------

void memcard_hardware::Kill( void )
{
    // Error check.
    ASSERT( s_Initialized );

    // Nuke the thread.
    delete m_pThread;
    delete m_pDispatcherMQ;

    s_DispatcherActive = FALSE;

    // Free the cardstat buffer.
    if( s_FileStats )
    {
        x_free( s_FileStats );
        s_FileStats = NULL;
    }

    // Free the file flag buffer.
    if( s_FileFlags )
    {
        x_free( s_FileFlags );
        s_FileFlags = NULL;
    }

    // Nuke the fileinfo.
    s_FileInfo.Clear();
    s_FileInfo.FreeExtra();

    // TODO: Free the work area (currently static).

    // All bad now...
    s_Initialized = FALSE;
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

    m_pRequestedFileName = FileName;
    m_pRequestedBuffer   = pBuffer;
    m_nRequestedBytes    = nBytes;
    m_RequestedOffset    = Offset;
}

//------------------------------------------------------------------------------

void memcard_hardware::SetOperation( memcard_op Operation )
{
    // Error check.
    ASSERT( s_Initialized );

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

    // Free the IO Buffer!
    FreeIOBuffer();

    // We have ignition...er or something like that...
    SendMessage( MSG_INITIATE );
}

//------------------------------------------------------------------------------

s32 memcard_hardware::GetFileList( xarray<xstring>& FileList )
{
    FileList.Clear();

    if( m_bIsFileListValid )
    {
        FileList.SetCapacity( m_nFileCount );

        for( s32 i=0 ; i<m_nFileCount ; i++ )
        {
            FileList.Append() = s_FileInfo[i].FileName;
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

        case MEMCARD_OP_IDLE:
        default:
            // Um, how did this happen?
            ASSERT( 0 );
            break;
    }
}

//------------------------------------------------------------------------------

enum mount_state {
STATE_MOUNT_START = 0,
STATE_MOUNT_CHECK,
STATE_MOUNT_GET_SECTOR_SIZE,
STATE_MOUNT_COMPLETED,
};

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
            InvalidateFileList();

            // Ignore these errors for this operation.
            AddErrorToIgnoreList( CARD_RESULT_ENCODING );
            AddErrorToIgnoreList( CARD_RESULT_BROKEN );

            // Now, no card is mounted...
//            ASSERT( GetMountedCard() == NO_CARD );  // <jhowa> I am handleing this on the UI end..
            SetMountedCard( NO_CARD );

            // Attempt to mount the card.
            Result = CARDMountAsync( m_RequestedCard, s_WorkArea, gcn_MountDetachCallback, gcn_DefaultCallback );
            
            // Just error check (callback will process).
            CheckError( Result );
            break;

        case STATE_MOUNT_CHECK:
            // Was an error ignored?
            if( ErrorWasIgnored() )
            {
                // Set the mounted card.
                SetMountedCard( m_RequestedCard );

                // Bail out now.
                BailOnIgnoredError();
            }
            else
            {
                // Check the card.
                Result = CARDCheckAsync( m_RequestedCard, gcn_DefaultCallback );

                // Just error check (callback will process).
                CheckError( Result );
            }
            break;

        case STATE_MOUNT_GET_SECTOR_SIZE:
            // Get the sector size.
            Result = CARDGetSectorSize( m_RequestedCard, &m_SectorSize );

            // Just error check (if card is compatible, we will process manually below).
            if( !CheckError( Result ) )
            {
                // Nintendo frowns on non-8k cards...
                if( m_SectorSize != 8192 )
                {
                    // Too bad, so sad...
                    CheckError( EXT_CARD_RESULT_INCOMPATIBLE );
                }
                else
                {
                    // Next!
                    SendMessage( MSG_PROCESS );
                }
            }
            break;

        case STATE_MOUNT_COMPLETED:
            // Set the mounted card.
            SetMountedCard( m_RequestedCard );

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

enum unmount_state {
STATE_UNMOUNT_START = 0,
STATE_UNMOUNT_COMPLETED,
};

void memcard_hardware::ProcessUnmount( void )
{
    s32 Result;

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_UNMOUNT_START:
            if( m_MountedCard == NO_CARD )
            {
                // No card is mounted...
                CheckError( CARD_RESULT_NOCARD );
            }
            else
            {
                // No file length.
                SetFileLength( -1 );

                // Nuke the file list.
                InvalidateFileList();

                // Attempt to unmount the card.
                Result = CARDUnmount( m_MountedCard );
            
                // No card is mounted.
                m_MountedCard = NO_CARD;

                // Check error and process.
                CheckError( Result, TRUE );
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

enum readfile_state {
STATE_READFILE_START = 0,
STATE_READFILE_READ,
STATE_READFILE_CLOSE,
STATE_READFILE_COMPLETED,
};

void memcard_hardware::ProcessReadFile( void )
{
    s32 Result;
    static s32 AlignedLength;
    static s32 TotalSize;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // No memory leaks!
        if( s_pFile )
        {
            x_free( s_pFile );
            s_pFile = NULL;
        }

        // Oops...bad.
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedFileName );
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes );

    // Calculate total file size.
    TotalSize = sizeof(gcn_save_file) + m_nRequestedBytes;

    // Calculate the aligned length.
    AlignedLength = (TotalSize + m_SectorSize-1) & ~(m_SectorSize-1);

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_READFILE_START:
            // Open the file.
	        Result = CARDOpen( m_MountedCard, (char*)m_pRequestedFileName, &s_CardFileInfo );

            // Error check and process.
            CheckError( Result, TRUE );
            break;
        
        case STATE_READFILE_READ:
            // Allocate memory for the save file.
            s_pFile = (gcn_save_file*)x_malloc( AlignedLength );
            x_memset( s_pFile, 0, AlignedLength );

            // Read the data
            Result = CARDReadAsync( &s_CardFileInfo, s_pFile, AlignedLength, 0, gcn_ReadFileCallback );
            
            // Just error check (callback will process and free s_pFile if there is an error).
            CheckError( Result );
            break;

        case STATE_READFILE_CLOSE:
            // Copy the data to the users buffer.
            x_memcpy( m_pRequestedBuffer, (s_pFile+1), m_nRequestedBytes );

            // Free up the memory.
            if( s_pFile )
            {
                x_free( s_pFile );
                s_pFile = NULL;
            }

            // Close the file.
            Result = CARDClose( &s_CardFileInfo );

            // Error check and process.
            CheckError( Result, TRUE );
            break;

        case STATE_READFILE_COMPLETED:
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

enum writefile_state {
STATE_WRITEFILE_START = 0,
STATE_WRITEFILE_SET_STAT,
STATE_WRITEFILE_WRITE,
STATE_WRITEFILE_CLOSE,
STATE_WRITEFILE_COMPLETED,
};

void memcard_hardware::ProcessWriteFile( void )
{
    s32      Result;
    s32      AlignedLength;
	s32      TotalSize;
    CARDStat Stat;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // No memory leaks!
        if( s_pFile )
        {
            x_free( s_pFile );
            s_pFile = NULL;
        }

        // Opps...
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedFileName );
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes );

    // Get total file size.
    TotalSize = sizeof(gcn_save_file) + m_nRequestedBytes;

    // Align it up!
    AlignedLength = (TotalSize + m_SectorSize-1) & ~(m_SectorSize-1);

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_WRITEFILE_START:
            // Try to create the file the file.
            Result = CARDCreateAsync( m_MountedCard, (char*)m_pRequestedFileName, AlignedLength, &s_CardFileInfo, gcn_DefaultCallback );

            // Just error check (callback will process).
            CheckError( Result );
            break;

        case STATE_WRITEFILE_SET_STAT:
            // Nuke the file list.
            InvalidateFileList();

            // Clear the status.
           	x_memset( &Stat, 0, sizeof(Stat) );

            // Set the card stat.
	        CARDSetCommentAddress( &Stat, (s32)&s_pFile->Comment - (s32)s_pFile );
	        CARDSetIconAddress   ( &Stat, (s32)&s_pFile->Icon - (s32)s_pFile );
	        CARDSetBannerFormat  ( &Stat, CARD_STAT_BANNER_NONE );
	        CARDSetIconAnim      ( &Stat, CARD_STAT_ANIM_LOOP );
	        CARDSetIconFormat    ( &Stat, 0, CARD_STAT_ICON_RGB5A3 );
	        CARDSetIconSpeed     ( &Stat, 0, CARD_STAT_SPEED_SLOW );
	        CARDSetIconSpeed     ( &Stat, 1, CARD_STAT_SPEED_END ); // <<== Terminate the icon
            
            // Write it to the memcard.
            Result = CARDSetStatusAsync( m_MountedCard, s_CardFileInfo.fileNo, &Stat, gcn_DefaultCallback );
                        
            // Just error check (callback will process).
            CheckError( Result );
            break;

        case STATE_WRITEFILE_WRITE:
            // Allocate memory for the save file.
            s_pFile = (gcn_save_file*)x_malloc( AlignedLength );
            x_memset( s_pFile, 0, AlignedLength );

            // Set up the file now.
            s_pFile->Checksum = 0; // TODO: Calculate the Checksum.

            if( !x_strcmp( g_FrontEndMgr.GetLanguage(), "ENG" ) )
            {
                x_strcpy( &s_pFile->Comment[ 0], s_CardComment1 );
                x_strcpy( &s_pFile->Comment[32], s_CardComment2 );
            }
            else if( !x_strcmp( g_FrontEndMgr.GetLanguage(), "GER" ) )
            {
                x_strcpy( &s_pFile->Comment[ 0], s_CardComment1_GER );
                x_strcpy( &s_pFile->Comment[32], s_CardComment2_GER );
            }
            else if( !x_strcmp( g_FrontEndMgr.GetLanguage(), "FRE" ) )
            {
                x_strcpy( &s_pFile->Comment[ 0], s_CardComment1_FRE );
                x_strcpy( &s_pFile->Comment[32], s_CardComment2_FRE );
            }
            else if( !x_strcmp( g_FrontEndMgr.GetLanguage(), "ITA" ) )
            {
                x_strcpy( &s_pFile->Comment[ 0], s_CardComment1_ITA );
                x_strcpy( &s_pFile->Comment[32], s_CardComment2_ITA );
            }
            else if( !x_strcmp( g_FrontEndMgr.GetLanguage(), "SPA" ) )
            {
                x_strcpy( &s_pFile->Comment[ 0], s_CardComment1_SPA );
                x_strcpy( &s_pFile->Comment[32], s_CardComment2_SPA );
            }
            else if( !x_strcmp( g_FrontEndMgr.GetLanguage(), "JAP" ) )
            {
                x_strcpy( &s_pFile->Comment[ 0], s_CardComment1_SPA );
                x_strcpy( &s_pFile->Comment[32], s_CardComment2_SPA );
            }
            else if( !x_strcmp( g_FrontEndMgr.GetLanguage(), "RUS" ) )
            {
                x_strcpy( &s_pFile->Comment[ 0], s_CardComment1_SPA );
                x_strcpy( &s_pFile->Comment[32], s_CardComment2_SPA );
            }




            x_memcpy( s_pFile->Icon, s_CardIcon, 2048 );
            // TODO: Copy the preferences.

            // Now copy the game data.
            x_memcpy( (s_pFile+1), m_pRequestedBuffer, m_nRequestedBytes );

            // Write the file.
            Result = CARDWriteAsync( &s_CardFileInfo, s_pFile, AlignedLength, 0, gcn_WriteFileCallback );

            // Just error check (callback will process and free s_pFile).
            CheckError( Result );
            break;

        case STATE_WRITEFILE_CLOSE:
            // Close the file.
            Result = CARDClose( &s_CardFileInfo );

            // Nuke the fileinfo.
            x_memset( &s_CardFileInfo, 0, sizeof(s_CardFileInfo) );

            // Error check and process.
            CheckError( Result, TRUE );
            break;

        case STATE_WRITEFILE_COMPLETED:
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

enum deletefile_state {
STATE_DELETEFILE_START = 0,
STATE_DELETEFILE_COMPLETED,
};

void memcard_hardware::ProcessDeleteFile( void )
{
    s32 Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // Gotta be valid.
    ASSERT( m_pRequestedFileName );    

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_DELETEFILE_START:
            // Wave bye bye...
            Result = CARDDeleteAsync( m_MountedCard, (char*)m_pRequestedFileName, gcn_DefaultCallback );

            // Just error check (callback will process).
            CheckError( Result );
            break;

        case STATE_DELETEFILE_COMPLETED:
            // Nuke the file list.
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

enum format_state {
STATE_FORMAT_START = 0,
STATE_FORMAT_COMPLETED,
};

void memcard_hardware::ProcessFormat( void )
{
    s32 Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_FORMAT_START:
            // No file length.
            SetFileLength( -1 );

            // Invalidate the file list.
            InvalidateFileList();

            // Wave bye bye...
            Result = CARDFormatAsync( m_MountedCard, gcn_DefaultCallback );
        
            // Just error check (callback will process).
            CheckError( Result );
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

enum readfilelist_state {
STATE_READFILELIST_START = 0,
};

void memcard_hardware::ProcessReadFileList( void )
{
    s32 Result;
    s32 i;
    s32 j;
    s32 nFiles;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_READFILELIST_START:
            // Init file list
            m_bIsFileListValid = FALSE;
            m_nFileCount       = 0;
            nFiles             = 0; 

            // Clear the file list.
            s_FileInfo.Clear();

            // 127 files available on the GameCube.
            for( i=0 ; i<MEMCARD_MAX_FILES ; i++ )
            {
                // Get the status of this file.
                Result = CARDGetStatus( m_MountedCard, i, &s_FileStats[i] );
                switch( Result )
                {
                    case CARD_RESULT_READY:
                        // Have a file!
                        s_FileFlags[i] = MEMCARD_FILE_EXISTS;
                        nFiles++;
                        break;

                    case CARD_RESULT_NOFILE:
                        // No file in this slot.
                        s_FileFlags[i] = 0;
                        break;

                    case CARD_RESULT_NOPERM:
                        // Don't have permissions on this file.
                        s_FileFlags[i] = MEMCARD_FILE_EXISTS+MEMCARD_FILE_NOPERMISSION;
                        break;

                    default:
                        // Musta been an error!
                        CheckError( Result );
                        return;
                        break;
                }
            }

            // Make space for the files.
            s_FileInfo.SetCapacity( nFiles );

            // Now fill in the file info data structures
            for( i=0,j=0 ; i<MEMCARD_MAX_FILES ; i++ )
            {
                if( s_FileFlags[i] == MEMCARD_FILE_EXISTS )
                {
                    ASSERT( j < nFiles );
                    s_FileInfo.Append();
                    x_strcpy( s_FileInfo[j].FileName, s_FileStats[i].fileName );
                    s_FileInfo[j].Length   = s_FileStats[i].length;
                    s_FileInfo[j].Index    = i;
                    j++;
                }
            }

            // Init file list
            m_nFileCount       = nFiles;
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

enum purgefilelist_state {
STATE_PURGEFILELIST_START = 0,
};

void memcard_hardware::ProcessPurgeFileList( void )
{
    s32 Result;
    (void)Result;

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

enum getfilelength_state {
STATE_GETFILELENGTHLIST_START = 0,
};

void memcard_hardware::ProcessGetFileLength( void )
{
    s32      Result;
    s32      i;
    CARDStat Stat;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    switch( m_SubState )
    {
        case STATE_GETFILELENGTHLIST_START:
            // No file length.
            SetFileLength( -1 );

            // Open the file.
	        Result = CARDOpen( m_MountedCard, (char*)m_pRequestedFileName, &s_CardFileInfo );
            if( CheckError( Result ) )
                return;

            // Get the file number.
            i = s_CardFileInfo.fileNo;

            // Close the file
            Result = CARDClose( &s_CardFileInfo );
            if( CheckError( Result ) )
                return;

            // Get the status.
            Result = CARDGetStatus( m_MountedCard, i, &Stat ); 
            if( CheckError( Result ) )
                return;

            // Set the length (ignore the header)
            SetFileLength( Stat.length - sizeof(gcn_save_file) );

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
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // Not implemented yet.
    ASSERT( 0 );
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

xbool memcard_hardware::IsCardConnected( s32 CardID )
{
    xbool IsConnected = FALSE;

    // Error check.
    ASSERT( s_Initialized );

    // Is a card connected?
    s32 Result = CARDProbeEx( CardID, NULL, NULL );

    // Determine what to do...
    switch( Result )
    {
        case CARD_RESULT_FATAL_ERROR:
            // Error due to program design (bad parameters, etc...).
            ASSERT( 0 );
            break;

        case CARD_RESULT_BUSY:
            // Try again later.
            break;

        case CARD_RESULT_NOCARD:
            // No object is inserted.
            break;

        case CARD_RESULT_WRONGDEVICE:
            // Found a non-memory card device.
            break;

        case CARD_RESULT_READY:
            // Found a memory card.
            IsConnected = TRUE;
            break;
    }

    // Tell the world.
    return IsConnected;
}

//------------------------------------------------------------------------------

static char* s_pIOBuffer    = NULL;
static s32   s_nBytesToRead = 0;
static s32   s_FirstByte    = 0;
static s32   s_Sector       = 0;
static s32   s_SectorByte   = 0;
static s32   s_BufferOffset = 0;

enum read_state {
STATE_READ_START = 0,
STATE_READ_FIRST,
STATE_READ_MIDDLE,
STATE_READ_COMPLETED,
};

void memcard_hardware::ProcessRead( void )
{
    s32 Result;
    s32 nBytesToCopy;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Oops...bad.
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedFileName );
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes > 0 );
    ASSERT( m_RequestedOffset >= 0 );

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_READ_START:
            // Allocate the IO Buffer.
            AllocIOBuffer();

            // Note how many bytes are left to read.
            s_nBytesToRead = m_nRequestedBytes;

            // Calculate first byte to read.
            s_FirstByte = sizeof(gcn_save_file) + m_RequestedOffset;

            // Calculate initial sector to read. 
            s_Sector = s_FirstByte / m_SectorSize;

            // Calculate byte offset within the sector
            s_SectorByte = s_FirstByte % m_SectorSize;

            // Initialize the buffer offset.
            s_BufferOffset = 0;

            // Open the file.
	        Result = CARDOpen( m_MountedCard, (char*)m_pRequestedFileName, &s_CardFileInfo );

            // Error check and process.
            CheckError( Result, TRUE );
            break;

        case STATE_READ_FIRST:
            // Read the data
            Result = CARDReadAsync( &s_CardFileInfo, s_pIOBuffer, m_SectorSize, s_Sector * m_SectorSize, gcn_ReadFileCallback );
            
            // Just error check (callback will process).
            CheckError( Result );
            break;

        case STATE_READ_MIDDLE:
            // Calculate number of bytes to copy.
            nBytesToCopy = m_SectorSize - s_SectorByte;
            
            // We don't want to copy too much data!
            if( nBytesToCopy > s_nBytesToRead )
                nBytesToCopy = s_nBytesToRead;

            // Copy the data.
            ASSERT( nBytesToCopy > 0 );
            ASSERT( nBytesToCopy <= (s32)m_SectorSize );
            x_memcpy( m_pRequestedBuffer + s_BufferOffset, s_pIOBuffer + s_SectorByte, nBytesToCopy );

            // Bump offset!
            s_BufferOffset += nBytesToCopy;

            // Next sector!
            s_Sector++;

            // Keep track of how much we still need to read.
            s_nBytesToRead -= nBytesToCopy;

            // No more partial reads after the first one!
            s_SectorByte = 0;

            // Any more left to read?
            if( s_nBytesToRead > 0 )
            {
                // Repeat same state!
                m_SubState--;

                // Read more.
                Result = CARDReadAsync( &s_CardFileInfo, s_pIOBuffer, m_SectorSize, s_Sector * m_SectorSize, gcn_ReadFileCallback );
                
                // Just error check (callback will process).
                CheckError( Result );
            }
            else
            {
                // Close the file.
                Result = CARDClose( &s_CardFileInfo );

                // Free up the IO buffer.
                FreeIOBuffer();

                // Error check and process.
                CheckError( Result, TRUE );
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

static s32 s_nBytesToWrite = 0;
static s32 s_nBytesToCopy = 0;

enum write_state {
STATE_WRITE_START = 0,
STATE_WRITE_PRE_READ,
STATE_WRITE_WRITE,
STATE_WRITE_COMPLETED,
};

void memcard_hardware::ProcessWrite( void )
{
    s32 Result;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // Opps...
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedFileName );
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes > 0 );
    ASSERT( m_RequestedOffset >= 0 );

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_WRITE_START:
            // Allocate the IO Buffer.
            AllocIOBuffer();

            // Note how many bytes are left to write.
            s_nBytesToWrite = m_nRequestedBytes;

            // Calculate first byte to write too.
            s_FirstByte = sizeof(gcn_save_file) + m_RequestedOffset;

            // Calculate initial sector to read (and write). 
            s_Sector = s_FirstByte / m_SectorSize;

            // Calculate byte offset within the sector.
            s_SectorByte = s_FirstByte % m_SectorSize;

            // Initialize the buffer offset.
            s_BufferOffset = 0;

            // Open the file.
	        Result = CARDOpen( m_MountedCard, (char*)m_pRequestedFileName, &s_CardFileInfo );

            // Error check and process.
            CheckError( Result, TRUE );
            break;

        case STATE_WRITE_PRE_READ:
            // Calculate number of bytes to copy.
            s_nBytesToCopy = m_SectorSize - s_SectorByte;
            
            // We don't want to copy too much data!
            if( s_nBytesToCopy > s_nBytesToWrite )
                s_nBytesToCopy = s_nBytesToWrite;

            // Partial write?
            if( s_nBytesToCopy < (s32)m_SectorSize )
            {
                // Need to fill buffer from file.
                Result = CARDReadAsync( &s_CardFileInfo, s_pIOBuffer, m_SectorSize, s_Sector * m_SectorSize, gcn_ReadFileCallback );
            
                // Just error check (callback will process).
                CheckError( Result );
            }
            else
            {
                // No need to pre-read, so do the write!
                g_MemcardHardware.SendMessage( MSG_PROCESS );
            }
            break;

        case STATE_WRITE_WRITE:
            // Any more left to write?
            if( s_nBytesToWrite > 0 )
            {
                // Copy the data into the io buffer.
                ASSERT( s_nBytesToCopy > 0 );
                ASSERT( s_nBytesToCopy <= (s32)m_SectorSize );
                x_memcpy( s_pIOBuffer + s_SectorByte, m_pRequestedBuffer + s_BufferOffset, s_nBytesToCopy );

                // No more partials after the first one!
                s_SectorByte = 0;

                // Bump offset!
                s_BufferOffset += s_nBytesToCopy;

                // Keep track of how much we still need to write.
                s_nBytesToWrite -= s_nBytesToCopy;

                // Next state will be the pre-read.
                m_SubState = STATE_WRITE_PRE_READ-1;

                // Write the buffer to the card.
                Result = CARDWriteAsync( &s_CardFileInfo, s_pIOBuffer, m_SectorSize, s_Sector * m_SectorSize, gcn_ReadFileCallback );
                
                // Next sector!
                s_Sector++;

                // Just error check (callback will process).
                CheckError( Result );
            }
            else
            {
                // Close the file.
                Result = CARDClose( &s_CardFileInfo );

                // Free up the IO buffer.
                FreeIOBuffer();

                // Error check and process.
                CheckError( Result, TRUE );
            }
            break;

        case STATE_WRITE_COMPLETED:
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

enum createfile_state {
STATE_CREATEFILE_START = 0,
STATE_CREATEFILE_SET_STAT,
STATE_CREATEFILE_WRITE,
STATE_CREATEFILE_CLOSE,
STATE_CREATEFILE_COMPLETED,
};

void memcard_hardware::ProcessCreateFile( void )
{
    s32      Result;
    s32      AlignedLength;
	s32      TotalSize;
    CARDStat Stat;

    // Card has to be mounted!
    if( m_MountedCard == NO_CARD )
    {
        // No memory leaks!
        if( s_pFile )
        {
            x_free( s_pFile );
            s_pFile = NULL;
        }

        // Opps...
        CheckError( CARD_RESULT_NOCARD );
        return;
    }

    // These need to be valid.
    ASSERT( m_pRequestedFileName );
    ASSERT( m_pRequestedBuffer );
    ASSERT( m_nRequestedBytes );

    // Get total file size.
    TotalSize = sizeof(gcn_save_file) + m_nRequestedBytes;

    // Align it up!
    AlignedLength = (TotalSize + m_SectorSize-1) & ~(m_SectorSize-1);

    // Decide what to do!
    switch( m_SubState )
    {
        case STATE_CREATEFILE_START:
            // Try to create the file the file.
            Result = CARDCreateAsync( m_MountedCard, (char*)m_pRequestedFileName, AlignedLength, &s_CardFileInfo, gcn_DefaultCallback );

            // Just error check (callback will process).
            CheckError( Result );
            break;

        case STATE_CREATEFILE_SET_STAT:
            // Nuke the file list.
            InvalidateFileList();

            // Clear the status.
           	x_memset( &Stat, 0, sizeof(Stat) );

            // Set the card stat.
	        CARDSetCommentAddress( &Stat, (s32)&s_pFile->Comment - (s32)s_pFile );
	        CARDSetIconAddress   ( &Stat, (s32)&s_pFile->Icon - (s32)s_pFile );
	        CARDSetBannerFormat  ( &Stat, CARD_STAT_BANNER_NONE );
	        CARDSetIconAnim      ( &Stat, CARD_STAT_ANIM_LOOP );
	        CARDSetIconFormat    ( &Stat, 0, CARD_STAT_ICON_RGB5A3 );
	        CARDSetIconSpeed     ( &Stat, 0, CARD_STAT_SPEED_SLOW );
	        CARDSetIconSpeed     ( &Stat, 1, CARD_STAT_SPEED_END ); // <<== Terminate the icon
            
            // Write it to the memcard.
            Result = CARDSetStatusAsync( m_MountedCard, s_CardFileInfo.fileNo, &Stat, gcn_DefaultCallback );
                        
            // Just error check (callback will process).
            CheckError( Result );
            break;

        case STATE_CREATEFILE_WRITE:
            // Set up the file now.
            s_pFile->Checksum = 0; // TODO: Calculate the Checksum.
            x_strcpy( s_pFile->Comment, "The Hobbit - Save Game File" );
            x_memcpy( s_pFile->Icon, s_CardIcon, 2048 );
            // TODO: Copy the preferences.

            // Write the file.
            Result = CARDWriteAsync( &s_CardFileInfo, s_pFile, AlignedLength, 0, gcn_WriteFileCallback );

            // Just error check (callback will process and free s_pFile).
            CheckError( Result );
            break;

        case STATE_CREATEFILE_CLOSE:
            // Close the file.
            Result = CARDClose( &s_CardFileInfo );

            // Nuke the fileinfo.
            x_memset( &s_CardFileInfo, 0, sizeof(s_CardFileInfo) );

            // Error check and process.
            CheckError( Result, TRUE );
            break;

        case STATE_CREATEFILE_COMPLETED:
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

void memcard_hardware::FreeIOBuffer( void )
{
    if( s_pIOBuffer )
    {
        x_free( s_pIOBuffer );
        s_pIOBuffer = NULL;
    }
}

//------------------------------------------------------------------------------

static u32 s_LastSize = 0;

void memcard_hardware::AllocIOBuffer( void )
{
    if( (s_pIOBuffer == NULL) || ( m_SectorSize != s_LastSize ) )
    {
        // Free the old one.
        FreeIOBuffer();

        // Allocate a new one.
        s_pIOBuffer = (char*)x_malloc( m_SectorSize );

        // Retain last size.
        s_LastSize = m_SectorSize;
    }    
}
