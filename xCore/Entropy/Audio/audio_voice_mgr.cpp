#include "audio_voice_mgr.hpp"
#include "audio_channel_mgr.hpp"
#include "audio_hardware.hpp"
#include "audio_private.hpp"
#include "audio_package.hpp"
#include "audio_inline.hpp"
#include "e_Audio.hpp"
#include "audio_stream_mgr.hpp"
#include "x_bytestream.hpp"
#include "x_log.hpp"

#if defined(rbrannon)
extern voice*   g_DebugVoice;
extern element* g_DebugElement;
extern channel* g_DebugChannel;
extern f32      g_DebugTime;
#endif

//#define UPDATE_STATE_LOGGING "audio_voice_mgr::Update(state)" 
//#define PLAY_TIME_LOGGING    "audio_voice_mgr::GetCurrentPlayTime"

//------------------------------------------------------------------------------
// Defines.

#if defined(TARGET_XBOX)
#define MAX_VOICES    (64)
#define MAX_ELEMENTS  (MAX_VOICES * 3)
#else
#define MAX_VOICES    (48)
#define MAX_ELEMENTS  (MAX_VOICES * 3)
#endif

#if defined(X_DEBUG)
#define ELEMENT_EXPIRE_DELAY    (0.250f)
#else
#define ELEMENT_EXPIRE_DELAY    (0.250f)
#endif
//------------------------------------------------------------------------------
// Enums.

enum voice_dirty_bits
{
    VOICE_DB_ELEMENT_CHANGE      = (1<<0),
    VOICE_DB_PAN_CHANGE          = (1<<1),
    VOICE_DB_VOLUME_CHANGE       = (1<<2),
    VOICE_DB_PITCH_CHANGE        = (1<<3),
    VOICE_DB_EFFECTSEND_CHANGE   = (1<<4),
    VOICE_DB_RELEASE_TIME_CHANGE = (1<<5),
};

//------------------------------------------------------------------------------
// Static variables.

static xbool        s_IsInitialized = FALSE;    // Semaphore.
static xbool        s_UpdatePriority;           // Update flag.
static voice        s_Voices[ MAX_VOICES ];     // Voice buffer.
static element      s_Elements[ MAX_ELEMENTS ]; // Element buffer.

//------------------------------------------------------------------------------

audio_voice_mgr g_AudioVoiceMgr;

//------------------------------------------------------------------------------

audio_voice_mgr::audio_voice_mgr( void )
{
    m_FirstVoice   = &s_Voices[0];
    m_LastVoice    = &s_Voices[MAX_VOICES-1];
    m_FirstElement = &s_Elements[0];
    m_LastElement  = &s_Elements[MAX_ELEMENTS-1];
    m_NumVoices    = MAX_VOICES;
    m_NumElements  = MAX_ELEMENTS;
}

//------------------------------------------------------------------------------

audio_voice_mgr::~audio_voice_mgr( void )
{
}

//------------------------------------------------------------------------------

void audio_voice_mgr::Init( void )
{
    s32 i;
    voice* pVoice;
    voice* pPrevVoice;
    element* pElement;
    element* pPrevElement;

    // Error check
    ASSERT( s_IsInitialized == FALSE );

    m_LockLevel = 0;

    // Snag the mutex.
    Lock();

    // It's initialized!
    s_IsInitialized = TRUE;

    // Previous is the free list!
    pPrevVoice = FreeVoices();

    // For each voice...
    for( i=0, pVoice=s_Voices ; i<MAX_VOICES ; i++, pVoice++ )
    {
        // Initialize it.
        x_memset( pVoice, 0, sizeof( voice ) );
        pVoice->Sequence = 32000;

        // Link it.
        pVoice->Link.pPrev = pPrevVoice;
        pVoice->Link.pNext = (pVoice+1);

        // Update previous.
        pPrevVoice = pVoice;
    }

    // Back up one...
    pVoice = pPrevVoice;

    // Last one is tail of free list.
    pVoice->Link.pNext = FreeVoices();

    // Initialize the free list.
    FreeVoices()->Link.pPrev = pVoice;
    FreeVoices()->Link.pNext = s_Voices;

    // Empty the used list.
    UsedVoices()->Link.pPrev =
    UsedVoices()->Link.pNext = UsedVoices();

    // Head/Tail now has lowest priority.
    UsedVoices()->Params.Priority = -1;

    // Previus is the free list!
    pPrevElement = FreeElements();

    // For each element...
    for( i=0, pElement=s_Elements ; i<MAX_ELEMENTS ; i++, pElement++ )
    {
        // Initialize it.
        x_memset( pElement, 0, sizeof( element ) );

        // Link it.
        pElement->Link.pPrev = pPrevElement;
        pElement->Link.pNext = (pElement+1);
        pElement->Type = (element_type)-1;

        // Update previous.
        pPrevElement = pElement;
    }

    // Back up one...
    pElement = pPrevElement;

    // Last one is tail of free list.
    pElement->Link.pNext = FreeElements();

    // Initialize the free list.
    FreeElements()->Link.pPrev = pElement;
    FreeElements()->Link.pNext = s_Elements;

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::Kill( void )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // No longer initialized.
    s_IsInitialized = FALSE;

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

element* audio_voice_mgr::AquireElement( void )
{
    element* pHead;
    element* pResult;

    // Get head/tail of free list and first free element.
    pHead   = FreeElements();
    pResult = pHead->Link.pNext;

    // Make sure list is not empty.
    if( pResult != pHead )
    {
        // Take it out of the free list.
        RemoveElementFromList( pResult );

        ASSERT(pResult->Type == (element_type)-1);
        pResult->Type = (element_type)-2;

        // Tell the world.
        return pResult;
    }
    else
    {
        // TODO: Put in warning that an element was ignored.
        return NULL;
    }
}

//------------------------------------------------------------------------------

void audio_voice_mgr::ReleaseElement( element* pElement, xbool ReleaseChannel )
{
    // Error check.
    ASSERT( VALID_ELEMENT( pElement ) );

#if defined(rbrannon)
    ASSERT( m_LockLevel > 0 );
#endif

    // Release the hardware channel?
    if( ReleaseChannel && pElement->pChannel )
    {
#if defined(rbrannon)
        if( pElement == g_DebugElement )
        {
            LOG_WARNING( "AudioDebug(audio_voice_mgr::ReleaseElement)",
                            "Freed g_DebugElement!" );
            LOG_FLUSH();
        }
#endif

        // Acquire the audio hardware.
        g_AudioHardware.Lock();

        // Give up the hardware resource.
        g_AudioHardware.ReleaseChannel( pElement->pChannel );
        
        // Now remove it from the channel list damnit.
        RemoveChannelFromList( pElement->pChannel );
        InsertChannelIntoList( pElement->pChannel, g_AudioChannelMgr.FreeList() );

        // Release it.
        g_AudioHardware.Unlock();
    }

    // Does the element have a voice?    
    if( pElement->pVoice )
    {
        // Error check.
        ASSERT( VALID_VOICE(pElement->pVoice) );

        // Notify the voice that an element has changed.
        pElement->pVoice->Dirty |= VOICE_DB_ELEMENT_CHANGE;
    }

    ASSERT(pElement->Type != (element_type)-1);
    // Take the element out of used list.
    RemoveElementFromList( pElement );

    // Nuke it!
    x_memset( pElement, 0, sizeof(element) );
    pElement->Type = (element_type)-1;

    // Put the element into the free list.
    InsertElementIntoList( pElement, FreeElements() );
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::PauseElement( element* pElement )
{
    // Error check.
    ASSERT( VALID_ELEMENT( pElement ) );

    // Stop the channel!
    if( VALID_CHANNEL( pElement->pChannel ) && (pElement->pChannel->State == STATE_RUNNING) )
    {
        // Pause it!
        g_AudioChannelMgr.Pause( pElement->pChannel );

        // Its not playing.
        pElement->State = ELEMENT_PAUSED;
    }
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::ResumeElement( element* pElement )
{
    // Error check.
    ASSERT( VALID_ELEMENT( pElement ) );

    // Stop the channel!
    if( VALID_CHANNEL( pElement->pChannel ) && (pElement->pChannel->State == STATE_PAUSED) )
    {
        g_AudioChannelMgr.Resume( pElement->pChannel );

        // Its playing.
        pElement->State = ELEMENT_PLAYING;
    }
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::ApplyElementVolume( element* pElement )
{
    // Error check.
    ASSERT( VALID_ELEMENT( pElement ) );
    ASSERT( VALID_CHANNEL( pElement->pChannel ) );

    // Set the volume.
    g_AudioChannelMgr.SetVolume( pElement->pChannel, pElement->Volume );
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::ApplyElementPan( element* pElement )
{
    // Error check.
    ASSERT( VALID_ELEMENT( pElement ) );
    ASSERT( VALID_CHANNEL( pElement->pChannel ) );
    ASSERT( pElement->pVoice );

    // If its not positional, then use the stereo pan?
    if( !pElement->pVoice->IsPositional )
    {
        g_AudioMgr.Calculate2dPan( pElement->Params.Pan2d, pElement->Params.Pan3d );
    }

    // Set the actual pan now.
    g_AudioChannelMgr.SetPan( pElement->pChannel, pElement->Params.Pan3d );
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::ApplyElementPitch( element* pElement )
{
    // Error check.
    ASSERT( VALID_ELEMENT( pElement ) );
    ASSERT( VALID_CHANNEL( pElement->pChannel ) );

    // Set the pitch.
    g_AudioChannelMgr.SetPitch( pElement->pChannel, pElement->Pitch );
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::ApplyElementEffectSend( element* pElement )
{
    // Error check.
    ASSERT( VALID_ELEMENT( pElement ) );
    ASSERT( VALID_CHANNEL( pElement->pChannel ) );

    // Set the effect send.
    g_AudioChannelMgr.SetEffectSend( pElement->pChannel, pElement->EffectSend );
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::StartElement( element* pElement )
{
    channel* pChannel = pElement->pChannel;

    // Error check.
    ASSERT( VALID_ELEMENT( pElement ) );
    ASSERT( VALID_CHANNEL( pChannel ) );

    // Calculate volume, pitch and effect send.
    pElement->Volume     = CalculateElementVolume( pElement );
    pElement->Pitch      = CalculateElementPitch( pElement );
    pElement->EffectSend = CalculateElementEffectSend( pElement );

    // If its not positional, then use the stereo pan.
    if( !pElement->pVoice->IsPositional )
    {
        g_AudioMgr.Calculate2dPan( pElement->Params.Pan2d, pElement->Params.Pan3d );
    }

    // Set the channel parameters.
    g_AudioChannelMgr.SetVolume( pChannel, pElement->Volume );
    g_AudioChannelMgr.SetPitch( pChannel, pElement->Pitch );
    g_AudioChannelMgr.SetPan( pChannel, pElement->Params.Pan3d );
    g_AudioChannelMgr.SetEffectSend( pChannel, pElement->EffectSend );

    // Start up the sound!
    g_AudioChannelMgr.Start( pChannel );

    // Its playing now!
    pElement->State = ELEMENT_PLAYING;
}

//------------------------------------------------------------------------------

extern void AudioDebug( const char* pString );

voice* audio_voice_mgr::AcquireVoice( s32 Priority, f32 AbsoluteVolume )
{
    voice* pVoice;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Get a voice from the free list.
    pVoice = FreeVoices()->Link.pNext;

    // Free list empty?
    if( pVoice == FreeVoices() )
    {
        // Get oldest, lowest priority voice.
        pVoice = UsedVoices()->Link.pPrev;
        ASSERT( VALID_VOICE(pVoice) );

        // Which voice is more important?
        if( (pVoice->Params.Priority <= Priority) /*|| ((pVoice->Params.Priority == Priority) && (pVoice->Volume < AbsoluteVolume))*/ ) 
        {
#if defined(rbrannon)
            if( pVoice == g_DebugVoice )
            {
                LOG_WARNING( "AudioDebug(audio_voice_mgr::AcquireVoice)",
                             "Freed - g_DebugVoice" );
            }
#endif // defined(rbrannon)
            // Free the voice, don't put it in the free list.
            FreeVoice( pVoice, FALSE );
        }
        else
        {
            // Cannot aquire a voice...
            //AudioDebug( xfs( "Cannot acquire voice! Priority = %d, Voice Priority = %d\n", Priority, pVoice->Params.Priority) );
            pVoice = NULL;
        }
    }
    else
    {
        // Take it out of the free list.
        ASSERT( VALID_VOICE(pVoice) );
        RemoveVoiceFromList( pVoice );
    }

    // Was a voice aquired?
    if( pVoice )
    {
        // Set the voices priority directly for the priority insertion.
        pVoice->Params.Priority = Priority;

        // Set voices volume directly for the priority insertion.
        pVoice->Volume = AbsoluteVolume;

        // Initialize the degreestosiund
        pVoice->DegreesToSound     = 0;
        pVoice->PrevDegreesToSound = 0;

        // Insert it into the list by its priority and volume (don't remove it from a list).
        PrioritizeVoice( pVoice, FALSE );  
    }

    // Release it.
    Unlock();

    // Tell the world
    return( pVoice );
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetVoiceTime( voice* pVoice )
{
    f32 Result = 0.0f;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag it.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        element* pElement;
        element* pHead;

        // Error check.
        ASSERT( VALID_VOICE(pVoice) );

        // Get start of element list.
        pHead    = (element*)&pVoice->Elements;
        pElement = pHead->Link.pNext;

        // Only if it has elements...
        if( pElement )
        {
            // For every element...
            while( pElement != pHead )
            {
                f32 Time;

                element* pNextElement = pElement->Link.pNext;

                // Calcualte this elements total time.
                Time = pElement->DeltaTime + ((f32)pElement->Sample.pHotSample->nSamples / (f32)pElement->Sample.pHotSample->SampleRate);

                // Return longest time.
                if( Time > Result )
                    Result = Time;
                
                // Walk the list
                pElement = pNextElement;
            }
        }
    }
    else
    {
        // TODO: Warning message.
    }

    // Release it.
    Unlock();
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetLipSync( voice* pVoice )
{
    // Error check.
    ASSERT( s_IsInitialized );

    Lock();

    // Only if its valid.
    if( pVoice )
    {
        element* pElement;
        element* pHead;

        // Only if its running or paused
        if( (pVoice->State == STATE_RUNNING) || (pVoice->State == STATE_PAUSED) )
        {
            // Error check.
            ASSERT( VALID_VOICE(pVoice) );

            // Get start of element list.
            pHead    = (element*)&pVoice->Elements;
            pElement = pHead->Link.pNext;

            // Only if it has elements...
            if( pElement != pHead )
            {
                if( pElement->Sample.pHotSample->LipSyncOffset != 0xffffffff )
                {
                    audio_package*  pPackage = pElement->pVoice->pPackage;
                    f32             dTime    = GetCurrentPlayTime( pVoice );
                    u8*             pLipSync = (u8*)((u32)pElement->Sample.pHotSample->LipSyncOffset + (u32)pPackage->m_LipSyncTable);
                    s32             Index;
                    s32             Result;

                    // Bump past sample rate (should be 30)
                    pLipSync++;

                    Index = (s32)(dTime * 30.0f);
                        
                    Result = (s32)pLipSync[Index];
                    Unlock();
                    return (f32)Result / 255.0f;
                }
            }
        }
    }
    else
    {
        // TODO: Warning message.
    }

    Unlock();
    return 0.0f;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::HasLipSync( voice* pVoice )
{

    // Error check.
    ASSERT( s_IsInitialized );

    // Acquire mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        element* pElement;
        element* pHead;

        // Error check.
        ASSERT( VALID_VOICE(pVoice) );

        // Get start of element list.
        pHead    = (element*)&pVoice->Elements;
        pElement = pHead->Link.pNext;

        // Only if it has elements...
        if( pElement != pHead )
        {
            xbool Result = pElement->Sample.pHotSample->LipSyncOffset != 0xffffffff;
            Unlock();
            return Result;
        }
    }
    else
    {
        // TODO: Warning message.
    }

    Unlock();
    return FALSE;
}

//------------------------------------------------------------------------------

s32 audio_voice_mgr::GetBreakPoints( voice* pVoice, f32* & BreakPoints )
{
    s32 Result = 0;

    // Error check.
    ASSERT( s_IsInitialized );

    // Acquire mutex.
    Lock();

    // Default is not any...
    BreakPoints = NULL;

    // Only if its valid.
    if( pVoice )
    {
        element* pElement;
        element* pHead;

        // Error check.
        ASSERT( VALID_VOICE(pVoice) );

        // Get start of element list.
        pHead    = (element*)&pVoice->Elements;
        pElement = pHead->Link.pNext;

        // Only if it has elements...
        if( pElement != pHead )
        {
            if( pElement->Sample.pHotSample->BreakPointOffset != 0xffffffff )
            {
                audio_package*  pPackage     = pElement->pVoice->pPackage;
                s32*            pBreakPoints = (s32*)((u32)pElement->Sample.pHotSample->BreakPointOffset + (u32)pPackage->m_BreakPointTable);
                
                Result      = *pBreakPoints++;
                BreakPoints = (f32*)pBreakPoints;
                Unlock();
                return Result;
            }
        }
    }
    else
    {
        // TODO: Warning message.
    }

    // No break points!
    Unlock();
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetCurrentPlayTime( voice* pVoice )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Snag mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        element* pElement;
        element* pHead;

        // Only if its running or paused
        if( (pVoice->State == STATE_RUNNING) || (pVoice->State == STATE_PAUSED) )
        {
            // Error check.
            ASSERT( VALID_VOICE(pVoice) );

            // Get start of element list.
            pHead    = (element*)&pVoice->Elements;
            pElement = pHead->Link.pNext;

            // Only if it has elements...
            if( pElement != pHead && pElement->pChannel )
            {
                u32 nSamples = g_AudioHardware.GetSamplesPlayed( pElement->pChannel );
#ifdef PLAY_TIME_LOGGING
                LOG_MESSAGE( PLAY_TIME_LOGGING, "nSamples: %d, nSamplesMax: %d", nSamples, pElement->pChannel->Sample.pHotSample->nSamples );
#endif
                if( nSamples > (u32)pElement->pChannel->Sample.pHotSample->nSamples )
                    nSamples = (u32)pElement->pChannel->Sample.pHotSample->nSamples;

                f32 Result = (f32)nSamples / (f32)pElement->pChannel->Sample.pHotSample->SampleRate;
                Unlock();
                return Result;
            }
        }
    }
    else
    {
        // TODO: Warning message.
    }

    Unlock();
    return 0.0f;
}

//------------------------------------------------------------------------------

const char* audio_voice_mgr::GetVoiceDescriptor( voice* pVoice )
{
    const char* pDescriptor = "NULL";

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag mutex.
    Lock();

    // Grab descriptor if voice if valid
    if( pVoice )
        pDescriptor = pVoice->pDescriptorName;

    Unlock();
    
    return pDescriptor;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::GetIsReady( voice* pVoice )
{
    // Error check.
    ASSERT( s_IsInitialized );

    Lock();

    // Only if its valid.
    if( pVoice )
    {
        element* pElement;
        element* pHead;

        // Only if its running or paused
        if( (pVoice->State == STATE_RUNNING) || (pVoice->State == STATE_PAUSED) || (pVoice->State == STATE_NOT_STARTED) )
        {
            // Error check.
            ASSERT( VALID_VOICE(pVoice) );

            // Get start of element list.
            pHead    = (element*)&pVoice->Elements;
            pElement = pHead->Link.pNext;

            // Only if it has elements...
            if( pElement != pHead )
            {
                xbool Result = pElement->State == ELEMENT_READY;
                Unlock();
                return Result;
            }
        }
    }
    else
    {
        // TODO: Warning message.
    }

    Unlock();
    return FALSE;
}

//------------------------------------------------------------------------------
/*
void audio_voice_mgr::ReleaseVoiceLoop( voice* pVoice )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        element* pElement;
        element* pHead;

        // Only if its running or paused
        if( (pVoice->State == STATE_RUNNING) || (pVoice->State == STATE_PAUSED) )
        {
            // Error check.
            ASSERT( VALID_VOICE(pVoice) );

            // Get start of element list.
            pHead    = (element*)&pVoice->Elements;
            pElement = pHead->Link.pNext;

            // Only if it has elements...
            if( pElement != pHead && pElement->pChannel )
            {
                //g_AudioHardware.ReleaseLoop( pElement->pChannel );
                pElement = pElement->Link.pNext;
            }
        }
    }
    else
    {
        // TODO: Warning message.
    }

    // Release it.
    Unlock();
}
*/
//------------------------------------------------------------------------------

void audio_voice_mgr::ReleaseVoice( voice* pVoice, f32 Time )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        xbool    bFreeImmediate = TRUE;
        
        // Gotta be valid.
        ASSERT( VALID_VOICE(pVoice) );

        // If the voice is just starting or already running then use the release time provided,
        // otherwise just kill it immediately.
        if( (pVoice->State == STATE_STARTING) || (pVoice->State == STATE_RUNNING) )
        {
            bFreeImmediate = FALSE;
        }

        // Stop it right now? (might cause a pop, but oh well...)
        if( (Time <= 0.0f) || bFreeImmediate )
        {
            // Free the voice, put it in the free list.
            FreeVoice( pVoice, TRUE );
        }
        else
        {
            // Note that the voice is releasing, set the delta volume.
            pVoice->IsReleasing = TRUE;
            pVoice->DeltaVolume = pVoice->Params.Volume / Time;
        }
    }
    else
    {
        // TODO: Warning message.
    }

    // Release it.
    Unlock();
}
                                   
//------------------------------------------------------------------------------

void audio_voice_mgr::ReleaseAllVoices( void )
{
    voice*   pHeadVoice;
    voice*   pVoice;
    voice*   pNextVoice;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Get head/tail of active voices, first active voice
    pHeadVoice = UsedVoices();
    pVoice     = pHeadVoice->Link.pNext;

    // For each active voice...
    while( pVoice != pHeadVoice )
    {
        // Get next voice.
        ASSERT( VALID_VOICE(pVoice) );
        pNextVoice = pVoice->Link.pNext;

        // Free the voice, put it in the free list.
        FreeVoice( pVoice, TRUE );

        // Walk the list...
        pVoice = pNextVoice;
    }

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::ReleasePackagesVoices( audio_package* pPackage )
{
    voice*   pHeadVoice;
    voice*   pVoice;
    voice*   pNextVoice;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Get head/tail of active voices, first active voice
    pHeadVoice = UsedVoices();
    pVoice     = pHeadVoice->Link.pNext;

    // For each active voice...
    while( pVoice != pHeadVoice )
    {
        // Get next voice.
        ASSERT( VALID_VOICE(pVoice) );
        pNextVoice = pVoice->Link.pNext;

        // Packages match?
        if( pVoice->pPackage == pPackage )
        {
            // Free the voice, put it in the free list.
            FreeVoice( pVoice, TRUE );
        }

        // Walk the list...
        pVoice = pNextVoice;
    }

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::StartVoice( voice* pVoice )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        // Make it start...
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoice->State == STATE_NOT_STARTED )
        {
            pVoice->State = STATE_STARTING;
        }
        else
        {
            // TODO: Warning message.
        }
    }
    else
    {
        // TODO: Warning message.
    }

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::Segue( voice* pVoice, voice* pVoiceToQ )
{
    xbool Result = FALSE;

    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        // Make it start...
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoiceToQ )
        {
            ASSERT( VALID_VOICE(pVoiceToQ) );
            pVoiceToQ->StartQ          = 0;
            pVoiceToQ->pSegueVoicePrev = pVoice;
        }
        pVoice->pSegueVoiceNext = pVoiceToQ;
        Result = TRUE;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetReleaseTime( voice* pVoice, f32 Time )
{
    xbool Result = FALSE;

    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        // Make it start...
        ASSERT( VALID_VOICE(pVoice) );
        if( Time <= 0.0f )
            Time = 0.0f;
        pVoice->ReleaseTime = Time;
        pVoice->Dirty |= VOICE_DB_RELEASE_TIME_CHANGE;
        Result = TRUE;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    return Result;
}

//------------------------------------------------------------------------------

void audio_voice_mgr::PauseVoice( voice* pVoice )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        // Make it start...
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoice->State == STATE_RUNNING )
        {
            pVoice->State = STATE_PAUSING;
        }
        else
        {
            // TODO: Warning message.
        }
    }
    else
    {
        // TODO: Warning message.
    }

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::ResumeVoice( voice* pVoice )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        // Make it start...
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoice->State == STATE_PAUSED )
        {
            pVoice->State = STATE_RESUMING;
        }
        else
        {
            // TODO: Warning message.
        }
    }
    else
    {
        // TODO: Warning message.
    }

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::PauseAllVoices( void )
{
    voice*   pHeadVoice;
    voice*   pVoice;
    voice*   pNextVoice;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Get head/tail of active voices, first active voice
    pHeadVoice = UsedVoices();
    pVoice     = pHeadVoice->Link.pNext;

    // For each active voice...
    while( pVoice != pHeadVoice )
    {
        // Get next voice.
        ASSERT( VALID_VOICE(pVoice) );
        pNextVoice = pVoice->Link.pNext;

        // What to do?
        switch( pVoice->State )
        {
            case STATE_RUNNING:
                pVoice->State = STATE_PAUSING;
                break;

            default:
                // TODO: Warning message
                break;
        }

        // Walk the list...
        pVoice = pNextVoice;
    }

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::ResumeAllVoices( void )
{
    voice*   pHeadVoice;
    voice*   pVoice;
    voice*   pNextVoice;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Get head/tail of active voices, first active voice
    pHeadVoice = UsedVoices();
    pVoice     = pHeadVoice->Link.pNext;

    // For each active voice...
    while( pVoice != pHeadVoice )
    {
        // Get next voice.
        ASSERT( VALID_VOICE(pVoice) );
        pNextVoice = pVoice->Link.pNext;

        // What to do?
        switch( pVoice->State )
        {
            case STATE_PAUSED:
                pVoice->State = STATE_RESUMING;
                break;

            default:
                // TODO: Warning message
                break;
        }

        // Walk the list...
        pVoice = pNextVoice;
    }

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::IsVoicePlaying( voice* pVoice )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = (pVoice->State == STATE_RUNNING);
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::IsVoiceStarting( voice* pVoice )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = (pVoice->State == STATE_STARTING);
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::IsVoiceReleasing( voice* pVoice )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        Result = pVoice->IsReleasing;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::IsVoiceReady( voice* pVoice )
{
    // Error check.
    ASSERT( s_IsInitialized );

    xbool Result = FALSE;

    Lock();

    // Only if its valid.
    if( pVoice )
    {
        // Error check.
        ASSERT( VALID_VOICE(pVoice) );

        // Get start of element list.
        element* pHead    = (element*)&pVoice->Elements;
        element* pElement = pHead->Link.pNext;

        // Only if it has elements...
        if( pElement != pHead )
        {
            Result = pElement->State == ELEMENT_READY;
        }
    }

    Unlock();

    return Result;
}

//------------------------------------------------------------------------------
s32 audio_voice_mgr::GetVoicePriority( voice* pVoice )
{
    s32 Result = -1;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = pVoice->Params.Priority;
    }
    else
    {
        // TODO: Warning message.
        Result = -1;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetVoiceUserVolume( voice* pVoice )
{
    f32 Result = 0.0f;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = pVoice->UserVolume;
    }
    else
    {
        // TODO: Warning message.
        Result = 0.0f;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceUserVolume( voice* pVoice, f32 Volume )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        pVoice->UserVolume = Volume;
        pVoice->Dirty |= VOICE_DB_VOLUME_CHANGE;
        Result = TRUE;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetVoiceRelativeVolume( voice* pVoice )
{
    f32 Result = 0.0f;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = pVoice->Params.Volume;
    }
    else
    {
        // TODO: Warning message.
        Result = 0.0f;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceRelativeVolume( voice* pVoice, f32 Volume )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        pVoice->Params.Volume = Volume;
        pVoice->Dirty |= VOICE_DB_VOLUME_CHANGE;
        Result =  TRUE;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetVoicePan( voice* pVoice )
{
    f32 Result = 0.0f;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = pVoice->Params.Pan2d;
    }
    else
    {
        // TODO: Warning message.
        Result = 0.0f;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoicePan( voice* pVoice, f32 Pan )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );

        if( pVoice->IsPanChangeable )
        {
            pVoice->Params.Pan2d = Pan;
            g_AudioMgr.Calculate2dPan( pVoice->Params.Pan2d, pVoice->Params.Pan3d );
            pVoice->Dirty |= VOICE_DB_PAN_CHANGE;
            Result = TRUE;
        }
        else
        {
            // TODO: Warning message.
            Result = FALSE;
        }
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetVoiceUserPitch( voice* pVoice )
{
    f32 Result = 0.0f;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = pVoice->UserPitch;
    }
    else
    {
        // TODO: Warning message.
        Result = 0.0f;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceUserPitch( voice* pVoice, f32 Pitch )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        pVoice->UserPitch = Pitch;
        pVoice->Dirty |= VOICE_DB_PITCH_CHANGE;
        Result = TRUE;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetVoiceRelativePitch( voice* pVoice )
{
    f32 Result = 0.0f;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = pVoice->Params.Pitch;
    }
    else
    {
        // TODO: Warning message.
        Result = 0.0f;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceRelativePitch( voice* pVoice, f32 Pitch )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        pVoice->Params.Pitch = Pitch;
        pVoice->Dirty |= VOICE_DB_PITCH_CHANGE;
        Result = TRUE;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetVoiceUserEffectSend( voice* pVoice )
{
    f32 Result = 0.0f;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = pVoice->UserEffectSend;
    }
    else
    {
        // TODO: Warning message.
        Result = 0.0f;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceUserEffectSend ( voice* pVoice, f32 EffectSend )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        pVoice->UserEffectSend = EffectSend;
        pVoice->Dirty |= VOICE_DB_EFFECTSEND_CHANGE;
        Result = TRUE;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

f32 audio_voice_mgr::GetVoiceRelativeEffectSend( voice* pVoice )
{
    f32 Result = 0.0f;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        Result = pVoice->Params.EffectSend;
    }
    else
    {
        // TODO: Warning message.
        Result = 0.0f;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceRelativeEffectSend ( voice* pVoice, f32 EffectSend )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        pVoice->Params.EffectSend = EffectSend;
        pVoice->Dirty |= VOICE_DB_EFFECTSEND_CHANGE;
        Result = TRUE;
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::GetVoicePosition( voice* pVoice, vector3& Position, s32& ZoneID )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoice->IsPositional )
        {
            Position = pVoice->Position;
            ZoneID   = pVoice->ZoneID;
            Result   = TRUE;
        }
        else
        {
            // TODO: Warning message.
            Result = FALSE;
        }
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoicePosition( voice* pVoice, const vector3& Position, s32 ZoneID )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoice->IsPositional )
        {
            pVoice->Position = Position;
            pVoice->ZoneID   = ZoneID;
            Result = TRUE;
        }
        else
        {
            // TODO: Warning message.
            Result = FALSE;
        }
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceUserFalloff( voice* pVoice, f32 Near, f32 Far )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoice->IsPositional )
        {
            pVoice->UserFarFalloff  = Far;
            pVoice->UserNearFalloff = Near;
            Result = TRUE;
        }
        else
        {
            // TODO: Warning message.
            Result = FALSE;
        }
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceRelativeFalloff( voice* pVoice, f32 Near, f32 Far )
{
    xbool Result = FALSE;

    // Error check.
    ASSERT( s_IsInitialized );

    // Snag the mutex.
    Lock();

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoice->IsPositional )
        {
            pVoice->Params.FarFalloff  = Far;
            pVoice->Params.NearFalloff = Near;
            pVoice->UserFarDiffuse     = Far;
            pVoice->UserNearDiffuse    = Near;
            Result = TRUE;
        }
        else
        {
            // TODO: Warning message.
            Result = FALSE;
        }
    }
    else
    {
        // TODO: Warning message.
        Result = FALSE;
    }

    // Release it.
    Unlock();

    // Tell the world.
    return Result;
}

//------------------------------------------------------------------------------

xbool audio_voice_mgr::SetVoiceUserDiffuse( voice* pVoice, f32 Near, f32 Far )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Only if its valid.
    if( pVoice )
    {
        ASSERT( VALID_VOICE(pVoice) );
        if( pVoice->IsPositional )
        {
            pVoice->UserFarDiffuse  = Far;
            pVoice->UserNearDiffuse = Near;
            return TRUE;
        }
        else
        {
            // TODO: Warning message.
            return FALSE;
        }
    }
    else
    {
        // TODO: Warning message.
        return FALSE;
    }

    return FALSE;
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::UpdateStartPending( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateStartPending" );

    element* pElement;
    element* pHeadElement;
    element* pNextElement;
    
    // Any pending elements left to fire off?
    if( (pElement = pVoice->pPendingElement) != NULL )
    {
        // Get list head/tail
        pHeadElement = (element*)&pVoice->Elements;

        // Process each element...
        while( (pElement != pHeadElement) && (pVoice->CursorTime > pElement->DeltaTime) )
        {
            // get next element in the list...
            pNextElement = pElement->Link.pNext;

            // Is the element ready?
            if( pElement->State == ELEMENT_READY )
            {
                // Has the element timed out? 100 millisecond leeway...
                if( (pElement->Type != COLD_SAMPLE) && ((pVoice->CursorTime - pElement->DeltaTime) > ELEMENT_EXPIRE_DELAY) )
                {
                    // Release the element, don't free the channel - it doesn't have one.
                    ReleaseElement( pElement, FALSE );
                }
                else
                {
                    // Should not have a channel...
                    if( pElement->pChannel == NULL )
                    {
                        // Attempt to aquire a hardware channel.
                        if( g_AudioChannelMgr.Acquire( pElement ) )
                        {
#if defined(rbrannon)
                            if( (pVoice == g_DebugVoice) && (g_DebugElement == NULL) )
                            {
                                g_DebugElement = pElement;
                                g_DebugChannel = pElement->pChannel;
                                g_DebugTime    = (f32)pElement->pChannel->Sample.pHotSample->nSamples / (f32)pElement->pChannel->Sample.pHotSample->SampleRate;
                                LOG_MESSAGE( "AudioDebug(audio_voice_mgr::UpdateStartPending)", 
                                             "AQUIRED! %s, pVoice: %08x, pElement: %08x, pChannel: %08x",
                                             g_DebugVoice->pDescriptorName,
                                             g_DebugVoice,
                                             g_DebugElement,
                                             g_DebugChannel );
                                LOG_FLUSH();
                            }
#endif // defined(rbrannon)
                            // Start it up!
                            StartElement( pElement );
                        }
#if defined(rbrannon)
                        else
                        {
                            if( pVoice == g_DebugVoice )
                            {
                                LOG_WARNING( "AudioDebug(audio_voice_mgr::UpdateStartPending)", 
                                             "AQUIRE FAILED! pVoice: %08x",
                                             g_DebugVoice );
                            }
                        }
#endif // defined(rbrannon)
                    }
                    else
                    {
                        // Start it up!
                        StartElement( pElement );
                    }
                }
            }
            else
            {
/*
                // TODO: Put in check to nuke elements that are NEVER ready...
                if (pElement->State == ELEMENT_LOADING)
                {
                    BREAK;
                }
*/
            }

            // Walk the list.
            pElement = pNextElement;
        }
    }
}

//------------------------------------------------------------------------------

inline voice* audio_voice_mgr::UpdateCheckElements( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateCheckElements" );

    element* pElement;
    element* pHead;

    // Get head/tail of list and first element in the list.
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;

    // Empty element list?
    if( pElement == pHead )
    {
        // Free it
        FreeVoice( pVoice, TRUE );

        // Its dead!
        return( NULL );
    }
    else
    {
        s32 Priority    = -1;
        s32 OldPriority = pVoice->Params.Priority;

        // No pending...
        pVoice->pPendingElement = NULL;

        // Walk the entire list...
        while( pElement != pHead )
        {
            // Is this one ready?
            if( (pElement->State == ELEMENT_READY) && (pVoice->pPendingElement == NULL) )
            {
                // Found it.
                pVoice->pPendingElement = pElement;
            }

            // Find the highest priority element...
            if( (pElement->Params.Priority > Priority) && (pElement->State < ELEMENT_DONE) )
                Priority = pElement->Params.Priority;

            // Walk the list
            pElement = pElement->Link.pNext;
        }
        
        // Different priority?
        if( Priority != OldPriority )
        {
            // Set the voices priority.
            pVoice->Params.Priority = Priority;

            // Update the voices priority.
            s_UpdatePriority = TRUE;
        }

        // Still have a voice...
        return( pVoice );
    }
}

//------------------------------------------------------------------------------

void audio_voice_mgr::UpdateReleaseTime( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateReleaseTime" );
    element* pElement;
    element* pHead;

    // Get head/tail of list and first element in the list.
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;

    // Empty element list?
    if( pElement != pHead )
    {
        // Walk the entire list...
        while( pElement != pHead )
        {
            channel* pChannel = pElement->pChannel;
            
            // Set the release position.
            if( pChannel )
                pChannel->ReleasePosition = (u32)(pVoice->ReleaseTime * (f32)pChannel->Sample.pHotSample->SampleRate);

            // Walk the list
            pElement = pElement->Link.pNext;
        }        
    }
}

//------------------------------------------------------------------------------

inline voice* audio_voice_mgr::UpdateStateStarting( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateStateStarting" );

    // Calculate voice priority, find the next pending element...
    pVoice = UpdateCheckElements( pVoice );

    // Only if voice still exists.
    if( pVoice )
    {
        // Voice is running now.
        pVoice->State = STATE_RUNNING;
    }

    return pVoice;
}

//------------------------------------------------------------------------------

inline voice* audio_voice_mgr::UpdateStateResuming( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateStateResuming" );

    // Calculate voice priority, find the next pending element...
    pVoice = UpdateCheckElements( pVoice );

    // Only if voice still exists.
    if( pVoice )
    {
        element* pHead;
        element* pElement;

        // Voice is running now.
        pVoice->State = STATE_RUNNING;

        // For every element in the list...
        pHead    = (element*)&pVoice->Elements;
        pElement = pHead->Link.pNext;
        while( pHead != pElement )
        {
            // Resume each element.
            ASSERT( VALID_ELEMENT( pElement ) );
            if( pElement->State == ELEMENT_PAUSED )
                ResumeElement( pElement );

            // Walk the list.
            pElement = pElement->Link.pNext;
        }
    }

    return pVoice;
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::UpdateVoice3d( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateVoice3d" );

    element* pHead;
    element* pElement;

    // Calculate the voices falloff.
    pVoice->NearFalloff = CalculateVoiceNearFalloff( pVoice );
    pVoice->FarFalloff  = CalculateVoiceFarFalloff( pVoice );

    // Calculate the voices diffusion.
    pVoice->NearDiffuse = CalculateVoiceNearDiffuse( pVoice );
    pVoice->FarDiffuse  = CalculateVoiceFarDiffuse( pVoice );

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        // Stop each element.
        ASSERT( VALID_ELEMENT( pElement ) );

        // Calculate the falloffs.
        f32 Near = CalculateElementNearFalloff( pElement );
        f32 Far  = CalculateElementFarFalloff( pElement );

        // Calculate the diffusion
        f32 NearDiffuse = CalculateElementNearDiffuse( pElement );
        f32 FarDiffuse  = CalculateElementFarDiffuse( pElement );

        // Calculate the 3d volume and pan.
        g_AudioMgr.Calculate3dVolumeAndPan( Near, Far, pElement->Params.RolloffCurve, 
                                            NearDiffuse, FarDiffuse, pVoice->Position, pVoice->ZoneID,
                                            pElement->PositionalVolume, pElement->Params.Pan3d, 
                                            pVoice->DegreesToSound, pVoice->PrevDegreesToSound,
                                            pVoice->EarID );

        // Now calculate it. (rmb - I think this call is redundant....)
        // TODO: rmb - Check to see if this line can be removed.
        pElement->Volume = CalculateElementVolume( pElement );
        
        // Walk the list.
        pElement = pElement->Link.pNext;
    }

    // set the dirty bits.
    pVoice->Dirty |= VOICE_DB_VOLUME_CHANGE+VOICE_DB_PAN_CHANGE;
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::UpdateVoiceVolumeAndPan ( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateVoiceVolumeAndPan" );

    element* pHead;
    element* pElement;

    // Calculate the voices volume.
    pVoice->Volume = CalculateVoiceVolume( pVoice );

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        // Stop each element.
        ASSERT( VALID_ELEMENT( pElement ) );

        // Calculate and apply the volume.
        pElement->Volume = CalculateElementVolume( pElement );
        if( pElement->State == ELEMENT_PLAYING ) 
        {
            ApplyElementVolume( pElement );
        }

        // Do something.
        if( pElement->IsPanChangeable )
        {
            // Set and apply the pan.
            pElement->Params.Pan2d = pVoice->Params.Pan2d;
            if( pElement->State == ELEMENT_PLAYING )
            {
                ApplyElementPan( pElement );
            }
        }
        else
        {
            // TODO: Warning message.
        }

        // Walk the list.
        pElement = pElement->Link.pNext;
    }
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::UpdateVoiceVolume( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateVoiceVolume" );

    element* pHead;
    element* pElement;

    // Calculate the voices volume.
    pVoice->Volume = CalculateVoiceVolume( pVoice );

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        // Stop each element.
        ASSERT( VALID_ELEMENT( pElement ) );

        // Calculate and apply the volume.
        pElement->Volume = CalculateElementVolume( pElement );
        if( pElement->State == ELEMENT_PLAYING ) 
        {
            ApplyElementVolume( pElement );
        }

        // Walk the list.
        pElement = pElement->Link.pNext;
    }
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::UpdateVoicePan( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateVoicePan" );

    element* pHead;
    element* pElement;

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        // Stop each element.
        ASSERT( VALID_ELEMENT( pElement ) );

        // Do something.
        if( pElement->IsPanChangeable )
        {
            // Set and apply the pan.
            pElement->Params.Pan2d = pVoice->Params.Pan2d;
            if( pElement->State == ELEMENT_PLAYING ) 
            {
                ApplyElementPan( pElement );
            }
        }
        else
        {
            // TODO: Warning message.
        }

        // Walk the list.
        pElement = pElement->Link.pNext;
    }
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::UpdateVoicePitch( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateVoicePitch" );

    element* pHead;
    element* pElement;

    // Calculate the voices volume.
    pVoice->Pitch = CalculateVoicePitch( pVoice );

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        // Stop each element.
        ASSERT( VALID_ELEMENT( pElement ) );

        // Calculate and apply the elements pitch.
        pElement->Pitch = CalculateElementPitch( pElement );
        if( pElement->State == ELEMENT_PLAYING ) 
        {        
            ApplyElementPitch( pElement );
        }

        // Walk the list.
        pElement = pElement->Link.pNext;
    }
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::UpdateVoiceEffectSend( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateVoiceEffectSend" );

    element* pHead;
    element* pElement;

    // Calculate the voices volume.
    pVoice->EffectSend = CalculateVoiceEffectSend( pVoice );

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        // Stop each element.
        ASSERT( VALID_ELEMENT( pElement ) );

        // Calculate and apply the elements pitch.
        pElement->EffectSend = CalculateElementEffectSend( pElement );
        if( pElement->State == ELEMENT_PLAYING ) 
        {        
            ApplyElementEffectSend( pElement );
        }

        // Walk the list.
        pElement = pElement->Link.pNext;
    }
}

//------------------------------------------------------------------------------

inline voice* audio_voice_mgr::UpdateStateRunning( voice* pVoice, f32 DeltaTime )
{
    CONTEXT( "audio_voice_mgr::UpdateStateRunning" );

    u32 Dirty;

    // Update the voices 3d stuff.
    if( pVoice->IsPositional )
        UpdateVoice3d( pVoice );

    // Is the voice releasing?
    if( pVoice->IsReleasing )
    {
        // Has the release state finished?
        if( pVoice->Params.Volume <= 0.0f )
        {
            // Free the voice, put it in the free list.
            FreeVoice( pVoice, TRUE );
            return NULL;
        }
        else
        {
            // Update volume by delta
            pVoice->Params.Volume -= (pVoice->DeltaVolume * DeltaTime);
            if( pVoice->Params.Volume < 0.0f )
                pVoice->Params.Volume = 0.0f;
            pVoice->Dirty |= VOICE_DB_VOLUME_CHANGE;
        }
    }

    // Local copy of dirty bits.
    Dirty = pVoice->Dirty;

    // Any dirty bits set?
    if( Dirty )
    {
        // Don't update the priority unless needed.
        s_UpdatePriority = FALSE;

        if( Dirty & (VOICE_DB_VOLUME_CHANGE+VOICE_DB_PAN_CHANGE) )
        {
            // Update the voices volume and pan (priority as well)
            UpdateVoiceVolumeAndPan( pVoice );

            // Clear the dirty bit.
            Dirty &= ~(VOICE_DB_VOLUME_CHANGE+VOICE_DB_PAN_CHANGE);

           // Need to update the voices priority.
            s_UpdatePriority = TRUE;
        }

        if( Dirty & VOICE_DB_VOLUME_CHANGE )
        {
            // Update the voices volume (and priority)
            UpdateVoiceVolume( pVoice );

            // Clear the dirty bit.
            Dirty &= ~VOICE_DB_VOLUME_CHANGE;

           // Need to update the voices priority.
            s_UpdatePriority = TRUE;
        }

        if( Dirty & VOICE_DB_PAN_CHANGE )
        {
            // Update the voices pan.
            UpdateVoicePan( pVoice );

            // Clear the dirty bit.
            Dirty &= ~VOICE_DB_PAN_CHANGE;
        }

        if( Dirty & VOICE_DB_PITCH_CHANGE )
        {
            // Update the voices pan.
            UpdateVoicePitch( pVoice );

            // Clear the dirty bit.
            Dirty &= ~VOICE_DB_PITCH_CHANGE;
        }

        if( Dirty & VOICE_DB_EFFECTSEND_CHANGE )
        {
            // Update the voices effect send.
            UpdateVoiceEffectSend( pVoice );

            // Clear the dirty bit.
            Dirty &= ~VOICE_DB_EFFECTSEND_CHANGE;
        }
        
        if( Dirty & VOICE_DB_RELEASE_TIME_CHANGE )
        {
            // Update the release time.
            UpdateReleaseTime( pVoice );

            // Clear the dirty bit
            Dirty &= ~VOICE_DB_RELEASE_TIME_CHANGE;
        }

        if( Dirty & VOICE_DB_ELEMENT_CHANGE )
        {
            // Calculate voice priority, find the next pending element...
            pVoice = UpdateCheckElements( pVoice );

            // Clear the dirty bit
            Dirty &= ~VOICE_DB_ELEMENT_CHANGE;
        }


        // Voice still around?
        if( pVoice )
        {
            // Update the dirty flags.
            pVoice->Dirty = Dirty;

            // Need to update the voices priority?
            if( s_UpdatePriority )
            {
                // Update the priority.
                PrioritizeVoice( pVoice, TRUE );
            }
        }
    }

    // Tell the world...
    return pVoice;
}

//------------------------------------------------------------------------------

inline void audio_voice_mgr::UpdateStatePausing( voice* pVoice, f32 Time )
{
    CONTEXT( "audio_voice_mgr::UpdateStatePausing" );

    element* pHead;
    element* pElement;

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        // Stop each element.
        ASSERT( VALID_ELEMENT( pElement ) );
        if( pElement->State == ELEMENT_PLAYING )
            PauseElement( pElement );

        // Walk the list.
        pElement = pElement->Link.pNext;
    }

    // Voice is stopped.
    pVoice->StopTime = Time;
    pVoice->State    = STATE_PAUSED;
}

//------------------------------------------------------------------------------

inline voice* audio_voice_mgr::UpdateCheckStreams( voice* pVoice )
{
    CONTEXT( "audio_voice_mgr::UpdateCheckStreams" );

    element* pHead;
    element* pElement;

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        pElement->bProcessed = FALSE;
        pElement = pElement->Link.pNext;
    }

    // For every element in the list...
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    while( pHead != pElement )
    {
        if( pElement->bProcessed == FALSE )
        {
            pElement->bProcessed = TRUE;

            // Element need to warm up?
            ASSERT( VALID_ELEMENT( pElement ) );
            if( pElement->State == ELEMENT_NEEDS_TO_LOAD )
            {
                ASSERT( pElement->Type == COLD_SAMPLE );
                
                // Time to warm it up?
                if( (pVoice->CursorTime + 1.5f) >= pElement->DeltaTime )
                {
                    audio_stream* pStream        = NULL;
                    element*      pLeftElement   = NULL;
                    element*      pRightElement  = NULL;
                    channel*      pLeftChannel   = NULL;
                    channel*      pRightChannel  = NULL;
                    
                    // Attempt to aquire a hardware channel.
                    if( pElement->pChannel == NULL )
                    {
                        pLeftElement = pElement;
                        pLeftElement->Params.Priority = 255;

                        if( g_AudioChannelMgr.Acquire( pLeftElement ) )
                        {
                            // Get left channel
                            pLeftChannel = pLeftElement->pChannel;

                            if( (pRightElement = pElement->pStereoElement) != NULL )
                            {
                                pRightElement->Params.Priority = 255;
                                if( g_AudioChannelMgr.Acquire( pRightElement ) )
                                {
                                    // Get right channel
                                    pRightChannel = pRightElement->pChannel;

                                    // Mark it processed
                                    pRightElement->bProcessed = TRUE;
                                }
                                else
                                {
                                    g_AudioChannelMgr.Release( pLeftElement->pChannel );
                                    pLeftElement->pChannel = NULL;
                                    return NULL;
                                    // Should NEVER get here!
                                    ASSERT( 0 );
                                }
                            }
                        }
                        else
                        {
                            return NULL;
                            BREAK;
                            // Should NEVER get here!
                            ASSERT( 0 );
                        }
                    }
                    else
                    {
                        pLeftElement = pElement;
                        pLeftChannel = pLeftElement->pChannel;
                        if( (pRightElement = pElement->pStereoElement) != NULL )
                            pRightChannel = pRightElement->pChannel;
                    }

                    // Try to aquire a stream.
                    pStream = g_AudioStreamMgr.AcquireStream( pElement->Sample.pColdSample->WaveformOffset,
                                                            pElement->Sample.pColdSample->WaveformLength,
                                                            pLeftChannel, pRightChannel );

                    if( pStream == COOLING_STREAM )
                    {
                        // Its cooling, so just chill out for a bit...
                    }
                    else if( pStream == NULL )
                    {
                        // Nuke the voice, put it in the freelist if the stream can't be started.
                        // TODO: Fix this so it just removes the elements.
                        FreeVoice( pVoice, TRUE );
                        return NULL;
                    }
                    else
                    {
                        // AHA! A stream is available!!!
                        ASSERT( pStream->pChannel[ LEFT_CHANNEL ] );
                        if( pStream->pChannel[ LEFT_CHANNEL ] )
                        {
                            // Instantiate the sample.
                            InstantiateStreamSample( pStream, LEFT_CHANNEL );

                            // Mark left as loading, set the aram
                            pLeftElement->State                        = ELEMENT_LOADING;
                            pLeftChannel->StreamData.pStream           = pStream;
                            pLeftChannel->StreamData.StreamControl     = TRUE;
                            pLeftChannel->StreamData.bStopLoop         = TRUE;
                            pLeftChannel->Sample.pHotSample->AudioRam  = pStream->ARAM[LEFT_CHANNEL][0];
                            pLeftChannel->Sample.pHotSample->LoopStart = 0;
                            pLeftChannel->Sample.pHotSample->LoopEnd   = STREAM_BUFFER_SIZE * 2;

                            // Init the channel.
                            g_AudioHardware.InitChannelStreamed( pLeftChannel );
                        }
                        else
                        {
                            FreeVoice( pVoice, TRUE );
                            return NULL;
                        }

                        /// Stereo?
                        if( pRightElement )
                        {
                            // Right channel will be the control.
                            // *** This important cause the right channel is operated on   ***
                            // *** last in the update.  The last channel to be operated on *** 
                            // *** MUST be the control!!!!                                 *** 
                            pLeftChannel->StreamData.StreamControl = FALSE;

                            // Instantiate the sample.
                            ASSERT( pStream->pChannel[RIGHT_CHANNEL] );
                            if( pStream->pChannel[RIGHT_CHANNEL] )
                            {
                                InstantiateStreamSample( pStream, RIGHT_CHANNEL );

                                // Mark right channel as loading, set the aram
                                pRightElement->State                        = ELEMENT_LOADING;
                                pRightChannel->StreamData.pStream           = pStream;
                                pRightChannel->StreamData.StreamControl     = TRUE;
                                pRightChannel->StreamData.bStopLoop         = TRUE;
                                pRightChannel->Sample.pHotSample->AudioRam  = pStream->ARAM[RIGHT_CHANNEL][0]; 
                                pRightChannel->Sample.pHotSample->LoopStart = 0;
                                pRightChannel->Sample.pHotSample->LoopEnd   = STREAM_BUFFER_SIZE * 2;

                                // Init the channel.
                                g_AudioHardware.InitChannelStreamed( pRightChannel );
                            }
                            else
                            {
                                FreeVoice( pVoice, TRUE );
                                return NULL;
                            }
                        }

                        // Warm it up!
                        pStream->bOpenStream = TRUE;
                    }
                }
            }
            // Finished loading?
            else if( pElement->State == ELEMENT_LOADED )
            {
                // Start loading the the second buffer.
                ASSERT( pElement->pChannel->StreamData.pStream );
                if( (!pElement->pChannel) ||
                    (!pElement->pChannel->StreamData.pStream) )
                {
                    FreeVoice( pVoice, TRUE );
                    return NULL;
                }
                if( !pElement->pChannel->StreamData.pStream->StreamDone )
                    g_AudioStreamMgr.ReadStream( pElement->pChannel->StreamData.pStream );

                // Mark it as ready.
                pElement->State = ELEMENT_READY;

                // Stereo? If so, mark stereo element as ready.
                if( pElement->pStereoElement )
                    pElement->pStereoElement->State = ELEMENT_READY;

                // Element has changed.
                pVoice->Dirty |= VOICE_DB_ELEMENT_CHANGE;

            }
        }

        // Walk the list.
        pElement = pElement->Link.pNext;
    }
    
    // Its all good!
    return pVoice;
}

//------------------------------------------------------------------------------

void audio_voice_mgr::Update( f32 DeltaTime )
{
    CONTEXT( "audio_voice_mgr::Update" );

    voice*   pHeadVoice;
    voice*   pVoice;
    voice*   pNextVoice;

    // Snag mutex.
    Lock();

    // Get head/tail of active voices, first active voice
    pHeadVoice = UsedVoices();
    pVoice     = pHeadVoice->Link.pNext;

    // For each active voice...
    while( pVoice != pHeadVoice )
    {
        // Get next voice.
        ASSERT( VALID_VOICE(pVoice) );
        pNextVoice = pVoice->Link.pNext;

        if( (void*)pVoice->Elements.pNext == (void*)&pVoice->Elements )
        {
            FreeVoice( pVoice, TRUE );
            pVoice = NULL;
        }
        else
        {
            // Need to warm up any streams?
            pVoice = UpdateCheckStreams( pVoice );
        }

        // What to do?
        if( pVoice )
        {
            switch( pVoice->State )
            {
                case STATE_NOT_STARTED:
#ifdef UPDATE_STATE_LOGGING
                    LOG_MESSAGE( UPDATE_STATE_LOGGING, "pVoice: %08x, State: STATE_NOT_STARTED", pVoice );
#endif
                    break;
 
                case STATE_STARTING:
#ifdef UPDATE_STATE_LOGGING
                    LOG_MESSAGE( UPDATE_STATE_LOGGING, "pVoice: %08x, State: STATE_STARTING", pVoice );
#endif
                    pVoice = UpdateStateStarting( pVoice );
                    if( pVoice )
                    {
                        pVoice->StartTime  = g_AudioMgr.m_Time;
                        pVoice = UpdateStateRunning( pVoice, 0.0f );
                    }
                    break;
 
                case STATE_RESUMING:
#ifdef UPDATE_STATE_LOGGING
                    LOG_MESSAGE( UPDATE_STATE_LOGGING, "pVoice: %08x, State: STATE_RESUMING", pVoice );
#endif
                    pVoice = UpdateStateResuming( pVoice );
                    if( pVoice )
                    {
                        pVoice->StartTime = g_AudioMgr.m_Time-pVoice->StopTime;
                        pVoice = UpdateStateRunning( pVoice, 0.0f );
                    }
                    break;
 
                case STATE_RUNNING:
#ifdef UPDATE_STATE_LOGGING
                    LOG_MESSAGE( UPDATE_STATE_LOGGING, "pVoice: %08x, State: STATE_RUNNING", pVoice );
#endif
                    // Update the voices cursor time.
                    pVoice->CursorTime += DeltaTime;
                    pVoice = UpdateStateRunning( pVoice, DeltaTime );
                    break;
 
                case STATE_PAUSING:
#ifdef UPDATE_STATE_LOGGING
                    LOG_MESSAGE( UPDATE_STATE_LOGGING, "pVoice: %08x, State: STATE_PAUSING", pVoice );
#endif
                    UpdateStatePausing( pVoice, g_AudioMgr.m_Time );
                    break;
 
                case STATE_PAUSED:
#ifdef UPDATE_STATE_LOGGING
                    LOG_MESSAGE( UPDATE_STATE_LOGGING, "pVoice: %08x, State: STATE_PAUSED", pVoice );
#endif
                    break;
            }
        }
    
        // Only if voice is still around and running...
        if( pVoice && (pVoice->State == STATE_RUNNING) )
        {
            // Attempt to start any pending elements...
            UpdateStartPending( pVoice );
        }

        // Walk the list.
        pVoice = pNextVoice;
    }

    // Release it.
    Unlock();
}


//------------------------------------------------------------------------------

void audio_voice_mgr::UpdateCheckQueued( void )
{
    voice* pVoice;
    voice* pHeadVoice;

    // Snag mutex.
    Lock();

    // Get head/tail of active voices, first active voice
    pHeadVoice = UsedVoices();
    pVoice     = pHeadVoice->Link.pNext;

    // For each active voice...
    while( pVoice != pHeadVoice )
    {
        // Get next voice.
        ASSERT( VALID_VOICE(pVoice) );

        if( pVoice->StartQ == 1 )
        {
            // Error check
            if( pVoice->State == STATE_NOT_STARTED )
            {
                UpdateStateStarting( pVoice );
                UpdateStateRunning( pVoice, 0.0f );
                UpdateStartPending( pVoice );
            }
            pVoice->StartQ = 2;
        }

        // Walk the list.
        pVoice = pVoice->Link.pNext;
    }

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------
s32 VOICE_PRIORITY_HIT=0;
s32 VOICE_PRIORITY_MISS=0;

void audio_voice_mgr::PrioritizeVoice( voice* pVoice, xbool RemoveFromList )
{
    // Error check.
    ASSERT( VALID_VOICE(pVoice) );

    // Check if where it currently is is valid
    xbool bRelocate = !RemoveFromList;

    voice* pRoot = (voice*)&UsedVoices()->Link;

    if( pVoice->Link.pPrev != pRoot && (bRelocate==FALSE)  )
    {
        voice* pPrev = pVoice->Link.pPrev;
        bRelocate = (
                     ( pVoice->Params.Priority > pPrev->Params.Priority ) ||
                     ((pVoice->Params.Priority == pPrev->Params.Priority) &&
                      (pVoice->Volume > pPrev->Volume ))
                    );
    }

    if( (pVoice->Link.pNext != pRoot) && (bRelocate==FALSE) )
    {
        voice* pNext = pVoice->Link.pNext;
        bRelocate = (
                     ( pVoice->Params.Priority < pNext->Params.Priority ) ||
                     ((pVoice->Params.Priority == pNext->Params.Priority) &&
                      (pVoice->Volume < pNext->Volume ))
                    );
    }

    if( bRelocate )
    {
        // Remove the channel from its current list?
        if( RemoveFromList )
            RemoveVoiceFromList( pVoice );

        // Get first used voice.
        voice* pInsert = UsedVoices()->Link.pNext;

        // Find the insertion point (based on priority only).
        while( pInsert->Params.Priority > pVoice->Params.Priority )
            pInsert = pInsert->Link.pNext;

        // Volume is secondary key.
        while( (pInsert->Params.Priority == pVoice->Params.Priority) && (pInsert->Volume > pVoice->Volume) )
            pInsert = pInsert->Link.pNext;

        // Insert it into the used list.
        InsertVoiceIntoList( pVoice, pInsert );

        VOICE_PRIORITY_MISS++;
    }
    else
    {
        VOICE_PRIORITY_HIT++;
    }

}

//------------------------------------------------------------------------------

void audio_voice_mgr::FreeVoice( voice* pVoice, xbool PutInFreeList )
{
    CONTEXT( "audio_voice_mgr::FreeVoice" );

    element* pElement;
    element* pHead;
    u32      Sequence;

    // Error check.
    ASSERT( VALID_VOICE(pVoice) );

    // Free up any elements in the voice.
    pHead    = (element*)&pVoice->Elements;
    pElement = pHead->Link.pNext;
    ASSERT( pElement );

    // Lock the audio hardware while we free the entire voice.
    g_AudioHardware.Lock();

    // For every element...
    while( pElement != pHead )
    {
        element* pNextElement = pElement->Link.pNext;
    
        // Release this element and the elements channel.
        ReleaseElement( pElement, TRUE );

        // Walk the list
        pElement = pNextElement;
    }

    // Let the hardware go.
    g_AudioHardware.Unlock();

    // Remove voice from the used list
    RemoveVoiceFromList( pVoice );

    // Bump sequence.
    Sequence = pVoice->Sequence+1;
    if( Sequence >= 32768 )
        Sequence = 1;

    // Nuke it.
    x_memset( pVoice, 0, sizeof(voice) );

    // Set sequence.
    pVoice->Sequence = Sequence;

    // Put it into the free list?
    if( PutInFreeList )
    {
        // I'm free!!!!
        InsertVoiceIntoList( pVoice, FreeVoices() );
    }
}

//------------------------------------------------------------------------------

void audio_voice_mgr::SetPackageVoicesDirty( audio_package* pPackage, u32 Bits )
{
    voice*   pHeadVoice;
    voice*   pVoice;

    // Get head/tail of active voices, first active voice
    pHeadVoice = UsedVoices();
    pVoice     = pHeadVoice->Link.pNext;

    // For each active voice...
    while( pVoice != pHeadVoice )
    {
        // Get next voice.
        ASSERT( VALID_VOICE(pVoice) );
        
        // Does this voice belong to the package?
        if( pVoice->pPackage == pPackage )
        {
            // Set the dirty bit.
            pVoice->Dirty |= Bits;
        }

        // Walk the list.
        pVoice = pVoice->Link.pNext;
    }
}

//------------------------------------------------------------------------------

void audio_voice_mgr::UpdateVoiceVolume( audio_package* pPackage )
{
    SetPackageVoicesDirty( pPackage, VOICE_DB_VOLUME_CHANGE );
}

//------------------------------------------------------------------------------

void audio_voice_mgr::UpdateVoicePitch( audio_package* pPackage )
{
    SetPackageVoicesDirty( pPackage, VOICE_DB_PITCH_CHANGE );
}

//------------------------------------------------------------------------------

void audio_voice_mgr::UpdateVoiceEffectSend( audio_package* pPackage )
{
    SetPackageVoicesDirty( pPackage, VOICE_DB_EFFECTSEND_CHANGE );
}

//------------------------------------------------------------------------------

void audio_voice_mgr::AppendElementToVoice( element* pElement, voice* pVoice )
{
    element* pHead;
    
    pHead = (element*)&pVoice->Elements;

    // Put it at end of list...
    pElement->Link.pNext          = pHead;
    pElement->Link.pPrev          = pHead->Link.pPrev;
    pHead->Link.pPrev->Link.pNext = pElement;
    pHead->Link.pPrev             = pElement;

    // Set elements voice pointer.
    pElement->pVoice = pVoice;

    // Element has changed.
    pVoice->Dirty |= VOICE_DB_ELEMENT_CHANGE;
}

//------------------------------------------------------------------------------

void audio_voice_mgr::InitSingleVoice( voice* pVoice, audio_package* pPackage )
{
    ASSERT( pVoice );
    ASSERT( pPackage );

    // Snag mutex.
    Lock();

    // Set the voices package.
    pVoice->pPackage = pPackage;

    // Empty the element list.
    pVoice->Elements.pNext =
    pVoice->Elements.pPrev = (element*)&pVoice->Elements;

    // No pending element, reset state.
    pVoice->pPendingElement = NULL;
    pVoice->State           = STATE_NOT_STARTED; 
    pVoice->CursorTime      = 0.0f;
    pVoice->IsReleasing     = FALSE;
    pVoice->DeltaVolume     = 0.0f;

    // Init the recursion depth
    pVoice->RecursionDepth = 0;

    // Clear the segue.
    pVoice->pSegueVoiceNext =
    pVoice->pSegueVoicePrev = NULL;
    pVoice->StartQ          = 0;
    pVoice->ReleaseTime     = 0.0f;

    // Calculate the voices parameters.
    pVoice->NearFalloff = CalculateVoiceNearFalloff( pVoice );
    pVoice->FarFalloff  = CalculateVoiceFarFalloff( pVoice );
    pVoice->NearDiffuse = CalculateVoiceNearDiffuse( pVoice );
    pVoice->FarDiffuse  = CalculateVoiceFarDiffuse( pVoice );
    pVoice->Volume      = CalculateVoiceVolume( pVoice );
    pVoice->Pitch       = CalculateVoicePitch( pVoice );
    pVoice->EffectSend  = CalculateVoiceEffectSend( pVoice );

    // Now set the voices dirty bits.
    pVoice->Dirty = VOICE_DB_ELEMENT_CHANGE +
                    VOICE_DB_PAN_CHANGE +
                    VOICE_DB_VOLUME_CHANGE +
                    VOICE_DB_PITCH_CHANGE +
                    VOICE_DB_EFFECTSEND_CHANGE +
                    VOICE_DB_ELEMENT_CHANGE;

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::InitSingleElement( element* pElement )
{
    s32 n;
    f32 Sign;

    // Snag mutex.
    Lock();

    // Calculate the elements parameters.
    pElement->PositionalVolume = 1.0f;
    
    // Volume variance defined?
    if( pElement->Params.VolumeVariance )
    {
        ASSERT( pElement->Params.VolumeVariance  > 0.0f );
        ASSERT( pElement->Params.VolumeVariance <= 1.0f );

        // Positive or negative?
        n = x_rand();
        if( n & 1 )
        {
            // Negative...
            Sign = -1.0f;
        }
        else
        {
            // Positive...
            Sign = 1.0f;
        }

        // Set the volume variance.
        pElement->VolumeVariance = 1.0f + (Sign * ((f32)(n % 100)) / 100.0f * pElement->Params.VolumeVariance);
    }
    else
    {
        pElement->VolumeVariance = 1.0f;
    }

    // Pitch variance defined?
    if( pElement->Params.PitchVariance )
    {
        ASSERT( pElement->Params.PitchVariance  > 0.0f );
        ASSERT( pElement->Params.PitchVariance <= 1.0f );

        // Positive or negative?
        n = x_rand();
        if( n & 1 )
        {
            // Negative...
            Sign = -1.0f;
        }
        else
        {
            // Positive...
            Sign = 1.0f;
        }

        // Set the pitch variance.
        pElement->PitchVariance = 1.0f + (Sign * ((f32)(n % 100)) / 100.0f * pElement->Params.VolumeVariance);
    }
    else
    {
        pElement->PitchVariance = 1.0f;
    }

    pElement->Volume           = CalculateElementVolume( pElement );
    pElement->Pitch            = CalculateElementPitch( pElement );
    pElement->EffectSend       = CalculateElementEffectSend( pElement );

    // Release it.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::InstantiateStreamSample ( audio_stream* pStream, s32 WhichChannel )
{
    CONTEXT( "audio_voice_mgr::InstantiateStreamSample" );

    compression_header* pHeader;

    switch( WhichChannel )
    {
        case LEFT_CHANNEL:
                ASSERT( pStream->pChannel[LEFT_CHANNEL] );
                // Get pointer to compression header.
                pHeader = (compression_header*)(pStream->pChannel[LEFT_CHANNEL]->Sample.pHotSample+1);

                // Make a copy of the sample
                pStream->Samples[LEFT_CHANNEL].Sample = *(pStream->pChannel[LEFT_CHANNEL]->Sample.pHotSample);

                // Make a copy of the compression header
                pStream->Samples[LEFT_CHANNEL].Header = *pHeader;
                
                // Point channel to the copy now
                pStream->pChannel[LEFT_CHANNEL]->Sample.pHotSample = &pStream->Samples[LEFT_CHANNEL].Sample;
                break;

        case RIGHT_CHANNEL:
                ASSERT( pStream->pChannel[RIGHT_CHANNEL] );

                if( pStream->pChannel[RIGHT_CHANNEL] )
                {
                    // Get pointer to compression header.
                    pHeader = (compression_header*)(pStream->pChannel[RIGHT_CHANNEL]->Sample.pHotSample+1);

                    // Make a copy of the sample
                    pStream->Samples[RIGHT_CHANNEL].Sample = *(pStream->pChannel[RIGHT_CHANNEL]->Sample.pHotSample);

                    // Make a copy of the compression header
                    pStream->Samples[RIGHT_CHANNEL].Header = *pHeader;
                    
                    // Point channel to the copy now
                    pStream->pChannel[RIGHT_CHANNEL]->Sample.pHotSample = &pStream->Samples[RIGHT_CHANNEL].Sample;
                }
                break;

        default:
            // Should never get here.
            ASSERT( 0 );
            break;
    }
}

//------------------------------------------------------------------------------

void audio_voice_mgr::UpdateAllVoiceVolumes( void )
{
    voice*   pHeadVoice;
    voice*   pVoice;

    // Snag the mutex.
    Lock();

    // Get head/tail of active voices, first active voice
    pHeadVoice = UsedVoices();
    pVoice     = pHeadVoice->Link.pNext;

    // For each active voice...
    while( pVoice != pHeadVoice )
    {
        // Gotta be valid...
        ASSERT( VALID_VOICE(pVoice) );
        
        // Set the dirty bit.
        pVoice->Dirty |= VOICE_DB_VOLUME_CHANGE;

        // Walk the list. NEXT!
        pVoice = pVoice->Link.pNext;
    }

    // Release mutex.
    Unlock();
}

//------------------------------------------------------------------------------

void audio_voice_mgr::SetPitchLock( voice* pVoice, xbool bPitchLock )
{
    // Snag the mutex.
    Lock();

    // Lock or unlock the pitch (as the case may be...)
    pVoice->bPitchLock = bPitchLock;

    // Release mutex.
    Unlock();
}

