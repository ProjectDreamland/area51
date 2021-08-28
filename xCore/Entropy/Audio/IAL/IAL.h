// IAL.h
//

#include "x_files.hpp"

#ifndef IAL_H
#define IAL_H

//==============================================================================

enum ial_state
{
    IAL_DONE,
    IAL_PLAY,
    IAL_PAUSED,
    IAL_STOP,
};

enum ial_format
{
    IAL_PCM,
};

struct ial_channel
{
    xbool       Allocated;
    s32         Sequence;

    s16*        pData;
    s32         SampleRate;
    s32         nSamples;

    s32         Cursor;
    s32         Fraction;
    bool        Looped;
    s32         LoopStart;
    s32         LoopEnd;

    ial_state   State;

    f32         VolumeL;
    f32         VolumeR;
    f32         Pan;
    f32         Pitch;

    // System data
    s32         MixedVolL;
    s32         MixedVolR;
};

typedef u32 ial_hchannel;

//==============================================================================

xbool IAL_Init( HWND hWnd );
void  IAL_Kill( void );
void  IAL_SetSystemVolume( f32 Volume ); // 0 -> 1

ial_hchannel    IAL_allocate_channel    ( void );
void            IAL_release_channel     ( ial_hchannel hChannel );

void            IAL_init_channel        ( ial_hchannel hChannel, void* pData, s32 nSamples, s32 LoopCount, s32 LoopStart, s32 LoopEnd,
                                          ial_format Format, s32 SampleRate, f32 VolumeL, f32 ValueR, f32 Pitch );

void            IAL_start_channel       ( ial_hchannel hChannel );
void            IAL_stop_channel        ( ial_hchannel hChannel );
void            IAL_pause_channel       ( ial_hchannel hChannel );
void            IAL_resume_channel      ( ial_hchannel hChannel );
void            IAL_end_channel         ( ial_hchannel hChannel );
ial_state       IAL_channel_status      ( ial_hchannel hChannel );
s32             IAL_channel_position    ( ial_hchannel hChannel );
void            IAL_set_channel_volume  ( ial_hchannel hChannel, f32 VolumeL, f32 VolumeR );
void            IAL_set_channel_pitch   ( ial_hchannel hChannel, f32 Pitch );
void			IAL_set_channel_looped	( ial_hchannel hChannel, bool Looped );
s32             IAL_get_output_amplitude( s32 Channel );
void            IAL_stop_loop           ( ial_hchannel hChannel, s32 nSamples );


//==============================================================================

#endif