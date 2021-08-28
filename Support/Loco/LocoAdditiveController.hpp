//=========================================================================
//
//  LocoAdditiveController.hpp
//
//  Allows masked animation playback on top of current animation
//
//=========================================================================

#ifndef __LOCO_ADDITIVE_CONTROLLER_HPP_
#define __LOCO_ADDITIVE_CONTROLLER_HPP_

//=========================================================================
// INCLUDES
//=========================================================================

#include "LocoAnimController.hpp"

//=========================================================================
// CLASS LOCO_ADDITIVE_CONTROLLER
//=========================================================================
class loco_additive_controller : public loco_anim_controller
{
// Functions
public:

        // Constructs a loco_additive_controller object.
        loco_additive_controller();

// Destroys a loco_additive_controller object, handles cleanup and de-allocation.
virtual ~loco_additive_controller();

// Member functions
public:

        // Blend time control functions
        void                SetBlendInTime      ( f32 Secs ) ;
        void                SetBlendOutTime     ( f32 Secs ) ;

        // Mixes the anims keyframes into the dest keyframes
virtual void                MixKeys             ( const info& Info, anim_key* pDestKey ); 

        // Advances animation and returns delta pos and delta yaw
virtual void                Advance             ( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw );


// Member variables
protected:
        f32             m_BlendInTime ;                         // Time to blend anim in
        f32             m_BlendOutTime ;                        // Time to blend anim out

};

//=========================================================================

#endif // __LOCO_ADDITIVE_CONTROLLER_HPP_

