//=========================================================================
//
//  LocoEyeController.hpp
//
//  Additive eye controller
//
//=========================================================================

#ifndef __LOCO_EYE_CONTROLLER_HPP_
#define __LOCO_EYE_CONTROLLER_HPP_

//=========================================================================
// INCLUDES
//=========================================================================

#include "LocoAnimController.hpp"


//=========================================================================
// CLASS LOCO_EYE_CONTROLLER
//=========================================================================
class loco_eye_controller : public loco_anim_controller
{
// Functions
public:

        // Constructs a loco_eye_controller object.
        loco_eye_controller() ;

        // Destroys a loco_eye_controller object, handles cleanup and de-allocation.
virtual ~loco_eye_controller() ;

// Member functions
public:

        // Sets location of animation data package
virtual void    SetAnimGroup    ( const anim_group::handle& hAnimGroup ) ;

        // Mixes the anims keyframes into the dest keyframes
virtual void    MixKeys         ( const info& Info, anim_key* pDestKey ) ; 

        // Advances animation and returns delta pos and delta yaw
virtual void    Advance         ( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw ) ;

        // Sets the position to look at. Will use blending by default
        void    SetLookAt       ( const vector3& Target, xbool bBlendTowards = TRUE ) ;

        // Sets the target blend speed
        void    SetTargetBlendSpeed( f32 BlendSpeed ) { m_TargetBlendSpeed = BlendSpeed; }
        
        // Returns TRUE if eye anims are present
        xbool   IsActive        ( void ) const;
        
// Member variables
protected:
        s16     m_iLEyeBone;                // Index of left eye bone
        s16     m_iREyeBone;                // Index of right eye bone
        s16     m_iLRAnim ;                 // Index of left/right animation
        s16     m_iUDAnim ;                 // Index of up/down animation
        radian  m_MinPitch, m_MaxPitch;     // Min/Max of anim up/down pitch
        radian  m_MinYaw,   m_MaxYaw;       // Min/Max of anim left/right yaw
        f32     m_TargetBlendSpeed;         // Speed to blend towards target
        vector3 m_LookAt ;                  // Current position that the eyes are looking at
        vector3 m_TargetLookAt ;            // Target position that the eyes need to look towards
        
//=========================================================================
// FRIENDS
//=========================================================================
friend class loco_char_anim_player;
friend class loco;
};

//=========================================================================

// Returns TRUE if eye anims are present
inline
xbool loco_eye_controller::IsActive( void ) const
{
    // Only active if anims and bones are all present
    return(     ( m_iLRAnim != -1 )
             && ( m_iUDAnim != -1 )
             && ( m_iLEyeBone != -1 )
             && ( m_iREyeBone != -1 ) );
}

//=========================================================================

#endif // __LOCO_EYE_CONTROLLER_HPP_

