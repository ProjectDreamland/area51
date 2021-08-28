//=========================================================================
//
//  LocoEyeController.cpp
//
//  Additive eye controller
//
//=========================================================================

#include "LocoEyeController.hpp"
#include "Entropy.hpp"


//=========================================================================
// FUNCTIONS
//=========================================================================

loco_eye_controller::loco_eye_controller() : 
    loco_anim_controller(),

    m_iLEyeBone         ( -1 ),         // Index of left eye bone
    m_iREyeBone         ( -1 ),         // Index of right eye bone

    m_iLRAnim           ( -1 ),         // Index of left/right animation
    m_iUDAnim           ( -1 ),         // Index of up/down animation

    m_MinPitch          ( -R_13 ),      // Min/Max of anim up/down pitch
    m_MaxPitch          ( R_15 ),
    
    m_MinYaw            ( -R_25 ),      // Min/Max of anim left/right yaw
    m_MaxYaw            ( R_25 ),
                  
    m_TargetBlendSpeed  ( 1.0f ),       // Speed to blend towards target
                            
    m_LookAt            ( 0, 0, 0 ),    // Current position that the eyes are looking at
    m_TargetLookAt      ( 0, 0, 0 )     // Target position that the eyes need to look towards
{
}

//=========================================================================

loco_eye_controller::~loco_eye_controller()
{
}

//=========================================================================

static radian3 GetLocalRot( const anim_group& AnimGroup, 
                            const anim_info&  AnimInfo, 
                                  s32         iFrame, 
                                  s32         iBone )
{
    // Get local key
    anim_key Key;
    AnimInfo.GetRawKey( iFrame, iBone, Key );
    
    // Go up hierarchy to get parent transform
    s32 iParent = AnimGroup.GetBoneParent( iBone );
    while( iParent != -1 )
    {
        // Get parent key
        anim_key ParentKey;
        AnimInfo.GetRawKey( iFrame, iParent, ParentKey );    
        
        // Apply
        Key.Rotation = Key.Rotation * ParentKey.Rotation;
    
        // Goto next parent
        iParent = AnimGroup.GetBoneParent( iParent );
    }
    
    // Get euler rotation
    return Key.Rotation.GetRotation();
}

//=========================================================================

static xbool UpdateRotLimits( const anim_group& AnimGroup,
                              const anim_info&  AnimInfo,
                                    s32         iBone,
                                    radian3&    MinRotLimit,
                                    radian3&    MaxRotLimit )
{
    // Get limits
    s32 nFrames = AnimInfo.GetNFrames()-1;
    radian3 MinRot  = GetLocalRot( AnimGroup, AnimInfo, 0,          iBone );
    radian3 MidRot  = GetLocalRot( AnimGroup, AnimInfo, nFrames>>1, iBone );
    radian3 MaxRot  = GetLocalRot( AnimGroup, AnimInfo, nFrames,    iBone );

    // Make relative to mid rotation
    MinRot -= MidRot;
    MaxRot -= MidRot;
    
    // Update limits
    MinRotLimit.Pitch = x_min( MinRotLimit.Pitch , MinRot.Pitch );
    MinRotLimit.Yaw   = x_min( MinRotLimit.Yaw   , MinRot.Yaw   );
    MinRotLimit.Roll  = x_min( MinRotLimit.Roll  , MinRot.Roll  );
    
    MinRotLimit.Pitch = x_min( MinRotLimit.Pitch , MaxRot.Pitch );
    MinRotLimit.Yaw   = x_min( MinRotLimit.Yaw   , MaxRot.Yaw   );
    MinRotLimit.Roll  = x_min( MinRotLimit.Roll  , MaxRot.Roll  );

    MaxRotLimit.Pitch = x_max( MaxRotLimit.Pitch , MinRot.Pitch );
    MaxRotLimit.Yaw   = x_max( MaxRotLimit.Yaw   , MinRot.Yaw   );
    MaxRotLimit.Roll  = x_max( MaxRotLimit.Roll  , MinRot.Roll  );
    
    MaxRotLimit.Pitch = x_max( MaxRotLimit.Pitch , MaxRot.Pitch );
    MaxRotLimit.Yaw   = x_max( MaxRotLimit.Yaw   , MaxRot.Yaw   );
    MaxRotLimit.Roll  = x_max( MaxRotLimit.Roll  , MaxRot.Roll  );
    
    return TRUE;
}

//=========================================================================

void loco_eye_controller::SetAnimGroup( const anim_group::handle& hAnimGroup )
{
    // Call base
    loco_anim_controller::SetAnimGroup( hAnimGroup ) ;

    // Turn off by default
    m_Weight = 0.0f ;

    // Lookup animations and bones
    const anim_group& AnimGroup = GetAnimGroup() ;
    m_iUDAnim   = AnimGroup.GetAnimIndex( "EYE_LOOK_UD" );
    m_iLRAnim   = AnimGroup.GetAnimIndex( "EYE_LOOK_LR" );
    m_iLEyeBone = AnimGroup.GetBoneIndex( "Face_L_Eye", TRUE );
    m_iREyeBone = AnimGroup.GetBoneIndex( "Face_R_Eye", TRUE );

    // Found everything?
    if( ( m_iLRAnim != -1 ) && ( m_iUDAnim != -1 ) && ( m_iLEyeBone != -1 ) && ( m_iREyeBone != -1 ) )
    {
        // Clear limits
        radian3 MinUD( R_360, R_360, R_360), MaxUD( -R_360, -R_360, -R_360 );
        radian3 MinLR( R_360, R_360, R_360), MaxLR( -R_360, -R_360, -R_360 );
        
        // Lookup animations
        const anim_info& UDAnimInfo = AnimGroup.GetAnimInfo( m_iUDAnim );
        const anim_info& LRAnimInfo = AnimGroup.GetAnimInfo( m_iLRAnim );
        
        // Get limits from animation
        UpdateRotLimits( AnimGroup, UDAnimInfo, m_iLEyeBone, MinUD, MaxUD );
        UpdateRotLimits( AnimGroup, UDAnimInfo, m_iREyeBone, MinUD, MaxUD ); 
        UpdateRotLimits( AnimGroup, LRAnimInfo, m_iLEyeBone, MinLR, MaxLR );
        UpdateRotLimits( AnimGroup, LRAnimInfo, m_iREyeBone, MinLR, MaxLR );
            
        // Grab limits    
        m_MinYaw = MinLR.Yaw;
        m_MaxYaw = MaxLR.Yaw;
        
        m_MinPitch = MinUD.Pitch;
        m_MaxPitch = MaxUD.Pitch;
                
        // Turn on
        m_Weight = 1.0f ;
    }        
}

//=========================================================================

static f32 ComputeFrame( radian Angle,
                         radian AngleMin,
                         radian AngleMax,
                         f32    FrameMin,
                         f32    FrameMax )
{
    f32 AngleStart, AngleEnd,
        FrameStart, FrameEnd ;

    // Limit angle
    if (Angle < AngleMin)
        Angle = AngleMin ;
    else
    if (Angle > AngleMax)
        Angle = AngleMax ;

    // Compute mid values
    f32 AngleMid = 0.0f ;
    f32 FrameMid = (FrameMin + FrameMax) * 0.5f ;

    // Setup angle/frame start/end
    if (Angle < AngleMid)
    {
        AngleStart = AngleMin ;
        FrameStart = FrameMin ;
        
        AngleEnd   = AngleMid ;
        FrameEnd   = FrameMid ;
    }
    else
    {
        AngleStart = AngleMid ;
        FrameStart = FrameMid ;
        
        AngleEnd   = AngleMax ;
        FrameEnd   = FrameMax ;
    }

    // Compute ranges
    f32 AngleRange = AngleEnd - AngleStart ;
    f32 FrameRange = FrameEnd - FrameStart ;

    // Compute frame
    f32 Frame = FrameStart + (FrameRange * ((Angle - AngleStart) / AngleRange)) ;

    return Frame ;
}

//=========================================================================

void loco_eye_controller::MixKeys( const info& Info, anim_key* pDestKey )
{
    // Turned off?
    if( m_Weight == 0 )
        return ;

    // To get here, eye bones and animations must be present!
    ASSERT( m_iLEyeBone != -1 );
    ASSERT( m_iREyeBone != -1 );
    ASSERT( m_iUDAnim   != -1 );
    ASSERT( m_iLRAnim   != -1 );

    // Skip if the eye bones are not currently being displayed by the geometry LOD
    if(     ( Info.m_nActiveBones <= m_iLEyeBone )
        ||  ( Info.m_nActiveBones <= m_iREyeBone ) )
        return;

    // Get delta from target 
    // (since head L2W rotation pts out the backwards due to max 180 export)
    vector3 Delta = Info.m_MidEyePosition - m_LookAt;

    // Put into space of head
    Delta = Info.m_HeadL2W.InvRotateVector(Delta) ;

    // Compute the pitch and yaw to set the eyes to
    radian Pitch, Yaw ;
    Delta.GetPitchYaw( Pitch, Yaw ) ;

    // Lookup anim infos
    const anim_info& UDAnimInfo = GetAnimInfo( m_iUDAnim ) ;
    const anim_info& LRAnimInfo = GetAnimInfo( m_iLRAnim ) ;
    
    // Lookup frame info
    s32 nUDFrames = UDAnimInfo.GetNFrames()-1;
    s32 nLRFrames = LRAnimInfo.GetNFrames()-1;
    
    // Convert pitch and yaw to animation frame
    f32 PitchFrame = ComputeFrame( -Pitch, m_MinPitch, m_MaxPitch, 0.0f, (f32)nUDFrames - 0.0001f ) ;
    f32 YawFrame   = ComputeFrame( -Yaw,  m_MinYaw,   m_MaxYaw,   0.0f, (f32)nLRFrames - 0.0001f ) ;

    // Add pitch keys to eyes
    AdditiveMixKeys( Info, m_iUDAnim, PitchFrame, nUDFrames >> 1, pDestKey ) ;

    // Add yaw keys to eyes
    AdditiveMixKeys( Info, m_iLRAnim, YawFrame  , nLRFrames >> 1, pDestKey ) ;
}
    
//=========================================================================

void loco_eye_controller::Advance( f32 nSeconds, vector3&  DeltaPos, radian& DeltaYaw )
{
    // No deltas are used
    DeltaPos.Zero() ;
    DeltaYaw = R_0 ;

    // Blend to new target eye positions
    f32 BlendSpeed = x_min( 1.0f, 30.0f * nSeconds * m_TargetBlendSpeed ) ;
    m_LookAt += (m_TargetLookAt - m_LookAt) * BlendSpeed ;
}

//=========================================================================

// Sets the position to look at. Will use blending by default
void loco_eye_controller::SetLookAt( const vector3& Target, xbool bBlendTowards /*= TRUE*/ )
{
    // If blending, update the new target
    if (bBlendTowards)
        m_TargetLookAt = Target ;
    else
        m_TargetLookAt = m_LookAt = Target ;
}

//=========================================================================
