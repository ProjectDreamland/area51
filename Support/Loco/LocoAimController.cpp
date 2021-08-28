//=========================================================================
//
//  LocoAimController.cpp
//
//=========================================================================

#include "Loco.hpp"
#include "e_Engine.hpp"



//=========================================================================
// FUNCTIONS
//=========================================================================

loco_aim_controller::loco_aim_controller ( void ) :
    loco_mask_controller(),

    m_wSide                     ( 0.0f  ),              // Positive is right negative is left
    m_wUpDown                   ( 0.0f  ),              // Positive is up negative is down
                                             
    m_BlendSpeed                ( 1.0f  ),              // Speed to get to target
    m_wTargetSide               ( 0.0f  ),              // Target side to blend to
    m_wTargetUpDown             ( 0.0f  ),              // Target up/down to blend to
                                                                    
    m_HorizMinLimit             ( -R_90 ),
    m_HorizMaxLimit             ( R_90  ),              // Horizontal angle limits
    m_VertMinLimit              ( -R_90 ),
    m_VertMaxLimit              ( R_90  ),              // Vertical angle limits

    m_WeightBlendDelta          ( 0.0f  ),              // Blends weight in and out
    
    m_pCurrentBoneMasksHoriz    ( NULL  ),              // Current bone masks
    m_pBlendBoneMasksHoriz      ( NULL  )               // Bone masks to blend from
{
    // Clear
    Clear();

    // Do not use anim frame blending
    m_BlendInTime  = 0.0f;
    m_BlendOutTime = 0.0f;
}

//=========================================================================

void loco_aim_controller::SetBlendFactor( f32 wSide, f32 wUpDown, f32 BlendSpeed )
{
    // Cap horiz rotation
    if (wSide < m_HorizMinLimit)
        wSide = m_HorizMinLimit ;
    else
    if (wSide > m_HorizMaxLimit)
        wSide = m_HorizMaxLimit ;

    // Cap vert rotation
    if (wUpDown < m_VertMinLimit)
        wUpDown = m_VertMinLimit ;
    else
    if (wUpDown > m_VertMaxLimit)
        wUpDown = m_VertMaxLimit ;

    // Set targets
    m_wTargetSide   = wSide ;
    m_wTargetUpDown = wUpDown ;
    m_BlendSpeed    = BlendSpeed ;

    // If no blend, snap directly to horizontal direction
    if (BlendSpeed == 0)
    {
        m_wSide   = m_wTargetSide ;
        m_wUpDown = m_wTargetUpDown ;
    }
}

//=========================================================================

void loco_aim_controller::ApplyDeltaHoriz( f32 Delta )
{
    // Apply to actual value
    m_wSide += Delta ;
    m_wSide  = MAX(m_HorizMinLimit, MIN(m_HorizMaxLimit, m_wSide)) ;

    // Apply to target value
    m_wTargetSide += Delta ;
    m_wTargetSide  = MAX(m_HorizMinLimit, MIN(m_HorizMaxLimit, m_wTargetSide)) ;
}

//=========================================================================

void loco_aim_controller::Clear( void )
{
    // Call base class
    loco_anim_controller::Clear() ;

    // Clear info
    m_BlendSpeed = 1.0f ;
    m_wSide      = m_wTargetSide   = 0;     
    m_wUpDown    = m_wTargetUpDown = 0;
    m_Weight     = 1;
    m_Rate       = 1.0f;
}

//=========================================================================

void loco_aim_controller::Advance( f32 DeltaTime, vector3& DeltaPos, radian& DeltaYaw )
{
    f32 Delta, FullDelta, MaxDelta ;

    // Call base class
    loco_mask_controller::Advance( DeltaTime, DeltaPos, DeltaYaw );

    // If there is no animation, then we are using the manual aimer
    // so we need to advance the bone blends ourselves
    // (since base class will not do anything if there is no anim)
    if( m_iAnim == -1 )
    {
        m_BoneBlend += m_BoneBlendDelta * DeltaTime;
        if( m_BoneBlend < 0.0f )
        {
            // End blending
            m_BoneBlend       = 0.0f;
            m_BoneBlendDelta  = 0.0f;
            m_pBlendBoneMasks = NULL;
        }
    }

    // Blend weight in/out
    m_Weight += m_WeightBlendDelta * DeltaTime ;
    m_Weight = x_min(1.0f, x_max(m_Weight, 0.0f)) ;

    // Clear deltas
    DeltaPos.Zero() ;
    DeltaYaw = 0 ;

    // Compute max delta, based on blend speed
    MaxDelta = m_BlendSpeed * DeltaTime * R_240 ;

    // Blend to side target
    FullDelta = m_wTargetSide - m_wSide ;
    Delta     = FullDelta * DeltaTime * 5.0f * m_BlendSpeed ;
    if (Delta > MaxDelta)
        Delta = MaxDelta ;
    else
    if (Delta < -MaxDelta)
        Delta = -MaxDelta ;
    if (ABS(Delta) > ABS(FullDelta))
        Delta = FullDelta ;
    m_wSide += Delta ;
    
    // Blend to up/down target
    FullDelta = m_wTargetUpDown - m_wUpDown ;
    Delta     = FullDelta * DeltaTime * 5.0f * m_BlendSpeed ;
    if (Delta > MaxDelta)
        Delta = MaxDelta ;
    else
    if (Delta < -MaxDelta)
        Delta = -MaxDelta ;
    if (ABS(Delta) > ABS(FullDelta))
        Delta = FullDelta ;
    m_wUpDown += Delta ;

}

//=========================================================================

void loco_aim_controller::MixKeys( const info& Info, anim_key* pDestKey )
{
    s32 i ;

    CONTEXT("loco_aim_controller::MixKeys") ;

    // Skip if controller has no weight
    if (m_Weight == 0.0f)
        return;

    // Get bone count from current mask
    ASSERT( m_pCurrentBoneMasks );
    ASSERT( m_pCurrentBoneMasksHoriz );
    s32 NBones = x_max( m_pCurrentBoneMasks->nBones, m_pCurrentBoneMasksHoriz->nBones );

    // Take into account vert blend mask count?
    if( m_pBlendBoneMasks )
        NBones = x_max( NBones, m_pBlendBoneMasks->nBones );
    
    // Take into account horiz blend mask count?
    if( m_pBlendBoneMasksHoriz )
        NBones = x_max( NBones, m_pBlendBoneMasksHoriz->nBones );

    // Take into account current LOD
    NBones = x_min( NBones, Info.m_nActiveBones );
    
    // Exit if nothing to do
    if( NBones == 0 )
        return;

    // If this fires, the above bone count logic in incorrect
    ASSERT( NBones <= Info.m_nActiveBones );

    // Lookup anim info
    const anim_group& AnimGroup = GetAnimGroup() ;

    // If this assert fires off, then either the geometry and anim group have different numbers of bones,
    // ie. the geometry and anim group file are using a different bind pose matxs,
    // or this particular anim group has a different number of bones than an anim group used in
    // the same player ie. multiple anim groups used in this player have different bind pose matxs.
    // (this can happen when using "PlayAnim" with other anim group packages).
    // Either way - fix your resources!
    ASSERTS( ( NBones <= GetAnimGroup().GetNBones()), "Incompatible anim group with bone lods!" ) ;

    // Compute anim space rotations (the aimer needs the inverse of these later).
    // Since the keys are in relative-to-parent-anim-space, each key just needs
    // multiplying by it's parent-anim-space key.
    quaternion* InvAnimSpaceRot = (quaternion*)base_player::GetMixBuffer( base_player::MIX_BUFFER_CONTROLLER );
    ASSERT(InvAnimSpaceRot);
    ASSERT(NBones <= MAX_ANIM_BONES) ;
    for (i = 0 ; i < NBones ; i++)
    {
        // Multiply by parent?
        s32 iParent = AnimGroup.GetBoneParent(i) ;
        if (iParent != -1)
        {
            ASSERT(iParent < i) ;    // Bones are always in parent->child order
            InvAnimSpaceRot[i] = InvAnimSpaceRot[iParent] * pDestKey[i].Rotation ;
        }
        else
            InvAnimSpaceRot[i] = pDestKey[i].Rotation ;
    }

    // Now compute inverse anim space rotations ready for the aimer to use
    for (i = 0 ; i < NBones ; i++)
        InvAnimSpaceRot[i].Invert() ;

    // Compute rotations, taking current controller weight into account.
    vector3 YawAxis    = vector3(0,-1,0) ;
    f32     YawAngle   = -m_wSide * m_Weight;
    vector3 PitchAxis  = vector3(-1,0,0) ;
    f32     PitchAngle = m_wUpDown * m_Weight;

    // Put pitch aim into correct anim space
    PitchAxis = Info.m_Local2AimSpace.Rotate(PitchAxis) ;
    PitchAxis.Normalize() ;

    // Put yaw aim into correct anim space
    YawAxis = Info.m_Local2AimSpace.Rotate(YawAxis) ;
    YawAxis.Normalize() ;

    // Apply over all bones that have weight
    for ( i = 0 ; i < NBones ; i++ )
    {
        // Compute vertical bone weight
        ASSERT( m_pCurrentBoneMasks );
        f32 WeightVert = m_pCurrentBoneMasks->Weights[ i ];
        if( m_pBlendBoneMasks )
            WeightVert += m_BoneBlend * ( m_pBlendBoneMasks->Weights[ i ] - WeightVert );

        // Compute horizontal bone weight
        ASSERT( m_pCurrentBoneMasksHoriz );
        f32 WeightHoriz = m_pCurrentBoneMasksHoriz->Weights[ i ];
        if( m_pBlendBoneMasksHoriz )
            WeightHoriz += m_BoneBlend * ( m_pBlendBoneMasksHoriz->Weights[ i ] - WeightHoriz );

        // Skip if weight has no influence
        if( ( WeightVert == 0.0f ) && ( WeightHoriz == 0.0f ) )
            continue ;

        // Lookup key and parent index
        anim_key& Key     = pDestKey[i] ;
        s32       iParent = AnimGroup.GetBoneParent(i) ;

        // Compute final pitch and yaw angle
        f32 PitchBoneAngle = PitchAngle * WeightVert ;
        f32 YawBoneAngle   = YawAngle   * WeightHoriz ;

        // Compute final pitch and yaw axis
        vector3 PitchBoneAxis, YawBoneAxis ;
        
        // SPACE FIXUP:
        // Since the pitch and yaw is in local-anim-space, but the actual keys 
        // are in relative-to-parent-anim-space, we need to put the pitch/yaw 
        // rotations into relative-to-parent-anim-space. This is achieved by
        // multiplying by the inverse-parent-anim-space rotation.
        //
        // NOTE:
        // This allows bones to be in a twisted idle pose pose (eg. like the grunt),
        // and have the pitch/yaw computations still work.
        if (iParent != -1)
        {
            // Make relative to parent
            PitchBoneAxis = InvAnimSpaceRot[iParent] * PitchAxis ;
            YawBoneAxis   = InvAnimSpaceRot[iParent] * YawAxis ;
        }
        else
        {
            PitchBoneAxis = PitchAxis ;
            YawBoneAxis   = YawAxis ;
        }

        // Compute final additive pitch and yaw
        quaternion PitchBoneRot(PitchBoneAxis, PitchBoneAngle) ;
        quaternion YawBoneRot  (YawBoneAxis,   YawBoneAngle) ;

        // Pre-apply pitch and yaw rotation to key
        // NOTE: It looks like the quaternions are multiplied from Left -> Right order,
        //       where as matrices are multiplied from Right -> Left order?!
        Key.Rotation    = PitchBoneRot * Key.Rotation ;
        Key.Rotation    = YawBoneRot   * Key.Rotation ;

        // NOTE: Because I'm pre-applying a rotation to the key, the translation is not affected.
    }
}

//=========================================================================
// Manual (coded) aiming control
//=========================================================================

void loco_aim_controller::SetHorizLimits ( radian HorizMinLimit, radian HorizMaxLimit )
{
    // Make sure the sign is correct
    ASSERT(HorizMinLimit <= HorizMaxLimit) ;

    m_HorizMinLimit = HorizMinLimit ;
    m_HorizMaxLimit = HorizMaxLimit ;
}

//=========================================================================

void loco_aim_controller::SetVertLimits  ( radian VertMinLimit,  radian VertMaxLimit )
{
    // Make sure the sign is correct
    ASSERT(VertMinLimit <= VertMaxLimit) ;

    m_VertMinLimit  = VertMinLimit ;
    m_VertMaxLimit  = VertMaxLimit ;
}

//=========================================================================
// Returns current aiming angles
//=========================================================================

radian loco_aim_controller::GetHorizAim( void ) const
{
    return m_wSide ;
}

//=========================================================================

radian loco_aim_controller::GetVertAim( void ) const
{
    return m_wUpDown ;
}

//=========================================================================

radian loco_aim_controller::GetTargetHorizAim( void ) const
{
    return m_wTargetSide ;
}

//=========================================================================

radian loco_aim_controller::GetTargetVertAim( void ) const
{
    return m_wTargetUpDown ;
}

//=========================================================================
// Weight blending
//=========================================================================

void loco_aim_controller::SetWeight( f32 Weight, f32 BlendTime )
{
    // No blending?
    if (BlendTime == 0.0f)
    {
        m_Weight           = Weight ;
        m_WeightBlendDelta = 0.0f ;
    }
    else
    {
        // Blend to new weight
        m_WeightBlendDelta = (Weight - m_Weight) / BlendTime ;
    }
}

//=========================================================================
// Bone mask control functions
//=========================================================================

void loco_aim_controller::SetBoneMasks( const geom::bone_masks& VertBoneMasks,
                                        const geom::bone_masks& HorizBoneMasks,
                                              f32               BlendTime       )
{
    // Call base class to setup vert masks
    loco_mask_controller::SetBoneMasks( VertBoneMasks, BlendTime );
    
    // Setup horiz masks
    if( ( m_pCurrentBoneMasksHoriz ) && ( BlendTime > 0.0f ) )
    {
        // Blending
        m_pBlendBoneMasksHoriz   = m_pCurrentBoneMasksHoriz;
        m_pCurrentBoneMasksHoriz = &HorizBoneMasks;
    }
    else
    {
        // Non blending
        m_pBlendBoneMasksHoriz   = NULL;
        m_pCurrentBoneMasksHoriz = &HorizBoneMasks;
    }
}

//=========================================================================
