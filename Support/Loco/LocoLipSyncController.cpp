//=========================================================================
//
//  LocoLipSyncController.hpp
//
//  Allows masked animation playback on top of current animation
//
//=========================================================================

#include "Entropy.hpp"
#include "Loco.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"


//=========================================================================
// FUNCTIONS
//=========================================================================

loco_lip_sync_controller::loco_lip_sync_controller() : loco_mask_controller()
{
    m_BlendInTime  = 0.0f;     // Time to blend anim in
    m_BlendOutTime = 0.5f;     // Time to blend anim out

    m_VoiceID      = 0;        // Voice ID of attached audio
}

//=========================================================================

loco_lip_sync_controller::~loco_lip_sync_controller()
{
}

//=========================================================================

// Sets a new animation and starts the audio
void loco_lip_sync_controller::SetAnim( const anim_group::handle& hAnimGroup, s32 iAnim, u32 VoiceID, u32 Flags )
{
    // Keep for later so we can stop it
    u32 PrevVoiceID = m_VoiceID;

    // Keep the voice ID
    m_VoiceID = VoiceID;

    // Call base class to start anim
    loco_mask_controller::SetAnim( hAnimGroup, iAnim, Flags );

    // Clear the weight ready for blending in
    m_Weight = 0;
    m_bIsBlendingIn = TRUE;

    // Clear bone mask blending
    m_BoneBlend       = 0.0f;
    m_BoneBlendDelta  = 0.0f;
    m_pBlendBoneMasks = NULL;

    // Stop current sound ( if any )
    g_AudioMgr.Release( PrevVoiceID, 0.0f );
    
    // Start anim audio timer
    m_Timer.Start( VoiceID, NULL_GUID, -1 );
}

//=========================================================================

// Sets a new animation and starts the audio
void loco_lip_sync_controller::SetAnim( const anim_group::handle& hAnimGroup, s32 iAnim, const char* pAudioName, u32 Flags )
{
    // Try start the sound if not in the artist viewer ( lip sync controller will start it when event is found )
    u32 VoiceID = g_AudioMgr.Play( pAudioName, ( Flags & loco::ANIM_FLAG_ARTIST_VIEWER ) == 0 );     // Name, AutoStart

    // Call main function
    SetAnim( hAnimGroup, iAnim, VoiceID, Flags );
}

//=========================================================================

void loco_lip_sync_controller::Advance( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw )
{
    // Clear blending status
    m_bIsBlendingIn  = FALSE;
    m_bIsBlendingOut = FALSE;

    // Skip if no anim
    if ( m_iAnim == -1 )
        return;

    // Lookup anim info
    const anim_info& AnimInfo = GetAnimInfo();

    // Lookup lip sync start frame event ( if any )
    f32 LipSyncStartFrame = AnimInfo.FindLipSyncEventStartFrame();
    if( LipSyncStartFrame < 0.0f )
        LipSyncStartFrame = 0.0f;

    // Advance bone blends
    m_BoneBlend += m_BoneBlendDelta * nSeconds;
    if( m_BoneBlend < 0.0f )
    {
        // End blending
        m_BoneBlend       = 0.0f;
        m_BoneBlendDelta  = 0.0f;
        m_pBlendBoneMasks = NULL;
    }

    // Lookup blend times
    f32   BlendInTime  = m_BlendInTime;
    f32   BlendOutTime = m_BlendOutTime;
    
    // If full body, then it will go straight onto the main anim controller so no blend time!
    if ( m_AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY )
        BlendInTime = BlendOutTime = 0.0f;

    // Update anim audio timer
    m_Timer.Advance( nSeconds );
        
    // Has the animation finished?
    if( IsAtEnd() )        
    {
        // Turn relative mode off for full body cinema
        if(     ( m_AnimFlags & loco::ANIM_FLAG_CINEMA )
            &&  ( m_AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY ) )
        {
            SetCinemaRelativeMode( FALSE );
        }

        // Allow updated from normal logic now the audio has ended
        m_Rate = 1.0f;

        // Advance the base class
        // NOTE: I'm not calling the base class "loco_mask_controller::Advance" because
        //       it updates the m_Weight in a different way than we want
        loco_anim_controller::Advance( nSeconds, DeltaPos, DeltaYaw );

        // Blend out
        m_bIsBlendingIn  = FALSE;
        m_bIsBlendingOut = TRUE;
    }
    // Is sound playing?        
    else if( m_Timer.IsVoiceValid() )
    {
        // Turn relative mode on for full body cinema
        if(     ( m_AnimFlags & loco::ANIM_FLAG_CINEMA )
            &&  ( m_AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY ) )
        {
            SetCinemaRelativeMode( TRUE );
        }

        // If waiting for lip sync event, let the animation run
        ASSERT( LipSyncStartFrame >= 0.0f );
        if( ( LipSyncStartFrame > 0.0f ) && ( m_Frame <= LipSyncStartFrame ) )
        {
            // Setup rate to advance anim as normal
            m_Rate = 1.0f;
            
            // Advance the base class
            // NOTE: I'm not calling the base class "loco_mask_controller::Advance" because
            //       it updates the m_Weight in a different way than we want
            loco_anim_controller::Advance( nSeconds, DeltaPos, DeltaYaw );
        }
        else
        {            
            // Update comes from cinema audio timer so stop logic updating the frame
            // NOTE: Only do this if it's a cinema, otherwise none cinema npcs will not move 
            // because the motion controller will not advance any and follow the motion prop!
            if( m_AnimFlags & loco::ANIM_FLAG_CINEMA )
                m_Rate = 0.0f;
        }
                
        // Default to blending in when audio is playing
        m_bIsBlendingIn = TRUE;

        // Update frame? (if audio is playing and it's not a cinema since cinema's update the frame)
        if(         ( ( m_AnimFlags & loco::ANIM_FLAG_CINEMA ) == 0 )
                &&  ( m_Timer.IsPlaying() ) )
        {        
            // Compute animation frame from predicted anim time
            f32 Frame = ( m_Timer.GetTime() *  ( f32 )AnimInfo.GetFPS() );
            Frame += LipSyncStartFrame;

            // Past the end of the anim?
            f32 EndFrame = (f32)( AnimInfo.GetNFrames() - 2 );
            if( Frame > EndFrame )
            {
                // Keep looping the anim?
                if( AnimInfo.DoesLoop() )
                {
                    // Loop
                    while( Frame > EndFrame )
                        Frame -= EndFrame;
                }
                else
                {
                    // Clamp to the last frame
                    Frame = EndFrame;
                    
                    // Anim has finished before the audio so blend out
                    m_bIsBlendingIn  = FALSE;
                    m_bIsBlendingOut = TRUE;
                }
            }                

            // Set position
            SetFrame( Frame );
        }
        else
        {
            // If this is the artist viewer and the start event has been found, start the audio!
            if( m_AnimFlags & loco::ANIM_FLAG_ARTIST_VIEWER )
            {
                // Past event start frame?
                if( m_Frame >= LipSyncStartFrame )
                {
                    // Sync up and start the audio
                    m_Frame = LipSyncStartFrame;
                    g_AudioMgr.Start( m_VoiceID );
                }                    
            }
        }
    }
    else
    {
        // Turn relative mode off for full body cinema
        if(     ( m_AnimFlags & loco::ANIM_FLAG_CINEMA )
            &&  ( m_AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY ) )
        {
            SetCinemaRelativeMode( FALSE );
        }
                    
        // Allow updated from normal logic now the audio has ended
        m_Rate = 1.0f;

        // Advance the base class
        // NOTE: I'm not calling the base class "loco_mask_controller::Advance" because
        //       it updates the m_Weight in a different way than we want
        loco_anim_controller::Advance( nSeconds, DeltaPos, DeltaYaw );
        
        // Has animation finished?
        if ( IsAtEnd() )
        {
            // Blend out
            m_bIsBlendingOut = TRUE;
        }
        else
        {
            // Blend in
            m_bIsBlendingIn = TRUE;
        }            
    }

    // Blend in?
    if( m_bIsBlendingIn )
    {
        // Use blend time?
        if ( BlendInTime > 0.0f )
        {
            m_Weight += nSeconds / BlendInTime;
            if( m_Weight > 1.0f )
                m_Weight = 1;
        }
        else
            m_Weight = 1;
    }
    
    // Blend out?
    if( m_bIsBlendingOut )
    {
        // Use blend time?
        if ( BlendOutTime > 0.0f )
            m_Weight -= nSeconds / BlendOutTime;
        else
            m_Weight = 0.0f;            
            
        // Turn off?            
        if( m_Weight <= 0 )
        {
            // Turn off anim and controller
            m_iAnim  = -1;
            m_Weight = 0;

            // Clear bone mask blending
            m_BoneBlend       = 0.0f;
            m_BoneBlendDelta  = 0.0f;
            m_pBlendBoneMasks = NULL;

            // Make sure voice ID is invalid so advance does not start setting anim frames
            m_VoiceID = 0;
            m_Timer.Stop();
        }
    }
}
   
//=========================================================================

// Clears the animation to a safe unused state
void loco_lip_sync_controller::Clear( void )
{
    // Call base class
    loco_mask_controller::Clear();    
}

//=========================================================================

// Stops lip sync controller
void loco_lip_sync_controller::Stop( void )
{
    // Only stop audio if this did not come from a cinema
    if( ( m_VoiceID ) && ( ( m_AnimFlags & loco::ANIM_FLAG_CINEMA ) == 0 ) )
        g_AudioMgr.Release( m_VoiceID, 0.0f );
    
    // Make sure voice ID is invalid so advance does not start setting anim frames
    m_VoiceID = 0;

    // Stop the audio anim timer    
    m_Timer.Stop();
    
    // Make sure relative mode is turned off for cinemas   
    if(     ( m_AnimFlags & loco::ANIM_FLAG_CINEMA )
        &&  ( m_AnimFlags & loco::ANIM_FLAG_MASK_TYPE_FULL_BODY ) )
    {    
        SetCinemaRelativeMode( FALSE );
    }
        
    // Let advance blend out the animation now the voice id has been released
}

//=========================================================================
