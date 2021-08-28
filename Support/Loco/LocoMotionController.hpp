//=========================================================================
//
//  LocoAnimController.hpp
//
//=========================================================================
#ifndef __LOCO_MOTION_CONTROLLER_HPP__
#define __LOCO_MOTION_CONTROLLER_HPP__

//=========================================================================
// INCLUDES
//=========================================================================

#include "LocoAnimController.hpp"



//=========================================================================
// CLASSES
//=========================================================================

class loco_motion_controller : public loco_anim_controller
{
//=====================================================================
// PUBLIC FUNCTIONS
//=====================================================================
public:
                             loco_motion_controller   ( void );
virtual                     ~loco_motion_controller   ( void );

        // Misc functions

        // Clears the animation to a safe unused state
virtual void                Clear               ( void );
    
        // Sets a new animation and initializes the blend buffer
virtual void                SetAnim             ( const anim_group::handle& hAnimGroup, s32 iAnim, u32 Flags = 0 ) ;

private:
        // Advance animation and extracts motion
        void                Advance             ( const anim_info& AnimInfo,
                                                        f32        DeltaFrame,
                                                        f32&       Frame,
                                                        f32&       PrevFrame,
                                                        s32&       Cycle,
                                                        s32&       PrevCycle,
                                                        vector3&   DeltaPos,
                                                        radian&    DeltaYaw );

public:
        // Advances animation and returns delta pos and delta yaw
virtual void                Advance             ( f32 DeltaTime, vector3&  DeltaPos, radian& DeltaYaw );

        // Root bone functions
        radian              GetRootBoneFixupYaw     ( void ) ;

        // Yaw functions
        radian              GetStartYaw         ( void ) ;
        void                ResetYaw            ( void ) ;
        void                ApplyDeltaYaw       ( radian DeltaYaw ) ;
        void                SetYaw              ( radian Yaw ) ;
        radian              GetYaw              ( void ) const ;

        // Returns info about animation playing
        f32                 GetMovementSpeed    ( void ) const;

        // Sets playback rate to match delta position (returns rate)
        f32                 SetMatchingRate     ( const vector3& DeltaPos, f32 DeltaTime, f32 RateMin, f32 RateMax );

        // Key mixing
        void                FixRootBoneKey      ( const anim_info& AnimInfo, f32 Frame, anim_key& RootBoneKey );
virtual void                GetInterpKeys       ( const info& Info, anim_key* pKey );

        // Motion functions
        xbool               IsUsingMotionProp   ( void ) const { return m_iMotionProp != -1; }
        void                GetMotionRawKey     ( const anim_info& AnimInfo, s32 Frame, anim_key& Key ) ;
        void                GetMotionInterpKey  ( const anim_info& AnimInfo, f32 Frame, anim_key& Key ) ;

        // Mix animation functions
        void                SetMixAnim          ( s32 iMixAnim, f32 Mix );
        
//=====================================================================
// DATA
//=====================================================================

protected:

        // Motion vars
        s32                 m_iMotionProp ;             // Index to motion prop or -1 if none
        radian              m_Yaw ;                     // Current yaw
        
        // Mix animation vars
        s32                 m_iMixAnim;                 // Index of mix animation (or -1 if none)
        f32                 m_Mix;                      // Amount of mix animation (0 = none, 1 = all)
        f32                 m_MixAnimFrame;             // Current frame of mix animation

//=====================================================================
// FRIENDS
//=====================================================================
friend class loco_char_anim_player;

};

//=========================================================================
// INLINES
//=========================================================================

inline
void loco_motion_controller::ResetYaw( void )
{
    m_Yaw = GetStartYaw() ;
}

//=========================================================================

inline
void loco_motion_controller::ApplyDeltaYaw( radian DeltaYaw )
{
    m_Yaw += DeltaYaw ;
}

//=========================================================================

inline
void loco_motion_controller::SetYaw( radian Yaw )
{
    m_Yaw = Yaw ;
}

//=========================================================================

inline
radian loco_motion_controller::GetYaw( void ) const
{
    return m_Yaw ;
}

//=========================================================================
// Mix animation functions
//=========================================================================

inline
void loco_motion_controller::SetMixAnim( s32 iMixAnim, f32 Mix )
{
    // Store info
    m_iMixAnim = iMixAnim;
    m_Mix      = Mix;
}

//=========================================================================
#endif // END __LOCO_MOTION_CONTROLLER_HPP__
//=========================================================================
