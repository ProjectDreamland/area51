#ifndef AUDIO_PRIVATE_PKG_HPP
#define AUDIO_PRIVATE_PKG_HPP

//------------------------------------------------------------------------------  
// The version identifier is limited to 16 bytes.  Please don't make it longer
// as x_strncpy (with 16 byte limit) is used to copy the identifier.
//------------------------------------------------------------------------------

#define GCN_PACKAGE_VERSION    "v1.7"
#define PS2_PACKAGE_VERSION    "v1.7"
#define XBOX_PACKAGE_VERSION   "v1.7"
#define PC_PACKAGE_VERSION     "v1.8"

//------------------------------------------------------------------------------
// ******* IMPORTANT ****** IMPORTANT ******* IMPORTANT ****** IMPORTANT *******
//
// If *any* of the buffer constants or package or descriptor encoding/decoding
// methods/macros are changed, the package version for all affected platforms 
// needs to be changed to force a re-packaging of samples.
//
// ******* IMPORTANT ****** IMPORTANT ******* IMPORTANT ****** IMPORTANT *******
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Streaming constants (these are not actually used, but are shown here
// to indicate how the size of the streaming buffers was determined).
//------------------------------------------------------------------------------

#define GCN_SAMPLE_RATE                 (32000.0f)
#define GCN_MAX_SEEK_TIME               (0.300f)             
#define GCN_BYTES_PER_SAMPLE            (2.0f)
#define GCN_ADPCM_COMPRESSION_RATIO     (3.5f)
#define GCN_ADPCM_BUFFER_SIZE_CALC      (2048 * ((s32)((GCN_SAMPLE_RATE * GCN_BYTES_PER_SAMPLE / GCN_ADPCM_COMPRESSION_RATIO * GCN_MAX_SEEK_TIME * (f32)MAX_STREAMS + 2047.0f) / 2048.0f)))

//------------------------------------------------------------------------------
// General streaming constants.

#define MAX_AUDIO_STREAMS               (4)
#define MAX_NON_AUDIO_STREAMS           (1)
#define MAX_STREAMS                     (MAX_AUDIO_STREAMS+MAX_NON_AUDIO_STREAMS)

//------------------------------------------------------------------------------
// Buffer constants.

#define GCN_ADPCM_CHANNEL_BUFFER_SIZE   (32*1024)
#define PS2_ADPCM_CHANNEL_BUFFER_SIZE    (32*1024)
#define XBOX_ADPCM_CHANNEL_BUFFER_SIZE    (32*1024)

#define MP3_BUFFER_SIZE                 (16*1024)

//------------------------------------------------------------------------------
// Package target identifiers

#define GCN_TARGET_ID                   "Gamecube"
#define PS2_TARGET_ID                   "PlayStation II"
#define XBOX_TARGET_ID                  "Xbox"
#define PC_TARGET_ID                    "Windows"

//------------------------------------------------------------------------------
// Audio package extension.

#define PACKAGE_EXTENSION               "audiopkg"

//------------------------------------------------------------------------------
// Sizes of package header data

#define VERSION_ID_SIZE                  (16) 
#define TARGET_ID_SIZE                   (16)
#define USER_ID_SIZE                     (16)
#define DATE_TIME_SIZE                   (16)

//------------------------------------------------------------------------------
// Index (element) encoding/decoding macros.
//------------------------------------------------------------------------------

// Index type is stored in bits 14-15.
#define GET_INDEX_TYPE( n )                     (index_type)((n >> 14) & 3)
#define SET_INDEX_TYPE( n, value )              { n &= 0x3fff; n |= (u16)((value & 3)<<14); }

// Index parameter flag is stored in bit 13
#define GET_INDEX_HAS_PARAMS( n )               (xbool)(( n & (1<<13) ) != 0)

// Pass the size of the parameters in bytes (including the flags).
#define SET_INDEX_HAS_PARAMS( n, value )        { n &= ~(1<<13); if( value ) n |= (1<<13); }

// Index is stored in bits 0-12.
#define GET_INDEX( n )                          (s32)(n & ((1<<13)-1))
#define SET_INDEX( n, value )                   { n &= ~((1<<13)-1); n |= ((u16)value & ((1<<13)-1)); }

// Maximum descriptor index (maximum number of "samples" per audio package).
#define MAX_DESCRIPTOR_INDEX                    (1<<13)

//------------------------------------------------------------------------------
// Descriptor encoding/decoding macros.
//------------------------------------------------------------------------------

// Descriptor type is stored in bits 14-15.
#define GET_DESCRIPTOR_TYPE( n )                (descriptor_type)((n >> 14) & 3)
#define SET_DESCRIPTOR_TYPE( n, value )         { n &= 0x3fff; n |= (u16)((value & 3)<<14); }

// Descriptor parameters flag stored in bit 13.
#define GET_DESCRIPTOR_HAS_PARAMS( n )          (xbool)(( n & (1<<13) ) != 0)
 
// Pass the size of the parameters in bytes (including the flags).
#define SET_DESCRIPTOR_HAS_PARAMS( n, value )   { n &= ~(1<<13); if( value ) n |= (1<<13); }

//------------------------------------------------------------------------------
// "Get" parameter bit macros.
//------------------------------------------------------------------------------

#define GET_PITCH_BIT( bits )                   (xbool)((bits & (1<<PITCH)) != 0)      
#define GET_PITCH_VARIANCE_BIT( bits )          (xbool)((bits & (1<<PITCH_VARIANCE)) != 0) 
#define GET_VOLUME_BIT( bits )                  (xbool)((bits & (1<<VOLUME)) != 0)        
#define GET_VOLUME_VARIANCE_BIT( bits )         (xbool)((bits & (1<<VOLUME_VARIANCE)) != 0)
#define GET_VOLUME_CENTER_BIT( bits )           (xbool)((bits & (1<<VOLUME_CENTER)) != 0)
#define GET_VOLUME_LFE_BIT( bits )              (xbool)((bits & (1<<VOLUME_LFE)) != 0)
#define GET_VOLUME_DUCK_BIT( bits )             (xbool)((bits & (1<<VOLUME_DUCK)) != 0)
#define GET_USER_DATA_BIT( bits )               (xbool)((bits & (1<<USER_DATA)) != 0)
#define GET_REPLAY_DELAY_BIT( bits )            (xbool)((bits & (1<<REPLAY_DELAY)) != 0)
#define GET_LAST_PLAY_BIT( bits )               (xbool)((bits & (1<<LAST_PLAY)) != 0)
#define GET_PAN_2D_BIT( bits )                  (xbool)((bits & (1<<PAN_2D)) != 0)            
#define GET_PRIORITY_BIT( bits )                (xbool)((bits & (1<<PRIORITY)) != 0)       
#define GET_EFFECT_SEND_BIT( bits )             (xbool)((bits & (1<<EFFECT_SEND)) != 0)    
#define GET_NEAR_FALLOFF_BIT( bits )            (xbool)((bits & (1<<NEAR_FALLOFF)) != 0)        
#define GET_FAR_FALLOFF_BIT( bits )             (xbool)((bits & (1<<FAR_FALLOFF)) != 0)        
#define GET_POSITION_BIT( bits )                (xbool)((bits & (1<<POSITION)) != 0)        
#define GET_ROLLOFF_METHOD_BIT( bits )          (xbool)((bits & (1<<ROLLOFF_METHOD)) != 0)
#define GET_NEAR_DIFFUSE_BIT( bits )            (xbool)((bits & (1<<NEAR_DIFFUSE)) != 0)
#define GET_FAR_DIFFUSE_BIT( bits )             (xbool)((bits & (1<<FAR_DIFFUSE)) != 0)
#define GET_PLAY_PERCENT_BIT( bits )            (xbool)((bits & (1<<PLAY_PERCENT)) != 0)

//------------------------------------------------------------------------------
// "Set" parameter bit macros.
//------------------------------------------------------------------------------

#define SET_PITCH_BIT( bits, flag )             {bits &= ~(1<<PITCH);           if (flag) bits |= (1<<PITCH);}     
#define SET_PITCH_VARIANCE_BIT( bits, flag )    {bits &= ~(1<<PITCH_VARIANCE);  if (flag) bits |= (1<<PITCH_VARIANCE);}       
#define SET_VOLUME_BIT( bits, flag )            {bits &= ~(1<<VOLUME);          if (flag) bits |= (1<<VOLUME);}       
#define SET_VOLUME_VARIANCE_BIT( bits, flag )   {bits &= ~(1<<VOLUME_VARIANCE); if (flag) bits |= (1<<VOLUME_VARIANCE);}       
#define SET_VOLUME_CENTER_BIT( bits, flag )     {bits &= ~(1<<VOLUME_CENTER);   if (flag) bits |= (1<<VOLUME_CENTER);}
#define SET_VOLUME_LFE_BIT( bits, flag )        {bits &= ~(1<<VOLUME_LFE);      if (flag) bits |= (1<<VOLUME_LFE);}
#define SET_VOLUME_DUCK_BIT( bits, flag )       {bits &= ~(1<<VOLUME_DUCK);     if (flag) bits |= (1<<VOLUME_DUCK);}
#define SET_USER_DATA_BIT( bits, flag )         {bits &= ~(1<<USER_DATA);       if (flag) bits |= (1<<USER_DATA);}
#define SET_REPLAY_DELAY_BIT( bits, flag )      {bits &= ~(1<<REPLAY_DELAY);    if (flag) bits |= (1<<REPLAY_DELAY);}       
#define SET_LAST_PLAY_BIT( bits, flag )         {bits &= ~(1<<LAST_PLAY);       if (flag) bits |= (1<<LAST_PLAY);}       
#define SET_PAN_2D_BIT( bits, flag )            {bits &= ~(1<<PAN_2D);          if (flag) bits |= (1<<PAN_2D);}       
#define SET_PRIORITY_BIT( bits, flag )          {bits &= ~(1<<PRIORITY);        if (flag) bits |= (1<<PRIORITY);}       
#define SET_EFFECT_SEND_BIT( bits, flag )       {bits &= ~(1<<EFFECT_SEND);     if (flag) bits |= (1<<EFFECT_SEND);}       
#define SET_NEAR_FALLOFF_BIT( bits, flag )      {bits &= ~(1<<NEAR_FALLOFF);    if (flag) bits |= (1<<NEAR_FALLOFF);}       
#define SET_FAR_FALLOFF_BIT( bits, flag )       {bits &= ~(1<<FAR_FALLOFF);     if (flag) bits |= (1<<FAR_FALLOFF);}       
#define SET_ROLLOFF_METHOD_BIT( bits, flag )    {bits &= ~(1<<ROLLOFF_METHOD);  if (flag) bits |= (1<<ROLLOFF_METHOD);}       
#define SET_NEAR_DIFFUSE_BIT( bits, flag )      {bits &= ~(1<<NEAR_DIFFUSE);    if (flag) bits |= (1<<NEAR_DIFFUSE);}       
#define SET_FAR_DIFFUSE_BIT( bits, flag )       {bits &= ~(1<<FAR_DIFFUSE);     if (flag) bits |= (1<<FAR_DIFFUSE);}       
#define SET_PLAY_PERCENT_BIT( bits, flag )      {bits &= ~(1<<PLAY_PERCENT);    if (flag) bits |= (1<<PLAY_PERCENT);}       

//------------------------------------------------------------------------------
// Parameter compression macros.
//------------------------------------------------------------------------------

#define FLOAT1_TO_U8BIT( n )                    ((u8)((n * 255.0f) + 0.5f))        
#define FLOAT10_TO_U8BIT( n )                   ((u8)((n * 255.0f / 10.0f) + 0.5f))
#define FLOAT1_TO_S8BIT( n )                    ((s8)(n * 127.0f))
#define FLOAT4_TO_U16BIT( n )                   ((u16)((n * 65535.0f / 4.0f) + 0.5f))
#define FLOAT1_TO_U16BIT( n )                   ((u16)((n * 65535.0f) + 0.5f))
#define FLOAT100_TO_U16BIT( n )                 ((u16)((n * 65535.0f / 100.0f) + 0.5f))
#define FLOAT_TENTH_TO_U16BIT( n )              ((u16)((n * 10) + 0.5f))              
                                              
//------------------------------------------------------------------------------
// Parameter de-compression macros.
//------------------------------------------------------------------------------

#define U8BIT_TO_FLOAT1( n )                    ((f32)n / 255.0f)        
#define U8BIT_TO_FLOAT10( n )                   ((f32)n * 10.0f / 255.0f)
#define S8BIT_TO_FLOAT1( n )                    ((f32)n / 127.0f)
#define U16BIT_TO_FLOAT4( n )                   ((f32)n * 4.0f / 65535.0f)
#define U16BIT_TO_FLOAT1( n )                   ((f32)n / 65535.0f)
#define U16BIT_TO_FLOAT100( n )                 ((f32)n * 100.0f / 65535.0f)
#define U16BIT_TO_FLOAT_TENTH( n )              ((f32)n / 10.0f)

//------------------------------------------------------------------------------
// "Get" flag bit macros.
//------------------------------------------------------------------------------

#define GET_SURROUND_ENABLED( bits )            (xbool)((bits & (1<<SURROUND_ENABLED)) != 0)      

//------------------------------------------------------------------------------
// "Set" flag bit macros.
//------------------------------------------------------------------------------

#define SET_SURROUND_ENABLED( bits, flag )      {bits &= ~(1<<SURROUND_ENABLED); if (flag) bits |= (1<<SURROUND_ENABLED);}     

//------------------------------------------------------------------------------
// Enums.
//------------------------------------------------------------------------------

enum rolloff_methods
{
    LINEAR_ROLLOFF=0, // 
    FAST_ROLLOFF,     // x ^ 2     falloff
    SLOW_ROLLOFF,     // x ^ (1/2) falloff
    NUM_ROLLOFFS,
};

enum compression_types
{
    ADPCM=0,
    PCM,
    MP3,
    NUM_COMPRESSION_TYPES,
};

enum compression_methods
{
    GCN_ADPCM_MONO=0,
    GCN_ADPCM_NON_INTERLEAVED_STEREO,
    GCN_ADPCM_MONO_STREAMED,
    GCN_ADPCM_INTERLEAVED_STEREO,

    PC_PCM_MONO,
    PC_PCM_NON_INTERLEAVED_STEREO,
    PC_PCM_MONO_STREAMED,
    PC_PCM_INTERLEAVED_STEREO,

    PS2_ADPCM_MONO,
    PS2_ADPCM_NON_INTERLEAVED_STEREO,
    PS2_ADPCM_MONO_STREAMED,
    PS2_ADPCM_INTERLEAVED_STEREO,

    XBOX_ADPCM_MONO,
    XBOX_ADPCM_NON_INTERLEAVED_STEREO,
    XBOX_ADPCM_MONO_STREAMED,
    XBOX_ADPCM_INTERLEAVED_STEREO,

    XBOX_PCM_MONO,
    XBOX_PCM_NON_INTERLEAVED_STEREO,
    XBOX_PCM_MONO_STREAMED,
    XBOX_PCM_INTERLEAVED_STEREO,

    PC_ADPCM_MONO,
    PC_ADPCM_NON_INTERLEAVED_STEREO,
    PC_ADPCM_MONO_STREAMED,
    PC_ADPCM_INTERLEAVED_STEREO,

    PC_MP3_MONO,
    PC_MP3_NON_INTERLEAVED_STEREO,
    PC_MP3_MONO_STREAMED,
    PC_MP3_INTERLEAVED_STEREO,

    GCN_MP3_MONO,
    GCN_MP3_NON_INTERLEAVED_STEREO,
    GCN_MP3_MONO_STREAMED,
    GCN_MP3_INTERLEAVED_STEREO,
};

enum element_type
{
    HOT_SAMPLE = 0,
    WARM_SAMPLE,
    COLD_SAMPLE,
    NUM_TEMPERATURES,
};

enum sample_temperature
{
    HOT=0,
    WARM,
    COLD,
};

enum descriptor_type
{
    SIMPLE = 0,
    COMPLEX,
    RANDOM_LIST,
    WEIGHTED_LIST,
    NUM_DESCRIPTOR_TYPES
};

enum index_type
{
    HOT_INDEX = 0,      // Index references a loaded sample.
    WARM_INDEX,         // Index references a "hybrid" sample.
    COLD_INDEX,         // Index references a streamed sample.
    DESCRIPTOR_INDEX,   // Index references an audio descriptor.
};

enum
{
    PITCH = 0,          // 16-bit represents [0.0..4.0]
    PITCH_VARIANCE,     // 16-bit represents +/- [0.0..1.0]
    VOLUME,             // 16-bit represents [0.0...1.0]
    VOLUME_VARIANCE,    // 16-bit represents +/- [0.0..1.0]
    VOLUME_CENTER,      // 16-bit represents [0.0...1.0]
    VOLUME_LFE,         // 16-bit represents [0.0...1.0]
    VOLUME_DUCK,        // 16-bit represents [0.0..1.0]
    USER_DATA,          // 16-bit represents whatever...
    REPLAY_DELAY,       // 16-bit represents [0.0..6553.5] seconds
    LAST_PLAY,          // 16-bit represents [0.0..6553.5] the last time the descriptor was played.         
    PAN_2D,             // 8-bit represents [-1.0..1.0]
    PRIORITY,           // 8-bit represents 256 priorities
    EFFECT_SEND,        // 8-bit represents [0.0..1.0]
    NEAR_FALLOFF,       // 8-bit represents [0%..1000%]
    FAR_FALLOFF,        // 8-bit represents [0%..1000%]
    ROLLOFF_METHOD,     // 8-bit represents 256 possible rollof methods.
    NEAR_DIFFUSE,       // 8-bit represents [0%..1000%]
    FAR_DIFFUSE,        // 8-bit represents [0%..1000%]
    PLAY_PERCENT,       // 8-bit represents [0..100]
    NUM_PARAMETERS,
};

enum
{
    SURROUND_ENABLED=0, // Use surround?
    NUM_FLAGS,
};

struct compressed_parameters
{
    u16 Bits1;          // Lower 16-bits describing which parameters are represented.
    u16 Bits2;          // Upper 16-bits describing which parameters are represented.
    u16 Flags;          // Bits representing flags.
    u16 Pitch;          // 16-bit represents [0.0..4.0]
    u16 PitchVariance;  // 16-bit represents +/- [0.0..1.0]
    u16 Volume;         // 16-bit represents [0.0...1.0]
    u16 VolumeVariance; // 16-bit represents +/- [0.0..1.0]
    u16 VolumeCenter;   // 16-bit represents [0.0...1.0]
    u16 VolumeLFE;      // 16-bit represents [0.0...1.0]
    u16 VolumeDuck;     // 16-bit represents [0.0...1.0]
    u16 UserData;       // 16-bit represents whatever...
    u16 ReplayDelay;    // 16-bit represents [0.0..6553.5] seconds.
    u16 LastPlay;       // 16-bit represents the last time the descriptor was played.
    s8  Pan2d;          // 8-bit represents [-1.0..1.0]
    u8  Priority;       // 8-bit represents 256 priorities
    u8  EffectSend;     // 8-bit represents [0.0..1.0]
    u8  NearFalloff;    // 8-bit represents [0%..1000%]
    u8  FarFalloff;     // 8-bit represents [0%..1000%]
    u8  RolloffCurve;   // 8-bit represents 256 rolloff methods.
    u8  NearDiffuse;    // 8-bit represents [0%..1000%]
    u8  FarDiffuse;     // 8-bit represents [0%..1000%]
    u8  PlayPercent;    // 8-bit represents [0..100]
};

struct uncompressed_parameters
{
    u32     Bits;           // Bits describing which parameters are represented
    u32     Flags;          // Bits describing the flags.
    f32     Pitch;          // [0.0..4.0]
    f32     PitchVariance;  // +/- [0.0..1.0]
    f32     Volume;         // [0.0...1.0]
    f32     VolumeVariance; // +/- [0.0..1.0]
    f32     VolumeCenter;   // [0.0...1.0]
    f32     VolumeLFE;      // [0.0...1.0]
    f32     VolumeDuck;     // [0.0...1.0]
    u32     UserData;       // 16-bits of whatever...
    f32     ReplayDelay;    // [0.1..100.0]
    f32     LastPlay;       // represents the last time the descriptor was played.
    f32     Pan2d;          // [-1.0..1.0] (2d stereo pan)
    s32     Priority;       // 0..255 priorities.
    f32     EffectSend;     // [0.0..1.0]
    f32     NearFalloff;    // [0%..1000%]
    f32     FarFalloff;     // [0%..1000%]
    s32     RolloffCurve;   // 0..255 rolloff methods.
    f32     NearDiffuse;    // [-1.0..0.0]
    f32     FarDiffuse;     // [-1.0..0.0]
    u32     PlayPercent;    // [0..100]
    vector4 Pan3d;          // 3d pan.
};

//------------------------------------------------------------------------------
// Package header structure.
//------------------------------------------------------------------------------

struct package_header
{
    s32                     Mram;
    s32                     Aram;
    uncompressed_parameters Params;
    s32                     nDescriptors;
    s32                     nIdentifiers;
    s32                     DescriptorFootprint;
    s32                     StringTableFootprint;
    s32                     LipSyncTableFootprint;
    s32                     BreakPointTableFootprint;
    s32                     MusicDataFootprint;
    s32                     nSampleHeaders[ NUM_TEMPERATURES ];
    s32                     nSampleIndices[ NUM_TEMPERATURES ];
    s32                     CompressionType[ NUM_TEMPERATURES ];
    s32                     HeaderSizes[ NUM_TEMPERATURES ];
};

//------------------------------------------------------------------------------
// Sample header structures.
//------------------------------------------------------------------------------

struct gcn_adpcm_info
{
    // start context
    s16 coef[16];
    u16 gain;
    u16 pred_scale;
    s16 yn1;
    s16 yn2;
};

struct gcn_adpcm_loop
{
    // loop context
    u16 loop_pred_scale;
    s16 loop_yn1;
    s16 loop_yn2;
};

struct gcn_adpcm_compression_header
{
    gcn_adpcm_info  Info;
    gcn_adpcm_loop  Loop;
};

union compression_header
{
    gcn_adpcm_compression_header gcn_ADPCM;
};

struct sample_header
{
    u32             AudioRam;         // Address in audio ram.
    u32             WaveformOffset;   // Offset within the file of the sample.
    u32             WaveformLength;   // Length of the waveform data.
    u32             LipSyncOffset;    // Offset within the LipSync data (-1 = not defined)
    u32             BreakPointOffset; // Offset within the BreakPoint data (-1 = not defined)
    u32             CompressionType;  // ADPCM, PCM, MP3, etc...
    s32             nSamples;         // Number of samples.
    s32             SampleRate;       // Sample rate.
    s32             LoopStart;
    s32             LoopEnd;
};

#endif // AUDIO_PRIVATE_PKG_HPP
