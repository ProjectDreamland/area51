#ifndef AUDIO_DEBUG_HPP
#define AUDIO_DEBUG_HPP

#include "audio_private.hpp"

#if 0

#define MAX_AUDIO_DEBUG_VOICES   (64)
#define MAX_AUDIO_DEBUG_ELEMENTS (6)

enum {
WARN_NOT_FOUND=0,
WARN_NOT_LOADED,
NUM_DEBUG_SAMPLES
};

struct debug_sample
{
    void*          pAram;
    char*          pName;
    sample_header* pSample;
    char*          pSampleBuffer;
};

class audio_debug
{

public:

struct debug_voice
{
    char*       pName;
    voice*      pVoice;                 // Pointer to the voice.
    element*    pElements[MAX_AUDIO_DEBUG_ELEMENTS];
};

                audio_debug          ( void );
               ~audio_debug          ( void );
void            Init                 ( void );
void            Kill                 ( void );
void            InitVoice            ( voice* pVoice );
void            UpdateVoice          ( voice* pVoice );
sample_header*  GetDebugSampleHeader ( s32 Index );
char*           GetDebugSampleName   ( s32 Index );
void*           GetDebugSampleARAM   ( s32 Index );

u64             m_ActiveBits;
debug_voice     m_Voices[MAX_AUDIO_DEBUG_VOICES];
debug_sample    m_DebugSamples[NUM_DEBUG_SAMPLES];
};

typedef void display_debug_text_fn( void );

class screen_debug_record 
{
public:

                screen_debug_record ( void );
               ~screen_debug_record ( void );
void            Init                ( void );
void            Kill                ( void );
void            OnLeft              ( void );
void            OnRight             ( void );
void            OnUp                ( void );
void            OnDown              ( void );
void            OnZoomIn            ( void );
void            OnZoomOut           ( void );

void            SetOnLeft           ( screen_debug_record* pRecord ); 
void            SetOnRight          ( screen_debug_record* pRecord );
void            SetOnUp             ( screen_debug_record* pRecord );
void            SetOnDown           ( screen_debug_record* pRecord );
void            SetOnZoomIn         ( screen_debug_record* pRecord );
void            SetOnZoomOut        ( screen_debug_record* pRecord );

};

extern audio_debug g_AudioDebug;

#endif

#endif // AUDIO_DEBUG_HPP