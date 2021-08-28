//=========================================================================
//
//  LocoRagdollController.hpp
//
//=========================================================================

#ifndef __LOCO_RAGDOLL_CONTROLLER_HPP__
#define __LOCO_RAGDOLL_CONTROLLER_HPP__

//=========================================================================
// INLCLUDES
//=========================================================================
#include "LocoMaskController.hpp"
#include "Ragdoll\Ragdoll.hpp"


//=========================================================================
// LOCOMOTION RAGDOLL CONTROLLER CLASS
//=========================================================================
class loco_ragdoll_controller : public loco_mask_controller
{
public:

    
    // Functions
                    loco_ragdoll_controller ( void );


    // Sets location of animation data package
    virtual void    SetAnimGroup        ( const anim_group::handle& hGroup ) ;

    // Clears the animation to a safe unused state
    virtual void    Clear               ( void ) ;

    // Advances the current track by logic time
    virtual void    Advance             ( f32 nSeconds, vector3& DeltaPos, radian& DeltaYaw ) ;

    // Controls the influence this anim has during the mixing process
    virtual void    SetWeight           ( f32 ParametricWeight ) ;
    virtual f32     GetWeight           ( void ) ;

    // Returns the raw keyframe data
    virtual void    GetInterpKeys       ( const info& Info, anim_key* pKey ) ;

    // Mixes the anims keyframes into the dest keyframes
    virtual void    MixKeys             ( const info& Info, anim_key* pDestKey ) ;

    // Util functions
    void            Init                ( const char* pGeomFileName, const char* pAnimFileName, ragdoll::type RagdollType, guid ObjectGuid = NULL ) ;
    void            SetBoneMask         ( s32 BoneIndex, f32 Weight, f32 BlendTime = 0.5f ) ;

protected:

    anim_group::handle  m_hAnimGroup;   // Group of anims we are using
    s32                 m_NBones ;      // Bone count
    f32                 m_Weight ;      // Weight of anim
    matrix4             m_L2W ;         // Local to world matrix
    matrix4             m_W2L ;         // World to local matrix
    
public:                 
    ragdoll             m_Ragdoll ;             // Ragdoll

};


//=========================================================================
// END
//=========================================================================
#endif