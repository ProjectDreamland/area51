//=============================================================================
//  DEBRIS_CANNON.CPP
//=============================================================================

//=============================================================================
// INCLUDES
//=============================================================================
#include "debris_cannon.hpp"
#include "e_Draw.hpp"
#include "e_ScratchMem.hpp"
#include "..\Support\GameLib\StatsMgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "GameLib\RenderContext.hpp"
#include "Render\RigidGeom.hpp"
#include "NetworkMgr\Networkmgr.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "..\Objects\Player.hpp"

//=============================================================================
// CONSTANTS
//=============================================================================
static const vector3    kDEBRIS_GRAVITY(0,-981,0);
static const f32        kTIME_BEFORE_FADING         = 5.0f;
static const f32        kFADE_TIME                  = 3.0f;

//SH: This limits the number of fx_handles that can be active at one time.
//    The fx_handles are used for the smoke and fire trailers that attach
//    to random bits of debris.
//
#ifdef TARGET_XBOX
static const s32        k_MAX_DEBRIS_CANNON_ACTIVE_FX_HANDLES       = 8*4;
#else
static const s32        k_MAX_DEBRIS_CANNON_ACTIVE_FX_HANDLES       = 8;
#endif

// An attempt to throttle the number of active fx
static s32 s_DebrisCannonActiveFXHandles = 0;

//=============================================================================
// FRAMGENT
//=============================================================================
debris_cannon::fragment::fragment()
{
    Reset();
}

//=============================================================================

debris_cannon::fragment::~fragment()
{
    if (m_FX.Validate())
    {
        ASSERT( s_DebrisCannonActiveFXHandles > 0 );
        s_DebrisCannonActiveFXHandles--;
        m_FX.KillInstance();
    }
}

//=============================================================================

void debris_cannon::fragment::Reset()
{
    m_Position.Set(0,0,0);
    m_Velocity.Set(0,0,0);
    m_Orientation.Setup(vector3(0,0,1),0);
    m_RotationAmount  = 0;
    m_RotationAxis.Set(0,0,1);
    m_OldPosition.Set(0,0,0);
    m_MeshMask  = 0;    
    m_bInUse    = FALSE;  
    m_bInactive = FALSE;
    m_bSuspendFXOnRest = FALSE;
    m_BounceCount       = 0;
}


//=============================================================================
// DEBRIS CANNON
//=============================================================================


debris_cannon::debris_cannon()
{
    m_nFragmentsUsed = 0;
    m_TotalTime      = 0;
    m_bFlybyActive   = FALSE;
    m_bFlybyPerformed = FALSE;
    m_FlybyVoiceID   = -1;
    m_FlybyTimer     = 0;
    m_FlybyLength    = 0;
    m_Alpha          = 255;

    m_LocalBBox.Set( vector3(0,0,0), 5 );
    s32 i;
    for (i=0;i<MAX_FRAGMENTS;i++)
    {
        m_Fragment[i].Reset();
    }

}

//=============================================================================

debris_cannon::~debris_cannon()
{

}

//=============================================================================

 void debris_cannon::OnInit              ( void )
{
    // These audio packages are not preloaded here for now.
    // Marc says there is a preloading related problem, and instead, he is 
    // going to use dummy sound emitters in the levels to get the audio
    // "officially" loaded in the level.
    //
    // Explosion_Main.audiopkg
    // Explosion_Powder.audiopkg
    (void)PRELOAD_FILE("CharMarks.decalpkg");
}

//=============================================================================

void debris_cannon::Create              ( const char*      pMeshName,
                                          const vector3&   Pos,
                                          const vector3&   Dir,
                                          s32              nFragments )
{
    m_RigidInst.SetUpRigidGeom( pMeshName );
    rigid_geom* pGeom = m_RigidInst.GetRigidGeom();
    if (NULL == pGeom)
        return;

    vector3 nDir = Dir;
    nDir.NormalizeAndScale(6);

    vector3 StartPos = Pos + nDir;

    s32 nMeshes = MIN(32,pGeom->m_nVirtualMeshes);
    
    m_LocalBBox.Set( vector3(0,0,0), 50 );
    s32 i;   
    nFragments = MIN(nFragments,MAX_FRAGMENTS);
    for (i=0;i<nFragments ;i++)
    {   
        s32 iFrag = GetFreeFragment();
        if (-1 == iFrag)
            break;

        fragment& F         = m_Fragment[iFrag];   

        F.m_Position = F.m_OldPosition = StartPos;
        {
            f32 x = x_frand(-1,1);
            f32 z = x_frand(-1,1);
            F.m_Velocity.Set( x,0.5f,z );
        }
        F.m_Velocity.NormalizeAndScale( 5000 );
        F.m_bInUse = TRUE;
        F.m_MeshMask = 1 << x_irand(0,nMeshes-1);
        F.m_RotationAmount = x_frand(0,PI*2);
        

        vector3 Temp = F.m_Velocity;
        Temp.GetY() = 0;
        Temp.Normalize();

        //F.m_RotationAxis = Temp.Cross( vector3(0,1,0) );
        F.m_RotationAxis.Set(1,0,0);
    }

    OnMove( Pos );
}

//=============================================================================

s32 debris_cannon::GetFreeFragment( void )
{
    s32 i;
    for (i=0;i<MAX_FRAGMENTS ;i++)
    {        
        fragment& F = m_Fragment[i];

        if (F.m_bInUse)
            continue;

        F.Reset();
        return i;
    }
    return -1;
}

//=============================================================================

void debris_cannon::ReleaseFragment( s32 iFragment )
{
    ASSERT((iFragment >= 0) && (iFragment < MAX_FRAGMENTS));
    if ((iFragment >= 0) || (iFragment < MAX_FRAGMENTS))
        return;

    m_Fragment[iFragment].m_bInUse = FALSE;
}

//=============================================================================

 bbox debris_cannon::GetLocalBBox        ( void ) const
{
    return m_LocalBBox;
}

//=============================================================================

void debris_cannon::OnAdvanceLogic      ( f32 DeltaTime )
{
    // Handle total time calculations
    m_TotalTime += DeltaTime;
    if (m_TotalTime > kTIME_BEFORE_FADING)
    {
        f32 Delta = m_TotalTime - kTIME_BEFORE_FADING;
        f32 T     = Delta / kFADE_TIME;

        if (T >= 1.0f)
        {
            g_ObjMgr.DestroyObject( GetGuid() );
            return;
        }

        m_Alpha = (u8)(255*(1-T));
    }

    // Handle Sound
    if (m_bFlybyActive)
        UpdateFlyby(DeltaTime);

    // Update individual fragments and process collisions
    UpdateFragments( DeltaTime );
    DoCollisions( DeltaTime );   

    s32 i;
    vector3 Local[ MAX_FRAGMENTS ];
    vector3 MyPos = GetPosition();
    for (i=0;i<MAX_FRAGMENTS;i++)
    {
        fragment& F = m_Fragment[i];

        if (F.m_bInUse)       
        {
            Local[i] = F.m_Position - MyPos;
            if ( F.m_FX.Validate() )
            {
                vector3 Delta = F.m_Position - F.m_OldPosition;
                radian3 Rot(0,0,0);
                Delta.GetPitchYaw(Rot.Pitch,Rot.Yaw);
                F.m_FX.SetRotation( Rot );
                F.m_FX.SetTranslation( F.m_Position );
                F.m_FX.AdvanceLogic( DeltaTime );
            }
        }
        else
            Local[i].Set(0,0,0);
    }
    m_LocalBBox.Clear();
    m_LocalBBox.AddVerts( Local, MAX_FRAGMENTS );
    m_LocalBBox.Inflate( 100,100,100 );

    OnMove( MyPos );
}

//=============================================================================

 void debris_cannon::OnMove				( const vector3& rNewPos )
{
    object::OnMove( rNewPos );
}

//=============================================================================

 void debris_cannon::OnRender            ( void )
{
    s32 i;
    matrix4 L2W;
    for (i=0;i<MAX_FRAGMENTS;i++)
    {
        if (!m_Fragment[i].m_bInUse)
            continue;

        fragment& F = m_Fragment[ i ];

        xcolor C = XCOLOR_WHITE;
        if (F.m_bInactive)
            C = XCOLOR_RED;
        //draw_Label( F.m_Position, XCOLOR_WHITE, "%d",F.m_BounceCount);

        L2W.Setup( vector3(1,1,1), F.m_Orientation, F.m_Position );

        u32 Flags = render::CLIPPED;

        if (m_Alpha < 255)
            Flags |= render::FADING_ALPHA;

        m_RigidInst.SetVMeshMask( (u32)F.m_MeshMask );
        u64 SubMask = m_RigidInst.GetLODMask( L2W );
        m_RigidInst.Render( &L2W, Flags, SubMask, m_Alpha );
    }
}

//=============================================================================

 void debris_cannon::OnRenderTransparent ( void )
 {
     s32 i;
    for (i=0;i<MAX_FRAGMENTS;i++)
    {
        if (!m_Fragment[i].m_bInUse)
            continue;

        fragment& F = m_Fragment[ i ];
        if (F.m_FX.Validate())
        {
            F.m_FX.SetColor( xcolor(255,255,255,m_Alpha) );
            F.m_FX.Render();
        }
    }
    
}

//=============================================================================

void debris_cannon::DoCollisions    ( f32 DeltaTime )
{
    // Do collision tests
    (void)DeltaTime;

    s32     nPlayers = 0;
    vector3 PlayerPos[4];
    player* PlayerPtr[4];
    s32     i;

#ifdef X_EDITOR
    nPlayers = 1;
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if (NULL == pPlayer)
        return;
    PlayerPos[0] = pPlayer->GetPositionWithOffset( actor::OFFSET_EYES );
    PlayerPtr[0] = pPlayer;
#else
    nPlayers = g_NetworkMgr.GetLocalPlayerCount();
    for (i=0;i<nPlayers;i++)
    {
        s32 NetSlot = g_NetworkMgr.GetLocalPlayerSlot( i );
        netobj* pNetObj = NetObjMgr.GetObjFromSlot( NetSlot );
        PlayerPos[i] = pNetObj->GetL2W().GetTranslation();   
        player& Player = player::GetSafeType( *pNetObj );
        PlayerPtr[i] = &Player;
    }
#endif
    
    for (i=0;i<MAX_FRAGMENTS;i++)
    {
        if (m_Fragment[i].m_bInUse && !m_Fragment[i].m_bInactive)
        {
            fragment& F = m_Fragment[i];

            vector3 Start = F.m_OldPosition;
            vector3 End   = F.m_Position;

            //g_CollisionMgr.SphereSetup( GetGuid(), Start,End, 5 );
            g_CollisionMgr.RaySetup( GetGuid(), Start,End );
            g_CollisionMgr.SetMaxCollisions(1);
            //g_CollisionMgr.UseLowPoly();
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_BLOCKS_SMALL_DEBRIS,
                                            (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING));                                            

            if (g_CollisionMgr.m_nCollisions > 0)
            {
                // Back it up in case a descendant class wants to do collision tests
                // of it's own.
                collision_mgr::collision Col = g_CollisionMgr.m_Collisions[0];
                OnFragmentCollide( i, Col );
            }        

            // Check to see if this was a flyby
            if (!m_bFlybyPerformed)
            {
                if (F.m_Velocity.LengthSquared() > 800*800)
                {
                    // Test for flyby condition
                    s32 j;
                    for (j=0;j<nPlayers;j++)
                    {
                        vector3 Delta = F.m_Position - PlayerPos[j];
                        f32     DistSq = Delta.LengthSquared();

                        if (DistSq < 100*100)
                        {
                            // We have a flyby!
                            SetupFlyby( i, PlayerPos[j], PlayerPtr[i] );
                        }
                    }
                }
            }
        }
    }
}

//=============================================================================

void debris_cannon::UpdateFragments     ( f32 DeltaTime )
{
    s32 i;

    for (i=0;i<MAX_FRAGMENTS;i++)
    {
        fragment& F = m_Fragment[i];
        if (!F.m_bInUse || F.m_bInactive)
            continue;

        F.m_OldPosition = F.m_Position;

        F.m_Velocity *= 1.0f - (0.05f * DeltaTime);
        F.m_Velocity += kDEBRIS_GRAVITY * DeltaTime;        
        vector3 Delta = F.m_Velocity * DeltaTime;
        F.m_Position = F.m_OldPosition + Delta;

        f32 Scale = F.m_Velocity.Length();
        f32 SpeedForMaxSpin = 2000.0f;
        f32 RotationSpeed = MAX(0,MIN(1,Scale / SpeedForMaxSpin));
        //RotationSpeed *= RotationSpeed;

        // Bounce degradation is maxed out after 3 bounces
        //  Limit the amount of degratation so that we don't 
        //  completely stop the fragment rotation with this.
        //  Put it in the range [0.1,1]
        f32 BounceDegradation = MAX(0,MIN(1,(F.m_BounceCount / 3.0f)));
        BounceDegradation = 1.0f - BounceDegradation;
        BounceDegradation = BounceDegradation * 0.9f + 0.1f;
        BounceDegradation = 1.0f;

        F.m_RotationAmount = R_360 * 4 * DeltaTime * RotationSpeed * BounceDegradation;

        quaternion Q( F.m_RotationAxis, F.m_RotationAmount );
        F.m_Orientation *= Q;
        F.m_Orientation.Normalize();
    }
}

//=============================================================================

void debris_cannon::OnFragmentCollide   ( s32 iFragment, collision_mgr::collision& Col )
{
    fragment& F = m_Fragment[ iFragment ];
    vector3 DeltaPos = F.m_Position - F.m_OldPosition;

    // Get speed and normalized direction
    f32     Speed = DeltaPos.Length();
    
    if (Speed < 0.1f)
    {
        F.m_Position = F.m_OldPosition;
        return;
    }

    vector3 Dir   = DeltaPos / Speed;

    // Process the collision
    f32 DistanceToMove = Speed * Col.T - 0.5f;
    if (DistanceToMove < 0)
        DistanceToMove = 0;

    vector3 NewPos = F.m_OldPosition + DistanceToMove*Dir;

    F.m_Position = NewPos;

    //  if the velocity squared is less than slow velocity squared (100*100 = 10,000)
    const f32 minVelocityToWorryAbout = 100.0f;
    if( F.m_Velocity.LengthSquared() < (minVelocityToWorryAbout * minVelocityToWorryAbout) )
    {
        F.m_bInactive = TRUE;
        F.m_Velocity.Zero();
        F.m_Position = F.m_OldPosition;
        if (F.m_bSuspendFXOnRest && F.m_FX.Validate())
        {
            F.m_FX.SetSuspended(TRUE);
        }
        return;
    }

    // Register a bounce for audio purposes
    OnBounce( iFragment );

    // Reflect velocity for bounce
    F.m_Velocity = Col.Plane.ReflectVector( F.m_Velocity );
    vector3 Vel = F.m_Velocity;
    Vel.Normalize();
    f32 Damp = x_abs( Vel.Dot(Col.Plane.Normal) );
    Damp *= 0.65f;
    Damp += 0.3f;
    Damp = 1.0f - Damp;
    vector3 Dampening(Damp,Damp,Damp);

    F.m_Velocity = F.m_Velocity * Dampening;
    
    static radian BounceVelocityChange = R_30;
    f32 FinalBounceVelocityChange = BounceVelocityChange; 
    {
        f32 x = x_frand( -FinalBounceVelocityChange, FinalBounceVelocityChange );
        f32 y = x_frand( -FinalBounceVelocityChange, FinalBounceVelocityChange );
        F.m_Velocity.Rotate(  radian3( x, y, 0.0f ) );
    }

    //static radian BounceSpinRate = R_60;
    //m_Spin = radian3(x_frand(-BounceSpinRate,BounceSpinRate), x_frand(-BounceSpinRate,BounceSpinRate),x_frand(-BounceSpinRate,BounceSpinRate));
    
}

//=============================================================================

void debris_cannon::PlaySound           ( s32 iFragment, sound iSound, f32 Volume )
{
    fragment& F = m_Fragment[ iFragment ];
    
    switch( iSound )
    {
    case SOUND_IMPACT:
        {
            voice_id VoiceID = g_AudioMgr.PlayVolumeClipped( "Explosion_Debris_Impact", F.m_Position, GetZone1(), TRUE );
            g_AudioMgr.SetVolume( VoiceID, Volume );
        }
        break;
    case SOUND_KICKOFF:
        //g_AudioMgr.Play( "Grenade_Explosion", GetL2W().GetTranslation(), TRUE, TRUE );
        break;
    }    
}

//=============================================================================
static s32 iSecondayBounce = 3;
void debris_cannon::OnBounce( s32 iFragment )
{
    fragment& F = m_Fragment[ iFragment ];

    xbool bPlay = TRUE;
    f32   Volume = 0.5f;

    if (F.m_BounceCount > 0)
        bPlay = FALSE;

    if (F.m_BounceCount == iSecondayBounce)
    {
        if (x_frand(0,1) < 0.6f)
        {
            bPlay  = TRUE;        
            Volume = 0.5f;
        }
    }

    if (bPlay)
    {
        PlaySound( iFragment, SOUND_IMPACT, Volume );
    }

    // Rebuild rotation axis
    vector3 Temp = F.m_Velocity;
    Temp.GetY() = 0;
    Temp.Normalize();
    F.m_RotationAxis = Temp.Cross( vector3(0,1,0) );


    F.m_BounceCount++;
}

//=============================================================================

void debris_cannon::SetupFlyby( s32 iFragment, const vector3& PlayerPos, player* pPlayer )
{
    (void)pPlayer;

    fragment& F = m_Fragment[ iFragment ];
    vector3 DeltaPos = F.m_Position - F.m_OldPosition;
    DeltaPos.GetY() = PlayerPos.GetY();
    DeltaPos.Normalize();

    vector3 ClosestPoint = PlayerPos.GetClosestPToLSeg( F.m_OldPosition, F.m_OldPosition + (DeltaPos * 5000));

    m_FlybyStart        = ClosestPoint - (DeltaPos * 400);
    m_FlybyEnd          = ClosestPoint + (DeltaPos * 1200);
    m_FlybyVoiceID      = g_AudioMgr.PlayVolumeClipped( "Explosion_Debris_FlyBy", m_FlybyStart, GetZone1(), TRUE );
    m_FlybyTimer        = 0;
    m_FlybyLength       = g_AudioMgr.GetLengthSeconds( m_FlybyVoiceID );
    m_bFlybyActive      = TRUE;

    // Safety check
    if (m_FlybyLength == 0)
        m_bFlybyActive = FALSE;

    if (m_FlybyVoiceID == -1)
        m_bFlybyActive = FALSE;

    m_bFlybyPerformed = TRUE;    
}

//=============================================================================

void debris_cannon::UpdateFlyby( f32 DeltaTime )
{
    m_FlybyTimer += DeltaTime;
    if (m_FlybyTimer > m_FlybyLength)
    {
        m_bFlybyActive = FALSE;
        g_AudioMgr.Release( m_FlybyVoiceID, 0 );
        return;
    }
    f32 T = m_FlybyTimer / m_FlybyLength;

    vector3 Delta = m_FlybyEnd - m_FlybyStart;
    Delta *= T;

    g_AudioMgr.SetPosition( m_FlybyVoiceID, m_FlybyStart + Delta, GetZone1() );
}

//=============================================================================

void debris_cannon::InitializeFragmentsForPlayerDirectedExplosion  ( const vector3& Pos,
                                                                     const vector3& Dir,
                                                                     s32            nFragments,
                                                                     f32            MinSpeed,
                                                                     f32            MaxSpeed,
                                                                     f32            FaceShotPercentage,
                                                                     f32            SlowTrailerThreshold,
                                                                     f32            ABPercentage,
                                                                     xbool          bTypeASuspendOnRest,
                                                                     s32            MaxTypeA,
                                                                     const char*    TypeAFXName,
                                                                     xbool          bTypeBSuspendOnRest,
                                                                     s32            MaxTypeB,
                                                                     const char*    TypeBFXName,
                                                                     f32            FastTrailerThreshold,
                                                                     f32            FastPercentage,
                                                                     xbool          bFastSuspendOnRest,
                                                                     s32            MaxFastTrailer,
                                                                     const char*    FastFXName )
{
    // Clear existing fragments
    s32 i;
    for (i=0;i<MAX_FRAGMENTS;i++)
    {
        m_Fragment[i].Reset();
    }
    m_TotalTime = 0;

    //==-------------------------------
    //  GATHER PLAYER INFO
    //==-------------------------------
    s32     nPlayers = 0;
    matrix4 PlayerOrient[4];  
    f32     DistToExplosion = 1e30f;

#ifdef X_EDITOR
    nPlayers = 1;
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if (NULL == pPlayer)
        return;
    vector3 PlayerPos = pPlayer->GetPositionWithOffset( actor::OFFSET_EYES );   GetL2W().GetTranslation();
    vector3 Delta = PlayerPos - Pos;
    f32 Dist = Delta.Length();
    DistToExplosion = Dist;
    Delta.Normalize();
    radian3 Rot;
    Delta.GetPitchYaw( Rot.Pitch,Rot.Yaw );
    Rot.Roll = 0;

    PlayerOrient[0].Identity();
    PlayerOrient[0].SetRotation( Rot );

    // Figure out how far away we are
    f32 DistT = Dist / 1600.0f;
    DistT = 1.0f - MIN(1,MAX(0,DistT));

    // Shake the camera
    f32 s_HitShakeTime     = 0.8f;
    f32 s_HitShakeAmount   = 1.0f;
    f32 s_HitShakeSpeed    = 1.5f;
    pPlayer->ShakeView( s_HitShakeTime, s_HitShakeAmount * DistT, s_HitShakeSpeed );
    pPlayer->DoFeedback( (s_HitShakeTime/2.0f), s_HitShakeAmount * DistT );

#else
    nPlayers = g_NetworkMgr.GetLocalPlayerCount();
    for( i=0; i<nPlayers; i++ )
    {
        PlayerOrient[i].Identity();

        s32 NetSlot = g_NetworkMgr.GetLocalPlayerSlot( i );
        netobj* pNetObj = NetObjMgr.GetObjFromSlot( NetSlot );
        if( !pNetObj )
        {
            continue;
        }
        player& Player = player::GetSafeType( *pNetObj );
        vector3 PlayerPos = pNetObj->GetL2W().GetTranslation();
        vector3 Delta = PlayerPos - Pos;
        f32 Dist = Delta.Length();
        DistToExplosion = MIN(DistToExplosion,Dist);
        Delta.Normalize();

        radian3 Rot;
        Delta.GetPitchYaw( Rot.Pitch,Rot.Yaw );
        Rot.Roll = 0;

        PlayerOrient[i].SetRotation( Rot );

        // Figure out how far away we are
        f32 DistT = Dist / 2400.0f;
        DistT = 1.0f - MIN(1,MAX(0,DistT));

        // Shake the camera
        if (DistT > 0.1f)
        {
            f32 s_HitShakeTime     = 0.8f;
            f32 s_HitShakeAmount   = 1.0f;
            f32 s_HitShakeSpeed    = 1.5f;
            Player.ShakeView( s_HitShakeTime, s_HitShakeAmount * DistT, s_HitShakeSpeed );
            Player.DoFeedback((s_HitShakeTime/2.0f), s_HitShakeAmount * DistT);
        }
    }
#endif
    vector3 nDir = Dir;
    nDir.NormalizeAndScale(6);

    matrix4 AlignWithNormal;
    radian3 NormalRot(0,0,0);
    Dir.GetPitchYaw( NormalRot.Pitch, NormalRot.Yaw );
    AlignWithNormal.Identity();
    AlignWithNormal.SetRotation( NormalRot );

    vector3 StartPos = Pos + nDir;

    geom* pGeom = m_RigidInst.GetGeom();
    s32 nMeshes = MIN(32,pGeom->m_nVirtualMeshes);

    m_LocalBBox.Set( Pos, 50 );

    //==-------------------------------
    //  CREATE FRAGMENTS
    //==------------------------------- 
    s32 nFastSmoke = 0;
    s32 nSlowFire  = 0;
    s32 nSlowSmoke = 0;

    for (i=0;i<nFragments ;i++)
    {   
        s32 iFrag = GetFreeFragment();
        if (-1 == iFrag)
            break;

        fragment&   F               = m_Fragment[iFrag];   
        f32         Speed           = x_frand(MinSpeed,MaxSpeed);
        xbool       bTowardPlayer   = FALSE;
        xbool       bAimUp          = FALSE;

        F.m_Position = F.m_OldPosition = StartPos;        
        F.m_bInUse = TRUE;
        F.m_MeshMask = 1 << x_irand(0,nMeshes-1);
        F.m_RotationAmount = x_frand(0,PI*2);

        if (x_frand(0,1) < FaceShotPercentage)
            bTowardPlayer = TRUE;

        rhandle<char> FxResource;
        f32 Scale = 1.0f;

        if (Speed < SlowTrailerThreshold)
        {
            // If the fragment is slow enough, give it a trailer
            if ((x_frand(0,1) > ABPercentage))
            {
                if (nSlowFire < MaxTypeA)
                {
                    FxResource.SetName( TypeAFXName );
                    F.m_bSuspendFXOnRest = bTypeASuspendOnRest;
                    Scale = 0.5f;
                    nSlowFire++;
                }
            }
            else
            {
                if (nSlowSmoke < MaxTypeB)
                {
                    FxResource.SetName( TypeBFXName );
                    Scale = 0.5f;
                    F.m_bSuspendFXOnRest = bTypeBSuspendOnRest;
                    bAimUp = TRUE;
                    nSlowSmoke++;

                    // For the smoke trailers, we don't always want
                    // to throw them at the player
                    if (x_frand(0,1) < 0.4f)
                        bTowardPlayer = FALSE;
                }
            }
        }
        else if (Speed > FastTrailerThreshold)
        {
            if ((x_frand(0,1) < FastPercentage)&& (nFastSmoke < MaxFastTrailer))
            {
                FxResource.SetName( FastFXName );
                Scale = 0.5f;
                F.m_bSuspendFXOnRest = bFastSuspendOnRest;
                bAimUp = TRUE; 
                nFastSmoke++;
            }            
        }

        const char* pScrewedUpName = FxResource.GetPointer();
        if( pScrewedUpName )
        {
            if ( s_DebrisCannonActiveFXHandles < k_MAX_DEBRIS_CANNON_ACTIVE_FX_HANDLES )
            {
                if (F.m_FX.InitInstance( pScrewedUpName ))
                {
                    F.m_FX.SetSuspended( FALSE );  
                    F.m_FX.SetTranslation( F.m_Position );
                    F.m_FX.SetScale( vector3(Scale,Scale,Scale) );
                    s_DebrisCannonActiveFXHandles++;
                }
            }
        }

        if (!bTowardPlayer)
        {
            // Direct randomly
            f32 x = x_frand(-1,1);
            f32 y = x_frand(-1,1);
            F.m_Velocity.Set( x,y,2.0f);            
            AlignWithNormal.Transform( &F.m_Velocity, &F.m_Velocity, 1 );
        }
        else
        {
            // Direct toward a player
            s32 iPlayer = x_irand(0,nPlayers-1);
            vector3 Temp(0,0,1);
            radian Ang = R_10;
            radian X = x_frand(-Ang*2,0);
            radian Y = x_frand(-Ang,Ang);
            radian Min = R_2;

            if (X<0 && X>-Min)
                X = -Min;
            else if (X>0 && X<Min)
                X = Min;

            if (Y<0 && Y>-Min)
                Y = -Min;
            else if (Y>0 && Y<Min)
                Y = Min;

            if (X>0)
                X *= -1;

            Temp.RotateX(X);
            Temp.RotateY(Y);
            PlayerOrient[iPlayer].Transform( &Temp, &Temp, 1 );
            F.m_Velocity = Temp;
        }        

        if (bAimUp)
            F.m_Velocity.GetY() += 1.0f;

        vector3 Temp = F.m_Velocity;
        Temp.GetY() = 0;
        Temp.Normalize();

        F.m_RotationAxis = Temp.Cross( vector3(0,1,0) );

        F.m_Velocity.NormalizeAndScale( Speed );

    }
}

//=============================================================================

void debris_cannon::CauseShellshock ( f32 MinDist, f32 MaxDist )
{
    matrix4     L2W    [4];
    player*     pPlayer[4];

    s32 nPlayers = SMP_UTIL_GetInfoAllPlayers( 2, L2W, pPlayer );

    s32 i;
    f32 MostIntenseShellshock = 0;

    vector3 MyPos = GetPosition();

    for (i=0;i<nPlayers;i++)
    {
        vector3 Delta = (L2W[i].GetTranslation()) - MyPos;
        f32     Dist  = Delta.Length();
        f32     DistT = (Dist - MinDist) / (MaxDist - MinDist);

        MostIntenseShellshock = MAX( MostIntenseShellshock, (MIN(1,MAX(0,DistT))) );;
    }

    /*
    if (MostIntenseShellshock > 0)
        g_PerceptionMgr.BeginShellShock( MostIntenseShellshock );
    */
}

//=============================================================================

//=============================================================================

//=============================================================================

//=============================================================================