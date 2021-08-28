
#include "AlienOrb.hpp"
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
#include "Turret.hpp"
#include "AlienSpawnTube.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "GameLib\RenderContext.hpp"
#include "Objects\Group.hpp"


//
//  ORB_CHOOSE_NEAREST_HOST -- Undefine this to make the orb choose randomly
//                             from all valid hosts
//
//#define ORB_CHOOSE_NEAREST_HOST 


#if (defined shird) && (defined X_EDITOR) && (defined X_DEBUG)
#define ORB_LOGGING_ENABLED
#endif

//=============================================================================
// CONSTANTS
//=============================================================================
static const f32 k_ORB_ARRIVE_AT_DISTANCE               = 11.0f;
static const s32 k_MAX_ORB_TARGETS                      = 16;
static const f32 k_OFFSET_FROM_TARGET_ORIGIN            = -125;
//=============================================================================
// SHARED
//=============================================================================
static guid     g_OrbTargetValidList    [ k_MAX_ORB_TARGETS ];
static guid     g_OrbTargetPotentialList[ k_MAX_ORB_TARGETS ];

//=============================================================================
// OBJECT DESCRIPTION
//=============================================================================

//=============================================================================

static struct alien_orb_desc : public object_desc
{
    alien_orb_desc( void ) : object_desc( 
        object::TYPE_ALIEN_ORB, 
        "Alien Orb", 
        "AI",
            object::ATTR_RENDERABLE       |
            object::ATTR_NEEDS_LOGIC_TIME,

            FLAGS_GENERIC_EDITOR_CREATE | 
            FLAGS_IS_DYNAMIC            |
            FLAGS_NO_ICON               |
            FLAGS_BURN_VERTEX_LIGHTING ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new alien_orb; }

} s_AlienOrb_Desc;

//=============================================================================

alien_orb::alien_orb()
{
    m_Health                    = 10;
    m_State                     = STATE_UNKNOWN;
    m_AliveTime                 = 0.0f;
    m_TimeInState               = 0.0f;
    m_TargetGuid                = NULL_GUID;    
    m_MoveSpeed                 = 300;    
    m_SearchTime                = 2.0f;
    m_ScanCenterObject          = NULL_GUID;
    m_CollisionParticleScale    = 1.0f;
    m_WanderRadius              = 200.0f;

    m_TurretLimitVolume         = NULL_GUID;
    m_SpawnTubeLimitVolume      = NULL_GUID;
    m_HostGroup                 = NULL_GUID;

    m_bDestroyed                = FALSE;
    m_ActivateWhenDead          = NULL_GUID;
    m_ActivateWhenArrived       = NULL_GUID;

    m_NormalParticleScale       = 1;
    m_NormalParticleGuid        = NULL_GUID;
    m_MutantParticleScale       = 1;
    m_MutantParticleGuid        = NULL_GUID;
    m_DeathParticleScale        = 1;
    m_DestructionParticleScale  = 1;

    m_CreationSoundID           = -1;
    m_CollisionSoundID          = -1;
    m_FlyingSoundID             = -1;
    m_ScriptedFlyingSoundID     = -1;
    m_DeathSoundID              = -1;
    m_DestructionSoundID        = -1;
    m_FlyingSoundVoice          = -1;

    m_CollisionSphereRadius     = 15.0f;

    m_bLostTarget               = FALSE;
    m_bOrbActive                = TRUE;

    m_Spawner                   = NULL_GUID;

    m_AttackDamage              = 15;
    m_AttackForce               = 2;

    m_SpawnTubeProb             = 50;

    m_LastGeometryPosition.Set(0,0,0);
    m_CurrentGeometryPosition.Set(0,0,0);
    m_Velocity.Set(0,0,0);
    m_TargetPos.Set(0,0,0);

    m_OrbitRadius               = 0;
    m_OrbitOffset               = 0;
    m_OrbitVerticalAmt          = 0;
    m_PersonalOffset            = x_frand(0,R_360);

    m_ScriptedPosition          = NULL_GUID;
    m_NextInActivationChain     = NULL_GUID;

    m_bTrackedPositionsInvalid = FALSE;
}

//=============================================================================

alien_orb::~alien_orb()
{
    if (!m_bDestroyed)
        HandleCommonDeath();
}

//=============================================================================

const object_desc& alien_orb::GetTypeDesc( void ) const
{
    return s_AlienOrb_Desc;
}

//=============================================================================

const object_desc& alien_orb::GetObjectType( void )
{
    return s_AlienOrb_Desc;
}

//=============================================================================

void alien_orb::Launch( const vector3& InitialPos,
                        const vector3& DestPos )
{
    OnMove(InitialPos);
    m_TargetPos = DestPos;
    m_LastGeometryPosition = m_CurrentGeometryPosition = InitialPos;
    MoveGeomAndParticles();
    SwitchState( STATE_LAUNCH );
}

//=============================================================================

void alien_orb::OnColCheck( void )
{    
    /*
    if (m_bDestroyed)
        return;
    if (!m_bOrbActive)
        return;

    anim_surface::OnColCheck();   

    g_CollisionMgr.StartApply( GetGuid() );
    g_CollisionMgr.ApplySphere( GetPosition(), m_CollisionSphereRadius );
    g_CollisionMgr.EndApply();
    */
}

//=============================================================================

void alien_orb::OnPain ( const pain& Pain )   // Tells object to recieve pain
{
    (void)Pain;
    if( m_bDestroyed )
        return;
    if (!m_bOrbActive)
        return;
    
    //m_Health -= Pain.DamageR0;       
}

//=============================================================================

void alien_orb::OnEnumProp      ( prop_enum&    List )
{
    anim_surface::OnEnumProp( List );

    List.PropEnumHeader( "Orb Properties",           "Alien Orb Projectile Properties", 0 );
    List.PropEnumFloat ( "Orb Properties\\Health",           "Hit points", 0 );
    List.PropEnumFloat ( "Orb Properties\\Damage Amount",    "Amount of damage to do when passing through an object that the orb can harm", 0 );
    List.PropEnumFloat ( "Orb Properties\\Damage Force",     "Amount of force to impart when passing through an object that the orb can harm", 0 );
    List.PropEnumFloat ( "Orb Properties\\Collision Radius", "How large of a sphere should be used to detect collision from bullets, etc...", 0 );
    List.PropEnumGuid  ( "Orb Properties\\Limit SpawnTube Volume", "The specified object's bounding box defines the volum that spawn tubes have to be in to be considered valid targets", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Orb Properties\\Limit Turret Volume", "The specified object's bounding box defines the volum that turrets have to be in to be considered valid targets", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Orb Properties\\Host Group",       "The group containing all possible hosts for this orb", PROP_TYPE_EXPOSE );
    List.PropEnumInt   ( "Orb Properties\\SpawnTube Probability", "0-100% probability of choosing spawntubes over turrets when multiple hosts are available", 0 );
    List.PropEnumGuid  ( "Orb Properties\\Target Guid",      "What object is the orb moving to.", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Orb Properties\\Scripted Position", "If set, the orb will NOT think and will always be at this position.", PROP_TYPE_EXPOSE );
    List.PropEnumFloat ( "Orb Properties\\Orbit Radius",     "How far out to orbit the scripted position", PROP_TYPE_EXPOSE );
    List.PropEnumFloat ( "Orb Properties\\Orbit Vertical Amt",   "How much to bob up and down when orbiting the scripted position", PROP_TYPE_EXPOSE );
    List.PropEnumAngle ( "Orb Properties\\Orbit Offset",     "Offset around the circle when orbiting the scripted position (ie: 180 = half way around)", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Orb Properties\\Next In Chain",    "Set to the guid of another alien orb.  When this orb dies, it will set the scripted position of the other orb to NULL, and will activate it.", PROP_TYPE_EXPOSE );
    List.PropEnumFloat ( "Orb Properties\\Move Speed",       "Speed in m/s.", PROP_TYPE_EXPOSE );
    List.PropEnumFloat ( "Orb Properties\\Search Time",      "How long the orb should wait before starting to move to the new target", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Orb Properties\\Search Center",    "Scanning is done using this as the center of the sphere.  If it is unset, the orb is the center", PROP_TYPE_EXPOSE );
    List.PropEnumFloat ( "Orb Properties\\Wander Radius",    "How much does the orb wander when it is flying", PROP_TYPE_EXPOSE );
    List.PropEnumExternal( "Orb Properties\\Collision Particles",
                            "Resource\0fxo\0",
                            "Particle Resource for this item",
                            PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat ( "Orb Properties\\Collision Particle Scale", "Scale for the collision particle effect", 0 );

    List.PropEnumString  ( "Orb Properties\\Audio Package Name", "The audio package associated with this alien orb object.", PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT );
    List.PropEnumExternal( "Orb Properties\\Audio Package",      "Resource\0audiopkg\0", "The audio package associated with this alien orb object.", PROP_TYPE_DONT_SHOW );
    List.PropEnumExternal( "Orb Properties\\Creation Sound",     "Sound\0soundexternal\0","The sound to play when the orb is created", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Orb Properties\\Collision Sound",    "Sound\0soundexternal\0","The sound to play when the orb collides with something", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Orb Properties\\Flying Sound",       "Sound\0soundexternal\0","The sound to play while the orb is alive", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Orb Properties\\Scripted Flying Sound", "Sound\0soundexternal\0","The sound to play while the orb is alive and is using it's scripted position", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Orb Properties\\Death Sound",        "Sound\0soundexternal\0","The sound to play when the orb dies because there are no hosts around", PROP_TYPE_MUST_ENUM );
    List.PropEnumExternal( "Orb Properties\\Destruction Sound",  "Sound\0soundexternal\0","The sound to play when the orb is killed by pain", PROP_TYPE_MUST_ENUM );
    
    List.PropEnumExternal( "Orb Properties\\Normal Particles",
                            "Resource\0fxo\0",
                            "Particles visible from human mode",
                            PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat ( "Orb Properties\\Normal Particle Scale", "Scale for the normal particle effect", 0 );

    List.PropEnumExternal( "Orb Properties\\Mutant Particles",
                            "Resource\0fxo\0",
                            "Particles visible from mutant mode",
                            PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat ( "Orb Properties\\Mutant Particle Scale", "Scale for the mutant particle effect", 0 );

    List.PropEnumGuid  ( "Orb Properties\\Activate when dead",    "What to activate when the orb expires either through death or destruction", PROP_TYPE_EXPOSE );
    List.PropEnumGuid  ( "Orb Properties\\Activate on Arrive",    "What to activate when the orb arrives at it's target", PROP_TYPE_EXPOSE );

    List.PropEnumExternal( "Orb Properties\\Death Particles",
                            "Resource\0fxo\0",
                            "Particles to play when orb cannot find any hosts",
                            PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat ( "Orb Properties\\Death Particle Scale", "Scale for the death particle effect", 0 );

    List.PropEnumExternal( "Orb Properties\\Destruction Particles",
                            "Resource\0fxo\0",
                            "Particles to play when orb is destroyed by pain",
                            PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat ( "Orb Properties\\Destruction Particle Scale", "Scale for the destruction particle effect", 0 );

}

//=============================================================================

xbool alien_orb::OnProperty      ( prop_query&   I    )
{
    if (anim_surface::OnProperty( I ))
    {
        // do nothing
    }
    else if (I.VarFloat( "Orb Properties\\Orbit Radius", m_OrbitRadius ))
    {}
    else if (I.VarFloat( "Orb Properties\\Orbit Vertical Amt", m_OrbitVerticalAmt))
    {}
    else if (I.VarAngle( "Orb Properties\\Orbit Offset", m_OrbitOffset))
    {}
    else if (I.VarGUID ( "Orb Properties\\Host Group", m_HostGroup ))
    {}
    else if (I.VarGUID ( "Orb Properties\\Next In Chain", m_NextInActivationChain))
    {}
    else if (SMP_UTIL_IsAudioVar( I, "Orb Properties\\Scripted Flying Sound", m_hAudioPackage, m_ScriptedFlyingSoundID ))
    {}
    else if (I.IsVar("Orb Properties\\Scripted Position"))
    {
        if (I.IsRead())
        {
            I.SetVarGUID( m_ScriptedPosition );
        }
        else
        {
            SetScriptedPosition( I.GetVarGUID() );
        }        
    }
    else if (I.IsVar("Orb Properties\\SpawnTube Probability"))
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
    else if (I.VarFloat( "Orb Properties\\Health", m_Health, 1 ))
    {
        // do nothing
    }
    else if (I.VarFloat( "Orb Properties\\Damage Amount", m_AttackDamage ))
    {}
    else if (I.VarFloat( "Orb Properties\\Damage Force", m_AttackForce ))
    {}
    else if (I.VarFloat( "Orb Properties\\Collision Radius", m_CollisionSphereRadius, 0 ))
    {
        // do nothing
    }
    else if (I.VarGUID( "Orb Properties\\Limit SpawnTube Volume", m_SpawnTubeLimitVolume ))
    {
        // do nothing
    }
    else if (I.VarGUID( "Orb Properties\\Limit Turret Volume", m_TurretLimitVolume ))
    {
        // do nothing
    }
    else if (I.VarGUID( "Orb Properties\\Target Guid", m_TargetGuid ))
    {
        // do nothing
    }
    else if (I.VarGUID( "Orb Properties\\Activate when dead", m_ActivateWhenDead  ))
    {
        // do nothing
    }
    else if (I.VarGUID( "Orb Properties\\Activate on Arrive", m_ActivateWhenArrived  ))
    {
        // do nothing
    }
    else if (I.VarFloat( "Orb Properties\\Move Speed", m_MoveSpeed ))
    {
        // do nothing
    }
    else if (I.VarFloat( "Orb Properties\\Search Time", m_SearchTime ))
    {
        // do nothing
    }
    else if (I.VarGUID( "Orb Properties\\Search Center", m_ScanCenterObject ))
    {
        // do nothing
    }
    else if (I.VarFloat( "Orb Properties\\Wander Radius", m_WanderRadius ))
    {
        // do nothing
    }
    else if( I.IsVar( "Orb Properties\\Collision Particles" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hCollisionParticles.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hCollisionParticles.SetName( pString );                

                // Load the particle effect.
                if( m_hCollisionParticles.IsLoaded() == FALSE )
                    m_hCollisionParticles.GetPointer();
            }
        }
    }
    else if (I.VarFloat( "Orb Properties\\Collision Particle Scale", m_CollisionParticleScale ))
    {
        // do nothing
    }
    else if( I.IsVar( "Orb Properties\\Normal Particles" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hNormalParticles.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hNormalParticles.SetName( pString );                

                // Load the particle effect.
                if( m_hNormalParticles.IsLoaded() == FALSE )
                    m_hNormalParticles.GetPointer();
            }
        }
    }
    else if (I.VarFloat( "Orb Properties\\Normal Particle Scale", m_NormalParticleScale ))
    {
        // do nothing
    }
    else if( I.IsVar( "Orb Properties\\Mutant Particles" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hMutantVisionParticles.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hMutantVisionParticles.SetName( pString );                

                // Load the particle effect.
                if( m_hMutantVisionParticles.IsLoaded() == FALSE )
                    m_hMutantVisionParticles.GetPointer();
            }
        }
    }
    else if (I.VarFloat( "Orb Properties\\Mutant Particle Scale", m_MutantParticleScale ))
    {
        // do nothing
    }
    else if( I.IsVar( "Orb Properties\\Death Particles" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hDeathParticles.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hDeathParticles.SetName( pString );                

                // Load the particle effect.
                if( m_hDeathParticles.IsLoaded() == FALSE )
                    m_hDeathParticles.GetPointer();
            }
        }
    }
    else if (I.VarFloat( "Orb Properties\\Death Particle Scale", m_DeathParticleScale ))
    {
        // do nothing
    }
    else if( I.IsVar( "Orb Properties\\Destruction Particles" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hDestructionParticles.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                m_hDestructionParticles.SetName( pString );                

                // Load the particle effect.
                if( m_hDestructionParticles.IsLoaded() == FALSE )
                    m_hDestructionParticles.GetPointer();
            }
        }
    }
    else if (I.VarFloat( "Orb Properties\\Destruction Particle Scale", m_DestructionParticleScale ))
    {
        // do nothing
    }
    else if( I.IsVar( "Orb Properties\\Audio Package" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = I.GetVarExternal();

            if( pString[0] )
            {
                if( xstring(pString) == "<null>" )
                {
                    m_hAudioPackage.SetName( "" );
                }
                else
                {
                    m_hAudioPackage.SetName( pString );                

                    // Load the audio package.
                    if( m_hAudioPackage.IsLoaded() == FALSE )
                        m_hAudioPackage.GetPointer();
                }
            }
        }
    }
    else if( I.IsVar( "Orb Properties\\Audio Package Name" ) )
    {
        // You can only read.
        if( I.IsRead() )
        {
            I.SetVarString( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
    }
    else if (SMP_UTIL_IsAudioVar( I, "Orb Properties\\Creation Sound", m_hAudioPackage, m_CreationSoundID ))
    {}
    else if (SMP_UTIL_IsAudioVar( I, "Orb Properties\\Collision Sound", m_hAudioPackage, m_CollisionSoundID ))
    {}
    else if (SMP_UTIL_IsAudioVar( I, "Orb Properties\\Flying Sound", m_hAudioPackage, m_FlyingSoundID ))
    {}
    else if (SMP_UTIL_IsAudioVar( I, "Orb Properties\\Death Sound", m_hAudioPackage, m_DeathSoundID ))
    {}
    else if (SMP_UTIL_IsAudioVar( I, "Orb Properties\\Destruction Sound", m_hAudioPackage, m_DestructionSoundID ))
    {}
    else
    {   
        return FALSE;
    }

    return TRUE;
}

//=============================================================================

void alien_orb::SetDeath( void )
{
    HandleCommonDeath();

    const matrix4& L2W = GetL2W();

    guid FXGuid = particle_emitter::CreatePresetParticleAndOrient( m_hDeathParticles.GetName(), 
                                                        L2W.GetRotation(), 
                                                        L2W.GetTranslation(),
                                                        GetZone1() );

    if (m_DeathSoundID != -1)
    {
        const char* pSoundName = g_StringMgr.GetString( m_DeathSoundID );
        if (pSoundName)
        {
            g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );
        }
    }

    object* pObj = g_ObjMgr.GetObjectByGuid( FXGuid );
    if (pObj)
    {
        if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
        {
            particle_emitter& PE = particle_emitter::GetSafeType( *pObj );

            PE.SetScale( m_DeathParticleScale );
        }
    }
}

//=============================================================================

void alien_orb::HandleCommonDeath( void )
{
    m_bDestroyed = TRUE;
    if (m_ActivateWhenDead)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ActivateWhenDead );
        if (NULL != pObj)
        {
            pObj->OnActivate(TRUE);
        }
    }
    
    if (m_NormalParticleGuid)
        g_ObjMgr.DestroyObject( m_NormalParticleGuid );
    m_NormalParticleGuid = NULL_GUID;

    if (m_MutantParticleGuid)
        g_ObjMgr.DestroyObject( m_MutantParticleGuid );
    m_MutantParticleGuid = NULL_GUID;

    StopMoving();

    // Handle activation chaining
    if (NULL_GUID != m_NextInActivationChain)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_NextInActivationChain );
        if (NULL != pObj)
        {
            if (pObj->IsKindOf( alien_orb::GetRTTI() ))
            {
                alien_orb& Orb = alien_orb::GetSafeType( *pObj );

                Orb.SetScriptedPosition( NULL_GUID );
                Orb.OnActivate( TRUE );
            }
        }
    }

    // Kill the orb            
    g_ObjMgr.DestroyObject( GetGuid() );
}

void alien_orb::SetDestroyed( void )
{    
    HandleCommonDeath();

    const matrix4& L2W = GetL2W();

    guid FXGuid = particle_emitter::CreatePresetParticleAndOrient( m_hDestructionParticles.GetName(), 
                                                        L2W.GetRotation(), 
                                                        L2W.GetTranslation(),
                                                        GetZone1() );

    if (m_DestructionSoundID != -1)
    {
        const char* pSoundName = g_StringMgr.GetString( m_DestructionSoundID );
        if (pSoundName)
        {
            g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );
        }
    }

    object* pObj = g_ObjMgr.GetObjectByGuid( FXGuid );
    if (pObj)
    {
        if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
        {
            particle_emitter& PE = particle_emitter::GetSafeType( *pObj );

            PE.SetScale( m_DestructionParticleScale );
        }
    }
}

//=============================================================================

void alien_orb::OnAdvanceLogic  ( f32 DeltaTime )
{
    if (m_Health <= 0)
    {
        if (!m_bDestroyed)
            SetDestroyed();

        return;
    }

    anim_surface::OnAdvanceLogic( DeltaTime );

    const matrix4& L2W = GetL2W();

    // Create the normal effect if it doesn't already exist
    if (NULL_GUID == m_NormalParticleGuid)
    {
#ifdef ORB_LOGGING_ENABLED
        LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "Creating normal particle effect" );
#endif
        // Create normal effect
        m_NormalParticleGuid = particle_emitter::CreatePresetParticleAndOrient( m_hNormalParticles.GetName(), 
            L2W.GetRotation(), 
            L2W.GetTranslation(),
            GetZone1() );
        object* pObj = g_ObjMgr.GetObjectByGuid( m_NormalParticleGuid );
        if (pObj)
        {
            if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
            {
                particle_emitter& PE = particle_emitter::GetSafeType( *pObj );
                PE.SetScale( m_NormalParticleScale );
            }
        }
    }

    // Create the mutant effect if it doesn't already exist
    if (NULL_GUID == m_MutantParticleGuid)
    {
#ifdef ORB_LOGGING_ENABLED
        LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "Creating mutant particle effect" );
#endif
        // Create mutant effect
        m_MutantParticleGuid = particle_emitter::CreatePresetParticleAndOrient( m_hMutantVisionParticles.GetName(), 
            L2W.GetRotation(), 
            L2W.GetTranslation(),
            GetZone1() );
        object* pObj = g_ObjMgr.GetObjectByGuid( m_MutantParticleGuid );
        if (pObj)
        {
            if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
            {
                particle_emitter& PE = particle_emitter::GetSafeType( *pObj );
                PE.SetScale( m_MutantParticleScale );
            }
        }
    }

    // Handle normal startup
    if (m_State == STATE_UNKNOWN)
    {
        SwitchState( STATE_LOOK_FOR_TARGET );
        m_bTrackedPositionsInvalid = TRUE;        
    }
        
    if (m_bTrackedPositionsInvalid)
    {
        m_CurrentGeometryPosition = GetPosition();        
        m_LastGeometryPosition = m_CurrentGeometryPosition;                      

        m_bTrackedPositionsInvalid = FALSE;
    }

    m_TimeInState += DeltaTime;
    m_AliveTime   += DeltaTime;

    xbool bRunNormalLogic = TRUE;

    if (m_ScriptedPosition)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ScriptedPosition );

        if (pObj)
        {
            bRunNormalLogic = FALSE;

            if (m_FlyingSoundVoice == -1)
            {
                // Start up the audio
                StartMoving();
            }

            vector3 Pos = pObj->GetPosition();

            //
            //  Should these be tweaks?
            //
            f32 TimeScale = R_120;          // deg per second that the orb orbits the point
            f32 VerticalScale = R_1 * 300;  // bob

            // Figure out how to orbit this point
            vector3     Arm(0,0,m_OrbitRadius);
            Arm.RotateY( m_OrbitOffset + m_AliveTime * TimeScale );

            // Factor in up/down motion
            f32 VerticalOfs = x_sin( m_AliveTime * VerticalScale + m_PersonalOffset) * m_OrbitVerticalAmt;

            Pos += Arm;
            Pos.GetY() += VerticalOfs;

            m_LastGeometryPosition = m_CurrentGeometryPosition;
            m_CurrentGeometryPosition = Pos;
            OnMove( Pos );

            MoveGeomAndParticles();
        }      
        else
        {
            // We have a scripted position guid, but the object
            // cannot be found.  Let's free ourself.
            SetScriptedPosition( NULL_GUID );
            bRunNormalLogic = TRUE;
        }
    }
    
    if (bRunNormalLogic)
    {
        switch( m_State )
        {
        case STATE_LAUNCH:
            {
                f32 Dist = (GetL2W().GetTranslation() - m_TargetPos).Length();

                if (Dist <= k_ORB_ARRIVE_AT_DISTANCE)
                {
#ifdef ORB_LOGGING_ENABLED
                    LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "Logic from STATE_LAUNCH decided we're close enough" );
#endif
                    SwitchState( STATE_LOOK_FOR_TARGET );
                }

                LaunchToTarget( DeltaTime );
            }
            break;
        case STATE_LOOK_FOR_TARGET:
            {
                if (m_TargetGuid == NULL_GUID)
                {
                    // Look for something
#ifdef ORB_LOGGING_ENABLED
                    LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "Logic for LOOK_FOR_TARGET still searching" );
#endif
                    FindTarget();
                }
                if (m_TimeInState >= m_SearchTime)
                {
                    if (m_TargetGuid == NULL_GUID)
                    {
#ifdef ORB_LOGGING_ENABLED
                        LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "Logic for LOOK_FOR_TARGET time exceeded (%f of %f), no target found",m_TimeInState,m_SearchTime );
#endif
                        SwitchState( STATE_DIEING );
                    }
                    else
                    {
#ifdef ORB_LOGGING_ENABLED
                        LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "Logic for LOOK_FOR_TARGET time exceeded (%f of %f), target found",m_TimeInState,m_SearchTime );
#endif
                        SwitchState( STATE_MOVE_TO_TARGET );
                    }
                }
            }
            break;
        case STATE_MOVE_TO_TARGET:
            {
                if ( !IsTargetValid() )
                {
#ifdef ORB_LOGGING_ENABLED
                    LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "Logic for MOVE_TO_TARGET target no longer valid" );
#endif
                    m_bLostTarget = TRUE;
                    m_TargetGuid  = NULL_GUID;
                    SwitchState( STATE_LOOK_FOR_TARGET );
                }
                else
                {
                    f32 Dist = GetDistToTarget();

                    if (Dist <= k_ORB_ARRIVE_AT_DISTANCE)
                    {
#ifdef ORB_LOGGING_ENABLED
                        LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "Logic for MOVE_TO_TARGET decided we're close enough" );
#endif
                        SwitchState( STATE_AT_TARGET );
                    }

                    MoveToTarget( DeltaTime );
                }
            }
            break;
        case STATE_AT_TARGET:
            break;        
        case STATE_DIEING:
            break;
        case STATE_SCRIPTED_MOTION:
            break;
        }
    }
    
    //==-------------------------------
    //  Collisions
    //==-------------------------------
    /*
    if (m_bOrbActive)
    {
        
        collision_mgr::collision    Forward;
        collision_mgr::collision    Backward;
        xbool                       bForwardHit = FALSE, bBackwardHit = FALSE;

        // Handle collision forward        
        g_CollisionMgr.RaySetup( GetGuid(), m_LastGeometryPosition, m_CurrentGeometryPosition );        
        g_CollisionMgr.AddToIgnoreList( m_Spawner );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER | object::ATTR_LIVING, object::ATTR_COLLISION_PERMEABLE);
        if (g_CollisionMgr.m_nCollisions > 0)
        {
            Forward = g_CollisionMgr.m_Collisions[0];
            bForwardHit = TRUE;
        }


        // Handle collision Backward        
        g_CollisionMgr.SphereSetup( GetGuid(), m_CurrentGeometryPosition, m_LastGeometryPosition, m_CollisionSphereRadius );        
        g_CollisionMgr.AddToIgnoreList( m_Spawner );
        g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_BLOCKS_CHARACTER | object::ATTR_LIVING, object::ATTR_COLLISION_PERMEABLE);
        if (g_CollisionMgr.m_nCollisions > 0)
        {
            Backward = g_CollisionMgr.m_Collisions[0];
            bBackwardHit = TRUE;
        }

        // Make sure we don't hit twice in the same spot
        if (bForwardHit && bBackwardHit)
        {
            if ((Forward.ObjectHitGuid == Backward.ObjectHitGuid) &&
                (Forward.Point - Backward.Point).Length() < 0.5f)
            {
                // If it's the same object and within half a centimeter, ignore the backward hit
                bBackwardHit = FALSE;
            }
        }

        if (bForwardHit)
        {
            DoCollisionHit( Forward );
        }

        if (bBackwardHit)
        {
            DoCollisionHit( Backward );
        }

    }
    */

    HandleRenderableStates();
}

//=============================================================================

void alien_orb::DoCollisionHit( const collision_mgr::collision& C )
{
    //add a light where the hit occurred
    g_LightMgr.AddFadingLight( C.Point, xcolor(152, 152, 76, 255), 200.0f, 1.0f, 1.8f );

    radian3 Orient;
    Orient.Zero();
    vector3 Normal = C.Plane.Normal;
    Normal *= -1;
    Normal.GetPitchYaw( Orient.Pitch, Orient.Yaw );
    Orient.Roll = x_frand( 0,R_360 );

    guid FXGuid = particle_emitter::CreatePresetParticleAndOrient( m_hCollisionParticles.GetName(), 
                                                        Orient, 
                                                        C.Point,
                                                        GetZone1() );

    object* pObj = g_ObjMgr.GetObjectByGuid( FXGuid );
    if (pObj)
    {
        if (pObj->IsKindOf( particle_emitter::GetRTTI() ))
        {
            particle_emitter& PE = particle_emitter::GetSafeType( *pObj );

            PE.SetScale( m_CollisionParticleScale );
        }
    }


    if (m_CollisionSoundID != -1)
    {
        const char* pSoundName = g_StringMgr.GetString( m_CollisionSoundID );
        if (pSoundName)
        {
            g_AudioMgr.PlayVolumeClipped( pSoundName, C.Point, GetZone1(), TRUE );
        }
    }

    if (C.ObjectHitGuid == m_TargetGuid)
    {
        // We've collided with our target
        SwitchState( STATE_AT_TARGET );
    }

    // Harm the player
    pObj = g_ObjMgr.GetObjectByGuid( C.ObjectHitGuid );
    if (pObj)
    {
        if (pObj->GetType() == object::TYPE_PLAYER)
        {
            pain Pain;
            Pain.Setup( "GENERIC_1", GetGuid(), GetPosition() );
            Pain.SetDirectHitGuid( C.ObjectHitGuid );
            Pain.SetDirection( m_Velocity );
            Pain.ApplyToObject( pObj );
        }
    }
}

//=============================================================================

void alien_orb::SwitchState( state NewState )
{
//    state OldState = m_State;

#ifdef ORB_LOGGING_ENABLED
    LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "SwitchState from %s to %s",GetStateName(m_State), GetStateName(NewState) );
#endif

    m_State = NewState;

    m_TimeInState = 0;

    switch( NewState )
    {
    case STATE_LAUNCH:
        {
            StartMoving();
        }
        break;
    case STATE_LOOK_FOR_TARGET:
        {            
            if (m_bLostTarget)
            {
                // Move the actual orb object position to the 
                // current location of the geometry.
                // This gets rid of any popping that would occur
                // if the orb was travelling to a target (wander > 0)
                // and suddenly went looking again (which would reset wander sharply to 0)
                OnMove( m_CurrentGeometryPosition );

                m_bLostTarget = FALSE;
            }

            StopMoving();

            FindTarget();
        }
        break;
    case STATE_MOVE_TO_TARGET:
        {
            StartMoving();
        }
        break;
    case STATE_AT_TARGET:
        {
            StopMoving();

            if (m_ActivateWhenArrived)
            {
                object* pObj = g_ObjMgr.GetObjectByGuid( m_ActivateWhenArrived );
                if (NULL != pObj)
                {
                    pObj->OnActivate(TRUE);
                }
            }

            // We have arrived!
            // Notify the target host   
            if (!EnterTargetObject())
            {                
                // Clear the target now that we have arrived
                m_TargetGuid = NULL_GUID;

                SwitchState( STATE_LOOK_FOR_TARGET );
            }
            else
            {
                // We're in.
                // Turn the orb off.
                // The target will be responsible for turning it back on
                DeactivateOrb();
                HandleRenderableStates();

                // Clear the target now that we have arrived
                m_TargetGuid = NULL_GUID;
            }            
        }
        break;
    case STATE_DIEING:
        {            
            SetDeath();
        }
    case STATE_SCRIPTED_MOTION:
        {
            StartMoving();
        }
        break;
    }
}

//=============================================================================

f32 alien_orb::GetDistToTarget ( void )
{
    if (m_TargetGuid == NULL_GUID)
        return 0;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if (NULL == pObj)
        return 0;

    vector3 TargetPos = pObj->GetPosition();
    vector3 MyPos     = GetPosition();

    if (pObj->IsKindOf( alien_spawn_tube::GetRTTI() ))
    {
        TargetPos += vector3(0,k_OFFSET_FROM_TARGET_ORIGIN,0);
    }

    vector3 Delta = TargetPos - MyPos;

    return Delta.Length();
}

//=============================================================================

xbool alien_orb::IsTargetValid   ( void )
{
    // If the target guid DOES NOT EXIST, it is invalid.
    // If the target guid IS NOT a valid target type, it is invalid.
    // If the target IS NOT reservable by me, it is invalid.
    // If the target IS destroyed, it is invalid
    if (m_TargetGuid == NULL_GUID)
        return FALSE;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if (NULL == pObj)
        return FALSE;

    if (pObj->IsKindOf( alien_spawn_tube::GetRTTI() ))
    {
        alien_spawn_tube& AST = alien_spawn_tube::GetSafeType( *pObj );  
        return AST.Reserve( GetGuid() );        
    }    
    else if (pObj->IsKindOf( turret::GetRTTI() ))
    {
        turret& T = turret::GetSafeType( *pObj );  
        if (T.IsDestroyed())
            return FALSE;
        return T.Reserve( GetGuid() );        
    }    

    return TRUE;
}

//=============================================================================

guid alien_orb::SelectTargetInVolume( object::type Type, const bbox& Volume, guid Guid, guid HostGroup )
{
    guid    RetVal              = NULL_GUID;
    s32     nTargets            = 0;
    s32     iClosestTarget      = -1;
    f32     ClosestTargetDist   = 1e30f;

    // Build the potential list
    s32     nPotentialHosts = 0;
    xbool   bUseHostGroup   = FALSE;

    if (NULL_GUID != HostGroup)
    {
        // Attempt to use the host group 
        // It will be treated as a list of potential hosts for the orb
        object* pObj = g_ObjMgr.GetObjectByGuid( HostGroup );
        if (NULL != pObj)
        {
            if (pObj->IsKindOf( group::GetRTTI() ))
            {
                bUseHostGroup = TRUE;
                
                group& HostGroup = group::GetSafeType( *pObj );

                s32 nHosts = HostGroup.GetNumChildren();

                s32 i;
                for (i=0;i<nHosts;i++)
                {   
                    guid gObj = HostGroup.GetChild(i);
                    if (NULL_GUID == gObj)
                        continue;
                    object* pObj = g_ObjMgr.GetObjectByGuid( gObj );
                    if (NULL == pObj)
                        continue;
                    if (pObj->GetType() == object::TYPE_ALIEN_SPAWN_TUBE)
                    {
                        g_OrbTargetPotentialList[ nPotentialHosts++ ] = gObj;
                    }
                    else if (pObj->GetType() == object::TYPE_TURRET)
                    {
                        object_ptr<turret> pTurret( pObj );
                        ASSERT(pTurret);
                        if (pTurret->RequiresOrb())
                            g_OrbTargetPotentialList[ nPotentialHosts++ ] = gObj;
                    }
                    if (nPotentialHosts >= k_MAX_ORB_TARGETS)
                        break;
                }            
            }
        }
    }

    if (!bUseHostGroup)
    {
        // The host group either wasn't set, or was invalid.  Let's do it the hard way    
        g_ObjMgr.SelectBBox( object::ATTR_ALL, Volume, Type );
        slot_id SlotID = g_ObjMgr.StartLoop();
        while ((SlotID != SLOT_NULL)&& (nPotentialHosts < k_MAX_ORB_TARGETS))
        {
            object* pObj = g_ObjMgr.GetObjectBySlot( SlotID );
            if (pObj)
            {
                if (pObj->GetType() == object::TYPE_ALIEN_SPAWN_TUBE)
                {
                    g_OrbTargetPotentialList[ nPotentialHosts++ ] = pObj->GetGuid();
                }
                else if  (pObj->GetType() == object::TYPE_TURRET )
                {
                    object_ptr<turret> pTurret( pObj );
                    ASSERT(pTurret);
                    if (pTurret->RequiresOrb())
                        g_OrbTargetPotentialList[ nPotentialHosts++ ] = pObj->GetGuid();
                }
            }
            SlotID = g_ObjMgr.GetNextResult( SlotID );
        }
        g_ObjMgr.EndLoop();
    }

    //
    // The potential host list has been built.
    // Not it is time to decide on valid targets
    //
    vector3 CallerPos(0,0,0);   

    object* pCaller = g_ObjMgr.GetObjectByGuid( Guid );
    if (pCaller)
    {
        CallerPos = pCaller->GetPosition();
    }
    /*
#ifdef ORB_LOGGING_ENABLED
    LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "FindTarget found %d possible hosts",nPotentialHosts);
#endif
*/
    s32 i;
    for (i=0;i<nPotentialHosts;i++)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( g_OrbTargetPotentialList[ i ] );
        if (pObj)
        {
            f32 Dist = 0;
            
            // If the caller specified it's guid, factor in distance
            if (Guid)
            {
                Dist = (pObj->GetPosition() - CallerPos).Length();
            }

            if (pObj->IsKindOf( alien_spawn_tube::GetRTTI()))
            {
                alien_spawn_tube& AST = alien_spawn_tube::GetSafeType( *pObj );                                                

                if (AST.CanBeReservedByMe( Guid ))
                {        
                    g_OrbTargetValidList[ nTargets ] = g_OrbTargetPotentialList[ i ];
                    if (Dist < ClosestTargetDist)
                    {
                        ClosestTargetDist = Dist;
                        iClosestTarget    = nTargets;
                    }
                    nTargets++;                                                                                                            
                }
            }
            else if (pObj->IsKindOf( turret::GetRTTI()))
            {
                turret& T = turret::GetSafeType( *pObj );
                {
                    if (!T.IsDestroyed())
                    {
                        if (T.CanBeReservedByMe( Guid ))
                        {
                            g_OrbTargetValidList[ nTargets ] = g_OrbTargetPotentialList[ i ];
                            if (Dist < ClosestTargetDist)
                            {
                                ClosestTargetDist = Dist;
                                iClosestTarget    = nTargets;
                            }
                            nTargets++;                                
                        }
                    }
                }
            }                
        }        
    }  
    /*
#ifdef ORB_LOGGING_ENABLED
    LOG_MESSAGE( xfs("alien_orb %s",(const char*)guid_ToString(GetGuid())), "FindTarget: %d valid hosts were found",nTargets);
#endif
    */

    //
    //  The valid target list has been compiled, make a decision!
    //
    if (nTargets > 0)
    {
        s32 iTarget = 0;

#ifdef ORB_CHOOSE_NEAREST_HOST
        // Choose closest target
        iTarget = iClosestTarget;        
#else
        // Choose random target
        iTarget = x_irand(0,nTargets-1);
#endif  
        
        RetVal = g_OrbTargetValidList[ iTarget ];
    }

    return RetVal;
}

xbool alien_orb::FindTarget      ( void )
{
    if (IsTargetValid())
        return TRUE;

    UpdateZoneTrack();

    vector3 ScanCenter = GetPosition();
    if (m_ScanCenterObject != NULL_GUID)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_ScanCenterObject );
        if (NULL != pObj)
        {
            ScanCenter = pObj->GetPosition();
        }            
    }

    //
    //  Find Targets
    //
    guid TargetTube = NULL_GUID;
    guid TargetTurret = NULL_GUID;
    
    if (m_SpawnTubeLimitVolume != NULL_GUID)
    {        
        object* pObj = g_ObjMgr.GetObjectByGuid( m_SpawnTubeLimitVolume );
        if (pObj)
        {
            TargetTube = SelectTargetInVolume( object::TYPE_ALIEN_SPAWN_TUBE, pObj->GetBBox(), GetGuid(), m_HostGroup );
        }
    }

    if (m_TurretLimitVolume != NULL_GUID)
    {        
        object* pObj = g_ObjMgr.GetObjectByGuid( m_TurretLimitVolume );
        if (pObj)
        {
            TargetTurret = SelectTargetInVolume( object::TYPE_TURRET, pObj->GetBBox(), GetGuid(), m_HostGroup );
        }
    }
    
    m_TargetGuid = NULL_GUID;

    if ((TargetTube != NULL_GUID) && (TargetTurret != NULL_GUID))
    {
        u8 iRand = x_irand(0,100);
        
        if (iRand <= m_SpawnTubeProb)
        {
            m_TargetGuid = TargetTube;
        }
        else
        {
            m_TargetGuid = TargetTurret;
        }
    }
    else
    {
        if (TargetTube != NULL_GUID)
        {
            m_TargetGuid = TargetTube;
        }
        else
        {
            m_TargetGuid = TargetTurret;
        }
    }    
    
    // Reserve the target
    return ReserveTarget();
}

//=============================================================================

#ifndef X_RETAIL
#ifdef TARGET_EDITOR
void alien_orb::OnDebugRender( void )
{
    if (m_bDestroyed)
        return;

    vector3 Pos = GetPosition();
    vector3 ScanCenter = GetPosition();

    object* pObj = g_ObjMgr.GetObjectByGuid( m_ScanCenterObject );
    if (NULL != pObj)
    {
        ScanCenter = pObj->GetPosition();
    }

    draw_Label( ScanCenter, XCOLOR_WHITE, "Scan Center" );
    draw_Line( Pos, ScanCenter, XCOLOR_GREEN );
    draw_Label( Pos + vector3(0,50,0), XCOLOR_WHITE, xfs("State: %s",GetStateName()));
    
    
    pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if (NULL != pObj)
    {
        draw_Line( Pos, m_TargetPos, XCOLOR_RED );
        draw_Label( m_TargetPos, XCOLOR_WHITE, "Target" );
    }

    if (NULL_GUID != m_SpawnTubeLimitVolume)
    {
        pObj = g_ObjMgr.GetObjectByGuid( m_SpawnTubeLimitVolume );
        if (pObj)
        {
            bbox Box = pObj->GetBBox();

            draw_Line( Pos, Box.GetCenter(), XCOLOR_YELLOW );
            draw_BBox( Box, XCOLOR_YELLOW );
            draw_Label( Box.GetCenter(), XCOLOR_YELLOW, "SPAWN TUBE LIMIT VOLUME" );
        }
    }
    if (NULL_GUID != m_TurretLimitVolume)
    {
        pObj = g_ObjMgr.GetObjectByGuid( m_TurretLimitVolume );
        if (pObj)
        {
            bbox Box = pObj->GetBBox();

            draw_Line( Pos, Box.GetCenter(), XCOLOR_YELLOW );
            draw_BBox( Box, XCOLOR_YELLOW );
            draw_Label( Box.GetCenter(), XCOLOR_YELLOW, "TURRET LIMIT VOLUME" );
        }
    }
    if (NULL_GUID != m_ScriptedPosition)
    {
        pObj = g_ObjMgr.GetObjectByGuid( m_ScriptedPosition );
        if (pObj)
        {
            const matrix4& L2W = pObj->GetL2W();
            vector3 ObjPos = L2W.GetTranslation();

            draw_Line( Pos, ObjPos, XCOLOR_AQUA );
            draw_Label( ObjPos, XCOLOR_AQUA, "SCRIPTED POS" );

            if (m_OrbitRadius > 0)
            {
                draw_Cylinder( ObjPos, m_OrbitRadius, m_OrbitVerticalAmt, 16, xcolor(0,255,255,128), FALSE );
            }
        }
    }
    if (NULL_GUID != m_NextInActivationChain)
    {
        pObj = g_ObjMgr.GetObjectByGuid( m_NextInActivationChain );
        if (pObj)
        {
            const matrix4& L2W = pObj->GetL2W();
            vector3 ObjPos = L2W.GetTranslation();

            draw_Line( Pos, ObjPos, XCOLOR_YELLOW );
            draw_Sphere( ObjPos, 10, XCOLOR_YELLOW );
            draw_Label( ObjPos, XCOLOR_YELLOW, "NEXT IN CHAIN" );            
        }
    }
}
#endif // TARGET_PC
#endif // X_RETAIL

//=============================================================================

void alien_orb::LaunchToTarget  ( f32 DeltaTime )
{
    vector3 MyPos = GetPosition();

    vector3 Delta = m_TargetPos - MyPos;
    f32     Dist  = Delta.Length();

    if (Dist < 0.1f)
        return;

    vector3 MyMove;
    f32 MoveSpeed = m_MoveSpeed * 2;

    if (Dist < (MoveSpeed * DeltaTime))
    {
        MyMove = Delta;
    }
    else
    {
        MyMove = Delta;
        MyMove.NormalizeAndScale( MoveSpeed * DeltaTime );
    }

    // Move the orb
    OnMove( MyPos + MyMove );

    m_LastGeometryPosition = m_CurrentGeometryPosition;
    m_CurrentGeometryPosition = MyPos;

    MoveGeomAndParticles();
}

//=============================================================================

static const f32 k_SCALE_FROM_START_TIME = 1.0f;

void alien_orb::MoveToTarget( f32 DeltaTime )
{
    if (!IsTargetValid())
        return;

    if (m_bDestroyed)
        return;

    object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if (NULL == pObj)
        return;

    m_TargetPos = pObj->GetPosition();
    vector3 MyPos = GetPosition();

    if (pObj->IsKindOf( alien_spawn_tube::GetRTTI() ))
    {
        m_TargetPos += vector3(0,k_OFFSET_FROM_TARGET_ORIGIN,0);
    }

    vector3 Delta = m_TargetPos - MyPos;
    f32     Dist  = Delta.Length();

    if (Dist < 0.1f)
        return;

    vector3 MyMove;
   
    if (Dist < (m_MoveSpeed * DeltaTime))
    {
        MyMove = Delta;
    }
    else
    {
        MyMove = Delta;
        MyMove.NormalizeAndScale( m_MoveSpeed * DeltaTime );
    }

    // Move the orb
    OnMove( MyPos + MyMove );

    // Figure out where to render the orb
    f32     TimeToTarget = Dist / m_MoveSpeed;
    f32     WanderScale = 0;

    if ( ( m_TimeInState < k_SCALE_FROM_START_TIME ) &&
         (TimeToTarget < k_SCALE_FROM_START_TIME) )
    {
        f32 TotalTime = m_TimeInState + TimeToTarget;

        ASSERT( TotalTime != 0.0f );

        // Start position and target position are within k_SCALE_FROM_START_TIME
        // seconds of travel.  We need to average out.
        f32 A = m_TimeInState / TotalTime;
        f32 B = TimeToTarget / TotalTime;
        WanderScale = (A+B)/2;
    }
    else
    if ( m_TimeInState < k_SCALE_FROM_START_TIME )
    {
        // Leaving start position
        WanderScale = m_TimeInState / k_SCALE_FROM_START_TIME;
    }
    else if (TimeToTarget < k_SCALE_FROM_START_TIME)
    {   
        // Approaching target position
        WanderScale = TimeToTarget / k_SCALE_FROM_START_TIME;
    }
    else
    {
        // In transit
        WanderScale = 1.0f;
    }

    ASSERT( WanderScale >= 0);
    ASSERT( WanderScale <= 1);

    radian Yaw,Pitch;
    Delta.GetPitchYaw( Pitch, Yaw );

    f32 WanderAmount = m_WanderRadius * WanderScale;

    //WanderAmount = 0;

    vector3 Wander( x_sin( m_AliveTime*2.3f ) * WanderAmount, 
                    x_cos( m_AliveTime*3.7f) * WanderAmount * 0.75f,
                    x_sin( m_AliveTime*2.95f ) * WanderAmount * 0.5f );
    
    Wander.RotateX( Pitch );
    Wander.RotateY( Yaw );

    m_LastGeometryPosition = m_CurrentGeometryPosition;
    m_CurrentGeometryPosition = MyPos + Wander;

    MoveGeomAndParticles();
}

//=============================================================================

void alien_orb::OnRender( void )
{
    CONTEXT( "alien_orb::OnRender" );
    if (m_bDestroyed)
        return;
    if (!m_bOrbActive)
        return;

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();       

    if( pRigidGeom && (m_hAnimGroup.IsLoaded() == TRUE) )
    {
        // Compute bones
        const matrix4* pBone = GetBoneL2Ws() ;
        if (!pBone)
            return ;

        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

        m_Inst.Render( pBone, Flags | GetRenderMode() );
#if (0)
#if defined(shird) && defined(X_DEBUG)
        object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
        if (NULL != pObj)
        {
            vector3 TargetPos = pObj->GetPosition();
            vector3 MyPos = GetPosition();

            vector3 Delta = TargetPos - MyPos;
            f32     Dist  = Delta.Length();
            vector3 MyMove;                      

            // Figure out where to render the orb
            f32     TimeToTarget = Dist / m_MoveSpeed;
            f32     WanderScale = 0;

            draw_Label( MyPos + vector3(0,k_OFFSET_FROM_TARGET_ORIGIN,0), XCOLOR_WHITE, "TTT: %5.3f   Dist:%5.3f",TimeToTarget, Dist );
        }
#endif
#endif
    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif // X_EDITOR
    }
}

//=============================================================================

void alien_orb::OnTransform( const matrix4& L2W )
{
    anim_surface::OnTransform( L2W );

    // Handle being teleported
    m_LastGeometryPosition = m_CurrentGeometryPosition = L2W.GetTranslation();
}

//=============================================================================

const char* alien_orb::GetStateName( state State )
{
    switch( State )
    {
    case STATE_UNKNOWN:
        return "STATE_UNKNOWN";
    case STATE_LAUNCH:
        return "STATE_LAUNCH";
    case STATE_LOOK_FOR_TARGET:
        return "STATE_LOOK_FOR_TARGET";
    case STATE_MOVE_TO_TARGET:
        return "STATE_MOVE_TO_TARGET";
    case STATE_AT_TARGET:
        return "STATE_AT_TARGET";
    case STATE_DIEING:
        return "STATE_DIEING";
    }
    return "UNKNOWN_STATE";
}

const char* alien_orb::GetStateName( void )
{
    return GetStateName( m_State );
}

//=============================================================================

void alien_orb::DeactivateOrb( void )
{
    m_bOrbActive = FALSE;
    m_TargetGuid = NULL_GUID;
    StopMoving();
}

//=============================================================================

void alien_orb::ActivateOrb( void )
{    
    m_bOrbActive = TRUE;
}

//=============================================================================

void alien_orb::HandleRenderableStates( void )
{
    //
    //  Depending on what mode the render context is in
    //  toggle the appropriate particles on/off
    //
    xbool bShowNormal  = FALSE;
    xbool bShowMutated = FALSE;

    if (!m_bOrbActive)
    {
        // If the orb is inactive, we leave the effects off
        bShowNormal = FALSE;
        bShowMutated = FALSE;
    }
    else
    {
        if (g_RenderContext.m_bIsMutated)
        {
            bShowNormal = FALSE;
            bShowMutated = TRUE;
        }
        else
        {
            bShowNormal = TRUE;
            bShowMutated = FALSE;
        }
    }

    object* pObj = g_ObjMgr.GetObjectByGuid( m_NormalParticleGuid );
    if (pObj)
    {
        if (bShowNormal)
            pObj->SetAttrBits( pObj->GetAttrBits() | object::ATTR_RENDERABLE );            
        else
            pObj->SetAttrBits( pObj->GetAttrBits() & (~object::ATTR_RENDERABLE) );            
    }

    pObj = g_ObjMgr.GetObjectByGuid( m_MutantParticleGuid );
    if (pObj)
    {
        if (bShowMutated)
            pObj->SetAttrBits( pObj->GetAttrBits() | object::ATTR_RENDERABLE );            
        else
            pObj->SetAttrBits( pObj->GetAttrBits() & (~object::ATTR_RENDERABLE) );            
    }
}

//=============================================================================

void alien_orb::SetSpawner( guid Spawner )
{
    m_Spawner = Spawner;
}

//=============================================================================

void alien_orb::StartMoving( void )
{
    // Kill existing sound if it exists
    if (m_FlyingSoundVoice != -1)
        g_AudioMgr.Release( m_FlyingSoundVoice, 0.0f );

    // Find new sound to play
    const char* pSoundName = NULL;

    if ((NULL_GUID != m_ScriptedPosition) && (-1 != m_ScriptedFlyingSoundID))
        pSoundName = g_StringMgr.GetString( m_ScriptedFlyingSoundID );

    if ((NULL == pSoundName) && (-1 != m_FlyingSoundID))
        pSoundName = g_StringMgr.GetString( m_FlyingSoundID );

    // If we have a sound name, light it up
    if (pSoundName)
    {               
        const matrix4& L2W = GetL2W();
        m_FlyingSoundVoice = g_AudioMgr.PlayVolumeClipped( pSoundName, L2W.GetTranslation(), GetZone1(), TRUE );        
    }
}

//=============================================================================

void alien_orb::StopMoving( void )
{
    if (m_FlyingSoundID != -1)
    {
        if (m_FlyingSoundVoice != -1)
            g_AudioMgr.Release( m_FlyingSoundVoice,0 );

        m_FlyingSoundVoice = -1;
    }
}
//=============================================================================

xbool alien_orb::EnterTargetObject( void )
{
    object* pObj = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
    if (NULL == pObj)
        return FALSE;

    if (pObj->IsKindOf( turret::GetRTTI() ))
    {
        turret& T = turret::GetSafeType( *pObj );
        if (T.OrbEnter( GetGuid() ))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if (pObj->IsKindOf( alien_spawn_tube::GetRTTI()))
    {
        alien_spawn_tube& AST = alien_spawn_tube::GetSafeType( *pObj );
        if (AST.OrbEnter( GetGuid() ))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}

//=============================================================================

xbool alien_orb::ReserveTarget( void )
{
    if (NULL_GUID == m_TargetGuid)
        return FALSE;

    object* pObj = g_ObjMgr.GetObjectByGuid(m_TargetGuid);

    if (NULL == pObj)
        return FALSE;

    if (pObj->IsKindOf( turret::GetRTTI() ))
    {
        turret& T = turret::GetSafeType( *pObj );
        if (T.CanBeReservedByMe( GetGuid() ))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else if (pObj->IsKindOf( alien_spawn_tube::GetRTTI() ))
    {
        alien_spawn_tube& AST = alien_spawn_tube::GetSafeType( *pObj );
        if (AST.CanBeReservedByMe( GetGuid() ))
        {
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    return FALSE;
}

//=============================================================================

void alien_orb::MoveGeomAndParticles( void )
{
    //
    //  MOVE ANIM_SURFACE
    //
    matrix4 L2W = GetL2W();
    L2W.SetTranslation( m_CurrentGeometryPosition );
    m_AnimPlayer.SetL2W( L2W );        

    //
    //  MOVE PARTICLES
    //
    object* pObj = g_ObjMgr.GetObjectByGuid( m_NormalParticleGuid );
    if (pObj)
    {
        pObj->OnTransform( L2W );
    }
    pObj = g_ObjMgr.GetObjectByGuid( m_MutantParticleGuid );
    if (pObj)
    {
        pObj->OnTransform( L2W );
    }

    //
    //  MOVE SOUND
    //
    if (m_FlyingSoundVoice != -1)
    {
        g_AudioMgr.SetPosition( m_FlyingSoundVoice, L2W.GetTranslation(), GetZone1() );
    }

}

//=============================================================================

void alien_orb::SetTurretLimit  ( guid Volume )
{
    m_TurretLimitVolume = Volume;
}

//=============================================================================

void alien_orb::SetSpawnTubeLimit ( guid Volume )
{
    m_SpawnTubeLimitVolume = Volume;
}

//=============================================================================

void alien_orb::SetSpawnTubeProb( u8 Prob )
{
    m_SpawnTubeProb = MIN(100,Prob);
}

//=============================================================================

void alien_orb::KillOrb( void )
{
    HandleCommonDeath();
    g_ObjMgr.DestroyObject( GetGuid() );
}

//=============================================================================

void alien_orb::SetHostGroup( guid Group )
{
    m_HostGroup = Group;
}

//=============================================================================

void alien_orb::SetScriptedPosition( guid NewScriptedPosition )
{
    m_ScriptedPosition = NewScriptedPosition;
    m_TargetGuid       = NULL_GUID;

    if (NULL_GUID != NewScriptedPosition)
    {
        SwitchState( STATE_SCRIPTED_MOTION );
    }
    else
    {
        SwitchState( STATE_LOOK_FOR_TARGET );
    }

    m_bTrackedPositionsInvalid = TRUE;    
}

//=============================================================================

#ifdef X_EDITOR

s32 alien_orb::OnValidateProperties( xstring& ErrorMsg )
{
    // Make sure we call base class to get errors
    s32 nErrors = anim_surface::OnValidateProperties( ErrorMsg );

    if (NULL_GUID != m_HostGroup)
    {
        object* pObj = g_ObjMgr.GetObjectByGuid(m_HostGroup);
        if (NULL == pObj)
        {
            nErrors++;
            ErrorMsg += "ERROR: Host Group guid for Alien Orb refers to nonexistant object [";
            ErrorMsg += guid_ToString( m_HostGroup );
            ErrorMsg += "]\n";
        }
        else
        { 
            if (!pObj->IsKindOf( group::GetRTTI() ))
            {
                nErrors++;
                ErrorMsg += "ERROR: Host Group guid for Alien Orb refers to object that is not a group [";
                ErrorMsg += guid_ToString( m_HostGroup );
                ErrorMsg += "]\n";
            }
        }
    }

    return nErrors;
}

#endif
