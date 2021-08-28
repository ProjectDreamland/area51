//=============================================================================================
// SEEKER PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "ProjectileSeeker.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Entropy\e_Draw.hpp"
#include "audiomgr\audiomgr.hpp"
#include "render\LightMgr.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "Objects\actor\actor.hpp"
#include "Decals\DecalMgr.hpp"

//=============================================================================================
// DEFINES
//=============================================================================================
#define GRENADE_ELASTICITY          0.4f
#define MAX_BOUNCES_PER_ADVANCE     2

f32 seeker_projectile::s_SeekerGrenade_Alert_Time = 1.2f;

const   xcolor  g_SeekerGrenadeTracer( 49, 49, 250, 200 );
const   f32     g_FadeTime = 0.4f;
const f32       k_MinTimeBetweenBroadcasts = 0.5f;
//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct seeker_projectile_desc : public object_desc
{
    seeker_projectile_desc( void ) : object_desc( 
            object::TYPE_SEEKER_PROJECTILE , 
            "Seeker Projectile", 
            "WEAPON",
            object::ATTR_NEEDS_LOGIC_TIME   |
            object::ATTR_COLLIDABLE         | 
            object::ATTR_NO_RUNTIME_SAVE    |
            object::ATTR_RENDERABLE         | 
            object::ATTR_SPACIAL_ENTRY,
            FLAGS_IS_DYNAMIC | FLAGS_GENERIC_EDITOR_CREATE )
            {}

    virtual object* Create          ( void )
    {
        return new seeker_projectile;
    }

} s_Seeker_Projectile_Desc;

//=============================================================================================

const object_desc&  seeker_projectile::GetTypeDesc     ( void ) const
{
    return s_Seeker_Projectile_Desc;
}


//=============================================================================================
const object_desc&  seeker_projectile::GetObjectType   ( void )
{
    return s_Seeker_Projectile_Desc;
}

//=============================================================================================

seeker_projectile::seeker_projectile()
{
	m_Speed                 = 1600.f;
    m_AliveTime             = 0.0f;
    m_GravityAcceleration   = 0.0f;
	m_CollisionPoint		= vector3( 0.f , 0.f , 0.f );  
    m_NormalCollision       = vector3( 0.f , 0.f , 0.f );  
    m_Spin                  = radian3(x_frand(-R_45,R_45), 0.0f, x_frand(-R_30,R_30));
    m_TotalSpin             = radian3(1.0f, 1.0f, 1.0f);

	m_MaxAliveTime          = 10.0f;
    m_ParticleExplosion     = particle_emitter::GRAV_GRENADE_EXPLOSION;
    m_ExplosionRadius       = 800.0f;
    m_YawRate               = R_60;
    m_SeekerTarget          = 0;
    m_bBroadcastAlert       = TRUE;
    m_TimeSinceLastBroadcast = 0.0f;
}

//=============================================================================================

seeker_projectile::~seeker_projectile()
{
    if (m_EffectTrail) 
    {
        if (g_ObjMgr.GetObjectByGuid( m_EffectTrail ) != NULL)
            g_ObjMgr.DestroyObject( m_EffectTrail );
    }
    m_EffectTrail = NULL;
}

//=============================================================================================

void seeker_projectile::Initialize( const vector3&        InitPos,
                                  const radian3&        InitRot,
                                  const vector3&        InheritedVelocity,
                                        f32             Speed,
                                        guid            OwnerGuid,
                                        pain_handle     PainHandle,
                                        xbool           bHitLiving = TRUE )
{
    // Call base class
    base_projectile::Initialize(InitPos, InitRot, InheritedVelocity, Speed, OwnerGuid , PainHandle, bHitLiving ) ;

    vector3 Rotation( 0.0f, 0.0f, 0.0f );
    Rotation.Rotate( InitRot );

/*    
     = m_Velocity;    
    Rotation.Normalize();


    f32 Pitch, Yaw = 0.0f;
    Rotation.GetPitchYaw( Pitch, Yaw );
    radian3 Rot( Pitch, Yaw, 1.0f );


    matrix4 Mat;
    Mat.SetRotation( Rot );
    Mat.SetTranslation( InitPos );
*/
    //super fast, give it a blazing trail
    m_EffectTrail =  particle_emitter::CreatePresetParticleAndOrient(
        particle_emitter::GRAV_GRENADE_TRAIL, 
        Rotation, 
        InitPos);
    object *particleObject = g_ObjMgr.GetObjectByGuid( m_EffectTrail );
    if( particleObject && particleObject->IsKindOf(particle_emitter::GetRTTI()) )
    {
        particle_emitter &pEmitter = particle_emitter::GetSafeType( *particleObject );
        pEmitter.SetScale( 2.0f );
        pEmitter.SetColor( XCOLOR_GREEN );
    }
}

//=============================================================================

bbox seeker_projectile::GetLocalBBox( void ) const 
{ 
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        return( pRigidGeom->m_Collision.BBox );
    }
    
    return( bbox( vector3( 10.0f, 10.0f, 10.0f),
                  vector3(-10.0f,-10.0f,-10.0f) ) );
}

//=============================================================================

void seeker_projectile::UpdatePhysics( const f32& DeltaTime )
{
    f32 fVelocity = m_Velocity.Length();
    if (fVelocity == 0.0f)
    {
        return;
    }

    OnProcessCollision( DeltaTime );

    //update the spin
    f32 fTimeSpeed = DeltaTime * 30.0f;
    
    m_TotalSpin += radian3(m_Spin.Pitch * fTimeSpeed, m_Spin.Yaw * fTimeSpeed, m_Spin.Roll * fTimeSpeed);

    m_RenderL2W = GetL2W();
    m_RenderL2W.SetRotation(m_TotalSpin);

//    x_DebugMsg(xfs("GRENADE %g,%g,%g\n",m_RenderL2W.GetTranslation().X,m_RenderL2W.GetTranslation().Y,m_RenderL2W.GetTranslation().Z));
//    x_DebugMsg(xfs("OBJECT  %g,%g,%g\n",GetL2W().GetTranslation().X,GetL2W().GetTranslation().Y,GetL2W().GetTranslation().Z));
}

//=============================================================================

void seeker_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    m_AliveTime += DeltaTime;
    m_TimeSinceLastBroadcast += DeltaTime;

    UpdateVelocity( DeltaTime );
    // every second give off a warning to those around me...
    if( m_TimeSinceLastBroadcast > k_MinTimeBetweenBroadcasts )
    {
        m_TimeSinceLastBroadcast = 0.0f;
        alert_package NotifyPackage ;
        NotifyPackage.m_Origin = GetGuid() ;
        NotifyPackage.m_Position = GetPosition();
        NotifyPackage.m_Type = alert_package::ALERT_TYPE_GRENADE ;
        NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC ;
        NotifyPackage.m_AlertRadius = m_Speed * 0.5f;
        NotifyPackage.m_Cause = GetGuid() ;
        NotifyPackage.m_FactionsSpecific = FACTION_NOT_SET;
        NotifyPackage.m_ZoneID = GetZone1();
        g_AudioManager.AppendAlertReceiver(NotifyPackage);
//        SMP_UTIL_Broadcast_Alert( NotifyPackage ) ;

        m_bBroadcastAlert = FALSE;
    }


    if (m_AliveTime > m_MaxAliveTime)
    {
        //blow this thing up...
        OnExplode();
        return;
    }
    else
    {
        if (m_bBroadcastAlert && (m_AliveTime > (m_MaxAliveTime - s_SeekerGrenade_Alert_Time )))
        {
            //give 1 second warning
            alert_package NotifyPackage ;
            NotifyPackage.m_Origin = GetGuid() ;
            NotifyPackage.m_Position = GetPosition();
            NotifyPackage.m_Type = alert_package::ALERT_TYPE_GRENADE ;
            NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC ;
            NotifyPackage.m_AlertRadius = m_ExplosionRadius * 1.1f ;
            NotifyPackage.m_Cause = GetGuid() ;
            NotifyPackage.m_FactionsSpecific = FACTION_NOT_SET;
            NotifyPackage.m_ZoneID = GetZone1();
            g_AudioManager.AppendAlertReceiver(NotifyPackage);
//            SMP_UTIL_Broadcast_Alert( NotifyPackage ) ;

            m_bBroadcastAlert = FALSE;
        }

        UpdatePhysics( DeltaTime );
        if( !(GetAttrBits() & ATTR_DESTROY) )
        {        
            OnMove( m_CurrentPosition );
        }
    }
}

//=============================================================================

void seeker_projectile::OnColCheck( void )
{
    object::OnColCheck();
}

//=============================================================================

void seeker_projectile::OnPain( const pain& Pain )
{
    (void)Pain;
    OnExplode();
}

//=============================================================================

void seeker_projectile::OnExplode( void )
{
    //add a light where the explosion occurred
    g_LightMgr.AddFadingLight( GetPosition(), xcolor(152, 152, 76, 255), 300.0f, 1.0f, 1.8f );

    if (m_EffectTrail) 
    {
        if (g_ObjMgr.GetObjectByGuid( m_EffectTrail ) != NULL)
        {
            g_ObjMgr.DestroyObject( m_EffectTrail );
            m_EffectTrail = NULL;
        }
    }

    //render particle explosion
    guid explodeGuid = particle_emitter::CreatePresetParticleAndOrient( 
            m_ParticleExplosion, 
            vector3(0.0f,0.0f,0.0f), 
            GetPosition() );

    object *particleObject = g_ObjMgr.GetObjectByGuid( explodeGuid );
    if( particleObject && particleObject->IsKindOf(particle_emitter::GetRTTI()) )
    {
        particle_emitter &pEmitter = particle_emitter::GetSafeType( *particleObject );
        pEmitter.SetScale( 0.2f );
    }

    //g_AudioManager.Play( "Grenade_Explosion", GetPosition() );

    g_AudioManager.Play( "GravCharge_Explosion", audio_manager::EXPLOSION, GetPosition(), 
        GetZone1(), GetGuid() );

    // create a decal
    decal_package* pPackage = m_hDecalPackage.GetPointer();
    if ( pPackage )
    {
        decal_definition* pDef = pPackage->GetDecalDef( "CharMarks", 0 );
        if( pDef )
        {
	        //only paste a decal if the object is at the last collision
            // point (within radius + 20 centimeters)
            f32 Size        = x_frand( 150.0f, 200.0f );
            f32 fDistToTest = GetBBox().GetRadiusSquared() + (20.0f * 20.0f);
            if ( (m_CollisionPoint - GetBBox().GetCenter()).LengthSquared() < fDistToTest )
            {
                g_DecalMgr.CreateDecalAtPoint( *pDef,
                                               m_CollisionPoint,
                                               m_NormalCollision,
                                               vector2( Size, Size ),
                                               pDef->RandomRoll() );
            }
        }
    }

    pain Pain;
    Pain.Setup( m_PainHandle, m_OwnerGuid, GetPosition() );
    Pain.SetDirection( m_Velocity );
    Pain.ApplyToWorld();

    alert_package NotifyPackage ;
    NotifyPackage.m_Origin = GetGuid() ;
    NotifyPackage.m_Position = GetPosition() ;
    NotifyPackage.m_Type = alert_package::ALERT_TYPE_EXPLOSION ;
    NotifyPackage.m_Target = alert_package::ALERT_TARGET_NPC ;
    NotifyPackage.m_AlertRadius = m_ExplosionRadius * 2.f ;
    NotifyPackage.m_Cause = NULL ;
    NotifyPackage.m_FactionsSpecific = FACTION_NOT_SET;
    NotifyPackage.m_ZoneID = GetZone1();
    g_AudioManager.AppendAlertReceiver(NotifyPackage);
//    SMP_UTIL_Broadcast_Alert( NotifyPackage ) ;
    
    //destroy the grenade
    g_ObjMgr.DestroyObject( GetGuid() );   

}

//=============================================================================

xbool seeker_projectile::OnProcessCollision( const f32& DeltaTime )
{
    vector3	DesiredPos	= m_CurrentPosition + m_Velocity * DeltaTime;
    
    // Fire up sphere collision 
    g_CollisionMgr.RaySetup( GetGuid(), 
							 GetBBox().GetCenter(),
                             GetBBox().GetCenter() + m_Velocity * DeltaTime );
    g_CollisionMgr.AddToIgnoreList( m_OwnerGuid );
    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE );
    
    //no collisions, move the object.
    if( g_CollisionMgr.m_nCollisions == 0 )
    {
        g_TracerMgr.AddTracer( tracer_mgr::TRACER_TYPE_BULLET, g_FadeTime, m_CurrentPosition, DesiredPos, g_SeekerGrenadeTracer );
        m_CurrentPosition = DesiredPos;
        return TRUE;
    }

    //process hit event...
    OnExplode( );

    return FALSE;
}

//=========================================================================

void seeker_projectile::OnMove( const vector3& NewPos )
{
    object::OnMove(NewPos);

    if (m_EffectTrail)
    {
        object* pEffectObject  = g_ObjMgr.GetObjectByGuid( m_EffectTrail );
        
        if ( pEffectObject != NULL )
            pEffectObject->OnMove( NewPos );
        else
            m_EffectTrail = NULL;
    }
}

//=========================================================================

void seeker_projectile::OnRender( void )
{
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();
    
    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
        
        m_RigidInst.Render( &GetL2W(), Flags );
    }
    else
    {
        draw_BBox( GetBBox(), XCOLOR_RED );
    }

    base_projectile::OnRender();

#ifdef X_EDITOR
//    draw_Line( GetPosition() , GetPosition() + m_NormalCollision );
//    draw_Line( GetPosition() , GetPosition() + m_Velocity , XCOLOR_BLUE );
#endif // X_EDITOR
}

//=========================================================================

void seeker_projectile::UpdateVelocity( f32 DeltaTime )
{
    // get the vector to target.
    object *targetObject = g_ObjMgr.GetObjectByGuid( m_SeekerTarget );
    if( targetObject )
    {
        vector3 targetPosition = targetObject->GetPosition();
        if( targetObject && targetObject->IsKindOf(actor::GetRTTI()) )
        {
            actor &actorSource = actor::GetSafeType( *targetObject );
            targetPosition = actorSource.GetPositionWithOffset( actor::OFFSET_CENTER );
        }

        vector3 toTarget = targetPosition - GetPosition();
        toTarget.Normalize();
        vector3 ourVelocity = m_Velocity;
        ourVelocity.Normalize();       

        vector3 rotAngle = toTarget - ourVelocity;
        f32 rotAmount = m_YawRate * DeltaTime;
        if( rotAngle.LengthSquared() < rotAmount * rotAmount )
        {
            m_Velocity = toTarget;
            m_Velocity.NormalizeAndScale(m_Speed);
        }
        else
        {
            rotAngle.NormalizeAndScale(rotAmount);
            ourVelocity+=rotAngle;
            m_Velocity = ourVelocity;
            m_Velocity.NormalizeAndScale(m_Speed);
        }
    }
}
//=============================================================================================

xbool seeker_projectile::LoadInstance( const char* pFileName )
{
	// Initialize the skin
	//m_RigidInst.OnProperty( g_PropQuery.WQueryExternal( "RenderInst\\File", pFileName ) );
    m_RigidInst.SetUpRigidGeom( pFileName );
	return TRUE;

}

//=============================================================================

