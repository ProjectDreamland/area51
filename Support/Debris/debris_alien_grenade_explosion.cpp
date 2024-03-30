#include "debris_alien_grenade_explosion.hpp"
#include "NetworkMgr\Networkmgr.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "..\Objects\Player.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Tracers\TracerMgr.hpp"

//=============================================================================
//  CONSTANTS
//=============================================================================

static  f32         kCLAMP_DISTANCE                 = 300.0f;
static  f32         kEXPAND_SPEED                   = 1200.0f;
static  f32         kMAX_COLLAPSE_DEVIATION         = 0.5f;
static  f32         kAGITATION_AMT                  = 0.0f;
static  f32         kEXPAND_TRACER_BIAS             = 0.5f;
static  f32         kCOLLAPSE_TRACER_BIAS           = 0.5f;


tweak_handle s_JBEAN_ExpandTime         ("JBEAN_Explosion_Expand_Time");
tweak_handle s_JBEAN_HoldTime           ("JBEAN_Explosion_Hold_Time");
tweak_handle s_JBEAN_CollapseTime       ("JBEAN_Explosion_Collapse_Time");

static const char*  s_pCoreFXNames[] = { PRELOAD_FILE("alien_grenade_Expl_1st.fxo"),                                        
                                         "",
                                         PRELOAD_FILE("alien_grenade_vacuum.fxo"),
                                         PRELOAD_FILE("alien_grenade_Expl_2nd.fxo") };

static const char*  s_pSoundFXNames[] = { "Explosion_JB_Initial",
                                          "",
                                          "Explosion_JB_Implosion",
                                          "Explosion_JB_Final" };

//=============================================================================
// OBJECT DESC.
//=============================================================================
static struct debris_alien_grenade_explosion_desc : public object_desc
{
    debris_alien_grenade_explosion_desc( void ) : object_desc( 
        object::TYPE_DEBRIS_ALIEN_GRENADE_EXPLOSION, 
        "AlienGrenadeExplosionsDebris", 
        "EFFECTS",
        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_RENDERABLE         | 
        object::ATTR_TRANSPARENT        |
        object::ATTR_NO_RUNTIME_SAVE,
        FLAGS_IS_DYNAMIC  )
    {}

    virtual object* Create          ( void )
    {
        return new debris_alien_grenade_explosion;
    }

} s_debris_alien_grenade_explosion_desc;

//=============================================================================================

const object_desc&  debris_alien_grenade_explosion::GetTypeDesc ( void ) const
{
    return s_debris_alien_grenade_explosion_desc;
}

//=============================================================================================

const object_desc&  debris_alien_grenade_explosion::GetObjectType ( void )
{
    return s_debris_alien_grenade_explosion_desc;
}

//=============================================================================================

debris_alien_grenade_explosion::debris_alien_grenade_explosion()
{
}

//=============================================================================================

debris_alien_grenade_explosion::~debris_alien_grenade_explosion()
{
}

//=============================================================================================

void debris_alien_grenade_explosion::Create   ( const char*      pMeshName,
                                                const vector3&   aPos,
                                                u32              Zones,
                                                const vector3&   Dir,
                                                s32              nFragments,
                                                guid             OriginGuid )
{
    m_RigidInst.SetUpRigidGeom( pMeshName );
    rigid_geom* pGeom = m_RigidInst.GetRigidGeom();
    if (NULL == pGeom)
        return;    

    vector3 Pos = Dir;
    Pos.NormalizeAndScale( 100.0f );
    Pos += aPos;

    SetZones( Zones );

    m_Mode              = MODE_EXPAND;
    m_ClampDistance     = kCLAMP_DISTANCE;
    m_CurrentDistance   = 0.0f;
    m_Speed             = kEXPAND_SPEED;
    m_Origin            = Pos;
    m_ModeTimer         = 0;
    m_OriginGuid        = OriginGuid;

    s32     i;   

    vector3 nDir = Dir;
    nDir.NormalizeAndScale(6);

    m_HitPlaneNormal = nDir;

    vector3 StartPos = Pos + nDir;

    s32 nMeshes = MIN(32,pGeom->m_nMeshes);

    OnMove( StartPos );
    m_LocalBBox.Set(vector3(0,0,0), 50 );

    rhandle<char> FxResource;    

    for (i=0;i<4;i++)
    {
        FxResource.SetName( s_pCoreFXNames[i] );
        const char* pScrewedUpName = FxResource.GetPointer();

        if( pScrewedUpName )
        {
            m_CoreFX[i].InitInstance( pScrewedUpName );
            m_CoreFX[i].SetSuspended( TRUE );  
            m_CoreFX[i].SetTranslation( Pos ); 
            m_CoreFX[i].SetScale(vector3(1,1,1));            
        }
    }
   
    m_CoreSize = 0.0f;
    
    nFragments = MIN(nFragments,MAX_FRAGMENTS);
    for (i=0;i<nFragments ;i++)
    {   
        s32 iFrag = GetFreeFragment();
        if (-1 == iFrag)
            break;

        fragment&       F       = m_Fragment  [ iFrag ];   
        fragment_ex&    FEX     = m_FragmentEx[ iFrag ];
        
        F.m_Position = F.m_OldPosition = StartPos;        
        F.m_bInUse = TRUE;
        F.m_MeshMask = 1 << x_irand(0,nMeshes-1);
        F.m_RotationAmount = x_frand(0,PI*2);
        FEX.m_TracerID = NULL_TRACER_ID;

        if ((i % 2) == 0)
        {
            FEX.m_TracerID = g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_ENERGY_SWOOSH, 
                                                    s_JBEAN_ExpandTime.GetF32() + s_JBEAN_HoldTime.GetF32() + kEXPAND_TRACER_BIAS, 
                                                    StartPos,
                                                    StartPos,
                                                    xcolor(60,200,250) );
        }

        /* // Turning off the trailers for now.  Going to try it with the tracer system
        FxResource.SetName( PRE_LOAD_FILE("DEB_Alien_World_002.fxo") );
        const char* pScrewedUpName = FxResource.GetPointer();
        if( pScrewedUpName )
        {
            f32 Scale = 0.5f;
            F.m_FX.InitInstance( pScrewedUpName );
            F.m_FX.SetSuspended( FALSE );  
            F.m_FX.SetTranslation( F.m_Position );
            F.m_FX.SetScale( vector3(Scale,Scale,Scale) );
        }
      */
        xbool bValid = TRUE;
        s32 Tries = 0;
        // Direct randomly
        do 
        {                   
            bValid = TRUE;
            f32 x = x_frand(-1,1);
            f32 y = x_frand(-1,1);
            f32 z = x_frand(-1,1);
            F.m_Velocity.Set(x,y,z);
            F.m_Velocity.Normalize();

            g_CollisionMgr.LineOfSightSetup( GetGuid(), StartPos, StartPos + F.m_Velocity * m_ClampDistance );
            g_CollisionMgr.SetMaxCollisions(1);            
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_BLOCKS_SMALL_DEBRIS,
                                            (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING));
            if (g_CollisionMgr.m_nCollisions != 0)
            {
                if (g_CollisionMgr.m_Collisions[0].T < 0.8f)
                {                    
                    bValid = FALSE;
                    //Tries++;
                }
            }
        }
        while ((!bValid) && (Tries < 5));

        vector3 Temp = F.m_Velocity;
        Temp.GetY() = 0;
        Temp.Normalize();        

        F.m_RotationAxis = Temp.Cross( vector3(0,1,0) ); 
        f32 x = x_frand(-1,1);
        f32 y = x_frand(-1,1);
        f32 z = x_frand(-1,1);
        F.m_RotationAxis.Set( x,y,z );
        F.m_RotationAxis.Normalize( );

        FEX.m_CollapseTimer = x_frand(0,kMAX_COLLAPSE_DEVIATION);        
        FEX.m_MaxDist = m_ClampDistance;

        quaternion Q( F.m_RotationAxis, x_frand(0,2*PI) );
        F.m_Orientation *= Q;
        F.m_Orientation.Normalize();
    }

    m_iCollisionSetupIndex = 0;

/*
    particle_emitter::CreatePresetParticleAndOrient( 
        particle_emitter::GRAV_GRENADE_EXPLOSION, 
        vector3(0.0f,0.0f,0.0f), 
        Pos );
        */

    g_AudioMgr.PlayVolumeClipped( s_pSoundFXNames[ MODE_EXPAND ], m_Origin, GetZone1(), TRUE );

    if (m_CoreFX[ MODE_EXPAND ].Validate())
    {
        m_CoreFX[ MODE_EXPAND ].SetSuspended( FALSE );
    }
}

//=============================================================================================

void debris_alien_grenade_explosion::OnAdvanceLogic( f32 DeltaTime )
{
    s32 i;

    for (i=0;i<4;i++)
    {
        if (m_CoreFX[i].Validate())
        {
            if (!m_CoreFX[i].IsFinished())
            {
                m_CoreFX[i].AdvanceLogic( DeltaTime );
            }
        }
    }

    if (m_Mode == MODE_EXPLODE)
    {
        debris_cannon::OnAdvanceLogic( DeltaTime );
        return;
    }

    m_ModeTimer += DeltaTime;

    if (m_iCollisionSetupIndex >= 0)
    {
        // Do some collision setups        
        s32 i;
        for (i=0;i<10;i++)
        {   
            if (m_iCollisionSetupIndex >= MAX_FRAGMENTS)
            {
                m_iCollisionSetupIndex = -1;
                break;
            }

            fragment&    F   = m_Fragment  [ m_iCollisionSetupIndex ];
            fragment_ex& FEX = m_FragmentEx[ m_iCollisionSetupIndex ];

            // Do the collision test for this fragment
            vector3 Dest = F.m_Velocity;
            Dest *= m_ClampDistance;
            Dest += m_Origin;

            g_CollisionMgr.RaySetup( GetGuid(), m_Origin, Dest );
            g_CollisionMgr.SetMaxCollisions(1);            
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                                            object::ATTR_BLOCKS_SMALL_DEBRIS,
                                            (object::object_attr)(object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING));                                            
            
            if (g_CollisionMgr.m_nCollisions > 0)
            {
                FEX.m_MaxDist = m_ClampDistance * g_CollisionMgr.m_Collisions[0].T;
            }

            m_iCollisionSetupIndex++;
        }
    }

    switch( m_Mode )
    {
        case MODE_EXPAND:
            {
                f32 T = m_TotalTime / s_JBEAN_ExpandTime.GetF32();

                m_SpinRateController = 1.0f;

                if (T >= 1.0f)
                {
                    m_Mode = MODE_HOLD;
                    m_CurrentDistance = m_ClampDistance;
                    m_CoreSize = 1.0f;
                    m_ModeTimer = 0;
                    
                    for (i=0;i<MAX_FRAGMENTS;i++)
                    {
                        fragment&    F   = m_Fragment  [i];
                        if (!F.m_bInUse || F.m_bInactive)
                            continue;
                        if (F.m_FX.Validate())
                            F.m_FX.SetSuspended(TRUE);
                    }
                    if (m_CoreFX[ MODE_HOLD ].Validate())
                    {
                        m_CoreFX[ MODE_HOLD ].Restart();
                        m_CoreFX[ MODE_HOLD ].SetSuspended( FALSE );
                    }

                }
                else
                {
                    m_CurrentDistance = x_sin( T*PI/2 );
                    m_CurrentDistance *= m_ClampDistance;
                    m_CoreSize = T;                   
                }
            }
        break;
        case MODE_HOLD:
            {
                f32 T = m_ModeTimer / s_JBEAN_HoldTime.GetF32();
                // Use the comfy part of the sin curve between PI/2 and 3PI/2 
                // to get nice acceleration and deceleration
                f32 MinSpinRate = 0.2f;
                m_SpinRateController = (x_sin( (PI/2) + (T*PI) ) + 1.0f) / 2.0f;
                m_SpinRateController = MAX(0,(m_SpinRateController - MinSpinRate) / (1.0f - MinSpinRate))+MinSpinRate;

                if (m_ModeTimer > s_JBEAN_HoldTime.GetF32())
                {
                    m_Mode = MODE_COLLAPSE;
                    m_ModeTimer = 0;

                    g_AudioMgr.PlayVolumeClipped( s_pSoundFXNames[ MODE_COLLAPSE ], m_Origin, GetZone1(), TRUE );
                    
                    for (i=0;i<MAX_FRAGMENTS;i++)
                    {
                        fragment&    F   = m_Fragment  [i];
                        if (!F.m_bInUse || F.m_bInactive)
                            continue;
                        fragment_ex& FEX = m_FragmentEx[i];

                        FEX.m_TracerID = g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_ENERGY_SWOOSH, 
                                                                s_JBEAN_CollapseTime.GetF32() + kCOLLAPSE_TRACER_BIAS,
                                                                F.m_Position,
                                                                F.m_Position,
                                                                xcolor(60,200,250) );
                    }  

                    if (m_CoreFX[ MODE_COLLAPSE ].Validate())
                    {
                        m_CoreFX[ MODE_COLLAPSE ].Restart();
                        m_CoreFX[ MODE_COLLAPSE ].SetSuspended( FALSE );
                    }

                }
            }
            break;
        case MODE_COLLAPSE:
            {
//                static f32 CollapseTime = 1.5f;
                static f32 CoreCollapseStart = 0.0f;
                static f32 CoreCollapseTime = 1.0f;
                m_CoreSize = 1.0f;
                if (m_ModeTimer > CoreCollapseStart)
                {
                    f32 T = (m_ModeTimer - CoreCollapseStart) / (CoreCollapseTime - CoreCollapseStart);
                    T = 1.0f - (MAX(0,MIN(1,T)));
                    m_CoreSize = T;
                }

                if ( m_ModeTimer > kMAX_COLLAPSE_DEVIATION + s_JBEAN_CollapseTime.GetF32() )
                {
                    m_Mode = MODE_EXPLODE;
                    m_ModeTimer = 0;

                    //==-------------------------------
                    //  Reinitialize fragments
                    //==-------------------------------
                    InitializeFragmentsForPlayerDirectedExplosion( m_Origin,                                    //Pos,
                                                                   m_HitPlaneNormal,                            //Dir,
                                                                   10,                                          //nFragments,
                                                                   1000,                                        //MinSpeed,
                                                                   5000,                                        //MaxSpeed,
                                                                   0.7f,                                        //FaceShotPercentage,
                                                                   2000,                                        //SlowTrailerThreshold,
                                                                   0.4f,                                        //ABPercentage,
                                                                   TRUE,                                        //bTypeASuspendOnRest,
                                                                   3,                                           //MaxTypeA,
                                                                   PRELOAD_FILE("DEB_fire_world_001.fxo"),      //TypeAFXName,
                                                                   TRUE,                                        //bTypeBSuspendOnRest,
                                                                   6,                                           //MaxTypeB,
                                                                   PRELOAD_FILE("DEB_smoketrail.fxo"),          //TypeBFXName,
                                                                   3000,                                        //FastTrailerThreshold,
                                                                   0.7f,                                        //FastPercentage,
                                                                   TRUE,                                        //bFastSuspendOnRest,
                                                                   6,                                           //MaxFastTrailer,
                                                                   PRELOAD_FILE("DEB_smoketrail_001.fxo"));     //FastFXName );

                    /*
                    //==-------------------------------
                    //  CREATE PARTICLE FX
                    //==-------------------------------
                    particle_emitter::CreatePresetParticleAndOrient( particle_emitter::JUMPING_BEAN_EXPLOSION, 
                                                                     vector3(0.0f,0.0f,0.0f), 
                                                                     m_Origin );
                    */

                    //==-------------------------------
                    //  KICK OFF AUDIO
                    //==-------------------------------
                    voice_id VoiceID = g_AudioMgr.PlayVolumeClipped( "MSN_Projectile_Explode", m_Origin, GetZone1(), TRUE );
                    g_AudioManager.NewAudioAlert( VoiceID, audio_manager::EXPLOSION, m_Origin, GetZone1(), GetGuid() );

                    //==-------------------------------
                    // <spock>PAIN!</spock>
                    //==-------------------------------
                    pain        Pain; 
                    pain_handle PainHandle("ALIEN_GRENADE_EXPLOSION_SECONDARY");                    
                    Pain.Setup( PainHandle, m_OriginGuid, m_Origin );
                    Pain.ApplyToWorld();

                    //==-------------------------------
                    //  Smack the player in the noggin
                    //==-------------------------------
                    CauseShellshock( 400, 1000.0f );

                    //==-------------------------------
                    //  Kick off FX
                    //==-------------------------------
                    if (m_CoreFX[ MODE_EXPLODE ].Validate())
                    {
                        m_CoreFX[ MODE_EXPLODE ].Restart();                        
                        m_CoreFX[ MODE_EXPLODE ].SetSuspended( FALSE );
                    }
                }
            }
            break;
        case MODE_EXPLODE:
            {                
                // Should never get here
                ASSERT(FALSE);
            }
            break;
    }


    debris_cannon::OnAdvanceLogic( DeltaTime );
}

//=============================================================================================

void debris_alien_grenade_explosion::UpdateFragments     ( f32 DeltaTime )
{
    if (m_Mode == MODE_EXPLODE)
    {
        debris_cannon::UpdateFragments( DeltaTime );
        return;
    }

    s32 i;

    f32 TimeForMode = s_JBEAN_ExpandTime.GetF32();
    if (m_Mode == MODE_HOLD)
        TimeForMode = s_JBEAN_HoldTime.GetF32();
    else if (m_Mode == MODE_COLLAPSE)
        TimeForMode = s_JBEAN_CollapseTime.GetF32();

    for (i=0;i<MAX_FRAGMENTS;i++)
    {
        fragment&    F   = m_Fragment  [i];
        fragment_ex& FEX = m_FragmentEx[i];
        
        if (!F.m_bInUse || F.m_bInactive)
            continue;

        switch(m_Mode)
        {
        case MODE_EXPAND:
            {
                F.m_OldPosition = F.m_Position;
                f32 Dist = MIN(m_CurrentDistance, FEX.m_MaxDist);
                
                vector3 Delta = F.m_Velocity * Dist;
                F.m_Position = m_Origin + Delta;

                if (FEX.m_TracerID != NULL_TRACER_ID)
                {
                    g_TracerMgr.UpdateTracer( FEX.m_TracerID, NULL, &F.m_Position, NULL );
                }
            }
            break;
        case MODE_HOLD:
            {
                /*
                f32 T = m_ModeTimer / s_JBEAN_HoldTime.GetF32();
                T = 1.0f - T;

                f32 Agitation = 1.0f - ((x_cos(T * 2 * PI) + 1.0f) / 2.0f);
                f32 Ofs = 10.0f * Agitation;
                vector3 Delta( x_frand(-Ofs,Ofs), x_frand(-Ofs,Ofs), x_frand(-Ofs,Ofs) );
                F.m_Position = m_Origin + F.m_Velocity * FEX.m_MaxDist + Delta;
                */
            }
            break;
        case MODE_COLLAPSE:
            {
                f32 T = (m_ModeTimer - FEX.m_CollapseTimer) / s_JBEAN_CollapseTime.GetF32();                
                T = MAX(0,MIN(1,T));
                T = 1.0f - (T*T*T);

                f32 CollapsedDist = m_ClampDistance * T;
                F.m_OldPosition = F.m_Position;

                CollapsedDist = MIN(CollapsedDist,FEX.m_MaxDist);
                vector3 Delta = F.m_Velocity * CollapsedDist;
                F.m_Position = m_Origin + Delta;

                if (FEX.m_TracerID != NULL_TRACER_ID)
                {
                    g_TracerMgr.UpdateTracer( FEX.m_TracerID, NULL, &F.m_Position, NULL );
                }

                if (T < 1.0f)
                {
                    if (F.m_FX.Validate())
                        F.m_FX.SetSuspended(FALSE);
                }
            }
            break;
        }

        // Handle agitation
        {
            f32 TStartOfCollapse = s_JBEAN_ExpandTime.GetF32() + s_JBEAN_HoldTime.GetF32();
            f32 T = (m_TotalTime - TStartOfCollapse + 0.5f) / 1.0f;

            T = MAX(0,MIN(1,T));

            f32 Agitation = 1.0f - ((x_cos(T * PI) + 1.0f) / 2.0f);
            f32 Ofs = kAGITATION_AMT * Agitation;
            f32 x = x_frand(-Ofs,Ofs);
            f32 y = x_frand(-Ofs,Ofs);
            f32 z = x_frand(-Ofs,Ofs);
            vector3 Delta( x,y,z );
            F.m_Position += Delta;
        }

        // Handle rotation
        F.m_RotationAmount = R_360 * 4 * DeltaTime * m_SpinRateController;

        quaternion Q( F.m_RotationAxis, F.m_RotationAmount );
        F.m_Orientation *= Q;
        F.m_Orientation.Normalize();
        if (F.m_FX.Validate())
        {
            radian3 Rot = Q.GetRotation();
            F.m_FX.SetRotation( Rot );
        }
    }

    
}

//=============================================================================================

void debris_alien_grenade_explosion::OnFragmentCollide   ( s32 iFragment, collision_mgr::collision& Col )
{
    if (m_Mode == MODE_EXPLODE)
    {
        debris_cannon::OnFragmentCollide( iFragment, Col );
        return;
    }
}

//=============================================================================================

void debris_alien_grenade_explosion::OnRenderTransparent( void )
{  
    debris_cannon::OnRenderTransparent();

    s32 i;
    for (i=0;i<4;i++)
    {
        if (m_CoreFX[i].Validate())
        {
            if (!m_CoreFX[i].IsFinished())
            {
                m_CoreFX[i].SetColor( xcolor(255,255,255,255) );
                m_CoreFX[i].Render();
            }
        }
    }
}

//=============================================================================================

//=============================================================================================

//=============================================================================================

//=============================================================================================


 