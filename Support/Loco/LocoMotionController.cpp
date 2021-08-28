//=========================================================================
//
//  LocoMotionController.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Loco.hpp"


//=========================================================================
// FUNCTIONS
//=========================================================================

loco_motion_controller::loco_motion_controller ( void ) : 
    loco_anim_controller(),
    m_iMotionProp   ( -1    ),  // Index to motion prop or -1 if none
    m_Yaw           ( 0     ),  // Current yaw

    m_iMixAnim      ( -1    ),  // Index of mix animation (or -1 if none)
    m_Mix           ( 0.0f  ),  // Amount of mix animation (0 = none, 1 = all)
    m_MixAnimFrame  ( 0.0f  )   // Current frame of mix animation
{
}

//=========================================================================

loco_motion_controller::~loco_motion_controller     ( void )
{
}

//=========================================================================
// Misc functions
//=========================================================================

void loco_motion_controller::Clear( void )
{
    // Call base class
    loco_anim_controller::Clear() ;

    // Clear
    m_iMotionProp       = -1,
    m_Yaw               = 0.0f ;

    m_iMixAnim          = -1;
    m_Mix               = 0.0f;
    m_MixAnimFrame      = 0.0f;
}

//=========================================================================

void loco_motion_controller::SetAnim( const anim_group::handle& hAnimGroup, s32 iAnim, u32 Flags )
{
    // No animation?
    if ( iAnim == -1 )
        return;
    
    // Call base class
    loco_anim_controller::SetAnim(hAnimGroup, iAnim, Flags) ;

    // Get anim info
    const anim_info& AnimInfo = GetAnimInfo() ;

    // Lookup motion prop
    m_iMotionProp = AnimInfo.GetPropChannel("MotionProp") ;

#ifndef X_RETAIL
    if( m_iMotionProp != -1 )
        ASSERTS( AnimInfo.GetPropParentBoneIndex( m_iMotionProp) == -1, "Motion prop must not have a parent!!" );
#endif    
}

//=========================================================================

// Advance animation and extracts motion
void loco_motion_controller::Advance( const anim_info& AnimInfo,
                                            f32        DeltaFrame,
                                            f32&       Frame,
                                            f32&       PrevFrame,
                                            s32&       Cycle,
                                            s32&       PrevCycle,
                                            vector3&   DeltaPos,
                                            radian&    DeltaYaw )
{
    // If motion prop is defined, then make sure it's in normal and mix anim!
    if( m_iMotionProp != -1 )
    {
        ASSERTS( m_iMotionProp == AnimInfo.GetPropChannel("MotionProp"),
                 xfs( "%s no MotionProp!", AnimInfo.GetName() ) );
    }

    // Lookup info from anim
    s32 nFrames = AnimInfo.GetNFrames();
    
    // Clear deltas
    DeltaPos.Zero() ;
    DeltaYaw = 0 ;

    // Get first and last keys of motion
    anim_key Key0;
    anim_key Key1;
    GetMotionRawKey( AnimInfo, 0, Key0 );
    GetMotionRawKey( AnimInfo, nFrames-1, Key1 );

    // Get root key from old frame
    anim_key OldKey;
    GetMotionInterpKey( AnimInfo, Frame, OldKey );

    // Remember previous frame and cycle
    PrevFrame = Frame;
    PrevCycle = Cycle;

    // Advance frame 
    Frame += DeltaFrame;

    // Update which cycle we are in and modulate the frame
    while( Frame >= (nFrames-1) )
    {
        Frame -= (nFrames-1);
        Frame += AnimInfo.GetLoopFrame();
        Cycle++;
    }

    // If the anim doesn't loop and we are past the end then peg at the end
    if( (!m_bLooping) && ((Cycle>0) || (Frame >= (nFrames-2))) )
    {
        PrevCycle = 0;
        Cycle     = 0;
        Frame     = (f32)(nFrames-2);
    }

    // Are we not in the same cycle?
    s32 OldCycle = PrevCycle;
    s32 NewCycle = Cycle;

    // Catch up to current cycle
    while( OldCycle < NewCycle )
    {
        // Advance to end of this cycle
        DeltaPos += Key1.Translation - OldKey.Translation;
        DeltaYaw += x_MinAngleDiff( Key1.Rotation.GetRotation().Yaw, OldKey.Rotation.GetRotation().Yaw );
        OldKey = Key0;
        OldCycle++;
    }

    // Get new key
    anim_key NewKey;
    GetMotionInterpKey( AnimInfo, Frame, NewKey );

    // Compute deltas in animation space
    DeltaPos += NewKey.Translation - OldKey.Translation;
    DeltaYaw += x_MinAngleDiff( NewKey.Rotation.GetRotation().Yaw, OldKey.Rotation.GetRotation().Yaw );

    // Clear delta XZ if we are not accumulating it (it's in the anim)
    if (!m_bAccumHorizMotion)
        DeltaPos.GetX() = DeltaPos.GetZ() = 0 ;

    // Clear delta Y if we are not accumulating it (it's in the anim)
    if (!m_bAccumVertMotion)
        DeltaPos.GetY() = 0 ;

    // Clear delta yaw if not accumulating yaw
    if (!m_bAccumYawMotion)
        DeltaYaw = 0 ;

    // Start with the anim yaw
    radian YawFix = GetRootBoneFixupYaw() ;

    // Since turns add to the yaw, I need to anti-rotate the translation by this amount
    // to keep it in sync!
    if ( m_bRemoveYawMotion )
    {
        // Get yaw of current frame
        radian CurrYaw  = NewKey.Rotation.GetRotation().Yaw ;

        // Using motion prop?    
        if( m_iMotionProp != -1 )
        {
            // Motion prop is along axis so remove it all
            YawFix -= CurrYaw + R_180;
        }
        else
        {
            // Get yaw of 1st frame
            radian FirstYaw = Key0.Rotation.GetRotation().Yaw ;

            // Remove what's been added so far!
            YawFix -= CurrYaw - FirstYaw ;
        }
    }

    // Adjust yaw
    m_Yaw += DeltaYaw ;

    // Rotate delta pos by current yaw
    DeltaPos.RotateY(YawFix) ;
}

//=========================================================================

void loco_motion_controller::Advance( f32 DeltaTime, vector3&  DeltaPos, radian& DeltaYaw )
{
    ASSERT( DeltaTime <= 0.1f );

    // No anim?
    if( m_iAnim == -1 )
        return;
    
    // If no rate (cinema object can drive anim) then exit now so PrevCycle, PrevFrame are not messed with which breaks cinema events
    if( m_Rate == 0.0f )
        return;
        
    // Lookup anim group
    const anim_group& AnimGroup = GetAnimGroup();
    
    // Lookup main anim info
    const anim_info&  AnimInfo  = AnimGroup.GetAnimInfo( m_iAnim );
    f32 nAnimFrames    = (f32)AnimInfo.GetNFrames()-1;
    f32 AnimFPS        = (f32)AnimInfo.GetFPS();
    f32 AnimLength     = nAnimFrames / AnimFPS;
    f32 AnimDeltaFrame = DeltaTime * AnimFPS * m_Rate;

    // Mixing both anims?
    if( m_iMixAnim != -1 )
    {
        // Lookup mix anim info
        const anim_info&  MixAnimInfo  = AnimGroup.GetAnimInfo( m_iMixAnim );
        f32 nMixAnimFrames    = (f32)MixAnimInfo.GetNFrames()-1;
        f32 MixAnimFPS        = (f32)MixAnimInfo.GetFPS();
        f32 MixAnimLength     = nMixAnimFrames / MixAnimFPS;
        f32 MixAnimDeltaFrame = DeltaTime * MixAnimFPS * m_Rate;

        // Get ratio of animations playback rate
        f32 Ratio = AnimLength / MixAnimLength;
        ASSERT( Ratio > 0.0001f );

        // Compute main anim delta frame:
        //  when Mix=0, use DeltaFrame0
        //  when Mix=1, use DeltaFrame1
        f32 DeltaFrame0 = AnimDeltaFrame;
        f32 DeltaFrame1 = AnimDeltaFrame * Ratio;
        f32 DeltaFrame  = DeltaFrame0 + ( m_Mix * ( DeltaFrame1 - DeltaFrame0 ) );

        // Compute mix anim frame via matching the anims parametrically
        m_MixAnimFrame = nMixAnimFrames * ( m_Frame / nAnimFrames );

        // Advance main animation and extract motion info
        Advance( AnimInfo,
                 DeltaFrame,
                 m_Frame,
                 m_PrevFrame,
                 m_Cycle,
                 m_PrevCycle,
                 DeltaPos,
                 DeltaYaw );

        // Compute mix anim delta frame:
        //  when Mix=0, use DeltaFrame1
        //  when Mix=1, use DeltaFrame0
        DeltaFrame0 = MixAnimDeltaFrame;
        DeltaFrame1 = MixAnimDeltaFrame / Ratio;
        DeltaFrame  = DeltaFrame1 + ( m_Mix * ( DeltaFrame0 - DeltaFrame1 ) );

        // Extract motion info from mix animation
        f32     PrevFrame = m_MixAnimFrame;
        s32     Cycle     = m_Cycle;
        s32     PrevCycle = Cycle;
        vector3 MixDeltaPos(0,0,0);
        radian  MixDeltaYaw = 0;
        Advance( MixAnimInfo,
                 DeltaFrame,
                 m_MixAnimFrame,
                 PrevFrame,
                 Cycle,
                 PrevCycle,
                 MixDeltaPos,
                 MixDeltaYaw );

        // Mix motion deltas into main deltas
        DeltaPos += m_Mix * ( MixDeltaPos - DeltaPos );
        DeltaYaw += m_Mix * ( x_MinAngleDiff( MixDeltaYaw, DeltaYaw ) );
    }
    else
    {
        // Compute main anim delta frame
        f32 DeltaFrame = DeltaTime * AnimFPS * m_Rate;
    
        // Advance main anim
        Advance( AnimInfo,
                 DeltaFrame,
                 m_Frame,
                 m_PrevFrame,
                 m_Cycle,
                 m_PrevCycle,
                 DeltaPos,
                 DeltaYaw );
    }
    
    ASSERT( x_abs( DeltaPos.GetX() ) < ( 100.0f * 10.0f ) );
    ASSERT( x_abs( DeltaPos.GetY() ) < ( 100.0f * 10.0f ) );
    ASSERT( x_abs( DeltaPos.GetZ() ) < ( 100.0f * 10.0f ) );
}

//=========================================================================
// Root bone functions
//=========================================================================

radian loco_motion_controller::GetRootBoneFixupYaw( void )
{
    // R_180 is because animations are exported 180 degrees out with the engine space!
    return m_Yaw + R_180 ;
}

//=========================================================================
// Yaw functions
//=========================================================================

radian loco_motion_controller::GetStartYaw( void )
{
    // No anim?
    if (m_iAnim == -1)
        return 0 ;

    // Lookup anim data
    const anim_info& AnimInfo = GetAnimInfo() ;

    // Get root node key of frame 0
    anim_key RootKey ;
    AnimInfo.GetRawKey(0,0,RootKey) ;

    // Get yaw
    radian3 Rot = RootKey.Rotation.GetRotation() ;
    return Rot.Yaw ;
}

//=========================================================================

f32 loco_motion_controller::GetMovementSpeed( void ) const
{
    // No animation?
    if ( m_iAnim == -1 )
        return 0.0f;

    // Lookup main anim move speed
    const anim_info& AnimInfo = GetAnimInfo();
    f32 MoveSpeed = AnimInfo.GetSpeed();
    
    // Using mix anim also?
    if( ( m_iMixAnim != -1 ) && ( m_Mix != 0.0f ) )
    {
        // Lookup mix anim move speed
        const anim_info& MixAnimInfo = GetAnimInfo( m_iMixAnim );
        f32 MixMoveSpeed = MixAnimInfo.GetSpeed();
        
        // Blend with main anim
        // 0 = All main anim
        // 1 = All mix anim
        MoveSpeed += ( MixMoveSpeed - MoveSpeed ) * m_Mix;
    }
    
    return MoveSpeed;
}

//=========================================================================

// Sets playback rate to match delta position (returns rate)
f32 loco_motion_controller::SetMatchingRate( const vector3& DeltaPos, f32 DeltaTime, f32 RateMin, f32 RateMax )
{
    // No animation?
    if( m_iAnim == -1 )
        return 0.0f;

    // Compute speeds
    f32 MoveSpeedSqr = x_sqr( DeltaPos.GetX() ) + x_sqr( DeltaPos.GetZ() );
    f32 AnimSpeed    = GetMovementSpeed() * DeltaTime;

    // Update rate
    f32 Rate = 0.0f;
    if( ( MoveSpeedSqr > x_sqr(0.000001f) ) && ( AnimSpeed > 0.000001f ) )
    {
        // Compute rate
        f32 MoveSpeed = x_sqrt( MoveSpeedSqr );
        Rate = MoveSpeed / AnimSpeed;

        // Set valid rate
        m_Rate = Rate;

        // Range check for anim rate, but return computed rate
        m_Rate = x_min( m_Rate, RateMax );
        m_Rate = x_max( m_Rate, RateMin );
    }

    return Rate;
}


//=========================================================================
// Key mixing
//=========================================================================

void loco_motion_controller::FixRootBoneKey( const anim_info& AnimInfo, f32 Frame, anim_key& RootBoneKey )
{
    // Lookup current key
    anim_key MotionKey ;
    GetMotionInterpKey( AnimInfo, Frame, MotionKey );

    // Remove XZ from animation?
    if( m_bRemoveHorizMotion )
    {
        RootBoneKey.Translation.GetX() -= MotionKey.Translation.GetX() ;
        RootBoneKey.Translation.GetZ() -= MotionKey.Translation.GetZ() ;
    }

    // Remove Y from animation?
    if( m_bRemoveVertMotion )
    {
        // Remove Y translation of the root bone
        RootBoneKey.Translation.GetY() -= MotionKey.Translation.GetY() ;
    }

    // Remove yaw from animation?
    if (m_bRemoveYawMotion)
    {
        radian DeltaYaw;
        radian CurrYaw = MotionKey.Rotation.GetRotation().Yaw ;

        // Using motion prop?        
        if( m_iMotionProp != -1 )
        {
            // Motion prop is along axis so remove it all
            DeltaYaw = -CurrYaw + R_180;
        }
        else
        {    
            // Get 1st key
            anim_key FirstKey;
            GetMotionRawKey( AnimInfo, 0, FirstKey) ;

            // Get yaw of first frame
            radian FirstYaw = FirstKey.Rotation.GetRotation().Yaw ;

            // Since root bone may not be down axis, use difference
            DeltaYaw = FirstYaw - CurrYaw;
        }

        // Make yaw be that of the 1st frame
        quaternion DeltaRot( vector3(0,1,0), DeltaYaw) ;
        RootBoneKey.Rotation = DeltaRot * RootBoneKey.Rotation ;
        RootBoneKey.Translation.RotateY(DeltaYaw) ;
    }
}

//=========================================================================

void loco_motion_controller::GetInterpKeys( const info& Info, anim_key* pKey )
{
    s32 i ;

    // Clear keys if no animation
    if( m_iAnim == -1 )
    {
        for( i=0; i< Info.m_nActiveBones; i++ )
            pKey[i].Identity();

        return;
    }

    // Lookup anim group
    const anim_group& AnimGroup = GetAnimGroup();

    // If this assert fires off, then either the geometry and anim group have different numbers of bones,
    // ie. the geometry and anim group file are using a different bind pose matxs,
    // or this particular anim group has a different number of bones than an anim group used in
    // the same player ie. multiple anim groups used in this player have different bind pose matxs.
    // (this can happen when using "PlayAnim" with other anim group packages).
    // Either way - fix your resources!
    ASSERTS( (Info.m_nActiveBones <= AnimGroup.GetNBones()), "Incompatible anim group with bone lods!" ) ;

    // Grab main animation keys and fix root bone key
    const anim_info&  AnimInfo  = AnimGroup.GetAnimInfo( m_iAnim );
    AnimInfo.GetInterpKeys( m_Frame, pKey, Info.m_nActiveBones ) ;
    FixRootBoneKey( AnimInfo, m_Frame, pKey[0] );
    
    // Blend in mix animation also?
    if( ( m_iMixAnim != -1 ) && ( m_Mix != 0.0f ) )
    {
        // Get mixing buffer
        anim_key* pMixAnimKeys = base_player::GetMixBuffer( base_player::MIX_BUFFER_TEMP );
        ASSERT( pMixAnimKeys );

        // Grab mix animation keys and fix root bone key
        const anim_info& MixAnimInfo = AnimGroup.GetAnimInfo( m_iMixAnim );
        MixAnimInfo.GetInterpKeys( m_MixAnimFrame, pMixAnimKeys, Info.m_nActiveBones ) ;
        FixRootBoneKey( MixAnimInfo, m_MixAnimFrame, pMixAnimKeys[0] );
        
        // Now blend the keys with the main anim
        for( i = 0 ; i < Info.m_nActiveBones ; i++ )
            pKey[i].Interpolate( pKey[i], pMixAnimKeys[i], m_Mix );
    }
}

//=========================================================================
// Motion functions
//=========================================================================

void loco_motion_controller::GetMotionRawKey( const anim_info& AnimInfo, s32 Frame, anim_key& Key )
{
    ASSERT( Frame>=0 );
    
    // If no motion prop, use the root bone
    if( ( m_iMotionProp < 0 ) || ( m_iMotionProp >= AnimInfo.GetNProps() ) )
        AnimInfo.GetRawKey( Frame, 0, Key );
    else
        AnimInfo.GetPropRawKey( m_iMotionProp, Frame, Key );
}

//=========================================================================

void loco_motion_controller::GetMotionInterpKey(const anim_info& AnimInfo, f32 Frame, anim_key& Key )
{
    ASSERT( Frame>=0 );
    
    // If no motion prop, use the root bone
    if( ( m_iMotionProp < 0 ) || ( m_iMotionProp >= AnimInfo.GetNProps() ) )
        AnimInfo.GetInterpKey( Frame, 0, Key );
    else
        AnimInfo.GetPropInterpKey( m_iMotionProp, Frame, Key );
}

//=========================================================================
