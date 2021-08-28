//=========================================================================
//
//  LocoLipSyncController.hpp
//
//  Allows masked animation playback on top of current animation
//
//=========================================================================

#ifndef __LOCO_LIP_SYNC_CONTROLLER_HPP_
#define __LOCO_LIP_SYNC_CONTROLLER_HPP_

//=========================================================================
// INCLUDES
//=========================================================================

#include "LocoMaskController.hpp"
#include "Animation\AnimAudioTimer.hpp"


//=========================================================================
// CLASS LOCO_MASK_CONTROLLER
//=========================================================================
class loco_lip_sync_controller : public loco_mask_controller
{
// Construction / destruction
public:

        // Constructs a loco_lip_sync_controller object.
        loco_lip_sync_controller();

// Destroys a loco_lip_sync_controller object, handles cleanup and de-allocation.
virtual ~loco_lip_sync_controller();

// Member functions
public:

        // Sets a new animation and starts the audio
virtual void                SetAnim             ( const anim_group::handle& hAnimGroup, s32 iAnim, u32 VoiceID, u32 Flags ) ;

        // Sets a new animation and starts the audio
virtual void                SetAnim             ( const anim_group::handle& hAnimGroup, s32 iAnim, const char* pAudioName, u32 Flags ) ;

        // Advances animation and returns delta pos and delta yaw
virtual void                Advance             ( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw );

        // Clears the animation to a safe unused state
virtual void                Clear               ( void ) ;

        // Stops lip sync controller
        void                Stop                ( void );

        // Returns audio time
        f32                 GetAudioTime        ( void ) const;
        
        // Returns voice ID
        s32                 GetVoiceID          ( void ) const;
    
// Member variables
protected:

        u32                 m_VoiceID;  // Voice ID of attached audio
        anim_audio_timer    m_Timer;    // Audio timer
};

//=========================================================================

// Returns audio time
inline
f32 loco_lip_sync_controller::GetAudioTime( void ) const
{
    return m_Timer.GetTime();
}

//=========================================================================

// Returns voice ID
inline
s32 loco_lip_sync_controller::GetVoiceID( void ) const
{
    return m_VoiceID;
}

//=========================================================================

#endif // __LOCO_LIP_SYNC_CONTROLLER_HPP_

