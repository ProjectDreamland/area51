//=========================================================================
//
//  LocoAimController.hpp
//
//=========================================================================

#ifndef __LOCO_AIM_CONTROLLER_HPP__
#define __LOCO_AIM_CONTROLLER_HPP__

//=========================================================================
// INLCLUDES
//=========================================================================
#include "LocoMaskController.hpp"


//=========================================================================
// FORWARD DECLARATIONS
//=========================================================================
class loco ;


//=========================================================================
// LOCOMOTION AIM CONTROLLER CLASS
//=========================================================================
class loco_aim_controller : public loco_mask_controller
{
public:

    // Functions
                                loco_aim_controller ( void ) ;
                                
            void                SetBlendFactor      ( f32 wSide, f32 wUpDown, f32 BlendSpeed = 0.5f ) ;
            void                ApplyDeltaHoriz     ( f32 Delta ) ;
                                
    virtual void                Clear               ( void );
    virtual void                Advance             ( f32 DeltaTime, vector3& DeltaPos, radian& DeltaYaw );
    virtual void                MixKeys             ( const info& Info, anim_key* pDestKey ); 

            const anim_info&    GetAnimInfo         ( s32 iSide );

    // Manual (coded) aiming control
            void                SetHorizLimits      ( radian HorizMinLimit, radian HorizMaxLimit ) ;
            void                SetVertLimits       ( radian VertMinLimit,  radian VertMaxLimit ) ;
            radian              GetHorizMinLimit    ( void )    { return m_HorizMinLimit; }
            radian              GetHorizMaxLimit    ( void )    { return m_HorizMaxLimit; }
            radian              GetVertMinLimit     ( void )    { return m_VertMinLimit; }
            radian              GetVertMaxLimit     ( void )    { return m_VertMaxLimit; }

    // Returns current and target aiming angles
            radian              GetHorizAim             ( void ) const;
            radian              GetVertAim              ( void ) const;
            
            radian              GetTargetHorizAim       ( void ) const;
            radian              GetTargetVertAim        ( void ) const;

    // Weight blending
            void                SetWeight               ( f32 Weight, f32 BlendTime = 0.0f ) ;
            void                SetBoneMasks            ( const geom::bone_masks& VertBoneMasks,
                                                          const geom::bone_masks& HorizBoneMasks,
                                                                f32               BlendTime = 0.5f ) ;

protected:

    f32                     m_wSide;                                // Positive is right negative is left
    f32                     m_wUpDown;                              // Positive is up negative is down
                                                                    
    f32                     m_BlendSpeed ;                          // Speed to get to target
    f32                     m_wTargetSide ;                         // Target side to blend to
    f32                     m_wTargetUpDown ;                       // Target up/down to blend to
                                                                    
    radian                  m_HorizMinLimit, m_HorizMaxLimit ;      // Horizontal angle limits
    radian                  m_VertMinLimit,  m_VertMaxLimit ;       // Vertical angle limits

    f32                     m_WeightBlendDelta ;                    // Blends weight in and out

    const geom::bone_masks* m_pCurrentBoneMasksHoriz;               // Current bone masks
    const geom::bone_masks* m_pBlendBoneMasksHoriz;                 // Bone masks to blend from
    
friend class loco;    
};


//=========================================================================
// END
//=========================================================================
#endif