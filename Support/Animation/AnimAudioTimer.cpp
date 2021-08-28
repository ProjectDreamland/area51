//=========================================================================
//
//  AnimAudioTimer.cpp
//
//  A simple class to smoothly predict the current elapsed time of a voice
//
//=========================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Entropy.hpp"
#include "AnimAudioTimer.hpp"
#include "AnimPlayer.hpp"
#include "Obj_mgr\obj_mgr.hpp"


//==============================================================================
//  DEFINES
//==============================================================================

// Camera cut detection thresholds
static f32 CAMERA_CUT_POS_THRESHOLD = 50.0f;
static f32 CAMERA_CUT_ROT_THRESHOLD = R_10;

// Time error defines
static f32 MAX_ERROR_TIME = 2.5f / 30.0f;   // Max error between predicted audio and anim time
static f32 MAX_ERROR_FIX  = 1.0f / 3.0f;    // Max error fix for catching up to audio time


//==============================================================================
//  FUNCTIONS
//==============================================================================

 anim_audio_timer::anim_audio_timer()
 {
    m_VoiceID               = 0;            // Audio manager voice ID
    m_CameraGuid            = NULL_GUID;    // Camera guid object (or NULL_GUID if none)
    m_iCameraBone           = -1;           // Camera bone (or -1 if none)
    m_AudioPrevTime         = 0.0f;         // Hardware voice elapsed time
    m_AudioCurrTime         = 0.0f;         // Hardware voice elapsed time
    m_AudioPredictedTime    = 0.0f;         // Smoothed out audio elapsed time
    m_AnimPredictedTime     = 0.0f;         // Smoothed out anim elapsed time
    m_Time                  = 0.0f;         // Final cinema time
 }
 
//==============================================================================

// Starts timer using audio
void anim_audio_timer::Start( s32 VoiceID, guid CameraGuid, s32 iCameraBone )
{
    m_VoiceID               = VoiceID;      // Audio manager voice ID
    m_CameraGuid            = CameraGuid;   // Camera guid object (or NULL_GUID if none)
    m_iCameraBone           = iCameraBone;  // Camera bone (or -1 if none)
    m_AudioPrevTime         = 0.0f;         // Hardware voice elapsed time from last frame
    m_AudioCurrTime         = 0.0f;         // Hardware voice elapsed time
    m_AudioPredictedTime    = 0.0f;         // Smoothed out audio elapsed time
    m_AnimPredictedTime     = 0.0f;         // Smoothed out anim elapsed time
    m_Time                  = 0.0f;         // Final cinema time
}

//==============================================================================

// Stops timer
void anim_audio_timer::Stop( void )
{
    // Clear voice ID
    m_VoiceID = 0;
}

//==============================================================================

// Returns TRUE if the voice is valid
xbool anim_audio_timer::IsVoiceValid( void )
{
    xbool bIsValid = g_AudioMgr.IsValidVoiceId( m_VoiceID );
    return bIsValid;
}

//==============================================================================

// Returns TRUE if the voice has warmed up and is playing
xbool anim_audio_timer::IsPlaying( void )
{
    xbool bIsValid      = g_AudioMgr.IsValidVoiceId( m_VoiceID );
    xbool bAudioRunning = ( m_AudioCurrTime > 0.0f );
    return( bIsValid && bAudioRunning );
}

//==============================================================================

// Returns camera local to world (if there is one) for specified time
void anim_audio_timer::GetCameraBoneL2W( matrix4& L2W, f32 Time )
{
    // Setup to default
    L2W.Identity();
 
    // No camera bone?
    if( m_iCameraBone == -1 )
        return;
        
    // Lookup camera
    object* pCamera = g_ObjMgr.GetObjectByGuid( m_CameraGuid );
    if( !pCamera )
        return;

    // Lookup animation player
    simple_anim_player* pAnimPlayer = pCamera->GetSimpleAnimPlayer();
    if( !pAnimPlayer )
        return;

    // Lookup current animation
    s32 iAnim = pAnimPlayer->GetAnimIndex();
    if( iAnim == -1 )
        return;
        
    // Lookup anim group
    const anim_group* pAnimGroup = pCamera->GetAnimGroupPtr();
    if( !pAnimGroup )
        return;
    
    // Lookup animation info
    const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( iAnim );
    
    // Convert time to integer frame
    s32 iFrame = (s32)( (f32)AnimInfo.GetFPS() * Time );
    
    // Compute bone L2W
    s32 iBone = m_iCameraBone;
    while( iBone != -1 )
    {
        // Lookup key
        anim_key Key;
        AnimInfo.GetRawKey( iFrame, iBone, Key );
        
        // Compute key L2W
        matrix4 KeyL2W;
        Key.Setup( KeyL2W );
        
        // Append onto bone
        L2W = KeyL2W * L2W;
        
        // Traverse up hierarchy chain
        iBone = pAnimGroup->GetBoneParent( iBone );
    }
    
    // Apply bind matrix
    L2W = L2W * pAnimGroup->GetBone( m_iCameraBone ).BindMatrixInv;
}

//==============================================================================

// Updates the predicted time
void anim_audio_timer::Advance( f32 DeltaTime )
{
    // Paused?
    if( DeltaTime == 0.0f )
        return;
        
    // Read sound position first incase voice becomes invalid
    // (also truncate to nearest animation frame for 30FPS playback)
    m_AudioCurrTime = g_AudioMgr.GetCurrentPlayTime( m_VoiceID );

    // Is sound playing?        
    if( g_AudioMgr.IsValidVoiceId( m_VoiceID ) )
    {
        // Read hardware value and make sure it's valid
/* // Removed this assert - rbrannon
        ASSERTS( ( ( m_AudioCurrTime == 0.0f ) || 
                   ( m_AudioCurrTime >= m_AudioPrevTime ) ), 
                  xfs("Audio Stream: %0.5f %0.5f", m_AudioPrevTime, m_AudioCurrTime ) );
*/

        // Is audio playing yet?
        if( m_AudioCurrTime > 0.0f )
        {        
            // Compute hardware delta time
            f32 AudioDeltaTime = m_AudioCurrTime - m_AudioPrevTime;
            m_AudioPrevTime = m_AudioCurrTime;

            // Compute smoothed out audio time
            if( AudioDeltaTime != 0.0f )
                m_AudioPredictedTime = m_AudioCurrTime;    // Audio thread updated
            else
                m_AudioPredictedTime += DeltaTime;          // Audio thread has blocked so predict using frame dt

            // Compute smoothed out anim time using frame dt
            m_AnimPredictedTime += DeltaTime;     
            
            // If this is the first time, sync up the anim exactly
            if( m_AudioPrevTime == 0.0f )                           
                m_AnimPredictedTime = m_AudioCurrTime;

            // Compute error of predicted anim time from predicted audio time
            f32 Error = m_AudioPredictedTime - m_AnimPredictedTime;

            // More than "x" frames out?
            if( x_abs( Error ) > MAX_ERROR_TIME )
            {
                // Limit error so it's applied over several frames to avoid pops
                f32 MaxError = DeltaTime * MAX_ERROR_FIX;
                Error = x_clamp( Error, -MaxError, MaxError );

                // Apply error adjustment
                m_AnimPredictedTime += Error;
            }
        }            
    }
    
    // Keep for later
    f32 PreviousTime = m_Time;
    
    // Start with using predicted animation time
    m_Time = m_AnimPredictedTime;

    // Lookup current and next frames of camera local to world
    matrix4 CurrL2W, NextL2W;
    GetCameraBoneL2W( CurrL2W, m_Time );
    GetCameraBoneL2W( NextL2W, m_Time + ( 1.0f / 30.0f ) );

    // Lookup camera position and rotation info
    vector3 CurrPos = CurrL2W.GetTranslation();
    vector3 NextPos = NextL2W.GetTranslation();
    vector3 CurrDir = CurrL2W * vector3( 0.0f, 0.0f, 1.0f );
    vector3 NextDir = NextL2W * vector3( 0.0f, 0.0f, 1.0f );
    xbool   bSnap   = FALSE;
    
    // Position exceeded cut threshold?
    vector3 DeltaPos = CurrPos - NextPos; 
    if( DeltaPos.LengthSquared() >= x_sqr( CAMERA_CUT_POS_THRESHOLD ) )
    {
        bSnap = TRUE;
    }

    // Rotation exceeded cut threshold?
    radian DeltaAngle = v3_AngleBetween( CurrDir, NextDir );
    if( x_abs( DeltaAngle ) >= CAMERA_CUT_ROT_THRESHOLD )
    {
        bSnap = TRUE;
    }

    // Turn off interpolation?
    if( bSnap )
    {
        // Truncate to next animation frame
        m_Time += 1.0f / 30.0f;
        m_Time = x_floor( m_Time * 30.0f ) / 30.0f;
        
#ifdef X_EDITOR
        // Flash text
        x_printfxy( 10,10, "******** CAMERA CUT ********" );
#endif        
    }
    
    // Do not let the time go backwards otherwise events will not fire off
    if( m_Time < PreviousTime )
        m_Time = PreviousTime;
}

//==============================================================================
