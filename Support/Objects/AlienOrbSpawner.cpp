
#include "AlienOrbSpawner.hpp"
#include "AlienOrb.hpp"
#include "AlienSpotter.hpp"
#include "Parsing\TextIn.hpp"
#include "Entropy.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "CollisionMgr\PolyCache.hpp"
#include "GameLib\RigidGeomCollision.hpp"
#include "Render\Render.hpp"
#include "EventMgr\EventMgr.hpp"
#include "render\LightMgr.hpp"
#include "audiomgr\audiomgr.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "GameLib\RenderContext.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Characters\Soldiers\soldier.hpp"
#include "navigation\CoverNode.hpp"
#include "objects\Group.hpp"

//=============================================================================
// CONSTANTS
//
//  k_SECURITY_UPDATE_RATE  = How often spotters update turrets and blackops
//                            about targets
//
//=============================================================================
static const s32 k_MAX_COVER_CHOICES        = 16;
static const f32 k_MIN_TIME_BETWEEN_SHOUTS  = 5.0f;
static const f32 k_LAUNCHED_ORB_SEARCH_TIME = 0.2f;

//=============================================================================
// SHARED
//=============================================================================
guid            s_CoverNodes[ k_MAX_COVER_CHOICES ];

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

//=============================================================================

static struct alien_orb_spawner_desc : public object_desc
{
    alien_orb_spawner_desc( void ) : object_desc( 
        object::TYPE_ALIEN_ORB_SPAWNER, 
        "Alien Orb Spawner", 
        "AI",
        object::ATTR_COLLIDABLE             | 
        object::ATTR_RENDERABLE             |
        object::ATTR_BLOCKS_ALL_PROJECTILES |         
        object::ATTR_NEEDS_LOGIC_TIME       |
        object::ATTR_SPACIAL_ENTRY,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_TARGETS_OBJS          |
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON               |
        FLAGS_BURN_VERTEX_LIGHTING ) {}

        //-------------------------------------------------------------------------

        virtual object* Create( void ) { return new alien_orb_spawner; }

} s_AlienSpotter_Desc;

//=============================================================================
//=============================================================================

const object_desc& alien_orb_spawner::GetTypeDesc( void ) const
{
    return s_AlienSpotter_Desc;
}

//=============================================================================

const object_desc& alien_orb_spawner::GetObjectType( void )
{
    return s_AlienSpotter_Desc;
}

//=============================================================================

alien_orb_spawner::alien_orb_spawner()
{
    m_SightConeAngle    = R_15;
    m_nOrbsInPool       = 4;
    m_nMaxOrbsActive    = 1;
    m_nActiveOrbs       = 0;
    m_OrbSpawnDelay     = 2.0f;
    m_OrbSpawnObj       = NULL_GUID;
    m_TimeSinceLastOrbLaunch = 0;
    m_OrbTemplateID     = -1;

    m_bTrackingLocked   = FALSE;

    m_LastTargetLockDir.Set(0,0,0);

    m_nOrbsToSpawnOnDeath = 0;

    m_AimStartLoopSoundID   = -1;
    m_AimStopSoundID        = -1;
    m_AimStartLoopVoiceID   = -1;

    m_TimeSinceLastSecurityUpdate = 0;

    m_SteeringSpeed.Set(R_180,R_160,0);

    m_CoverObject           = NULL_GUID;
    m_CoverPosition.Set(0,0,0);

    m_Velocity.Set(100,0,0);
    m_MinVelocity           = 50.0f;
    m_MaxVelocity           = 200.0f;
    m_CoverObject           = NULL_GUID;
    m_CurT                  = 0;

    m_MaxCoverDist          = 600.0f;
    m_MinCoverDist          = 30.0f;


    m_SpawnTubeLimitVolume  = NULL_GUID;
    m_TurretLimitVolume     = NULL_GUID;
    m_HostGroup             = NULL_GUID;
    m_SpawnTubeProb         = 50;

    m_TimeSinceLastShout    = 0;

    m_AOSState              = AOS_STATE_IDLE;

    m_TimeSinceLastAttack       = 0;
    m_TimeBetweenAttackChecks   = 2;
    m_AttackProb                = 10;
    m_EmptyAttackProb           = 20;
    m_AttackDamage              = 2;
    m_AttackForce               = 10;

    m_AttackMaxTime             = 5;
    m_AttackDistance            = 200.0f;
    m_ChargeupVoiceID           = -1;
    m_AttackCountdownTimer      = 0;

    m_AttackChargeParticleGuid  = NULL_GUID;
    m_AttackParticleGuid        = NULL_GUID;

    m_TimeSinceLastShout        = k_MIN_TIME_BETWEEN_SHOUTS+1;

    m_TimeToStandStillWhileFiring = 2.0f;

    m_TimeInAOSState            = 0;
    m_TimeBetweenCoverChanges   = 6.0f;
    m_LastTarget                = NULL_GUID;

    m_PatrolTarget              = NULL_GUID;

    m_CoverGroup                = NULL_GUID;

    s32 i;
    for (i=0;i<MAX_ACTIVE_ORBS;i++)
    {
        m_ActiveOrbs[i] = NULL_GUID;
    }
}

//=============================================================================

alien_orb_spawner::~alien_orb_spawner()
{
}

//=============================================================================

void alien_orb_spawner::OnEnumProp( prop_enum&    List )
{
    turret::OnEnumProp( List );

    List.PropEnumHeader( "Alien Orb Spawner",                    "Alien Orb Spawner Properties", 0 );
    List.PropEnumFileName( "Alien Orb Spawner\\Orb Blueprint Path",
                      "Area51 blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||",
                      "Resource for this item",
                      PROP_TYPE_MUST_ENUM );

    List.PropEnumExternal( "Alien Orb Spawner\\Audio Package",     "Resource\0audiopkg\0", "The audio package associated with this Alien Orb Spawner object.", 0 );
    List.PropEnumExternal( "Alien Orb Spawner\\Aim Start Sound",   "Sound\0soundexternal\0","The sound to play when the orb is created", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Alien Orb Spawner\\Aim Stop Sound",    "Sound\0soundexternal\0","The sound to play when the orb collides with something", PROP_TYPE_MUST_ENUM );


    List.PropEnumAngle ( "Alien Orb Spawner\\Sight Cone Angle",      "", 0 );    
    List.PropEnumInt   ( "Alien Orb Spawner\\Number of Orbs",        "Number of orbs the spotter is capable of launching", PROP_TYPE_EXPOSE );
    List.PropEnumInt   ( "Alien Orb Spawner\\Number of Active Orbs", "Number of orbs that can be out at a time", PROP_TYPE_EXPOSE );
    List.PropEnumInt   ( "Alien Orb Spawner\\Number of Death Orbs",  "Number of orbs to have in the world at death", 0 );
    List.PropEnumFloat ( "Alien Orb Spawner\\Orb Spawn Delay",       "Seconds between orb spawns", 0 );
    List.PropEnumGuid  ( "Alien Orb Spawner\\Orb Spawn Point",       "Where the orbs spawn from", 0 );    
    List.PropEnumGuid  ( "Alien Orb Spawner\\Activate When Empty",   "What to activate when number of orbs reaches zero", 0 );    

    List.PropEnumGuid  ( "Alien Orb Spawner\\Limit SpawnTube Volume","The specified object's bounding box defines the volum that spawn tubes have to be in to be considered valid targets", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Alien Orb Spawner\\Limit Turret Volume",   "The specified object's bounding box defines the volum that turrets have to be in to be considered valid targets", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Alien Orb Spawner\\Host Group",            "The group containing all possible hosts for orbs spawned by this spawner", PROP_TYPE_EXPOSE );
    List.PropEnumInt   ( "Alien Orb Spawner\\SpawnTube Probability", "0-100% probability of choosing spawntubes over turrets when multiple hosts are available", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Alien Orb Spawner\\Patrol Target",         "Object to follow when no target is available", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Alien Orb Spawner\\Forced Destination",    "Spawner MUST go to this object if it is valid", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Alien Orb Spawner\\Cover Group",           "Guid of a group object that contains the cover nodes this orb spawner is to use", PROP_TYPE_EXPOSE );
    

    List.PropEnumRotation( "Alien Orb Spawner\\Steering Speed",      "How fast the spawner can steer in degrees per second", 0 );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Min Movement Speed",        "cm/sec speed of the spawner", 0 );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Max Movement Speed",        "cm/sec speed of the spawner", 0 );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Min Cover Approach Dist", "", 0 );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Max Cover Approach Dist", "", 0 );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Cover Change Time", "How many seconds will the spawner stay at a cover node", 0 );

    List.PropEnumFloat   ( "Alien Orb Spawner\\Time Between Attack Checks", "How often the spawner checks to go offensive", PROP_TYPE_EXPOSE );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Attack Prob", "0-100 probability of deciding to attack during an attack check", PROP_TYPE_EXPOSE );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Empty Attack Prob", "0-100 probability of deciding to attack during an attack check when NO ORBS remain in the spawner", PROP_TYPE_EXPOSE );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Attack Damage", "", PROP_TYPE_EXPOSE  );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Attack Force", "", PROP_TYPE_EXPOSE  );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Attack Max Chargeup Time", "How long to charge before detonating", PROP_TYPE_EXPOSE );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Attack Desired Distance", "", PROP_TYPE_EXPOSE );
    List.PropEnumFloat   ( "Alien Orb Spawner\\Attack Retreat Delay", "How long to wait after firing before retreating to cover", PROP_TYPE_EXPOSE );

}

//=============================================================================

xbool alien_orb_spawner::OnProperty( prop_query&   I    )
{
    if (turret::OnProperty(I))
    {}
    else if (I.IsVar("Alien Orb Spawner\\SpawnTube Probability"))
    {
        if (I.IsRead())
        {
            I.SetVarInt( m_SpawnTubeProb );
        }
        else
        {
            m_SpawnTubeProb = MIN(100,MAX(0,I.GetVarInt()));
        }
    }
    else if (I.VarGUID ( "Alien Orb Spawner\\Host Group", m_HostGroup ))
    {}
    else if (I.VarGUID ( "Alien Orb Spawner\\Cover Group", m_CoverGroup ))
    {}
    else if (I.VarGUID ( "Alien Orb Spawner\\Activate When Empty", m_ActivateWhenEmpty))
    {}
    else if (I.VarGUID ( "Alien Orb Spawner\\Forced Destination", m_ForcedDestination))
    {
        if (m_ForcedDestination)
        {
            EnterAOSState( AOS_STATE_SCRIPTED_MOVEMENT );
            m_bAOSStateLocked = TRUE;
        }
        else
        {
            m_bAOSStateLocked = FALSE;
            EnterAOSState( AOS_STATE_IDLE );
        }
    }
    else if (I.VarFloat( "Alien Orb Spawner\\Empty Attack Prob", m_EmptyAttackProb, 0, 100 ))
    {}
    else if (I.VarGUID ( "Alien Orb Spawner\\Patrol Target", m_PatrolTarget ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Cover Change Time", m_TimeBetweenCoverChanges ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Attack Retreat Delay", m_TimeToStandStillWhileFiring ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Attack Desired Distance", m_AttackDistance ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Attack Max Chargeup Time", m_AttackMaxTime ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Attack Damage", m_AttackDamage ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Attack Force", m_AttackForce ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Time Between Attack Checks", m_TimeBetweenAttackChecks ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Attack Prob", m_AttackProb, 0, 100 ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Min Cover Approach Dist", m_MinCoverDist ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Max Cover Approach Dist", m_MaxCoverDist ))
    {}
    else if (I.VarGUID( "Alien Orb Spawner\\Limit SpawnTube Volume", m_SpawnTubeLimitVolume ))
    {}
    else if (I.VarGUID( "Alien Orb Spawner\\Limit Turret Volume", m_TurretLimitVolume ))
    {}
    else if (I.VarRotation("Alien Orb Spawner\\Steering Speed",m_SteeringSpeed))
    {}
    else if (I.VarFloat("Alien Orb Spawner\\Min Movement Speed",m_MinVelocity))
    {}
    else if (I.VarFloat("Alien Orb Spawner\\Max Movement Speed",m_MaxVelocity))
    {}
    else if (I.VarAngle( "Alien Orb Spawner\\Sight Cone Angle", m_SightConeAngle ))
    {}
    else if (I.VarInt( "Alien Orb Spawner\\Number of Orbs", m_nOrbsInPool ))
    {}
    else if (I.VarInt( "Alien Orb Spawner\\Number of Active Orbs", m_nMaxOrbsActive ))
    {}
    else if (I.VarInt( "Alien Orb Spawner\\Number of Death Orbs", m_nOrbsToSpawnOnDeath ))
    {}
    else if (I.VarFloat( "Alien Orb Spawner\\Orb Spawn Delay", m_OrbSpawnDelay ))
    {}
    else if (I.VarGUID( "Alien Orb Spawner\\Orb Spawn Point", m_OrbSpawnObj ))
    {}
    else if( I.IsVar( "Alien Orb Spawner\\Orb Blueprint Path" ) )
    {
        if( I.IsRead() )
        {
            if( m_OrbTemplateID < 0 )
            {
                I.SetVarFileName("",256);
            }
            else
            {
                I.SetVarFileName( g_TemplateStringMgr.GetString( m_OrbTemplateID ), 256 );
            }            
        }
        else
        {
            if ( x_strlen( I.GetVarFileName() ) > 0 )
            {
                m_OrbTemplateID = g_TemplateStringMgr.Add( I.GetVarFileName() );
            }
        }        
    }
    else if( I.IsVar( "Alien Orb Spawner\\Audio Package" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hSpotterAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hSpotterAudioPackage.SetName( pString );                

                // Load the audio package.
                if( m_hSpotterAudioPackage.IsLoaded() == FALSE )
                    m_hSpotterAudioPackage.GetPointer();
            }
        }
    }
    else if( I.IsVar( "Alien Orb Spawner\\Aim Start Sound" ) )
    {
        if( I.IsRead() )
        {
            if( m_AimStartLoopSoundID != -1 )
                I.SetVarExternal( g_StringMgr.GetString( m_AimStartLoopSoundID ), 64 );
            else
                I.SetVarExternal( "", 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = I.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                s32 PkgIndex = String.Find( '\\', 0 );

                if( PkgIndex != -1 )
                {
                    xstring Pkg = String.Left( PkgIndex );
                    String.Delete( 0, PkgIndex+1 );

                    m_hSpotterAudioPackage.SetName( Pkg );                

                    // Load the audio package.
                    if( m_hSpotterAudioPackage.IsLoaded() == FALSE )
                        m_hSpotterAudioPackage.GetPointer();
                }

                m_AimStartLoopSoundID = g_StringMgr.Add( String );
            }
        }
    }
    else if( I.IsVar( "Alien Orb Spawner\\Aim Stop Sound" ) )
    {
        if( I.IsRead() )
        {
            if( m_AimStopSoundID != -1 )
                I.SetVarExternal( g_StringMgr.GetString( m_AimStopSoundID ), 64 );
            else
                I.SetVarExternal( "", 64 );
        }
        else
        {
            // Get the FileName
            xstring ExtString = I.GetVarExternal();
            if( !ExtString.IsEmpty() )
            {
                xstring String( ExtString );

                s32 PkgIndex = String.Find( '\\', 0 );

                if( PkgIndex != -1 )
                {
                    xstring Pkg = String.Left( PkgIndex );
                    String.Delete( 0, PkgIndex+1 );

                    m_hSpotterAudioPackage.SetName( Pkg );                

                    // Load the audio package.
                    if( m_hSpotterAudioPackage.IsLoaded() == FALSE )
                        m_hSpotterAudioPackage.GetPointer();
                }

                m_AimStopSoundID = g_StringMgr.Add( String );
            }
        }
    }
    else
    {
        return FALSE;
    }
    return TRUE;
}

//=============================================================================

void alien_orb_spawner::OnAdvanceLogic  ( f32 DeltaTime )
{
    turret::OnAdvanceLogic( DeltaTime );

    if (IsDestroyed())
        return;

    m_CurT               += DeltaTime;
    m_TimeSinceLastShout += DeltaTime;
    m_TimeSinceLastAttack+= DeltaTime;
    m_TimeInAOSState     += DeltaTime;
    m_TimeSinceTargetLost+= DeltaTime;

    UpdateCover( m_CoverObject );

    m_TimeSinceLastOrbLaunch += DeltaTime;
    
    //if (m_LastAimedTrackingStatus == TRACK_LOCKED)
    if (m_TargetGuid != NULL_GUID)
    {
        // See if it's time to launch an orb
        if (m_TimeSinceLastOrbLaunch > m_OrbSpawnDelay)
        {
            if (m_nOrbsInPool != 0)
            {
                LaunchOrb();
            }
        }
    }
    
    if (m_CoverObject)
    {
        // Update cover pos
        object* pObj = g_ObjMgr.GetObjectByGuid( m_CoverObject );
        if (pObj)
        {
            m_CoverPosition = pObj->GetPosition();
        }
    }

    // Handle movement
    if (   (m_CoverPosition.GetX() == 0)
        && (m_CoverPosition.GetY() == 0)
        && (m_CoverPosition.GetZ() == 0))
    {
        // Uninitialized case.  Target the current pos instead
        m_CoverPosition = GetPosition();  
        m_LastIdlePos   = m_CoverPosition;
    }

    vector3 DesiredPos = m_CoverPosition;
    switch(m_AOSState)
    {
    case AOS_STATE_DEFENSIVE:
        DesiredPos = m_CoverPosition;
        if (m_TimeSinceLastAttack > m_TimeBetweenAttackChecks)
        {
            m_TimeSinceLastAttack = 0;
            f32 Prob = m_AttackProb;
            if (m_nOrbsInPool == 0)
                Prob = m_EmptyAttackProb;
            if (x_frand(0,100) < Prob)
            {
                EnterAOSState( AOS_STATE_OFFENSIVE );
            }            
        }
        if (m_TimeInAOSState > m_TimeBetweenCoverChanges)
        {
            FindCoverFromTarget( m_CoverObject );
            m_TimeInAOSState = 0;
        }
        if (m_TimeSinceTargetLost > m_TimeCool)
        {
            EnterAOSState( AOS_STATE_IDLE );
        }
        break;
    case AOS_STATE_FIRING:         
        {            
            // Stay still until the particle effect is dead, then go defensive
            DesiredPos = m_LastIdlePos;
            HandleFiring( DeltaTime );            
        }
        break;
    case AOS_STATE_IDLE: 
        if (NULL_GUID == m_PatrolTarget)
        {
            DesiredPos = m_LastIdlePos;
        }
        else
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_PatrolTarget );
            if (pObj)
            {
                DesiredPos = pObj->GetPosition();
            }
            else
            {
                DesiredPos = m_LastIdlePos;
            }
        }
        break;
    case AOS_STATE_OFFENSIVE:
        {
            // for now, rush the player
            object* pObj = g_ObjMgr.GetObjectByGuid( m_AttackGuid );
            if (pObj)
            {
                if (pObj->IsKindOf( actor::GetRTTI() ))
                {
                    actor& A = actor::GetSafeType( *pObj );
                    DesiredPos = A.GetPositionWithOffset( actor::OFFSET_EYES );
                    radian Yaw = A.GetSightYaw();
                    vector3 Facing(0,0,m_AttackDistance);
                    Facing.RotateY( Yaw );
                    DesiredPos += Facing;
                }
                else
                {
                    DesiredPos = pObj->GetPosition();
                }
            }
            else
                EnterAOSState( AOS_STATE_DEFENSIVE );            

            HandleOffensive(DeltaTime);
        }
        break;
    case AOS_STATE_SCRIPTED_MOVEMENT:
        {
            // Override DesiredPos if m_ForcedDestination is set
            if (m_ForcedDestination)
            {
                object* pObj = g_ObjMgr.GetObjectByGuid(m_ForcedDestination);
                if (pObj)
                {
                    DesiredPos = pObj->GetPosition();
                }
            }
            else
            {
                EnterAOSState( AOS_STATE_IDLE );
            }
        }
        break;
    }

    
    HandleMove( DesiredPos, DeltaTime );

    MoveDependants();
}

//=============================================================================

void alien_orb_spawner::EnterAOSState( aosstate State )
{
    if (m_bAOSStateLocked)
        return;


    // Handle exit
    switch( m_AOSState )
    {
    case AOS_STATE_IDLE:        
        break;
    case AOS_STATE_DEFENSIVE:
        break;
    case AOS_STATE_OFFENSIVE:
        {
            ASSERT( m_AttackGuid != NULL_GUID );
            ASSERT(m_AttackCountdownTimer <= 0.0f);

            m_RadiusFire = 0;
            m_AttackGuid = NULL_GUID;
            g_AudioMgr.Release(m_ChargeupVoiceID,0.0f);
            if (m_AttackChargeParticleGuid)
                g_ObjMgr.DestroyObject( m_AttackChargeParticleGuid );
            m_AttackChargeParticleGuid = NULL_GUID;
        }
        break;
    case AOS_STATE_FIRING:  
        {
            g_ObjMgr.DestroyObject(m_AttackParticleGuid );
            m_AttackParticleGuid = NULL_GUID;
            m_TimeSinceLastAttack = 0;
            FindCoverFromTarget(NULL_GUID);

            object* pObj = g_ObjMgr.GetObjectByGuid(m_CoverObject);
            if (pObj)
            {
                vector3 Pos = pObj->GetPosition();
                m_Velocity = Pos - GetPosition();
                m_Velocity.NormalizeAndScale(m_MinVelocity);
            }
        }
        break;
    }

    xbool bOkToSwitch = TRUE;
    // Handle entry
    switch( State )
    {
    case AOS_STATE_IDLE:   
        m_LastIdlePos = GetPosition();
        break;
    case AOS_STATE_DEFENSIVE:
        break;
    case AOS_STATE_OFFENSIVE:
        if ( NULL_GUID != m_ForcedDestination )
            bOkToSwitch = FALSE;
        else
        if ( NULL_GUID == m_TargetGuid )
            bOkToSwitch = FALSE;
        else
        {
            if (IsInMyZone(g_ObjMgr.GetObjectByGuid(m_TargetGuid)))
            {                
                m_AttackGuid            = m_TargetGuid;
                m_RadiusFire            = 500;   
                m_AttackCountdownTimer  = m_AttackMaxTime;        

                m_AttackChargeParticleGuid = particle_emitter::CreatePresetParticleAndOrient( "sh_spotter_chargeup",
                    radian3(0,0,0),
                    GetPosition(),
                    GetZone1()  );

                m_ChargeupVoiceID = g_AudioMgr.PlayVolumeClipped( "Forcefield_loop", GetPosition(), GetZone1(), TRUE );    
            }
            else
            {
                bOkToSwitch = FALSE;
            }
        }
        break;
    case AOS_STATE_FIRING:    
        if ( NULL_GUID != m_ForcedDestination )
            bOkToSwitch = FALSE;
        else
        {
            m_AttackCountdownTimer = m_TimeToStandStillWhileFiring;
            m_LastIdlePos = GetPosition();                
            m_AttackParticleGuid = particle_emitter::CreatePresetParticleAndOrient( "sh_spotter_detonate",
                radian3(0,0,0),
                GetPosition(),
                GetZone1()  );             
            g_AudioMgr.PlayVolumeClipped( "Alien_Teleport", GetPosition(), GetZone1(), TRUE );    
            break;
        }
    }

    if (bOkToSwitch)
    {
        m_AOSState = State;
        m_TimeInAOSState = 0;
    }
}

//=============================================================================

void alien_orb_spawner::HandleMove( const vector3& DesiredPos, f32 DeltaTime )
{
    // Moving is forbidden while firing
    if (m_AOSState == AOS_STATE_FIRING)
        return;

    vector3 MyPos = GetPosition();
    vector3 Delta = DesiredPos - MyPos;
    f32     Dist  = Delta.Length();

    radian CurPitch,CurYaw;
    radian DesiredPitch,DesiredYaw;

    m_Velocity.GetPitchYaw( CurPitch, CurYaw );
    Delta.GetPitchYaw( DesiredPitch, DesiredYaw );

    radian DeltaP = x_MinAngleDiff( DesiredPitch, CurPitch );
    radian DeltaY = x_MinAngleDiff( DesiredYaw,   CurYaw   );

    radian TurnP, TurnY;

    TurnP  = m_SteeringSpeed.Pitch * DeltaTime;
    TurnY  = m_SteeringSpeed.Yaw * DeltaTime;

    if( x_abs( DeltaP ) < TurnP )
    {
        CurPitch = DesiredPitch;
    }
    else
    {
        if( DeltaP > R_0 )  
            CurPitch += TurnP;
        else                
            CurPitch -= TurnP;
    }

    if( x_abs( DeltaY ) < TurnY )
    {
        CurYaw = DesiredYaw;     
    }
    else
    {
        if( DeltaY > R_0 )  
            CurYaw += TurnY;
        else
            CurYaw -= TurnY;
    }

    CurYaw   = x_ModAngle( CurYaw   );
    CurPitch = x_ModAngle( CurPitch );

    f32 VelLength = m_MinVelocity;    

    f32 DistT = (Dist-m_MinCoverDist) / (m_MaxCoverDist-m_MinCoverDist);

    if (DistT > 1)
        VelLength = m_MaxVelocity;
    else if (DistT < 0)
        VelLength = m_MinVelocity;
    else
        VelLength = ((DistT) * (m_MaxVelocity - m_MinVelocity)) + m_MinVelocity;

    if ((m_TimeInAOSState < 1.0f) && (m_AOSState != AOS_STATE_OFFENSIVE))
    {
        VelLength *= m_TimeInAOSState;
    }

    m_Velocity.Set(0,0,VelLength);

    m_Velocity.Rotate( radian3(CurPitch,CurYaw,0));

    vector3 Vel = m_Velocity;
    Vel *= DeltaTime;

    // Update random
    if (m_CurT > 100)
        m_CurT -= 100;

    vector3 Rand( x_sin(m_CurT*2), x_cos(m_CurT*1.2f), x_sin(m_CurT*0.7f));

    Rand *= 1.0f * DistT;


    MyPos += Vel + Rand;

    OnMove( MyPos );
}

//=============================================================================

xbool alien_orb_spawner::LaunchOrb( void )
{
    
    // Bail if we are empty
    if ( m_nOrbsInPool == 0 )
        return FALSE;

    // Bail if there are no valid targets
    if (!AreValidTargets())
        return FALSE;

    s32 i;
    s32 iSlot = -1;

    for (i=0;i<m_nMaxOrbsActive;i++)
    {
        if (m_ActiveOrbs[i] == NULL_GUID)
        {
            iSlot = i;
            break;
        }
        else
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_ActiveOrbs[i] );
            if (NULL == pObj)
            {
                iSlot=i;
                break;
            }
             
            if (!pObj->IsKindOf( alien_orb::GetRTTI() ))
            {                
                iSlot=i;
                break;
            }             
        }
    }
    if (iSlot == -1)
        return FALSE;

    m_ActiveOrbs[iSlot] = NULL_GUID;

    vector3 Pos,Dir;
    radian3 Rot;

    //Get Launch Point
    if (m_OrbSpawnObj != NULL_GUID)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_OrbSpawnObj );
        if (pObj)
        {
            const matrix4& L2W = pObj->GetL2W();

            Pos = L2W.GetTranslation();
            Rot = L2W.GetRotation();
            Dir.Set(0,0,1);
            Dir.Rotate(Rot);
        }
    }
    else
    {
        GetLaunchPoint(Pos,Dir,Rot);
    }

    // create orb
    guid gObj = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString( m_OrbTemplateID ), Pos, Rot, GetZone1(), GetZone2() );
    object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
    if (pObj)
    {        
        alien_orb& Orb = alien_orb::GetSafeType( *pObj );  
        Orb.SetSpawner( GetGuid() );

        Orb.SetSpawnTubeLimit( m_SpawnTubeLimitVolume );
        Orb.SetTurretLimit( m_TurretLimitVolume );
        Orb.SetHostGroup( m_HostGroup );
        Orb.SetSpawnTubeProb( m_SpawnTubeProb );
        Orb.SetSearchTime( k_LAUNCHED_ORB_SEARCH_TIME );

        matrix4 L2W( vector3(1,1,1), Rot, Pos );
        Orb.OnTransform( L2W );  

        Dir = Pos - GetPosition();
        Dir.NormalizeAndScale(100.0f);
        
        Orb.Launch( Pos, Pos + Dir );
    }

    m_ActiveOrbs[ iSlot ] = gObj;
    
    m_TimeSinceLastOrbLaunch = 0;

    if (m_nOrbsInPool > 0)    
        m_nOrbsInPool--;

    if (m_nOrbsInPool == 0)
    {
        if (m_ActivateWhenEmpty)
        {
            object* pObj = g_ObjMgr.GetObjectByGuid(m_ActivateWhenEmpty);
            if (pObj)
            {
                pObj->OnActivate(TRUE);
            }
        }
    }

    return TRUE;
}

//=========================================================================

turret::tracking_status alien_orb_spawner::TrackTarget( f32 DeltaTime )
{
    tracking_status Ret = TRACK_NO_TARGET;

    if (m_TargetGuid == 0)
    {        
        return TRACK_NO_TARGET;
    }
    else
    if (!CanSenseTarget() && !CanFireAtTarget())
    {
        return TRACK_NO_TARGET;
    }
    else
    if (!IsTargetValid())
    {
        return TRACK_NO_TARGET;
    }
    else
    {   
        vector3 TargetPos = GetTargetPos();
        vector3 SensorPos;
        radian3 SensorRot;
        GetSensorInfo( SensorPos, SensorRot );

        // We want to work with position and angles in local turret space
        matrix4 W2L = GetL2W();

        W2L.InvertRT();

        TargetPos = W2L.Transform( TargetPos );
        SensorPos = W2L.Transform( SensorPos );

        vector3 Delta = TargetPos - SensorPos;
        m_LocalTargetDir = Delta;
        m_LocalTargetDir.Normalize();

        f32 Dot = m_LocalTargetDir.Dot( m_LastTargetLockDir );
        

        if (( Dot > cos(m_SightConeAngle) ) && ( Dot <= 1.0f ))
        {
            // Target is still inside the sight cone.
            // We return the happy "TRACK_LOCKED" in this case
            // to force the turret to think it is locked on.
            Ret = TRACK_LOCKED;
        }
        else
        {            
            Ret = turret::TrackTarget( DeltaTime );
            if (Ret == TRACK_AIMING)
            {
                
            }
            if (Ret == TRACK_LOCKED)
            {
                m_LastTargetLockDir = m_LocalTargetDir;
            }
        }
    }

    if (Ret == TRACK_NO_TARGET)
    {
        SetTargetGuid(0);
        StopAiming();
    }
    else
    {
        if (Ret == TRACK_AIMING)
        {            
            StartAiming();
        }
        else if ((Ret == TRACK_LOCKED) || (Ret == TRACK_OUTSIDE_FOF))
        {
            StopAiming();
        }
    }
    return Ret;
}

//=========================================================================

void alien_orb_spawner::SetTargetGuid   ( guid Guid )
{
    if ((Guid == NULL_GUID) || (Guid != m_TargetGuid))
    {
        m_bTrackingLocked = FALSE;
        m_LastTargetLockDir.Set(0,0,0);
    }

    if (NULL_GUID == Guid)
        m_TimeSinceTargetLost = 0;

    if (NULL_GUID != Guid)
        m_LastTarget = Guid;

    // If we are offensive, let the attack code handle it
    if (m_AOSState != AOS_STATE_OFFENSIVE)
    {
        EnterAOSState( AOS_STATE_DEFENSIVE );
    }
    

    Shout( Guid );

    turret::SetTargetGuid( Guid );
}

//=========================================================================

xbool alien_orb_spawner::GetLaunchPoint( vector3& Pos, vector3& Dir, radian3& Rot )
{
    xbool bValid = FALSE;

    if( m_hAnimGroup.IsLoaded() )
    {     
        anim_group* pAnimGroup = m_hAnimGroup.GetPointer();

        s32   iBone  = pAnimGroup->GetBoneIndex( "orb_spawn" );
        
        if (iBone != -1)
        {   
            const matrix4* pL2W = m_AnimPlayer.GetBoneL2W( iBone, TRUE );
            if (NULL != pL2W)
            {
                matrix4 L2W = *pL2W;

                L2W.PreTranslate( m_AnimPlayer.GetBoneBindPosition( iBone ) );

                Pos = L2W.GetTranslation();
                Rot = L2W.GetRotation();
                Dir.Set(0,0,1);
                Dir.Rotate( Rot );
                
                bValid = TRUE;
            }
        }
    }
    if (!bValid)
    {
        // Just use the position of the turret
        const matrix4& L2W = GetL2W();
        Pos = L2W.GetTranslation();
        Rot = L2W.GetRotation();
        Dir.Set(0,0,1);
        Dir.Rotate( Rot );
        return FALSE;
    }    
    return TRUE;
}

//=========================================================================

void alien_orb_spawner::OnEnterState( state NewState )
{   
    turret::OnEnterState( NewState );

    if (NewState == STATE_DESTROYED)
    {
        // Spawn orbs when we die
        DeathSpawnOrbs();

        // Kill off fx
        if (m_AttackChargeParticleGuid)
            g_ObjMgr.DestroyObject( m_AttackChargeParticleGuid );
        m_AttackChargeParticleGuid = NULL_GUID;

        g_AudioMgr.Release(m_ChargeupVoiceID,0.0f);

        g_ObjMgr.DestroyObject( GetGuid() );        
    }
    else if (NewState == STATE_FIRING)
    {
        // FIRE!

    }
}

//=========================================================================

void alien_orb_spawner::StartAiming()
{
    if (m_AimStartLoopVoiceID != -1)
        return;

    if (m_AimStartLoopSoundID != -1)
    {
        const char* pSoundName = g_StringMgr.GetString( m_AimStartLoopSoundID );
        if (pSoundName)
        {
            const matrix4& L2W = GetL2W();
            m_AimStartLoopVoiceID = g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );
        }
    }
}

//=========================================================================

void alien_orb_spawner::StopAiming()
{
    if (m_AimStartLoopVoiceID != -1)
    {
        g_AudioMgr.Release( m_AimStartLoopVoiceID, 0.0f );

        m_AimStartLoopVoiceID = -1;

        if (m_AimStopSoundID != -1)
        {
            const char* pSoundName = g_StringMgr.GetString( m_AimStopSoundID );
            if (pSoundName)
            {
                const matrix4& L2W = GetL2W();
                g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );
            }
        }
    }
}

//=========================================================================

void alien_orb_spawner::DeathSpawnOrbs( void )
{
    // Create multiple orbs, and don't bother tracking their guids.
    // We're dead anyway... :(

    // Bail if there are no valid targets
    if (!AreValidTargets())
        return;

    vector3 Pos,Dir;
    radian3 Rot;

    //Get Launch Point
    if (m_OrbSpawnObj != NULL_GUID)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_OrbSpawnObj );
        if (pObj)
        {
            const matrix4& L2W = pObj->GetL2W();

            Pos = L2W.GetTranslation();
            Rot = L2W.GetRotation();
            Dir.Set(0,0,1);
            Dir.Rotate(Rot);
        }
    }
    else
    {
        GetLaunchPoint(Pos,Dir,Rot);
    }

    // Count the number of currently active orbs
    s32 nToSpawn = m_nOrbsToSpawnOnDeath;

    s32 i;
    for (i=0;i<m_nMaxOrbsActive;i++)
    {
        if (m_ActiveOrbs[i] != NULL_GUID)
        {
            object* pObj = g_ObjMgr.GetObjectByGuid( m_ActiveOrbs[i] );
            if (NULL == pObj)
            {
                continue;
            }
    
            if (pObj->IsKindOf( alien_orb::GetRTTI() ))
            {                
                nToSpawn--;
                break;
            }             
        }
    }

    // create orb
    for (i=0;i<nToSpawn;i++)
    {    
        guid gObj = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString( m_OrbTemplateID ), Pos, Rot, GetZone1(), GetZone2() );
        object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
        if (pObj)
        {        
            alien_orb& Orb = alien_orb::GetSafeType( *pObj );  
            Orb.SetSpawner( GetGuid() );
            Orb.SetSpawnTubeLimit( m_SpawnTubeLimitVolume );
            Orb.SetTurretLimit( m_TurretLimitVolume );


            matrix4 L2W( vector3(1,1,1), Rot, Pos );
            Orb.OnTransform( L2W );  

            Dir = Pos - GetPosition();
            Dir.NormalizeAndScale(100.0f);

            vector3     Rand;
            Rand.Set( x_frand(-100,100), x_frand(-100,100), x_frand(-100,100) );
            
            Orb.Launch( Pos, Pos + Dir + Rand );
        }
    }
}

//=========================================================================

#ifndef X_RETAIL
void alien_orb_spawner::OnDebugRender( void )
{
    turret::OnDebugRender();

    vector3 MyPos = GetPosition();
    vector3 Vel = m_Velocity;
    Vel.NormalizeAndScale(500.0f);

    draw_Line( MyPos, MyPos + Vel, XCOLOR_WHITE );
    draw_Label( MyPos + Vel, XCOLOR_WHITE, "VEL" );

    draw_Line( MyPos, m_CoverPosition, XCOLOR_GREEN );
    draw_Label( m_CoverPosition, XCOLOR_GREEN, "TARGET" );

    if (m_AOSState == AOS_STATE_OFFENSIVE)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_AttackGuid );
        if (pObj)
        {
            if (pObj->IsKindOf( actor::GetRTTI() ))
            {
                actor& A = actor::GetSafeType( *pObj );
                vector3 DesiredPos = A.GetPositionWithOffset( actor::OFFSET_EYES );
                radian Yaw = A.GetSightYaw();
                vector3 Facing(0,0,200);
                Facing.RotateY( Yaw );
                DesiredPos += Facing;

                draw_Sphere( DesiredPos, 5, XCOLOR_RED );
            }
        }        
    }
}
#endif // X_RETAIL

//=========================================================================

void alien_orb_spawner::UpdateCover( guid InvalidCover  )
{
    if (NULL_GUID == m_TargetGuid)
    {
        // No target, just stay where we are
        return;
    }

    object* pTarget = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if (NULL == pTarget)
        return;
        
    if (NULL_GUID == m_CoverObject)
        FindCoverFromTarget(InvalidCover);

    if (NULL_GUID == m_CoverObject)
        return;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_CoverObject );
    if (NULL == pObj)
        return;

    if (!pObj->IsKindOf( cover_node::GetRTTI() ))
    {
        ASSERTS( FALSE, "AlienOrbSpawner cover node isn't actually a cover_node object!" );
        return;
    }

    cover_node& CN = cover_node::GetSafeType( *pObj );

    

    if (CN.IsCoverFromLocation( pTarget->GetPosition() ))
    {
        // Later, we can decide to break cover once in a while
        return;
    }

    FindCoverFromTarget(InvalidCover);
}

//=========================================================================

void alien_orb_spawner::FindCoverFromTarget( guid InvalidCover )
{ 
    guid Target = m_TargetGuid;
    if (NULL_GUID == Target)
    {        
        Target = m_LastTarget;
    }

    if (NULL_GUID == Target)
        return;

    object* pTarget = g_ObjMgr.GetObjectByGuid( Target );
    if (NULL == pTarget)
        return;

    vector3 TargetPos = pTarget->GetPosition();

    slot_id iSlot = g_ObjMgr.GetFirst( object::TYPE_COVER_NODE );

    s32 nCover = 0;

    object* pGroup = NULL;
    if ( NULL_GUID != m_CoverGroup )
    {
        pGroup = g_ObjMgr.GetObjectByGuid( m_CoverGroup );
        if (pGroup)
        {
            if (!pGroup->IsKindOf( group::GetRTTI() ))
            {
                pGroup = NULL;
            }
        }
    }

    if (NULL == pGroup)
    {
        // No cover group assigned.  Loop through all cover
        while ((SLOT_NULL != iSlot) && (nCover < k_MAX_COVER_CHOICES))
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( iSlot );
            if (pObj)
            {
                if (pObj->GetGuid() != InvalidCover )
                {
                    if (pObj->IsKindOf( cover_node::GetRTTI() ))
                    {
                        if (IsInMyZone(pObj))
                        {                        
                            cover_node CN = cover_node::GetSafeType( *pObj );

                            if (CN.GetNumValidCoverPackages()==0)
                            {
                                if (CN.IsCoverFromLocation( TargetPos ))
                                {
                                    // Add to list
                                    s_CoverNodes[ nCover++ ] = CN.GetGuid();
                                }
                            }
                        }
                    }
                }
            }
            iSlot = g_ObjMgr.GetNext( iSlot );
        }
    }
    else
    {
        // We have a cover group, use it to limit the search space
        group& Group = group::GetSafeType( *pGroup );

        s32 nGroupCover = Group.GetNumChildren( object::TYPE_COVER_NODE );

        s32 i;
        for (i=0;i<nGroupCover;i++)
        {
            guid gObj = Group.GetChild( i, object::TYPE_COVER_NODE );
            object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
            if (pObj)
            {
                if (pObj->GetGuid() == InvalidCover )
                    continue;
                if (!pObj->IsKindOf( cover_node::GetRTTI() ))
                    continue;
                if (!IsInMyZone(pObj))
                    continue;

                cover_node CN = cover_node::GetSafeType( *pObj );

                if (CN.GetNumValidCoverPackages()==0)
                {
                    if (CN.IsCoverFromLocation( TargetPos ))
                    {
                        // Add to list
                        s_CoverNodes[ nCover++ ] = CN.GetGuid();
                    }
                }                
            }            
        }
    }
    if (nCover == 0)
        return;

    s32 iCover = x_irand(0,nCover);

    m_CoverObject = s_CoverNodes[ iCover ]; 
}

//=========================================================================

void alien_orb_spawner::OnPain( const pain& Pain )
{
    turret::OnPain(Pain);

    AlertTo( Pain.GetOriginGuid() );

    Shout( Pain.GetOriginGuid() );

    FindCoverFromTarget( m_CoverObject );
}
//=========================================================================

void alien_orb_spawner::Shout( guid About )
{
    if (m_TimeSinceLastShout < k_MIN_TIME_BETWEEN_SHOUTS)
        return;

    m_TimeSinceLastShout = 0;

    slot_id iSlot = g_ObjMgr.GetFirst( object::TYPE_ALIEN_ORB_SPAWNER );

    while (SLOT_NULL != iSlot)
    {
        object* pObj = g_ObjMgr.GetObjectBySlot( iSlot );
        if (pObj)
        {
            if (pObj->IsKindOf( alien_orb_spawner::GetRTTI()))
            {
                if (IsInMyZone( pObj ))
                {
                    alien_orb_spawner& AOS = alien_orb_spawner::GetSafeType( *pObj );
                    AOS.AlertTo( About );
                }
            }
        }
        iSlot = g_ObjMgr.GetNext( iSlot );
    }
}

//=========================================================================

void alien_orb_spawner::AlertTo( guid Who )
{
    m_RadiusFire = 80000;
    m_RadiusSense= 80000;

    if (m_TargetGuid == NULL_GUID)
    {
        SetTargetGuid( Who );
    }
}

//=========================================================================

void alien_orb_spawner::TryToFireAtTarget( void )
{
    /*
    if (m_AOSState != AOS_STATE_OFFENSIVE)
        return;

    if (    CanFireAtTarget()                                           
        && ( m_TargetGuid != m_TrackingGuid ) )       
    {
        EnterAOSState( AOS_STATE_FIRING );
    }
    */
}

//=========================================================================

void alien_orb_spawner::TransmitPain( const bbox& Box )
{    
    (void)Box;
    /*
    pain PainEvent;
    PainEvent.Type      = (pain::type)pain::TYPE_ELECTROCUTION;
    PainEvent.Center    = GetPosition();
    PainEvent.Origin    = GetGuid();    
    PainEvent.DamageR0  = m_AttackDamage; 
    PainEvent.DamageR1  = 0; 
    PainEvent.ForceR0   = m_AttackForce;
    PainEvent.ForceR1   = 0;
    PainEvent.RadiusR0  = 0;
    PainEvent.RadiusR1  = Box.GetRadius();
    //PainEvent.Collision = g_CollisionMgr.m_Collisions[i];
    PainEvent.Direction.Normalize();

    vector3 PainPos = Box.GetCenter();

    static const s32 kMaxPainObjects = 16;
    
    // Collect a list of objects that need to receive pain. By
    // using a list rather than calling pain right away we avoid
    // nested SelectBBox calls.
    s32     nPainObjects = 0;
    slot_id PainList[kMaxPainObjects];
    g_ObjMgr.SelectBBox( object::ATTR_DAMAGEABLE, Box );
    for ( slot_id iSlot = g_ObjMgr.StartLoop();
          (SLOT_NULL != iSlot) && (nPainObjects<kMaxPainObjects);
          iSlot = g_ObjMgr.GetNextResult( iSlot ) )
    {
        PainList[nPainObjects++] = iSlot;
    }
    g_ObjMgr.EndLoop();

    // now call pain for each of the objects we've selected
    for( s32 i=0; i<nPainObjects; i++ )
    {
        object* pObj = g_ObjMgr.GetObjectBySlot( PainList[i] );
        if (pObj)
        {
            vector3 ObjPos = pObj->GetPosition();
            vector3 Delta = PainPos - ObjPos;
            Delta.NormalizeAndScale(50);
            PainEvent.PtOfImpact= ObjPos + Delta;

            Delta.NormalizeAndScale(-1);

            PainEvent.Direction = Delta;

            pObj->OnPain( PainEvent );
        }
    }
    */
}

//=========================================================================

xbool alien_orb_spawner::IsInMyZone( const object* pObj ) const
{
    u16 Zone1 = GetZone1();
    u16 Zone2 = GetZone2();

    xbool bValid = FALSE;

    u16 Z1,Z2;                    
    Z1 = pObj->GetZone1();
    Z2 = pObj->GetZone2();

    if (Zone1 != 0)
    {
        if ((Z1 == Zone1) || (Z2 == Zone1))
            bValid = TRUE;
    }
    if (Zone2 != 0)
    {
        if ((Z1 == Zone2) || (Z2 == Zone2))
            bValid = TRUE;
    }                

    return bValid;
}

//=========================================================================

xbool alien_orb_spawner::IsInMyZone( const guid& Guid ) const
{
    if (NULL_GUID == Guid)
        return FALSE;
    object* pObj = g_ObjMgr.GetObjectByGuid(Guid);
    if (NULL == pObj)
        return FALSE;

    u16 Zone1 = GetZone1();
    u16 Zone2 = GetZone2();

    xbool bValid = FALSE;

    u16 Z1,Z2;                    
    Z1 = pObj->GetZone1();
    Z2 = pObj->GetZone2();

    if (Zone1 != 0)
    {
        if ((Z1 == Zone1) || (Z2 == Zone1))
            bValid = TRUE;
    }
    if (Zone2 != 0)
    {
        if ((Z1 == Zone2) || (Z2 == Zone2))
            bValid = TRUE;
    }                

    return bValid;
}
//=========================================================================

void alien_orb_spawner::HandleOffensive( f32 DeltaTime )
{
    m_AttackCountdownTimer -= DeltaTime;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_AttackGuid );
    if (NULL == pObj)
    {
        EnterAOSState( AOS_STATE_DEFENSIVE );
        return;
    }

    //vector3 Delta = pObj->GetPosition() - GetPosition();
    //f32     Dist = Delta.Length();

    if (/*(Dist <= m_AttackDistance) ||*/ (m_AttackCountdownTimer <= 0.0f))
    {
        EnterAOSState( AOS_STATE_FIRING );
    }
    else
    {        
        // Handle audio
        f32 TimeT = 1-(m_AttackCountdownTimer / m_AttackMaxTime);

        f32 Pitch = 1.0f + 0.5f * TimeT;

        g_AudioMgr.SetPitch( m_ChargeupVoiceID, Pitch );
    }
}

//=========================================================================

void alien_orb_spawner::HandleFiring( f32 DeltaTime )
{
    m_AttackCountdownTimer -= DeltaTime;

    object* pObj = g_ObjMgr.GetObjectByGuid(m_AttackParticleGuid);
    if ((NULL == pObj) || (m_AttackCountdownTimer <= 0))
    {
        EnterAOSState( AOS_STATE_DEFENSIVE );       
    }
    else
    {        
        TransmitPain( pObj->GetBBox() );        
    }    
}

//=========================================================================

void alien_orb_spawner::MoveDependants( void )
{
    if (m_AttackChargeParticleGuid)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid(m_AttackChargeParticleGuid);
        if (pObj)
        {
            pObj->OnMove( GetPosition() );
        }
    }
    if (m_ChargeupVoiceID)
    {
        g_AudioMgr.SetPosition( m_ChargeupVoiceID, GetPosition(), GetZone1() );
    }
}

//=========================================================================

xbool alien_orb_spawner::AreValidTargets( void )
{
    xbool bValidTargets = FALSE;
    if (m_SpawnTubeLimitVolume != NULL_GUID)
    {        
        object* pObj = g_ObjMgr.GetObjectByGuid( m_SpawnTubeLimitVolume );
        if (pObj)
        {
            if (alien_orb::SelectTargetInVolume( object::TYPE_ALIEN_SPAWN_TUBE, pObj->GetBBox(), GetGuid(), m_HostGroup ))
            {
                bValidTargets = TRUE;
            }
        }
    }

    if ((m_TurretLimitVolume != NULL_GUID) && (!bValidTargets))
    {        
        object* pObj = g_ObjMgr.GetObjectByGuid( m_TurretLimitVolume );
        if (pObj)
        {
            if (alien_orb::SelectTargetInVolume( object::TYPE_TURRET, pObj->GetBBox(), GetGuid(), m_HostGroup ))
            {
                bValidTargets = TRUE;
            }
        }
    }

    // Bail if there are no targets around
    return bValidTargets;
}

#ifdef X_EDITOR

s32 alien_orb_spawner::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = object::OnValidateProperties( ErrorMsg );

    if (NULL_GUID != m_CoverGroup)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid(m_CoverGroup);
        if (NULL == pObj)
        {
            nErrors++;
            ErrorMsg += "ERROR: Cover Group guid for Alien Orb Spawner refers to nonexistant object [";
            ErrorMsg += guid_ToString( m_CoverGroup );
            ErrorMsg += "]\n";
        }
        else
        {
            if (!pObj->IsKindOf( group::GetRTTI() ))
            {
                nErrors++;
                ErrorMsg += "ERROR: Cover Group guid for Alien Orb Spawner refers to object that is not a group [";
                ErrorMsg += guid_ToString( m_CoverGroup );
                ErrorMsg += "]\n";
            }
        }
    }


    // check host group
    if (NULL_GUID != m_HostGroup)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid(m_HostGroup);
        if (NULL == pObj)
        {
            nErrors++;
            ErrorMsg += "ERROR: Host Group guid for Alien Orb Spawner refers to nonexistant object [";
            ErrorMsg += guid_ToString( m_CoverGroup );
            ErrorMsg += "]\n";
        }
        else
        {
            if (!pObj->IsKindOf( group::GetRTTI() ))
            {
                nErrors++;
                ErrorMsg += "ERROR: Host Group guid for Alien Orb Spawner refers to object that is not a group [";
                ErrorMsg += guid_ToString( m_CoverGroup );
                ErrorMsg += "]\n";
            }
        }
    }

    return nErrors;
}

#endif