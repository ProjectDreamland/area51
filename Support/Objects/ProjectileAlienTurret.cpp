#ifndef DISABLE_LEGACY_CODE
//=============================================================================================
// SEEKER PROJECTILE
//=============================================================================================

//=============================================================================================
// INCLUDES
//=============================================================================================
#include "ProjectileAlienTurret.hpp"
#include "objects\ParticleEmiter.hpp"
#include "Entropy\e_Draw.hpp"
#include "audiomgr\audiomgr.hpp"
#include "render\LightMgr.hpp"
#include "..\Support\Tracers\TracerMgr.hpp"
#include "Objects\actor\actor.hpp"
#include "Decals\DecalMgr.hpp"

//=============================================================================================
// STATICS & CONSTANTS
//=============================================================================================

f32 alien_turret_projectile::s_SeekerGrenade_Alert_Time = 1.2f;

const   xcolor  g_SeekerGrenadeTracer( 49, 49, 250, 200 );
const   f32     g_FadeTime = 0.4f;
const f32       k_MinTimeBetweenBroadcasts = 0.5f;


//=============================================================================================
// OBJECT DESC
//=============================================================================================
static struct alien_turret_projectile_desc : public object_desc
{
    alien_turret_projectile_desc( void ) : object_desc( 
            object::TYPE_ALIEN_TURRET_PROJECTILE , 
            "Alien Turret Projectile", 
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
        return new alien_turret_projectile;
    }

} s_Alien_Turret_Projectile_Desc;

//=============================================================================================

const object_desc&  alien_turret_projectile::GetTypeDesc     ( void ) const
{
    return s_Alien_Turret_Projectile_Desc;
}


//=============================================================================================
const object_desc&  alien_turret_projectile::GetObjectType   ( void )
{
    return s_Alien_Turret_Projectile_Desc;
}

//=============================================================================================

alien_turret_projectile::alien_turret_projectile()
{
	m_Speed                 = 800.f;
    m_AliveTime             = 0.0f;
    m_PainType              = pain::TYPE_GENERIC;
	m_DamageAmount          = 10.f;
	m_CollisionPoint		= vector3( 0.f , 0.f , 0.f );  
    m_NormalCollision       = vector3( 0.f , 0.f , 0.f );  
    
	m_MaxAliveTime          = 10.0f;
    m_ParticleExplosion     = particle_emitter::CAMERA_EXPLOSION;
    m_ExplosionRadius       = 200.0f;
    m_YawRate               = 0;
    m_MaxYawRate            = R_60;
    m_YawIncreaseRate       = R_40;
    m_LaunchConeSize        = R_20;
    m_Target                = 0;
    m_bBroadcastAlert       = TRUE;
    m_TimeSinceLastBroadcast = 0.0f;

    m_bFirstUpdate          = 1;

    m_bDeflecting           = FALSE;
    m_bAvoidObstacles       = FALSE;
}

//=============================================================================================

alien_turret_projectile::~alien_turret_projectile()
{
    if (m_EffectTrail) 
    {
        if (g_ObjMgr.GetObjectByGuid( m_EffectTrail ) != NULL)
            g_ObjMgr.DestroyObject( m_EffectTrail );
    }
    m_EffectTrail = NULL;
}

//=============================================================================================
void alien_turret_projectile::ReportProjectileCreation( void )
{
    //super fast, give it a blazing trail
    const matrix4& L2W = GetL2W();
    radian3 Rotation = L2W.GetRotation();
    vector3 Dir(0,0,1);
    Dir.Rotate(Rotation);
/*
    m_EffectTrail =  particle_emitter::CreatePresetParticleAndOrient(
        particle_emitter::GRAV_GRENADE_TRAIL, 
        Rotation, 
        Dir);   
*/
    m_Effect = particle_emitter::CreatePresetParticleAndOrient( "sh_photon.fxo", Dir, 
                                                            L2W.GetTranslation(), GetZone1() );
}

//=============================================================================

bbox alien_turret_projectile::GetLocalBBox( void ) const 
{ 
    /*
    rigid_geom* pRigidGeom = m_RigidInst.GetRigidGeom();

    if( pRigidGeom )
    {
        return( pRigidGeom->m_Collision.BBox );
    }
    */
    return( bbox( vector3( 10.0f, 10.0f, 10.0f),
                  vector3(-10.0f,-10.0f,-10.0f) ) );
}

//=============================================================================

void alien_turret_projectile::UpdatePhysics( const f32& DeltaTime )
{
    f32 fVelocity = m_Velocity.Length();
    if (fVelocity == 0.0f)
    {
        return;
    }

    if (m_bFirstUpdate)
    {
        
        m_bFirstUpdate = 0;
        
        vector3 Vel = m_Velocity;
        radian  P,Y;
        Vel.GetPitchYaw(P,Y);
        P += x_frand( -m_LaunchConeSize/2, m_LaunchConeSize/2 );
        Y += x_frand( -m_LaunchConeSize/2, m_LaunchConeSize/2 );

        Vel.Set(0,0,m_Velocity.Length());
        Vel.Rotate( radian3(P,Y,0) );
        m_Velocity = Vel;
    }
    
    OnProcessCollision( DeltaTime );
}

//=============================================================================

void alien_turret_projectile::OnAdvanceLogic( f32 DeltaTime )
{
    m_AliveTime += DeltaTime;
    m_TimeSinceLastBroadcast += DeltaTime;

    m_YawRate += m_YawIncreaseRate * DeltaTime;

    m_YawRate = MIN( m_YawRate, m_MaxYawRate );

    UpdateVelocity( DeltaTime );
    /*
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
    */

    if (m_AliveTime > m_MaxAliveTime)
    {
        //blow this thing up...
        OnExplode();
        return;
    }
    else
    {
        /*
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
        */

        UpdatePhysics( DeltaTime );
        if( !(GetAttrBits() & ATTR_DESTROY) )
        {        
            OnMove( m_CurrentPosition );
        }
    }
}

//=============================================================================

void alien_turret_projectile::OnColCheck( void )
{
    object::OnColCheck();
}

//=============================================================================

void alien_turret_projectile::OnPain( const pain& Pain )
{
    (void)Pain;
    OnExplode();
}

//=============================================================================

void alien_turret_projectile::OnExplode( void )
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
    if (m_Effect) 
    {
        if (g_ObjMgr.GetObjectByGuid( m_Effect ) != NULL)
        {
            g_ObjMgr.DestroyObject( m_Effect );
            m_Effect = NULL;
        }
    }

    //render particle explosion
    particle_emitter::CreatePresetParticleAndOrient( 
            m_ParticleExplosion, 
            vector3(0.0f,0.0f,0.0f), 
            GetPosition() );

    //g_AudioManager.Play( "Grenade_Explosion", GetPosition() );

    g_AudioManager.Play( "Grenade_Explosion", audio_manager::EXPLOSION, GetPosition(), 
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
            f32 Size        = x_frand( 20.0f, 80.0f );
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

    //create pain area
    bbox Pain(GetPosition(),m_ExplosionRadius);
    
    //collect all damagable objects in range (MAX 32)
    const s32 kMAX_CONTACTS = 128;
    slot_id idList[kMAX_CONTACTS];
    s32 contactCount = 1;

    g_ObjMgr.SelectBBox( ATTR_DAMAGEABLE , Pain ,object::TYPE_ALL_TYPES );    
    slot_id aID = g_ObjMgr.StartLoop();
    while( aID != SLOT_NULL )
    {
        if ( contactCount >= kMAX_CONTACTS )
            break ;

        idList[contactCount]= aID;
        ++contactCount;

        aID = g_ObjMgr.GetNextResult( aID );
    }
    g_ObjMgr.EndLoop();

    //now damage the collected objects
    while( --contactCount)
    {
        object* pObject = g_ObjMgr.GetObjectBySlot(idList[contactCount] );
        if (pObject)
        {
            f32 DamageMax = m_DamageAmount;
            f32 ForceMax = m_ForceAmount;

            //for living objects (player, npc's) we will add in cover protection
            if (pObject->GetAttrBits() & ATTR_LIVING || pObject->GetAttrBits() & ATTR_DAMAGEABLE )
            {
                //calc LOS, since we don't want as much force or damage if they are behind cover
                vector3 ObjPos = pObject->GetBBox().GetCenter();
                vector3 GrenadeColPos = GetPosition();
                GrenadeColPos.GetY() = GetBBox().Max.GetY();

                f32 nCoverAmount = 0;

                //ok, calc first at 25% height
                ObjPos.GetY() -= (pObject->GetBBox().GetRadius()/2);
                if (!g_ObjMgr.HasLOS(pObject->GetGuid(), ObjPos,GetGuid(), GrenadeColPos ) )
                {
                    //no line of sight, reduce impact
                    nCoverAmount += 1.0f;
                }

                //ok, calc second at 75% height
                ObjPos.GetY() += pObject->GetBBox().GetRadius();
                if (!g_ObjMgr.HasLOS( pObject->GetGuid(), ObjPos, GetGuid(), GrenadeColPos ) )
                {
                    //no line of sight, reduce impact
                    nCoverAmount += 1.0f;
                }

                //damage reduction of 1/3 per cover 
                DamageMax = DamageMax * (1.0f-(nCoverAmount/2.0f));
                //force reduction of 1/4 per cover (more concussion than damage)
                ForceMax = ForceMax * (1.0f-(nCoverAmount/3.0f));
            }

            pain PainEvent;
            PainEvent.Type      = (pain::type)m_PainType;
            PainEvent.Center    = GetPosition();
            PainEvent.Origin    = m_OwnerGuid ;
            PainEvent.PtOfImpact= pObject->GetBBox().GetCenter();
            PainEvent.Direction = m_Velocity;
            PainEvent.DamageR0  = DamageMax; 
            PainEvent.DamageR1  = 0; 
            PainEvent.ForceR0   = ForceMax*3;
            PainEvent.ForceR1   = 0;
            PainEvent.RadiusR0  = 0;
            PainEvent.RadiusR1  = m_ExplosionRadius;
            //PainEvent.Collision = g_CollisionMgr.m_Collisions[i];
            PainEvent.Direction.Normalize();
    
            pObject->OnPain( PainEvent ); 
        }
    }

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

xbool alien_turret_projectile::OnProcessCollision( const f32& DeltaTime )
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

void alien_turret_projectile::OnMove( const vector3& NewPos )
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
    if (m_Effect)
    {
        object* pEffectObject  = g_ObjMgr.GetObjectByGuid( m_Effect );
        
        if ( pEffectObject != NULL )
        {
            matrix4 L2W = GetL2W();

            L2W.ClearRotation();
            L2W.SetRotation( radian3(0,0,x_frand(0,2*PI)));

            //pEffectObject->OnMove( NewPos );
            pEffectObject->OnTransform( L2W );
            
        }
        else
            m_EffectTrail = NULL;
    }
}

//=========================================================================

void alien_turret_projectile::OnRender( void )
{
#ifdef X_EDITOR
    draw_Line( GetPosition(), m_TargetPos, XCOLOR_YELLOW );
    draw_Label( m_TargetPos, XCOLOR_YELLOW, "TARGET" );
#endif

    base_projectile::OnRender();

#ifdef X_EDITOR
//    draw_Line( GetPosition() , GetPosition() + m_NormalCollision );
//    draw_Line( GetPosition() , GetPosition() + m_Velocity , XCOLOR_BLUE );
#endif // X_EDITOR
}

//=========================================================================

void alien_turret_projectile::UpdateVelocity( f32 DeltaTime )
{
    // get the vector to target.
    object* pTargetObject = g_ObjMgr.GetObjectByGuid( m_Target );
    if( pTargetObject )
    {
        vector3 targetPosition;
        
        if (m_bDeflecting)
        {   
            targetPosition = m_LastValidTargetPos;
            f32 Dist = (targetPosition - m_CurrentPosition).Length();
            if (Dist < 20.0f)
                m_bDeflecting = FALSE;
        }
        
        if (!m_bDeflecting)
        {
            targetPosition = pTargetObject->GetPosition();
            if( pTargetObject && pTargetObject->IsKindOf(actor::GetRTTI()) )
            {
                actor &actorSource = actor::GetSafeType( *pTargetObject );
                targetPosition = actorSource.GetPositionWithOffset( actor::OFFSET_CENTER );
            }
        }

        

        f32 rotAmount = m_YawRate * DeltaTime;

        if (m_bAvoidObstacles)
        {
        
            rotAmount = R_360 * DeltaTime;

            //
            //  Check for obstacle
            //
            
            vector3 Forward = (targetPosition - m_CurrentPosition);
            Forward.NormalizeAndScale(m_Speed);
            Forward *= 0.25f;    // 1 second out
            g_CollisionMgr.RaySetup( GetGuid(), 
                m_CurrentPosition, m_CurrentPosition + Forward );
            g_CollisionMgr.AddToIgnoreList( m_OwnerGuid );
            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, object::ATTR_COLLIDABLE, object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );

            if (g_CollisionMgr.m_nCollisions > 0)
            {
                collision_mgr::collision& C = g_CollisionMgr.m_Collisions[ 0 ];

                // Reflect off of the collision point
                vector3 N = C.Plane.Normal;
                vector3 Dir = Forward;
                Dir.Normalize();
                vector3 Refl = (N+N)+Dir;
                Refl.NormalizeAndScale(200.0f);
                vector3 ReflectedPoint = C.Point + Refl;
                
                targetPosition = ReflectedPoint;
                rotAmount = R_360 * DeltaTime;

                if (!m_bDeflecting)
                {
                    m_bDeflecting = TRUE;
                    m_LastValidTargetPos = targetPosition;
                }
            }
            else
            {
                // nothing between projectile and target
                if (m_bDeflecting)
                {
                    m_bDeflecting = FALSE;
                }
            }

        } // end of avoid

        m_TargetPos = targetPosition;

        vector3 toTarget = targetPosition - GetPosition();
        toTarget.Normalize();
        vector3 ourVelocity = m_Velocity;
        ourVelocity.Normalize();       

        vector3 rotAngle = toTarget - ourVelocity;
        
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

xbool alien_turret_projectile::LoadInstance( const char* pFileName )
{
    (void)pFileName;
	// Initialize the skin
	//m_RigidInst.OnProperty( g_PropQuery.WQueryExternal( "RenderInst\\File", pFileName ) );
    //m_RigidInst.SetUpRigidGeom( pFileName );
	return TRUE;

}

//=============================================================================

void alien_turret_projectile::OnEnumProp( prop_enum& List )
{
    base_projectile::OnEnumProp( List );
        
    List.AddHeader( "Alien Projectile",           "Alien Turret Projectile Properties" );
    List.AddAngle ( "Alien Projectile\\Max Yaw Rate",    "How many degrees can the projectile turn per second." );
    List.AddAngle ( "Alien Projectile\\Yaw Increase Rate",    "How many degrees does the projectile yaw rate increase per second.  It starts at zero at launch, and increases until it reaches the max rate.  This property controls how quickly the yaw increases." );
    List.AddAngle ( "Alien Projectile\\Launch Cone Size",     "How large is the launch cone.  The cone determines how much the shot spreads when it is fired.");

    //m_RigidInst.OnEnumProp( List );
}

//=============================================================================


xbool alien_turret_projectile::OnProperty( prop_query& I )
{
    if ( base_projectile::OnProperty( I ) )
    {
        // Nothing to do
    }
    else if ( I.VarAngle( "Alien Projectile\\Max Yaw Rate", m_MaxYawRate ))
    {
        // Nothing to do
    }
    else if ( I.VarAngle( "Alien Projectile\\Yaw Increase Rate", m_YawIncreaseRate ))
    {
        // Nothing to do
    }
    else if ( I.VarAngle( "Alien Projectile\\Launch Cone Size", m_LaunchConeSize ))
    {
        // Nothing to do
    }
    else
    {
        return FALSE;
    }
    /*
    else if (m_RigidInst.OnProperty( I ))
    {
        return TRUE;
    }
    */
    return TRUE;
}

//=============================================================================

void alien_turret_projectile::AvoidObstacles      ( xbool bAvoid )
{
    m_bAvoidObstacles = bAvoid;
}

#endif
