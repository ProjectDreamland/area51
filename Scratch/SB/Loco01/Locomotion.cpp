
#include "Locomotion.hpp"
#include "e_Engine.hpp"


//==============================================================================
//==============================================================================
//==============================================================================
// ANIMATION LOADER
//==============================================================================
//==============================================================================
//==============================================================================
class anim_loader : public rsc_loader
{
public:
    anim_loader( const char* pType, const char* pExt ) : rsc_loader(pType,pExt) {}

    virtual void*   PreLoad         ( X_FILE* pFP   );
    virtual void*   Resolve         ( void*   pData ) {return pData;}
    virtual void    Unload          ( void*   pData ) ;
};

//==============================================================================

anim_loader AnimLoader("Animation",".anim");

//==============================================================================

void* anim_loader::PreLoad( X_FILE* pFP )
{
    x_DebugMsg("loading animation\n");

    // Allocate a new instance of geometry
    anim_group* pAnimGroup = new anim_group();
    ASSERT( pAnimGroup );

    // Load geom
    pAnimGroup->Load( pFP );
    
    return pAnimGroup;
}

//==============================================================================

void anim_loader::Unload( void* pData  )
{
    x_DebugMsg("unloading animation\n");
    delete (anim_group*)pData;
}


//=========================================================================
//=========================================================================
//=========================================================================
// loco_aim_controler
//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================

void loco_aim_controler::SetBlendFactor( f32 wSide, f32 wUpDown, f32 BlendTime )
{
    m_wSide       = wSide;
    m_wUpDown     = wUpDown;
    m_BlendLength = BlendTime;
    m_BlendFrame  = 0;
}

//=========================================================================

loco_aim_controler::loco_aim_controler  ( void )
{
    Clear();
}

//=========================================================================

void loco_aim_controler::Init( s32 iLT, s32 iLC, s32 iLB,
                               s32 iCT, s32 iCC, s32 iCB,
                               s32 iRT, s32 iRC, s32 iRB )
{
    m_iLT = iLT;
    m_iLC = iLC;
    m_iLB = iLB;
    m_iCT = iCT;
    m_iCC = iCC;
    m_iCB = iCB;
    m_iRT = iRT;
    m_iRC = iRC;
    m_iRB = iRB;

    m_nFrames = GetAnimData( m_iCC ).GetNFrames();    
    
    ASSERT( GetAnimData( m_iLT ).GetNFrames() == m_nFrames );
    ASSERT( GetAnimData( m_iLC ).GetNFrames() == m_nFrames );
    ASSERT( GetAnimData( m_iLB ).GetNFrames() == m_nFrames );
    ASSERT( GetAnimData( m_iCT ).GetNFrames() == m_nFrames );
    ASSERT( GetAnimData( m_iCC ).GetNFrames() == m_nFrames );
    ASSERT( GetAnimData( m_iCB ).GetNFrames() == m_nFrames );
    ASSERT( GetAnimData( m_iRT ).GetNFrames() == m_nFrames );
    ASSERT( GetAnimData( m_iRC ).GetNFrames() == m_nFrames );
    ASSERT( GetAnimData( m_iRB ).GetNFrames() == m_nFrames );
}

//=========================================================================

void loco_aim_controler::SetWeight( f32 ParametricWeight )
{
    m_Weight = ParametricWeight;
}

//=========================================================================

f32 loco_aim_controler::GetWeight( void )
{
    return m_Weight;
}

//=========================================================================

void loco_aim_controler::MixKeys( anim_key* pDestKey )
{
    // If we aren't playing anything then just return
    if( (m_iCC == -1) || (m_Weight==0.0f) )
        return;

    s32 i;
    s32 nBones = GetAnimGroup().GetNBones();

    // Read interpolated keys from the animation
    GetInterpKeys( m_TempKey );

    // Check if this animation has bone masks
    if( GetAnimData( m_iCC ).HasMasks() )
    {
        // Blend destination into track keys by weight amount
        for( i=0; i<nBones; i++ )
        if( !GetAnimData( m_iCC ).IsBoneMasked(i) )
            pDestKey[i].InterpolateSRT( pDestKey[i], m_TempKey[i], m_Weight );
    }
    else
    {
        // Blend destination into track keys by weight amount
        for( i=0; i<nBones; i++ )
        {
            pDestKey[i].InterpolateSRT( pDestKey[i], m_TempKey[i], m_BoneMask[i].Weight*m_Weight );
        }
    }
}

//=========================================================================

void loco_aim_controler::MixKey( s32 iBone, anim_key& DestKey )
{
    // If we aren't playing anything then just return
    if( (m_iCC == -1) || (m_Weight==0.0f) )
        return;

    // Read interpolated keys from the animation
    anim_key Temp;
    GetInterpKey( iBone, Temp );

    // Check if this animation has bone masks
    if( !GetAnimData( m_iCC ).IsBoneMasked(iBone) )
        DestKey.InterpolateSRT( DestKey, Temp, m_Weight );
}

//=========================================================================

void loco_aim_controler::GetInterpKeys( anim_key* pKey )
{
    s32 i;
    s32 nBones = GetAnimGroup().GetNBones();

    // Clear keys if no animation
    if( m_iCC == -1 )
    {
        for( i=0; i<nBones; i++ )
        {
            pKey[i].Rotation.Identity();
            pKey[i].Scale(1,1,1);
            pKey[i].Translation(0,0,0);
        }
        return;
    }

    //
    // Interpolate the side keys
    //
    f32         wVBlend, wHBlend;
    anim_key    Temp1Key[MAX_ANIM_TRACK_BONES];     
    anim_key    Temp2Key[MAX_ANIM_TRACK_BONES];     

    //
    // Solve first vertical interpolation
    //
    GetAnimData( m_iCC ).GetInterpKeys( m_Frame, pKey );

    if( m_wUpDown >= 0 )
    {
        GetAnimData( m_iCT ).GetInterpKeys( m_Frame, Temp1Key );
        wVBlend = m_wUpDown;
    }
    else
    {
        GetAnimData( m_iCB ).GetInterpKeys( m_Frame, Temp1Key );
        wVBlend = -m_wUpDown;
    }

    for( i=0; i<nBones; i++ )
    {
        pKey[i].InterpolateSRT( pKey[i], Temp1Key[i], wVBlend );
    }

    //
    // Solve second vertical interpolation
    //
    if( m_wSide >= 0 )
    {
        GetAnimData( m_iRC ).GetInterpKeys( m_Frame, Temp2Key );
        wHBlend = m_wSide;

        if( m_wUpDown >= 0 )
        {
            GetAnimData( m_iRT ).GetInterpKeys( m_Frame, Temp1Key );
        }
        else
        {
            GetAnimData( m_iRB ).GetInterpKeys( m_Frame, Temp1Key );
        }
    }
    else
    {
        GetAnimData( m_iLC ).GetInterpKeys( m_Frame, Temp2Key );
        wHBlend = -m_wSide;

        if( m_wUpDown >= 0 )
        {
            GetAnimData( m_iLT ).GetInterpKeys( m_Frame, Temp1Key );
        }
        else
        {
            GetAnimData( m_iLB ).GetInterpKeys( m_Frame, Temp1Key );
        }
    }

    for( i=0; i<nBones; i++ )
    {
        Temp1Key[i].InterpolateSRT( Temp2Key[i], Temp1Key[i], wVBlend );
    }

    //
    // Solve final horizontal interpolation
    //
    for( i=0; i<nBones; i++ )
    {
        pKey[i].InterpolateSRT( pKey[i], Temp1Key[i], wHBlend );
    }

    //
    // Blend with previous anim exit keyframes
    //
    if( m_BlendLength > 0.0f )
    {
        f32 T = m_BlendFrame / m_BlendLength;
        for( i=0; i<nBones; i++ )
            if( m_BoneMask[i].Weight > 0 )
            {
                pKey[i].InterpolateSR( m_pBlendKey[i], pKey[i], T );
                m_pBlendKey[i] = pKey[i];
            }
    }   
}

//=========================================================================

void loco_aim_controler::SetBoneMask( f32 Weight, const char* pBoneName )
{
    s32 Index = GetAnimGroup().GetBoneIndex( pBoneName );
    ASSERT( Index != -1 );

    m_BoneMask[Index].iBone  = Index;
    m_BoneMask[Index].Weight = Weight;    
}

//=========================================================================

void loco_aim_controler::GetInterpKey( s32 iBone, anim_key& Key )
{
    // Clear keys if no animation
    if( m_iCC == -1 )
    {
        Key.Rotation.Identity();
        Key.Scale(1,1,1);
        Key.Translation(0,0,0);
        return;
    }

    ASSERT( 0 );
    //
    // Interpolate the side keys
    //
    /*
    {
        f32         wBlend;
        anim_key    TempKey;

        GetAnimData( m_iCC ).GetInterpKey( m_Frame, iBone, Key );

        if( m_wSide >= 0 )
        {
            GetAnimData( m_iRight ).GetInterpKey( m_Frame, iBone, TempKey );
            wBlend = m_wSide;
        }
        else
        {
            GetAnimData( m_iLeft  ).GetInterpKey( m_Frame, iBone, TempKey );
            wBlend = -m_wSide;
        }

        Key.InterpolateSR( Key, TempKey, wBlend );
    }
    */

    //
    // Blend with previous anim exit keyframes
    //
    if( m_BlendLength > 0.0f )
    {
        f32 T = m_BlendFrame / m_BlendLength;
        Key.InterpolateSR( m_pBlendKey[iBone], Key, T );
    }
}

//=========================================================================

void loco_aim_controler::Clear( void )
{
    m_iLT = -1;
    m_iLC = -1;
    m_iLB = -1;
    m_iCT = -1;
    m_iCC = -1;
    m_iCB = -1;
    m_iRT = -1;
    m_iRC = -1;
    m_iRB = -1;

    m_wSide     = 0.5;     
    m_wUpDown   = 0.5;
    m_Weight    = 1;
    m_Rate      = 1.0f;

    x_memset( m_BoneMask, 0, sizeof(bone_mask)*MAX_ANIM_TRACK_BONES );

    if( m_pBlendKey )
    {
        const anim_group& AG = GetAnimGroup();
        for( s32 i=0; i<AG.GetNBones(); i++ )
        {
            m_pBlendKey[i].Rotation.Identity();
            m_pBlendKey[i].Translation.Set(0,0,0);
            m_pBlendKey[i].Scale.Set(1,1,1);
        }
    }
}

//=========================================================================

void loco_aim_controler::SetAnimGroup( const rhandle<anim_group>& hGroup )
{
    if(m_pBlendKey) 
        delete[] m_pBlendKey;

    m_hAnimGroup        = hGroup;
   const anim_group& AG = GetAnimGroup();

    ASSERT( AG.GetNBones() <= MAX_ANIM_TRACK_BONES );

    m_pBlendKey = new anim_key[ AG.GetNBones() ];
    ASSERT( m_pBlendKey );

    Clear();
}

//=========================================================================

void loco_aim_controler::Advance( f32 nSeconds, vector3& DeltaPos, radian& DeltaYaw )
{
    DeltaPos.Zero() ;
    DeltaYaw = 0 ;

    if( m_iCC == -1 )
        return;

    //
    // Remember previous frame and cycle
    //
    m_PrevFrame = m_Frame;
    m_PrevCycle = m_Cycle;

    //
    // Count down blend time
    //
    m_BlendFrame += x_abs(nSeconds);
    if( m_BlendFrame >= m_BlendLength )
    {
        m_BlendFrame = 0.0f;
        m_BlendLength= 0.0f;
    }

    //
    // Advance frame 
    //
    f32 nFrames = nSeconds * (f32)GetAnimData( m_iCC ).GetFPS() * m_Rate;
    m_Frame += nFrames;

    // Update which cycle we are in and modulate the frame
    while( m_Frame >= (m_nFrames-1) )
    {
        m_Frame -= (m_nFrames-1);
        m_Cycle++;
    }

    // If the anim doesn't loop and we are past the end then peg at the end
    if( (!GetAnimData( m_iCC ).DoesLoop()) && ((m_Cycle>0) || (m_Frame >= (m_nFrames-2))) )
    {
        m_Cycle = 0;
        m_Frame = (f32)(m_nFrames-2);
    }
}

//=========================================================================

const anim_data& loco_aim_controler::GetAnimData( s32 iAnim ) 
{
    ASSERT( iAnim >= 0 );
    return GetAnimGroup().GetAnimData( iAnim );
}

//=========================================================================

const anim_group& loco_aim_controler::GetAnimGroup( void )
{
    anim_group* pGroup = (anim_group*)m_hAnimGroup.GetPointer();
    ASSERT( pGroup );
    return *pGroup;
}

//=========================================================================
//=========================================================================
//=========================================================================
// LOCOMOTION
//=========================================================================
//=========================================================================
//=========================================================================

//=========================================================================
locomotion::locomotion( void )
{
    m_bTransition = FALSE ;
}

//=========================================================================

void locomotion::Initialize( const char* pFileName )
{
    //
    // Initialize the player
    //
    m_AnimGroup.SetName( pFileName );
    m_Player.SetTrackController( 2, &m_Controler );
    m_Player.SetAnimGroup( m_AnimGroup );
    
    // TEMP!!
    //m_Player.SetTrackController( 2, NULL) ;

    
    //
    // Send the Initialize message
    //
    OnInit();
}

//=========================================================================

void locomotion::OnAdvance( f32 nSeconds )
{
    vector3 DeltaPos;
    m_Player.Advance( nSeconds, DeltaPos );
}

//=========================================================================

void locomotion::RenderSkeleton( xbool LabelBones )
{
    //m_Player.RenderSkeleton( LabelBones);

    radian Yaw = m_Player.GetYaw();
    vector3 P(0,0,100);
    P.RotateY(Yaw);
    draw_Line( m_Player.GetPosition(), m_Player.GetPosition()+P, XCOLOR_WHITE );
    
    Yaw = m_MotionYaw;//m_DestYaw ;
    P.Set(0,0,200);
    P.RotateY(Yaw);
    draw_Line( m_Player.GetPosition(), m_Player.GetPosition()+P, XCOLOR_YELLOW );
    
    //Yaw = m_DestYaw ;
    //P.Set(0,0,200);
    //P.RotateY(Yaw);
    //draw_Line( m_Player.GetPosition(), m_Player.GetPosition()+P, XCOLOR_YELLOW );
#if 0    
    matrix4 RootL2W = m_Player.GetBoneL2W(0);
    vector3 Q;
    P.Set(0,0,100);
    P = RootL2W.RotateVector(P);
    P.RotateY(R_180);
    P  += RootL2W.GetTranslation();
    Q   = P;
    P.Y = RootL2W.GetTranslation().Y;
    draw_Line( RootL2W.GetTranslation(), P, XCOLOR_RED );
    draw_Line( P, Q, XCOLOR_GREEN );    
#endif
}

//=========================================================================

void locomotion::ComputeL2W( matrix4* pL2W )
{
    x_memcpy( pL2W, m_Player.GetBoneL2Ws(), m_Player.GetNBones()*sizeof(matrix4) );
}

//=========================================================================

void locomotion::SetMoveAt( vector3& Target )
{
    m_MoveAt = Target;
}

//=========================================================================

void locomotion::SetLootAt( vector3& Target )
{
    m_LookAt = Target;
}

//=========================================================================

void locomotion::ComputeAim( radian& H, radian& V )
{
    vector3 V1, V2, CurDir, DesireDir;
    CurDir.Set(0,0,1);

    matrix4 M = m_Player.GetBoneL2W(0);
    M.InvertSRT();

    DesireDir = M * (m_LookAt - vector3(0,150,0));
    DesireDir.RotateY(R_180);

    //
    // Get the horizontal componet
    //
    V1.Set( CurDir.X,    0, CurDir.Z );
    V2.Set( DesireDir.X, 0, DesireDir.Z );

    H = v3_AngleBetween( V1, V2 );
    H *= 1.1f;//0.95f; // fuch factor for the aiming

    if( v3_Dot( vector3(1,0,0), V2 ) < 0 )
        H = -H;        

    //
    // Get the vertical componet
    //
    V1.Set( CurDir.Y, 0, CurDir.Z );
    V2.Set( DesireDir.Y, 0, DesireDir.Z );

    V = v3_AngleBetween( V1, V2 );
    if( v3_Dot( vector3(1,0,0), V2 ) < 0 )
        V = -V;   

    //x_printf( "%f %f \n", H, V );
    V = fMin( 1, fMax( -1, V) );
}

//=========================================================================

radian locomotion::ComputeMoveDir( void )
{
    vector3 DesireDir;
    vector3 CurDir;
    f32     R;

    //
    // make him move towards the right direction for the actual movement
    //
    DesireDir = m_MoveAt - m_Player.GetPosition();
    CurDir    = vector3(0,0,100) ;//* m_Player.GetRotation() ;
    R         = v3_AngleBetween( DesireDir, CurDir );

    if( v3_Dot( vector3(1,0,0), DesireDir ) < 0 )
        R = -R;        

    return R;
}

//=========================================================================

// Returns that target look yaw the character needs to obtain
radian locomotion::ComputeLookYaw( void )
{
    // Lookup move yaw to obtain
    vector3 MoveDir = m_MoveAt - m_Player.GetPosition() ;
    radian  MoveYaw  = MoveDir.GetYaw() ;

    // Lookup look yaw to obtain
    vector3 LookDir  = m_LookAt - m_Player.GetPosition() ;
    radian  LookYaw  = LookDir.GetYaw() ;

    // Setup 4 possible yaws to be at (making sure they are 90 degrees to movement yaw needed)
    radian DestYawList[4] ;
    DestYawList[0] = MoveYaw + R_0 ;
    DestYawList[1] = MoveYaw + R_90 ;
    DestYawList[2] = MoveYaw + R_180 ;
    DestYawList[3] = MoveYaw + R_270 ;

    // Find the closest yaw that matched the look yaw needed
    radian BestYaw   = DestYawList[0] ;
    radian BestDelta = F32_MAX ;
    s32    BestIndex = 0 ;
    for (s32 i = 0 ; i < 4 ; i++)
    {
        radian Delta = ABS(x_MinAngleDiff(DestYawList[i], LookYaw)) ;
        if (Delta < BestDelta)
        {
            BestDelta = Delta ;
            BestIndex = i ;
        }
    }
    return DestYawList[BestIndex] ;
}

//=========================================================================

// Returns motion, given the facing yaw
locomotion::motion locomotion::ComputeMotion( radian FacingYaw )
{
    // Setup the yaw that each motion currently represents
    radian MotionYaw[4] ;
    MotionYaw[0] = FacingYaw ;          // Forward
    MotionYaw[1] = FacingYaw + R_90 ;   // Left
    MotionYaw[2] = FacingYaw + R_180 ;  // Back
    MotionYaw[3] = FacingYaw + R_270 ;  // Right

    // Lookup move yaw to obtain
    vector3 MoveDir = m_MoveAt - m_Player.GetPosition() ;
    radian  MoveYaw  = MoveDir.GetYaw() ;

    // Choose closest motion to the direction we want to go
    s32 BestIndex = -1 ;
    f32 BestDelta = F32_MAX ;
    for (s32 i = 0 ; i < 4 ; i++)
    {
        radian Delta = ABS(x_MinAngleDiff(MoveYaw, MotionYaw[i])) ;
        if (Delta < BestDelta)
        {
            BestDelta = Delta ;
            BestIndex = i ;
        }
    }

    return (motion)BestIndex ;
}

//=========================================================================

xbool locomotion::SetMotionAnim( locomotion::motion FromMotion, 
                                 locomotion::motion ToMotion )
{
    m_Motion = ToMotion ;

    // Animn to choose
    const char* pAnim = NULL ;

    f32 BlendTime = 0.0f ;

    // Jump straight to motion?
    if (FromMotion == ToMotion)
    {
        // Wait for end of transitions
        if ( (m_bTransition) && (!m_Player.IsAtEnd()) )
            return FALSE ;

        switch(ToMotion)
        {
            case locomotion::MOTION_FORWARD:
                pAnim = "HSO_PROWL_FORWARD" ;
                break;
            case locomotion::MOTION_LEFT:
                pAnim = "HSO_PROWL_STRAFE_L" ;
                break;
            case locomotion::MOTION_RIGHT:
                pAnim = "HSO_PROWL_STRAFE_R" ;
                break;
            case locomotion::MOTION_BACK:
                pAnim = "HSO_PROWL_BACKWARD" ;
                break;
        }

        if (m_bTransition)
            BlendTime = 0.1f ; 
        else
            BlendTime = 0.25f; 

        m_bTransition = FALSE ;

    }
    else
    {
        // Wait for end of normal anim
        if (!m_Player.IsAtEnd())
            return FALSE ;

        m_bTransition = TRUE ;

        if ((FromMotion == MOTION_FORWARD) && (ToMotion == MOTION_RIGHT))
            pAnim = "HSO_PROWL_BLEND_F2R" ;
        else
        if ((FromMotion == MOTION_FORWARD) && (ToMotion == MOTION_LEFT))
            pAnim = "HSO_PROWL_BLEND_F2L" ;
        else
        if ((FromMotion == MOTION_FORWARD) && (ToMotion == MOTION_BACK))
            pAnim = "HSO_PROWL_BLEND_F2R" ;
        else
        if ((FromMotion == MOTION_BACK) && (ToMotion == MOTION_RIGHT))
            pAnim = "HSO_PROWL_BLEND_B2R" ;
        else
        if ((FromMotion == MOTION_BACK) && (ToMotion == MOTION_LEFT))
            pAnim = "HSO_PROWL_BLEND_B2L" ;
        else
        if ((FromMotion == MOTION_BACK) && (ToMotion == MOTION_FORWARD))
            pAnim = "HSO_PROWL_BLEND_B2L" ;
        else
        if ((FromMotion == MOTION_LEFT) && (ToMotion == MOTION_FORWARD))
            pAnim = "HSO_PROWL_BLEND_L2F" ;
        else
        if ((FromMotion == MOTION_LEFT) && (ToMotion == MOTION_BACK))
            pAnim = "HSO_PROWL_BLEND_L2B" ;
        else
        if ((FromMotion == MOTION_LEFT) && (ToMotion == MOTION_RIGHT))
            pAnim = "HSO_PROWL_BLEND_L2F" ;
        else
        if ((FromMotion == MOTION_RIGHT) && (ToMotion == MOTION_FORWARD))
            pAnim = "HSO_PROWL_BLEND_R2F" ;
        else
        if ((FromMotion == MOTION_RIGHT) && (ToMotion == MOTION_BACK))
            pAnim = "HSO_PROWL_BLEND_R2B" ;
        else
        if ((FromMotion == MOTION_RIGHT) && (ToMotion == MOTION_LEFT))
            pAnim = "HSO_PROWL_BLEND_R2B" ;

        BlendTime = 0.1f ;
    }

    // Switch to new anim
    ASSERT(pAnim) ;
    if (!m_Player.IsPlaying(pAnim))
    {
        x_DebugMsg("NewAnim:%s\n", pAnim) ;
        x_printf("NewAnim:%s\n", pAnim) ;
    }
    m_Player.SetAnimHoriz(pAnim, BlendTime) ;

    return TRUE ;
}

//=========================================================================


locomotion::motion locomotion::ComputeMotion( void )
{

#if 1
    // TEMP
    //m_LookAt = m_MoveAt ;

    // Compute look yaw to obtain
    radian LookYaw    = ComputeLookYaw() ;
    m_DestYaw = LookYaw ;

    // Compute motion to move at if we are facing look yaw
    motion LookMotion = ComputeMotion(LookYaw) ;

    // Compute motion to move at, given current facing yaw
    motion CurrMotion = ComputeMotion(m_Player.GetYaw()) ;

    // Get delta yaw
    radian CurrYaw  = m_Player.GetYaw() ;
    radian DeltaYaw = x_MinAngleDiff(LookYaw, CurrYaw) ;

    // Transition done?
    if ( (m_bTransition) && (m_Player.IsAtEnd()) )
    {
        m_bTransition = FALSE ;

        // Show info
        x_DebugMsg("Exiting transition from anim end!\n") ;
        x_printf("Exiting transition from anim end!\n") ;
    }

    // Playing a transition?
    if (m_bTransition)
    {
        // Good?
        if (ABS(DeltaYaw) < R_10)
        {
            //SetMotionAnim(LookMotion, LookMotion) ;
            //SetMotionAnim(CurrMotion, CurrMotion) ;
            //m_bTransition = FALSE ;
            //m_MotionYaw = m_Player.GetYaw() ;

            // Show info
            //x_DebugMsg("Exiting transition from delta yaw!\n") ;
            //x_printf("Exiting transition from delta yaw!\n") ;
        }

        // Keep going at current animation
        return m_ToTransition ;
    }
    else
    {
        m_MotionYaw = m_Player.GetYaw() ;

        // Goto look animation?
        if (ABS(DeltaYaw) < R_45)
        //if (0)
        {
            //if (DeltaYaw > (R_1/10.0f))
                //DeltaYaw = (R_1/10.0f) ;
            //else
            //if (DeltaYaw < -(R_1/10.0f))
                //DeltaYaw = -(R_1/10.0f) ;

            //m_Player.SetYaw(CurrYaw + DeltaYaw) ;
            m_Player.SetYaw(LookYaw) ;
            SetMotionAnim(LookMotion, LookMotion) ;
        }
        else
        {
            // Possible transition needed
            SetMotionAnim(CurrMotion, LookMotion) ;
        }
    }

    // Snap to current look?
    //if (ABS(DeltaYaw) < R_45)
    //{
        //m_Player.SetYaw(LookYaw) ;
        //return CurrMotion ;
    //}

    // Need to transition
    return CurrMotion ;

#else
    //
    // choose which animation we should be playing
    //    
    vector3 MoveDir, LookDir;
    MoveDir = m_MoveAt - m_Player.GetPosition();
    LookDir = m_LookAt - m_Player.GetPosition();
    MoveDir.Y = 0;
    LookDir.Y = 0;
    MoveDir.Normalize();
    LookDir.Normalize();



    vector3 D[4]; // F,L,B,R;
    f32 DirDot[4] = 
    { 
        x_cos(R_90/2), 
        x_cos(R_90/2), 
        x_cos(R_90/2), 
        x_cos(R_90/2), 
    };

    // Setup directions
    D[0] = LookDir;
    for( s32 i=0; i<3; i++ )
    {
        D[i+1] = D[i];
        D[i+1].RotateY(R_90);
    }

    //
    // Check if we're already happy
    //
    if( (m_Motion>=MOTION_FORWARD) && (m_Motion<=MOTION_RIGHT) )
    {
        // If the angle between the current motion the movement direction is < 45 then we are good
        if( D[m_Motion].Dot( MoveDir ) >= DirDot[m_Motion] )
            return m_Motion;
    }

    //
    // Need to find a new direction
    //
    s32 iBestDir = -1;
    f32 BestDot = -1;
    for( i=0; i<4; i++ )
    {
        f32 Dot = D[i].Dot( MoveDir );
        if( (Dot >= DirDot[i]) && (Dot > BestDot) )
        {
            BestDot = Dot;
            iBestDir = i;
        }
    }
    ASSERT( iBestDir >= 0 );

    return (motion)iBestDir;
#endif
}

