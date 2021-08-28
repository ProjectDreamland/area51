/*
  © Logitech 2001-2003.  All rights reserved.

  liblgaud.h

  library definition for liblgaud.a
  part of liblgaud for PlayStation(tm)2

  The Logitech USB Audio SDK, including all acompanying documentation, is
  protected by intellectual property laws.  All use of the Logitech 
  USB Audio SDK is subject to the License Agreement found in the
  "ReadMe License Agreement" file and in the Reference Manual.  All rights
  not expressly granted by Logitech are reserved.

  06/15/2001    1.01    nkutty       initial draft
  07/12/2001    1.02    nkutty       changed u_long to int (for IOP consistency)
  08/01/2001    1.03    nkutty       added lgAudEnumHint and lgAudAEnumHint()
  09/06/2001    1.04    rbosa        added lgAudPausePlayback() and
                                     lgAudResumePlayback(); renamed the
                                     SetVolume functions for playback and
                                     recording to provide consistent naming
  09/26/2001    1.05    tburgel      added flag on lgAudRead(), lgAudWrite()
                                     to allow for blocking/non-blocking calls
  11/02/2001    1.06    tburgel      added new functions to check the buffering
                                     status on recording/playback internal buffers:
                                     lgAudGetRemainingPlaybackBytes(),
                                     lgAudGetAvailableRecordingBytes(),
                                     extended enum with value such that even with
                                     "short enums", we get a guaranteed 32 bit
                                     value (for CodeWarrior/GCC interoperability)
  01/23/2003    1.07    tburgel      added lgAud[A]WriteVag(), renamed #defines
                                     to avoid name clashes with other SDKs
  05/21/2003    1.08    nkutty       fixed IOP memory leak on unplug
                                     fixed crash with async calls
  07/15/2003    1.09    tburgel      removed lgAudAEnumHint(), LGAUD_HINT_IKNOWNUTTIN
                                     added return code to lgAudASync() to retrieve
                                     the status of async operations after they complete
*/

#ifndef _LIBLGAUD_H_INCLUDED_
#define _LIBLGAUD_H_INCLUDED_

#include <sys/types.h>

#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
extern "C" {
#endif

//************************************************************************
// Terminal Types Ids
//************************************************************************

typedef enum lgAudPinID
{
    LGAUD_PIN_NONE = 0,
    LGAUD_PIN_MASTER,
    LGAUD_PIN_MIC,
    LGAUD_PIN_LINE,
    LGAUD_PIN_DIGITAL,
    LGAUD_PIN_WAVE,
    LGAUD_PIN_SPEAKER,
    LGAUD_PIN_HEADPHONES,
    LGAUD_PIN_SUBWOOFER,
    LGAUD_PIN_EMBEDDED,
    LGAUD_PIN_HACK_TO_EXTEND_ENUM_TO_32_BIT = 0x7fffffff
} lgAudPinID;


//************************************************************************
// Audio Controls
//************************************************************************

#define LGAUD_CTRL_MUTE                 (0x0001)
#define LGAUD_CTRL_VOLUME               (0x0002)
#define LGAUD_CTRL_BASS                 (0x0004)
#define LGAUD_CTRL_MID                  (0x0008)
#define LGAUD_CTRL_TREBLE               (0x0010)
#define LGAUD_CTRL_GRAPHIC_EQUALIZER    (0x0020)
#define LGAUD_CTRL_AUTOMATIC_GAIN       (0x0040)
#define LGAUD_CTRL_DELAY                (0x0080)
#define LGAUD_CTRL_BASS_BOOST           (0x0100)
#define LGAUD_CTRL_LOUDNESS             (0x0200)

#define LGAUD_CH_BOTH                   0
#define LGAUD_CH_LEFT                   1
#define LGAUD_CH_RIGHT                  2


//************************************************************************
// Mixing
//************************************************************************

typedef struct lgAudMixerDesc
{
    lgAudPinID Pid;     // which input/output this relates to
    u_short Controls;   // Combination of LGAUD_CTRLS values
} lgAudMixerDesc;

typedef struct lgAudMixerRequest
{
    u_char Mode;      // LGMODE_PLAYBACK for playback or LGMODE_RECORDING for recording
    lgAudPinID Pid;   // which input/output this relates to
    u_short Control;  // One of the LGAUD_CTRLS values
    u_char Where;     // LGCH_BOTH, LGCH_LEFT, or LGCH_RIGHT
    u_short Value;    // 0..100%
} lgAudMixerRequest;


//************************************************************************
// Sample Formats
//************************************************************************

typedef struct lgAudSamplingFormat
{
    u_char  Channels;
    u_char  BitResolution;
    u_short LowerSamplingRate;
    u_short HigherSamplingRate;
} lgAudSamplingFormat;

typedef struct lgAudSamplingRequest
{
    u_char  Channels;
    u_char  BitResolution;
    u_short SamplingRate;
    u_short BufferMilliseconds;
} lgAudSamplingRequest;


//************************************************************************
// Opening Audio Devices
//************************************************************************

#define LGAUD_MODE_PLAYBACK             (1)
#define LGAUD_MODE_RECORDING            (2)
#define LGAUD_MODE_PLAYRECORD           (LGAUD_MODE_PLAYBACK | LGAUD_MODE_RECORDING)

typedef struct lgAudOpenParam
{
    u_char Mode;
    lgAudSamplingRequest RecordingFormat;
    lgAudSamplingRequest PlaybackFormat;
} lgAudOpenParam;


//************************************************************************
// Device Description
//************************************************************************

#define LGAUD_MAX_FORMATS                 (16)
#define LGAUD_MAX_MIXERS                  (8)

typedef struct lgAudDeviceDesc
{
    u_char RecordingFormatsCount;
    lgAudSamplingFormat RecordingFormats[LGAUD_MAX_FORMATS];
    u_char PlaybackFormatsCount;
    lgAudSamplingFormat PlaybackFormats[LGAUD_MAX_FORMATS];
    u_char RecordingMixersCount;
    lgAudMixerDesc RecordingMixers[LGAUD_MAX_MIXERS];
    u_char PlaybackMixersCount;
    lgAudMixerDesc PlaybackMixers[LGAUD_MAX_MIXERS];
} lgAudDeviceDesc;


//************************************************************************
// Miscellaneous
//************************************************************************

#define LGAUD_IRXNAME                "lgaud.irx"
#define LGAUD_IN_MEM_MODULENAME      "lgAud"

#define LGAUD_INVALID_DEVICE           (-1)

// the maximum size of audio data that can be sent to IOP from EE
// (this is customizable by load-time options with lgaud.irx)
#define LGAUD_DEFAULT_MAX_BUFFER_BYTES     (1<<16)

// lgAudRead/lgAudARead/lgAudWrite/lgAudAWrite blocking behavior
#define LGAUD_BLOCKMODE_BLOCKING                (0)
#define LGAUD_BLOCKMODE_NOT_BLOCKING            (1)

// lgAudEnumHint hints
#define LGAUD_HINT_NOENUMNEEDED                 (0)
#define LGAUD_HINT_ENUMNEEDED                   (1)
// the following is OBSOLETE, the library never returns this value
// it is left solely for convenience of porting
#define LGAUD_HINT_IKNOWNUTTIN                  (2)


//************************************************************************
// Prototypes
//************************************************************************

#if defined(__R5900__) || defined(__ee__)
// For EE only:
// lgAudInit needs to allocate memory, depending on the size of streaming
// buffers chosen for the IRX. If no custom allocaters are passed in,
// lgAudInit will call the standard malloc() and free(); if you want
// to have your own memory allocators, pass them to lgAudInit(),
// otherwise pass NULL.

typedef void * (*lgAud_Malloc)(size_t size);
typedef void   (*lgAud_Free)(void * ptr);

// initializes the EE side library, connects RPC etc...
int lgAudInit(lgAud_Malloc malloc_cb, lgAud_Free free_cb);
// releases all resources that were acquired during lgAudInit,
// takes down RPC (after this, no more communication with lgaud.irx
// is possible, unless you call lgAudInit() again!)
int lgAudDeInit(void);
#endif

int lgAudEnumHint(int* hint);
int lgAudEnumerate(int index, lgAudDeviceDesc* description);
int lgAudOpen(int index, lgAudOpenParam* openparam, int* device);
int lgAudClose(int device);
int lgAudRead(int device, int blockmode, u_char* dest, int* size);
int lgAudWrite(int device, int blockmode, u_char* src, int* size);
int lgAudWriteVag(int device, int blockmode, u_char* src, int* size);
int lgAudGetRemainingPlaybackBytes(int device, int* size);
int lgAudGetAvailableRecordingBytes(int device, int* size);
int lgAudGetMixer(int device, lgAudMixerRequest* mixrequest);
int lgAudSetMixer(int device, lgAudMixerRequest* mixrequest);
int lgAudStartRecording(int device);
int lgAudStopRecording(int device);
int lgAudStartPlayback(int device);
int lgAudStopPlayback(int device);
int lgAudResumePlayback(int device);
int lgAudPrepareForReboot(void);

// pausing playback is the same as stopping... just make sure
// you call lgAudResumePlayback() afterwards instead of
// lgAudStartPlayback()...
#define lgAudPausePlayback              lgAudStopPlayback    

int lgAudSetPlaybackVolume(int device, u_char channel, u_char value);
int lgAudSetRecordingVolume(int device, u_char channel, u_char value);

#if defined(__R5900__) || defined(__ee__)
// asynchronous calls (available on EE only)
#define LGAUD_ASYNC_MODE_WAIT           (0)
#define LGAUD_ASYNC_MODE_NOWAIT         (1)

int lgAudASync(int mode, int *returnvalue);
int lgAudARead(int device, int blockmode, u_char* dest, int* size);
int lgAudAWrite(int device, int blockmode, u_char* src, int* size);
int lgAudAWriteVag(int device, int blockmode, u_char* src, int* size);

// lgAudAEnumHint is now obsolete. For compatibility reasons,
// it will perform the same function as the regular enum hint
#define lgAudAEnumHint(hint) lgAudEnumHint(hint)
#endif


//************************************************************************
// Error codes
//************************************************************************

#define LGAUD_SUCCESS                          (0x00000000)
#define LGAUD_ERROR                            (0x80000000)
#define LGAUD_ERR_NO_MORE_DEVICES               (0x80000002)
#define LGAUD_ERR_INVALID_DEVICETYPE            (0x80000003)
#define LGAUD_ERR_INVALID_PARAMETER             (0x80000004)
#define LGAUD_ERR_ALREADY_OPENED                (0x80000005)
#define LGAUD_ERR_DEVICE_LOST                   (0x80000006)
#define LGAUD_ERR_OUT_OF_MEMORY                 (0x80000007)
#define LGAUD_ERR_ASYNC_CALL_PENDING            (0x80000008)
#define LGAUD_ERR_NO_SPACE_LEFT                 (0x80000009)
#define LGAUD_ERR_NEED_INIT                     (0x8000000a)

// quick test against error condition
#define LGAUD_SUCCEEDED(x)                      (0 == ((x) & LGAUD_ERROR))
#define LGAUD_FAILED(x)                         (0 != ((x) & LGAUD_ERROR))


#if defined(_LANGUAGE_C_PLUS_PLUS)||defined(__cplusplus)||defined(c_plusplus)
}
#endif

#endif // _LIBLGAUD_H_INCLUDED_
