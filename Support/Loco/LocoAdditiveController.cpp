//=========================================================================
//
//  LocoAdditiveController.cpp
//
//  Allows additive animation playback on top of current animation
//
//=========================================================================

#include "LocoAdditiveController.hpp"

//=========================================================================
// FUNCTIONS
//=========================================================================

loco_additive_controller::loco_additive_controller() : loco_anim_controller()
{
    m_BlendInTime  = 0.1f ;    // Time to blend anim in
    m_BlendOutTime = 0.1f ;    // Time to blend anim out
}

//=========================================================================

loco_additive_controller::~loco_additive_controller()
{
}

//=========================================================================
// Blend time control functions
//=========================================================================
void loco_additive_controller::SetBlendInTime( f32 Secs )
{
    ASSERT(Secs >= 0) ;
    m_BlendInTime = Secs ;
}

//=========================================================================

void loco_additive_controller::SetBlendOutTime( f32 Secs )
{
    ASSERT(Secs >= 0) ;
    m_BlendOutTime = Secs ;
}

//=========================================================================

void loco_additive_controller::MixKeys( const info& Info, anim_key* pDestKey )
{
    // Additively mixes the anims keyframes with the dest keyframes
    AdditiveMixKeys(Info, m_iAnim, m_Frame, 0, pDestKey) ;
}
    
//=========================================================================

void loco_additive_controller::Advance( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw )
{
    // No animation?
    if ( m_iAnim == -1 )
        return;

    // Advance the base class
    loco_anim_controller::Advance( nSeconds, DeltaPos, DeltaYaw ) ;

    // Skip deltas
    DeltaPos.Zero() ;
    DeltaYaw = 0 ;

    // Lookup info
    const anim_info& AnimInfo = GetAnimInfo();

    // Compute frame info
    f32     NFrames        = MAX(1, (f32)m_nFrames-2) ;
    f32     FPS            = (f32)AnimInfo.GetFPS();
    f32     BlendInFrames  = MAX(1, m_BlendInTime  * FPS) ;
    f32     BlendOutFrames = MAX(1, m_BlendOutTime * FPS) ;
    f32     BlendInFrame   = BlendInFrames ;
    f32     BlendOutFrame  = NFrames - BlendOutFrames ;

    // Only blend in on first cycle
    if ( (m_Cycle == 0) && (m_Frame < BlendInFrame) )
    {
        // Blend in
        m_Weight = m_Frame / BlendInFrames ;
    }
    else
    // Only blend out if not looping
    if ( (!m_bLooping) && (m_Frame > BlendOutFrame) )
    {
        // Blend out
        m_Weight = 1.0f - ((m_Frame - BlendOutFrame) / BlendOutFrames) ;
    }
    else
    {
        // Full anim
        m_Weight = 1.0f ;
    }

    // Range check
    if (m_Weight < 0)
        m_Weight = 0 ;
    else
    if (m_Weight > 1)
        m_Weight = 1 ;
}

//=========================================================================
