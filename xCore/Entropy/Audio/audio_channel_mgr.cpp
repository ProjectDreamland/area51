#include "audio_package.hpp"
#include "audio_channel_mgr.hpp"
#include "audio_voice_mgr.hpp"
#include "audio_hardware.hpp"
#include "audio_package.hpp"
#include "audio_inline.hpp"
#include "e_ScratchMem.hpp"
#include "x_log.hpp"

//------------------------------------------------------------------------------

static xbool s_IsInitialized = FALSE;        // Sentinel

//------------------------------------------------------------------------------

audio_channel_mgr g_AudioChannelMgr;

//------------------------------------------------------------------------------

audio_channel_mgr::audio_channel_mgr( void )
{
}

//------------------------------------------------------------------------------

audio_channel_mgr::~audio_channel_mgr( void )
{
}
        
//------------------------------------------------------------------------------

void audio_channel_mgr::Init( void )
{
    s32 i;
    s32 n;
    channel* pChannel;
    channel* pPrev;

    // Error check.
    ASSERT( s_IsInitialized == FALSE );

    // Init previous.
    pPrev = FreeList();

    // Get first channel and number of channels.
    pChannel = g_AudioHardware.GetChannelBuffer();
    n        = g_AudioHardware.NumChannels();

    // For each hardware channel...
    for( i=0 ; i<n ; i++, pChannel++ )
    {
        // Put link it.
        pChannel->Link.pPrev = pPrev;
        pChannel->Link.pNext = (pChannel+1);

        // Update previous.
        pPrev = pChannel;
    }

    // Back up one...
    pChannel = pPrev;

    // Last one is tail of free list.
    pChannel->Link.pNext = FreeList();

    // Initialize the free list.
    FreeList()->Link.pPrev = pChannel;
    FreeList()->Link.pNext = g_AudioHardware.GetChannelBuffer();

    // Initialize used list.
    UsedList()->Link.pPrev =
    UsedList()->Link.pNext = UsedList();

    // Head/Tail is now lowest priority.
    UsedList()->Priority = -1;

    // Set up fake hardware priority.
    // TODO: This is GCN only!
    g_AudioChannelMgr.UsedList()->Hardware.Priority = 1;

    // It's initialized!
    s_IsInitialized = TRUE;
}

//------------------------------------------------------------------------------

void audio_channel_mgr::Kill( void )
{
    // Error check.
    ASSERT( s_IsInitialized );

    // Not initialized anymore...
    s_IsInitialized = FALSE;
}

//------------------------------------------------------------------------------

xbool DEBUG_ACQUIRE_CHANNEL_FAIL = 0;

xbool audio_channel_mgr::Acquire( element* pElement )
{
    CONTEXT( "audio_channel_mgr::Acquire" );

    channel* pResult;
    channel* pHead;
    channel* pChannel;

    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_ELEMENT(pElement) );

    g_AudioHardware.Lock();

    // Get head/tail of free list and first free channel.
    pHead    = FreeList();
    pChannel = pHead->Link.pNext;

    // Is free list empty?
    if( pChannel == pHead )
    {
        // Get oldest, lowest priority channel.
        pChannel = UsedList()->Link.pPrev;

        // Which channel is more important?
        if( (pChannel->Priority < pElement->Params.Priority) )
//        || ((pChannel->Priority == pElement->Params.Priority) && (pChannel->Volume <= pElement->Params.Volume)) ) 
        {
#if defined(rbrannon)
            extern channel* g_DebugChannel;
            if( pChannel == g_DebugChannel )
            {
                LOG_WARNING( "AudioDebug(audio_channel_mgr::Acquire)",
                             "Freed g_DebugChannel!" );
                LOG_FLUSH();
            }
#endif // defined(rbrannon)

            // Free the channel, don't put it in the free list, nuke the element.
            Free( pChannel, FALSE, TRUE );

            // Get outta my way!  I'm more important!
            pResult = pChannel;
        }
        else
        {
            // Cannot aquire a channel...
            pResult = NULL;
        }
    }
    else
    {
        // Take it out of the free list.
        RemoveChannelFromList( pChannel );

        // Use this one!
        pResult = pChannel;
    }

    // Was a channel aquired?
    if( pResult )
    {
        // Init state and dirty bits.
        pResult->State = STATE_NOT_STARTED;
        pResult->Dirty = 0;

        // Init the stream.
        pResult->StreamData.pStream = NULL;

        // Set pointer to parent voice element.
        pResult->pElement           = pElement;

        // Clear the release position.
        pResult->ReleasePosition    = 0;

        // Inherit data from the voice element.
        pResult->Priority   = pElement->Params.Priority;
        pResult->Type       = pElement->Type;
        pResult->Sample     = pElement->Sample;
        pResult->Volume     = pElement->Volume; 
        pResult->Pitch      = pElement->Pitch;
        pResult->EffectSend = pElement->EffectSend;
        pResult->Pan2d      = pElement->Params.Pan2d;
        pResult->Pan3d      = pElement->Params.Pan3d;

        // Clear the read stream flag.
        pResult->StreamData.bReadStream= FALSE;

        // Insert it into the used list based on the priority/volume.
        UpdatePriorityList( pResult, FALSE );

        // Fake the hardware priority (until callback runs, GCN only).
        // Do this by using the priority of the next channel.
        g_AudioHardware.DuplicatePriority( pResult, pResult->Link.pNext );

        // Aquire the hardware channel, if we can...
        if( g_AudioHardware.AcquireChannel( pResult ) )
        {
            // Can initialize hot samples...
            if( pElement->Type == HOT_SAMPLE )
            {
                // Now initilize the hardware channel.
                g_AudioHardware.InitChannel( pResult );
            }

            // Set elements channel.
            pElement->pChannel = pResult;
        }
        // DOH! Could not aquire a hardware channel...
        else
        {
            // Take it out of the used list.
            RemoveChannelFromList( pResult );

             // Put channel back in free list.
            InsertChannelIntoList( pResult, FreeList() );

            // Too bad...so sad...
            pResult = NULL;
        }
    }

    g_AudioHardware.Unlock();

    #if !defined(X_RETAIL) || defined(X_QA)
    if( pResult == NULL && DEBUG_ACQUIRE_CHANNEL_FAIL )
    {
        if( pElement && pElement->pVoice )
        {
            x_DebugMsg( 7, "%s Failed to acquire hardware channel!", pElement->pVoice->pDescriptorName );
        }
    }
    #endif // !defined(X_RETAIL)

    // Tell the world.
    return( pResult != NULL );
}

//------------------------------------------------------------------------------

void audio_channel_mgr::Release( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    g_AudioHardware.Lock();

    // Free the channel, put it into the free channel list, Don't nuke the element.
    Free( pChannel, TRUE, FALSE );

    g_AudioHardware.Unlock();
}
        
//------------------------------------------------------------------------------

void audio_channel_mgr::Start( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    // Set state to starting.
    pChannel->State = STATE_STARTING;
}

//------------------------------------------------------------------------------

void audio_channel_mgr::Pause( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    // Set the state to stopping
    pChannel->State = STATE_PAUSING;
}

//------------------------------------------------------------------------------

void audio_channel_mgr::Resume( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    // Set the state to stopping.
    pChannel->State = STATE_RESUMING;
}

//------------------------------------------------------------------------------

xbool audio_channel_mgr::IsPlaying( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    return g_AudioHardware.IsChannelActive( pChannel );
}

//------------------------------------------------------------------------------

s32 audio_channel_mgr::GetPriority( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    return( pChannel->Priority );
}

//------------------------------------------------------------------------------

void audio_channel_mgr::SetPriority( channel* pChannel, s32 Priority ) 
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );
    ASSERT( (Priority >= 0) && (Priority <= 255) );

    g_AudioHardware.Lock();

    // Set the channels priority.
    pChannel->Priority = Priority;

    // Update the channels position in the used list.
    UpdatePriorityList( pChannel, TRUE );
    
    g_AudioHardware.Unlock();
}

//------------------------------------------------------------------------------

f32 audio_channel_mgr::GetVolume( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    return( pChannel->Volume );
}

//------------------------------------------------------------------------------

void audio_channel_mgr::SetVolume( channel* pChannel, f32 Volume )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    g_AudioHardware.Lock();

    // Limit the volume.
    if( Volume < 0.0f )
    {
        Volume = 0.0f;

        // TODO: Issue warning.
    }

    if( Volume > 1.0f )
    {
        Volume = 1.0f;

        // TODO: Issue warning.
    }

    // Set the volume.
    pChannel->Volume = Volume;

    // Update the channels position in the used list.
    UpdatePriorityList( pChannel, TRUE );
    
    // Set the dirty bit.
    pChannel->Dirty |= CHANNEL_DB_VOLUME;

    g_AudioHardware.Unlock();
}

//------------------------------------------------------------------------------

void audio_channel_mgr::GetPan( channel* pChannel, vector4& Pan )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    Pan = pChannel->Pan3d;
}

//------------------------------------------------------------------------------

void audio_channel_mgr::SetPan( channel* pChannel, vector4& Pan )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    // Set the pan
    pChannel->Pan3d = Pan;

    // Set the dirty bit
    pChannel->Dirty |= CHANNEL_DB_PAN;
}

//------------------------------------------------------------------------------

f32 audio_channel_mgr::GetPitch( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    return( pChannel->Pitch );
}

//------------------------------------------------------------------------------

void audio_channel_mgr::SetPitch( channel* pChannel, f32 Pitch )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    if( Pitch > 2.0f )
    {
        Pitch = 2.0f;
        // TODO: Put in warning.
    }
    else if( Pitch < 1.0f/64.0f )
    {
        Pitch = 1.0f/64.0f;
        // TODO: Put in warning.
    }

    // Set the pitch
    pChannel->Pitch = Pitch;

    // Set the dirty bit
    pChannel->Dirty |= CHANNEL_DB_PITCH;
}

//------------------------------------------------------------------------------

f32 audio_channel_mgr::GetEffectSend( channel* pChannel )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    return( pChannel->EffectSend );
}

//------------------------------------------------------------------------------

void audio_channel_mgr::SetEffectSend( channel* pChannel, f32 EffectSend )
{
    // Error check.
    ASSERT( s_IsInitialized );
    ASSERT( VALID_CHANNEL(pChannel) );

    // Limit the effect send.
    if( EffectSend < 0.0f )
    {
        EffectSend = 0.0f;

        // TODO: Issue warning.
    }

    if( EffectSend > 1.0f )
    {
        EffectSend = 1.0f;

        // TODO: Issue warning.
    }

    // Set the volume.
    pChannel->EffectSend = EffectSend;

    // Set the dirty bit.
    pChannel->Dirty |= CHANNEL_DB_EFFECTSEND;
}

//------------------------------------------------------------------------------

void audio_channel_mgr::Update( void )
{
    CONTEXT( "audio_channel_mgr::Update" );

    channel* pChannel;
    channel* pHead;

    g_AudioHardware.Lock();
    {
        CONTEXT( "audio_channel_mgr::UpdateLocked" );
        // Get head/tail of used channel list and first member.
        pHead    = UsedList();
        pChannel = pHead->Link.pNext;

        // For each channel...
        while( pChannel != pHead )
        {
            channel* pNext;

            // Error check.
            ASSERT( VALID_CHANNEL(pChannel) );

            // Get next used channel.
            pNext = pChannel->Link.pNext;

            // Was the hardware channel lost?
            if( !g_AudioHardware.IsChannelActive( pChannel) )
            {
                element* pElement;
                element* pStereo=NULL;

                g_AudioVoiceMgr.Lock();

                // Get the channels element.
                pElement = pChannel->pElement;
            
                // Get the elements stereo partner.
                if( pElement )
                    pStereo = pElement->pStereoElement;

                // Free the channel, put it in the free channel list, nuke the element as well.
                Free( pChannel, TRUE, TRUE );

                // Did channel have an element?
                if( pElement )
                {
                    // Was the element stereo?
                    ASSERT( VALID_ELEMENT(pElement) );
                    if( pStereo )
                    {
                        // Get the stereo channel.
                        ASSERT( VALID_ELEMENT(pStereo) );
                        pChannel = pStereo->pChannel;

                        // Only if the stereo channel is valid...
                        if( pChannel )
                        {
                            // Error check.
                            ASSERT( VALID_CHANNEL(pChannel) );

                            // Are we about to nuke the next?
                            if( pChannel == pNext )
                            {
                                // Keep walking...
                                pNext = pChannel->Link.pNext;
                            }

                            // Stereo channel active?
                            if( g_AudioHardware.IsChannelActive( pChannel) )
                            {
                                // Free the stereo channel, put it in the free channel list, nuke the element as well.
                                Free( pChannel, TRUE, TRUE );
                            }
                        }
                    }
                }

                g_AudioVoiceMgr.Unlock();

            }

            // Next!
            pChannel = pNext;
        }
    }

    g_AudioHardware.Unlock();
}

//------------------------------------------------------------------------------

void audio_channel_mgr::Free( channel* pChannel, xbool PutInFreeList, xbool FreeParent )
{
    element* pElement;

    // Error check.
    ASSERT( VALID_CHANNEL(pChannel) );
    ASSERT( g_AudioHardware.CanModifyChannelList() );

    // Nuke the hardware channel.
    g_AudioHardware.Lock();
    
    ASSERT(pChannel->pElement->Type != (element_type)-1);
    g_AudioHardware.ReleaseChannel( pChannel );
    g_AudioHardware.Unlock();

    // Get the channels element.
    pElement = pChannel->pElement;
    ASSERT(pElement->Type != (element_type)-1);

    // Now remove it from the used list.
    RemoveChannelFromList( pChannel );

    // Put it into the free channel list?
    if( PutInFreeList )
    {
        // Put the channel into freelist.
        InsertChannelIntoList( pChannel, FreeList() );

        // Clear the channels element (it doesn't need this in the free list...).
        pChannel->pElement = NULL;
    }

    // Element valid?
    if( FreeParent && pElement )
    {
        // Release the element, don't recurse and release the channel..hehe...
        ASSERT( VALID_ELEMENT(pElement) );
        g_AudioVoiceMgr.ReleaseElement( pElement, FALSE );
    }

   // Note the need to update the priorities in the audio callback.
   g_AudioHardware.SetDirtyBit( CALLBACK_DB_PRIORITY );
}

//------------------------------------------------------------------------------

void audio_channel_mgr::UpdatePriorityList( channel* pChannel, xbool RemoveFromList )
{
    channel* pInsert;

    // Error check.
    ASSERT( VALID_CHANNEL(pChannel) );
    ASSERT( g_AudioHardware.CanModifyChannelList() );

    // Remove the channel from the used list?
    if( RemoveFromList )
    {
        RemoveChannelFromList( pChannel );
    }

    // Get first used channel.
    pInsert = UsedList()->Link.pNext;

    // Find the insertion point (based on priority only).
    while( pInsert->Priority > pChannel->Priority )
        pInsert = pInsert->Link.pNext;

    // Volume is secondary key.
    while( (pInsert->Priority == pChannel->Priority) && (pInsert->Volume > pChannel->Volume) )
        pInsert = pInsert->Link.pNext;

    // Insert it into the used list.
    InsertChannelIntoList( pChannel, pInsert );

    // Note the need to update the hardware priorities in the audio callback.
    g_AudioHardware.SetDirtyBit( CALLBACK_DB_PRIORITY );
}
