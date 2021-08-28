/************************************************************************
*                                                                       *
*   Xbox.h -- This module defines the Xbox APIs                         *
*                                                                       *
*   Copyright (c) 2000 - 2003 Microsoft Corp. All rights reserved.      *
*                                                                       *
************************************************************************/
#ifndef _XBOX_
#define _XBOX_


//
// Define API decoration for direct importing of DLL references.
//

#define XBOXAPI

#ifdef __cplusplus
extern "C" {
#endif

XBOXAPI
PVOID
WINAPI
XLoadSectionA(
    IN LPCSTR pSectionName
    );
#define XLoadSection  XLoadSectionA

XBOXAPI
BOOL
WINAPI
XFreeSectionA(
    IN LPCSTR pSectionName
    );
#define XFreeSection  XFreeSectionA

XBOXAPI
HANDLE
WINAPI
XGetSectionHandleA(
    IN LPCSTR pSectionName
    );
#define XGetSectionHandle  XGetSectionHandleA

XBOXAPI
PVOID
WINAPI
XLoadSectionByHandle(
    IN HANDLE hSection
    );

XBOXAPI
BOOL
WINAPI
XFreeSectionByHandle(
    HANDLE hSection
    );

XBOXAPI
DWORD
WINAPI
XGetSectionSize(
    HANDLE hSection
    );

#define MAX_DISPLAY_BLOCKS  50001

XBOXAPI
DWORD
WINAPI
XGetDisplayBlocks(
    IN LPCSTR lpPathName
    );


#define XSAVEGAME_NOCOPY   1

XBOXAPI
DWORD
WINAPI
XCreateSaveGame(
    IN LPCSTR lpRootPathName,
    IN LPCWSTR lpSaveGameName,
    IN DWORD dwCreationDisposition,
    IN DWORD dwCreateFlags,
    OUT LPSTR lpPathBuffer,
    IN UINT uSize
    );

XBOXAPI
DWORD
WINAPI
XDeleteSaveGame(
    IN LPCSTR lpRootPathName,
    IN LPCWSTR lpSaveGameName
    );

#define MAX_GAMENAME     128

typedef struct _XGAME_FIND_DATA {
    WIN32_FIND_DATAA wfd;
    CHAR szSaveGameDirectory[MAX_PATH];
    WCHAR szSaveGameName[MAX_GAMENAME];
} XGAME_FIND_DATA, *PXGAME_FIND_DATA;

XBOXAPI
HANDLE
WINAPI
XFindFirstSaveGame(
    IN LPCSTR lpRootPathName,
    OUT PXGAME_FIND_DATA pFindGameData
    );

XBOXAPI
BOOL
WINAPI
XFindNextSaveGame(
    IN HANDLE hFindGame,
    OUT PXGAME_FIND_DATA pFindGameData
    );

XBOXAPI
BOOL
WINAPI
XFindClose(
    IN HANDLE hFind
    );

#define MAX_NICKNAME 32

XBOXAPI
BOOL
WINAPI
XSetNicknameW(
    IN LPCWSTR lpNickname,
    IN BOOL fPreserveCase
    );
#define XSetNickname XSetNicknameW

XBOXAPI
HANDLE
WINAPI
XFindFirstNicknameW(
    IN BOOL fThisTitleOnly,
    OUT LPWSTR lpNickname,
    IN UINT uSize
    );
#define XFindFirstNickname XFindFirstNicknameW

XBOXAPI
BOOL
WINAPI
XFindNextNicknameW(
    IN HANDLE hFindNickname,
    OUT LPWSTR lpNickname,
    IN UINT uSize
    );
#define XFindNextNickname XFindNextNicknameW

typedef ULONGLONG XOFFERING_ID;

#define MAX_CONTENT_DISPLAY_NAME 128

typedef struct _XCONTENT_FIND_DATA {
    WIN32_FIND_DATAA wfd;
    XOFFERING_ID qwOfferingId;
    DWORD dwFlags;
    CHAR szContentDirectory[MAX_PATH];
    WCHAR szDisplayName[MAX_CONTENT_DISPLAY_NAME];
} XCONTENT_FIND_DATA, *PXCONTENT_FIND_DATA;

XBOXAPI
HANDLE
WINAPI
XFindFirstContent(
    IN LPCSTR lpRootPathName,
    IN DWORD dwFlagFilter,
    OUT PXCONTENT_FIND_DATA pFindContentData
    );

XBOXAPI
BOOL
WINAPI
XFindNextContent(
    IN HANDLE hFindContent,
    OUT PXCONTENT_FIND_DATA pFindContentData
    );

XBOXAPI
BOOL
WINAPI
XGetContentInstallLocation(
    IN DWORD dwTitleID,
    IN LPCSTR lpSourceMetadataFileName,
    OUT LPSTR lpInstallDirectory
    );

XBOXAPI
BOOL
WINAPI
XGetContentInstallLocationFromIDs(
    IN DWORD dwTitleID,
    IN XOFFERING_ID xOfferingID,
    OUT LPSTR lpInstallDirectory
    );

XBOXAPI
BOOL
WINAPI
XInstallContentSignatures(
    IN DWORD dwTitleID,
    IN LPCSTR lpSourceMetadataFileName,
    IN LPCSTR lpDestinationDirectory
    );

XBOXAPI
BOOL
WINAPI
XCreateContentSimple(
    IN DWORD dwTitleID,
    IN XOFFERING_ID xOfferingID,
    IN DWORD dwContentFlags,
    IN LPCWSTR lpContentName,
    IN LPCSTR lpDestinationDirectory
    );

XBOXAPI
BOOL
WINAPI
XRemoveContent(
    IN LPCSTR lpDirectoryName
    );


XBOXAPI
HANDLE
WINAPI
XLoadContentSignaturesEx(
    IN DWORD dwTitleID,
    IN LPCSTR lpDirectoryName
    );

#define XLoadContentSignatures(lpDirectoryName) \
    XLoadContentSignaturesEx(0, lpDirectoryName)

XBOXAPI
BOOL
WINAPI
XLocateSignatureByIndex(
    IN HANDLE hSignature,
    IN DWORD dwSignatureIndex,
    OUT LPBYTE *ppbSignatureData,
    OUT LPDWORD pdwSignatureSize
    );

XBOXAPI
BOOL
WINAPI
XLocateSignatureByName(
    IN HANDLE hSignature,
    IN LPCSTR lpFileName,
    IN DWORD dwFileOffset,
    IN DWORD dwDataSize,
    OUT LPBYTE *ppbSignatureData,
    OUT LPDWORD pdwSignatureSize
    );

XBOXAPI
BOOL
WINAPI
XCalculateContentSignature(
    IN LPBYTE pbData,
    IN DWORD dwDataSize,
    OUT LPBYTE pbSignature,
    IN OUT LPDWORD pdwSignatureSize
    );

XBOXAPI
VOID
WINAPI
XCloseContentSignatures(
    IN HANDLE hSignature
    );


#define MAX_SONG_NAME       32
#define MAX_SOUNDTRACK_NAME 32
#define MAX_SOUNDTRACKS     100
#define MAX_SONGS_IN_SNDTRK 500

typedef struct _XSOUNDTRACK_DATA {
    UINT    uSoundtrackId;
    UINT    uSongCount;
    UINT    uSoundtrackLength;
    WCHAR   szName[MAX_SOUNDTRACK_NAME];
} XSOUNDTRACK_DATA, *PXSOUNDTRACK_DATA;

XBOXAPI
HANDLE
WINAPI
XFindFirstSoundtrack(
    OUT PXSOUNDTRACK_DATA pSoundtrackData
    );

XBOXAPI
BOOL
WINAPI
XFindNextSoundtrack(
    IN HANDLE hFindHandle,
    OUT PXSOUNDTRACK_DATA pSoundtrackData
    );

XBOXAPI
HANDLE
WINAPI
XOpenSoundtrackSong(
    IN DWORD dwSongId,
    IN BOOL fAsyncMode
    );

XBOXAPI
BOOL
WINAPI
XGetSoundtrackSongInfo(
    IN DWORD dwSoundtrackId,
    IN UINT uIndex,
    OUT PDWORD pdwSongId,
    OUT PDWORD pdwSongLength,
    OUT OPTIONAL PWSTR szNameBuffer,
    IN UINT uBufferSize
    );

XBOXAPI
DWORD
WINAPI
XAddSoundtrack(
    IN  LPCWSTR pszSoundtrackName,
    OUT PUINT pdwSoundtrackId
    );

typedef
DWORD
(WINAPI *LP_SOUNDTRACK_PROGRESS_ROUTINE)(
    IN PCWSTR pszSoundtrackName,
    IN PCWSTR pszSongName,
    IN LARGE_INTEGER TotalFileSize,
    IN LARGE_INTEGER TotalBytesTransferred,
    IN LPVOID Context OPTIONAL
    );

XBOXAPI
HRESULT
WINAPI
XAddSongToSoundtrack(
    IN  UINT dwSoundtrackId,
    IN  LPCSTR pszSongPath,
    IN  LPCWSTR pszSongName OPTIONAL,
    IN  LP_SOUNDTRACK_PROGRESS_ROUTINE lpRoutine OPTIONAL,
    IN  LPVOID Context OPTIONAL,
    OUT PUINT pdwSongId OPTIONAL
    );

#define XC_LANGUAGE_ENGLISH         1
#define XC_LANGUAGE_JAPANESE        2
#define XC_LANGUAGE_GERMAN          3
#define XC_LANGUAGE_FRENCH          4
#define XC_LANGUAGE_SPANISH         5
#define XC_LANGUAGE_ITALIAN         6
#define XC_LANGUAGE_KOREAN          7
#define XC_LANGUAGE_TCHINESE        8
#define XC_LANGUAGE_PORTUGUESE      9


XBOXAPI
DWORD
WINAPI
XGetLanguage(
    VOID
    );


#define XC_AUTO_LOGON_ALLOWED       1
#define XC_AUTO_LOGON_NOT_ALLOWED   2

XBOXAPI
DWORD
WINAPI
XGetAutoLogonFlag(
    VOID
    );


#define XC_AV_PACK_SCART            0
#define XC_AV_PACK_HDTV             1
#define XC_AV_PACK_VGA              2
#define XC_AV_PACK_RFU              3
#define XC_AV_PACK_SVIDEO           4
#define XC_AV_PACK_STANDARD         6

XBOXAPI
DWORD
WINAPI
XGetAVPack(
    VOID
    );

#define XC_VIDEO_STANDARD_NTSC_M    1
#define XC_VIDEO_STANDARD_NTSC_J    2
#define XC_VIDEO_STANDARD_PAL_I     3

XBOXAPI
DWORD
WINAPI
XGetVideoStandard(
    VOID
    );

#define XC_VIDEO_FLAGS_WIDESCREEN   0x00000001
#define XC_VIDEO_FLAGS_HDTV_720p    0x00000002
#define XC_VIDEO_FLAGS_HDTV_1080i   0x00000004
#define XC_VIDEO_FLAGS_HDTV_480p    0x00000008
#define XC_VIDEO_FLAGS_LETTERBOX    0x00000010
#define XC_VIDEO_FLAGS_PAL_60Hz     0x00000040

XBOXAPI
DWORD
WINAPI
XGetVideoFlags(
    VOID
    );

#define XC_AUDIO_FLAGS_STEREO       0x00000000
#define XC_AUDIO_FLAGS_MONO         0x00000001
#define XC_AUDIO_FLAGS_SURROUND     0x00000002
#define XC_AUDIO_FLAGS_ENABLE_AC3   0x00010000
#define XC_AUDIO_FLAGS_ENABLE_DTS   0x00020000

#define XC_AUDIO_FLAGS_BASICMASK    0x0000FFFF
#define XC_AUDIO_FLAGS_ENCODEDMASK  0xFFFF0000

#define XC_AUDIO_FLAGS_BASIC(c)      ((DWORD)(c) & XC_AUDIO_FLAGS_BASICMASK)
#define XC_AUDIO_FLAGS_ENCODED(c)    ((DWORD)(c) & XC_AUDIO_FLAGS_ENCODEDMASK)
#define XC_AUDIO_FLAGS_COMBINED(b,e) (XC_AUDIO_FLAGS_BASIC(b) | XC_AUDIO_FLAGS_ENCODED(e))

XBOXAPI
DWORD
WINAPI
XGetAudioFlags(
    VOID
    );

#define XC_PC_ESRB_ALL              0
#define XC_PC_ESRB_ADULT            1
#define XC_PC_ESRB_MATURE           2
#define XC_PC_ESRB_TEEN             3
#define XC_PC_ESRB_EVERYONE         4
#define XC_PC_ESRB_KIDS_TO_ADULTS   5
#define XC_PC_ESRB_EARLY_CHILDHOOD  6

XBOXAPI
DWORD
WINAPI
XGetParentalControlSetting(
    VOID
    );

#define XC_GAME_REGION_NA             0x00000001
#define XC_GAME_REGION_JAPAN          0x00000002
#define XC_GAME_REGION_RESTOFWORLD    0x00000004
#define XC_GAME_REGION_MANUFACTURING  0x80000000

XBOXAPI
DWORD
WINAPI
XGetGameRegion(
    VOID
    );


typedef struct _XPP_DEVICE_TYPE
{
    ULONG Reserved[3];
} XPP_DEVICE_TYPE, *PXPP_DEVICE_TYPE;

extern XPP_DEVICE_TYPE XDEVICE_TYPE_GAMEPAD_TABLE;
extern XPP_DEVICE_TYPE XDEVICE_TYPE_MEMORY_UNIT_TABLE;
extern XPP_DEVICE_TYPE XDEVICE_TYPE_VOICE_MICROPHONE_TABLE;
extern XPP_DEVICE_TYPE XDEVICE_TYPE_VOICE_HEADPHONE_TABLE;
extern XPP_DEVICE_TYPE XDEVICE_TYPE_HIGHFIDELITY_MICROPHONE_TABLE;

#define     XDEVICE_TYPE_GAMEPAD           (&XDEVICE_TYPE_GAMEPAD_TABLE)
#define     XDEVICE_TYPE_MEMORY_UNIT       (&XDEVICE_TYPE_MEMORY_UNIT_TABLE)
#define     XDEVICE_TYPE_VOICE_MICROPHONE   (&XDEVICE_TYPE_VOICE_MICROPHONE_TABLE)
#define     XDEVICE_TYPE_VOICE_HEADPHONE    (&XDEVICE_TYPE_VOICE_HEADPHONE_TABLE)
#define     XDEVICE_TYPE_HIGHFIDELITY_MICROPHONE (&XDEVICE_TYPE_HIGHFIDELITY_MICROPHONE_TABLE)

#ifdef DEBUG_MOUSE
extern  XPP_DEVICE_TYPE            XDEVICE_TYPE_DEBUG_MOUSE_TABLE;
#define XDEVICE_TYPE_DEBUG_MOUSE (&XDEVICE_TYPE_DEBUG_MOUSE_TABLE)
#endif //DEBUG_MOUSE



#ifdef DEBUG_KEYBOARD
#include <xkbd.h>
#endif

#define     XDEVICE_PORT0               0
#define     XDEVICE_PORT1               1
#define     XDEVICE_PORT2               2
#define     XDEVICE_PORT3               3

#define     XDEVICE_NO_SLOT             0
#define     XDEVICE_TOP_SLOT            0
#define     XDEVICE_BOTTOM_SLOT         1

#define     XDEVICE_PORT0_MASK          (1 << XDEVICE_PORT0)
#define     XDEVICE_PORT1_MASK          (1 << XDEVICE_PORT1)
#define     XDEVICE_PORT2_MASK          (1 << XDEVICE_PORT2)
#define     XDEVICE_PORT3_MASK          (1 << XDEVICE_PORT3)
#define     XDEVICE_PORT0_TOP_MASK      (1 << XDEVICE_PORT0)
#define     XDEVICE_PORT1_TOP_MASK      (1 << XDEVICE_PORT1)
#define     XDEVICE_PORT2_TOP_MASK      (1 << XDEVICE_PORT2)
#define     XDEVICE_PORT3_TOP_MASK      (1 << XDEVICE_PORT3)
#define     XDEVICE_PORT0_BOTTOM_MASK   (1 << (XDEVICE_PORT0 + 16))
#define     XDEVICE_PORT1_BOTTOM_MASK   (1 << (XDEVICE_PORT1 + 16))
#define     XDEVICE_PORT2_BOTTOM_MASK   (1 << (XDEVICE_PORT2 + 16))
#define     XDEVICE_PORT3_BOTTOM_MASK   (1 << (XDEVICE_PORT3 + 16))

typedef struct _XDEVICE_PREALLOC_TYPE
{
    PXPP_DEVICE_TYPE DeviceType;
    DWORD            dwPreallocCount;
} XDEVICE_PREALLOC_TYPE, *PXDEVICE_PREALLOC_TYPE;

#define XGetPortCount() 4


XBOXAPI
VOID
WINAPI
XInitDevices(
    DWORD                  dwPreallocTypeCount,
    PXDEVICE_PREALLOC_TYPE PreallocTypes
    );

XBOXAPI
DWORD
WINAPI
XGetDevices(
    IN PXPP_DEVICE_TYPE DeviceType
    );

XBOXAPI
BOOL
WINAPI
XGetDeviceChanges(
    IN PXPP_DEVICE_TYPE DeviceType,
    OUT PDWORD pdwInsertions,
    OUT PDWORD pdwRemovals
    );


#define XDEVICE_ENUMERATION_IDLE 0
#define XDEVICE_ENUMERATION_BUSY 1

XBOXAPI
DWORD
WINAPI
XGetDeviceEnumerationStatus();


#include <PSHPACK1.H>

typedef struct _XINPUT_GAMEPAD
{
    WORD    wButtons;
    BYTE    bAnalogButtons[8];
    SHORT   sThumbLX;
    SHORT   sThumbLY;
    SHORT   sThumbRX;
    SHORT   sThumbRY;
} XINPUT_GAMEPAD, *PXINPUT_GAMEPAD;

#define XINPUT_GAMEPAD_DPAD_UP           0x0001
#define XINPUT_GAMEPAD_DPAD_DOWN         0x0002
#define XINPUT_GAMEPAD_DPAD_LEFT         0x0004
#define XINPUT_GAMEPAD_DPAD_RIGHT        0x0008
#define XINPUT_GAMEPAD_START             0x0010
#define XINPUT_GAMEPAD_BACK              0x0020
#define XINPUT_GAMEPAD_LEFT_THUMB        0x0040
#define XINPUT_GAMEPAD_RIGHT_THUMB       0x0080
#define XINPUT_LIGHTGUN_ONSCREEN         0x2000
#define XINPUT_LIGHTGUN_FRAME_DOUBLER    0x4000
#define XINPUT_LIGHTGUN_LINE_DOUBLER     0x8000


#define XINPUT_GAMEPAD_A                0
#define XINPUT_GAMEPAD_B                1
#define XINPUT_GAMEPAD_X                2
#define XINPUT_GAMEPAD_Y                3
#define XINPUT_GAMEPAD_BLACK            4
#define XINPUT_GAMEPAD_WHITE            5
#define XINPUT_GAMEPAD_LEFT_TRIGGER     6
#define XINPUT_GAMEPAD_RIGHT_TRIGGER    7



#define XINPUT_GAMEPAD_MAX_CROSSTALK    30

typedef struct _XINPUT_RUMBLE
{
   WORD   wLeftMotorSpeed;
   WORD   wRightMotorSpeed;
} XINPUT_RUMBLE, *PXINPUT_RUMBLE;

#ifdef DEBUG_MOUSE
typedef struct _XINPUT_MOUSE
{
    BYTE bButtons;
    char cMickeysX;
    char cMickeysY;
    char cWheel;
} XINPUT_MOUSE, *PXINPUT_MOUSE;

#define XINPUT_DEBUG_MOUSE_LEFT_BUTTON    0x01
#define XINPUT_DEBUG_MOUSE_RIGHT_BUTTON   0x02
#define XINPUT_DEBUG_MOUSE_MIDDLE_BUTTON  0x04
#define XINPUT_DEBUG_MOUSE_XBUTTON1       0x08
#define XINPUT_DEBUG_MOUSE_XBUTTON2       0x10
#endif //DEBUG_MOUSE

typedef struct _XINPUT_STATE
{
    DWORD dwPacketNumber;
    union
    {
        XINPUT_GAMEPAD Gamepad;
#ifdef DEBUG_MOUSE
        XINPUT_MOUSE   DebugMouse;
#endif //DEBUG_MOUSE
    };
} XINPUT_STATE, *PXINPUT_STATE;


#define XINPUT_FEEDBACK_HEADER_INTERNAL_SIZE 58
typedef struct _XINPUT_FEEDBACK_HEADER
{
    DWORD           dwStatus;
    HANDLE OPTIONAL hEvent;
    BYTE            Reserved[XINPUT_FEEDBACK_HEADER_INTERNAL_SIZE];
} XINPUT_FEEDBACK_HEADER, *PXINPUT_FEEDBACK_HEADER;

typedef struct _XINPUT_FEEDBACK
{
    XINPUT_FEEDBACK_HEADER Header;
    union
    {
      XINPUT_RUMBLE Rumble;
    };
} XINPUT_FEEDBACK, *PXINPUT_FEEDBACK;

typedef struct _XINPUT_CAPABILITIES
{
    BYTE    SubType;
    WORD    Reserved;
    union
    {
      XINPUT_GAMEPAD Gamepad;
    } In;
    union
    {
      XINPUT_RUMBLE Rumble;
    } Out;
} XINPUT_CAPABILITIES, *PXINPUT_CAPABILITIES;

#include <POPPACK.H>

#define XINPUT_DEVSUBTYPE_GC_GAMEPAD              0x01
#define XINPUT_DEVSUBTYPE_GC_GAMEPAD_ALT          0x02
#define XINPUT_DEVSUBTYPE_GC_WHEEL                0x10
#define XINPUT_DEVSUBTYPE_GC_ARCADE_STICK         0x20
#define XINPUT_DEVSUBTYPE_GC_DIGITAL_ARCADE_STICK 0x21
#define XINPUT_DEVSUBTYPE_GC_FLIGHT_STICK         0x30
#define XINPUT_DEVSUBTYPE_GC_SNOWBOARD            0x40
#define XINPUT_DEVSUBTYPE_GC_LIGHTGUN             0x50
#define XINPUT_DEVSUBTYPE_GC_RADIO_FLIGHT_CONTROL 0x60
#define XINPUT_DEVSUBTYPE_GC_FISHING_ROD          0x70
#define XINPUT_DEVSUBTYPE_GC_DANCEPAD             0x80

typedef struct _XINPUT_POLLING_PARAMETERS
{
    BYTE       fAutoPoll:1;
    BYTE       fInterruptOut:1;
    BYTE       ReservedMBZ1:6;
    BYTE       bInputInterval;
    BYTE       bOutputInterval;
    BYTE       ReservedMBZ2;
} XINPUT_POLLING_PARAMETERS, *PXINPUT_POLLING_PARAMETERS;

XBOXAPI
HANDLE
WINAPI
XInputOpen(
    IN PXPP_DEVICE_TYPE DeviceType,
    IN DWORD dwPort,
    IN DWORD dwSlot,
    IN PXINPUT_POLLING_PARAMETERS pPollingParameters OPTIONAL
    );

XBOXAPI
VOID
WINAPI
XInputClose(
    IN HANDLE hDevice
    );

XBOXAPI
DWORD
WINAPI
XInputGetState(
    IN HANDLE hDevice,
    OUT PXINPUT_STATE  pState
    );

XBOXAPI
DWORD
WINAPI
XInputPoll(
    IN HANDLE hDevice
    );

XBOXAPI
DWORD
WINAPI
XInputSetState(
    IN HANDLE hDevice,
    IN OUT PXINPUT_FEEDBACK pFeedback
    );

XBOXAPI
DWORD
WINAPI
XInputGetCapabilities(
    IN HANDLE hDevice,
    OUT PXINPUT_CAPABILITIES pCapabilities
    );


typedef struct _XINPUT_DEVICE_DESCRIPTION
{
    WORD wVendorID;
    WORD wProductID;
    WORD wVersion;
} XINPUT_DEVICE_DESCRIPTION, *PXINPUT_DEVICE_DESCRIPTION;

XBOXAPI
DWORD
WINAPI
XInputGetDeviceDescription(
 IN  HANDLE hDevice,
 OUT PXINPUT_DEVICE_DESCRIPTION pDescription
);

#define XINPUT_LIGHTGUN_CALIBRATION_CENTER_X      0
#define XINPUT_LIGHTGUN_CALIBRATION_CENTER_Y      0
#define XINPUT_LIGHTGUN_CALIBRATION_UPPERLEFT_X  -25000
#define XINPUT_LIGHTGUN_CALIBRATION_UPPERLEFT_Y   25000

typedef struct _XINPUT_LIGHTGUN_CALIBRATION_OFFSETS
{
    WORD wCenterX;
    WORD wCenterY;
    WORD wUpperLeftX;
    WORD wUpperLeftY;
} XINPUT_LIGHTGUN_CALIBRATION_OFFSETS, *PXINPUT_LIGHTGUN_CALIBRATION_OFFSETS;

XBOXAPI
DWORD
WINAPI
XInputSetLightgunCalibration(
    IN HANDLE hDevice,
    IN PXINPUT_LIGHTGUN_CALIBRATION_OFFSETS pCalibrationOffsets
);

XBOXAPI
DWORD
WINAPI
XMountMUA(
    IN DWORD dwPort,
    IN DWORD dwSlot,
    OUT PCHAR pchDrive
    );
#define XMountMU  XMountMUA


XBOXAPI
DWORD
WINAPI
XUnmountMU(
    IN DWORD dwPort,
    IN DWORD dwSlot
    );

XBOXAPI
DWORD
WINAPI
XMUPortFromDriveLetterA(
    IN CHAR chDrive
    );
#define XMUPortFromDriveLetter  XMUPortFromDriveLetterA

XBOXAPI
DWORD
WINAPI
XMUSlotFromDriveLetterA(
    IN CHAR chDrive
    );
#define XMUSlotFromDriveLetter  XMUSlotFromDriveLetterA

#define MAX_MUNAME 32

XBOXAPI
DWORD
WINAPI
XMUNameFromDriveLetter(
    IN CHAR chDrive,
    OUT LPWSTR lpName,
    IN UINT cchName
    );


#define XINIT_MOUNT_UTILITY_DRIVE               0x00000001
#define XINIT_FORMAT_UTILITY_DRIVE              0x00000002
#define XINIT_LIMIT_DEVKIT_MEMORY               0x00000004
#define XINIT_DONT_MODIFY_HARD_DISK             0x00000010

#define XINIT_UTILITY_DRIVE_16K_CLUSTER_SIZE    0x00000000
#define XINIT_UTILITY_DRIVE_32K_CLUSTER_SIZE    0x40000000
#define XINIT_UTILITY_DRIVE_64K_CLUSTER_SIZE    0x80000000
#define XINIT_UTILITY_DRIVE_128K_CLUSTER_SIZE   0xC0000000

XBOXAPI
BOOL
WINAPI
XMountUtilityDrive(
    IN BOOL fFormatClean
    );

XBOXAPI
BOOL
WINAPI
XFormatUtilityDrive(
    VOID
    );

XBOXAPI
BOOL
WINAPI
XMountSecondaryUtilityDrive(
    VOID
    );

XBOXAPI
BOOL
WINAPI
XSwapUtilityDrives(
    VOID
    );


XBOXAPI
BOOL
WINAPI
XFormatSecondaryUtilityDrive(
    VOID
    );


XBOXAPI
DWORD
WINAPI
XMountAlternateTitleA(
    IN LPCSTR lpRootPath,
    IN DWORD dwAltTitleId,
    OUT PCHAR pchDrive
    );
#define XMountAlternateTitle  XMountAlternateTitleA

XBOXAPI
DWORD
WINAPI
XUnmountAlternateTitleA(
    IN CHAR chDrive
    );
#define XUnmountAlternateTitle  XUnmountAlternateTitleA

XBOXAPI
DWORD
WINAPI
XGetDiskSectorSizeA(
    IN LPCSTR lpRootPathName
    );
#define XGetDiskSectorSize  XGetDiskSectorSizeA

#define XBOX_HD_SECTOR_SIZE   512
#define XBOX_DVD_SECTOR_SIZE  2048
#define XBOX_MU_SECTOR_SIZE   4096

XBOXAPI
DWORD
WINAPI
XGetDiskClusterSizeA(
    IN LPCSTR lpRootPathName
    );
#define XGetDiskClusterSize  XGetDiskClusterSizeA

#define MAX_LAUNCH_DATA_SIZE 3072

typedef struct _LAUNCH_DATA
{
    BYTE Data[MAX_LAUNCH_DATA_SIZE];
} LAUNCH_DATA, *PLAUNCH_DATA;

typedef struct _LD_LAUNCH_DASHBOARD
{
    DWORD dwReason;
    DWORD dwContext;
    DWORD dwParameter1;
    DWORD dwParameter2;
    BYTE  Reserved[MAX_LAUNCH_DATA_SIZE - 16];
} LD_LAUNCH_DASHBOARD, *PLD_LAUNCH_DASHBOARD;


#define XLD_LAUNCH_DASHBOARD_MAIN_MENU  0 // Does not return to application
#define XLD_LAUNCH_DASHBOARD_ERROR      1 // Does not return to application
#define XLD_LAUNCH_DASHBOARD_MEMORY     2
#define XLD_LAUNCH_DASHBOARD_SETTINGS   3
#define XLD_LAUNCH_DASHBOARD_MUSIC      4

#define XLD_LAUNCH_DASHBOARD_NETWORK_CONFIGURATION      6
#define XLD_LAUNCH_DASHBOARD_NEW_ACCOUNT_SIGNUP         7
#define XLD_LAUNCH_DASHBOARD_ACCOUNT_MANAGEMENT         8
#define XLD_LAUNCH_DASHBOARD_ONLINE_MENU                9
#define XLD_LAUNCH_DASHBOARD_DVDPLAYER                  0x100

//
// When XDash is launched with XLD_LAUNCH_DASHBOARD_ERROR,
// LD_LAUNCH_DASHBOARD.dwParameter1 field contains one of
// the following error codes.
//
#define XLD_ERROR_INVALID_XBE           1
#define XLD_ERROR_INVALID_HARD_DISK     2
#define XLD_ERROR_XBE_REGION            3
#define XLD_ERROR_XBE_PARENTAL_CONTROL  4
#define XLD_ERROR_XBE_MEDIA_TYPE        5

//
// When the dwReason is XLD_LAUNCH_DASHBOARD_SETTINGS,
// LD_LAUNCH_DASHBOARD.dwParameter1 will have 0 or more
// of the following flags set.
//
#define XLD_SETTINGS_CLOCK              0x01 // Does not return to application with context
#define XLD_SETTINGS_TIMEZONE           0x02 // Does not return to application with context
#define XLD_SETTINGS_LANGUAGE           0x04 // Does not return to application with context
#define XLD_SETTINGS_VIDEO              0x08
#define XLD_SETTINGS_AUDIO              0x10


typedef struct _LD_FROM_DASHBOARD
{
    DWORD dwContext;
    BYTE  Reserved[MAX_LAUNCH_DATA_SIZE - 4];
} LD_FROM_DASHBOARD, *PLD_FROM_DASHBOARD;

typedef struct _LD_FROM_DEBUGGER_CMDLINE
{
    CHAR szCmdLine[MAX_LAUNCH_DATA_SIZE];
} LD_FROM_DEBUGGER_CMDLINE, *PLD_FROM_DEBUGGER_CMDLINE;

#define XLDEMO_RUNMODE_KIOSKMODE        0x01
#define XLDEMO_RUNMODE_USERSELECTED     0x02

typedef struct _LD_DEMO  // Required for launching into and out of demos, data type is LDT_TITLE
{
    DWORD dwID;
    DWORD dwRunmode;
    DWORD dwTimeout;
    CHAR  szLauncherXBE[64];
    CHAR  szLaunchedXBE[64];
    BYTE  Reserved[MAX_LAUNCH_DATA_SIZE - 140];
} LD_DEMO, *PLD_DEMO;

// value of LD_DEMO.dwID
#define LAUNCH_DATA_DEMO_ID 'CDX1'

#define LD_UPDATE_FLAG_DATA_PRESENT       ((DWORD)0x00000001)


typedef struct _LD_FROM_UPDATE  // Required for launching out of auto-updates, data type is LDT_FROM_UPDATE
{
    DWORD   dwContext;
    HRESULT hr;
    DWORD   dwFlags;
    DWORD   dwReserved[4];
    BYTE    Data[MAX_LAUNCH_DATA_SIZE - 28];
} LD_FROM_UPDATE, *PLD_FROM_UPDATE;

XBOXAPI
DWORD
WINAPI
XLaunchNewImageA(
    IN LPCSTR lpImagePath,
    IN PLAUNCH_DATA pLaunchData
    );
#define XLaunchNewImage XLaunchNewImageA

#define LDT_TITLE                 0
#define LDT_FROM_DASHBOARD        2
#define LDT_FROM_DEBUGGER_CMDLINE 3
#define LDT_FROM_UPDATE           4


XBOXAPI
DWORD
WINAPI
XGetLaunchInfo(
    OUT PDWORD pdwLaunchDataType,
    OUT PLAUNCH_DATA pLaunchData
    );


typedef VOID (WINAPI *XTHREAD_NOTIFY_PROC)(BOOL fCreate);
typedef struct _XTHREAD_NOTIFICATION {
    LIST_ENTRY ListEntry;
    XTHREAD_NOTIFY_PROC pfnNotifyRoutine;
} XTHREAD_NOTIFICATION, *PXTHREAD_NOTIFICATION;

XBOXAPI
VOID
WINAPI
XRegisterThreadNotifyRoutine(
    IN OUT PXTHREAD_NOTIFICATION pThreadNotification,
    IN BOOL fRegister
    );

XBOXAPI
VOID
WINAPI
XSetProcessQuantumLength(
    IN DWORD dwMilliseconds
    );

XBOXAPI
DWORD
WINAPI
XGetProcessQuantumLength(
    VOID
    );

XBOXAPI
BOOL
WINAPI
XSetFileCacheSize(
    IN SIZE_T dwCacheSize
    );

XBOXAPI
SIZE_T
WINAPI
XGetFileCacheSize(
    VOID
    );

DWORD
WINAPI
XGetFilePhysicalSortKey(
    HANDLE hFile
    );

XBOXAPI
VOID
WINAPI
XSaveFloatingPointStateForDpc(
    VOID
    );

XBOXAPI
VOID
WINAPI
XRestoreFloatingPointStateForDpc(
    VOID
    );

XBOXAPI
LPVOID
WINAPI
XPhysicalAlloc(
    IN SIZE_T dwSize,
    IN ULONG_PTR ulPhysicalAddress,
    IN ULONG_PTR ulAlignment,
    IN DWORD flProtect
    );

XBOXAPI
LPVOID
WINAPI
XPhysicalAllocEx(
    IN SIZE_T dwSize,
    IN ULONG_PTR ulLowestAcceptableAddress,
    IN ULONG_PTR ulHighestAcceptableAddress,
    IN ULONG_PTR ulAlignment,
    IN DWORD flProtect
    );

XBOXAPI
SIZE_T
WINAPI
XPhysicalSize(
    IN LPVOID lpAddress
    );

XBOXAPI
VOID
WINAPI
XPhysicalProtect(
    IN LPVOID lpAddress,
    IN SIZE_T dwSize,
    IN DWORD flNewProtect
    );

XBOXAPI
VOID
WINAPI
XPhysicalFree(
    IN LPVOID lpAddress
    );

XBOXAPI
DWORD
WINAPI
XQueryMemoryProtect(
    IN LPVOID lpAddress
    );

//
// allocation attribute definitions for XMemAlloc and XMemFree APIs
//

#define XALLOC_MEMTYPE_HEAP                     0
#define XALLOC_MEMTYPE_PHYSICAL                 1

#define XALLOC_MEMPROTECT_READONLY              0
#define XALLOC_MEMPROTECT_NOCACHE               1
#define XALLOC_MEMPROTECT_READWRITE             2
#define XALLOC_MEMPROTECT_WRITECOMBINE          3

#define XALLOC_ALIGNMENT_DEFAULT                0x0
#define XALLOC_ALIGNMENT_4                      0x1
#define XALLOC_ALIGNMENT_8                      0x2
#define XALLOC_ALIGNMENT_16                     0x4

#define XALLOC_PHYSICAL_ALIGNMENT_DEFAULT       0x0 // Default is 4K alignment
#define XALLOC_PHYSICAL_ALIGNMENT_4K_BELOW_16M  0x1
#define XALLOC_PHYSICAL_ALIGNMENT_4             0x2
#define XALLOC_PHYSICAL_ALIGNMENT_8             0x3
#define XALLOC_PHYSICAL_ALIGNMENT_16            0x4
#define XALLOC_PHYSICAL_ALIGNMENT_32            0x5
#define XALLOC_PHYSICAL_ALIGNMENT_64            0x6
#define XALLOC_PHYSICAL_ALIGNMENT_128           0x7
#define XALLOC_PHYSICAL_ALIGNMENT_256           0x8
#define XALLOC_PHYSICAL_ALIGNMENT_512           0x9
#define XALLOC_PHYSICAL_ALIGNMENT_1K            0xA
#define XALLOC_PHYSICAL_ALIGNMENT_2K            0xB
#define XALLOC_PHYSICAL_ALIGNMENT_4K            0xC
#define XALLOC_PHYSICAL_ALIGNMENT_8K            0xD
#define XALLOC_PHYSICAL_ALIGNMENT_16K           0xE
#define XALLOC_PHYSICAL_ALIGNMENT_32K           0xF

typedef enum _XALLOC_ALLOCATOR_IDS {
    eXALLOCAllocatorId_GameMin = 0,
    eXALLOCAllocatorId_GameMax = 127,
    eXALLOCAllocatorId_MsReservedMin = 128,
    eXALLOCAllocatorId_D3D8 = 128,
    eXALLOCAllocatorId_D3DX8,
    eXALLOCAllocatorId_DSOUND,
    eXALLOCAllocatorId_XAPI,
    eXALLOCAllocatorId_XACT,
    eXALLOCAllocatorId_XBOXKERNEL,
    eXALLOCAllocatorId_XBDM,
    eXALLOCAllocatorId_XGRAPHICS,
    eXALLOCAllocatorId_XONLINE,
    eXALLOCAllocatorId_XVOICE,
    eXALLOCAllocatorId_XHV,
    eXALLOCAllocatorId_USB,
    eXALLOCAllocatorId_XMV,
    eXALLOCAllocatorId_SHADERCOMPILER,
    eXALLOCAllocatorId_UIX,
    eXALLOCAllocatorId_MsReservedMax = 191,
    eXALLOCAllocatorId_MiddlewareReservedMin = 192,
    eXALLOCAllocatorId_MiddlewareReservedMax = 255
} XALLOC_ALLOCATOR_IDS;

typedef struct _XALLOC_ATTRIBUTES {
    DWORD dwObjectType:13;
    DWORD dwHeapTracksAttributes:1;
    DWORD dwMustSucceed:1;
    DWORD dwFixedSize:1;
    DWORD dwAllocatorId:8;
    DWORD dwAlignment:4;
    DWORD dwMemoryProtect:2;
    DWORD dwZeroInitialize:1;
    DWORD dwMemoryType:1;
} XALLOC_ATTRIBUTES, *PXALLOC_ATTRIBUTES;

#define MAKE_XALLOC_ATTRIBUTES(ObjectType,\
                               HeapTracksAttributes,\
                               MustSucceed,\
                               FixedSize,\
                               AllocatorId,\
                               Alignment,\
                               MemoryProtect,\
                               ZeroInitialize,\
                               MemoryType)\
    ((DWORD)( ObjectType | \
             (HeapTracksAttributes << 13) | \
             (MustSucceed << 14) | \
             (FixedSize << 15) | \
             (AllocatorId << 16) | \
             (Alignment << 24) | \
             (MemoryProtect << 28) | \
             (ZeroInitialize << 30) | \
             (MemoryType << 31)))

#define XALLOC_IS_PHYSICAL(Attributes) ((BOOL)(Attributes & 0x80000000)!=0)

XBOXAPI
LPVOID
WINAPI
XMemAlloc(
    IN SIZE_T dwSize,
    IN DWORD dwAllocAttributes
    );

XBOXAPI
LPVOID
WINAPI
XMemAllocDefault(
    IN SIZE_T dwSize,
    IN DWORD dwAllocAttributes
    );

XBOXAPI
VOID
WINAPI
XMemFree(
    IN PVOID pAddress,
    IN DWORD dwAllocAttributes
    );

XBOXAPI
VOID
WINAPI
XMemFreeDefault(
    IN PVOID pAddress,
    IN DWORD dwAllocAttributes
    );

XBOXAPI
SIZE_T
WINAPI
XMemSize(
    IN PVOID pAddress,
    IN DWORD dwAllocAttributes
    );

XBOXAPI
SIZE_T
WINAPI
XMemSizeDefault(
    IN PVOID pAddress,
    IN DWORD dwAllocAttributes
    );

XBOXAPI
VOID
WINAPI
XSetAttributesOnHeapAlloc(
    PVOID pBaseAddress,
    DWORD dwAllocAttributes);

XBOXAPI
DWORD
WINAPI
XGetAttributesOnHeapAlloc(
    PVOID pBaseAddress);


#ifdef _DEBUG

#define XVER_DEVKIT 1
#define XVER_RETAIL 2

XBOXAPI
DWORD
WINAPI
XDebugGetSystemVersionA(
    OUT LPSTR pszVersionString,
    IN UINT cchVersionString
    );
#define XDebugGetSystemVersion XDebugGetSystemVersionA

XBOXAPI
DWORD
WINAPI
XDebugGetXTLVersionA(
    OUT LPSTR pszVersionString,
    IN UINT cchVersionString
    );
#define XDebugGetXTLVersion XDebugGetXTLVersionA

#endif // _DEBUG

#define XCALCSIG_SIGNATURE_SIZE         20

typedef struct {
    BYTE Signature[XCALCSIG_SIGNATURE_SIZE];
} XCALCSIG_SIGNATURE, *PXCALCSIG_SIGNATURE;

#define XCALCSIG_FLAG_SAVE_GAME         (0x00000000)
#define XCALCSIG_FLAG_NON_ROAMABLE      (0x00000001)
#define XCALCSIG_FLAG_CONTENT           (0x00000002)
#define XCALCSIG_FLAG_DIGEST            (0x00000004)
#define XCALCSIG_FLAG_ONLINE            (0x00000008)

XBOXAPI
DWORD
WINAPI
XCalculateSignatureGetSize(
    IN DWORD dwFlags
    );

XBOXAPI
HANDLE
WINAPI
XCalculateSignatureBegin(
    IN DWORD dwFlags
    );

XBOXAPI
HANDLE
WINAPI
XCalculateSignatureBeginEx(
    IN DWORD dwFlags,
    IN DWORD dwAltTitleId
    );

XBOXAPI
DWORD
WINAPI
XCalculateSignatureUpdate(
    IN HANDLE hCalcSig,
    IN const BYTE *pbData,
    IN ULONG cbData
    );

XBOXAPI
DWORD
WINAPI
XCalculateSignatureEnd(
    IN HANDLE hCalcSig,
    OUT PVOID pSignature
    );

XBOXAPI
ULONG
WINAPI
XAutoPowerDownTimeRemaining();


#define     STATS_OFFLINE_LOCALUSERNAME_MAX_LENGTH      MAX_NICKNAME

typedef enum _XOFFLINE_STAT_TYPE {
    XOFFLINE_STAT_NONE,
    XOFFLINE_STAT_LONG,
    XOFFLINE_STAT_LONGLONG,
    XOFFLINE_STAT_DOUBLE,
} XOFFLINE_STAT_TYPE;


XBOXAPI
DWORD
WINAPI
XWriteStatStore(
    IN LPCWSTR lpLocalUserName,
    IN DWORD dwLeaderBoardIndex,
    IN WORD wAttributeIndex,
    XOFFLINE_STAT_TYPE Type,
    VOID* pStatValue
    );

XBOXAPI
DWORD
WINAPI
XClearStatStore(
    IN LPCWSTR lpLocalUserName
    );


#ifdef __cplusplus
}
#endif


#endif // _XBOX_

