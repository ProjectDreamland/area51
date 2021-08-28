/**************************************************************************
 *
 *  Copyright (C) 2002 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       xact.h
 *  Content:    XACT Runtime Engine APIs.
 *
 **************************************************************************/

#ifndef __XACT_ENGINE_INCLUDED__
#define __XACT_ENGINE_INCLUDED__

#pragma warning(disable:4201)
#pragma pack(push, 8)

//
// Forward declarations
//

typedef struct IXACTEngine IXACTEngine;
typedef IXACTEngine *LPXACTENGINE;
typedef IXACTEngine *PXACTENGINE;

typedef struct IXACTSoundBank IXACTSoundBank;
typedef IXACTSoundBank *LPXACTSOUNDBANK;
typedef IXACTSoundBank *PXACTSOUNDBANK;

typedef struct IXACTSoundSource IXACTSoundSource;
typedef IXACTSoundSource *LPXACTSOUNDSOURCE;
typedef IXACTSoundSource *PXACTSOUNDSOURCE;

typedef struct IXACTSoundCue IXACTSoundCue;
typedef IXACTSoundCue *LPXACTSOUNDCUE;
typedef IXACTSoundCue *PXACTSOUNDCUE;

typedef struct IXACTWaveBank IXACTWaveBank;
typedef IXACTWaveBank *LPXACTWAVEBANK;
typedef IXACTWaveBank *PXACTWAVEBANK;

typedef struct IXACTWmaPlayList IXACTWmaPlayList;
typedef IXACTWmaPlayList *LPXACTWMAPLAYLIST;
typedef IXACTWmaPlayList *PXACTWMAPLAYLIST;

typedef struct IXACTWmaSong IXACTWmaSong;
typedef IXACTWmaSong *LPXACTWMASONG;
typedef IXACTWmaSong *PXACTWMASONG;


//
// Structures and types
//

//
// Notifications
//

typedef enum _XACT_NOTIFICATION_TYPE {
    eXACTNotification_Start = 0,
    eXACTNotification_Stop,
    eXACTNotification_Marker,
    eXACTNotification_StreamDataAvailable,
    eXACTNotification_SoundCueProcessing,
    eXACTNotification_Max
} XACT_NOTIFICATION_TYPE;

typedef struct _XACT_NOTIFICATION_START XACT_NOTIFICATION_START;
typedef struct _XACT_NOTIFICATION_START *PXACT_NOTIFICATION_START;
 
typedef struct _XACT_NOTIFICATION_STOP XACT_NOTIFICATION_STOP;
typedef struct _XACT_NOTIFICATION_STOP *PXACT_NOTIFICATION_STOP;
 
typedef struct _XACT_NOTIFICATION_MARKER {
    DWORD   dwData;
} XACT_NOTIFICATION_MARKER, *PXACT_NOTIFICATION_MARKER;

typedef struct _XACT_NOTIFICATION_STREAM_DATA_AVAILABLE XACT_NOTIFICATION_STREAM_DATA_AVAILABLE;
typedef struct _XACT_NOTIFICATION_STREAM_DATA_AVAILABLE *PXACT_NOTIFICATION_STREAM_DATA_AVAILABLE;

//
// only notification with data is Marker, thus its the only one 
// in the union below
//

typedef union _XACT_NOTIFICATION_UNION {
    XACT_NOTIFICATION_MARKER                Marker;
} XACT_NOTIFICATION_UNION; 

typedef struct _XACT_NOTIFICATION_DESCRIPTION{    
    WORD             wType;
    WORD             wFlags;

    union {
        PXACTSOUNDBANK   pSoundBank;
        PXACTWAVEBANK    pWaveBank;
    } u;

    DWORD            dwSoundCueIndex;
    PXACTSOUNDCUE    pSoundCue; 

    PVOID            pvContext;
    HANDLE           hEvent;
} XACT_NOTIFICATION_DESCRIPTION, *PXACT_NOTIFICATION_DESCRIPTION;

typedef const XACT_NOTIFICATION_DESCRIPTION *PCXACT_NOTIFICATION_DESCRIPTION;

typedef struct _XACT_NOTIFICATION{   
    XACT_NOTIFICATION_DESCRIPTION       Header;
    XACT_NOTIFICATION_UNION             Data;
    REFERENCE_TIME                      rtTimeStamp;
} XACT_NOTIFICATION, *PXACT_NOTIFICATION;        

typedef const XACT_NOTIFICATION *PCXACT_NOTIFICATION;

//
// Engine creation parameters
//
 
typedef struct _XACT_RUNTIME_PARAMETERS {
    DWORD                       dwMax2DHwVoices;
    DWORD                       dwMax3DHwVoices;
    DWORD                       dwMaxConcurrentStreams;
    DWORD                       dwMaxNotifications;
    DWORD                       dwInteractiveAudioLookaheadTime; // Time in ms
} XACT_RUNTIME_PARAMETERS, *PXACT_RUNTIME_PARAMETERS;

typedef const XACT_RUNTIME_PARAMETERS *PCXACT_RUNTIME_PARAMETERS;

//
// Streaming wavebank parameters
//

typedef struct _XACT_WAVEBANK_STREAMING_PARAMETERS {
    HANDLE                      hFile;                          // file handle associated with wavebank data
    DWORD                       dwOffset;                       // offset within file of wavebank header
    DWORD                       dwPacketSizeInMilliSecs;        // stream packet size to use for each stream (in ms)
    DWORD                       dwPrimePacketSizeInMilliSecs;   // stream packet used for all same stream instances, for priming
} XACT_WAVEBANK_STREAMING_PARAMETERS, *PXACT_WAVEBANK_STREAMING_PARAMETERS;

typedef const XACT_WAVEBANK_STREAMING_PARAMETERS *PCXACT_WAVEBANK_STREAMING_PARAMETERS;

//
// runtime sound source properties
//

typedef struct _XACT_SOUNDSOURCE_PROPERTIES {
    DWORD           dwHighestCuePriority;
    DSVOICEPROPS    HwVoiceProperties;
} XACT_SOUNDSOURCE_PROPERTIES, *PXACT_SOUNDSOURCE_PROPERTIES;

//
// runtime parameter control
//

typedef struct _XACT_PARAMETER_ENTRY {    
    WORD    wScope;
    WORD    wFlags;
    WORD    wCategoryIndex;    
    WORD    wStepCount;
    FLOAT   flStepSize;
    FLOAT   flStartValue;
    union {
        PXACTSOUNDCUE pSoundCue;
        PXACTSOUNDBANK pSoundBank;
    } u;             
    PCSTR   pszFriendlyName;  
} XACT_PARAMETER_ENTRY, *PXACT_PARAMETER_ENTRY;

typedef const XACT_PARAMETER_ENTRY *PCXACT_PARAMETER_ENTRY;

typedef struct _XACT_PARAMETER_CONTROL_DESC {
    DWORD                   dwEntryCount;   
    PXACT_PARAMETER_ENTRY   paEntries;
} XACT_PARAMETER_CONTROL_DESC, *PXACT_PARAMETER_CONTROL_DESC;

typedef const XACT_PARAMETER_CONTROL_DESC *PCXACT_PARAMETER_CONTROL_DESC;

#define XACT_FLAG_PARAMETER_CONTROL_IMMEDIATE   0x0001

//
// Wma play list add song description
//

typedef struct _XACT_WMA_PLAYLIST_ADD {
    DWORD   dwType;
    PCSTR   pszFileName;
    DWORD   dwSoundtrackId;
    DWORD   dwSongId;
    DWORD   dwSongIndex;
} XACT_WMA_PLAYLIST_ADD, *PXACT_WMA_PLAYLIST_ADD;

typedef const XACT_WMA_PLAYLIST_ADD *PCXACT_WMA_PLAYLIST_ADD;

//
// wma playlist properties
//

typedef struct _XACT_WMA_PLAYLIST_PROPERTIES {
    DWORD           dwPlaybackFlags;
    DWORD           dwSongEntryCount;
    PXACTWMASONG    pFirstSong;
    PXACTWMASONG    pLastSong;
} XACT_WMA_PLAYLIST_PROPERTIES, *PXACT_WMA_PLAYLIST_PROPERTIES;

typedef const XACT_WMA_PLAYLIST_PROPERTIES *PCXACT_WMA_PLAYLIST_PROPERTIES;

//
// wma song description
//

typedef struct _XACT_WMASONG_DESCRIPTION 
{
    WMAXMOFileContDesc  Content;
    WMAXMOFileHeader    Header;
} XACT_WMASONG_DESCRIPTION, *PXACT_WMASONG_DESCRIPTION;

//
// Constants
//

//
// memory object types for game replaceable allocation scheme
//

typedef enum _XACT_MEMORY_OBJECT_TYPE
{
    eXACTMemoryObject_EngineInstance = 0,
    eXACTMemoryObject_PriorityQueue,
    eXACTMemoryObject_ChildSoundCue,
    eXACTMemoryObject_SoundCueInstance,
    eXACTMemoryObject_WaveBankInstance,
    eXACTMemoryObject_SoundBankInstance,
    eXACTMemoryObject_SoundSourceInstance,
    eXACTMemoryObject_WmaPlayListInstance,
    eXACTMemoryObject_WaveBankHeader,
    eXACTMemoryObject_WmaSongContext,
    eXACTMemoryObject_WmaSongDescription,
    eXACTMemoryObject_CueNameHashEntry,
    eXACTMemoryObject_NotificationContext,
    eXACTMemoryObject_TrackContext,
    eXACTMemoryObject_PacketContext,
    eXACTMemoryObject_EventContext,
    eXACTMemoryObject_RuntimeEvent,
    eXACTMemoryObject_Misc,
    eXACTMemoryObject_AuditionOnly,
    eXACTMemoryObject_PacketBuffer,
    eXACTMemoryObject_ReadaheadPacketBuffer,
    eXACTMemoryObject_ParameterControlContext,
    eXACTMemoryObject_Max    
} XACT_MEMORY_OBJECT_TYPE, *PXACT_MEMORY_OBJECT_TYPE;

//
// notification type
//

#define XACT_NOTIFICATION_TYPE_UNUSED   0xFFFF

//
// Flags used when registering notifications
//

#define XACT_FLAG_NOTIFICATION_PERSIST  0x8000

//
// Flags used when flushing or retrieving notifications
//

#define XACT_FLAG_NOTIFICATION_USE_WAVEBANK             0x0001
#define XACT_FLAG_NOTIFICATION_USE_SOUNDCUE_INDEX       0x0002
#define XACT_FLAG_NOTIFICATION_USE_SOUNDCUE_INSTANCE    0x0004

#define XACT_MASK_NOTIFICATION_FLAGS (XACT_FLAG_NOTIFICATION_USE_SOUNDCUE_INSTANCE | XACT_FLAG_NOTIFICATION_USE_WAVEBANK | XACT_FLAG_NOTIFICATION_USE_SOUNDCUE_INDEX)

//
// Other notification flags returned
//

#define XACT_FLAG_NOTIFICATION_SOUNDCUE_DESTROYED   0x0008

//
// Format assumed for all wma assets
//

#define XACT_WMA_SONG_SAMPLING_RATE     44100
#define XACT_WMA_SONG_BITS_PER_SAMPLE   16

//
// Sound source flags
//

#define XACT_FLAG_SOUNDSOURCE_2D                0x00000001
#define XACT_FLAG_SOUNDSOURCE_3D                0x00000002

#define XACT_FLAG_SOUNDSOURCE_STATUS_ACTIVE     0x00000001


#define XACT_MASK_SOUNDSOURCE_FLAGS         (XACT_FLAG_SOUNDSOURCE_3D | XACT_FLAG_SOUNDSOURCE_2D)

//
// Sound cue flags
//

#define XACT_FLAG_SOUNDCUE_AUTORELEASE          0x0001
#define XACT_FLAG_SOUNDCUE_IMMEDIATE            0x0002
#define XACT_FLAG_SOUNDCUE_SYNCHRONOUS          XACT_FLAG_SOUNDCUE_IMMEDIATE
#define XACT_FLAG_SOUNDCUE_PREPARED             0x0004
#define XACT_FLAG_SOUNDCUE_PRIME                0x0008
#define XACT_FLAG_SOUNDCUE_USE_LAST_VARIATION   0x0010
#define XACT_FLAG_SOUNDCUE_FLUSH                0x0200

#define XACT_SOUNDCUE_INDEX_UNUSED          0xFFFFFFFF
#define XACT_SOUNDCUE_INDEX_ALL             0xFFFFFFFF
#define XACT_TRACK_INDEX_UNUSED             0xFFFFFFFF

//
// Infinite loop count
//

#define XACT_LOOP_INFINITE              0xFFFF

//
// parameter control types
//

typedef enum _XACT_PARAMETER_CONTROL_SCOPE {
    eXACTParameterControlScope_Global = 0,
    eXACTParameterControlScope_SoundCueInstance,    
    eXACTParameterControlScope_Max
} XACT_PARAMETER_CONTROL_SCOPE;

//
// predefined friendly names for controls
//

#define XACT_PARAMETER_CONTROL_MASTER_VOLUME    "=MasterVolume"
#define XACT_PARAMETER_CONTROL_SOUND_VARIATION  "=SoundVariation"
#define XACT_PARAMETER_CONTROL_WAVE_VARIATION   "=WaveVariation"

//
// Wma PlayList Add API flags
//

typedef enum _XACT_WMA_PLAYLIST_ADD_TYPE
{
    eXACTWmaPlayListAdd_Directory = 1,
    eXACTWmaPlayListAdd_File,
    eXACTWmaPlayListAdd_Soundtrack,
    eXACTWmaPlayListAdd_SoundtrackSong,
    eXACTWmaPlayListAdd_Max
} XACT_WMA_PLAYLIST_ADD_TYPE;

//
// Wma PlayList playback flags
//

#define XACT_FLAG_WMAPLAYLIST_PLAYBACK_RANDOM   0x00000001
#define XACT_FLAG_WMAPLAYLIST_PLAYBACK_LOOP     0x00000002
#define XACT_MASK_WMAPLAYLIST_PLAYBACK_FLAGS    (XACT_FLAG_WMAPLAYLIST_PLAYBACK_RANDOM | XACT_FLAG_WMAPLAYLIST_PLAYBACK_LOOP)

//
// SetMasterVolume and GlobalPause API params
//

#define XACT_CATEGORY_INDEX_UNUSED          0xFF

//
// IXACTSoundBank::PrepareEx/PlayEx data
//

typedef struct _XACT_PREPARE_SOUNDCUE
{
    DWORD                           dwFlags;
    DWORD                           dwCueIndex;
    PXACTSOUNDSOURCE                pSoundSource;
    PCXACT_PARAMETER_CONTROL_DESC   pParameterControls;
} XACT_PREPARE_SOUNDCUE, *PXACT_PREPARE_SOUNDCUE;

typedef const XACT_PREPARE_SOUNDCUE *PCXACT_PREPARE_SOUNDCUE;

//
// debug output callback support
//

typedef enum _XACT_DEBUG_OUTPUT_LEVELS
{
    eXACTDebugLevel_Undefined = 0,
    eXACTDebugLevel_Error,
    eXACTDebugLevel_Warning,
    eXACTDebugLevel_Info,
    eXACTDebugLevel_Spam,
} XACT_DEBUG_OUTPUT_LEVELS;

typedef VOID (CALLBACK *PFNXACTDEBUGOUTPUTCALLBACK)(DWORD dwDebugOutputLevel, PCSTR pszDebugOutput);

//
// IXACTSoundBank::SelectVariation data
//

#define XACT_FLAG_SELECT_VARIATION_SOUND_INDEX  0x00000001
#define XACT_FLAG_SELECT_VARIATION_SOUND_VALUE  0x00000002
#define XACT_FLAG_SELECT_VARIATION_SOUND_FLAGS  (XACT_FLAG_SELECT_VARIATION_SOUND_INDEX | XACT_FLAG_SELECT_VARIATION_SOUND_VALUE)
#define XACT_FLAG_SELECT_VARIATION_WAVE_INDEX   0x00000004
#define XACT_FLAG_SELECT_VARIATION_WAVE_VALUE   0x00000008
#define XACT_FLAG_SELECT_VARIATION_WAVE_FLAGS   (XACT_FLAG_SELECT_VARIATION_WAVE_INDEX | XACT_FLAG_SELECT_VARIATION_WAVE_VALUE)
#define XACT_FLAG_SELECT_VARIATION_FLAGS        (XACT_FLAG_SELECT_VARIATION_SOUND_FLAGS | XACT_FLAG_SELECT_VARIATION_WAVE_FLAGS)

typedef struct _XACT_SOUNDBANK_SELECT_VARIATION
{
    DWORD   dwFlags;

    union
    {
        DWORD   dwIndex;
        FLOAT   flValue;
    } Sound;

    union
    {
        DWORD   dwIndex;
        FLOAT   flValue;
    } Wave;
} XACT_SOUNDBANK_SELECT_VARIATION, *PXACT_SOUNDBANK_SELECT_VARIATION;

typedef const XACT_SOUNDBANK_SELECT_VARIATION *PCXACT_SOUNDBANK_SELECT_VARIATION;

//
// IXACTSoundBank::GetSoundCueProperties
//

#define XACT_WAVE_INDEX_UNUSED  0xFFFF

#define XACT_FLAG_SOUNDCUE_PROPERTIES_3D    0x00000001
#define XACT_FLAG_SOUNDCUE_PROPERTIES_FLAGS (XACT_FLAG_SOUNDCUE_PROPERTIES_3D)

typedef struct _XACT_SOUNDCUE_PROPERTIES
{
    DWORD   dwFlags;            // Flags
    DWORD   dwPriority;         // Priority
    LONG    lVolume;            // Sound volume (dB * 100)
    LONG    lPitch;             // Pitch (semitone * 4096 / 12)
    DWORD   dwLayer;            // Layer
    DWORD   dwCategory;         // Category (0-based, or 255 if sound has no category)
    LONG    lParametricEQGain;  // Parametric EQ gain [-8192, 32767]
    DWORD   dwParametricEQQ;    // Parametric EQ Q [0, 4]
    DWORD   dwParametricEQFc;   // Parametric EQ frequency
    DWORD   dwLoopCount;        // Loop count (highest loop count of all tracks)
    DWORD   dwTrackCount;       // Number of tracks
    DWORD   dwSoundIndex;       // Sound index
    DWORD   dwWaveIndex;        // Wave index
    DWORD   dwLength;           // Length in ms

    // 3D properties

    LONG    lI3DL2Volume;       // I3DL2 volume send (dB * 100)
    LONG    lLFEVolume;         // LFE volume send (dB * 100)
    DWORD   dwInsideConeAngle;  // Buffer inside cone angle
    DWORD   dwOutsideConeAngle; // Buffer outside cone angle
    LONG    lConeOutsideVolume; // Volume outside the cone
    DWORD   dwMode;             // 3D processing mode
    FLOAT   flMinDistance;      // Minimum distance value
    FLOAT   flMaxDistance;      // Maximum distance value    
    FLOAT   flDistanceFactor;   // Distance factor
    FLOAT   flRolloffFactor;    // Rolloff factor
    FLOAT   flDopplerFactor;    // Doppler factor
} XACT_SOUNDCUE_PROPERTIES, *PXACT_SOUNDCUE_PROPERTIES;

//
// IXACTEngine file I/O callback support
//

typedef BOOL (__stdcall * XACT_READFILE_CALLBACK)(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);
typedef BOOL (__stdcall * XACT_GETOVERLAPPEDRESULT_CALLBACK)(HANDLE hFile, LPOVERLAPPED lpOverlapped, LPDWORD lpNumberOfBytesTransferred, BOOL bWait);

typedef struct _XACT_FILEIO_CALLBACKS
{
    XACT_READFILE_CALLBACK            ReadFileCallback;
    XACT_GETOVERLAPPEDRESULT_CALLBACK GetOverlappedResultCallback;
} XACT_FILEIO_CALLBACKS, *PXACT_FILEIO_CALLBACKS;

typedef const XACT_FILEIO_CALLBACKS *PCXACT_FILEIO_CALLBACKS;

//
// Real-time data
//

typedef struct _XACT_REALTIME_AUDIO_DATA 
{
    DSOUTPUTLEVELS  OutputLevels;
    DSCAPS          DSoundCaps;
    DWORD           dwXactMemoryUsage;
    BYTE            bXactAvailable2DBuffers;
    BYTE            bXactAvailable2DStreams;
    BYTE            bXactAvailable3DBuffers;
    BYTE            bReserved;
} XACT_REALTIME_AUDIO_DATA, *PXACT_REALTIME_AUDIO_DATA;

//
// Variation table range
//

#define XACT_VARIATION_TABLE_WEIGHT_MIN 0
#define XACT_VARIATION_TABLE_WEIGHT_MAX 10000

//
// Variables
//

#define XACT_GLOBAL_VARIABLE_MIN_INDEX  0
#define XACT_GLOBAL_VARIABLE_MAX_INDEX  31
#define XACT_GLOBAL_VARIABLE_MIN_VALUE  XACT_VARIATION_TABLE_WEIGHT_MIN
#define XACT_GLOBAL_VARIABLE_MAX_VALUE  XACT_VARIATION_TABLE_WEIGHT_MAX

//
// IXACTEngine
//

STDAPI XACTEngineCreate(PCXACT_RUNTIME_PARAMETERS pParams, PXACTENGINE *ppEngine);
STDAPI_(void) XACTEngineDoWork(void);
STDAPI_(void) XACTEngineSetFileIOCallbacks(PCXACT_FILEIO_CALLBACKS pFileIOCallbacks);

STDAPI_(ULONG) IXACTEngine_AddRef(PXACTENGINE pEngine);
STDAPI_(ULONG) IXACTEngine_Release(PXACTENGINE pEngine);
STDAPI IXACTEngine_DownloadEffectsImage(PXACTENGINE pEngine, PVOID pvData, DWORD dwSize, LPCDSEFFECTIMAGELOC pEffectLoc, LPDSEFFECTIMAGEDESC *ppImageDesc);
STDAPI IXACTEngine_CreateSoundSource(PXACTENGINE pEngine, DWORD dwFlags, PXACTSOUNDSOURCE *ppSoundSource);
STDAPI IXACTEngine_CreateSoundBank(PXACTENGINE pEngine, PVOID pvData, DWORD dwSize, PXACTSOUNDBANK *ppSoundBank);

STDAPI IXACTEngine_RegisterWaveBank(PXACTENGINE pEngine, PVOID pvData, DWORD dwSize, PXACTWAVEBANK * ppWaveBank);
STDAPI IXACTEngine_RegisterStreamedWaveBank(PXACTENGINE pEngine, PCXACT_WAVEBANK_STREAMING_PARAMETERS pParams, PXACTWAVEBANK *ppWaveBank);
STDAPI IXACTEngine_UnRegisterWaveBank(PXACTENGINE pEngine, PXACTWAVEBANK pWaveBank);

STDAPI IXACTEngine_SetMasterVolume(PXACTENGINE pEngine, WORD wCategory, LONG lVolume);
STDAPI IXACTEngine_GlobalPause(PXACTENGINE pEngine, WORD wCategory, BOOL fPause);
STDAPI IXACTEngine_SetParameterControl(PXACTENGINE pEngine, PCXACT_PARAMETER_CONTROL_DESC pParams);

STDAPI IXACTEngine_SetI3dl2Listener(PXACTENGINE pEngine, LPCDSI3DL2LISTENER pds3dl, DWORD dwApply);
STDAPI IXACTEngine_SetListenerOrientation(PXACTENGINE pEngine, FLOAT xFront, FLOAT yFront, FLOAT zFront, FLOAT xTop, FLOAT yTop, FLOAT zTop, DWORD dwApply);
STDAPI IXACTEngine_SetListenerPosition(PXACTENGINE pEngine, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IXACTEngine_SetListenerVelocity(PXACTENGINE pEngine, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IXACTEngine_CommitDeferredSettings(PXACTENGINE pEngine);
STDAPI IXACTEngine_EnableHeadphones(PXACTENGINE pEngine, BOOL fEnabled);

STDAPI IXACTEngine_RegisterNotification(PXACTENGINE pEngine, PCXACT_NOTIFICATION_DESCRIPTION pNotificationDesc);
STDAPI IXACTEngine_UnRegisterNotification(PXACTENGINE pEngine, PCXACT_NOTIFICATION_DESCRIPTION pNotificationDesc);
STDAPI IXACTEngine_GetNotification(PXACTENGINE pEngine, PCXACT_NOTIFICATION_DESCRIPTION pNotificationDesc, PXACT_NOTIFICATION pNotification);
STDAPI IXACTEngine_FlushNotification(PXACTENGINE pEngine, PCXACT_NOTIFICATION_DESCRIPTION pNotificationDesc);
STDAPI IXACTEngine_SetVariable(PXACTENGINE pEngine, DWORD dwVariable, WORD wValue, DWORD dwApply);
STDAPI IXACTEngine_GetVariable(PXACTENGINE pEngine, DWORD dwVariable, PWORD pwValue);
STDAPI IXACTEngine_GetRealtimeData(PXACTENGINE pEngine, XACT_REALTIME_AUDIO_DATA *pData);

#if defined(__cplusplus) && !defined(CINTERFACE)

struct IXACTEngine
{

    __inline ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return IXACTEngine_AddRef(this);
    }

    __inline ULONG STDMETHODCALLTYPE Release(void)
    {
        return IXACTEngine_Release(this);
    }

    __inline HRESULT STDMETHODCALLTYPE DownloadEffectsImage(PVOID pvData, DWORD dwSize, LPCDSEFFECTIMAGELOC pEffectLoc, LPDSEFFECTIMAGEDESC *ppImageDesc)
    {
        return IXACTEngine_DownloadEffectsImage(this, pvData, dwSize, pEffectLoc, ppImageDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE CreateSoundSource(DWORD dwFlags,PXACTSOUNDSOURCE *ppSoundSource)
    {
        return IXACTEngine_CreateSoundSource(this, dwFlags, ppSoundSource);
    }

    __inline HRESULT STDMETHODCALLTYPE CreateSoundBank(PVOID pvData, DWORD dwSize, PXACTSOUNDBANK *ppSoundBank)
    {
        return IXACTEngine_CreateSoundBank(this, pvData, dwSize, ppSoundBank);
    }

    __inline HRESULT STDMETHODCALLTYPE RegisterWaveBank(PVOID pvData, DWORD dwSize, PXACTWAVEBANK *ppWaveBank)
    {
        return IXACTEngine_RegisterWaveBank(this, pvData, dwSize, ppWaveBank);
    }

    __inline HRESULT STDMETHODCALLTYPE RegisterStreamedWaveBank(PCXACT_WAVEBANK_STREAMING_PARAMETERS pParams, PXACTWAVEBANK *ppWaveBank)
    {
        return IXACTEngine_RegisterStreamedWaveBank(this, pParams, ppWaveBank);
    }

    __inline HRESULT STDMETHODCALLTYPE UnRegisterWaveBank(PXACTWAVEBANK pWaveBank)
    {
        return IXACTEngine_UnRegisterWaveBank(this, pWaveBank);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMasterVolume( WORD wCategory, LONG lVolume)
    {
        return IXACTEngine_SetMasterVolume(this, wCategory, lVolume);
    }
    
    __inline HRESULT STDMETHODCALLTYPE SetI3dl2Listener(LPCDSI3DL2LISTENER pds3dl, DWORD dwApply)
    {
        return IXACTEngine_SetI3dl2Listener(this, pds3dl, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetListenerOrientation(FLOAT xFront, FLOAT yFront, FLOAT zFront, FLOAT xTop, FLOAT yTop, FLOAT zTop, DWORD dwApply)
    {
        return IXACTEngine_SetListenerOrientation(this, xFront, yFront, zFront, xTop, yTop, zTop, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetListenerPosition(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IXACTEngine_SetListenerPosition(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetListenerVelocity(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IXACTEngine_SetListenerVelocity(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE GlobalPause(WORD wCategory, BOOL fPause)
    {
        return IXACTEngine_GlobalPause(this, wCategory, fPause);
    }

    __inline HRESULT STDMETHODCALLTYPE RegisterNotification(PCXACT_NOTIFICATION_DESCRIPTION pNotificationDesc)
    {
        return IXACTEngine_RegisterNotification(this, pNotificationDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE UnRegisterNotification(PCXACT_NOTIFICATION_DESCRIPTION pNotificationDesc)
    {
        return IXACTEngine_UnRegisterNotification(this, pNotificationDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE GetNotification(PCXACT_NOTIFICATION_DESCRIPTION pNotificationDesc, PXACT_NOTIFICATION pNotification)
    {
        return IXACTEngine_GetNotification(this, pNotificationDesc, pNotification);
    }

    __inline HRESULT STDMETHODCALLTYPE FlushNotification(PCXACT_NOTIFICATION_DESCRIPTION pNotificationDesc)
    {
        return IXACTEngine_FlushNotification(this, pNotificationDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE SetVariable(DWORD dwVariable, WORD wValue, DWORD dwApply)
    {
        return IXACTEngine_SetVariable(this, dwVariable, wValue, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE GetVariable(DWORD dwVariable, PWORD pwValue)
    {
        return IXACTEngine_GetVariable(this, dwVariable, pwValue);
    }

    __inline HRESULT STDMETHODCALLTYPE GetRealtimeData(XACT_REALTIME_AUDIO_DATA *pData)
    {
        return IXACTEngine_GetRealtimeData(this, pData);
    }

    __inline HRESULT STDMETHODCALLTYPE CommitDeferredSettings(void)
    {
        return IXACTEngine_CommitDeferredSettings(this);
    }

    __inline HRESULT STDMETHODCALLTYPE EnableHeadphones(BOOL fEnabled)
    {
        return IXACTEngine_EnableHeadphones(this, fEnabled);
    }

    __inline HRESULT STDMETHODCALLTYPE SetParameterControl(PCXACT_PARAMETER_CONTROL_DESC pParams)
    {
        return IXACTEngine_SetParameterControl(this,pParams);
    }

};

#endif // defined(__cplusplus) && !defined(CINTERFACE)

//
// IXACTSoundSource
//

STDAPI_(ULONG) IXACTSoundSource_AddRef(PXACTSOUNDSOURCE pSoundSource);
STDAPI_(ULONG) IXACTSoundSource_Release(PXACTSOUNDSOURCE pSoundSource);
STDAPI IXACTSoundSource_SetPosition(PXACTSOUNDSOURCE pSoundSource, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IXACTSoundSource_SetConeOrientation(PXACTSOUNDSOURCE pSoundSource,FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IXACTSoundSource_SetI3DL2Source(PXACTSOUNDSOURCE pSoundSource,LPCDSI3DL2BUFFER pds3db, DWORD dwApply);
STDAPI IXACTSoundSource_SetVelocity(PXACTSOUNDSOURCE pSoundSource,FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IXACTSoundSource_SetPitch(PXACTSOUNDSOURCE pSoundSource, LONG lPitch);
STDAPI IXACTSoundSource_SetFilter(PXACTSOUNDSOURCE pSoundSource, LPCDSFILTERDESC pFilterDesc);
STDAPI IXACTSoundSource_SetMixBins(PXACTSOUNDSOURCE pSoundSource, LPCDSMIXBINS pMixBins);
STDAPI IXACTSoundSource_SetMixBinVolumes(PXACTSOUNDSOURCE pSoundSource, LPCDSMIXBINS pMixBins);
STDAPI IXACTSoundSource_SetMode(PXACTSOUNDSOURCE pSoundSource, DWORD dwMode, DWORD dwApply);
STDAPI IXACTSoundSource_GetStatus(PXACTSOUNDSOURCE pSoundSource, PDWORD pdwStatus);
STDAPI IXACTSoundSource_GetProperties(PXACTSOUNDSOURCE pSoundSource, PXACT_SOUNDSOURCE_PROPERTIES pProperties);
STDAPI IXACTSoundSource_StopSoundCues(PXACTSOUNDSOURCE pSoundSource);

#if defined(__cplusplus) && !defined(CINTERFACE)

struct IXACTSoundSource
{

    __inline ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return IXACTSoundSource_AddRef(this);
    }

    __inline ULONG STDMETHODCALLTYPE Release(void)
    {
        return IXACTSoundSource_Release(this);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPosition( FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IXACTSoundSource_SetPosition(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetConeOrientation(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IXACTSoundSource_SetConeOrientation(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetI3DL2Source(LPCDSI3DL2BUFFER pds3db, DWORD dwApply)
    {
        return IXACTSoundSource_SetI3DL2Source(this, pds3db, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetVelocity(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IXACTSoundSource_SetVelocity(this, x,  y,  z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMixBins(LPCDSMIXBINS pMixBins)
    {
        return IXACTSoundSource_SetMixBins(this, pMixBins);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMixBinVolumes(LPCDSMIXBINS pMixBins)
    {
        return IXACTSoundSource_SetMixBinVolumes(this, pMixBins);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMode(DWORD dwMode, DWORD dwApply)
    {
        return IXACTSoundSource_SetMode(this, dwMode,dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE GetStatus(PDWORD pdwStatus)
    {
        return IXACTSoundSource_GetStatus(this, pdwStatus);
    }
    
    __inline HRESULT STDMETHODCALLTYPE GetProperties(PXACT_SOUNDSOURCE_PROPERTIES pProperties)
    {
        return IXACTSoundSource_GetProperties(this, pProperties);
    }

    __inline HRESULT STDMETHODCALLTYPE StopSoundCues()
    {
        return IXACTSoundSource_StopSoundCues(this);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPitch(LONG lPitch)
    {
        return IXACTSoundSource_SetPitch(this,lPitch);
    }

    __inline HRESULT STDMETHODCALLTYPE SetFilter(LPCDSFILTERDESC pFilterDesc)
    {
        return IXACTSoundSource_SetFilter(this, pFilterDesc);
    }
};

#endif // defined(__cplusplus) && !defined(CINTERFACE)

//
// IXACTSoundBank
//

STDAPI_(ULONG) IXACTSoundBank_AddRef(PXACTSOUNDBANK pBank);
STDAPI_(ULONG) IXACTSoundBank_Release(PXACTSOUNDBANK pBank);
STDAPI IXACTSoundBank_GetSoundCueIndexFromFriendlyName(PXACTSOUNDBANK pBank, PCSTR pFriendlyName, PDWORD pdwSoundCueIndex);
STDAPI IXACTSoundBank_PrepareEx(PXACTSOUNDBANK pBank, PCXACT_PREPARE_SOUNDCUE pPrepareData, PXACTSOUNDCUE *ppSoundCue);
STDAPI IXACTSoundBank_PlayEx(PXACTSOUNDBANK pBank, PCXACT_PREPARE_SOUNDCUE pPrepareData, PXACTSOUNDCUE *ppSoundCue);
STDAPI IXACTSoundBank_Stop(PXACTSOUNDBANK pBank, DWORD dwSoundCueIndex, DWORD dwFlags, PXACTSOUNDCUE pSoundCue);
STDAPI IXACTSoundBank_CreateWmaPlayList(PXACTSOUNDBANK pBank, DWORD dwSoundCueIndex, DWORD dwPlaybackFlags, PXACTWMAPLAYLIST *ppWmaPlayList);
STDAPI IXACTSoundBank_SelectVariation(PXACTSOUNDBANK pBank, DWORD dwSoundCueIndex, PCXACT_SOUNDBANK_SELECT_VARIATION pVariation);
STDAPI IXACTSoundBank_GetSoundCueProperties(PXACTSOUNDBANK pBank, DWORD dwSoundCueIndex, PXACT_SOUNDCUE_PROPERTIES pSoundCueProperties);
STDAPI IXACTSoundBank_PauseSoundCue(PXACTSOUNDBANK pBank, PXACTSOUNDCUE pSoundCue, BOOL fPause);

__inline HRESULT IXACTSoundBank_Prepare(PXACTSOUNDBANK pBank, DWORD dwCueIndex, DWORD dwFlags, PXACTSOUNDCUE *ppSoundCue)
{
    XACT_PREPARE_SOUNDCUE   PrepareData;
    
    PrepareData.dwFlags = dwFlags;
    PrepareData.dwCueIndex = dwCueIndex;
    PrepareData.pSoundSource = NULL;
    PrepareData.pParameterControls = NULL;
    
    return IXACTSoundBank_PrepareEx(pBank, &PrepareData, ppSoundCue);
}

__inline HRESULT IXACTSoundBank_Play(PXACTSOUNDBANK pBank, DWORD dwCueIndex, PXACTSOUNDSOURCE pSoundSource, DWORD dwFlags, PXACTSOUNDCUE *ppSoundCue)
{
    XACT_PREPARE_SOUNDCUE   PrepareData;
    
    PrepareData.dwFlags = dwFlags;
    PrepareData.dwCueIndex = dwCueIndex;
    PrepareData.pSoundSource = pSoundSource;
    PrepareData.pParameterControls = NULL;
    
    return IXACTSoundBank_PlayEx(pBank, &PrepareData, ppSoundCue);
}

#if defined(__cplusplus) && !defined(CINTERFACE)

struct IXACTSoundBank
{
    __inline ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return IXACTSoundBank_AddRef(this);
    }

    __inline ULONG STDMETHODCALLTYPE Release(void)
    {
        return IXACTSoundBank_Release(this);
    }

    __inline HRESULT STDMETHODCALLTYPE GetSoundCueIndexFromFriendlyName(PCSTR pFriendlyName, PDWORD pdwSoundCueIndex)
    {
        return IXACTSoundBank_GetSoundCueIndexFromFriendlyName(this, pFriendlyName, pdwSoundCueIndex);
    }

    __inline HRESULT STDMETHODCALLTYPE Prepare(DWORD dwSoundCueIndex, DWORD dwFlags, PXACTSOUNDCUE *ppSoundCue)
    {
        return IXACTSoundBank_Prepare(this, dwSoundCueIndex, dwFlags, ppSoundCue);
    }

    __inline HRESULT STDMETHODCALLTYPE PrepareEx(PCXACT_PREPARE_SOUNDCUE pPrepareData, PXACTSOUNDCUE *ppSoundCue)
    {
        return IXACTSoundBank_PrepareEx(this, pPrepareData, ppSoundCue);
    }

    __inline HRESULT STDMETHODCALLTYPE Play(DWORD dwSoundCueIndex, PXACTSOUNDSOURCE pSoundSource, DWORD dwFlags, PXACTSOUNDCUE *ppSoundCue)
    {
        return IXACTSoundBank_Play(this, dwSoundCueIndex, pSoundSource, dwFlags, ppSoundCue);
    }

    __inline HRESULT STDMETHODCALLTYPE PlayEx(PCXACT_PREPARE_SOUNDCUE pPrepareData, PXACTSOUNDCUE *ppSoundCue)
    {
        return IXACTSoundBank_PlayEx(this, pPrepareData, ppSoundCue);
    }

    __inline HRESULT STDMETHODCALLTYPE Stop( DWORD dwSoundCueIndex, DWORD dwFlags, PXACTSOUNDCUE pSoundCue)
    {
        return IXACTSoundBank_Stop(this, dwSoundCueIndex, dwFlags, pSoundCue);
    }
    
    __inline HRESULT STDMETHODCALLTYPE CreateWmaPlayList(DWORD dwSoundCueIndex, DWORD dwPlaybackFlags, PXACTWMAPLAYLIST *ppWmaPlayList)
    {
        return IXACTSoundBank_CreateWmaPlayList(this, dwSoundCueIndex, dwPlaybackFlags, ppWmaPlayList);
    }

    __inline HRESULT STDMETHODCALLTYPE SelectVariation(DWORD dwSoundCueIndex, PCXACT_SOUNDBANK_SELECT_VARIATION pVariation)
    {
        return IXACTSoundBank_SelectVariation(this, dwSoundCueIndex, pVariation);
    }

    __inline HRESULT STDMETHODCALLTYPE GetSoundCueProperties(DWORD dwSoundCueIndex, PXACT_SOUNDCUE_PROPERTIES pSoundCueProperties)
    {
        return IXACTSoundBank_GetSoundCueProperties(this, dwSoundCueIndex, pSoundCueProperties);
    }

    __inline HRESULT STDMETHODCALLTYPE PauseSoundCue(PXACTSOUNDCUE pSoundCue, BOOL fPause)
    {
        return IXACTSoundBank_PauseSoundCue(this, pSoundCue, fPause);
    }
};

#endif // defined(__cplusplus) && !defined(CINTERFACE)

//
// wma playlist 
//

STDAPI_(ULONG) IXACTWmaPlayList_AddRef(PXACTWMAPLAYLIST pPlayList);
STDAPI_(ULONG) IXACTWmaPlayList_Release(PXACTWMAPLAYLIST pPlayList);
STDAPI IXACTWmaPlayList_Add(PXACTWMAPLAYLIST pPlayList, PCXACT_WMA_PLAYLIST_ADD pDesc, PXACTWMASONG *ppSong);
STDAPI IXACTWmaPlayList_Remove(PXACTWMAPLAYLIST pPlayList, PXACTWMASONG pSong);
STDAPI IXACTWmaPlayList_SetCurrent(PXACTWMAPLAYLIST pPlayList, PXACTWMASONG pSong);
STDAPI IXACTWmaPlayList_Next(PXACTWMAPLAYLIST pPlayList);
STDAPI IXACTWmaPlayList_Previous(PXACTWMAPLAYLIST pPlayList);
STDAPI IXACTWmaPlayList_GetCurrentSongInfo(PXACTWMAPLAYLIST pPlayList, PDWORD pdwSongLength, PWCHAR pszNameBuffer, DWORD dwBufferSize, PXACTWMASONG *ppSong);
STDAPI IXACTWmaPlayList_GetCurrentSongInfoEx(PXACTWMAPLAYLIST pPlayList, PXACT_WMASONG_DESCRIPTION pDesc, PXACTWMASONG *ppSong);
STDAPI IXACTWmaPlayList_SetPlaybackBehavior(PXACTWMAPLAYLIST pPlayList, DWORD dwFlags);
STDAPI IXACTWmaPlayList_GetProperties(PXACTWMAPLAYLIST pPlayList, PXACT_WMA_PLAYLIST_PROPERTIES pProperties);

#if defined(__cplusplus) && !defined(CINTERFACE)

struct IXACTWmaPlayList
{

    __inline ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return IXACTWmaPlayList_AddRef(this);
    }

    __inline ULONG STDMETHODCALLTYPE Release(void)
    {
        return IXACTWmaPlayList_Release(this);
    }

    __inline HRESULT STDMETHODCALLTYPE Add(PCXACT_WMA_PLAYLIST_ADD pDesc, PXACTWMASONG *ppSong)
    {
        return IXACTWmaPlayList_Add(this, pDesc, ppSong);
    }
    __inline HRESULT STDMETHODCALLTYPE Remove(PXACTWMASONG pSong)
    {
        return IXACTWmaPlayList_Remove(this, pSong);
    }

    __inline HRESULT STDMETHODCALLTYPE SetCurrent(PXACTWMASONG pSong)
    {
        return IXACTWmaPlayList_SetCurrent(this, pSong);
    }

    __inline HRESULT STDMETHODCALLTYPE Next()
    {
        return IXACTWmaPlayList_Next(this);
    }

    __inline HRESULT STDMETHODCALLTYPE Previous()
    {
        return IXACTWmaPlayList_Previous(this);
    }

    __inline HRESULT STDMETHODCALLTYPE GetCurrentSongInfo(PDWORD pdwSongLength, PWCHAR pszNameBuffer, DWORD dwBufferSize, PXACTWMASONG *ppSong)
    {
        return IXACTWmaPlayList_GetCurrentSongInfo(this, pdwSongLength,  pszNameBuffer, dwBufferSize, ppSong);
    }

    __inline HRESULT STDMETHODCALLTYPE GetCurrentSongInfoEx(PXACT_WMASONG_DESCRIPTION pDesc, PXACTWMASONG *ppSong)
    {
        return IXACTWmaPlayList_GetCurrentSongInfoEx(this, pDesc, ppSong);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPlaybackBehavior(DWORD dwFlags)
    {
        return IXACTWmaPlayList_SetPlaybackBehavior(this, dwFlags);
    }

    __inline HRESULT STDMETHODCALLTYPE GetProperties(PXACT_WMA_PLAYLIST_PROPERTIES pProperties)
    {
        return IXACTWmaPlayList_GetProperties(this, pProperties);
    }

};

#endif // defined(__cplusplus) && !defined(CINTERFACE)



#pragma pack(pop)

#endif // __XACT_ENGINE INCLUDED__
