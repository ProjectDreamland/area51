//=========================================================================
//
//  AnimAudioTimer.hpp
//
//  A simple class to smoothly predict the current elapsed time of a voice
//
//=========================================================================

#ifndef __ANIM_AUDIO_TIMER_HPP__
#define __ANIM_AUDIO_TIMER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================
#include "x_types.hpp"
#include "x_math.hpp"


//=========================================================================
// CLASSES
//=========================================================================
class anim_audio_timer
{
// Functions
public:
                anim_audio_timer();
                
        // Starts timer using audio
        void    Start           ( s32 VoiceID, guid CameraGuid, s32 iCameraBone );        
        
        // Stops timer
        void    Stop            ( void );
        
        // Returns TRUE if the voice is valid
        xbool   IsVoiceValid    ( void );
        
        // Returns TRUE if the voice has warmed up and is playing
        xbool   IsPlaying       ( void );

        // Returns camera local to world (if there is one) for specified time
        void    GetCameraBoneL2W( matrix4& L2W, f32 Time );
        
        // Updates the predicted time
        void    Advance         ( f32 DeltaTime );
        
        // Returns the current predicted time
        f32     GetTime         ( void ) const;
        
        // Returns the voice ID
        s32     GetVoiceID      ( void ) const;
        
// Data
protected:
        s32         m_VoiceID;              // Audio manager voice ID
        guid        m_CameraGuid;           // Camera guid object (or NULL_GUID if none)
        s32         m_iCameraBone;          // Camera bone (or -1 if none)
        f32         m_AudioPrevTime;        // Hardware voice elapsed time
        f32         m_AudioCurrTime;        // Hardware voice elapsed time
        f32         m_AudioPredictedTime;   // Smoothed out audio elapsed time
        f32         m_AnimPredictedTime;    // Smoothed out anim elapsed time
        f32         m_Time;                 // Final computed cinema time
        
};

//=========================================================================
// INLINE FUNCTIONS
//=========================================================================

// Returns the voice ID
inline
s32 anim_audio_timer::GetVoiceID( void ) const
{
    return m_VoiceID;
}

//=========================================================================

// Returns the cinema time
inline
f32 anim_audio_timer::GetTime( void ) const
{
    return m_Time;
}

//=========================================================================



#endif // __ANIM_AUDIO_TIMER_HPP__

