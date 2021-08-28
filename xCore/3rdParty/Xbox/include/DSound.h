/**************************************************************************
 *
 *  Copyright (C) 1995-2002 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       dsound.h
 *  Content:    Xbox DirectSound.
 *
 **************************************************************************/

#ifndef __DSOUND_INCLUDED__
#define __DSOUND_INCLUDED__

#include <xtl.h>
#include <dsfxparm.h>

#pragma warning(disable:4201)

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(p)
#endif // UNREFERENCED_PARAMETER

//
// Forward declarations
//

typedef struct XMediaObject XMediaObject;
typedef XMediaObject *LPXMEDIAOBJECT;

typedef struct XFileMediaObject XFileMediaObject;
typedef XFileMediaObject *LPXFILEMEDIAOBJECT;

typedef struct XWaveFileMediaObject XWaveFileMediaObject;
typedef XWaveFileMediaObject *LPXWAVEFILEMEDIAOBJECT;

typedef struct XWmaFileMediaObject XWmaFileMediaObject;
typedef XWmaFileMediaObject *LPXWMAFILEMEDIAOBJECT;

typedef struct IDirectSound IDirectSound;
typedef IDirectSound *LPDIRECTSOUND;

#define IDirectSound8 IDirectSound
#define LPDIRECTSOUND8 LPDIRECTSOUND

#define IDirectSound3DListener IDirectSound
#define LPDIRECTSOUND3DLISTENER LPDIRECTSOUND

typedef struct IDirectSoundBuffer IDirectSoundBuffer;
typedef IDirectSoundBuffer *LPDIRECTSOUNDBUFFER;

#define IDirectSoundBuffer8 IDirectSoundBuffer
#define LPDIRECTSOUNDBUFFER8 LPDIRECTSOUNDBUFFER

#define IDirectSound3DBuffer IDirectSoundBuffer
#define LPDIRECTSOUND3DBUFFER LPDIRECTSOUNDBUFFER

#define IDirectSoundNotify IDirectSoundBuffer
#define LPDIRECTSOUNDNOTIFY LPDIRECTSOUNDBUFFER

typedef struct IDirectSoundStream IDirectSoundStream;
typedef IDirectSoundStream *LPDIRECTSOUNDSTREAM;

typedef struct XAc97MediaObject XAc97MediaObject;
typedef XAc97MediaObject *LPAC97MEDIAOBJECT;

//
// Return Codes
//

#define _FACDS 0x878
#define MAKE_DSHRESULT(code) MAKE_HRESULT(1, _FACDS, code)

// The function completed successfully
#define DS_OK                   S_OK                    

// The control (vol, pan, etc.) requested by the caller is not available
#define DSERR_CONTROLUNAVAIL    MAKE_DSHRESULT(30)      

// This call is not valid for the current state of this object
#define DSERR_INVALIDCALL       MAKE_DSHRESULT(50)      

// An undetermined error occurred inside the DirectSound subsystem
#define DSERR_GENERIC           E_FAIL                  

// Not enough free memory is available to complete the operation
#define DSERR_OUTOFMEMORY       E_OUTOFMEMORY           

// The function called is not supported at this time
#define DSERR_UNSUPPORTED       E_NOTIMPL               

// No sound driver is available for use
#define DSERR_NODRIVER          MAKE_DSHRESULT(120)     

// This object does not support aggregation
#define DSERR_NOAGGREGATION     CLASS_E_NOAGGREGATION   

//
// Format tags
//

#define WAVE_FORMAT_PCM                     1
#define WAVE_FORMAT_XBOX_ADPCM              0x0069
#define WAVE_FORMAT_VOXWARE_VR12            0x0077
#define WAVE_FORMAT_VOXWARE_SC03            0x007A
#define WAVE_FORMAT_VOXWARE_SC06            0x007B
#define WAVE_FORMAT_EXTENSIBLE              0xFFFE

//
// WAVEFORMATEXTENSIBLE sub-format identifiers
//

EXTERN_C const GUID KSDATAFORMAT_SUBTYPE_PCM;
EXTERN_C const GUID KSDATAFORMAT_SUBTYPE_XBOX_ADPCM;

//
// FOURCC codes
//

#ifndef MAKEFOURCC

#define MAKEFOURCC(ch0, ch1, ch2, ch3) \
    ((FOURCC)(BYTE)(ch0) | ((FOURCC)(BYTE)(ch1) << 8) | \
    ((FOURCC)(BYTE)(ch2) << 16) | ((FOURCC)(BYTE)(ch3) << 24 ))

#endif // MAKEFOURCC

//
// memory object types for game replaceable allocation scheme
//

typedef enum _DSOUND_MEMORY_OBJECT_TYPE
{
    eDSoundMemoryObject_WMADecoder = 0,
    eDSoundMemoryObject_WMAXMediaObject,
    eDSoundMemoryObject_WMALookaheadBuffer,
    eDSoundMemoryObject_WMAAsyncContext,
    eDSoundMemoryObject_Max    
} DSOUND_MEMORY_OBJECT_TYPE, *PDSOUND_MEMORY_OBJECT_TYPE, *LPDSOUND_MEMORY_OBJECT_TYPE;

//
// XMediaObject constants
//

#define XMO_STATUSF_ACCEPT_INPUT_DATA           0x00000001      // The object is ready to accept input data
#define XMO_STATUSF_ACCEPT_OUTPUT_DATA          0x00000002      // The object is ready to provide output data
#define XMO_STATUSF_MASK                        0x00000003
                                                
#define XMO_STREAMF_FIXED_SAMPLE_SIZE           0x00000001      // The object supports only a fixed sample size
#define XMO_STREAMF_FIXED_PACKET_ALIGNMENT      0x00000002      // The object supports only a fixed packet alignment
#define XMO_STREAMF_INPUT_ASYNC                 0x00000004      // The object supports receiving input data asynchronously
#define XMO_STREAMF_OUTPUT_ASYNC                0x00000008      // The object supports providing output data asynchronously
#define XMO_STREAMF_IN_PLACE                    0x00000010      // The object supports in-place modification of data
#define XMO_STREAMF_MASK                        0x0000001F

#define XMEDIAPACKET_STATUS_SUCCESS             S_OK            // The packet completed successfully
#define XMEDIAPACKET_STATUS_PENDING             E_PENDING       // The packet is waiting to be processed
#define XMEDIAPACKET_STATUS_FLUSHED             E_ABORT         // The packet was completed as a result of a Flush operation
#define XMEDIAPACKET_STATUS_FAILURE             E_FAIL          // The packet was completed as a result of a failure

//
// Cooperative levels (not used on Xbox)
//

#define DSSCL_NORMAL                0x00000001
#define DSSCL_PRIORITY              0x00000002
#define DSSCL_EXCLUSIVE             0x00000003
#define DSSCL_WRITEPRIMARY          0x00000004

//
// Speaker configuration
//

#define DSSPEAKER_SURROUND          XC_AUDIO_FLAGS_SURROUND     // Dolby Surround
#define DSSPEAKER_STEREO            XC_AUDIO_FLAGS_STEREO       // Stereo
#define DSSPEAKER_MONO              XC_AUDIO_FLAGS_MONO         // Mono
#define DSSPEAKER_ENABLE_AC3        XC_AUDIO_FLAGS_ENABLE_AC3   // Enable Dolby Digital output
#define DSSPEAKER_ENABLE_DTS        XC_AUDIO_FLAGS_ENABLE_DTS   // Enable DTS output
#define DSSPEAKER_USE_DEFAULT       0xFFFFFFFF                  // Use the speaker config set in the Dashboard

#define DSSPEAKER_BASIC(c)          XC_AUDIO_FLAGS_BASIC(c)
#define DSSPEAKER_ENCODED(c)        XC_AUDIO_FLAGS_ENCODED(c)
#define DSSPEAKER_COMBINED(b,e)     XC_AUDIO_FLAGS_COMBINED(b,e)

#define XAudioGetSpeakerConfig      XGetAudioFlags
                                        
//
// DirectSound global headroom ranges
//

#define DSHEADROOM_MIN              0               // Minimum valid headroom value
#define DSHEADROOM_MAX              7               // Maximum valid headroom value
#define DSHEADROOM_DEFAULT          1               // Default headroom value
                                                                        
//
// DirectSound Buffer creation flags
//

#define DSBCAPS_CTRL3D              0x00000010      // The buffer supports 3D
#define DSBCAPS_CTRLFREQUENCY       0x00000020      // The buffer supports frequency changes
#define DSBCAPS_CTRLVOLUME          0x00000080      // The buffer supports volume changes
#define DSBCAPS_CTRLPOSITIONNOTIFY  0x00000100      // The buffer supports position notifications
#define DSBCAPS_MIXIN               0x00002000      // The buffer is to be used as the destination of a submix operation
#define DSBCAPS_MUTE3DATMAXDISTANCE 0x00020000      // The 3D buffer is muted at max distance and beyond
#define DSBCAPS_LOCDEFER            0x00040000      // The buffer does not acquire resources at creation
#define DSBCAPS_FXIN                0x00080000      // The buffer is to be used as the destination of a post-effects submix operation
#define DSBCAPS_FXIN2               0x00100000      // Does not require SetOutputBuffer; does require Play/Stop
                                                                        
//
// IDirectSoundBuffer::Play(Ex) flags
//

#define DSBPLAY_LOOPING             0x00000001      // The buffer should play in a loop
#define DSBPLAY_FROMSTART           0x00000002      // Play the buffer from the beginning, regardless of current position
#define DSBPLAY_SYNCHPLAYBACK       0x00000004      // Synchronize playback of multiple buffers and streams

//
// IDirectSoundBuffer::StopEx flags
//

#define DSBSTOPEX_IMMEDIATE         0x00000000      // The buffer should stop immediately
#define DSBSTOPEX_ENVELOPE          0x00000001      // The buffer should enter its release phase
#define DSBSTOPEX_RELEASEWAVEFORM   0x00000002      // The buffer should break out of the loop region and enter its release phase
                                                                        
//
// Buffer status flags
//

#define DSBSTATUS_PLAYING           0x00000001      // The buffer is playing
#define DSBSTATUS_PAUSED            0x00000002      // The buffer is paused
#define DSBSTATUS_LOOPING           0x00000004      // The buffer is playing in a loop
                                                                            
//
// IDirectSoundBuffer::Lock flags
//

#define DSBLOCK_FROMWRITECURSOR     0x00000001      // Lock the buffer from the current write cursor position
#define DSBLOCK_ENTIREBUFFER        0x00000002      // Lock the entire buffer
                                                                            
//
// Buffer frequency range
//

#define DSBFREQUENCY_MIN            188             // Minimum valid frequency value
#define DSBFREQUENCY_MAX            191983          // Maximum valid frequency value
#define DSBFREQUENCY_ORIGINAL       0               // Reserved value meaning original frequency
                                                                        
//
// Buffer volume range
//

#define DSBVOLUME_MIN               -10000          // Maximum valid attenuation value
#define DSBVOLUME_MAX               0               // Minimum valid attenuation value
#define DSBVOLUME_HW_MIN            -6400           // Minimum volume supported by Xbox hardware

//
// Buffer headroom range 
//

#define DSBHEADROOM_MIN             0               // Minimum valid headroom value
#define DSBHEADROOM_MAX             10000           // Maximum valid headroom value
#define DSBHEADROOM_DEFAULT_2D      600             // Default headroom value for 2D voices
#define DSBHEADROOM_DEFAULT_3D      0               // Default headroom value for 3D voices
#define DSBHEADROOM_DEFAULT_SUBMIX  0               // Default headroom value for submix destinations

//
// Buffer pitch range
//

#define DSBPITCH_MIN                -32767          // Minimum valid pitch value
#define DSBPITCH_MAX                8191            // Maximum valid pitch value
                                                                        
//
// Buffer size range
//

#define DSBSIZE_MIN                 4               // Minimum valid buffer size, in bytes
#define DSBSIZE_MAX                 0x0FFFFFFF      // Maximum valid buffer size, in bytes
                                                                        
//
// Reserved notification offset values
//

#define DSBPN_OFFSETSTOP            0xFFFFFFFF      // Offset value representing "stop" to IDirectSoundNotify

//
// DirectSound Stream creation flags
//

#define DSSTREAMCAPS_CTRL3D                 DSBCAPS_CTRL3D              // The stream supports 3D
#define DSSTREAMCAPS_CTRLFREQUENCY          DSBCAPS_CTRLFREQUENCY       // The stream supports frequency changes
#define DSSTREAMCAPS_CTRLVOLUME             DSBCAPS_CTRLVOLUME          // The stream supports volume changes
#define DSSTREAMCAPS_MUTE3DATMAXDISTANCE    DSBCAPS_MUTE3DATMAXDISTANCE // The 3D stream is muted at max distance and beyond
#define DSSTREAMCAPS_LOCDEFER               DSBCAPS_LOCDEFER            // The stream does not acquire resources at creation
#define DSSTREAMCAPS_NOCOALESCE             0x20000000                  // Do not coalesce packets when writing to hardware
#define DSSTREAMCAPS_ACCURATENOTIFY         0x40000000                  // Do not coalesce packets when writing to hardware and provide more accurate completion notifications
                                                                    
//
// Stream frequency range
//

#define DSSTREAMFREQUENCY_MIN       DSBFREQUENCY_MIN            // Minimum valid frequency value
#define DSSTREAMFREQUENCY_MAX       DSBFREQUENCY_MAX            // Maximum valid frequency value
#define DSSTREAMFREQUENCY_ORIGINAL  DSBFREQUENCY_ORIGINAL       // Reserved value meaning original frequency
                                                                    
//
// Stream volume range
//

#define DSSTREAMVOLUME_MIN          DSBVOLUME_MIN               // Minimum valid volume value
#define DSSTREAMVOLUME_MAX          DSBVOLUME_MAX               // Maximum valid volume value
#define DSSTREAMVOLUME_HW_MIN       DSBVOLUME_HW_MIN            // Minimum volume supported by Xbox hardware

//
// Stream headroom range 
//

#define DSSTREAMHEADROOM_MIN        DSBHEADROOM_MIN             // Minimum valid headroom value
#define DSSTREAMHEADROOM_MAX        DSBHEADROOM_MAX             // Maximum valid headroom value
#define DSSTREAMHEADROOM_DEFAULT_2D DSBHEADROOM_DEFAULT_2D      // Default headroom value for 2D voices
#define DSSTREAMHEADROOM_DEFAULT_3D DSBHEADROOM_DEFAULT_3D      // Default headroom value for 3D voices

//
// Buffer pitch range
//

#define DSSTREAMPITCH_MIN           DSBPITCH_MIN                // Minimum valid pitch value
#define DSSTREAMPITCH_MAX           DSBPITCH_MAX                // Maximum valid pitch value
                                                                        
//
// Buffer pause state
//

#define DSBPAUSE_RESUME             0x00000000                  // Resume a paused buffer
#define DSBPAUSE_PAUSE              0x00000001                  // Pause the buffer
#define DSBPAUSE_SYNCHPLAYBACK      0x00000002                  // Synchronize playback of multiple buffers and streams

//
// Stream pause state
//

#define DSSTREAMPAUSE_RESUME            0x00000000              // Resume a paused stream
#define DSSTREAMPAUSE_PAUSE             0x00000001              // Pause the stream
#define DSSTREAMPAUSE_SYNCHPLAYBACK     0x00000002              // Synchronize playback of multiple buffers and streams
#define DSSTREAMPAUSE_PAUSENOACTIVATE   0x00000003              // Pause the stream without activating the voice

//
// IDirectSoundStream::Stop flags
//

#define DSSTREAMFLUSHEX_IMMEDIATE   0x00000000      // The stream should flush immediately (same as calling Flush)
#define DSSTREAMFLUSHEX_ASYNC       0x00000001      // The stream should begin a flush operation and complete it during DoWork
#define DSSTREAMFLUSHEX_ENVELOPE    0x00000002      // The stream should begin a flush operation using a release envelope
#define DSSTREAMFLUSHEX_ENVELOPE2   0x00000004      // The stream should enter its release segment and accept no more packets once completed

//
// Stream status flags
//

#define DSSTREAMSTATUS_READY            XMO_STATUSF_ACCEPT_INPUT_DATA   // The object is ready to accept input data
#define DSSTREAMSTATUS_PLAYING          0x00010000                      // The stream is playing
#define DSSTREAMSTATUS_PAUSED           0x00020000                      // The stream is paused
#define DSSTREAMSTATUS_STARVED          0x00040000                      // The stream is starved
#define DSSTREAMSTATUS_ENVELOPECOMPLETE 0x00080000                      // The release envelope segment is complete
                                                                        
//
// 3D modes
//

#define DS3DMODE_NORMAL             0x00000000      // Normal 3D mode
#define DS3DMODE_HEADRELATIVE       0x00000001      // Head-relative 3D mode
#define DS3DMODE_DISABLE            0x00000002      // Disable 3D processing

//
// 3D parameter flags
//

#define DS3D_IMMEDIATE              0x00000000      // Apply the values immediately
#define DS3D_DEFERRED               0x00000001      // Defer the values until CommitDeferredSettings is called

//
// 3D bounds and defaults
//

#define DS3D_MINDISTANCEFACTOR          FLT_MIN         // Minimum valid distance factor value
#define DS3D_MAXDISTANCEFACTOR          FLT_MAX         // Maximum valid distance factor value
#define DS3D_DEFAULTDISTANCEFACTOR      1.0f            // Default distance factor value
                                        
#define DS3D_MINROLLOFFFACTOR           0.0f            // Minimum valid rolloff factor value
#define DS3D_MAXROLLOFFFACTOR           1000.0f         // Maximum valid rolloff factor value
#define DS3D_DEFAULTROLLOFFFACTOR       1.0f            // Default rolloff factor value
                                        
#define DS3D_MINDOPPLERFACTOR           0.0f            // Minimum valid Doppler factor value
#define DS3D_MAXDOPPLERFACTOR           10.0f           // Maximum valid Doppler factor value
#define DS3D_DEFAULTDOPPLERFACTOR       1.0f            // Default Doppler factor value
                                        
#define DS3D_MINMINDISTANCE             1.17549e-37f    // Minimum minimum distance value
#define DS3D_MAXMINDISTANCE             FLT_MAX         // Maximum minimum distance value
#define DS3D_DEFAULTMINDISTANCE         1.0f            // Default minimum distance value

#define DS3D_MINMAXDISTANCE             1.17549e-37f    // Minimum maximum distance value
#define DS3D_MAXMAXDISTANCE             FLT_MAX         // Maximum maximum distance value
#define DS3D_DEFAULTMAXDISTANCE         1000000000.0f   // Default maximum distance value
                                        
#define DS3D_MINCONEANGLE               0               // Minimum valid cone angle value
#define DS3D_MAXCONEANGLE               360             // Maximum valid cone angle value
#define DS3D_DEFAULTCONEANGLE           360             // Default cone angle value
                                        
#define DS3D_DEFAULTORIENTFRONT_X       0.0f            // Default front orientation (x)
#define DS3D_DEFAULTORIENTFRONT_Y       0.0f            // Default front orientation (y)
#define DS3D_DEFAULTORIENTFRONT_Z       1.0f            // Default front orientation (z)
                                        
#define DS3D_DEFAULTORIENTTOP_X         0.0f            // Default top orientation (x)
#define DS3D_DEFAULTORIENTTOP_Y         1.0f            // Default top orientation (y)
#define DS3D_DEFAULTORIENTTOP_Z         0.0f            // Default top orientation (z)
                                        
#define DS3D_DEFAULTCONEORIENT_X        0.0f            // Default cone orientation (x)
#define DS3D_DEFAULTCONEORIENT_Y        0.0f            // Default cone orientation (y)
#define DS3D_DEFAULTCONEORIENT_Z        1.0f            // Default cone orientation (z)
                                        
#define DS3D_DEFAULTPOSITION_X          0.0f            // Default position (x)
#define DS3D_DEFAULTPOSITION_Y          0.0f            // Default position (y)
#define DS3D_DEFAULTPOSITION_Z          0.0f            // Default position (z)
                                        
#define DS3D_DEFAULTVELOCITY_X          0.0f            // Default velocity (x)
#define DS3D_DEFAULTVELOCITY_Y          0.0f            // Default velocity (y)
#define DS3D_DEFAULTVELOCITY_Z          0.0f            // Default velocity (z)

#define DS3D_DEFAULTCONEOUTSIDEVOLUME   DSBVOLUME_MAX   // Default cone outside volume

//
// I3DL2 bounds and defaults
//

#define DSI3DL2LISTENER_MINROOM                     -10000
#define DSI3DL2LISTENER_MAXROOM                     0
#define DSI3DL2LISTENER_DEFAULTROOM                 -10000
#define DSI3DL2LISTENER_MINROOMHF                   -10000
#define DSI3DL2LISTENER_MAXROOMHF                   0
#define DSI3DL2LISTENER_DEFAULTROOMHF               0
#define DSI3DL2LISTENER_MINROOMROLLOFFFACTOR        0.0f
#define DSI3DL2LISTENER_MAXROOMROLLOFFFACTOR        10.0f
#define DSI3DL2LISTENER_DEFAULTROOMROLLOFFFACTOR    0.0f
#define DSI3DL2LISTENER_MINDECAYTIME                0.1f
#define DSI3DL2LISTENER_MAXDECAYTIME                20.0f
#define DSI3DL2LISTENER_DEFAULTDECAYTIME            1.0f
#define DSI3DL2LISTENER_MINDECAYHFRATIO             0.1f
#define DSI3DL2LISTENER_MAXDECAYHFRATIO             2.0f
#define DSI3DL2LISTENER_DEFAULTDECAYHFRATIO         0.5f
#define DSI3DL2LISTENER_MINREFLECTIONS              -10000
#define DSI3DL2LISTENER_MAXREFLECTIONS              1000
#define DSI3DL2LISTENER_DEFAULTREFLECTIONS          -10000
#define DSI3DL2LISTENER_MINREFLECTIONSDELAY         0.0f
#define DSI3DL2LISTENER_MAXREFLECTIONSDELAY         0.3f
#define DSI3DL2LISTENER_DEFAULTREFLECTIONSDELAY     0.02f
#define DSI3DL2LISTENER_MINREVERB                   -10000
#define DSI3DL2LISTENER_MAXREVERB                   2000
#define DSI3DL2LISTENER_DEFAULTREVERB               -10000
#define DSI3DL2LISTENER_MINREVERBDELAY              0.0f
#define DSI3DL2LISTENER_MAXREVERBDELAY              0.1f
#define DSI3DL2LISTENER_DEFAULTREVERBDELAY          0.04f
#define DSI3DL2LISTENER_MINDIFFUSION                0.0f
#define DSI3DL2LISTENER_MAXDIFFUSION                100.0f
#define DSI3DL2LISTENER_DEFAULTDIFFUSION            100.0f
#define DSI3DL2LISTENER_MINDENSITY                  0.0f
#define DSI3DL2LISTENER_MAXDENSITY                  100.0f
#define DSI3DL2LISTENER_DEFAULTDENSITY              100.0f
#define DSI3DL2LISTENER_MINHFREFERENCE              20.0f
#define DSI3DL2LISTENER_MAXHFREFERENCE              20000.0f
#define DSI3DL2LISTENER_DEFAULTHFREFERENCE          5000.0f

#define DSI3DL2BUFFER_MINDIRECT                     -10000
#define DSI3DL2BUFFER_MAXDIRECT                     1000
#define DSI3DL2BUFFER_DEFAULTDIRECT                 0
#define DSI3DL2BUFFER_MINDIRECTHF                   -10000
#define DSI3DL2BUFFER_MAXDIRECTHF                   0
#define DSI3DL2BUFFER_DEFAULTDIRECTHF               0
#define DSI3DL2BUFFER_MINROOM                       -10000
#define DSI3DL2BUFFER_MAXROOM                       1000
#define DSI3DL2BUFFER_DEFAULTROOM                   0
#define DSI3DL2BUFFER_MINROOMHF                     -10000
#define DSI3DL2BUFFER_MAXROOMHF                     0
#define DSI3DL2BUFFER_DEFAULTROOMHF                 0
#define DSI3DL2BUFFER_MINROOMROLLOFFFACTOR          0.0f
#define DSI3DL2BUFFER_MAXROOMROLLOFFFACTOR          10.f
#define DSI3DL2BUFFER_DEFAULTROOMROLLOFFFACTOR      0.0f
#define DSI3DL2BUFFER_MINOBSTRUCTION                -10000
#define DSI3DL2BUFFER_MAXOBSTRUCTION                0
#define DSI3DL2BUFFER_DEFAULTOBSTRUCTION            0
#define DSI3DL2BUFFER_MINOBSTRUCTIONLFRATIO         0.0f
#define DSI3DL2BUFFER_MAXOBSTRUCTIONLFRATIO         1.0f
#define DSI3DL2BUFFER_DEFAULTOBSTRUCTIONLFRATIO     0.0f
#define DSI3DL2BUFFER_MINOCCLUSION                  -10000
#define DSI3DL2BUFFER_MAXOCCLUSION                  0
#define DSI3DL2BUFFER_DEFAULTOCCLUSION              0
#define DSI3DL2BUFFER_MINOCCLUSIONLFRATIO           0.0f
#define DSI3DL2BUFFER_MAXOCCLUSIONLFRATIO           1.0f
#define DSI3DL2BUFFER_DEFAULTOCCLUSIONLFRATIO       0.25f

//
// I3DL2 listener environmental presets
//

#define DSI3DL2_ENVIRONMENT_PRESET_DEFAULT           -1000,   -100, 0.0f, 1.49f, 0.83f,  -2602, 0.007f,    200, 0.011f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_DEFAULT2         -10000,      0, 0.0f, 1.00f, 0.50f, -10000, 0.020f, -10000, 0.040f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_GENERIC           -1000,   -100, 0.0f, 1.49f, 0.83f,  -2602, 0.007f,    200, 0.011f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_PADDEDCELL        -1000,  -6000, 0.0f, 0.17f, 0.10f,  -1204, 0.001f,    207, 0.002f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_ROOM              -1000,   -454, 0.0f, 0.40f, 0.83f,  -1646, 0.002f,     53, 0.003f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_BATHROOM          -1000,  -1200, 0.0f, 1.49f, 0.54f,   -370, 0.007f,   1030, 0.011f, 100.0f,  60.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_LIVINGROOM        -1000,  -6000, 0.0f, 0.50f, 0.10f,  -1376, 0.003f,  -1104, 0.004f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_STONEROOM         -1000,   -300, 0.0f, 2.31f, 0.64f,   -711, 0.012f,     83, 0.017f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_AUDITORIUM        -1000,   -476, 0.0f, 4.32f, 0.59f,   -789, 0.020f,   -289, 0.030f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_CONCERTHALL       -1000,   -500, 0.0f, 3.92f, 0.70f,  -1230, 0.020f,     -2, 0.029f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_CAVE              -1000,      0, 0.0f, 2.91f, 1.30f,   -602, 0.015f,   -302, 0.022f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_ARENA             -1000,   -698, 0.0f, 7.24f, 0.33f,  -1166, 0.020f,     16, 0.030f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_HANGAR            -1000,  -1000, 0.0f,10.05f, 0.23f,   -602, 0.020f,    198, 0.030f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_CARPETEDHALLWAY   -1000,  -4000, 0.0f, 0.30f, 0.10f,  -1831, 0.002f,  -1630, 0.030f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_HALLWAY           -1000,   -300, 0.0f, 1.49f, 0.59f,  -1219, 0.007f,    441, 0.011f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_STONECORRIDOR     -1000,   -237, 0.0f, 2.70f, 0.79f,  -1214, 0.013f,    395, 0.020f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_ALLEY             -1000,   -270, 0.0f, 1.49f, 0.86f,  -1204, 0.007f,     -4, 0.011f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_FOREST            -1000,  -3300, 0.0f, 1.49f, 0.54f,  -2560, 0.162f,   -613, 0.088f,  79.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_CITY              -1000,   -800, 0.0f, 1.49f, 0.67f,  -2273, 0.007f,  -2217, 0.011f,  50.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_MOUNTAINS         -1000,  -2500, 0.0f, 1.49f, 0.21f,  -2780, 0.300f,  -2014, 0.100f,  27.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_QUARRY            -1000,  -1000, 0.0f, 1.49f, 0.83f, -10000, 0.061f,    500, 0.025f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_PLAIN             -1000,  -2000, 0.0f, 1.49f, 0.50f,  -2466, 0.179f,  -2514, 0.100f,  21.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_PARKINGLOT        -1000,      0, 0.0f, 1.65f, 1.50f,  -1363, 0.008f,  -1153, 0.012f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_SEWERPIPE         -1000,  -1000, 0.0f, 2.81f, 0.14f,    429, 0.014f,    648, 0.021f,  80.0f,  60.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_UNDERWATER        -1000,  -4000, 0.0f, 1.49f, 0.10f,   -449, 0.007f,   1700, 0.011f, 100.0f, 100.0f, 5000.0f
#define DSI3DL2_ENVIRONMENT_PRESET_NOREVERB         -10000, -10000, 0.0f, 1.00f, 1.00f, -10000, 0.000f, -10000, 0.000f,   0.0f,   0.0f, 5000.0f

//
// I3DL2 source material presets
//

#define DSI3DL2_MATERIAL_PRESET_SINGLEWINDOW        -2800, 0.71f
#define DSI3DL2_MATERIAL_PRESET_DOUBLEWINDOW        -5000, 0.40f
#define DSI3DL2_MATERIAL_PRESET_THINDOOR            -1800, 0.66f
#define DSI3DL2_MATERIAL_PRESET_THICKDOOR           -4400, 0.64f
#define DSI3DL2_MATERIAL_PRESET_WOODWALL            -4000, 0.50f
#define DSI3DL2_MATERIAL_PRESET_BRICKWALL           -5000, 0.60f
#define DSI3DL2_MATERIAL_PRESET_STONEWALL           -6000, 0.68f
#define DSI3DL2_MATERIAL_PRESET_CURTAIN             -1200, 0.15f

//
// MixBin identifiers
//

#define DSMIXBIN_FRONT_LEFT         0
#define DSMIXBIN_FRONT_RIGHT        1
#define DSMIXBIN_FRONT_CENTER       2
#define DSMIXBIN_LOW_FREQUENCY      3
#define DSMIXBIN_BACK_LEFT          4
#define DSMIXBIN_BACK_RIGHT         5
#define DSMIXBIN_SPEAKERS_FIRST     DSMIXBIN_FRONT_LEFT
#define DSMIXBIN_SPEAKERS_LAST      DSMIXBIN_BACK_RIGHT
#define DSMIXBIN_SPEAKERS_COUNT     (DSMIXBIN_SPEAKERS_LAST - DSMIXBIN_SPEAKERS_FIRST + 1)
                                    
#define DSMIXBIN_XTLK_FRONT_LEFT    6
#define DSMIXBIN_XTLK_FRONT_RIGHT   7
#define DSMIXBIN_XTLK_BACK_LEFT     8
#define DSMIXBIN_XTLK_BACK_RIGHT    9
#define DSMIXBIN_XTLK_FIRST         DSMIXBIN_XTLK_FRONT_LEFT
#define DSMIXBIN_XTLK_LAST          DSMIXBIN_XTLK_BACK_RIGHT
#define DSMIXBIN_XTLK_COUNT         (DSMIXBIN_XTLK_LAST - DSMIXBIN_XTLK_FIRST + 1)
                                    
#define DSMIXBIN_I3DL2              10
                                    
#define DSMIXBIN_FXSEND_0           11
#define DSMIXBIN_FXSEND_1           12
#define DSMIXBIN_FXSEND_2           13
#define DSMIXBIN_FXSEND_3           14
#define DSMIXBIN_FXSEND_4           15
#define DSMIXBIN_FXSEND_5           16
#define DSMIXBIN_FXSEND_6           17
#define DSMIXBIN_FXSEND_7           18
#define DSMIXBIN_FXSEND_8           19
#define DSMIXBIN_FXSEND_9           20
#define DSMIXBIN_FXSEND_10          21
#define DSMIXBIN_FXSEND_11          22
#define DSMIXBIN_FXSEND_12          23
#define DSMIXBIN_FXSEND_13          24
#define DSMIXBIN_FXSEND_14          25
#define DSMIXBIN_FXSEND_15          26
#define DSMIXBIN_FXSEND_16          27
#define DSMIXBIN_FXSEND_17          28
#define DSMIXBIN_FXSEND_18          29
#define DSMIXBIN_FXSEND_19          30
#define DSMIXBIN_FXSEND_FIRST       DSMIXBIN_FXSEND_0
#define DSMIXBIN_FXSEND_LAST        DSMIXBIN_FXSEND_19
#define DSMIXBIN_FXSEND_COUNT       (DSMIXBIN_FXSEND_LAST - DSMIXBIN_FXSEND_FIRST + 1)
                                    
#define DSMIXBIN_SUBMIX             31

#define DSMIXBIN_FIRST              DSMIXBIN_FRONT_LEFT
#define DSMIXBIN_LAST               DSMIXBIN_SUBMIX
#define DSMIXBIN_COUNT              (DSMIXBIN_LAST - DSMIXBIN_FIRST + 1)
                                    
#define DSMIXBIN_3D_FRONT_LEFT      DSMIXBIN_XTLK_FRONT_LEFT
#define DSMIXBIN_3D_FRONT_RIGHT     DSMIXBIN_XTLK_FRONT_RIGHT
#define DSMIXBIN_3D_BACK_LEFT       DSMIXBIN_XTLK_BACK_LEFT
#define DSMIXBIN_3D_BACK_RIGHT      DSMIXBIN_XTLK_BACK_RIGHT
#define DSMIXBIN_3D_FIRST           DSMIXBIN_XTLK_FIRST
#define DSMIXBIN_3D_LAST            DSMIXBIN_XTLK_LAST
#define DSMIXBIN_3D_COUNT           DSMIXBIN_XTLK_COUNT

#define DSMIXBIN_VOICE_0            DSMIXBIN_FXSEND_16
#define DSMIXBIN_VOICE_1            DSMIXBIN_FXSEND_17
#define DSMIXBIN_VOICE_2            DSMIXBIN_FXSEND_18
#define DSMIXBIN_VOICE_3            DSMIXBIN_FXSEND_19
#define DSMIXBIN_VOICE_FIRST        DSMIXBIN_VOICE_0
#define DSMIXBIN_VOICE_LAST         DSMIXBIN_VOICE_3
#define DSMIXBIN_VOICE_COUNT        DSMIXBIN_VOICE_LAST - DSMIXBIN_VOICE_FIRST + 1)

//
// Maximum mixbin assignment count
//

#define DSMIXBIN_ASSIGNMENT_MAX     8

//
// Default and required mixbin assignments
//

#define DSMIXBINVOLUMEPAIRS_DEFAULT_MONO \
    { DSMIXBIN_FRONT_LEFT, 0 }, \
    { DSMIXBIN_FRONT_RIGHT, 0 }

#define DSMIXBINVOLUMEPAIRS_DEFAULT_STEREO \
    { DSMIXBIN_FRONT_LEFT, 0 }, \
    { DSMIXBIN_FRONT_RIGHT, 0 }

#define DSMIXBINVOLUMEPAIRS_DEFAULT_4CHANNEL \
    { DSMIXBIN_FRONT_LEFT, 0 }, \
    { DSMIXBIN_FRONT_RIGHT, 0 }, \
    { DSMIXBIN_BACK_LEFT, 0 }, \
    { DSMIXBIN_BACK_RIGHT, 0 }

#define DSMIXBINVOLUMEPAIRS_DEFAULT_6CHANNEL \
    { DSMIXBIN_FRONT_LEFT, 0 }, \
    { DSMIXBIN_FRONT_RIGHT, 0 }, \
    { DSMIXBIN_FRONT_CENTER, 0 }, \
    { DSMIXBIN_LOW_FREQUENCY, 0 }, \
    { DSMIXBIN_BACK_LEFT, 0 }, \
    { DSMIXBIN_BACK_RIGHT, 0 }

#define DSMIXBINVOLUMEPAIRS_REQUIRED_3D \
    { DSMIXBIN_3D_FRONT_LEFT, 0 }, \
    { DSMIXBIN_3D_BACK_LEFT, 0 }, \
    { DSMIXBIN_3D_FRONT_RIGHT, 0 }, \
    { DSMIXBIN_3D_BACK_RIGHT, 0 }
        
#define DSMIXBINVOLUMEPAIRS_DEFAULT_3D \
    DSMIXBINVOLUMEPAIRS_REQUIRED_3D, \
    { DSMIXBIN_I3DL2, 0 }

#define DSMIXBINVOLUMEPAIRS_REQUIRED_5CHANNEL_3D \
    DSMIXBINVOLUMEPAIRS_REQUIRED_3D, \
    { DSMIXBIN_FRONT_CENTER, 0 }
        
#define DSMIXBINVOLUMEPAIRS_DEFAULT_5CHANNEL_3D \
    DSMIXBINVOLUMEPAIRS_REQUIRED_5CHANNEL_3D, \
    { DSMIXBIN_I3DL2, 0 }

#define DSMIXBINVOLUMEPAIRS_DEFAULT_5CHANNEL_3D_PLUS_LFE \
    DSMIXBINVOLUMEPAIRS_DEFAULT_5CHANNEL_3D, \
    { DSMIXBIN_LOW_FREQUENCY, 0 }

#ifdef _XBOX

//
// WAVEFORMATEXTENSIBLE speaker identifiers
//

#define SPEAKER_FRONT_LEFT          0x00000001
#define SPEAKER_FRONT_RIGHT         0x00000002
#define SPEAKER_FRONT_CENTER        0x00000004
#define SPEAKER_LOW_FREQUENCY       0x00000008
#define SPEAKER_BACK_LEFT           0x00000010
#define SPEAKER_BACK_RIGHT          0x00000020
#define SPEAKER_MASK                0x0000003F

#endif //_XBOX

//
// Low-frequency occilator identifiers
//

#define DSLFO_MULTI                 0x00000000      // Multi-function LFO
#define DSLFO_PITCH                 0x00000001      // Pitch-only LFO

//
// Low-frequency occilator parameter boundaries and defaults
//

#define DSLFO_DELAY_MIN             0
#define DSLFO_DELAY_MAX             65535
#define DSLFO_DELAY_DEFAULT         0

#define DSLFO_DELTA_MIN             0
#define DSLFO_DELTA_MAX             1023
#define DSLFO_DELTA_DEFAULT         0

#define DSLFO_PITCHMOD_MIN          -128
#define DSLFO_PITCHMOD_MAX          127
#define DSLFO_PITCHMOD_DEFAULT      0

#define DSLFO_FCRANGE_MIN           -128
#define DSLFO_FCRANGE_MAX           127
#define DSLFO_FCRANGE_DEFAULT       0

#define DSLFO_AMPMOD_MIN            -128
#define DSLFO_AMPMOD_MAX            128
#define DSLFO_AMPMOD_DEFAULT        0

//
// Envelope generator identifiers
//

#define DSEG_MULTI                  0x00000000      // Multi-function EG
#define DSEG_AMPLITUDE              0x00000001      // Amplitude-only EG

//
// Envelope generator modes
//

#define DSEG_MODE_DISABLE           0x00000000      // The envelope is disabled and the envelope value is always full-scale
#define DSEG_MODE_DELAY             0x00000001      // Starts with the envelope at zero amplitude with an initial delay
#define DSEG_MODE_ATTACK            0x00000002      // Bypasses the initial delay and goes directly to the attack envelope
#define DSEG_MODE_HOLD              0x00000003      // Bypasses the attack segment and immediately goes full scale

//
// Envelope generator parameter boundaries and defaults
//

#define DSEG_DELAY_MIN              0
#define DSEG_DELAY_MAX              4095
#define DSEG_DELAY_DEFAULT          0

#define DSEG_ATTACK_MIN             0
#define DSEG_ATTACK_MAX             4095
#define DSEG_ATTACK_DEFAULT         0

#define DSEG_HOLD_MIN               0
#define DSEG_HOLD_MAX               4095
#define DSEG_HOLD_DEFAULT           0

#define DSEG_DECAY_MIN              0
#define DSEG_DECAY_MAX              4095
#define DSEG_DECAY_DEFAULT          0

#define DSEG_RELEASE_MIN            0
#define DSEG_RELEASE_MAX            4095
#define DSEG_RELEASE_DEFAULT        0

#define DSEG_SUSTAIN_MIN            0
#define DSEG_SUSTAIN_MAX            255
#define DSEG_SUSTAIN_DEFAULT        255

#define DSEG_PITCHSCALE_MIN         -128
#define DSEG_PITCHSCALE_MAX         127
#define DSEG_PITCHSCALE_DEFAULT     0

#define DSEG_FILTERCUTOFF_MIN       -128
#define DSEG_FILTERCUTOFF_MAX       127
#define DSEG_FILTERCUTOFF_DEFAULT   0

//
// Filter modes
//

#define DSFILTER_MODE_BYPASS        0x00000000      // The filter is bypassed
#define DSFILTER_MODE_DLS2          0x00000001      // DLS2 mode
#define DSFILTER_MODE_PARAMEQ       0x00000002      // Parametric equalizer mode
#define DSFILTER_MODE_MULTI         0x00000003      // Multifunction mode

//
// Effects parameter flags
//

#define DSFX_IMMEDIATE              0x00000000      // Apply the values immediately
#define DSFX_DEFERRED               0x00000001      // Defer the values until CommitEffectsData is called

//
// Effect index identifiers (for DSEFFECTIMAGELOC)
//

#define DSFX_IMAGELOC_UNUSED        0xFFFFFFFF      // The effect does not appear in the image

//
// AC'97 channel types
//

#define DSAC97_CHANNEL_ANALOG       0x00000000
#define DSAC97_CHANNEL_DIGITAL      0x00000001

//
// AC'97 digital channel modes
//

#define DSAC97_MODE_PCM             0x02000000
#define DSAC97_MODE_ENCODED         0x02000002

//
// AC'97 packet counts
//

#define DSAC97_MAX_ATTACHED_PACKETS 31

//
// WMA in-memory decoder data callback
//

typedef DWORD (CALLBACK *LPFNWMAXMODATACALLBACK)(LPVOID pvContext, DWORD dwOffset, DWORD dwByteCount, LPVOID *ppvData);

//
// XAudioDownloadEffectsImage flags
//

#define XAUDIO_DOWNLOADFX_EXTERNFILE        0x00000000
#define XAUDIO_DOWNLOADFX_XBESECTION        0x00000001

//
// Structures and types
//

#pragma pack(push, 1)

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_

typedef struct tWAVEFORMATEX
{
    WORD            wFormatTag;             // Format type
    WORD            nChannels;              // Channel count
    DWORD           nSamplesPerSec;         // Sampling rate
    DWORD           nAvgBytesPerSec;        // Average number of bytes per second
    WORD            nBlockAlign;            // Block size of data
    WORD            wBitsPerSample;         // Count of bits per mono sample
    WORD            cbSize;                 // Bytes of extra format information following this structure
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;

typedef const WAVEFORMATEX *LPCWAVEFORMATEX;

#endif // _WAVEFORMATEX_

typedef struct xbox_adpcmwaveformat_tag 
{
    WAVEFORMATEX    wfx;                    // WAVEFORMATEX data
    WORD            wSamplesPerBlock;       // Count of samples per encoded block.  Must be 64.
} XBOXADPCMWAVEFORMAT, *PXBOXADPCMWAVEFORMAT, *LPXBOXADPCMWAVEFORMAT;

typedef const XBOXADPCMWAVEFORMAT *LPCXBOXADPCMWAVEFORMAT;

#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_

typedef struct 
{
    WAVEFORMATEX    Format;                 // WAVEFORMATEX data

    union 
    {
        WORD        wValidBitsPerSample;    // Bits of precision
        WORD        wSamplesPerBlock;       // Samples per block of audio data
        WORD        wReserved;              // Unused -- must be 0
    } Samples;

    DWORD           dwChannelMask;          // Channel usage bitmask
    GUID            SubFormat;              // Sub-format identifier
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE, *LPWAVEFORMATEXTENSIBLE;

typedef const WAVEFORMATEXTENSIBLE *LPCWAVEFORMATEXTENSIBLE;

#endif // _WAVEFORMATEXTENSIBLE_

#pragma pack(pop)

typedef LONGLONG REFERENCE_TIME, *PREFERENCE_TIME, *LPREFERENCE_TIME;

typedef struct _XMEDIAPACKET
{
    LPVOID          pvBuffer;               // Packet data buffer
    DWORD           dwMaxSize;              // Packet data buffer size, in bytes
    LPDWORD         pdwCompletedSize;       // Optional pointer to completed size, in bytes
    LPDWORD         pdwStatus;              // Optional pointer to buffer status
    union 
    {
        HANDLE      hCompletionEvent;       // Optional packet completion event
        LPVOID      pContext;               // Optional callback function packet context
    };
    PREFERENCE_TIME prtTimestamp;           // Optional packet timestamp
} XMEDIAPACKET, *PXMEDIAPACKET, *LPXMEDIAPACKET;

typedef const XMEDIAPACKET *LPCXMEDIAPACKET;

typedef struct _XMEDIAINFO
{
    DWORD           dwFlags;                // Object flags (XMEDIA_STREAMF_*)
    DWORD           dwInputSize;            // Input buffer size
    DWORD           dwOutputSize;           // Output buffer size
    DWORD           dwMaxLookahead;         // Maximum amount of data that must be buffered to prevent underrun
} XMEDIAINFO, *PXMEDIAINFO, *LPXMEDIAINFO;

typedef const XMEDIAINFO *LPCXMEDIAINFO;

typedef struct _DSOUTPUTLEVELS 
{
    DWORD   dwAnalogLeftTotalPeak;          // analog peak
    DWORD   dwAnalogRightTotalPeak;
    DWORD   dwAnalogLeftTotalRMS;           // analog RMS
    DWORD   dwAnalogRightTotalRMS;
    DWORD   dwDigitalFrontLeftPeak;         // digital peak levels
    DWORD   dwDigitalFrontCenterPeak;
    DWORD   dwDigitalFrontRightPeak;
    DWORD   dwDigitalBackLeftPeak;
    DWORD   dwDigitalBackRightPeak;
    DWORD   dwDigitalLowFrequencyPeak;
    DWORD   dwDigitalFrontLeftRMS;          // digital RMS levels
    DWORD   dwDigitalFrontCenterRMS;
    DWORD   dwDigitalFrontRightRMS;
    DWORD   dwDigitalBackLeftRMS;
    DWORD   dwDigitalBackRightRMS;
    DWORD   dwDigitalLowFrequencyRMS;
} DSOUTPUTLEVELS, *PDSOUTPUTLEVELS, *LPDSOUTPUTLEVELS;

typedef const DSOUTPUTLEVELS *LPCDSOUTPUTLEVELS;

typedef struct _DSCAPS                                  
{                                                       
    DWORD           dwFree2DBuffers;        // Number of available 2D sound buffers
    DWORD           dwFree3DBuffers;        // Number of available 3D sound buffers
    DWORD           dwFreeBufferSGEs;       // Number of available buffer scatter/gather entries
    DWORD           dwMemoryAllocated;      // Total amount of memory allocated by DirectSound
} DSCAPS, *LPDSCAPS;

typedef const DSCAPS *LPCDSCAPS;

typedef struct _DSMIXBINVOLUMEPAIR
{
    DWORD           dwMixBin;               // MixBin identifier
    LONG            lVolume;                // MixBin volume
} DSMIXBINVOLUMEPAIR, *LPDSMIXBINVOLUMEPAIR;

typedef const DSMIXBINVOLUMEPAIR *LPCDSMIXBINVOLUMEPAIR;

typedef struct _DSMIXBINS
{
    DWORD                   dwMixBinCount;          // Count of mixbins to assign the voice to or mixbins to set volume on
    LPCDSMIXBINVOLUMEPAIR   lpMixBinVolumePairs;    // MixBin identifier/volume pairs
} DSMIXBINS, *LPDSMIXBINS;

typedef const DSMIXBINS *LPCDSMIXBINS;

typedef struct _DSBUFFERDESC
{
    DWORD               dwSize;             // Structure size, in bytes
    DWORD               dwFlags;            // DSBCAPS flags
    DWORD               dwBufferBytes;      // Buffer size, in bytes
    LPWAVEFORMATEX      lpwfxFormat;        // Buffer format
    LPCDSMIXBINS        lpMixBins;          // Output mixbin identifier/volume pairs
    DWORD               dwInputMixBin;      // Input mixbin identifier (FXIN buffers only)
} DSBUFFERDESC, *LPDSBUFFERDESC;

typedef const DSBUFFERDESC *LPCDSBUFFERDESC;

typedef struct _DSBPOSITIONNOTIFY
{
    DWORD           dwOffset;               // Play cursor position
    HANDLE          hEventNotify;           // Notification event
} DSBPOSITIONNOTIFY, *LPDSBPOSITIONNOTIFY;

typedef const DSBPOSITIONNOTIFY *LPCDSBPOSITIONNOTIFY;
typedef VOID (CALLBACK *LPFNXMEDIAOBJECTCALLBACK)(LPVOID pStreamContext, LPVOID pPacketContext, DWORD dwStatus);
typedef LPFNXMEDIAOBJECTCALLBACK PFNXMEDIAOBJECTCALLBACK;

typedef struct _DSSTREAMDESC
{
    DWORD                       dwFlags;                // DSSTREAMCAPS flags
    DWORD                       dwMaxAttachedPackets;   // Maximum count of packets that will be simultaneously submitted to the stream
    LPWAVEFORMATEX              lpwfxFormat;            // Stream format
    LPFNXMEDIAOBJECTCALLBACK    lpfnCallback;           // Packet completion callback routine
    LPVOID                      lpvContext;             // Packet completion callback routine context
    LPCDSMIXBINS                lpMixBins;              // Output mixbin identifier/volume pairs
} DSSTREAMDESC, *LPDSSTREAMDESC;

typedef const DSSTREAMDESC *LPCDSSTREAMDESC;

typedef struct _DS3DBUFFER
{
    DWORD           dwSize;                 // Structure size, in bytes
    D3DXVECTOR3     vPosition;              // Buffer 3D position
    D3DXVECTOR3     vVelocity;              // Buffer 3D velocity
    DWORD           dwInsideConeAngle;      // Buffer inside cone angle
    DWORD           dwOutsideConeAngle;     // Buffer outside cone angle
    D3DXVECTOR3     vConeOrientation;       // Buffer cone orientation
    LONG            lConeOutsideVolume;     // Volume outside the cone
    FLOAT           flMinDistance;          // Minimum distance value
    FLOAT           flMaxDistance;          // Maximum distance value
    DWORD           dwMode;                 // 3D processing mode
    FLOAT           flDistanceFactor;       // Distance factor
    FLOAT           flRolloffFactor;        // Rolloff factor
    FLOAT           flDopplerFactor;        // Doppler factor
} DS3DBUFFER, *LPDS3DBUFFER;

typedef const DS3DBUFFER *LPCDS3DBUFFER;

typedef struct _DS3DLISTENER
{
    DWORD           dwSize;                 // Structure size, in bytes
    D3DXVECTOR3     vPosition;              // Listener 3D position
    D3DXVECTOR3     vVelocity;              // Listener 3D velocity
    D3DXVECTOR3     vOrientFront;           // Listener front orientation
    D3DXVECTOR3     vOrientTop;             // Listener top orientation
    FLOAT           flDistanceFactor;       // Distance factor
    FLOAT           flRolloffFactor;        // Rolloff factor
    FLOAT           flDopplerFactor;        // Doppler factor
} DS3DLISTENER, *LPDS3DLISTENER;

typedef const DS3DLISTENER *LPCDS3DLISTENER;

typedef struct _DSI3DL2OBSTRUCTION
{
    LONG            lHFLevel;               // [-10000, 0] default: 0 mB
    FLOAT           flLFRatio;              // [0.0, 1.0] default: 0.0
} DSI3DL2OBSTRUCTION, *LPDSI3DL2OBSTRUCTION;

typedef const DSI3DL2OBSTRUCTION *LPCDSI3DL2OBSTRUCTION;

typedef struct _DSI3DL2OCCLUSION
{
    LONG            lHFLevel;               // [-10000, 0] default: 0 mB
    FLOAT           flLFRatio;              // [0.0, 1.0] default: 0.25
} DSI3DL2OCCLUSION, *LPDSI3DL2OCCLUSION;

typedef const DSI3DL2OCCLUSION *LPCDSI3DL2OCCLUSION;

typedef struct _DSI3DL2BUFFER
{
    LONG                lDirect;            // [-10000, 1000] default: 0 mB
    LONG                lDirectHF;          // [-10000, 0] default: 0 mB
    LONG                lRoom;              // [-10000, 1000] default: 0 mB
    LONG                lRoomHF;            // [-10000, 0] default: 0 mB
    FLOAT               flRoomRolloffFactor;// [0.0, 10.0] default: 0.0
    DSI3DL2OBSTRUCTION  Obstruction;        // Source obstruction parameters
    DSI3DL2OCCLUSION    Occlusion;          // Source occlusion parameters
} DSI3DL2BUFFER, *LPDSI3DL2BUFFER;

typedef const DSI3DL2BUFFER *LPCDSI3DL2BUFFER;

typedef struct _DSI3DL2LISTENER
{
    LONG            lRoom;                  // [-10000, 0] default: -10000 mB
    LONG            lRoomHF;                // [-10000, 0] default: 0 mB
    FLOAT           flRoomRolloffFactor;    // [0.0, 10.0] default: 0.0
    FLOAT           flDecayTime;            // [0.1, 20.0] default: 1.0 s
    FLOAT           flDecayHFRatio;         // [0.1, 2.0] default: 0.5
    LONG            lReflections;           // [-10000, 1000] default: -10000 mB
    FLOAT           flReflectionsDelay;     // [0.0, 0.3] default: 0.02 s
    LONG            lReverb;                // [-10000, 2000] default: -10000 mB
    FLOAT           flReverbDelay;          // [0.0, 0.1] default: 0.04 s
    FLOAT           flDiffusion;            // [0.0, 100.0] default: 100.0 %
    FLOAT           flDensity;              // [0.0, 100.0] default: 100.0 %
    FLOAT           flHFReference;          // [20.0, 20000.0] default: 5000.0 Hz
} DSI3DL2LISTENER, *LPDSI3DL2LISTENER;

typedef const DSI3DL2LISTENER *LPCDSI3DL2LISTENER;

typedef struct _DSLFODESC
{
    DWORD           dwLFO;                  // LFO to set data on
    DWORD           dwDelay;                // Initial delay before LFO is applied, in 32-sample blocks
    DWORD           dwDelta;                // Delta added to LFO each frame
    LONG            lPitchModulation;       // Pitch modulation
    LONG            lFilterCutOffRange;     // Frequency cutoff range (multi-function LFO only)
    LONG            lAmplitudeModulation;   // Amplitude modulation (multi-function LFO only)
} DSLFODESC, *LPDSLFODESC;

typedef const DSLFODESC *LPCDSLFODESC;

typedef struct _DSENVELOPEDESC
{
    DWORD           dwEG;                   // Envelope generator to set data on
    DWORD           dwMode;                 // Envelope mode
    DWORD           dwDelay;                // Count of 512-sample blocks to delay before attack
    DWORD           dwAttack;               // Attack segment length, in 512-sample blocks
    DWORD           dwHold;                 // Count of 512-sample blocks to hold after attack
    DWORD           dwDecay;                // Decay segment length, in 512-sample blocks
    DWORD           dwRelease;              // Release segment length, in 512-sample blocks
    DWORD           dwSustain;              // Sustain level
    LONG            lPitchScale;            // Pitch scale (multi-function envelope only)
    LONG            lFilterCutOff;          // Filter cut-off (multi-function envelope only)
} DSENVELOPEDESC, *LPDSENVELOPEDESC;

typedef const DSENVELOPEDESC *LPCDSENVELOPEDESC;

typedef struct _DSFILTERDESC
{
    DWORD           dwMode;                 // Filter mode
    DWORD           dwQCoefficient;         // Q-coefficient (PEQ only)
    DWORD           adwCoefficients[4];     // Filter coefficients
} DSFILTERDESC, *LPDSFILTERDESC;

typedef const DSFILTERDESC *LPCDSFILTERDESC;

typedef struct _DSEFFECTMAP
{
    LPVOID          lpvCodeSegment;         // Starting address of the DSP code segment
    DWORD           dwCodeSize;             // Code segment size, in DWORDs
    LPVOID          lpvStateSegment;        // Starting address of the effect state segment
    DWORD           dwStateSize;            // Effect state segment size, in DWORDs
    LPVOID          lpvYMemorySegment;      // Starting address of the DSP Y-memory segment
    DWORD           dwYMemorySize;          // Y-memory segment size, in DWORDs            
    LPVOID          lpvScratchSegment;      // Starting address of the scratch memory segment
    DWORD           dwScratchSize;          // Scratch segment size, in DWORDs
} DSEFFECTMAP, *LPDSEFFECTMAP;

typedef const DSEFFECTMAP *LPCDSEFFECTMAP;

typedef struct _DSEFFECTIMAGEDESC
{
    DWORD           dwEffectCount;          // Count of effects in the image
    DWORD           dwTotalScratchSize;     // total FX delay line scratch used
    DSEFFECTMAP     aEffectMaps[1];         // Variable-length array of effect maps
} DSEFFECTIMAGEDESC, *LPDSEFFECTIMAGEDESC;

typedef const DSEFFECTIMAGEDESC *LPCDSEFFECTIMAGEDESC;

typedef struct _DSEFFECTIMAGELOC
{
    DWORD           dwI3DL2ReverbIndex;     // I3DL2 reverb effect index
    DWORD           dwCrosstalkIndex;       // Crosstalk cancellation effect index
} DSEFFECTIMAGELOC, *LPDSEFFECTIMAGELOC;

typedef const DSEFFECTIMAGELOC *LPCDSEFFECTIMAGELOC;

typedef enum _DSFX_EFFECT_TYPE
{
    DSFX_EFFECT_TYPE_AMPMOD_MONO,
    DSFX_EFFECT_TYPE_AMPMOD_STEREO,
    DSFX_EFFECT_TYPE_CHORUS_MONO,
    DSFX_EFFECT_TYPE_CHORUS_STEREO,
    DSFX_EFFECT_TYPE_DISTORTION,
    DSFX_EFFECT_TYPE_ECHO_MONO,
    DSFX_EFFECT_TYPE_ECHO_STEREO,
    DSFX_EFFECT_TYPE_FLANGE_MONO,
    DSFX_EFFECT_TYPE_FLANGE_STEREO,
    DSFX_EFFECT_TYPE_IIR,
    DSFX_EFFECT_TYPE_IIR2,
    DSFX_EFFECT_TYPE_OSCILLATOR,
    DSFX_EFFECT_TYPE_I3DL2REVERB,
    DSFX_EFFECT_TYPE_MINIREVERB,
    DSFX_EFFECT_TYPE_RMS,
    DSFX_EFFECT_TYPE_SPLITTER,
    DSFX_EFFECT_TYPE_MIXER_2x1,
    DSFX_EFFECT_TYPE_SAMPLE_RATE_CONVERTER
} DSFX_EFFECT_TYPE;

typedef struct _DSFX_HIGH_LEVEL_EFFECT_DESCRIPTION
{
    DSFX_EFFECT_TYPE   effectType;
    union
    {
        struct
        {
            FLOAT   flFrequency;    // [0.0, 20000.0] (Hertz)
            FLOAT   flGain;         // [-30.0, 30.0] (DB)
            FLOAT   flQ;            // (0.0, 30.0] Cannot be 0.0
        } IIR2;
        
        struct 
        {
            FLOAT   flGain;                 // [-30.0, 30.0] (DB)
            FLOAT   flPreFilterFrequency;   // [0.0, 20000.0] (Hertz)
            FLOAT   flPreFilterGain;        // [-30.0, 30.0] (DB)
            FLOAT   flPreFilterQ;           // (0.0, 30.0] Cannot be 0.0
            FLOAT   flPostFilterFrequency;  // [0.0, 20000.0] (Hertz)
            FLOAT   flPostFilterGain;       // [-30.0, 30.0] (DB)
            FLOAT   flPostFilterQ;          // (0.0, 30.0] Cannot be 0.0
        } Distortion;
        
        DSI3DL2LISTENER I3DL2Reverb;
    };
} DSFX_HIGH_LEVEL_EFFECT_DESCRIPTION, *LPDSFX_HIGH_LEVEL_EFFECT_DESCRIPTION, *PDSFX_HIGH_LEVEL_EFFECT_DESCRIPTION;

typedef const DSFX_HIGH_LEVEL_EFFECT_DESCRIPTION* LPCDSFX_HIGH_LEVEL_EFFECT_DESCRIPTION;

typedef struct _DSFX_RAW_EFFECT_DESCRIPTION
{
    DSFX_EFFECT_TYPE   effectType;
    union
    {
        struct
        {
            DWORD dwB0;
            DWORD dwB1;
            DWORD dwB2;
            DWORD dwA1;
            DWORD dwA2;
        } IIR2;
        
        struct
        {
            DWORD dwPreFilterB0;
            DWORD dwPreFilterB1;
            DWORD dwPreFilterB2;
            DWORD dwPreFilterA1;
            DWORD dwPreFilterA2;
            DWORD dwPostFilterB0;
            DWORD dwPostFilterB1;
            DWORD dwPostFilterB2;
            DWORD dwPostFilterA1;
            DWORD dwPostFilterA2;
        } Distortion;
        
        struct
        {
            DWORD                dwReflectionsInputDelay[5];
            DWORD                dwShortReverbInputDelay;
            DWORD                dwLongReverbInputDelay[8];
            DWORD                dwReflectionsFeedbackDelay[4];
            DWORD                dwLongReverbFeedbackDelay;
            DWORD                dwShortReverbInputGain[8];
            DWORD                dwLongReverbInputGain;
            DWORD                dwLongReverbCrossfeedGain;
            DWORD                dwReflectionsOutputGain[4];
            DWORD                dwShortReverbOutputGain;
            DWORD                dwLongReverbOutputGain;
            DWORD                dwChannelCount;
            DSFX_I3DL2REVERB_IIR IIR[10];
        } I3DL2Reverb;
    };
} DSFX_RAW_EFFECT_DESCRIPTION, *LPDSFX_RAW_EFFECT_DESCRIPTION, *PDSFX_RAW_EFFECT_DESCRIPTION;

typedef const DSFX_RAW_EFFECT_DESCRIPTION* LPCDSFX_RAW_EFFECT_DESCRIPTION;

typedef struct _DSVOICEPROPS
{
    DWORD               dwMixBinCount;                              // Active mixbin count
    DSMIXBINVOLUMEPAIR  MixBinVolumePairs[DSMIXBIN_ASSIGNMENT_MAX]; // Mixbin/volume pairs
    LONG                lPitch;                                     // Voice pitch
    LONG                l3DDistanceVolume;                          // 3D distance attenuation
    LONG                l3DConeVolume;                              // 3D cone volume
    LONG                l3DDopplerPitch;                            // 3D Doppler pitch
    LONG                lI3DL2DirectVolume;                         // I3DL2 direct path volume
    LONG                lI3DL2RoomVolume;                           // I3DL2 reflected path volume
} DSVOICEPROPS, *LPDSVOICEPROPS;

typedef const DSVOICEPROPS *LPCDSVOICEPROPS;

#include <pshpack1.h>

typedef struct _WMAXMODECODERPARAMETERS
{
    LPCSTR  pszFileName;
    HANDLE  hFile;
    DWORD   dwFileOffset;
    DWORD   dwLookaheadBufferSize;
} WMAXMODECODERPARAMETERS, *LPWMAXMODECODERPARAMETERS;

typedef const WMAXMODECODERPARAMETERS *LPCWMAXMODECODERPARAMETERS;

typedef struct _WMAXMOFileContDesc
{
    WORD        wTitleLength;
    WORD        wAuthorLength;
    WORD        wCopyrightLength;
    WORD        wDescriptionLength;
    WORD        wRatingLength;
    WCHAR *     pTitle;
    WCHAR *     pAuthor;
    WCHAR *     pCopyright;
    WCHAR *     pDescription;
    WCHAR *     pRating;
} WMAXMOFileContDesc, *LPWMAXMOFileContDesc;

#include <poppack.h>

typedef struct _WMAXMOFileHeader
{
    DWORD       dwVersion;
    DWORD       dwSampleRate;
    DWORD       dwNumChannels;
    DWORD       dwDuration;
    DWORD       dwBitrate;
} WMAXMOFileHeader;

typedef DWORD FOURCC, *PFOURCC, *LPFOURCC;

//
// Globals
//

EXTERN_C const DSMIXBINS DirectSoundDefaultMixBins_Mono;
EXTERN_C const DSMIXBINS DirectSoundDefaultMixBins_Stereo;
EXTERN_C const DSMIXBINS DirectSoundDefaultMixBins_4Channel;
EXTERN_C const DSMIXBINS DirectSoundDefaultMixBins_6Channel;

EXTERN_C const DSMIXBINS DirectSoundRequiredMixBins_3D;
EXTERN_C const DSMIXBINS DirectSoundDefaultMixBins_3D;
EXTERN_C const DSMIXBINS DirectSoundRequiredMixBins_5Channel3D;
EXTERN_C const DSMIXBINS DirectSoundDefaultMixBins_5Channel3D;
EXTERN_C const DSMIXBINS DirectSoundDefaulMixBins_5Channel3D_PlusLFE;

EXTERN_C const DS3DBUFFER DirectSoundDefault3DBuffer;
EXTERN_C const DSI3DL2BUFFER DirectSoundDefaultI3DL2Buffer;

EXTERN_C const DS3DLISTENER DirectSoundDefault3DListener;

EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Default;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Default2;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Generic;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_PaddedCell;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Room;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Bathroom;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_LivingRoom;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_StoneRoom;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Auditorium;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_ConcertHall;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Cave;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Arena;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Hangar;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_CarpetedHallway;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Hallway;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_StoneCorridor;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Alley;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Forest;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_City;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Mountains;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Quarry;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Plain;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_ParkingLot;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_SewerPipe;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_Underwater;
EXTERN_C const DSI3DL2LISTENER DirectSoundI3DL2ListenerPreset_NoReverb;

EXTERN_C DWORD g_dwDirectSoundDebugLevel;
EXTERN_C DWORD g_dwDirectSoundDebugBreakLevel;

typedef BOOL (CALLBACK *LPFNDSDPFCALLBACK)(DWORD dwLevel, LPCSTR pszString);
EXTERN_C LPFNDSDPFCALLBACK g_pfnDirectSoundDebugCallback;

//
// API
//

STDAPI DirectSoundCreate(LPGUID pguidDeviceId, LPDIRECTSOUND *ppDirectSound, LPUNKNOWN pUnkOuter);
STDAPI DirectSoundCreateBuffer(LPCDSBUFFERDESC pdsbd, LPDIRECTSOUNDBUFFER *ppBuffer);
STDAPI DirectSoundCreateStream(LPCDSSTREAMDESC pdssd, LPDIRECTSOUNDSTREAM *ppStream);
STDAPI_(void) DirectSoundDoWork(void);
STDAPI_(void) DirectSoundUseFullHRTF(void);
STDAPI_(void) DirectSoundUseLightHRTF(void);
STDAPI_(void) DirectSoundUseFullHRTF4Channel(void);
STDAPI_(void) DirectSoundUseLightHRTF4Channel(void);
STDAPI_(void) DirectSoundOverrideSpeakerConfig(DWORD dwSpeakerConfig);
STDAPI_(DWORD) DirectSoundGetSampleTime(void);
STDAPI_(VOID) DirectSoundDumpMemoryUsage(BOOL fAssertNone);

STDAPI_(void) XAudioCreatePcmFormat(WORD nChannels, DWORD nSamplesPerSec, WORD wBitsPerSample, LPWAVEFORMATEX pwfx);
STDAPI_(void) XAudioCreateAdpcmFormat(WORD nChannels, DWORD nSamplesPerSec, LPXBOXADPCMWAVEFORMAT pwfx);

STDAPI_(LONG) XAudioCalculatePitch(DWORD dwFrequency);

STDAPI XWmaDecoderCreateMediaObject(LPCWMAXMODECODERPARAMETERS pParameters, XWmaFileMediaObject **ppMediaObject);
STDAPI WmaCreateDecoder(LPCSTR pszFileName, HANDLE hFile, BOOL fAsyncMode, DWORD dwLookaheadBufferSize, DWORD dwMaxPackets, DWORD dwYieldRate, LPWAVEFORMATEX pwfxCompressed, XFileMediaObject **ppMediaObject);
STDAPI WmaCreateInMemoryDecoder(LPFNWMAXMODATACALLBACK pfnCallback, LPVOID pvContext, DWORD dwYieldRate, LPWAVEFORMATEX pwfxCompressed, LPXMEDIAOBJECT *ppMediaObject);

STDAPI WmaCreateDecoderEx(LPCSTR pszFileName, HANDLE hFile, BOOL fAsyncMode, DWORD dwLookaheadBufferSize, DWORD dwMaxPackets, DWORD dwYieldRate, LPWAVEFORMATEX pwfxCompressed, XWmaFileMediaObject **ppMediaObject);
STDAPI WmaCreateInMemoryDecoderEx(LPFNWMAXMODATACALLBACK pfnCallback, LPVOID pvContext, DWORD dwYieldRate, LPWAVEFORMATEX pwfxCompressed, XWmaFileMediaObject **ppMediaObject);

STDAPI Ac97CreateMediaObject(DWORD dwChannel, LPFNXMEDIAOBJECTCALLBACK pfnCallback, LPVOID pvContext, LPAC97MEDIAOBJECT *ppMediaObject);

STDAPI XFileCreateMediaObject(LPCSTR pszFileName, DWORD dwDesiredAccess, DWORD dwShareMode, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, XFileMediaObject **ppMediaObject);
STDAPI XFileCreateMediaObjectEx(HANDLE hFile, XFileMediaObject **ppMediaObject);
STDAPI XFileCreateMediaObjectAsync(HANDLE hFile, DWORD dwMaxPackets, XFileMediaObject **ppMediaObject);

STDAPI XWaveFileCreateMediaObject(LPCSTR pszFileName, LPCWAVEFORMATEX *ppwfxFormat, XFileMediaObject **ppMediaObject);
STDAPI XWaveFileCreateMediaObjectEx(LPCSTR pszFileName, HANDLE hFile, XWaveFileMediaObject **ppMediaObject);

STDAPI XAudioDownloadEffectsImage(LPCSTR pszImageName, LPCDSEFFECTIMAGELOC pImageLoc, DWORD dwFlags, LPDSEFFECTIMAGEDESC *ppImageDesc);

STDAPI XAudioSetEffectData(DWORD dwEffectIndex, LPCDSFX_HIGH_LEVEL_EFFECT_DESCRIPTION pDesc, LPDSFX_RAW_EFFECT_DESCRIPTION pRawDesc);

//
// IUnknown
//

#ifndef IUnknown_AddRef
#if defined(__cplusplus) && !defined(CINTERFACE)
#define IUnknown_AddRef(p)  p->AddRef()
#else // defined(__cplusplus) && !defined(CINTERFACE)
#define IUnknown_AddRef(p)  p->lpVtbl->AddRef(p)
#endif // defined(__cplusplus) && !defined(CINTERFACE)
#endif // IUnknown_Addref

#ifndef IUnknown_Release
#if defined(__cplusplus) && !defined(CINTERFACE)
#define IUnknown_Release(p) p->Release()
#else // defined(__cplusplus) && !defined(CINTERFACE)
#define IUnknown_Release(p) p->lpVtbl->Release(p)
#endif // defined(__cplusplus) && !defined(CINTERFACE)
#endif // IUnknown_Release

//
// XMediaObject
//

#undef INTERFACE
#define INTERFACE XMediaObject

DECLARE_INTERFACE(XMediaObject)
{
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // XMediaObject methods
    STDMETHOD(GetInfo)(THIS_ LPXMEDIAINFO pInfo) PURE;
    STDMETHOD(GetStatus)(THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Process)(THIS_ LPCXMEDIAPACKET pInputPacket, LPCXMEDIAPACKET pOutputPacket) PURE;
    STDMETHOD(Discontinuity)(THIS) PURE;
    STDMETHOD(Flush)(THIS) PURE;
};

#define XMediaObject_AddRef             IUnknown_AddRef
#define XMediaObject_Release            IUnknown_Release

#if defined(__cplusplus) && !defined(CINTERFACE)

#define XMediaObject_GetInfo(p, a)      p->GetInfo(a)
#define XMediaObject_GetStatus(p, a)    p->GetStatus(a)
#define XMediaObject_Process(p, a, b)   p->Process(a, b)
#define XMediaObject_Discontinuity(p)   p->Discontinuity()
#define XMediaObject_Flush(p)           p->Flush()

#else // defined(__cplusplus) && !defined(CINTERFACE)

#define XMediaObject_GetInfo(p, a)      p->lpVtbl->GetInfo(p, a)
#define XMediaObject_GetStatus(p, a)    p->lpVtbl->GetStatus(p, a)
#define XMediaObject_Process(p, a, b)   p->lpVtbl->Process(p, a, b)
#define XMediaObject_Discontinuity(p)   p->lpVtbl->Discontinuity(p)
#define XMediaObject_Flush(p)           p->lpVtbl->Flush(p)

#endif // defined(__cplusplus) && !defined(CINTERFACE)

//
// XFileMediaObject
//

#undef INTERFACE
#define INTERFACE XFileMediaObject

DECLARE_INTERFACE_(XFileMediaObject, XMediaObject)
{
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // XMediaObject methods
    STDMETHOD(GetInfo)(THIS_ LPXMEDIAINFO pInfo) PURE;
    STDMETHOD(GetStatus)(THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Process)(THIS_ LPCXMEDIAPACKET pInputPacket, LPCXMEDIAPACKET pOutputPacket) PURE;
    STDMETHOD(Discontinuity)(THIS) PURE;
    STDMETHOD(Flush)(THIS) PURE;

    // XFileMediaObject methods
    STDMETHOD(Seek)(THIS_ LONG lOffset, DWORD dwOrigin, LPDWORD pdwAbsolute) PURE;
    STDMETHOD(GetLength)(THIS_ LPDWORD pdwLength) PURE;
    STDMETHOD_(VOID,DoWork)(THIS) PURE;

};

#define XFileMediaObject_AddRef             IUnknown_AddRef
#define XFileMediaObject_Release            IUnknown_Release

#define XFileMediaObject_GetInfo            XMediaObject_GetInfo
#define XFileMediaObject_GetStatus          XMediaObject_GetStatus
#define XFileMediaObject_Process            XMediaObject_Process
#define XFileMediaObject_Discontinuity      XMediaObject_Discontinuity
#define XFileMediaObject_Flush              XMediaObject_Flush

#if defined(__cplusplus) && !defined(CINTERFACE)

#define XFileMediaObject_Seek(p, a, b, c)   p->Seek(a, b, c)
#define XFileMediaObject_GetLength(p, a)    p->GetLength(a)
#define XFileMediaObject_DoWork(p)          p->DoWork()

#else // defined(__cplusplus) && !defined(CINTERFACE)

#define XFileMediaObject_Seek(p, a, b, c)   p->lpVtbl->Seek(p, a, b, c)
#define XFileMediaObject_GetLength(p, a)    p->lpVtbl->GetLength(p, a)
#define XFileMediaObject_DoWork(p)          p->lpVtbl->DoWork(p)


#endif // defined(__cplusplus) && !defined(CINTERFACE)

//
// XWaveFileMediaObject
//

#undef INTERFACE
#define INTERFACE XWaveFileMediaObject

DECLARE_INTERFACE_(XWaveFileMediaObject, XFileMediaObject)
{
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // XMediaObject methods
    STDMETHOD(GetInfo)(THIS_ LPXMEDIAINFO pInfo) PURE;
    STDMETHOD(GetStatus)(THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Process)(THIS_ LPCXMEDIAPACKET pInputPacket, LPCXMEDIAPACKET pOutputPacket) PURE;
    STDMETHOD(Discontinuity)(THIS) PURE;
    STDMETHOD(Flush)(THIS) PURE;

    // XFileMediaObject methods
    STDMETHOD(Seek)(THIS_ LONG lOffset, DWORD dwOrigin, LPDWORD pdwAbsolute) PURE;
    STDMETHOD(GetLength)(THIS_ LPDWORD pdwLength) PURE;
    STDMETHOD_(VOID,DoWork)(THIS) PURE;

    // XWaveFileMediaObject methods
    STDMETHOD(GetFormat)(THIS_ LPCWAVEFORMATEX *ppwfxFormat) PURE;
    STDMETHOD(GetLoopRegion)(THIS_ LPDWORD pdwLoopStart, LPDWORD pdwLoopLength) PURE;
};

#define XFileMediaObject_AddRef             IUnknown_AddRef
#define XFileMediaObject_Release            IUnknown_Release

#define XWaveFileMediaObject_GetInfo        XFileMediaObject_GetInfo
#define XWaveFileMediaObject_GetStatus      XFileMediaObject_GetStatus
#define XWaveFileMediaObject_Process        XFileMediaObject_Process
#define XWaveFileMediaObject_Discontinuity  XFileMediaObject_Discontinuity
#define XWaveFileMediaObject_Flush          XFileMediaObject_Flush
#define XWaveFileMediaObject_Seek           XFileMediaObject_Seek
#define XWaveFileMediaObject_GetLength      XFileMediaObject_GetLength
#define XWaveFileMediaObject_DoWork         XFileMediaObject_DoWork

#if defined(__cplusplus) && !defined(CINTERFACE)

#define XWaveFileMediaObject_GetFormat(p, a)        p->GetFormat(a)
#define XWaveFileMediaObject_GetLoopRegion(p, a, b) p->GetLoopRegion(a, b)

#else // defined(__cplusplus) && !defined(CINTERFACE)

#define XWaveFileMediaObject_GetFormat(p, a)        p->lpVtbl->GetFormat(p, a)
#define XWaveFileMediaObject_GetLoopRegion(p, a, b) p->lpVtbl->GetLoopRegion(p, a, b)

#endif // defined(__cplusplus) && !defined(CINTERFACE)

//
// XWmaFileMediaObject
//

#undef INTERFACE
#define INTERFACE XWmaFileMediaObject

DECLARE_INTERFACE_(XWmaFileMediaObject, XFileMediaObject)
{
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // XMediaObject methods
    STDMETHOD(GetInfo)(THIS_ LPXMEDIAINFO pInfo) PURE;
    STDMETHOD(GetStatus)(THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Process)(THIS_ LPCXMEDIAPACKET pInputPacket, LPCXMEDIAPACKET pOutputPacket) PURE;
    STDMETHOD(Discontinuity)(THIS) PURE;
    STDMETHOD(Flush)(THIS) PURE;

    // XFileMediaObject methods
    STDMETHOD(Seek)(THIS_ LONG lOffset, DWORD dwOrigin, LPDWORD pdwAbsolute) PURE;
    STDMETHOD(GetLength)(THIS_ LPDWORD pdwLength) PURE;
    STDMETHOD_(VOID,DoWork)(THIS) PURE;

    // XWmaFileMediaObject methods
    STDMETHOD(GetFileHeader)(THIS_ WMAXMOFileHeader* pFileHeader) PURE;
    STDMETHOD(GetFileContentDescription)(THIS_ WMAXMOFileContDesc* pContentDesc) PURE;
    STDMETHOD(SeekToTime)(THIS_ DWORD dwSeek, LPDWORD pdwAcutalSeek) PURE;


};

#define XWmaFileMediaObject_AddRef          IUnknown_AddRef
#define XWmaFileMediaObject_Release         IUnknown_Release

#define XWmaFileMediaObject_GetInfo         XMediaObject_GetInfo
#define XWmaFileMediaObject_GetStatus       XMediaObject_GetStatus
#define XWmaFileMediaObject_Process         XMediaObject_Process
#define XWmaFileMediaObject_Discontinuity   XMediaObject_Discontinuity
#define XWmaFileMediaObject_Flush           XMediaObject_Flush

#define XWmaFileMediaObject_Seek            XFileMediaObject_Seek
#define XWmaFileMediaObject_GetLength       XFileMediaObject_GetLength
#define XWmaFileMediaObject_DoWork          XFileMediaObject_DoWork

#if defined(__cplusplus) && !defined(CINTERFACE)

#define XWmaFileMediaObject_GetFileHeader(p, a)             p->GetFileHeader(a)
#define XWmaFileMediaObject_GetFileContentDescription(p, a) p->GetFileContentDescription(a)

#else // defined(__cplusplus) && !defined(CINTERFACE)

#define XWmaFileMediaObject_GetFileHeader(p, a)             p->lpVtbl->GetFileHeader(p, a)
#define XWmaFileMediaObject_GetFileContentDescription(p, a) p->lpVtbl->GetFileContentDescription(p,a)
#define XWmaFileMediaObject_SeekToTime(p, a, b)             p->lpVtbl->SeekToTime(p,a,b)


#endif // defined(__cplusplus) && !defined(CINTERFACE)

//
// IDirectSound
//

#if defined(__cplusplus) && !defined(CINTERFACE)

STDAPI IDirectSound_QueryInterface(LPDIRECTSOUND pDirectSound, REFIID iid, LPVOID *ppvInterface);

#else // defined(__cplusplus) && !defined(CINTERFACE)

STDAPI IDirectSound_QueryInterfaceC(LPDIRECTSOUND pDirectSound, const IID *iid, LPVOID *ppvInterface);
#define IDirectSound_QueryInterface IDirectSound_QueryInterfaceC

#endif // defined(__cplusplus) && !defined(CINTERFACE)

STDAPI_(ULONG) IDirectSound_AddRef(LPDIRECTSOUND pDirectSound);
STDAPI_(ULONG) IDirectSound_Release(LPDIRECTSOUND pDirectSound);
STDAPI IDirectSound_GetCaps(LPDIRECTSOUND pDirectSound, LPDSCAPS pdsc);
STDAPI IDirectSound_CreateSoundBuffer(LPDIRECTSOUND pDirectSound, LPCDSBUFFERDESC pdsbd, LPDIRECTSOUNDBUFFER *ppBuffer, LPUNKNOWN pUnkOuter);
STDAPI IDirectSound_CreateSoundStream(LPDIRECTSOUND pDirectSound, LPCDSSTREAMDESC pdssd, LPDIRECTSOUNDSTREAM *ppStream, LPUNKNOWN pUnkOuter);
STDAPI IDirectSound_GetSpeakerConfig(LPDIRECTSOUND pDirectSound, LPDWORD pdwSpeakerConfig);
STDAPI IDirectSound_SetCooperativeLevel(LPDIRECTSOUND pDirectSound, HWND hWnd, DWORD dwLevel);
STDAPI IDirectSound_Compact(LPDIRECTSOUND pDirectSound);
STDAPI IDirectSound_DownloadEffectsImage(LPDIRECTSOUND pDirectSound, LPCVOID pvImageBuffer, DWORD dwImageSize, LPCDSEFFECTIMAGELOC pImageLoc, LPDSEFFECTIMAGEDESC *ppImageDesc);
STDAPI IDirectSound_GetEffectData(LPDIRECTSOUND pDirectSound, DWORD dwEffectIndex, DWORD dwOffset, LPVOID pvData, DWORD dwDataSize);
STDAPI IDirectSound_SetEffectData(LPDIRECTSOUND pDirectSound, DWORD dwEffectIndex, DWORD dwOffset, LPCVOID pvData, DWORD dwDataSize, DWORD dwApply);
STDAPI IDirectSound_CommitEffectData(LPDIRECTSOUND pDirectSound);
STDAPI IDirectSound_EnableHeadphones(LPDIRECTSOUND pDirectSound, BOOL fEnabled);
STDAPI IDirectSound_SetMixBinHeadroom(LPDIRECTSOUND pDirectSound, DWORD dwMixBin, DWORD dwHeadroom);
STDAPI IDirectSound_SetAllParameters(LPDIRECTSOUND pDirectSound, LPCDS3DLISTENER pds3dl, DWORD dwApply);
STDAPI IDirectSound_SetOrientation(LPDIRECTSOUND pDirectSound, FLOAT xFront, FLOAT yFront, FLOAT zFront, FLOAT xTop, FLOAT yTop, FLOAT zTop, DWORD dwApply);
STDAPI IDirectSound_SetPosition(LPDIRECTSOUND pDirectSound, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IDirectSound_SetVelocity(LPDIRECTSOUND pDirectSound, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IDirectSound_SetDistanceFactor(LPDIRECTSOUND pDirectSound, FLOAT flDistanceFactor, DWORD dwApply);
STDAPI IDirectSound_SetDopplerFactor(LPDIRECTSOUND pDirectSound, FLOAT flDopplerFactor, DWORD dwApply);
STDAPI IDirectSound_SetRolloffFactor(LPDIRECTSOUND pDirectSound, FLOAT flRolloffFactor, DWORD dwApply);
STDAPI IDirectSound_SetI3DL2Listener(LPDIRECTSOUND pDirectSound, LPCDSI3DL2LISTENER pds3dl, DWORD dwApply);
STDAPI IDirectSound_CommitDeferredSettings(LPDIRECTSOUND pDirectSound);
STDAPI IDirectSound_GetTime(LPDIRECTSOUND pDirectSound, REFERENCE_TIME *prtCurrent);
STDAPI IDirectSound_GetOutputLevels(LPDIRECTSOUND pDirectSound, LPDSOUTPUTLEVELS pOutputLevels, BOOL fResetPeakValues);
STDAPI IDirectSound_SynchPlayback(LPDIRECTSOUND pDirectSound);

#if defined(__cplusplus) && !defined(CINTERFACE)

struct IDirectSound
{
    __inline HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppvInterface)
    {
        return IDirectSound_QueryInterface(this, iid, ppvInterface);
    }

    __inline ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return IDirectSound_AddRef(this);
    }

    __inline ULONG STDMETHODCALLTYPE Release(void)
    {
        return IDirectSound_Release(this);
    }

    __inline HRESULT STDMETHODCALLTYPE GetCaps(LPDSCAPS pdsc)
    {
        return IDirectSound_GetCaps(this, pdsc);
    }

    __inline HRESULT STDMETHODCALLTYPE CreateSoundBuffer(LPCDSBUFFERDESC pdsbd, LPDIRECTSOUNDBUFFER *ppBuffer, LPUNKNOWN pUnkOuter)
    {
        return IDirectSound_CreateSoundBuffer(this, pdsbd, ppBuffer, pUnkOuter);
    }

    __inline HRESULT STDMETHODCALLTYPE CreateSoundStream(LPCDSSTREAMDESC pdssd, LPDIRECTSOUNDSTREAM *ppStream, LPUNKNOWN pUnkOuter)
    {
        return IDirectSound_CreateSoundStream(this, pdssd, ppStream, pUnkOuter);
    }

    __inline HRESULT STDMETHODCALLTYPE GetSpeakerConfig(LPDWORD pdwSpeakerConfig)
    {
        return IDirectSound_GetSpeakerConfig(this, pdwSpeakerConfig);
    }

    __inline HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hWnd, DWORD dwLevel)
    {
        return IDirectSound_SetCooperativeLevel(this, hWnd, dwLevel);
    }

    __inline HRESULT STDMETHODCALLTYPE Compact(void)
    {
        return IDirectSound_Compact(this);
    }

    __inline HRESULT STDMETHODCALLTYPE DownloadEffectsImage(LPCVOID pvImageBuffer, DWORD dwImageSize, LPCDSEFFECTIMAGELOC pImageLoc, LPDSEFFECTIMAGEDESC *ppImageDesc)
    {
        return IDirectSound_DownloadEffectsImage(this, pvImageBuffer, dwImageSize, pImageLoc, ppImageDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE GetEffectData(DWORD dwEffectIndex, DWORD dwOffset, LPVOID pvData, DWORD dwDataSize)
    {
        return IDirectSound_GetEffectData(this, dwEffectIndex, dwOffset, pvData, dwDataSize);
    }

    __inline HRESULT STDMETHODCALLTYPE SetEffectData(DWORD dwEffectIndex, DWORD dwOffset, LPCVOID pvData, DWORD dwDataSize, DWORD dwApply)
    {
        return IDirectSound_SetEffectData(this, dwEffectIndex, dwOffset, pvData, dwDataSize, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE CommitEffectData(void)
    {
        return IDirectSound_CommitEffectData(this);
    }

    __inline HRESULT STDMETHODCALLTYPE EnableHeadphones(BOOL fEnabled)
    {
        return IDirectSound_EnableHeadphones(this, fEnabled);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMixBinHeadroom(DWORD dwMixBin, DWORD dwHeadroom)
    {
        return IDirectSound_SetMixBinHeadroom(this, dwMixBin, dwHeadroom);
    }

    __inline HRESULT STDMETHODCALLTYPE SetAllParameters(LPCDS3DLISTENER pds3dl, DWORD dwApply)
    {
        return IDirectSound_SetAllParameters(this, pds3dl, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetDistanceFactor(FLOAT flDistanceFactor, DWORD dwApply)
    {
        return IDirectSound_SetDistanceFactor(this, flDistanceFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetDopplerFactor(FLOAT flDopplerFactor, DWORD dwApply)
    {
        return IDirectSound_SetDopplerFactor(this, flDopplerFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetOrientation(FLOAT xFront, FLOAT yFront, FLOAT zFront, FLOAT xTop, FLOAT yTop, FLOAT zTop, DWORD dwApply)
    {
        return IDirectSound_SetOrientation(this, xFront, yFront, zFront, xTop, yTop, zTop, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPosition(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IDirectSound_SetPosition(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetRolloffFactor(FLOAT flRolloffFactor, DWORD dwApply)
    {
        return IDirectSound_SetRolloffFactor(this, flRolloffFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetVelocity(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IDirectSound_SetVelocity(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetI3DL2Listener(LPCDSI3DL2LISTENER pds3dl, DWORD dwApply)
    {
        return IDirectSound_SetI3DL2Listener(this, pds3dl, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE CommitDeferredSettings(void)
    {
        return IDirectSound_CommitDeferredSettings(this);
    }

    __inline HRESULT STDMETHODCALLTYPE GetTime(REFERENCE_TIME *prtCurrent)
    {
        return IDirectSound_GetTime(this, prtCurrent);
    }

    __inline HRESULT STDMETHODCALLTYPE GetOutputLevels(LPDSOUTPUTLEVELS pOutputLevels, BOOL fResetPeakValues)
    {
        return IDirectSound_GetOutputLevels(this, pOutputLevels, fResetPeakValues);
    }

    __inline HRESULT STDMETHODCALLTYPE SynchPlayback(void)
    {
        return IDirectSound_SynchPlayback(this);
    }
};

#endif // defined(__cplusplus) && !defined(CINTERFACE)

#define IDirectSound8_QueryInterface                        IDirectSound_QueryInterface
#define IDirectSound8_AddRef                                IDirectSound_AddRef
#define IDirectSound8_Release                               IDirectSound_Release
#define IDirectSound8_GetCaps                               IDirectSound_GetCaps
#define IDirectSound8_CreateSoundBuffer                     IDirectSound_CreateSoundBuffer
#define IDirectSound8_GetSpeakerConfig                      IDirectSound_GetSpeakerConfig
#define IDirectSound8_Compact                               IDirectSound_Compact
#define IDirectSound8_SetCooperativeLevel                   IDirectSound_SetCooperativeLevel

#define IDirectSound3DListener_QueryInterface               IDirectSound_QueryInterface
#define IDirectSound3DListener_AddRef                       IDirectSound_AddRef
#define IDirectSound3DListener_Release                      IDirectSound_Release
#define IDirectSound3DListener_SetAllParameters             IDirectSound_SetAllParameters
#define IDirectSound3DListener_SetDistanceFactor            IDirectSound_SetDistanceFactor
#define IDirectSound3DListener_SetDopplerFactor             IDirectSound_SetDopplerFactor
#define IDirectSound3DListener_SetOrientation               IDirectSound_SetOrientation
#define IDirectSound3DListener_SetPosition                  IDirectSound_SetPosition
#define IDirectSound3DListener_SetRolloffFactor             IDirectSound_SetRolloffFactor
#define IDirectSound3DListener_SetVelocity                  IDirectSound_SetVelocity
#define IDirectSound3DListener_CommitDeferredSettings       IDirectSound_CommitDeferredSettings

#define IReferenceClock_QueryInterface                      IDirectSound_QueryInterface
#define IReferenceClock_AddRef                              IDirectSound_AddRef
#define IReferenceClock_Release                             IDirectSound_Release
#define IReferenceClock_GetTime                             IDirectSound_GetTime

//
// IDirectSoundBuffer
//

#if defined(__cplusplus) && !defined(CINTERFACE)

STDAPI IDirectSoundBuffer_QueryInterface(LPDIRECTSOUNDBUFFER pBuffer, REFIID iid, LPVOID *ppvInterface);

#else // defined(__cplusplus) && !defined(CINTERFACE)

STDAPI IDirectSoundBuffer_QueryInterfaceC(LPDIRECTSOUNDBUFFER pBuffer, const IID *iid, LPVOID *ppvInterface);
#define IDirectSoundBuffer_QueryInterface IDirectSoundBuffer_QueryInterfaceC

#endif // defined(__cplusplus) && !defined(CINTERFACE)

STDAPI_(ULONG) IDirectSoundBuffer_AddRef(LPDIRECTSOUNDBUFFER pBuffer);
STDAPI_(ULONG) IDirectSoundBuffer_Release(LPDIRECTSOUNDBUFFER pBuffer);
STDAPI IDirectSoundBuffer_SetFormat(LPDIRECTSOUNDBUFFER pBuffer, LPCWAVEFORMATEX pwfxFormat);
STDAPI IDirectSoundBuffer_SetFrequency(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwFrequency);
STDAPI IDirectSoundBuffer_SetVolume(LPDIRECTSOUNDBUFFER pBuffer, LONG lVolume);
STDAPI IDirectSoundBuffer_SetPitch(LPDIRECTSOUNDBUFFER pBuffer, LONG lPitch);
STDAPI IDirectSoundBuffer_SetLFO(LPDIRECTSOUNDBUFFER pBuffer, LPCDSLFODESC pLFODesc);
STDAPI IDirectSoundBuffer_SetEG(LPDIRECTSOUNDBUFFER pBuffer, LPCDSENVELOPEDESC pEnvelopeDesc);
STDAPI IDirectSoundBuffer_SetFilter(LPDIRECTSOUNDBUFFER pBuffer, LPCDSFILTERDESC pFilterDesc);
STDAPI IDirectSoundBuffer_SetHeadroom(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwHeadroom);
STDAPI IDirectSoundBuffer_SetOutputBuffer(LPDIRECTSOUNDBUFFER pBuffer, LPDIRECTSOUNDBUFFER pOutputBuffer);
STDAPI IDirectSoundBuffer_SetMixBins(LPDIRECTSOUNDBUFFER pBuffer, LPCDSMIXBINS pMixBins);
STDAPI IDirectSoundBuffer_SetMixBinVolumes(LPDIRECTSOUNDBUFFER pBuffer, LPCDSMIXBINS pMixBins);
STDAPI IDirectSoundBuffer_SetAllParameters(LPDIRECTSOUNDBUFFER pBuffer, LPCDS3DBUFFER pds3db, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetConeAngles(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetConeOrientation(LPDIRECTSOUNDBUFFER pBuffer, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetConeOutsideVolume(LPDIRECTSOUNDBUFFER pBuffer, LONG lConeOutsideVolume, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetMaxDistance(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flMaxDistance, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetMinDistance(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flMinDistance, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetMode(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwMode, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetPosition(LPDIRECTSOUNDBUFFER pBuffer, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetVelocity(LPDIRECTSOUNDBUFFER pBuffer, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetDistanceFactor(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flDistanceFactor, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetDopplerFactor(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flDopplerFactor, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetRolloffFactor(LPDIRECTSOUNDBUFFER pBuffer, FLOAT flRolloffFactor, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetRolloffCurve(LPDIRECTSOUNDBUFFER pBuffer, const FLOAT *pflPoints, DWORD dwPointCount, DWORD dwApply);
STDAPI IDirectSoundBuffer_SetI3DL2Source(LPDIRECTSOUNDBUFFER pBuffer, LPCDSI3DL2BUFFER pds3db, DWORD dwApply);
STDAPI IDirectSoundBuffer_Play(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags);
STDAPI IDirectSoundBuffer_PlayEx(LPDIRECTSOUNDBUFFER pBuffer, REFERENCE_TIME rtTimeStamp, DWORD dwFlags);
STDAPI IDirectSoundBuffer_Stop(LPDIRECTSOUNDBUFFER pBuffer);
STDAPI IDirectSoundBuffer_StopEx(LPDIRECTSOUNDBUFFER pBuffer, REFERENCE_TIME rtTimeStamp, DWORD dwFlags);
STDAPI IDirectSoundBuffer_Pause(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwPause);
STDAPI IDirectSoundBuffer_PauseEx(LPDIRECTSOUNDBUFFER pBuffer, REFERENCE_TIME rtTimestamp, DWORD dwPause);
STDAPI IDirectSoundBuffer_SetPlayRegion(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwPlayStart, DWORD dwPlayLength);
STDAPI IDirectSoundBuffer_SetLoopRegion(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwLoopStart, DWORD dwLoopLength);
STDAPI IDirectSoundBuffer_GetStatus(LPDIRECTSOUNDBUFFER pBuffer, LPDWORD pdwStatus);
STDAPI IDirectSoundBuffer_GetCurrentPosition(LPDIRECTSOUNDBUFFER pBuffer, LPDWORD pdwPlayCursor, LPDWORD pdwWriteCursor);
STDAPI IDirectSoundBuffer_SetCurrentPosition(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwPlayCursor);
STDAPI IDirectSoundBuffer_SetBufferData(LPDIRECTSOUNDBUFFER pBuffer, LPVOID pvBufferData, DWORD dwBufferBytes);
STDAPI IDirectSoundBuffer_Lock(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags);
STDAPI IDirectSoundBuffer_Unlock(LPDIRECTSOUNDBUFFER pBuffer, LPVOID pvLock1, DWORD dwLockSize1, LPVOID pvLock2, DWORD dwLockSize2);
STDAPI IDirectSoundBuffer_Restore(LPDIRECTSOUNDBUFFER pBuffer);
STDAPI IDirectSoundBuffer_SetNotificationPositions(LPDIRECTSOUNDBUFFER pBuffer, DWORD dwNotifyCount, LPCDSBPOSITIONNOTIFY paNotifies);
STDAPI IDirectSoundBuffer_GetVoiceProperties(LPDIRECTSOUNDBUFFER pBuffer, LPDSVOICEPROPS pVoiceProps);

#if defined(__cplusplus) && !defined(CINTERFACE)                

struct IDirectSoundBuffer
{
    __inline HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppvInterface)
    {
        return IDirectSoundBuffer_QueryInterface(this, iid, ppvInterface);
    }

    __inline ULONG STDMETHODCALLTYPE AddRef(void)
    {
        return IDirectSoundBuffer_AddRef(this);
    }

    __inline ULONG STDMETHODCALLTYPE Release(void)
    {
        return IDirectSoundBuffer_Release(this);
    }

    __inline HRESULT STDMETHODCALLTYPE SetFormat(LPCWAVEFORMATEX pwfxFormat)
    {
        return IDirectSoundBuffer_SetFormat(this, pwfxFormat);
    }

    __inline HRESULT STDMETHODCALLTYPE SetFrequency(DWORD dwFrequency)
    {
        return IDirectSoundBuffer_SetFrequency(this, dwFrequency);
    }

    __inline HRESULT STDMETHODCALLTYPE SetVolume(LONG lVolume)
    {
        return IDirectSoundBuffer_SetVolume(this, lVolume);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPitch(LONG lPitch)
    {
        return IDirectSoundBuffer_SetPitch(this, lPitch);
    }

    __inline HRESULT STDMETHODCALLTYPE SetLFO(LPCDSLFODESC pLFODesc)
    {
        return IDirectSoundBuffer_SetLFO(this, pLFODesc);
    }

    __inline HRESULT STDMETHODCALLTYPE SetEG(LPCDSENVELOPEDESC pEnvelopeDesc)
    {
        return IDirectSoundBuffer_SetEG(this, pEnvelopeDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE SetFilter(LPCDSFILTERDESC pFilterDesc)
    {
        return IDirectSoundBuffer_SetFilter(this, pFilterDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE SetHeadroom(DWORD dwHeadroom)
    {
        return IDirectSoundBuffer_SetHeadroom(this, dwHeadroom);
    }

    __inline HRESULT STDMETHODCALLTYPE SetOutputBuffer(LPDIRECTSOUNDBUFFER pOutputBuffer)
    {
        return IDirectSoundBuffer_SetOutputBuffer(this, pOutputBuffer);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMixBins(LPCDSMIXBINS pMixBins)
    {
        return IDirectSoundBuffer_SetMixBins(this, pMixBins);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMixBinVolumes(LPCDSMIXBINS pMixBins)
    {
        return IDirectSoundBuffer_SetMixBinVolumes(this, pMixBins);
    }

    __inline HRESULT STDMETHODCALLTYPE SetAllParameters(LPCDS3DBUFFER pds3db, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetAllParameters(this, pds3db, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetConeAngles(DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetConeAngles(this, dwInsideConeAngle, dwOutsideConeAngle, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetConeOrientation(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetConeOrientation(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetConeOutsideVolume(LONG lConeOutsideVolume, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetConeOutsideVolume(this, lConeOutsideVolume, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMaxDistance(FLOAT flMaxDistance, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetMaxDistance(this, flMaxDistance, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMinDistance(FLOAT flMinDistance, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetMinDistance(this, flMinDistance, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMode(DWORD dwMode, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetMode(this, dwMode, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPosition(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetPosition(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetVelocity(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetVelocity(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetDistanceFactor(FLOAT flDistanceFactor, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetDistanceFactor(this, flDistanceFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetDopplerFactor(FLOAT flDopplerFactor, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetDopplerFactor(this, flDopplerFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetRolloffFactor(FLOAT flRolloffFactor, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetRolloffFactor(this, flRolloffFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetRolloffCurve(const FLOAT *pflPoints, DWORD dwPointCount, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetRolloffCurve(this, pflPoints, dwPointCount, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetI3DL2Source(LPCDSI3DL2BUFFER pds3db, DWORD dwApply)
    {
        return IDirectSoundBuffer_SetI3DL2Source(this, pds3db, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE Play(DWORD dwReserved1, DWORD dwReserved2, DWORD dwFlags)
    {   
        return IDirectSoundBuffer_Play(this, dwReserved1, dwReserved2, dwFlags);
    }

    __inline HRESULT STDMETHODCALLTYPE PlayEx(REFERENCE_TIME rtTimeStamp, DWORD dwFlags)
    {
        return IDirectSoundBuffer_PlayEx(this, rtTimeStamp, dwFlags);
    }

    __inline HRESULT STDMETHODCALLTYPE Stop(void)
    {
        return IDirectSoundBuffer_Stop(this);
    }

    __inline HRESULT STDMETHODCALLTYPE StopEx(REFERENCE_TIME rtTimeStamp, DWORD dwFlags)
    {
        return IDirectSoundBuffer_StopEx(this, rtTimeStamp, dwFlags);
    }

    __inline HRESULT STDMETHODCALLTYPE Pause(DWORD dwPause)
    {
        return IDirectSoundBuffer_Pause(this, dwPause);
    }

    __inline HRESULT STDMETHODCALLTYPE PauseEx(REFERENCE_TIME rtTimestamp, DWORD dwPause)
    {
        return IDirectSoundBuffer_PauseEx(this, rtTimestamp, dwPause);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPlayRegion(DWORD dwPlayStart, DWORD dwPlayLength)
    {
        return IDirectSoundBuffer_SetPlayRegion(this, dwPlayStart, dwPlayLength);
    }

    __inline HRESULT STDMETHODCALLTYPE SetLoopRegion(DWORD dwLoopStart, DWORD dwLoopLength)
    {
        return IDirectSoundBuffer_SetLoopRegion(this, dwLoopStart, dwLoopLength);
    }

    __inline HRESULT STDMETHODCALLTYPE GetStatus(LPDWORD pdwStatus)
    {
        return IDirectSoundBuffer_GetStatus(this, pdwStatus);
    }

    __inline HRESULT STDMETHODCALLTYPE GetCurrentPosition(LPDWORD pdwPlayCursor, LPDWORD pdwWriteCursor)
    {
        return IDirectSoundBuffer_GetCurrentPosition(this, pdwPlayCursor, pdwWriteCursor);
    }

    __inline HRESULT STDMETHODCALLTYPE SetCurrentPosition(DWORD dwPlayCursor)
    {
        return IDirectSoundBuffer_SetCurrentPosition(this, dwPlayCursor);
    }

    __inline HRESULT STDMETHODCALLTYPE SetBufferData(LPVOID pvBufferData, DWORD dwBufferBytes)
    {
        return IDirectSoundBuffer_SetBufferData(this, pvBufferData, dwBufferBytes);
    }

    __inline HRESULT STDMETHODCALLTYPE Lock(DWORD dwOffset, DWORD dwBytes, LPVOID *ppvAudioPtr1, LPDWORD pdwAudioBytes1, LPVOID *ppvAudioPtr2, LPDWORD pdwAudioBytes2, DWORD dwFlags)
    {
        return IDirectSoundBuffer_Lock(this, dwOffset, dwBytes, ppvAudioPtr1, pdwAudioBytes1, ppvAudioPtr2, pdwAudioBytes2, dwFlags);
    }

    __inline HRESULT STDMETHODCALLTYPE Unlock(LPVOID pvLock1, DWORD dwLockSize1, LPVOID pvLock2, DWORD dwLockSize2)
    {
        return IDirectSoundBuffer_Unlock(this, pvLock1, dwLockSize1, pvLock2, dwLockSize2);
    }

    __inline HRESULT STDMETHODCALLTYPE Restore(void)
    {
        return IDirectSoundBuffer_Restore(this);
    }

    __inline HRESULT STDMETHODCALLTYPE SetNotificationPositions(DWORD dwNotifyCount, LPCDSBPOSITIONNOTIFY paNotifies)
    {
        return IDirectSoundBuffer_SetNotificationPositions(this, dwNotifyCount, paNotifies);
    }

    __inline HRESULT STDMETHODCALLTYPE GetVoiceProperties(LPDSVOICEPROPS pVoiceProps)
    {
        return IDirectSoundBuffer_GetVoiceProperties(this, pVoiceProps);
    }
};

#endif // defined(__cplusplus) && !defined(CINTERFACE)                

#define IDirectSoundBuffer8_QueryInterface          IDirectSoundBuffer_QueryInterface
#define IDirectSoundBuffer8_AddRef                  IDirectSoundBuffer_AddRef
#define IDirectSoundBuffer8_Release                 IDirectSoundBuffer_Release
#define IDirectSoundBuffer8_SetFormat               IDirectSoundBuffer_SetFormat
#define IDirectSoundBuffer8_Play                    IDirectSoundBuffer_Play              
#define IDirectSoundBuffer8_Stop                    IDirectSoundBuffer_Stop              
#define IDirectSoundBuffer8_GetStatus               IDirectSoundBuffer_GetStatus         
#define IDirectSoundBuffer8_GetCurrentPosition      IDirectSoundBuffer_GetCurrentPosition
#define IDirectSoundBuffer8_SetCurrentPosition      IDirectSoundBuffer_SetCurrentPosition
#define IDirectSoundBuffer8_Lock                    IDirectSoundBuffer_Lock              
#define IDirectSoundBuffer8_SetFrequency            IDirectSoundBuffer_SetFrequency
#define IDirectSoundBuffer8_SetVolume               IDirectSoundBuffer_SetVolume         
                                                    
#define IDirectSound3DBuffer_QueryInterface         IDirectSoundBuffer_QueryInterface
#define IDirectSound3DBuffer_AddRef                 IDirectSoundBuffer_AddRef
#define IDirectSound3DBuffer_Release                IDirectSoundBuffer_Release
#define IDirectSound3DBuffer_SetAllParameters       IDirectSoundBuffer_SetAllParameters  
#define IDirectSound3DBuffer_SetConeAngles          IDirectSoundBuffer_SetConeAngles     
#define IDirectSound3DBuffer_SetConeOrientation     IDirectSoundBuffer_SetConeOrientation
#define IDirectSound3DBuffer_SetConeOutsideVolume   IDirectSoundBuffer_SetConeOutsideVolume
#define IDirectSound3DBuffer_SetMaxDistance         IDirectSoundBuffer_SetMaxDistance    
#define IDirectSound3DBuffer_SetMinDistance         IDirectSoundBuffer_SetMinDistance    
#define IDirectSound3DBuffer_SetMode                IDirectSoundBuffer_SetMode           
#define IDirectSound3DBuffer_SetPosition            IDirectSoundBuffer_SetPosition       
#define IDirectSound3DBuffer_SetVelocity            IDirectSoundBuffer_SetVelocity       
                                                    
#define IDirectSoundNotify_QueryInterface           IDirectSoundBuffer_QueryInterface
#define IDirectSoundNotify_AddRef                   IDirectSoundBuffer_AddRef
#define IDirectSoundNotify_Release                  IDirectSoundBuffer_Release
#define IDirectSoundNotify_SetNotificationPositions IDirectSoundBuffer_SetNotificationPositions

//
// IDirectSoundStream
//

#if defined(__cplusplus) && !defined(CINTERFACE)

STDAPI IDirectSoundStream_QueryInterface(LPDIRECTSOUNDSTREAM pStream, REFIID iid, LPVOID *ppvInterface);

#else // defined(__cplusplus) && !defined(CINTERFACE)

STDAPI IDirectSoundStream_QueryInterfaceC(LPDIRECTSOUNDSTREAM pStream, const IID *iid, LPVOID *ppvInterface);
#define IDirectSoundStream_QueryInterface IDirectSoundStream_QueryInterfaceC

#endif // defined(__cplusplus) && !defined(CINTERFACE)

STDAPI IDirectSoundStream_SetFormat(LPDIRECTSOUNDSTREAM pStream, LPCWAVEFORMATEX pwfxFormat);
STDAPI IDirectSoundStream_SetFrequency(LPDIRECTSOUNDSTREAM pStream, DWORD dwFrequency);
STDAPI IDirectSoundStream_SetVolume(LPDIRECTSOUNDSTREAM pStream, LONG lVolume);
STDAPI IDirectSoundStream_SetPitch(LPDIRECTSOUNDSTREAM pStream, LONG lPitch);
STDAPI IDirectSoundStream_SetLFO(LPDIRECTSOUNDSTREAM pStream, LPCDSLFODESC pLFODesc);
STDAPI IDirectSoundStream_SetEG(LPDIRECTSOUNDSTREAM pStream, LPCDSENVELOPEDESC pEnvelopeDesc);
STDAPI IDirectSoundStream_SetFilter(LPDIRECTSOUNDSTREAM pStream, LPCDSFILTERDESC pFilterDesc);
STDAPI IDirectSoundStream_SetHeadroom(LPDIRECTSOUNDSTREAM pStream, DWORD dwHeadroom);
STDAPI IDirectSoundStream_SetOutputBuffer(LPDIRECTSOUNDSTREAM pStream, LPDIRECTSOUNDBUFFER pOutputBuffer);
STDAPI IDirectSoundStream_SetMixBins(LPDIRECTSOUNDSTREAM pStream, LPCDSMIXBINS pMixBins);
STDAPI IDirectSoundStream_SetMixBinVolumes(LPDIRECTSOUNDSTREAM pStream, LPCDSMIXBINS pMixBins);
STDAPI IDirectSoundStream_SetAllParameters(LPDIRECTSOUNDSTREAM pStream, LPCDS3DBUFFER pds3db, DWORD dwApply);
STDAPI IDirectSoundStream_SetConeAngles(LPDIRECTSOUNDSTREAM pStream, DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply);
STDAPI IDirectSoundStream_SetConeOrientation(LPDIRECTSOUNDSTREAM pStream, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IDirectSoundStream_SetConeOutsideVolume(LPDIRECTSOUNDSTREAM pStream, LONG lConeOutsideVolume, DWORD dwApply);
STDAPI IDirectSoundStream_SetMaxDistance(LPDIRECTSOUNDSTREAM pStream, FLOAT flMaxDistance, DWORD dwApply);
STDAPI IDirectSoundStream_SetMinDistance(LPDIRECTSOUNDSTREAM pStream, FLOAT flMinDistance, DWORD dwApply);
STDAPI IDirectSoundStream_SetMode(LPDIRECTSOUNDSTREAM pStream, DWORD dwMode, DWORD dwApply);
STDAPI IDirectSoundStream_SetPosition(LPDIRECTSOUNDSTREAM pStream, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IDirectSoundStream_SetVelocity(LPDIRECTSOUNDSTREAM pStream, FLOAT x, FLOAT y, FLOAT z, DWORD dwApply);
STDAPI IDirectSoundStream_SetDistanceFactor(LPDIRECTSOUNDSTREAM pStream, FLOAT flDistanceFactor, DWORD dwApply);
STDAPI IDirectSoundStream_SetDopplerFactor(LPDIRECTSOUNDSTREAM pStream, FLOAT flDopplerFactor, DWORD dwApply);
STDAPI IDirectSoundStream_SetRolloffFactor(LPDIRECTSOUNDSTREAM pStream, FLOAT flRolloffFactor, DWORD dwApply);
STDAPI IDirectSoundStream_SetRolloffCurve(LPDIRECTSOUNDSTREAM pStream, const FLOAT *pflPoints, DWORD dwPointCount, DWORD dwApply);
STDAPI IDirectSoundStream_SetI3DL2Source(LPDIRECTSOUNDSTREAM pStream, LPCDSI3DL2BUFFER pds3db, DWORD dwApply);
STDAPI IDirectSoundStream_Pause(LPDIRECTSOUNDSTREAM pStream, DWORD dwPause);
STDAPI IDirectSoundStream_PauseEx(LPDIRECTSOUNDSTREAM pStream, REFERENCE_TIME rtTimestamp, DWORD dwPause);
STDAPI IDirectSoundStream_FlushEx(LPDIRECTSOUNDSTREAM pStream, REFERENCE_TIME rtTimeStamp, DWORD dwFlags);
STDAPI IDirectSoundStream_GetVoiceProperties(LPDIRECTSOUNDSTREAM pStream, LPDSVOICEPROPS pVoiceProps);

#define IDirectSoundStream_AddRef           IUnknown_AddRef
#define IDirectSoundStream_Release          IUnknown_Release

#define IDirectSoundStream_GetInfo          XMediaObject_GetInfo
#define IDirectSoundStream_GetStatus        XMediaObject_GetStatus
#define IDirectSoundStream_Process          XMediaObject_Process
#define IDirectSoundStream_Discontinuity    XMediaObject_Discontinuity
#define IDirectSoundStream_Flush            XMediaObject_Flush

#undef INTERFACE
#define INTERFACE IDirectSoundStream

DECLARE_INTERFACE_(IDirectSoundStream, XMediaObject)
{
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // XMediaObject methods
    STDMETHOD(GetInfo)(THIS_ LPXMEDIAINFO pInfo) PURE;
    STDMETHOD(GetStatus)(THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Process)(THIS_ LPCXMEDIAPACKET pInputPacket, LPCXMEDIAPACKET pOutputPacket) PURE;
    STDMETHOD(Discontinuity)(THIS) PURE;
    STDMETHOD(Flush)(THIS) PURE;

#if defined(__cplusplus) && !defined(CINTERFACE)

    __inline HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, LPVOID *ppvInterface)
    {
        return IDirectSoundStream_QueryInterface(this, iid, ppvInterface);
    }

    __inline HRESULT STDMETHODCALLTYPE SetFormat(LPCWAVEFORMATEX pwfxFormat)
    {
        return IDirectSoundStream_SetFormat(this, pwfxFormat);
    }

    __inline HRESULT STDMETHODCALLTYPE SetFrequency(DWORD dwFrequency)
    {
        return IDirectSoundStream_SetFrequency(this, dwFrequency);
    }

    __inline HRESULT STDMETHODCALLTYPE SetVolume(LONG lVolume)
    {
        return IDirectSoundStream_SetVolume(this, lVolume);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPitch(LONG lPitch)
    {
        return IDirectSoundStream_SetPitch(this, lPitch);
    }

    __inline HRESULT STDMETHODCALLTYPE SetLFO(LPCDSLFODESC pLFODesc)
    {
        return IDirectSoundStream_SetLFO(this, pLFODesc);
    }

    __inline HRESULT STDMETHODCALLTYPE SetEG(LPCDSENVELOPEDESC pEnvelopeDesc)
    {
        return IDirectSoundStream_SetEG(this, pEnvelopeDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE SetFilter(LPCDSFILTERDESC pFilterDesc)
    {
        return IDirectSoundStream_SetFilter(this, pFilterDesc);
    }

    __inline HRESULT STDMETHODCALLTYPE SetHeadroom(DWORD dwHeadroom)
    {
        return IDirectSoundStream_SetHeadroom(this, dwHeadroom);
    }

    __inline HRESULT STDMETHODCALLTYPE SetOutputBuffer(LPDIRECTSOUNDBUFFER pOutputBuffer)
    {
        return IDirectSoundStream_SetOutputBuffer(this, pOutputBuffer);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMixBins(LPCDSMIXBINS pMixBins)
    {
        return IDirectSoundStream_SetMixBins(this, pMixBins);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMixBinVolumes(LPCDSMIXBINS pMixBins)
    {
        return IDirectSoundStream_SetMixBinVolumes(this, pMixBins);
    }

    __inline HRESULT STDMETHODCALLTYPE SetAllParameters(LPCDS3DBUFFER pds3db, DWORD dwApply)
    {
        return IDirectSoundStream_SetAllParameters(this, pds3db, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetConeAngles(DWORD dwInsideConeAngle, DWORD dwOutsideConeAngle, DWORD dwApply)
    {
        return IDirectSoundStream_SetConeAngles(this, dwInsideConeAngle, dwOutsideConeAngle, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetConeOrientation(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IDirectSoundStream_SetConeOrientation(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetConeOutsideVolume(LONG lConeOutsideVolume, DWORD dwApply)
    {
        return IDirectSoundStream_SetConeOutsideVolume(this, lConeOutsideVolume, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMaxDistance(FLOAT flMaxDistance, DWORD dwApply)
    {
        return IDirectSoundStream_SetMaxDistance(this, flMaxDistance, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMinDistance(FLOAT flMinDistance, DWORD dwApply)
    {
        return IDirectSoundStream_SetMinDistance(this, flMinDistance, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetMode(DWORD dwMode, DWORD dwApply)
    {
        return IDirectSoundStream_SetMode(this, dwMode, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetPosition(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IDirectSoundStream_SetPosition(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetVelocity(FLOAT x, FLOAT y, FLOAT z, DWORD dwApply)
    {
        return IDirectSoundStream_SetVelocity(this, x, y, z, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetDistanceFactor(FLOAT flDistanceFactor, DWORD dwApply)
    {
        return IDirectSoundStream_SetDistanceFactor(this, flDistanceFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetDopplerFactor(FLOAT flDopplerFactor, DWORD dwApply)
    {
        return IDirectSoundStream_SetDopplerFactor(this, flDopplerFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetRolloffFactor(FLOAT flRolloffFactor, DWORD dwApply)
    {
        return IDirectSoundStream_SetRolloffFactor(this, flRolloffFactor, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetRolloffCurve(const FLOAT *pflPoints, DWORD dwPointCount, DWORD dwApply)
    {
        return IDirectSoundStream_SetRolloffCurve(this, pflPoints, dwPointCount, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE SetI3DL2Source(LPCDSI3DL2BUFFER pds3db, DWORD dwApply)
    {
        return IDirectSoundStream_SetI3DL2Source(this, pds3db, dwApply);
    }

    __inline HRESULT STDMETHODCALLTYPE Pause(DWORD dwPause)
    {
        return IDirectSoundStream_Pause(this, dwPause);
    }

    __inline HRESULT STDMETHODCALLTYPE PauseEx(REFERENCE_TIME rtTimestamp, DWORD dwPause)
    {
        return IDirectSoundStream_PauseEx(this, rtTimestamp, dwPause);
    }

    __inline HRESULT STDMETHODCALLTYPE FlushEx(REFERENCE_TIME rtTimeStamp, DWORD dwFlags)
    {
        return IDirectSoundStream_FlushEx(this, rtTimeStamp, dwFlags);
    }

    __inline HRESULT STDMETHODCALLTYPE GetVoiceProperties(LPDSVOICEPROPS pVoiceProps)
    {
        return IDirectSoundStream_GetVoiceProperties(this, pVoiceProps);
    }
    
#endif // defined(__cplusplus) && !defined(CINTERFACE)

};

//
// XAc97MediaObject
//

#undef INTERFACE
#define INTERFACE XAc97MediaObject

DECLARE_INTERFACE_(XAc97MediaObject, XMediaObject)
{
    // IUnknown methods
    STDMETHOD_(ULONG, AddRef)(THIS) PURE;
    STDMETHOD_(ULONG, Release)(THIS) PURE;

    // XMediaObject methods
    STDMETHOD(GetInfo)(THIS_ LPXMEDIAINFO pInfo) PURE;
    STDMETHOD(GetStatus)(THIS_ LPDWORD pdwStatus) PURE;
    STDMETHOD(Process)(THIS_ LPCXMEDIAPACKET pInputPacket, LPCXMEDIAPACKET pOutputPacket) PURE;
    STDMETHOD(Discontinuity)(THIS) PURE;
    STDMETHOD(Flush)(THIS) PURE;

    // XAc97MediaObject methods
    STDMETHOD(SetMode)(THIS_ DWORD dwMode) PURE;
    STDMETHOD(GetCurrentPosition)(THIS_ LPDWORD pdwMode) PURE;
};

#define XAc97MediaObject_AddRef                     IUnknown_AddRef
#define XAc97MediaObject_Release                    IUnknown_Release
                                                    
#define XAc97MediaObject_GetInfo                    XMediaObject_GetInfo
#define XAc97MediaObject_GetStatus                  XMediaObject_GetStatus
#define XAc97MediaObject_Process                    XMediaObject_Process
#define XAc97MediaObject_Discontinuity              XMediaObject_Discontinuity
#define XAc97MediaObject_Flush                      XMediaObject_Flush

#if defined(__cplusplus) && !defined(CINTERFACE)

#define XAc97MediaObject_SetMode(p, a)              p->SetMode(a)
#define XAc97MediaObject_GetCurrentPosition(p, a)   p->GetCurrentPosition(a)

#else // defined(__cplusplus) && !defined(CINTERFACE)

#define XAc97MediaObject_SetMode(p, a)              p->lpVtbl->SetMode(p, a)
#define XAc97MediaObject_GetCurrentPosition(p, a)   p->lpVtbl->GetCurrentPosition(p, a)

#endif // defined(__cplusplus) && !defined(CINTERFACE)

//
// Multimedia timer support
//

#define MMSYSERR_BASE               0
#define MMSYSERR_NOERROR            0                   // No error

#define TIMERR_BASE                 96
#define TIMERR_NOERROR              (0)                 // No error
#define TIMERR_NOCANDO              (TIMERR_BASE+1)     // Request not completed
#define TIMERR_STRUCT               (TIMERR_BASE+33)    // Time struct size

#define TIME_MS                     0x0001              // Time in milliseconds
#define TIME_SAMPLES                0x0002              // Number of wave samples
#define TIME_BYTES                  0x0004              // Current byte offset
#define TIME_SMPTE                  0x0008              // SMPTE time
#define TIME_MIDI                   0x0010              // MIDI time
#define TIME_TICKS                  0x0020              // Ticks within MIDI stream

#define TIME_ONESHOT                0x0000              // Program timer for single event
#define TIME_PERIODIC               0x0001              // Program for continuous periodic event

#define TIME_CALLBACK_FUNCTION      0x0000              // Callback is function
#define TIME_CALLBACK_EVENT_SET     0x0010              // Callback is event - use SetEvent
#define TIME_CALLBACK_EVENT_PULSE   0x0020              // Callback is event - use PulseEvent

typedef void (CALLBACK TIMECALLBACK)(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);

typedef TIMECALLBACK *LPTIMECALLBACK;

#ifdef _XBOX

typedef struct mmtime_tag
{
    UINT            wType;                  // Indicates the contents of the union
    union
    {
        DWORD       ms;                     // Milliseconds
        DWORD       sample;                 // Samples
        DWORD       cb;                     // Byte count
        DWORD       ticks;                  // Ticks in MIDI stream

        struct
        {
            BYTE    hour;                   // Hours
            BYTE    min;                    // Minutes
            BYTE    sec;                    // Seconds
            BYTE    frame;                  // Frames
            BYTE    fps;                    // Frames per second
            BYTE    dummy;                  // Pad
            BYTE    pad[2];
        } smpte;

        struct
        {
            DWORD songptrpos;               // Song pointer position
        } midi;
    } u;
} MMTIME, *PMMTIME, *LPMMTIME;

typedef const MMTIME *LPCMMTIME;

typedef UINT MMRESULT;

EXTERN_C MMRESULT WINAPI timeGetSystemTime(LPMMTIME pmmt, UINT cbmmt);
EXTERN_C MMRESULT WINAPI timeSetEvent(UINT uDelay, UINT uResolution, LPTIMECALLBACK fptc, DWORD dwUser, UINT fuEvent);
EXTERN_C MMRESULT WINAPI timeKillEvent(UINT uTimerID);

#define timeGetTime GetTickCount

#endif //_XBOX

#pragma warning(default:4201)

#endif // __DSOUND_INCLUDED__
