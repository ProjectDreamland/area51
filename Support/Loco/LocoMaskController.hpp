//=========================================================================
//
//  LocoMaskController.hpp
//
//  Allows masked animation playback on top of current animation
//
//=========================================================================

#ifndef __LOCO_MASK_CONTROLLER_HPP_
#define __LOCO_MASK_CONTROLLER_HPP_

//=========================================================================
// INCLUDES
//=========================================================================

#include "LocoAnimController.hpp"

//=========================================================================
// CLASS LOCO_MASK_CONTROLLER
//=========================================================================
class loco_mask_controller : public loco_anim_controller
{
// Construction / destruction
public:

        // Constructs a loco_mask_controller object.
        loco_mask_controller();

// Destroys a loco_mask_controller object, handles cleanup and de-allocation.
virtual ~loco_mask_controller();

// Member functions
public:
        // Bone mask control functions
        f32                 GetBoneWeight       ( s32 iBone );
        void                SetBoneMasks        ( const geom::bone_masks& BoneMasks, f32 BlendTime = 0.5f ) ;

        // Blend time control functions
        void                SetBlendInTime      ( f32 Secs ) ;
        void                SetBlendOutTime     ( f32 Secs ) ;

        // Mixes the anims keyframes into the dest keyframes
virtual void                MixKeys             ( const info& Info, anim_key* pDestKey ); 

        // Advances animation and returns delta pos and delta yaw
virtual void                Advance             ( f32 DeltaTime, vector3&  DeltaPos, radian& DeltaYaw );

        // Blends out the mask controller
        void                Stop                ( void );


// Member variables
protected:
        f32                     m_BlendInTime ;         // Time to blend anim in
        f32                     m_BlendOutTime ;        // Time to blend anim out
        
        const geom::bone_masks* m_pCurrentBoneMasks;    // Current bone masks
        const geom::bone_masks* m_pBlendBoneMasks;      // Bone masks to blend from
        f32                     m_BoneBlend ;           // 1 = "BlendBoneMasks", 0 = "CurrentBoneMasks"
        f32                     m_BoneBlendDelta ;      // Decrements to 0
};

//=========================================================================

inline
f32 loco_mask_controller::GetBoneWeight( s32 iBone )
{
    // Compute bone weight and skip if it has no influence
    ASSERT( m_pCurrentBoneMasks );
    f32 Weight = m_pCurrentBoneMasks->Weights[ iBone ];
    if( m_pBlendBoneMasks )
        Weight += m_BoneBlend * (m_pBlendBoneMasks->Weights[ iBone ] - Weight);
    Weight *= m_Weight;
    return Weight;
}

//=========================================================================

#endif // __LOCO_MASK_CONTROLLER_HPP_

