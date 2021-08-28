/************************************************************************
*                                    ~~                                   *
*   dmusici.h -- This module contains the API for the                   *
*                DirectMusic performance layer                          *
*                                                                       *
*   Copyright (c) 1998-1999 Microsoft Corporation
*                                                                       *
************************************************************************/

#ifndef _DMUSICI_
#define _DMUSICI_

#include <xtl.h>
#ifndef _DUMMY_MSG
#define _DUMMY_MSG
struct MSG;
typedef struct MSG* LPMSG;
#endif
#include <xobjbase.h>
#include <pshpack8.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef WORD            TRANSITION_TYPE;
typedef long            MUSIC_TIME;

#define MT_MIN          0x80000000  /* Minimum music time value. */
#define MT_MAX          0x7FFFFFFF  /* Maximum music time value. */

#define DMUS_PPQ        768     /* parts per quarter note */

interface IDirectMusicPerformance;
interface IDirectMusicSegment;
interface IDirectMusicSegmentState;
interface IDirectMusicLoader;
interface IDirectMusicScript;
interface IDirectMusicAudioPath;
interface IDirectMusicTool;
interface IDirectMusicGraph;
interface IDirectMusicObject;
#ifndef __cplusplus 
typedef interface IDirectMusicPerformance IDirectMusicPerformance;
typedef interface IDirectMusicSegment IDirectMusicSegment;
typedef interface IDirectMusicSegmentState IDirectMusicSegmentState;
typedef interface IDirectMusicObject IDirectMusicObject;
typedef interface IDirectMusicLoader IDirectMusicLoader;
typedef interface IDirectMusicScript IDirectMusicScript;
typedef interface IDirectMusicAudioPath IDirectMusicAudioPath;
typedef interface IDirectMusicTool IDirectMusicTool;
typedef interface IDirectMusicGraph IDirectMusicGraph;
#endif


#define DMUS_PMSG_PART                                                                              \
    DWORD               dwSize;                                                                     \
    REFERENCE_TIME      rtTime;             /* real time (in 100 nanosecond increments) */          \
    MUSIC_TIME          mtTime;             /* music time */                                        \
    DWORD               dwFlags;            /* various bits (see DMUS_PMSGF_FLAGS enumeration) */    \
    DWORD               dwPChannel;         /* Performance Channel. The Performance can */          \
                                            /* use this to determine the port/channel. */           \
    DWORD               dwVirtualTrackID;   /* virtual track ID */                                  \
    IDirectMusicTool*   pTool;              /* tool interface pointer */                            \
    IDirectMusicGraph*  pGraph;             /* tool graph interface pointer */                      \
    DWORD               dwType;             /* PMSG type (see DMUS_PMSGT_TYPES defines) */              \
    DWORD               dwVoiceID;          /* unique voice id which allows synthesizers to */      \
                                            /* identify a specific event. For DirectX 6.0, */       \
                                            /* this field should always be 0. */                    \
    DWORD               dwGroupID;          /* Track group id */                                 \
    IUnknown*           punkUser;           /* user com pointer, auto released upon PMSG free */

/* every DMUS_PMSG is based off of this structure. The Performance needs 
   to access these members consistently in every PMSG that goes through it. */
typedef struct _DMUS_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

} DMUS_PMSG;

#define DMUS_PCHANNEL_BROADCAST_PERFORMANCE 0xFFFFFFFF  /* PMsg is sent on all PChannels of the performance. */
#define DMUS_PCHANNEL_BROADCAST_AUDIOPATH   0xFFFFFFFE  /* PMsg is sent on all PChannels of the audio path. */
#define DMUS_PCHANNEL_BROADCAST_SEGMENT     0xFFFFFFFD  /* PMsg is sent on all PChannels of the segment. */
#define DMUS_PCHANNEL_BROADCAST_GROUPS      0xFFFFFFFC  /* A duplicate PMsg is for each Channels Groups in the performance. */

/*  The DMUS_PATH constants are used in conjunction with GetObjectInPath to find a requested
    interface at a particular stage in the audio path. 
*/
#define DMUS_PATH_SEGMENT          0x1000      /* Get the segment itself (from a segment state.) */
#define DMUS_PATH_SEGMENT_GRAPH    0x1200      /* Get the segment's tool graph. */
#define DMUS_PATH_SEGMENT_TOOL     0x1300      /* Look in Tool Graph of Segment. */
#define DMUS_PATH_AUDIOPATH        0x2000      /* Get the audiopath itself (from a segment state.) */
#define DMUS_PATH_AUDIOPATH_GRAPH  0x2200      /* Get the audiopath's tool graph. */
#define DMUS_PATH_AUDIOPATH_TOOL   0x2300      /* Look in Tool Graph of Audio Path. */
#define DMUS_PATH_PERFORMANCE      0x3000      /* Access the performance. */
#define DMUS_PATH_PERFORMANCE_GRAPH 0x3200     /* Get the performance's tool graph. */
#define DMUS_PATH_PERFORMANCE_TOOL 0x3300      /* Look in Tool Graph of Performance. */
#define DMUS_PATH_BUFFER           0x6000      /* Look in DirectSoundBuffer. */

/*  To ignore PChannels when calling GetObjectInPath(), use the DMUS_PCHANNEL_ALL constant. */
#define DMUS_PCHANNEL_ALL           0xFFFFFFFB      

/*  The DMUS_APATH types are used in conjunction with CreateStandardAudioPath to
    build default path types. _SHARED_ means the same buffer is shared across multiple
    instantiations of the audiopath type. _DYNAMIC_ means a unique buffer is created
    every time. 
*/

/* A standard music set up with stereo outs and no reverb or chorus send. */
#define DMUS_APATH_SHARED_STEREO             0xFFFF0001
/* A standard music set up with stereo outs and reverb & chorus sends. */
#define DMUS_APATH_SHARED_STEREOPLUSREVERB   0xFFFF0002
/* An audio path with one dynamic bus from the synth feeding to a dynamic mono buffer. */
#define DMUS_APATH_DYNAMIC_MONO              0xFFFF0003   
/* An audio path with one dynamic bus from the synth feeding to a dynamic 3d buffer.*/
#define DMUS_APATH_DYNAMIC_3D                0xFFFF0004
/* Sends to quad mixbins on channels 1 through 4. */
#define DMUS_APATH_MIXBIN_QUAD               ((1 << DSMIXBIN_FRONT_LEFT ) | (1 << DSMIXBIN_FRONT_RIGHT ) | (1 << DSMIXBIN_BACK_LEFT ) | (1 << DSMIXBIN_BACK_RIGHT) )
/* Sends to quad mixbins on channels 1 through 4 and environmental reverb on 5. */
#define DMUS_APATH_MIXBIN_QUAD_ENV           ((1 << DSMIXBIN_FRONT_LEFT ) | (1 << DSMIXBIN_FRONT_RIGHT ) | (1 << DSMIXBIN_BACK_LEFT ) | (1 << DSMIXBIN_BACK_RIGHT ) | (1 << DSMIXBIN_I3DL2) )
/* Sends to quad mixbins on channels 1 through 4 and music reverb and chorus on 5, 6. */
#define DMUS_APATH_MIXBIN_QUAD_MUSIC         ((1 << DSMIXBIN_FRONT_LEFT ) | (1 << DSMIXBIN_FRONT_RIGHT ) | (1 << DSMIXBIN_BACK_LEFT ) | (1 << DSMIXBIN_BACK_RIGHT ) | (1 << DSMIXBIN_FXSEND_0 ) | (1 << DSMIXBIN_FXSEND_1) )
/* Sends to 5.1 mixbins on channels 1 through 6. */
#define DMUS_APATH_MIXBIN_5DOT1              ((1 << DSMIXBIN_FRONT_LEFT ) | (1 << DSMIXBIN_FRONT_RIGHT ) | (1 << DSMIXBIN_FRONT_CENTER ) | (1 << DSMIXBIN_LOW_FREQUENCY ) | (1 << DSMIXBIN_BACK_LEFT ) | (1 << DSMIXBIN_BACK_RIGHT) )
/* Sends to 5.1 mixbins on channels 1 through 6 and environmental reverb on 7. */
#define DMUS_APATH_MIXBIN_5DOT1_ENV          ((1 << DSMIXBIN_FRONT_LEFT ) | (1 << DSMIXBIN_FRONT_RIGHT ) | (1 << DSMIXBIN_FRONT_CENTER ) | (1 << DSMIXBIN_LOW_FREQUENCY ) | (1 << DSMIXBIN_BACK_LEFT ) | (1 << DSMIXBIN_BACK_RIGHT ) | (1 << DSMIXBIN_I3DL2) )
/* Sends to 5.1 mixbins on channels 1 through 6 and music reverb and chorus on 7, 8. */
#define DMUS_APATH_MIXBIN_5DOT1_MUSIC        ((1 << DSMIXBIN_FRONT_LEFT ) | (1 << DSMIXBIN_FRONT_RIGHT ) | (1 << DSMIXBIN_FRONT_CENTER ) | (1 << DSMIXBIN_LOW_FREQUENCY ) | (1 << DSMIXBIN_BACK_LEFT ) | (1 << DSMIXBIN_BACK_RIGHT ) | (1 << DSMIXBIN_FXSEND_0 ) | (1 << DSMIXBIN_FXSEND_1) )
/* 1,2 -> Stereo, 3 -> environmental reverb, 4 through 8 -> effects. */
#define DMUS_APATH_MIXBIN_STEREO_EFFECTS     ((1 << DSMIXBIN_FRONT_LEFT ) | (1 << DSMIXBIN_FRONT_RIGHT ) | (1 << DSMIXBIN_I3DL2 ) | (1 << DSMIXBIN_FXSEND_0 ) | (1 << DSMIXBIN_FXSEND_1 ) | (1 << DSMIXBIN_FXSEND_2 ) | (1 << DSMIXBIN_FXSEND_3 ) | (1 << DSMIXBIN_FXSEND_4) )


#define DMUS_INITAUDIO_NOTHREADS     0x1     /* If this is set, the app must call DirectMusicDoWork. */

/* DMUS_PMSGF_FLAGS fill the DMUS_PMSG's dwFlags member */
typedef enum enumDMUS_PMSGF_FLAGS
{
    DMUS_PMSGF_REFTIME          = 1,      /* if rtTime is valid */
    DMUS_PMSGF_MUSICTIME        = 2,      /* if mtTime is valid */
    DMUS_PMSGF_TOOL_IMMEDIATE   = 4,      /* if PMSG should be processed immediately */ 
    DMUS_PMSGF_TOOL_QUEUE       = 8,      /* if PMSG should be processed a little early, at Queue time */
    DMUS_PMSGF_TOOL_ATTIME      = 0x10,   /* if PMSG should be processed at the time stamp */
    DMUS_PMSGF_TOOL_FLUSH       = 0x20,   /* if PMSG is being flushed */
    DMUS_PMSGF_LOCKTOREFTIME    = 0x40,   /* if rtTime can not be overriden by a tempo change. */
    DMUS_PMSGF_DX8              = 0x80    /* if the message has DX8 or later extensions. */
    /* The values of DMUS_TIME_RESOLVE_FLAGS may also be used inside the */
    /* DMUS_PMSG's dwFlags member. */
} DMUS_PMSGF_FLAGS;

/* DMUS_PMSGT_TYPES fill the DMUS_PMSG's dwType member */
typedef enum enumDMUS_PMSGT_TYPES
{
    DMUS_PMSGT_MIDI             = 0,      /* MIDI short message */
    DMUS_PMSGT_NOTE             = 1,      /* Interactive Music Note */
    DMUS_PMSGT_SYSEX            = 2,      /* MIDI long message (system exclusive message) */
    DMUS_PMSGT_NOTIFICATION     = 3,      /* Notification message */
    DMUS_PMSGT_TEMPO            = 4,      /* Tempo message */
    DMUS_PMSGT_CURVE            = 5,      /* Control change / pitch bend, etc. curve */
    DMUS_PMSGT_TIMESIG          = 6,      /* Time signature */
    DMUS_PMSGT_PATCH            = 7,      /* Patch changes */
    DMUS_PMSGT_TRANSPOSE        = 8,      /* Transposition messages */
    DMUS_PMSGT_CHANNEL_PRIORITY = 9,      /* Channel priority */
    DMUS_PMSGT_STOP             = 10,     /* Stop message */
    DMUS_PMSGT_DIRTY            = 11,     /* Tells Tools that cache GetParam() info to refresh */
    DMUS_PMSGT_WAVE             = 12,     /* Carries control information for playing a wave. */
    DMUS_PMSGT_LYRIC            = 13,     /* Lyric message from lyric track. */
    DMUS_PMSGT_SCRIPTLYRIC      = 14,     /* Lyric message sent by a script with the Trace function. */
    DMUS_PMSGT_USER             = 255     /* User message */
} DMUS_PMSGT_TYPES;

/* DMUS_SEGF_FLAGS correspond to IDirectMusicPerformance::PlaySegment, and other API */
typedef enum enumDMUS_SEGF_FLAGS
{
    DMUS_SEGF_REFTIME           = 1<<6,   /* 0x40 Time parameter is in reference time  */
    DMUS_SEGF_SECONDARY         = 1<<7,   /* 0x80 Secondary segment */
    DMUS_SEGF_QUEUE             = 1<<8,   /* 0x100 Queue at the end of the primary segment queue (primary only) */
    DMUS_SEGF_CONTROL           = 1<<9,   /* 0x200 Play as a control track (secondary segments only) */
    DMUS_SEGF_AFTERPREPARETIME  = 1<<10,  /* 0x400 Play after the prepare time (See IDirectMusicPerformance::GetPrepareTime) */
    DMUS_SEGF_GRID              = 1<<11,  /* 0x800 Play on grid boundary */
    DMUS_SEGF_BEAT              = 1<<12,  /* 0x1000 Play on beat boundary */
    DMUS_SEGF_MEASURE           = 1<<13,  /* 0x2000 Play on measure boundary */
    DMUS_SEGF_DEFAULT           = 1<<14,  /* 0x4000 Use segment's default boundary */
    DMUS_SEGF_NOINVALIDATE      = 1<<15,  /* 0x8000 Play without invalidating the currently playing segment(s) */
    DMUS_SEGF_ALIGN             = 1<<16,  /* 0x10000 Align segment with requested boundary, but switch at first valid point */
    DMUS_SEGF_VALID_START_BEAT  = 1<<17,  /* 0x20000 In conjunction with DMUS_SEGF_ALIGN, allows the switch to occur on any beat. */
    DMUS_SEGF_VALID_START_GRID  = 1<<18,  /* 0x40000 In conjunction with DMUS_SEGF_ALIGN, allows the switch to occur on any grid. */
    DMUS_SEGF_VALID_START_TICK  = 1<<19,  /* 0x80000 In conjunction with DMUS_SEGF_ALIGN, allows the switch to occur any time. */
    DMUS_SEGF_AUTOTRANSITION    = 1<<20,  /* 0x100000 Compose and play a transition segment, using either the transition template or transition embedded in song. */
    DMUS_SEGF_AFTERQUEUETIME    = 1<<21,  /* 0x200000 Make sure to play after the queue time. This is default for primary segments */
    DMUS_SEGF_AFTERLATENCYTIME  = 1<<22,  /* 0x400000 Make sure to play after the latency time. This is true for all segments, so this is a nop */
    DMUS_SEGF_SEGMENTEND        = 1<<23,  /* 0x800000 Play at the next end of segment. */
    DMUS_SEGF_MARKER            = 1<<24,  /* 0x1000000 Play at next marker in the primary segment. If there are no markers, default to any other resolution requests. */
    DMUS_SEGF_TIMESIG_ALWAYS    = 1<<25,  /* 0x2000000 Even if there is no primary segment, align start time with current time signature. */
    DMUS_SEGF_USE_AUDIOPATH     = 1<<26,  /* 0x4000000 Uses the audio path that is embedded in the segment or song. */
    DMUS_SEGF_VALID_START_MEASURE = 1<<27,/* 0x8000000 In conjunction with DMUS_SEGF_ALIGN, allows the switch to occur on any bar. */
    DMUS_SEGF_INVALIDATE_PRI    = 1<<28   /* 0x10000000 invalidate only the current primary seg state */
} DMUS_SEGF_FLAGS;

#define DMUS_SEG_REPEAT_INFINITE    0xFFFFFFFF  /* For IDirectMusicSegment::SetRepeat*/
#define DMUS_SEG_ALLTRACKS          0x80000000  /* For IDirectMusicSegment::SetParam() and SetTrackConfig() - selects all tracks instead on nth index. */
#define DMUS_SEG_ANYTRACK           0x80000000  /* For IDirectMusicSegment::GetParam() - checks each track until it finds one that returns data (not DMUS_E_NOT_FOUND.) */
                                                

/* DMUS_TIME_RESOLVE_FLAGS correspond to IDirectMusicPerformance::GetResolvedTime, and can */
/* also be used interchangeably with the corresponding DMUS_SEGF_FLAGS, since their values */
/* are intentionally the same */
typedef enum enumDMUS_TIME_RESOLVE_FLAGS
{
    DMUS_TIME_RESOLVE_AFTERPREPARETIME  = DMUS_SEGF_AFTERPREPARETIME,
    DMUS_TIME_RESOLVE_AFTERQUEUETIME    = DMUS_SEGF_AFTERQUEUETIME,
    DMUS_TIME_RESOLVE_AFTERLATENCYTIME  = DMUS_SEGF_AFTERLATENCYTIME,
    DMUS_TIME_RESOLVE_GRID              = DMUS_SEGF_GRID,
    DMUS_TIME_RESOLVE_BEAT              = DMUS_SEGF_BEAT,
    DMUS_TIME_RESOLVE_MEASURE           = DMUS_SEGF_MEASURE,
    DMUS_TIME_RESOLVE_MARKER            = DMUS_SEGF_MARKER,
    DMUS_TIME_RESOLVE_SEGMENTEND        = DMUS_SEGF_SEGMENTEND,
} DMUS_TIME_RESOLVE_FLAGS;

/* The following flags are sent inside the DMUS_CHORD_KEY.dwFlags parameter */
typedef enum enumDMUS_CHORDKEYF_FLAGS
{
    DMUS_CHORDKEYF_SILENT            = 1,      /* is the chord silent? */
} DMUS_CHORDKEYF_FLAGS;

#define DMUS_MAXSUBCHORD 8

typedef struct _DMUS_SUBCHORD
{
    DWORD   dwChordPattern;     /* Notes in the subchord */
    DWORD   dwScalePattern;     /* Notes in the scale */
    DWORD   dwInversionPoints;  /* Where inversions can occur */
    DWORD   dwLevels;           /* Which levels are supported by this subchord */
    BYTE    bChordRoot;         /* Root of the subchord */
    BYTE    bScaleRoot;         /* Root of the scale */
} DMUS_SUBCHORD;

typedef struct _DMUS_CHORD_KEY
{
    WCHAR           wszName[16];        /* Name of the chord */
    WORD            wMeasure;           /* Measure this falls on */
    BYTE            bBeat;              /* Beat this falls on */
    BYTE            bSubChordCount;     /* Number of chords in the list of subchords */
    DMUS_SUBCHORD   SubChordList[DMUS_MAXSUBCHORD]; /* List of sub chords */
    DWORD           dwScale;            /* Scale underlying the entire chord */
    BYTE            bKey;               /* Key underlying the entire chord */
    BYTE            bFlags;             /* Miscelaneous flags */
} DMUS_CHORD_KEY;

typedef DMUS_CHORD_KEY DMUS_CHORD_PARAM; /* DMUS_CHORD_KEY defined in dmusici.h */

/* DMUS_NOTE_PMSG */
typedef struct _DMUS_NOTE_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    MUSIC_TIME mtDuration;     /* duration */
    WORD    wMusicValue;       /* Description of note in chord and key. */
    WORD    wMeasure;          /* Measure in which this note occurs */
    short   nOffset;           /* Offset from grid at which this note occurs */
    BYTE    bBeat;             /* Beat (in measure) at which this note occurs */
    BYTE    bGrid;             /* Grid offset from beat at which this note occurs */
    BYTE    bVelocity;         /* Note velocity */
    BYTE    bFlags;            /* see DMUS_NOTEF_FLAGS */
    BYTE    bTimeRange;        /* Range to randomize time. */
    BYTE    bDurRange;         /* Range to randomize duration. */
    BYTE    bVelRange;         /* Range to randomize velocity. */
    BYTE    bPlayModeFlags;    /* Play mode */
    BYTE    bSubChordLevel;    /* Which subchord level this note uses.  */
    BYTE    bMidiValue;        /* The MIDI note value, converted from wMusicValue */
    char    cTranspose;        /* Transposition to add to midi note value after converted from wMusicValue. */
} DMUS_NOTE_PMSG;

typedef enum enumDMUS_NOTEF_FLAGS
{
    DMUS_NOTEF_NOTEON = 1,              /* Set if this is a MIDI Note On. Otherwise, it is MIDI Note Off */
    DMUS_NOTEF_NOINVALIDATE = 2,        /* Don't invalidate this note off. */
    DMUS_NOTEF_NOINVALIDATE_INSCALE = 4,/* Don't invalidate if still within the scale. */
    DMUS_NOTEF_NOINVALIDATE_INCHORD = 8,/* Don't invalidate if still within the chord. */
    DMUS_NOTEF_REGENERATE = 0x10,       /* Regenerate the note on an invalidate. */
} DMUS_NOTEF_FLAGS;

/* The DMUS_PLAYMODE_FLAGS are used to determine how to convert wMusicValue
   into the appropriate bMidiValue.
*/

typedef enum enumDMUS_PLAYMODE_FLAGS
{
    DMUS_PLAYMODE_KEY_ROOT          = 1,  /* Transpose on top of the key root. */
    DMUS_PLAYMODE_CHORD_ROOT        = 2,  /* Transpose on top of the chord root. */
    DMUS_PLAYMODE_SCALE_INTERVALS   = 4,  /* Use scale intervals from scale pattern. */
    DMUS_PLAYMODE_CHORD_INTERVALS   = 8,  /* Use chord intervals from chord pattern. */
    DMUS_PLAYMODE_NONE              = 16, /* No mode. Indicates the parent part's mode should be used. */
} DMUS_PLAYMODE_FLAGS;

/* The following are playback modes that can be created by combining the DMUS_PLAYMODE_FLAGS
   in various ways:
*/

/* Fixed. wMusicValue holds final MIDI note value. This is used for drums, sound effects, and sequenced
   notes that should not be transposed by the chord or scale.
*/
#define DMUS_PLAYMODE_FIXED             0  
/* In fixed to key, the musicvalue is again a fixed MIDI value, but it
   is transposed on top of the key root. 
*/
#define DMUS_PLAYMODE_FIXEDTOKEY        DMUS_PLAYMODE_KEY_ROOT
/* In fixed to chord, the musicvalue is also a fixed MIDI value, but it
   is transposed on top of the chord root. 
*/
#define DMUS_PLAYMODE_FIXEDTOCHORD      DMUS_PLAYMODE_CHORD_ROOT
/* In Pedalpoint, the key root is used and the notes only track the intervals in
   the scale. The chord root and intervals are completely ignored. This is useful
   for melodic lines that play relative to the key root.
*/
#define DMUS_PLAYMODE_PEDALPOINT        (DMUS_PLAYMODE_KEY_ROOT | DMUS_PLAYMODE_SCALE_INTERVALS)
/* In the Melodic mode, the chord root is used but the notes only track the intervals in
   the scale. The key root and chord intervals are completely ignored. This is useful
   for melodic lines that play relative to the chord root. 
*/
#define DMUS_PLAYMODE_MELODIC           (DMUS_PLAYMODE_CHORD_ROOT | DMUS_PLAYMODE_SCALE_INTERVALS)
/* Normal chord mode is the prevalent playback mode. 
   The notes track the intervals in the chord, which is based on the chord root. 
   If there is a scale component to the MusicValue, the additional intervals 
   are pulled from the scale and added.
   If the chord does not have an interval to match the chord component of
   the MusicValue, the note is silent.
*/
#define DMUS_PLAYMODE_NORMALCHORD       (DMUS_PLAYMODE_CHORD_ROOT | DMUS_PLAYMODE_CHORD_INTERVALS)
/* If it is desirable to play a note that is above the top of the chord, the
   always play mode (known as "purpleized" in a former life) finds a position
   for the note by using intervals from the scale. Essentially, this mode is
   a combination of the Normal and Melodic playback modes, where a failure
   in Normal causes a second try in Melodic mode.
*/
#define DMUS_PLAYMODE_ALWAYSPLAY        (DMUS_PLAYMODE_MELODIC | DMUS_PLAYMODE_NORMALCHORD)

/* In PedalpointChord, the key root is used and the notes only track the intervals in
   the chord. The chord root and scale intervals are completely ignored. This is useful
   for chordal lines that play relative to the key root.
*/
#define DMUS_PLAYMODE_PEDALPOINTCHORD   (DMUS_PLAYMODE_KEY_ROOT | DMUS_PLAYMODE_CHORD_INTERVALS)

/* For completeness, here's a mode that tries for pedalpointchord, but if it fails
   uses scale intervals
*/
#define DMUS_PLAYMODE_PEDALPOINTALWAYS  (DMUS_PLAYMODE_PEDALPOINT | DMUS_PLAYMODE_PEDALPOINTCHORD)


/*  Legacy names for modes... */
#define DMUS_PLAYMODE_PURPLEIZED        DMUS_PLAYMODE_ALWAYSPLAY
#define DMUS_PLAYMODE_SCALE_ROOT        DMUS_PLAYMODE_KEY_ROOT
#define DMUS_PLAYMODE_FIXEDTOSCALE      DMUS_PLAYMODE_FIXEDTOKEY

/* DMUS_MIDI_PMSG */
typedef struct _DMUS_MIDI_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    BYTE    bStatus;
    BYTE    bByte1;
    BYTE    bByte2;
    BYTE    bPad[1];
} DMUS_MIDI_PMSG;

/* DMUS_PATCH_PMSG */
typedef struct _DMUS_PATCH_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    BYTE    byInstrument;
    BYTE    byMSB;
    BYTE    byLSB;
    BYTE    byPad[1];
} DMUS_PATCH_PMSG;

/* DMUS_TRANSPOSE_PMSG */
typedef struct _DMUS_TRANSPOSE_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    short   nTranspose;
    WORD            wMergeIndex;     /* Allows multiple parameters to be merged (pitchbend, volume, and expression.)*/
} DMUS_TRANSPOSE_PMSG;

/* DMUS_CHANNEL_PRIORITY_PMSG */
typedef struct _DMUS_CHANNEL_PRIORITY_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    DWORD   dwChannelPriority;
} DMUS_CHANNEL_PRIORITY_PMSG;

/* DMUS_TEMPO_PMSG */
typedef struct _DMUS_TEMPO_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    double  dblTempo;                       /* the tempo */
} DMUS_TEMPO_PMSG;

#define DMUS_TEMPO_MAX          1000
#define DMUS_TEMPO_MIN          1

#define DMUS_MASTERTEMPO_MAX    100.0f
#define DMUS_MASTERTEMPO_MIN    0.01f

/* DMUS_SYSEX_PMSG */
typedef struct _DMUS_SYSEX_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    DWORD   dwLen;          /* length of the data */
    BYTE    abData[1];      /* array of data, length equal to dwLen */
} DMUS_SYSEX_PMSG;

/* DMUS_CURVE_PMSG */
typedef struct _DMUS_CURVE_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    MUSIC_TIME      mtDuration;      /* how long this curve lasts */
    MUSIC_TIME      mtOriginalStart; /* must be set to either zero when this PMSG is created or to the original mtTime of the curve */
    MUSIC_TIME      mtResetDuration; /* how long after the curve is finished to allow a flush or
                                        invalidation to reset to the reset value, nResetValue */
    short           nStartValue;     /* curve's start value */
    short           nEndValue;       /* curve's end value */
    short           nResetValue;     /* curve's reset value, set when a flush or invalidation
                                        occurs within mtDuration + mtResetDuration */
    WORD            wMeasure;        /* Measure in which this curve occurs */
    short           nOffset;         /* Offset from grid at which this curve occurs */
    BYTE            bBeat;           /* Beat (in measure) at which this curve occurs */
    BYTE            bGrid;           /* Grid offset from beat at which this curve occurs */
    BYTE            bType;           /* type of curve */
    BYTE            bCurveShape;     /* shape of curve */
    BYTE            bCCData;         /* CC# if this is a control change type */
    BYTE            bFlags;          /* Curve reset and start from current value flags. */
    WORD            wParamType;      /* RPN or NRPN parameter number. */
    WORD            wMergeIndex;     /* Allows multiple parameters to be merged (pitchbend, volume, and expression.)*/
} DMUS_CURVE_PMSG;

typedef enum enumDMUS_CURVE_FLAGS
{
    DMUS_CURVE_RESET = 1,            /* When set, the nResetValue must be sent when the 
                                        time is reached or an invalidate occurs because
                                        of a transition. If not set, the curve stays
                                        permanently stuck at the new value. */
    DMUS_CURVE_START_FROM_CURRENT = 2/* Ignore Start, start the curve at the current value. 
                                        This only works for volume, expression, and pitchbend. */
} DMUS_CURVE_FLAGS;

     

/* Curve shapes */
enum
{ 
    DMUS_CURVES_LINEAR  = 0,
    DMUS_CURVES_INSTANT = 1,
    DMUS_CURVES_EXP     = 2,
    DMUS_CURVES_LOG     = 3,
    DMUS_CURVES_SINE    = 4
};
/* curve types */
#define DMUS_CURVET_PBCURVE      0x03   /* Pitch bend curve. */
#define DMUS_CURVET_CCCURVE      0x04   /* Control change curve. */
#define DMUS_CURVET_MATCURVE     0x05   /* Mono aftertouch curve. */
#define DMUS_CURVET_PATCURVE     0x06   /* Poly aftertouch curve. */
#define DMUS_CURVET_RPNCURVE     0x07   /* RPN curve with curve type in wParamType. */
#define DMUS_CURVET_NRPNCURVE    0x08   /* NRPN curve with curve type in wParamType. */

/* DMUS_TIMESIG_PMSG */
typedef struct _DMUS_TIMESIG_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    /* Time signatures define how many beats per measure, which note receives */
    /* the beat, and the grid resolution. */
    BYTE    bBeatsPerMeasure;       /* beats per measure (top of time sig) */
    BYTE    bBeat;                  /* what note receives the beat (bottom of time sig.) */
                                    /* we can assume that 0 means 256th note */
    WORD    wGridsPerBeat;          /* grids per beat */
} DMUS_TIMESIG_PMSG;



/* notification type values */
/* The following correspond to GUID_NOTIFICATION_SEGMENT */
#define DMUS_NOTIFICATION_SEGSTART       0
#define DMUS_NOTIFICATION_SEGEND         1
#define DMUS_NOTIFICATION_SEGALMOSTEND   2
#define DMUS_NOTIFICATION_SEGLOOP        3
#define DMUS_NOTIFICATION_SEGABORT       4
/* The following correspond to GUID_NOTIFICATION_PERFORMANCE */
#define DMUS_NOTIFICATION_MUSICSTARTED   0
#define DMUS_NOTIFICATION_MUSICSTOPPED   1
#define DMUS_NOTIFICATION_MUSICALMOSTEND 2
/* The following corresponds to GUID_NOTIFICATION_MEASUREANDBEAT */
#define DMUS_NOTIFICATION_MEASUREBEAT    0
/* The following corresponds to GUID_NOTIFICATION_CHORD */
#define DMUS_NOTIFICATION_CHORD          0
/* The following correspond to GUID_NOTIFICATION_COMMAND */
#define DMUS_NOTIFICATION_GROOVE         0
#define DMUS_NOTIFICATION_EMBELLISHMENT  1
/* The following corresponds to GUID_NOTIFICATION_RECOMPOSE */
#define DMUS_NOTIFICATION_RECOMPOSE          0

/* DMUS_NOTIFICATION_PMSG */
typedef struct _DMUS_NOTIFICATION_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */

    GUID    guidNotificationType;
    DWORD   dwNotificationOption;
    DWORD   dwField1;
    DWORD   dwField2;
} DMUS_NOTIFICATION_PMSG;

/* DMUS_WAVE_PMSG */
typedef struct _DMUS_WAVE_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */
    
    REFERENCE_TIME rtDuration;      /* Duration of the wave, in either reference time or music time. */  
    long    lOffset;                /* Offset from actual time to logical time, in music or ref time. */
    long    lVolume;                /* Initial volume, in 100ths of a dB. */
    long    lPitch;                 /* Initial pitch, in 100ths of a semitone. */
    DWORD   dwStartOffset;          /* How far into the wave to start, in sample time units. (XBOX only) */
    DWORD   dwLoopStart;            /* Starting loop point. (XBOX only) */
    DWORD   dwLoopEnd;              /* Ending loop point. (XBOX only) */
    BYTE    bFlags;                 /* Flags, including DMUS_WAVEF_OFF... */
} DMUS_WAVE_PMSG;


#define DMUS_READAHEAD_MIN          50         /* Readahead min in milliseconds */

#define DMUS_WAVEF_OFF              0x01       /* If wave is playing and this is the off message. */
#define DMUS_WAVEF_STREAMING        0x02       /* If wave is streaming. */
#define DMUS_WAVEF_NOINVALIDATE     0x04       /* Don't invalidate this wave. */
#define DMUS_WAVEF_NOPREROLL        0x08       /* Don't preroll any wave data. */  
#define DMUS_WAVEF_IGNORELOOPS      0x20       /* Ignore segment looping. */

/* DMUS_LYRIC_PMSG */
typedef struct _DMUS_LYRIC_PMSG
{
    /* begin DMUS_PMSG_PART */
    DMUS_PMSG_PART
    /* end DMUS_PMSG_PART */
    
    WCHAR    wszString[1];      /* null-terminated Unicode lyric string (structure is actually larger than size 1) */
} DMUS_LYRIC_PMSG;

#define DMUS_MAX_NAME           64         /* Maximum object name length. */
#define DMUS_MAX_CATEGORY       64         /* Maximum object category name length. */
#define DMUS_MAX_FILENAME       MAX_PATH

typedef struct _DMUS_VERSION {
  DWORD    dwVersionMS;
  DWORD    dwVersionLS;
}DMUS_VERSION, FAR *LPDMUS_VERSION;

/* Time Signature structure, used by IDirectMusicStyle */
/* Also used as a parameter for GetParam() and SetParam */
typedef struct _DMUS_TIMESIGNATURE
{
    MUSIC_TIME mtTime;
    BYTE    bBeatsPerMeasure;       /* beats per measure (top of time sig) */
    BYTE    bBeat;                  /* what note receives the beat (bottom of time sig.) */
                                    /* we can assume that 0 means 256th note */
    WORD    wGridsPerBeat;          /* grids per beat */
} DMUS_TIMESIGNATURE;

typedef struct _DMUS_RHYTHM_PARAM
{
    DMUS_TIMESIGNATURE  TimeSig;
    DWORD               dwRhythmPattern;
} DMUS_RHYTHM_PARAM;

typedef struct _DMUS_TEMPO_PARAM
{
    MUSIC_TIME  mtTime;
    double      dblTempo;
} DMUS_TEMPO_PARAM;


typedef struct _DMUS_MUTE_PARAM
{
    DWORD   dwPChannel;
    DWORD   dwPChannelMap;
    BOOL    fMute;
} DMUS_MUTE_PARAM;

typedef struct _DMUS_VALID_START_PARAM
{
    MUSIC_TIME mtTime;                      /* Time of the first legal start 
                                               point after (or including) the requested time. 
                                               This is a returned value.
                                               Time format is the relative offset from requested time. */
} DMUS_VALID_START_PARAM;

typedef struct _DMUS_PLAY_MARKER_PARAM
{
    MUSIC_TIME mtTime;                      /* Time of the first legal segment play 
                                               marker before (or including) the requested time. 
                                               This is a returned value.
                                               Time format is the relative offset from requested time. */
} DMUS_PLAY_MARKER_PARAM;

/*      The DMUSOBJECTDESC structure is used to communicate everything you could */
/*      possibly use to describe a DirectMusic object.  */

typedef struct _DMUS_OBJECTDESC
{
    DWORD          dwSize;                 /* Size of this structure. */
    DWORD          dwValidData;            /* Flags indicating which fields below are valid. */
    GUID           guidObject;             /* Unique ID for this object. */
    GUID           guidClass;              /* GUID for the class of object. */
    FILETIME       ftDate;                 /* Last edited date of object. */
    DMUS_VERSION   vVersion;               /* Version. */
    WCHAR          wszName[DMUS_MAX_NAME]; /* Name of object. */
    WCHAR          wszCategory[DMUS_MAX_CATEGORY]; /* Category for object (optional). */
    WCHAR          wszFileName[DMUS_MAX_FILENAME]; /* File path. */
    LONGLONG       llMemLength;            /* Size of Memory data. */
    LPBYTE         pbMemData;              /* Memory pointer for data. */
    IStream *      pStream;                /* Stream with data. */
} DMUS_OBJECTDESC;

typedef DMUS_OBJECTDESC *LPDMUS_OBJECTDESC;

/*      Flags for dwValidData. When set, a flag indicates that the  */
/*      corresponding field in DMUSOBJECTDESC holds valid data. */

#define DMUS_OBJ_OBJECT         (1 << 0)     /* Object GUID is valid. */
#define DMUS_OBJ_CLASS          (1 << 1)     /* Class GUID is valid. */
#define DMUS_OBJ_NAME           (1 << 2)     /* Name is valid. */
#define DMUS_OBJ_CATEGORY       (1 << 3)     /* Category is valid. */
#define DMUS_OBJ_FILENAME       (1 << 4)     /* File path is valid. */
#define DMUS_OBJ_FULLPATH       (1 << 5)     /* Path is full path. */
#define DMUS_OBJ_URL            (1 << 6)     /* Path is URL. */
#define DMUS_OBJ_VERSION        (1 << 7)     /* Version is valid. */
#define DMUS_OBJ_DATE           (1 << 8)     /* Date is valid. */
#define DMUS_OBJ_LOADED         (1 << 9)     /* Object is currently loaded in memory. */
#define DMUS_OBJ_MEMORY         (1 << 10)    /* Object is pointed to by pbMemData. */
#define DMUS_OBJ_STREAM         (1 << 11)    /* Object is stored in pStream. */

/*      The DMUS_SCRIPT_ERRORINFO structure describes an error that occurred in a script.
        It is returned by methods in IDirectMusicScript. */
typedef struct _DMUS_SCRIPT_ERRORINFO
{
    DWORD dwSize; /* Size of this structure. */
    HRESULT hr;
    ULONG ulLineNumber;
    LONG ichCharPosition;
    CHAR wszSourceFile[DMUS_MAX_FILENAME];
    CHAR wszSourceComponent[DMUS_MAX_FILENAME];
    CHAR wszDescription[DMUS_MAX_FILENAME];
    CHAR wszSourceLineText[DMUS_MAX_FILENAME];
} DMUS_SCRIPT_ERRORINFO;


typedef struct _DMUS_COMMAND_PARAM_2
{
    MUSIC_TIME mtTime;
    BYTE bCommand;
    BYTE bGrooveLevel;
    BYTE bGrooveRange;
    BYTE bRepeatMode;
} DMUS_COMMAND_PARAM_2;


/*////////////////////////////////////////////////////////////////////
// IDirectMusicObject */
#undef  INTERFACE
#define INTERFACE  IDirectMusicObject
DECLARE_INTERFACE_(IDirectMusicObject, IUnknown)
{
    /* IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /* IDirectMusicObject */
    STDMETHOD(GetDescriptor)        (THIS_ LPDMUS_OBJECTDESC pDesc) PURE;
    STDMETHOD(SetDescriptor)        (THIS_ LPDMUS_OBJECTDESC pDesc) PURE;
    STDMETHOD(ParseDescriptor)      (THIS_ LPSTREAM pStream, 
                                           LPDMUS_OBJECTDESC pDesc) PURE;
};

typedef IDirectMusicObject IDirectMusicObject8;

/*////////////////////////////////////////////////////////////////////
// IDirectMusicLoader */
#undef  INTERFACE
#define INTERFACE  IDirectMusicLoader
DECLARE_INTERFACE_(IDirectMusicLoader, IUnknown)
{
    /* IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /* IDirectMusicLoader */
    /* IDirectMusicLoader */
    STDMETHOD(GetObject)            (THIS_ LPDMUS_OBJECTDESC pDesc,
                                           REFIID riid,
                                           LPVOID FAR *ppv) PURE;
    STDMETHOD(SetObject)            (THIS_ LPDMUS_OBJECTDESC pDesc) PURE;
    STDMETHOD(SetSearchDirectory)   (THIS_ REFGUID rguidClass, 
                                           const char *pzPath, 
                                           BOOL fClear) PURE;
    STDMETHOD(ClearCache)           (THIS_ REFGUID rguidClass) PURE;
    STDMETHOD_(void, CollectGarbage)                (THIS) PURE;
    STDMETHOD(ReleaseObjectByUnknown)               (THIS_ IUnknown *pObject) PURE;
    STDMETHOD(LoadObjectFromFile)                   (THIS_ REFGUID rguidClassID, 
                                                           REFIID iidInterfaceID, 
                                                           const char *pzFilePath, 
                                                           void ** ppObject) PURE;
};                                  

typedef IDirectMusicLoader IDirectMusicLoader8;

/*  Stream objects must support IDirectMusicGetLoader interface to access loader while file parsing. 
    If you write your own loader, you will need to add this interface to the stream object.
*/

#undef  INTERFACE
#define INTERFACE  IDirectMusicGetLoader
DECLARE_INTERFACE_(IDirectMusicGetLoader, IUnknown)
{
    /* IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /* IDirectMusicGetLoader */
    STDMETHOD(GetLoader)            (THIS_ IDirectMusicLoader ** ppLoader) PURE;
};

typedef IDirectMusicGetLoader IDirectMusicGetLoader8;

/*////////////////////////////////////////////////////////////////////
// IDirectMusicSegment */
#undef  INTERFACE
#define INTERFACE  IDirectMusicSegment
DECLARE_INTERFACE_(IDirectMusicSegment, IUnknown)
{
    /*  IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /*  IDirectMusicSegment */
    STDMETHOD(SetRepeats)               (THIS_ DWORD  dwRepeats) PURE;
    STDMETHOD(Compose)                  (THIS_ MUSIC_TIME mtTime,
                                               IDirectMusicSegment* pFromSegment,
                                               IDirectMusicSegment* pToSegment,
                                               IDirectMusicSegment** ppComposedSegment) PURE;
    STDMETHOD(Download)                 (THIS_ IUnknown *pAudioPath) PURE;
    STDMETHOD(Unload)                   (THIS_ IUnknown *pAudioPath) PURE;
    STDMETHOD(GetLength)                (THIS_ MUSIC_TIME* pmtLength) PURE;
    STDMETHOD(SetLength)                (THIS_ MUSIC_TIME mtLength) PURE;
    STDMETHOD(SetClockTimeLength)       (THIS_ REFERENCE_TIME rtLength, BOOL fClockTime) PURE;
    STDMETHOD(GetClockTimeLength)       (THIS_ REFERENCE_TIME *prtLength, BOOL *pfClockTime) PURE;
    STDMETHOD(SetLoopPoints)            (THIS_ MUSIC_TIME mtStart, 
                                               MUSIC_TIME mtEnd) PURE;
    STDMETHOD(GetLoopPoints)            (THIS_ MUSIC_TIME* pmtStart, 
                                               MUSIC_TIME* pmtEnd) PURE;
    STDMETHOD(SetStartPoint)            (THIS_ MUSIC_TIME mtStart) PURE;
    STDMETHOD(GetStartPoint)            (THIS_ MUSIC_TIME* pmtStart) PURE;
    STDMETHOD(SetWavePlaybackParams)    (THIS_ DWORD dwFlags, DWORD dwReadAhead) PURE;
    STDMETHOD(AddNotificationType)      (THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(RemoveNotificationType)   (THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(GetRepeats)               (THIS_ DWORD* pdwRepeats) PURE;

};

typedef IDirectMusicSegment IDirectMusicSegment8;


/*/////////////////////////////////////////////////////////////////////
// IDirectMusicSegmentState */
#undef  INTERFACE
#define INTERFACE  IDirectMusicSegmentState
DECLARE_INTERFACE_(IDirectMusicSegmentState, IUnknown)
{
    /*  IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /*  IDirectMusicSegmentState */
    STDMETHOD(GetObjectInPath)      (THIS_ DWORD dwPChannel,    /* PChannel to search. */
                                           DWORD dwStage,       /* Which stage in the path. */
                                           DWORD dwBuffer,      /* Which buffer to address, if more than one. */
                                           REFGUID guidObject,  /* ClassID of object. */
                                           DWORD dwIndex,       /* Which object of that class. */
                                           REFGUID iidInterface,/* Requested COM interface. */
                                           void ** ppObject) PURE; /* Pointer to interface. */
    STDMETHOD(SetVolume)            (THIS_ long lVolume,        /* Gain, in 100ths of a dB. This must be negative (0 represents full volume.) */
                                           DWORD dwDuration) PURE;/* Duration of volume ramp in  milliseconds. Note that 0 is more efficient. */
    STDMETHOD(SetPitch)             (THIS_ long lPitch,         /* Pitch bend, in 100ths of a semitone. */
                                           DWORD dwDuration) PURE;/* Duration of pitch ramp in  milliseconds. Note that 0 is more efficient. */
    STDMETHOD(GetSegment )          (THIS_ IDirectMusicSegment** ppSegment) PURE;
    STDMETHOD(GetStartTime)         (THIS_ MUSIC_TIME* pmtStart) PURE;
};

typedef IDirectMusicSegmentState IDirectMusicSegmentState8;

/*////////////////////////////////////////////////////////////////////
// IDirectMusicAudioPath */
#undef  INTERFACE
#define INTERFACE  IDirectMusicAudioPath
DECLARE_INTERFACE_(IDirectMusicAudioPath, IUnknown)
{
    /*  IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /*  IDirectMusicAudioPath */
    STDMETHOD(GetObjectInPath)      (THIS_ DWORD dwPChannel,    /* PChannel to search. */
                                           DWORD dwStage,       /* Which stage in the path. */
                                           DWORD dwBuffer,      /* Which buffer to address, if more than one. */
                                           REFGUID guidObject,  /* ClassID of object. */
                                           DWORD dwIndex,       /* Which object of that class. */
                                           REFGUID iidInterface,/* Requested COM interface. */
                                           void ** ppObject) PURE; /* Pointer to interface. */
    STDMETHOD(Activate)             (THIS_ BOOL fActivate) PURE;/* True to activate, False to deactivate. */
    STDMETHOD(SetVolume)            (THIS_ long lVolume,        /* Gain, in 100ths of a dB. This must be negative (0 represents full volume.) */
                                           DWORD dwDuration) PURE;/* Duration of volume ramp in  milliseconds. Note that 0 is more efficient. */
    STDMETHOD(SetPitch)             (THIS_ long lPitch,         /* Pitch bend, in 100ths of a semitone. */
                                           DWORD dwDuration) PURE;/* Duration of pitch ramp in  milliseconds. Note that 0 is more efficient. */
};

typedef IDirectMusicAudioPath IDirectMusicAudioPath8;


/*////////////////////////////////////////////////////////////////////
// IDirectMusicPerformance */

#undef  INTERFACE
#define INTERFACE  IDirectMusicPerformance
DECLARE_INTERFACE_(IDirectMusicPerformance, IUnknown)
{
    /*  IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /*  IDirectMusicPerformance */
    STDMETHOD(GetSegmentState)      (THIS_ IDirectMusicSegmentState** ppSegmentState, 
                                           MUSIC_TIME mtTime) PURE;
    STDMETHOD(SendPMsg)             (THIS_ DMUS_PMSG* pPMSG) PURE;
    STDMETHOD(MusicToReferenceTime) (THIS_ MUSIC_TIME mtTime, 
                                           REFERENCE_TIME* prtTime) PURE;
    STDMETHOD(ReferenceToMusicTime) (THIS_ REFERENCE_TIME rtTime, 
                                           MUSIC_TIME* pmtTime) PURE;
    STDMETHOD(IsPlaying)            (THIS_ IDirectMusicSegment* pSegment, 
                                           IDirectMusicSegmentState* pSegState) PURE;
    STDMETHOD(GetTime)              (THIS_ REFERENCE_TIME* prtNow, 
                                           MUSIC_TIME* pmtNow) PURE;
    STDMETHOD(AllocPMsg)            (THIS_ ULONG cb, 
                                           DMUS_PMSG** ppPMSG) PURE;
    STDMETHOD(FreePMsg)             (THIS_ DMUS_PMSG* pPMSG) PURE;
    STDMETHOD(GetNotificationPMsg)  (THIS_ DMUS_NOTIFICATION_PMSG** ppNotificationPMsg) PURE;
    STDMETHOD(AddNotificationType)  (THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(RemoveNotificationType)(THIS_ REFGUID rguidNotificationType) PURE;
    STDMETHOD(GetGlobalParam)       (THIS_ REFGUID rguidType, 
                                           void* pParam, 
                                           DWORD dwSize) PURE;
    STDMETHOD(SetGlobalParam)       (THIS_ REFGUID rguidType, 
                                           void* pParam, 
                                           DWORD dwSize) PURE;
    STDMETHOD(PlaySegmentEx)        (THIS_ IUnknown* pSource,                       /* Segment to play. Alternately, could be an IDirectMusicSong  */
                                           const char *pzSegmentName,                     /* If song, which segment in the song  */
                                           IUnknown* pTransition,                   /* Optional template segment to compose transition with. */
                                           DWORD dwFlags,                           /* DMUS_SEGF_ flags. */ 
                                           __int64 i64StartTime,                    /* Time to start playback. */
                                           IDirectMusicSegmentState** ppSegmentState, /* Returned Segment State. */
                                           IUnknown *pFrom,                         /* Optional segmentstate or audiopath to replace. */
                                           IUnknown *pAudioPath) PURE;              /* Optional audioPath to play on. */
    STDMETHOD(StopEx)               (THIS_ IUnknown *pObjectToStop,                 /* Segstate, AudioPath, Segment, or Song. */ 
                                           __int64 i64StopTime, 
                                           DWORD dwFlags) PURE;
    STDMETHOD(ClonePMsg)            (THIS_ DMUS_PMSG* pSourcePMSG,
                                           DMUS_PMSG** ppCopyPMSG) PURE;
    STDMETHOD(CreateAudioPath)      (THIS_ IUnknown *pSourceConfig,                 /* Source configuration, from AudioPathConfig file. */
                                           BOOL fActivate,                          /* TRUE to activate on creation. */
                                           IDirectMusicAudioPath **ppNewPath) PURE; /* Returns created audiopath. */                                           
    STDMETHOD(CreateStandardAudioPath)(THIS_ DWORD dwType,                          /* Type of path to create. */
                                           DWORD dwPChannelCount,                   /* How many PChannels to allocate for it. */
                                           BOOL fActivate,                          /* TRUE to activate on creation. */
                                           IDirectMusicAudioPath **ppNewPath) PURE; /* Returns created audiopath. */
    STDMETHOD(SetDefaultAudioPath)  (THIS_ IDirectMusicAudioPath *pAudioPath) PURE;
    STDMETHOD(GetDefaultAudioPath)  (THIS_ IDirectMusicAudioPath **ppAudioPath) PURE;
    STDMETHOD(GetParamEx)           (THIS_ REFGUID rguidType,                       /* GetParam command ID. */
                                           DWORD dwTrackID,                         /* Virtual track ID of caller. */
                                           DWORD dwGroupBits,                       /* Group bits of caller. */
                                           DWORD dwIndex,                           /* Index to Nth parameter. */
                                           MUSIC_TIME mtTime,                       /* Time of requested parameter. */
                                           MUSIC_TIME* pmtNext,                     /* Returned delta to next parameter. */
                                           void* pParam) PURE;                      /* Data structure to fill with parameter. */
    STDMETHOD(InitAudioX)           (THIS_ DWORD dwDefaultPathType,                 /* Requested default audio path type, also optional. */
                                           DWORD dwPChannelCount,                   /* Number of PChannels, if default audio path to be created. */
                                           DWORD dwVoiceCount,                      /* Number of Voices (DSoundBuffers) allocated by synth. */
                                           DWORD dwFlags) PURE;                 
    STDMETHOD(CloseDown)            (THIS) PURE;
    STDMETHOD(Invalidate)           (THIS_ MUSIC_TIME mtTime, 
                                           DWORD dwFlags) PURE;
};

typedef IDirectMusicPerformance IDirectMusicPerformance8;


/*////////////////////////////////////////////////////////////////////
// IDirectMusicGraph */
#undef  INTERFACE
#define INTERFACE  IDirectMusicGraph
DECLARE_INTERFACE_(IDirectMusicGraph, IUnknown)
{
    /*  IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /*  IDirectMusicGraph */
    STDMETHOD(StampPMsg)            (THIS_ DMUS_PMSG* pPMSG) PURE;
    STDMETHOD(InsertTool)           (THIS_ IDirectMusicTool* pTool, 
                                           DWORD* pdwPChannels, 
                                           DWORD cPChannels, 
                                           LONG lIndex) PURE;
    STDMETHOD(GetTool)              (THIS_ DWORD dwIndex, 
                                           IDirectMusicTool** ppTool) PURE;
    STDMETHOD(RemoveTool)           (THIS_ IDirectMusicTool* pTool) PURE;
};

typedef IDirectMusicGraph IDirectMusicGraph8;

/*////////////////////////////////////////////////////////////////////
// IDirectMusicTool */
#undef  INTERFACE
#define INTERFACE  IDirectMusicTool
DECLARE_INTERFACE_(IDirectMusicTool, IUnknown)
{
    /*  IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /*  IDirectMusicTool */
    STDMETHOD(Init)                 (THIS_ IDirectMusicGraph* pGraph) PURE;
    STDMETHOD(GetMsgDeliveryType)   (THIS_ DWORD* pdwDeliveryType ) PURE;
    STDMETHOD(GetMediaTypeArraySize)(THIS_ DWORD* pdwNumElements ) PURE;
    STDMETHOD(GetMediaTypes)        (THIS_ DWORD** padwMediaTypes, 
                                           DWORD dwNumElements) PURE;
    STDMETHOD(ProcessPMsg)          (THIS_ IDirectMusicPerformance* pPerf, 
                                           DMUS_PMSG* pPMSG) PURE;
    STDMETHOD(Flush)                (THIS_ IDirectMusicPerformance* pPerf, 
                                           DMUS_PMSG* pPMSG, 
                                           REFERENCE_TIME rtTime) PURE;
};


/*/////////////////////////////////////////////////////////////////////
// IDirectMusicScript */

#undef  INTERFACE
#define INTERFACE  IDirectMusicScript
DECLARE_INTERFACE_(IDirectMusicScript, IUnknown)
{
    /*  IUnknown */
    STDMETHOD(QueryInterface)       (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)        (THIS) PURE;
    STDMETHOD_(ULONG,Release)       (THIS) PURE;

    /*  IDirectMusicScript */
    STDMETHOD(Init)                     (THIS_ IDirectMusicPerformance *pPerformance,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(CallRoutine)              (THIS_ const char *pszRoutineName,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(SetVariableNumber)        (THIS_ const char *pszVariableName,
                                               LONG lValue,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(GetVariableNumber)        (THIS_ const char *pszVariableName,
                                               LONG *plValue,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(SetVariableObject)        (THIS_ const char *pszVariableName,
                                               IUnknown *punkValue,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(GetVariableObject)        (THIS_ const char *pszVariableName,
                                               REFIID riid,
                                               LPVOID FAR *ppv,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(SetVariableString)        (THIS_ const char *pszVariableName,
                                               const char * pszValue,
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
    STDMETHOD(GetVariableString)        (THIS_ const char *pszVariableName,
                                               char *pszValue,
                                               LONG lLength,
                                               LONG *plConverted, 
                                               DMUS_SCRIPT_ERRORINFO *pErrorInfo) PURE;
};

typedef IDirectMusicScript IDirectMusicScript8;


/* CLSID's */
DEFINE_GUID(CLSID_DirectMusicPerformance,0xd2ac2881, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicSegment,0xd2ac2882, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicLoader,0xd2ac2892, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicCollection,0x480ff4b0, 0x28b2, 0x11d1, 0xbe, 0xf7, 0x0, 0xc0, 0x4f, 0xbf, 0x8f, 0xef);
DEFINE_GUID(CLSID_DirectMusicSegmentState,0xd2ac2883, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicGraph,0xd2ac2884, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicStyle,0xd2ac288a, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicChordMap,0xd2ac288f, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicComposer,0xd2ac2890, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectMusicScript,0x810b5013, 0xe88d, 0x11d2, 0x8b, 0xc1, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xb6); /* {810B5013-E88D-11d2-8BC1-00600893B1B6} */
DEFINE_GUID(CLSID_DirectMusicContainer,0x9301e380, 0x1f22, 0x11d3, 0x82, 0x26, 0xd2, 0xfa, 0x76, 0x25, 0x5d, 0x47);
DEFINE_GUID(CLSID_DirectMusicAudioPathConfig,0xee0b9ca0, 0xa81e, 0x11d3, 0x9b, 0xd1, 0x0, 0x80, 0xc7, 0x15, 0xa, 0x74);
DEFINE_GUID(CLSID_DirectMusicPatternTrack,0xd2ac2897, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(CLSID_DirectSoundWave,0x8a667154, 0xf9cb, 0x11d2, 0xad, 0x8a, 0x0, 0x60, 0xb0, 0x57, 0x5a, 0xbc);
DEFINE_GUID(CLSID_DirectMusicSong, 0xaed5f0a5, 0xd972, 0x483d, 0xa3, 0x84, 0x64, 0x9d, 0xfe, 0xb9, 0xc1, 0x81);
DEFINE_GUID(CLSID_DirectMusicStreamStream, 0xf34feac1, 0xe3af, 0x49ad, 0x83, 0x97, 0xb, 0xed, 0x32, 0x3e, 0xf9, 0x6b);
DEFINE_GUID(CLSID_DirectMusicMemStream, 0x75ccb447, 0x8d3f, 0x4154, 0xab, 0xad, 0x59, 0x60, 0xae, 0xd4, 0xba, 0x63);
DEFINE_GUID(CLSID_DirectMusicFileStream, 0xf12f2c7d, 0x3651, 0x486f, 0xb9, 0xfa, 0x16, 0xe1, 0x1d, 0x15, 0x24, 0xfd);

/* Special GUID for all object types. This is used by the loader. */
DEFINE_GUID(GUID_DirectMusicAllTypes,0xd2ac2893, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

/* Notification guids */
DEFINE_GUID(GUID_NOTIFICATION_SEGMENT,0xd2ac2899, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_PERFORMANCE,0x81f75bc5, 0x4e5d, 0x11d2, 0xbc, 0xc7, 0x0, 0xa0, 0xc9, 0x22, 0xe6, 0xeb);
DEFINE_GUID(GUID_NOTIFICATION_MEASUREANDBEAT,0xd2ac289a, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_CHORD,0xd2ac289b, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_COMMAND,0xd2ac289c, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_NOTIFICATION_RECOMPOSE, 0xd348372b, 0x945b, 0x45ae, 0xa5, 0x22, 0x45, 0xf, 0x12, 0x5b, 0x84, 0xa5);

/* Track param type guids */

/* Use to get a DMUS_COMMAND_PARAM_2 param in the Command track */
DEFINE_GUID(GUID_CommandParam2, 0x28f97ef7, 0x9538, 0x11d2, 0x97, 0xa9, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

/* Use to get/set a DMUS_COMMAND_PARAM_2 param to be used as the command following all commands in
the Command track (this information can't be saved) */
DEFINE_GUID(GUID_CommandParamNext, 0x472afe7a, 0x281b, 0x11d3, 0x81, 0x7d, 0x0, 0xc0, 0x4f, 0xa3, 0x6e, 0x58);

/* Use to get/set a DMUS_CHORD_PARAM param in the Chord track */
DEFINE_GUID(GUID_ChordParam,0xd2ac289e, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

/* Use to get a DMUS_RHYTHM_PARAM param in the Chord track */
DEFINE_GUID(GUID_RhythmParam,0xd2ac289f, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

/* Use to get/set an IDirectMusicStyle param in the Style track */
DEFINE_GUID(GUID_IDirectMusicStyle,0xd2ac28a1, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

/* Use to get a DMUS_TIMESIGNATURE param in the Style and TimeSig tracks */
DEFINE_GUID(GUID_TimeSignature,0xd2ac28a4, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

/* Use to get/set a DMUS_TEMPO_PARAM param in the Tempo track */
DEFINE_GUID(GUID_TempoParam,0xd2ac28a5, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

/* Use to get the next valid point in a segment at which it may start */
DEFINE_GUID(GUID_Valid_Start_Time,0x7f6b1760, 0x1fdb, 0x11d3, 0x82, 0x26, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0);

/* Use to get the next point in the currently playing primary segment at which a new segment may start */
DEFINE_GUID(GUID_Play_Marker,0xd8761a41, 0x801a, 0x11d3, 0x9b, 0xd1, 0xda, 0xf7, 0xe1, 0xc3, 0xd8, 0x34);


/* Use to get/set a DMUS_MUTE_PARAM param in the Mute track */
DEFINE_GUID(GUID_MuteParam,0xd2ac28af, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);



/* Global data guids */
DEFINE_GUID(GUID_PerfMasterTempo,0xd2ac28b0, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_PerfMasterVolume,0xd2ac28b1, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);
DEFINE_GUID(GUID_PerfMasterGrooveLevel,0xd2ac28b2, 0xb39b, 0x11d1, 0x87, 0x4, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);

/* GUID for default GM/GS dls collection. */
DEFINE_GUID(GUID_DefaultGMCollection, 0xf17e8673, 0xc3b4, 0x11d1, 0x87, 0xb, 0x0, 0x60, 0x8, 0x93, 0xb1, 0xbd);


/* IID's */
DEFINE_GUID(IID_IDirectMusicLoader,0x4fdad3f6, 0xe642, 0x4a1b, 0x90, 0x28, 0x1c, 0x2f, 0xfd, 0x91, 0x5c, 0x2a);
DEFINE_GUID(IID_IDirectMusicSegment, 0x3fc8898, 0xac24, 0x4bb8, 0xaf, 0x2f, 0x6f, 0xba, 0xb6, 0x40, 0x8a, 0x8e);
DEFINE_GUID(IID_IDirectMusicSegmentState, 0xfbdf2f1d, 0x6378, 0x43ba, 0x97, 0x29, 0x4b, 0x74, 0xb1, 0xdb, 0x3b, 0xd5);
DEFINE_GUID(IID_IDirectMusicPerformance,0x37a8aa56, 0x79fd, 0x4fcc, 0x8b, 0x58, 0xd3, 0x9d, 0x75, 0x86, 0x1f, 0x3);
DEFINE_GUID(IID_IDirectMusicGraph,0x5ae1e2a9, 0x38d7, 0x42a2, 0x9d, 0x31, 0xa1, 0x9c, 0x9a, 0x93, 0x6a, 0x4a);
DEFINE_GUID(IID_IDirectMusicGetLoader,0xb0e1656f, 0x3e45, 0x418e, 0x9b, 0x2d, 0x34, 0xd5, 0x33, 0xd1, 0x77, 0xe1);
DEFINE_GUID(IID_IDirectMusicObject,0x632aee51, 0xb9d, 0x4ea4, 0x9b, 0x60, 0x23, 0xcc, 0x58, 0xf5, 0x56, 0x1e);
DEFINE_GUID(IID_IDirectMusicTool,0xe59eeefe, 0x7a62, 0x4ca7, 0x8b, 0x47, 0x1f, 0xdd, 0x72, 0x8f, 0xba, 0x57);
DEFINE_GUID(IID_IDirectMusicScript, 0x801413c2, 0x392, 0x4265, 0xb3, 0x1a, 0x13, 0xd, 0x7, 0xdf, 0x31, 0xd0);
#define IID_IDirectMusicScript8 IID_IDirectMusicScript
DEFINE_GUID(IID_IDirectMusicAudioPath,0x242ed927, 0xf094, 0x42e4, 0x9b, 0xb9, 0x52, 0xd2, 0x14, 0x19, 0x94, 0x4a);
#define IID_IDirectMusicAudioPath8 IID_IDirectMusicAudioPath
/* unchanged interfaces (alias only) */
#define IID_IDirectMusicGraph8 IID_IDirectMusicGraph
#define IID_IDirectMusicLoader8 IID_IDirectMusicLoader
#define IID_IDirectMusicPerformance8 IID_IDirectMusicPerformance
#define IID_IDirectMusicSegment8 IID_IDirectMusicSegment
#define IID_IDirectMusicSegmentState8 IID_IDirectMusicSegmentState

// Special GUID meaning "select all objects" for use in GetObjectInPath()
DEFINE_GUID(GUID_All_Objects, 0xaa114de5, 0xc262, 0x4169, 0xa1, 0xc8, 0x23, 0xd6, 0x98, 0xcc, 0x73, 0xb5);

HRESULT WINAPI DirectMusicCreateInstance(REFCLSID clsid, LPUNKNOWN pUnkOuter, REFIID iid, LPVOID *ppvInterface);
void WINAPI DirectMusicDoWork(DWORD dwQuantum);

typedef HRESULT (CALLBACK* LPDIRECTMUSICFACTORYFN)(REFCLSID clsid, LPUNKNOWN pUnkOuter, REFIID iid, LPVOID *ppvInterface);
HRESULT CALLBACK DirectMusicDefaultFactory(REFCLSID clsid, LPUNKNOWN pUnkOuter, REFIID iid, LPVOID *ppvInterface);

#undef  INTERFACE
#define INTERFACE  IDirectMusicHeap
DECLARE_INTERFACE_(IDirectMusicHeap, IUnknown)
{
    /*  IUnknown */
    STDMETHOD(QueryInterface)               (THIS_ REFIID, LPVOID FAR *) PURE;
    STDMETHOD_(ULONG,AddRef)                (THIS) PURE;
    STDMETHOD_(ULONG,Release)               (THIS) PURE;

    /*  IDirectMusicHeap */
    STDMETHOD(Allocate)   (THIS_ DWORD cbSize, PVOID* pData) PURE;
    STDMETHOD(GetSize) (THIS_ PVOID pData, LPDWORD pcbSize) PURE;
    STDMETHOD(Free)   (THIS_ PVOID pData) PURE;
};

void* WINAPI DirectMusicAlloc(size_t cb);
void  WINAPI DirectMusicFree(void *pv);
void* WINAPI DirectMusicPhysicalAlloc(size_t dwSize);
void WINAPI DirectMusicPhysicalFree(void* lpAddress);

HRESULT WINAPI DirectMusicCreateDefaultHeap(IDirectMusicHeap** ppHeap);
HRESULT WINAPI DirectMusicCreateDefaultPhysicalHeap(IDirectMusicHeap** ppHeap);

HRESULT WINAPI DirectMusicCreateFixedSizeHeap(DWORD cbSize, IDirectMusicHeap** ppHeap);
HRESULT WINAPI DirectMusicCreateFixedSizePhysicalHeap(DWORD cbSize, IDirectMusicHeap** ppHeap);

HRESULT WINAPI DirectMusicInitialize();
HRESULT WINAPI DirectMusicInitializeEx(IDirectMusicHeap* pNormalHeap, 
    IDirectMusicHeap* pPhysicalHeap,
    LPDIRECTMUSICFACTORYFN pFactory);
HRESULT WINAPI DirectMusicInitializeFixedSizeHeaps(DWORD cbNormalHeapSize, 
    DWORD cbPhysicalHeapSize,
    LPDIRECTMUSICFACTORYFN pFactory);

#ifdef __cplusplus
}; /* extern "C" */
#endif

#include <poppack.h>

long WINAPI DirectMusicMemCheck(DWORD dwMemType, char **ppName);
void WINAPI DirectMusicMemDump();
void WINAPI DirectMusicSetDebugLevel(int iDebugLevel, int iRIPLevel);

#endif /* #ifndef _DMUSICI_ */

/************************************************************************
*                                                                       *
*   dmerror.h -- Error code returned by DirectMusic API's               *
*                                                                       *
*   Copyright (c) 1998-1999 Microsoft Corporation
*                                                                       *
************************************************************************/

#ifndef _DMERROR_
#define _DMERROR_

#define FACILITY_DIRECTMUSIC      0x878       /* Shared with DirectSound */
#define DMUS_ERRBASE              0x1000      /* Make error codes human readable in hex */

#ifndef MAKE_HRESULT
#define MAKE_HRESULT(sev,fac,code) \
    ((HRESULT) (((unsigned long)(sev)<<31) | ((unsigned long)(fac)<<16) | ((unsigned long)(code))) )
#endif
    
#define MAKE_DMHRESULTSUCCESS(code)     MAKE_HRESULT(0, FACILITY_DIRECTMUSIC, (DMUS_ERRBASE + (code)))
#define MAKE_DMHRESULTERROR(code)       MAKE_HRESULT(1, FACILITY_DIRECTMUSIC, (DMUS_ERRBASE + (code)))

/* DMUS_S_REQUEUE
 *
 * Return value from IDirectMusicTool::ProcessPMsg() which indicates to the
 * performance that it should cue the PMsg again automatically.
 */
#define DMUS_S_REQUEUE                  MAKE_DMHRESULTSUCCESS(0x200)

/* DMUS_S_FREE
 *
 * Return value from IDirectMusicTool::ProcessPMsg() which indicates to the
 * performance that it should free the PMsg automatically.
 */
#define DMUS_S_FREE                     MAKE_DMHRESULTSUCCESS(0x201)

/* DMUS_S_END
 *
 * Return value from IDirectMusicTrack::Play() which indicates to the
 * segment that the track has no more data after mtEnd.
 */
#define DMUS_S_END                      MAKE_DMHRESULTSUCCESS(0x202)

/* DMUS_S_LAST_TOOL
 *
 * Returned from IDirectMusicGraph::StampPMsg(), this indicates that the PMsg
 * is already stamped with the last tool in the graph. The returned PMsg's
 * tool pointer is now NULL.
 */
#define DMUS_S_LAST_TOOL                MAKE_DMHRESULTSUCCESS(0x211)

/* DMUS_S_OVER_CHORD
 *
 * Returned from IDirectMusicPerformance::MusicToMIDI(), this indicates 
 * that no note has been calculated because the music value has the note 
 * at a position higher than the top note of the chord. This applies only
 * to DMUS_PLAYMODE_NORMALCHORD play mode. This success code indicates
 * that the caller should not do anything with the note. It is not meant
 * to be played against this chord.
 */
#define DMUS_S_OVER_CHORD               MAKE_DMHRESULTSUCCESS(0x212)

/* DMUS_S_UP_OCTAVE
 *
 * Returned from IDirectMusicPerformance::MIDIToMusic(),  and
 * IDirectMusicPerformance::MusicToMIDI(), this indicates 
 * that the note conversion generated a note value that is below 0, 
 * so it has been bumped up one or more octaves to be in the proper
 * MIDI range of 0 through 127. 
 * Note that this is valid for MIDIToMusic() when using play modes
 * DMUS_PLAYMODE_FIXEDTOCHORD and DMUS_PLAYMODE_FIXEDTOKEY, both of
 * which store MIDI values in wMusicValue. With MusicToMIDI(), it is
 * valid for all play modes.
 * Ofcourse, DMUS_PLAYMODE_FIXED will never return this success code.
 */
#define DMUS_S_UP_OCTAVE                MAKE_DMHRESULTSUCCESS(0x213)

/* DMUS_S_DOWN_OCTAVE
 *
 * Returned from IDirectMusicPerformance::MIDIToMusic(),  and
 * IDirectMusicPerformance::MusicToMIDI(), this indicates 
 * that the note conversion generated a note value that is above 127, 
 * so it has been bumped down one or more octaves to be in the proper
 * MIDI range of 0 through 127. 
 * Note that this is valid for MIDIToMusic() when using play modes
 * DMUS_PLAYMODE_FIXEDTOCHORD and DMUS_PLAYMODE_FIXEDTOKEY, both of
 * which store MIDI values in wMusicValue. With MusicToMIDI(), it is
 * valid for all play modes.
 * Ofcourse, DMUS_PLAYMODE_FIXED will never return this success code.
 */
#define DMUS_S_DOWN_OCTAVE              MAKE_DMHRESULTSUCCESS(0x214)


/* DMUS_S_GARBAGE_COLLECTED
 *
 * The requested operation was not performed because during CollectGarbage
 * the loader determined that the object had been released.
 */
#define DMUS_S_GARBAGE_COLLECTED        MAKE_DMHRESULTSUCCESS(0x216)


/* DMUS_E_SET_UNSUPPORTED
 *
 * The specified property item may not be set on the target object.
 */
#define DMUS_E_SET_UNSUPPORTED          MAKE_DMHRESULTERROR(0x0123)

/* DMUS_E_GET_UNSUPPORTED
 *
 * The specified property item may not be retrieved from the target object.
 */ 
#define DMUS_E_GET_UNSUPPORTED          MAKE_DMHRESULTERROR(0x0124)


/* DMUS_E_NOT_FOUND
 *
 * The requested item was not contained by the object.
 */
#define DMUS_E_NOT_FOUND                MAKE_DMHRESULTERROR(0x0161)

/* DMUS_E_NOT_INIT
 *
 * A required object is not initialized or failed to initialize.
 */
#define DMUS_E_NOT_INIT                 MAKE_DMHRESULTERROR(0x0162)


/* DMUS_E_TIME_PAST
 *
 * The time is in the past, and the operation can not succeed.
 */
#define DMUS_E_TIME_PAST                MAKE_DMHRESULTERROR(0x0165)

/* DMUS_E_LOADER_FAILEDCREATE
 *
 * Unable to find or create object.
 */
#define DMUS_E_LOADER_FAILEDCREATE      MAKE_DMHRESULTERROR(0x0184)

/* DMUS_E_LOADER_OBJECTNOTFOUND
 *
 * Object was not found.
 */
#define DMUS_E_LOADER_OBJECTNOTFOUND    MAKE_DMHRESULTERROR(0x0185)

/* DMUS_E_INVALIDFILE
 *
 * The file requested is not a valid file.
 */
#define DMUS_E_INVALIDFILE              MAKE_DMHRESULTERROR(0x0200) 

/* DMUS_E_CANNOT_CONVERT
 *
 * A call to MIDIToMusic() or MusicToMIDI() resulted in an error because
 * the requested conversion could not happen. This usually occurs when the
 * provided DMUS_CHORD_KEY structure has an invalid chord or scale pattern.
 */
#define DMUS_E_CANNOT_CONVERT           MAKE_DMHRESULTERROR(0x0207)

/* DMUS_E_SCRIPT_ERROR_IN_SCRIPT
 *
 * An error was encountered while parsing or executing the script.
 * The pErrorInfo parameter (if supplied) was filled with information about the error.
 */
#define DMUS_E_SCRIPT_ERROR_IN_SCRIPT        MAKE_DMHRESULTERROR(0x0215)

/* DMUS_E_SCRIPT_VARIABLE_NOT_FOUND
 *
 * The script does not contain a variable with the specified name.
 */
#define DMUS_E_SCRIPT_VARIABLE_NOT_FOUND     MAKE_DMHRESULTERROR(0x021A)

/* DMUS_E_SCRIPT_ROUTINE_NOT_FOUND
 *
 * The script does not contain a routine with the specified name.
 */
#define DMUS_E_SCRIPT_ROUTINE_NOT_FOUND      MAKE_DMHRESULTERROR(0x021B)

/* DMUS_E_SCRIPT_CONTENT_READONLY
 *
 * Scripts variables for content referenced or embedded in a script cannot be set.
 */
#define DMUS_E_SCRIPT_CONTENT_READONLY       MAKE_DMHRESULTERROR(0x021C)

/* DMUS_E_SCRIPT_NOT_A_REFERENCE
 *
 * Attempt was made to set a script's variable by reference to a value that was
 * not an object type.
 */
#define DMUS_E_SCRIPT_NOT_A_REFERENCE        MAKE_DMHRESULTERROR(0x021D)

/* DMUS_E_SCRIPT_VALUE_NOT_SUPPORTED
 *
 * Attempt was made to set a script's variable by value to an object that does
 * not support a default value property.
 */
#define DMUS_E_SCRIPT_VALUE_NOT_SUPPORTED    MAKE_DMHRESULTERROR(0x021E)


/* DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR
 *
 * A script written in AudioVBScript could not be read because it contained a statement that
 * is not allowed by the AudioVBScript language.
 */
#define DMUS_E_AUDIOVBSCRIPT_SYNTAXERROR     MAKE_DMHRESULTERROR(0x0223)

/* DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR
 *
 * A script routine written in AudioVBScript failed because an invalid operation occurred.  For example,
 * adding the number 3 to a segment object would produce this error.  So would attempting to call a routine
 * that doesn't exist.
 */
#define DMUS_E_AUDIOVBSCRIPT_RUNTIMEERROR     MAKE_DMHRESULTERROR(0x0224)

/* DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE
 *
 * A script routine written in AudioVBScript failed because a function outside of a script failed to complete.
 * For example, a call to PlaySegment that fails to play because of low memory would return this error.
 */
#define DMUS_E_AUDIOVBSCRIPT_OPERATIONFAILURE     MAKE_DMHRESULTERROR(0x0225)

/* DMUS_E_NO_AUDIOPATH_CONFIG
 *
 * A segment or song was asked for its embedded audio path configuration,
 * but there isn't any. 
 */
#define DMUS_E_NO_AUDIOPATH_CONFIG     MAKE_DMHRESULTERROR(0x0228)

#endif

