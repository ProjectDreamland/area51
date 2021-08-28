//=========================================================================
//
//  LocoMaskController.hpp
//
//  Allows masked animation playback on top of current animation
//
//=========================================================================

#include "LocoMaskController.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================

loco_mask_controller::loco_mask_controller() :
        loco_anim_controller(),
        m_BlendInTime       ( 0.25f ),  // Time to blend anim in
        m_BlendOutTime      ( 0.25f ),  // Time to blend anim out
        m_pCurrentBoneMasks ( NULL ),   // Current bone masks
        m_pBlendBoneMasks   ( NULL ),   // Bone masks to blend from
        m_BoneBlend         ( 0.0f ),   // 1 = "BlendBoneMasks", 0 = "CurrentBoneMasks"
        m_BoneBlendDelta    ( 0.0f )    // Decrements to 0
{
}

//=========================================================================

loco_mask_controller::~loco_mask_controller()
{
}

//=========================================================================
// Bone mask control functions
//=========================================================================

void loco_mask_controller::SetBoneMasks( const geom::bone_masks& BoneMasks, f32 BlendTime )
{
    // Blend?
    if( ( m_pCurrentBoneMasks ) && ( BlendTime > 0.0f ) )
    {
        // Copy current to blend and start blending
        m_pBlendBoneMasks   = m_pCurrentBoneMasks;
        m_pCurrentBoneMasks = &BoneMasks;
        m_BoneBlend         = 1.0f;
        m_BoneBlendDelta    = -1.0f / BlendTime;
    }
    else
    {
        // Non blending
        m_pBlendBoneMasks   = NULL;
        m_pCurrentBoneMasks = &BoneMasks;
        m_BoneBlend         = 0.0f;
    }
}

//=========================================================================
// Blend time control functions
//=========================================================================

void loco_mask_controller::SetBlendInTime( f32 Secs )
{
    ASSERT(Secs >= 0) ;
    m_BlendInTime = Secs ;
}

//=========================================================================

void loco_mask_controller::SetBlendOutTime( f32 Secs )
{
    ASSERT(Secs >= 0) ;
    m_BlendOutTime = Secs ;
}

//=========================================================================

void loco_mask_controller::MixKeys( const info& Info, anim_key* pDestKey )
{
    // Mixes with masks the anims keyframes with the dest keyframes
    if( ( m_pCurrentBoneMasks ) && ( m_pBlendBoneMasks ) )
        MaskedMixKeys(Info, m_iAnim, m_Frame, *m_pCurrentBoneMasks, *m_pBlendBoneMasks, m_BoneBlend, pDestKey) ;
    else
    if( m_pCurrentBoneMasks )
        MaskedMixKeys(Info, m_iAnim, m_Frame, *m_pCurrentBoneMasks, pDestKey) ;
}

//=========================================================================

void loco_mask_controller::Advance( f32 DeltaTime, vector3&  DeltaPos, radian& DeltaYaw )
{
    // No animation?
    if ( m_iAnim == -1 )
        return;

    // Advance the base class
    loco_anim_controller::Advance( DeltaTime, DeltaPos, DeltaYaw ) ;

    // Force blend out?
    if( m_bIsBlendingOut )
    {
        // Lookup blend time or use default if non specified
        f32 BlendOutTime = 0.25f;
        if( m_BlendOutTime > 0.0f )    
            BlendOutTime = m_BlendOutTime;

        // Blend out the weight      
        m_Weight -= DeltaTime / BlendOutTime;      
    }
    else
    // Blend animation in/out?
    if( ( m_BlendInTime != 0.0f ) || ( m_BlendOutTime != 0.0f ) )
    {
        // Lookup info
        const anim_info& AnimInfo = GetAnimInfo();

        // Compute frame info
        f32 NFrames        = MAX(1, (f32)m_nFrames-2) ;
        f32 FPS            = (f32)AnimInfo.GetFPS();
        f32 BlendInFrames  = MAX(1, m_BlendInTime  * FPS) ;
        f32 BlendOutFrames = MAX(1, m_BlendOutTime * FPS) ;
        f32 BlendInFrame   = BlendInFrames ;
        f32 BlendOutFrame  = NFrames - BlendOutFrames ;

        // Only blend in on first cycle
        if ( (m_Cycle == 0) && (m_BlendInTime != 0.0f) && (m_Frame < BlendInFrame) )
        {
            // Blend in
            m_Weight = m_Frame / BlendInFrames ;
        }
        else
        // Only blend out if not looping
        if ( (!m_bLooping) && (m_BlendOutTime != 0.0f) && (m_Frame > BlendOutFrame) )
        {
            // Blend out
            m_Weight = 1.0f - ((m_Frame - BlendOutFrame) / BlendOutFrames) ;
        }
        else
        {
            // Full anim
            m_Weight = 1.0f ;
        }
    }

    // Range check
    if (m_Weight < 0)
        m_Weight = 0 ;
    else
    if (m_Weight > 1)
        m_Weight = 1 ;

    // Advance bone blends
    m_BoneBlend += m_BoneBlendDelta * DeltaTime;
    if( m_BoneBlend < 0.0f )
    {
        // End blending
        m_BoneBlend       = 0.0f;
        m_BoneBlendDelta  = 0.0f;
        m_pBlendBoneMasks = NULL;
    }
}

//=========================================================================

// Blends out the mask controller
void loco_mask_controller::Stop( void )
{
    // Blend out
    m_bIsBlendingOut = TRUE;
}

//=========================================================================
