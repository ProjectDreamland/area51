//=========================================================================
//
//  LocoRagdollController.cpp
//
//=========================================================================

#include "LocoRagdollController.hpp"
#include "e_Engine.hpp"


// Functions
//=========================================================================

loco_ragdoll_controller::loco_ragdoll_controller( void ) : loco_mask_controller()
{
    m_Weight = 1.0f ;
    m_NBones = 0 ;
    m_L2W.Identity() ;
    m_W2L.Identity() ;
    
    // Do not use anim frame blending
    m_BlendInTime  = 0.0f;
    m_BlendOutTime = 0.0f;
}

//=========================================================================

// Sets location of animation data package
void loco_ragdoll_controller::SetAnimGroup( const anim_group::handle& hGroup )
{
    // Keep handle
    m_hAnimGroup = hGroup ;

    // Keep number of bones
    anim_group* pAnimGroup = (anim_group*)m_hAnimGroup.GetPointer() ;
    if (pAnimGroup)
        m_NBones = pAnimGroup->GetNBones() ;
}

//=========================================================================

// Clears the animation to a safe unused state
void loco_ragdoll_controller::Clear( void )
{
}

//=========================================================================

// Advances the current track by logic time
void loco_ragdoll_controller::Advance( f32 nSeconds, vector3& DeltaPos, radian& DeltaYaw )
{
    // Call base class
    loco_mask_controller::Advance( nSeconds, DeltaPos, DeltaYaw );

    // Advance ragdoll
    m_Ragdoll.Advance(nSeconds) ;

    // Clear deltas
    DeltaPos.Zero() ;
    DeltaYaw = 0 ;
}

//=========================================================================

// Controls the influence this anim has during the mixing process
void loco_ragdoll_controller::SetWeight( f32 ParametricWeight )
{
    m_Weight = ParametricWeight ;
}

//=========================================================================

f32 loco_ragdoll_controller::GetWeight( void )
{
    return m_Weight ;
}

//=========================================================================

// Returns the raw keyframe data
void loco_ragdoll_controller::GetInterpKeys( const info& Info, anim_key* pKey )
{
    // Should never get called...
    (void)Info ;
    (void)pKey ;
}

//=========================================================================

// Mixes the anims keyframes into the dest keyframes
void loco_ragdoll_controller::MixKeys( const info& Info, anim_key* pDestKey )
{
    CONTEXT("loco_ragdoll_controller::MixKeys") ;

    s32 i ;

    // Grab matrices
    m_L2W = m_W2L = Info.m_Local2World ;
    m_W2L.InvertRT() ;

    // Get animation group
    anim_group* pAnimGroup = m_hAnimGroup.GetPointer() ;
    if (!pAnimGroup)
        return ;

    // Get # of bones to compute
    s32 nBones = Info.m_nActiveBones ;

    // If this assert fires off, then either the geometry and anim group have different numbers of bones,
    // ie. the geometry and anim group file are using a different bind pose matxs,
    // or this particular anim group has a different number of bones than an anim group used in
    // the same player ie. multiple anim groups used in this player have different bind pose matxs.
    // (this can happen when using "PlayAnim" with other anim group packages).
    // Either way - fix your resources!
    ASSERTS( (nBones <= GetAnimGroup().GetNBones()), "Incompatible anim group with bone lods!" ) ;

    // Allocate local->world matrices
    matrix4* pWorldMatrices = (matrix4*)smem_BufferAlloc(nBones * sizeof(matrix4)) ;
    if (!pWorldMatrices)
        return ;

    // Allocate relative matrices
    matrix4* pRelativeMatrices = (matrix4*)smem_BufferAlloc(nBones * sizeof(matrix4)) ;
    if (!pRelativeMatrices)
        return ;

    // Setup world space matrices from ragdoll
    m_Ragdoll.ComputeMatrices(pWorldMatrices, nBones) ;

    // Now put matrices into relative space
    for (i = 0 ; i < nBones ; i++)
    {
        const anim_bone& Bone = pAnimGroup->GetBone(i) ;
        if (Bone.iParent != -1)
        {
            // Lookup matrices
            matrix4& World  = pWorldMatrices[i] ;
            matrix4& Parent = pWorldMatrices[Bone.iParent] ;

            // Compute
            matrix4 InvParent = Parent ;
            InvParent.InvertRT() ;

            // Put into relative space
            pRelativeMatrices[i] = InvParent * World ;
        }
        else
        {
            // Keep as is
            pRelativeMatrices[i] = pWorldMatrices[i] ;
        }
    }

    // Matrices need to be in anim player local space!
    pRelativeMatrices[0] = m_W2L * pRelativeMatrices[0] ;

    // Now blend in the relative matrices
    anim_key Key ;   
    Key.Identity() ;
    for (i = 0 ; i < nBones ; i++)
    {
        // Get bone wieght
        f32 W = GetBoneWeight( i );
        if (W > 0)
        {
            matrix4& Rel = pRelativeMatrices[i] ;
            Key.Rotation    = Rel.GetQuaternion() ;
            Key.Translation = Rel.GetTranslation() ;
            pDestKey[i].Interpolate( pDestKey[i], Key, W * m_Weight );
        }
    }
}

//=========================================================================

// Util functions
void loco_ragdoll_controller::Init( const char* pGeomFileName, const char* pAnimFileName, ragdoll::type RagdollType, guid ObjectGuid )
{
    m_Ragdoll.Init(pGeomFileName, pAnimFileName, RagdollType, ObjectGuid) ;
}

//=========================================================================

void loco_ragdoll_controller::SetBoneMask( s32 BoneIndex, f32 Weight, f32 BlendTime /*= 0.5f*/ )
{
    // Not yet implemented
    (void)BoneIndex ;
    (void)Weight ;
    (void)BlendTime ;
}

//=========================================================================

