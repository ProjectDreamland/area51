#ifndef AUDIO_PRIVATE_HPP
#define AUDIO_PRIVATE_HPP

#include "x_files.hpp"
#include "audio_ram_mgr.hpp"
#include "..\IOManager\io_filesystem.hpp"

#ifdef TARGET_GCN
#include <dolphin.h>
#endif

#ifdef TARGET_PC
//#include "..\3rdparty\miles6\include\mss.h"
#include "IAL\IAL.h"
#endif

#ifdef TARGET_XBOX
#   include "xbox/xbox_private.hpp"
#endif

#include "audio_private_pkg.hpp"

//------------------------------------------------------------------------------
// Defines.

//------------------------------------------------------------------------------
// Platform specific constants (for use at runtime on target platform).

#ifdef TARGET_GCN
#define PACKAGE_VERSION                 GCN_PACKAGE_VERSION
#define TARGET_ID                       GCN_TARGET_ID
#define ADPCM_BUFFER_SIZE               GCN_ADPCM_CHANNEL_BUFFER_SIZE
#define STREAM_BUFFER_SIZE              (32 * 1024)
#endif

#ifdef TARGET_PC
#define PACKAGE_VERSION                 PC_PACKAGE_VERSION
#define TARGET_ID                       PC_TARGET_ID
#define STREAM_BUFFER_SIZE              (32 * 1024)
#endif

#ifdef TARGET_PS2
#define PACKAGE_VERSION                 PS2_PACKAGE_VERSION
#define TARGET_ID                       PS2_TARGET_ID
#define ADPCM_BUFFER_SIZE               PS2_ADPCM_CHANNEL_BUFFER_SIZE
#define STREAM_BUFFER_SIZE              (32 * 1024)
#endif

#ifdef TARGET_XBOX
#define PACKAGE_VERSION                 XBOX_PACKAGE_VERSION
#define TARGET_ID                       XBOX_TARGET_ID
#define ADPCM_BUFFER_SIZE               XBOX_ADPCM_CHANNEL_BUFFER_SIZE
#define STREAM_BUFFER_SIZE              (32 * 1024)
#endif

#define COOLING_STREAM                          ((audio_stream*)-1)
#define NO_STREAM                               ((audio_stream*)0)

// Limit the recursion depth on descriptors.
#define MAX_DESCRIPTOR_RECURSION_DEPTH          (10)

// Error checking macros
#define VALID_CHANNEL( pChannel )               g_AudioHardware.IsValidChannel( pChannel )
#define VALID_ELEMENT( pElement )               g_AudioVoiceMgr.IsValidElement( pElement )
#define VALID_VOICE( pVoice )                   g_AudioVoiceMgr.IsValidVoice( pVoice )

//------------------------------------------------------------------------------
// Enums.

enum stream_type 
{
    INACTIVE      = 0,
    MONO_STREAM   = 1,
    STEREO_STREAM = 2,
};

enum stream_channels
{
    LEFT_CHANNEL = 0,
    RIGHT_CHANNEL,
    MAX_STREAM_CHANNELS,
};

enum element_state
{
    ELEMENT_READY = 0,          // Ready to play!
    ELEMENT_PLAYING,            // Playing!
    ELEMENT_PAUSED,             // Paused.
    ELEMENT_NEEDS_TO_LOAD,      // Element needs to be cued.
    ELEMENT_LOADING,            // Element is currently being loaded.
    ELEMENT_LOADED,             // Element has been loaded.
    ELEMENT_DONE,               // Done!
};

enum channel_state
{
    STATE_NOT_STARTED = 0,
    STATE_STARTING,
    STATE_RUNNING,
    STATE_PAUSING,
    STATE_PAUSED,
    STATE_RESUMING,
    STATE_STOPPED,
};

enum callback_dirty_bits
{
    CALLBACK_DB_PRIORITY = (1<<0),
};

enum channel_dirty_bits
{
    CHANNEL_DB_VOLUME     = (1<<0),
    CHANNEL_DB_PAN        = (1<<1),
    CHANNEL_DB_PITCH      = (1<<2),
    CHANNEL_DB_EFFECTSEND = (1<<3),
};

//------------------------------------------------------------------------------
// Typedefs.

typedef s32 voice_id;           // Bottom 16 bits index, upper 16 bits sequence.
typedef s32 channel_id;         // Bottom 16 bits index, upper 16 bits sequence.
typedef s32 ear_id;             // Bottom 16 bits index, upper 16 bits sequence.

//------------------------------------------------------------------------------
// Structs.

struct voice;
struct element;
class  audio_package;
class  io_request;
struct sample_header;
struct warm_sample;
struct channel;
struct audio_stream;

typedef sample_header hot_sample;
typedef sample_header cold_sample;

//------------------------------------------------------------------------------

struct package_identifier
{
    char VersionID[VERSION_ID_SIZE];
    char TargetID[TARGET_ID_SIZE];
    char UserId[USER_ID_SIZE];
    char DateTime[DATE_TIME_SIZE];
};

struct descriptor_identifier
{
    u16            StringOffset;
    u16            Index;
    audio_package* pPackage;
};

struct element_link
{
    element* pPrev;                     // Pointer to previous element.
    element* pNext;                     // Pointer to next element.
};

struct voice_link
{
    voice* pPrev;                       // Pointer to previous voice.
    voice* pNext;                       // Pointer to next voice.
};

union sample_pointer
{
    hot_sample*     pHotSample;
    warm_sample*    pWarmSample;
    cold_sample*    pColdSample;
};

struct voice
{
    voice_link              Link;                   // Voice linked list.
    audio_package*          pPackage;               // Parent (pointer to the package).
    element_link            Elements;               // List of elements in this voice.
    element*                pPendingElement;        // Pointer to the next element to be fired off.
    void*                   pDescriptor;            // Pointer to voices descriptor.
    char*                   pDescriptorName;        //
    uncompressed_parameters Params;                 // Parameters set in the audio packager.
    voice*                  pSegueVoiceNext;
    voice*                  pSegueVoicePrev;
    f32                     ReleaseTime;  
    s32                     StartQ;     

    f32                     UserVolume;             // Application settable modifier.
    f32                     Volume;                 // The actual volume of the voice.
                                                    
    f32                     UserPitch;              // Application settable modifier.
    f32                     Pitch;                  // The actual pitch of the voice.
                                                    
    f32                     UserEffectSend;         // Application settable modifier.
    f32                     EffectSend;             // The actual effect send of the voice.
                            
    f32                     UserNearFalloff;        // Application settable modifier.
    f32                     NearFalloff;            // The actual near Falloff.
                            
    f32                     UserFarFalloff;         // Application settable modifier.
    f32                     FarFalloff;             // The actual near Falloff.

    f32                     UserNearDiffuse;        // Application settable modifier.
    f32                     NearDiffuse;            // Near diffusion clip.
    
    f32                     UserFarDiffuse;         // Application settable modifier.
    f32                     FarDiffuse;             // Far diffusion clip.
                            
    xbool                   IsPanChangeable;        // Flag: Can pan be changed?                                                  
    xbool                   IsPositional;           // Is this voice positional?
    xbool                   IsReleasing;            // Is this voice releasing?
    f32                     DeltaVolume;            // Releasing delta volume.
    vector3                 Position;               // Voices position
    u32                     Dirty;                  // Voices dirty bits
    u32                     Sequence;               // Bumped evertime the voice is used.                           
    f32                     StopTime;               // Time voice was stopped.
    f32                     StartTime;              // Audio tick voice was started.
    f32                     CursorTime;             // Voices time cursor.
    u32                     State;                  // Voices status. 
    u32                     RecursionDepth;         // Keep track of recursion depth.
    s32                     DegreesToSound;
    s32                     PrevDegreesToSound;
    s32                     EarID;
    s32                     ZoneID;
    xbool                   bPitchLock;
};                      

struct element
{
         element_link            Link;                   // Voice element list.        
         voice*                  pVoice;                 // Parent (pointer to elements virtual voice).
         element*                pStereoElement;         // Pointer to other stereo element.
         channel*                pChannel;               // Hardware channel(s).
         uncompressed_parameters Params;                 // Parameters set in the audio packager
         f32                     PositionalVolume;       // 3d volume (for positional voices only)
         f32                     Volume;                 // The actual volume of the element.
         f32                     VolumeVariance;         // The volume variance of the element
         f32                     Pitch;                  // The actual volume of the element.
         f32                     PitchVariance;          // The pitch variance of the element.
         f32                     EffectSend;             // The actual effect send of the voice.
         xbool                   IsPanChangeable;        // Flag: Can pan be changed?                            
         element_type            Type;                   // Type of element.
         sample_pointer          Sample;                 // Sample for this element.                            
         f32                     DeltaTime;              // Delta-time to start playback.
volatile element_state           State;                  // Status of this element.
         xbool                   bProcessed;
};                          

struct hardware_data
{
    s32                 Priority;   // Hardware priority.
#ifdef TARGET_GCN
    AXVPB*              pVPB;       // Voice pointer.
    u16                 Ve;         // Hardware state mirror.
#endif

#ifdef TARGET_PC
    ial_hchannel        hChannel;   // IAL channel handle
    s32                 CurrentPosition;
    s32                 BasePosition;
    xbool               InUse;
#endif

#ifdef TARGET_PS2
	s32					ChannelId;	// Channel ID
    u32                 RelativeBufferPosition;
#endif

#ifdef TARGET_XBOX
    LPDIRECTSOUNDBUFFER8 pdsBuffer;
    DSMIXBINVOLUMEPAIR  dsmbvp[6];            // Mix bin volumes
    DSMIXBINS           dsmb;                 // DSound MixBin struct
    s32                 CurrentPosition;
    s32                 BasePosition;
    xbool               IsStarted;
    xbool               InUse;
    xbool               IsLooped;
#endif
};

struct channel_link     
{                       
    channel*            pPrev;
    channel*            pNext;
};

struct stream_data
{
    audio_stream*       pStream;            // Pointer to the stream (if any).
    u32                 PreviousPosition;   // Previous position within the sample.
    xbool               StreamControl;      // Does this channel control a stream?
    xbool               bReadStream;        // Need to read from stream?
    xbool               bStopLoop;          // Need to stop the stream buffer from looping?
    u32                 WriteCursor;        // MP3 decompression write cursor.
    u32                 nSamplesPlayed;     // Number of samples that have been played (for mp3 playback)
};

struct channel
{
    channel_link        Link;               // List link.
    element_type        Type;               // HOT, WARM, COLD?    
    sample_pointer      Sample;             // Pointer to the sample.
    element*            pElement;           // Parent.
    u32                 StartPosition;      // A-RAM start position.
    u32                 EndPosition;        // A-RAM end position.
    u32                 MidPoint;           // Sample buffer mid-point.
    u32                 CurrBufferPosition; // Current play position within the sample buffer.
    u32                 PrevBufferPosition; // Previous play position within the sample buffer.
    u32                 nSamplesBase;
    u32                 nSamplesAdjust;
    u32                 PlayPosition;       // Current play position (samples).
    u32                 ReleasePosition;    // Play position to release on (samples).
    f32                 Volume;             // Volume [0.0..1.0]
    f32                 Pitch;              // Pitch [0.0..4.0]
    f32                 Pan2d;              // Pan [-1.0..1.0]
    vector4             Pan3d;              // 3d pan
    f32                 EffectSend;         // Effect Send [0.0..1.0]
    u32                 Dirty;              // Dirty bits.
    s32                 Priority;           // Priority [0..255]
    channel_state       State;              // State variable.
    stream_data         StreamData;
    hardware_data       Hardware;           // Hardware specific data.
};

struct stream_sample_instance
{
    hot_sample         Sample;
    compression_header Header;
};

struct audio_stream
{
volatile stream_type            Type;                                   // Type of stream (free/mono/stereo).
         compression_types      CompressionType;                        // How is this streams data compressed?
         stream_sample_instance Samples[MAX_STREAM_CHANNELS];           // Sample instances.
volatile xbool                  StreamDone;                             // Stream done?
         s32                    Index;                                  // Index of the stream.
         io_request*            pIoRequest;                             // Pointer to the io request.
         io_open_file*          FileHandle;                             // File handle the stream is reading from.
         u32                    WaveformOffset;                         // Offset within the file.
         u32                    WaveformLength;                         // Length of the waveforem data.
         u32                    WaveformCursor;                         // Current position within file.
         channel*               pChannel[MAX_STREAM_CHANNELS];          // Channel pointers. 
         u32                    ARAM[MAX_STREAM_CHANNELS][2];           // ARAM Stereo Buffers (double buffered).
         u32                    MainRAM[2];                             // Main Ram MP3 Stereo Buffers (double buffered).
         u32                    ReadBufferSize;                         // Size of the read buffer.
volatile u32                    ARAMWriteBuffer;                        // Which aram buffer is the write buffer?
volatile void*                  HandleMP3;                              // MP3 de-coder handle.
volatile s32                    CursorMP3;                              // Cursor within the mp3 stream.
volatile xbool                  bStartStream;                           // Start the stream up?
volatile xbool                  bStopStream;                            // Stop the stream?
volatile xbool                  bOpenStream;                            // Open the stream?
};

//------------------------------------------------------------------------------
// In progress...

struct warm_sample
{
    hot_sample*          pHotSample;         // Hot part...
    cold_sample*         pColdSample;        // Cold part...
};

#endif
