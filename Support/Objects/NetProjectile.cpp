//==============================================================================
//
//  NetProjectile.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "NetProjectile.hpp"
#include "Entropy/e_Draw.hpp"
#include "Objects/ClothObject.hpp"
#include "Objects/Flag.hpp"
#include "Objects/Actor/Actor.hpp"
#include "Characters/Character.hpp"
#include "Objects/Corpse.hpp"
#include "Objects/DestructibleObj.hpp"
#include "Objects/SuperDestructible.hpp"
#include "Objects/Turret.hpp"
#include "Objects/AlienShield.hpp"
#include "Objects/alienglob.hpp"
#include "Objects/ParticleEmiter.hpp"
#include "Objects/Player.hpp"

#ifndef X_EDITOR
#include "NetworkMgr/NetworkMgr.hpp"
#endif

#ifdef cgalley
#define ENABLE_LOGGING 0
#else
#define ENABLE_LOGGING 0
#endif

//==============================================================================
//  DEFINITIONS
//==============================================================================

#define MAX_BOUNCES_PER_LOGIC   2

static f32 NET_PROJECTILE_CLOTH_IMPACT_FORCE = 1000.0f;


//==============================================================================
//  STORAGE
//==============================================================================

vector3 net_proj::m_OldPos;
vector3 net_proj::m_NewPos;
vector3 net_proj::m_Velocity;
f32     net_proj::m_DeltaTime;

//==============================================================================
//  FUNCTIONS
//==============================================================================

net_proj::net_proj( void ) :
    m_ImpactNormal( 0.0f, 1.0f, 0.0f ),
    m_BlendVector( 0.0f, 0.0f, 0.0f ),
    m_BlendTimer( 0.0f ),
    m_BlendTotalTime( 8.0f / 30.0f )
{
    m_OriginGuid         = 0;
    m_OriginNetSlot      = -1;
    m_Finished           = FALSE;

    m_Orientation.Zero();

    m_Age                =   0.0f;
    m_Gravity            = 980.0f;

    m_AtRest             = TRUE;
    m_Exploded           = FALSE;
    m_ExplodedTimer      = 3.0f;

    m_bIsSticky          = FALSE;
    m_bIsAttached        = FALSE;
    m_AttachedBoneIndex  = -1;
    m_AttachedLocalPos.Zero();
    m_AttachedObjectGuid = 0;
                       
    m_ImpactCount        = 0;
    m_bIsLargeProjectile = FALSE;
    m_BounceFactorRun    =  0.75f;
    m_BounceFactorRise   = -0.50f;

    m_bThroughGlass      = TRUE;

#ifndef X_EDITOR 
    m_NetType            = netobj::TYPE_NULL;
#endif  
}

//==============================================================================

net_proj::~net_proj( void )
{
    // If there is an effect, get rid of it.
}

//==============================================================================

void net_proj::SetOrigin( guid OriginGuid, s32  OriginNetSlot )
{
    m_OriginGuid    = OriginGuid;
    m_OriginNetSlot = OriginNetSlot;
}

//==============================================================================

void net_proj::SetStart( const vector3& Position, 
                         const radian3& Orientation,
                         const vector3& Velocity,
                               s32      Zone1, 
                               s32      Zone2,
                               f32      Gravity )
{
    m_StartPos = Position;
    m_StartVel = Velocity;
    m_Gravity  = Gravity;

    m_TimeT    = 0.0f;
    m_AtRest   = FALSE;

    SetOrientation( Orientation );
    SetPosition   ( Position, Zone1, Zone2 );

    // Blending issues?

    #ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_START;
    m_NetDirtyBits |= DIRTY_TIME_T;
    m_NetDirtyBits |= DIRTY_ATTACH;
    #endif
}                     

//==============================================================================

void net_proj::SetOrientation( const radian3& Orientation )
{
    m_Orientation = Orientation;

    #ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_POSITION;
    #endif
}

//==============================================================================

void net_proj::SetPosition( const vector3& Position,
                                  s32      Zone1,
                                  s32      Zone2 )
{
    matrix4 L2W;

    L2W.Identity();
    L2W.Rotate( m_Orientation );
    L2W.Translate( Position );

    OnTransform( L2W );

    SetZone1( Zone1 );
    SetZone2( Zone2 );
    g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );

    #ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_POSITION;
    #endif
}

//=============================================================================================

void net_proj::Setup( guid           OriginGuid,
                      s32            OriginNetSlot,
                      const vector3& Position,
                      const radian3& Orientation,
                      const vector3& Velocity,
                      s32            Zone1,
                      s32            Zone2,
                      pain_handle    PainHandle )
{
    SetOrigin    ( OriginGuid, OriginNetSlot );
    SetStart     ( Position, Orientation, Velocity, Zone1, Zone2, m_Gravity );
    SetPainHandle( PainHandle );

    // Set flag to say broadcast alert when ready.
    
}

//==============================================================================

void net_proj::SetSticky( xbool IsSticky )
{
    m_bIsSticky = IsSticky;
}

//==============================================================================

const vector3& net_proj::GetStartVelocity( void )
{
    return m_StartVel;
}

//==============================================================================

void net_proj::ComputePosAndVelAtTimeT( f32 T )
{
    m_Velocity = m_StartVel;

    if( m_Gravity == 0.0f )
    {
        // Linear movement.
        m_NewPos   = m_StartPos + m_StartVel * T;
    }
    else
    {
        // Arcing movement.

        f32 Factor = T * T * 0.5f;

        m_NewPos           = m_StartPos;            // Start...
        m_NewPos          += m_StartVel * T;        //  + Linear
        m_NewPos.GetY()   -= m_Gravity * Factor;    //  - Gravity
        m_Velocity.GetY() -= m_Gravity * T;
    }
}

//==============================================================================

void net_proj::SetPainHandle( pain_handle PainHandle )
{
    m_PainHandle = PainHandle;
}

//==============================================================================

bbox net_proj::GetLocalBBox( void ) const
{
    // TO DO

    // Compute bbox which includes geometry and effect.
    bbox BBox( vector3(0,0,0), 25.0f );
    return( BBox );
}

//==============================================================================

s32 net_proj::GetMaterial( void ) const
{
    return( MAT_TYPE_NULL );
}

//==============================================================================

u32 net_proj::GetRenderMode( void )
{
    u32 Mode = render::WIREFRAME;

    if( GetAttrBits() & ATTR_EDITOR_PLACEMENT_OBJECT )
    {
        Mode = render::PULSED;
    }
    else if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        if( GetAttrBits() & ATTR_EDITOR_BLUE_PRINT )
            Mode |= render::WIREFRAME2;
        else
            Mode |= render::WIREFRAME;
    }

    return( Mode );
}

//==============================================================================

void net_proj::OnColCheck( void )
{
    // TO DO
}

//==============================================================================

void net_proj::OnMove( const vector3& NewPos )
{
    // Move the geometry.
    // Move the effect.
    // Move the audio.
    // Update the spatial database.  object::OnMove()?
    // Zoning?
    // Blending?
    object::OnMove( NewPos );
    UpdateZoneTrack();
}

//=========================================================================

void net_proj::UpdateZoneTrack( void )
{ 
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, GetPosition() );
}

//==============================================================================

void net_proj::OnRender( void )
{
}

//==============================================================================

void net_proj::OnRenderTransparent( void )
{   
#if !defined( CONFIG_RETAIL )
    draw_Marker( GetPosition(), XCOLOR_BLUE );
    draw_Sphere( GetPosition(), 50.0f, XCOLOR_RED );
    draw_BBox( GetBBox() );
#endif // !defined( CONFIG_RETAIL )
}

//==============================================================================

void net_proj::OnAdvanceLogic( f32 DeltaTime )
{
    // Projectile has exploded and now just needs to time out and destroy itself
    if( m_Exploded )
    {
#ifndef X_EDITOR
        if( m_OwningClient == g_NetworkMgr.GetClientIndex() )
#endif
        {
            m_ExplodedTimer -= DeltaTime;
            if( m_ExplodedTimer <= 0.0f )
            {
                // LOG_MESSAGE( "net_proj::OnAdvanceLogic", "Destroyed after Exploded" );
                DESTROY_NET_OBJECT( this );
            }
        }

        return;
    }

    // Age gracefully.
    m_Age   += DeltaTime;
    m_TimeT += DeltaTime;

    // Age the blend
    m_BlendTimer -= DeltaTime;
    if( m_BlendTimer < 0.0f )
    {
        m_BlendTimer = 0.0f;
    }

    // Is the projectile attached to something
    if( m_bIsAttached )
    {
        // New Position
        vector3 Position;

        object* pTarget = g_ObjMgr.GetObjectByGuid( m_AttachedObjectGuid );
        if( pTarget )
        {
            if( pTarget->IsKindOf( actor::GetRTTI() ) && (m_AttachedBoneIndex != -1) )
            {
                // m_CurrentPosition is now the hit location + offset from bone
                // follow the stuck object around
                // Update position
                actor* pActor = (actor*)pTarget;
                const matrix4& L2W = pActor->GetLocoPointer()->m_Player.GetBoneL2W( m_AttachedBoneIndex );
                Position = L2W * m_AttachedLocalPos;
            }
            else
            {
                // Just use the root of the object
                const matrix4& L2W = pTarget->GetL2W();
                Position = L2W * m_AttachedLocalPos;
            }

            OnMove( Position );
        }

        return;
    }

    // Save off deltatime so we can consume it piece by piece
    m_DeltaTime = DeltaTime;

    // Is this... the end?  Maybe this should be in the descendant.
    //if( m_Finished && "effect is done" )
    //{
    //
    //}

    // Are we moving?  If not, bail.
    if( m_AtRest )
        return;

    s32 Iterations = 0;

    // Although we haven't impacted yet, setting this to TRUE will simplify
    // the code needed to get us into the next loop.  m_Impact is set to FALSE
    // early within the loop.
    m_Impact = TRUE;    

    m_OldPos = GetPosition();

    while( (!m_Finished) && (!m_AtRest) && (!m_bIsAttached) && (m_Impact) && (m_DeltaTime >= 0.0f) &&
           (Iterations < MAX_BOUNCES_PER_LOGIC) &&
           !(GetAttrBits() & ATTR_DESTROY) )
    {
        Iterations += 1;
        m_Impact    = FALSE;

        // Let's move!
        // Determine where we WANT to go.

        f32 NewT = m_TimeT + m_DeltaTime;

        // Now that we know where we want to go, attempt to move there.  If we 
        // do hit something, we will rebound and attempt to consume any 
        // remaining delta time.  If we hit too many times, we will just bail 
        // out.

        ComputePosAndVelAtTimeT( NewT );

        CLOG_MESSAGE( ENABLE_LOGGING, "net_proj::OnAdvanceLogic", 
                                      "Collide from (%4.2f,%4.2f,%4.2f) to (%4.2f,%4.2f,%4.2f).",
                                      m_OldPos.GetX(), m_OldPos.GetY(), m_OldPos.GetZ(), 
                                      m_NewPos.GetX(), m_NewPos.GetY(), m_NewPos.GetZ() );

/* TODO: CJ: Projectiles should be spheres but the high poly sphere collision is too slow right now.
        g_CollisionMgr.SphereSetup( GetGuid(), m_OldPos, m_NewPos, 20.0f ); // TO DO - Get diameter from object.
        g_CollisionMgr.UseLowPoly();
*/
        if( GetDoCollisions() )
        {        
            g_CollisionMgr.RaySetup( GetGuid(), m_OldPos, m_NewPos );

            g_CollisionMgr.SetMaxCollisions( 10 );
            g_CollisionMgr.AddToIgnoreList( m_OriginGuid );
            if( m_bIsLargeProjectile )
            {        
                g_CollisionMgr.CheckCollisions( 
                        object::TYPE_ALL_TYPES, 
                        object::ATTR_BLOCKS_LARGE_PROJECTILES);
            }
            else
            {
                g_CollisionMgr.CheckCollisions( 
                        object::TYPE_ALL_TYPES, 
                        object::ATTR_BLOCKS_SMALL_PROJECTILES);
            }

            
            // No collision?  Then bail out of this loop.
            if( g_CollisionMgr.m_nCollisions == 0 )
            {
                CLOG_MESSAGE( ENABLE_LOGGING, "net_proj::OnAdvanceLogic", 
                                            "Clean move from (%4.2f,%4.2f,%4.2f) to (%4.2f,%4.2f,%4.2f).",
                                            m_OldPos.GetX(), m_OldPos.GetY(), m_OldPos.GetZ(), 
                                            m_NewPos.GetX(), m_NewPos.GetY(), m_NewPos.GetZ() );

                // Since m_Impact is FALSE, the surrounding do/while loop will 
                // terminate.
                break;
            }

            //
            // Backup collisions
            //
            s32 nCollisions = g_CollisionMgr.m_nCollisions;
            ASSERT( nCollisions <= MAX_COLLISION_MGR_COLLISIONS );
            if( nCollisions > MAX_COLLISION_MGR_COLLISIONS ) nCollisions = MAX_COLLISION_MGR_COLLISIONS;

            collision_mgr::collision CollBackup[MAX_COLLISION_MGR_COLLISIONS];
            x_memcpy( CollBackup, g_CollisionMgr.m_Collisions, sizeof( collision_mgr::collision )*nCollisions );

            // Process the collisions in order.
            for( s32 i=0; i<nCollisions; i++ )
            {
                collision_mgr::collision& Coll = CollBackup[i];

                // Skip over collisions with unidentifiable objects.
                if( Coll.ObjectHitGuid == 0 )
                {
                    LOG_WARNING( "net_proj::OnAdvanceLogic",
                                "Collision with 'unidentifiable' object." );
                    continue;
                }

                // Let's start interacting with the object we have hit.
                object* pObject = g_ObjMgr.GetObjectByGuid( Coll.ObjectHitGuid );
                if( !pObject )
                {
                    LOG_WARNING( "net_proj::OnAdvanceLogic",
                                "Collision with 'unretrievable' object." );
                    continue;
                }

                // Just go through cloth objects.
                if( pObject->IsKindOf( cloth_object::GetRTTI() ) )
                {
                    // Notify the cloth of the impact.
                    cloth_object* pClothObject = (cloth_object*)pObject;
                    pClothObject->OnProjectileImpact( *this, 
                                                    m_Velocity,
                                                    Coll.PrimitiveKey, 
                                                    Coll.Point, FALSE, NET_PROJECTILE_CLOTH_IMPACT_FORCE );
                                                      
                    // Do NOT alter the trajectory as a result of the cloth.  
                    // Doing so would screw up the networking.
                    continue;                
                }
                
                // Just go through flag objects.
                if( pObject->IsKindOf( flag::GetRTTI() ) )
                {
                    // Notify the cloth of the impact.
                    flag* pFlag = (flag*)pObject;
                    pFlag->OnProjectileImpact( *this, 
                                                m_Velocity,
                                                Coll.PrimitiveKey, 
                                                Coll.Point, FALSE, NET_PROJECTILE_CLOTH_IMPACT_FORCE );
                                                
                    // Do NOT alter the trajectory as a result of the cloth.  
                    // Doing so would screw up the networking.
                    continue;                
                }

                //
                // Is this a piece of glass
                //
                if( m_bThroughGlass && (Coll.Flags == object::MAT_TYPE_GLASS) )
                {
                    //x_DebugMsg("PROJECTILE HIT GLASS!!! %d of %d\n",i,nCollisions);

                    particle_emitter::CreateProjectileCollisionEffect( Coll, m_OriginGuid );

                    pain Pain;
                    Pain.Setup( m_PainHandle, m_OriginGuid, Coll.Point );
                    Pain.SetDirection( m_Velocity );
                    Pain.SetDirectHitGuid( pObject->GetGuid() );
                    Pain.SetCollisionInfo( Coll );
                    //Pain.SetCustomScalar( CalcDamageDegradation(Coll.Point) );
                    Pain.ApplyToObject( pObject );

                    // Don't stop processing other collisions.  We're going to
                    // go right through the glass.
                    continue;
                }

                // Just go through permeable objects.
                if( pObject->GetAttrBits() & object::ATTR_COLLISION_PERMEABLE )
                {
                    pObject->OnColNotify( *this );
                    continue;
                }

                // If we are here, then we have hit something worth a reaction.  The
                // object (which descends from this class and implements function
                // OnCollision) can react how it sees fit.
                OnImpact( Coll, pObject );

                // What are our options now?  And how does OnImpact signal this?
                //  + Object has come to a stop.
                //      - Set m_AtRest to TRUE.
                //      - Set m_Impact to TRUE to break out of this loop.
                //  + Object has blown up!
                //      - Set m_AtRest to TRUE.
                //      - Set m_Finished to TRUE.
                //      - Set m_RenderEffect as appropriate.
                //      - Set m_Impact to TRUE to break out of this loop.
                //  + Object has bounced.
                //      - Determine new starting point and velocity.
                //      - Leave m_AtRest at FALSE.
                //      - Set the DIRTY_START dirty bit.
                //      - Set m_Impact to TRUE to break out of this loop.
                //  + Object has decided to ignore this collision.
                //      - Leave m_Impact at FALSE to stay in this loop.

                if( m_Impact )
                {
                    break;
                    // This will break out of the 'for' loop and then iterate again
                    // within the surrounding 'do/while' loop as long as we haven't
                    // exceeded the iteration limit.
                }
            }
        }
    }

    //
    // For better or worse, we have a new position.  Put the object there.
    //

    CLOG_MESSAGE( ENABLE_LOGGING, 
                  "net_proj::OnAdvanceLogic", 
                  "TimeT:%4.2f - Position:(%4.2f,%4.2f,%4.2f) - Velocity:(%4.2f,%4.2f,%4.2f)", 
                  m_TimeT,
                  m_NewPos.GetX(),   m_NewPos.GetY(),   m_NewPos.GetZ(),
                  m_Velocity.GetX(), m_Velocity.GetY(), m_Velocity.GetZ() );

    // Don't move objects marked for destroy
    if( !(GetAttrBits() & object::ATTR_DESTROY) )
        OnMove( m_NewPos );
}

//===========================================================================

xbool net_proj::CheckHitValidObject( object* pObject )
{
    if( ( pObject->IsKindOf( actor::GetRTTI()                  ) ||
          pObject->IsKindOf( destructible_obj::GetRTTI()       ) ||
          pObject->IsKindOf( super_destructible_obj::GetRTTI() ) ||
          pObject->IsKindOf( turret::GetRTTI()                 ) ||
          pObject->IsKindOf( alien_glob::GetRTTI()             ) ||
          pObject->IsKindOf( alien_shield::GetRTTI()           ) ||
          pObject->IsKindOf( corpse::GetRTTI()                 ) ) )
    {
        return TRUE;
    }

    return FALSE;
}

//===========================================================================

xbool net_proj::CheckHitThreatObject( object* pObject )
{
    if( pObject->IsKindOf(actor::GetRTTI())  || 
        pObject->IsKindOf(turret::GetRTTI()) ||
        pObject->IsKindOf(alien_glob::GetRTTI()) ||
        pObject->IsKindOf(alien_shield::GetRTTI()) )
    {
        return TRUE;
    }

    return FALSE;
}

//==============================================================================

void net_proj::OnImpact( collision_mgr::collision& Coll, object* pTarget )
{
    (void)pTarget;

    // Are we sticky and did we hit a sticky type target
    if( m_bIsSticky && CheckHitValidObject(pTarget) )
    {
        // Attach to target
        OnAttachToObject( Coll, pTarget );
        m_Impact = TRUE;
        return;
    }

    // Save impact information for decal generation, etc.
    m_ImpactPoint   = Coll.Point;
    m_ImpactNormal  = Coll.Plane.Normal;
    m_ImpactCount++;

    // We want to back off by 1cm, but we can't back up beyond T=0.
    ASSERT( (Coll.T >= 0.0f) && (Coll.T <= 1.0f) );

    // Consume some of the delta time
    m_DeltaTime -= m_DeltaTime * Coll.T;

    vector3 ToImpact = (m_NewPos - m_OldPos) * Coll.T;
    vector3 Movement = m_NewPos - m_OldPos;
    f32     Travel   = ToImpact.Length();

    CLOG_MESSAGE( ENABLE_LOGGING, "net_proj::OnImpact", 
                                  "TimeT:%4.2f - CollT:%4.2f - Pos:(%4.2f,%4.2f,%4.2f) - SurfaceNormal:(%4.2f,%4.2f,%4.2f)",
                                  m_TimeT, Coll.T, 
                                  Coll.Point.GetX(),
                                  Coll.Point.GetY(),
                                  Coll.Point.GetZ(),
                                  Coll.Plane.Normal.GetX(), 
                                  Coll.Plane.Normal.GetY(), 
                                  Coll.Plane.Normal.GetZ() );

    // TODO: CJ: Adjust backoff of ray if we can't move to sphere

    if( Travel < 1.0f )
    {
        // Travel distance was too short to be able to back off a full 1cm.
        m_NewPos = m_OldPos;
    }
    else
    {        
        vector3 AdjustedMovement = Movement;
        AdjustedMovement.NormalizeAndScale( Travel - 1.0f );
        m_NewPos = m_OldPos + AdjustedMovement;
    }

    // Should we come to rest?  Only if:
    //  - Velocity is less than 100cm/sec, and
    //  - Surface isn't too steep.

    if( (m_Velocity.LengthSquared() < SQR(100.0f)) && 
        (Coll.Plane.Normal.GetY()   > 0.5f) )
    {
        // Come to a stop.
        m_AtRest = TRUE;
        m_Impact = TRUE;

        CLOG_MESSAGE( ENABLE_LOGGING, "net_proj::OnImpact", "At Rest" );

        #ifndef X_EDITOR
        m_NetDirtyBits |= DIRTY_POSITION;
        #endif
    }
    else
    {
        // Rebound!

        vector3 Rise;
        vector3 Run;

        Coll.Plane.GetComponents( m_Velocity, Run, Rise );

        m_Impact   = TRUE;
        m_TimeT    = 0.0f;
        m_StartPos = m_NewPos;
        m_OldPos   = m_NewPos;
        m_StartVel = (Run * m_BounceFactorRun) + (Rise * m_BounceFactorRise);

        CLOG_MESSAGE( ENABLE_LOGGING, "net_proj::OnImpact",
                                      "Rebound - OldVel (%4.2f,%4.2f,%4.2f) - NewVel(%4.2f,%4.2f,%4.2f)",
                                      m_Velocity.GetX(), m_Velocity.GetY(), m_Velocity.GetZ(), 
                                      m_StartVel.GetX(), m_StartVel.GetY(), m_StartVel.GetZ() );

        #ifndef X_EDITOR
        m_NetDirtyBits |= DIRTY_START;
        #endif
    }
}

//==============================================================================

void net_proj::OnAttachToObject( collision_mgr::collision& Coll, object* pTarget )
{
    // TODO: CJ: This position may not be correct, may need to recompute position based on collT, etc.
    vector3 Position = Coll.Point;

    // look for bone to stick to
    if( pTarget )
    {
        // Flag attached
        m_bIsAttached        = TRUE;
        m_AttachedObjectGuid = Coll.ObjectHitGuid;

        // Clear attach info in-case it's attached to root of object
        m_AttachedBoneIndex = -1;
        m_AttachedLocalPos.Zero();

        // KSS -- FIXME -- Hack to fixup character firing weapons because they only using BBoxes
        object* pObj = g_ObjMgr.GetObjectByGuid( m_OriginGuid );
        if( pObj && pObj->IsKindOf( character::GetRTTI() ) )
        {
            return;
        }

        if( pTarget )
        {
            if( pTarget->IsKindOf( actor::GetRTTI() ) )//&& BoneCollisionWasUsed....)
            {
                // Get L2W matrices
                actor* pActor = (actor*)pTarget;

                // If this is a player, just stick the projectile to their position
                if( pActor->IsPlayer() )
                {
                    // Attach to root of object
                    const matrix4& L2W = pTarget->GetL2W();
                    matrix4 W2L = m4_InvertRT( L2W );
                    m_AttachedLocalPos = W2L * Position;
                    m_AttachedBoneIndex = -1;
                }
                else
                {
                    // Get W2L for bone
                    m_AttachedBoneIndex = Coll.PrimitiveKey & 0xFF;    // Get bone that was hit
                    const matrix4& L2W = pActor->GetLocoPointer()->m_Player.GetBoneL2W( m_AttachedBoneIndex );

                    matrix4 W2L = m4_InvertRT( L2W );
                    m_AttachedLocalPos = W2L * Position;

                    if( pActor->IsCharacter() )
                    {
                        character &attachedCharacter = character::GetSafeType(*pActor);
                        attachedCharacter.SetProjectileAttached();
                    }
                }
            }
            else
            {
                // Attach to root of object
                const matrix4& L2W = pTarget->GetL2W();
                matrix4 W2L = m4_InvertRT( L2W );
                m_AttachedLocalPos = W2L * Position;
            }
        }
    }
}

//==============================================================================

void net_proj::OnExplode( void )
{
    m_Exploded = TRUE;
#ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_EXPLODED;
#endif

    CLOG_MESSAGE( ENABLE_LOGGING, 
                  "net_proj::OnExplode", 
                  "Position:(%4.2f,%4.2f,%4.2f)", 
                  GetPosition().GetX(), GetPosition().GetY(), GetPosition().GetZ() );
}

//==============================================================================
#ifndef X_EDITOR
//==============================================================================

void net_proj::net_Logic( f32 /*DeltaTime*/ )
{
}

//==============================================================================
/*
        DIRTY_POSITION = 0x00000001,    // Only used when at rest.
        DIRTY_START    = 0x00000002,    // Only used when NOT at rest.
        DIRTY_TIME_T   = 0x00000004,    // Only used when NOT at rest.
        DIRTY_ATTACH   = 0x00000008,
*/

void net_proj::net_AcceptUpdate( const bitstream& BS )
{
    // LOG_MESSAGE( "net_proj::net_AcceptUpdate", "" );

//  u32 Bits = 0x00000000;

    if( BS.ReadFlag() )
    {
        BS.ReadRangedS32( m_OriginNetSlot, -1, 31 );
        net_AcceptActivate( BS );

        m_OriginGuid = 0;
        if( IN_RANGE( 0, m_OriginNetSlot, 31 ) )
        {
            actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( m_OriginNetSlot );
            
            if( pActor )
            { 
                m_OriginGuid = pActor->GetGuid();

                new_weapon* pWeapon = pActor->GetCurrentWeaponPtr();
                if( pWeapon )
                {
                    if( pWeapon->GetFiringStartPosition( m_BlendVector ) )
                    {
                        m_BlendTimer = m_BlendTotalTime;
                    }
                }
            }
        }
//      Bits |= netobj::ACTIVATE_BIT;
    }

    if( BS.ReadFlag( m_AtRest ) )
    {
        if( BS.ReadFlag() )
        {
            vector3 Position;
            radian3 Orientation;
            s32     Zone1;
            s32     Zone2;

            BS.ReadVector( Position );
            BS.ReadF32   ( Orientation.Pitch );
            BS.ReadF32   ( Orientation.Yaw   );
            BS.ReadF32   ( Orientation.Roll  );
            BS.ReadRangedS32( Zone1, 0, 255 );
            BS.ReadRangedS32( Zone2, 0, 255 );

            SetOrientation( Orientation );
            SetPosition   ( Position, Zone1, Zone2 );

            CLOG_MESSAGE( ENABLE_LOGGING, 
                          "net_proj::net_AcceptUpdate", 
                          "[At Rest] Position:(%4.2f,%4.2f,%4.2f)", 
                          m_NewPos.GetX(), m_NewPos.GetY(), m_NewPos.GetZ() );

//          Bits |= DIRTY_POSITION;
        }
    }
    else
    {
        if( BS.ReadFlag() )
        {
            s32     Zone1;
            s32     Zone2;

            BS.ReadVector( m_StartPos );
            BS.ReadVector( m_StartVel );
            BS.ReadRangedS32( Zone1, 0, 255 );
            BS.ReadRangedS32( Zone2, 0, 255 );
            net_AcceptStart( BS );

            if( BS.ReadFlag() )
            {
                m_BlendVector.Zero();
                m_BlendTimer = 0.0f;
            }
            else
            {
                m_BlendVector = m_BlendVector - m_StartPos;
            }

            SetStart( m_StartPos, radian3(0,0,0), m_StartVel, Zone1, Zone2, m_Gravity );

            CLOG_MESSAGE( ENABLE_LOGGING, 
                          "net_proj::net_AcceptUpdate", 
                          "[Start] Position:(%4.2f,%4.2f,%4.2f) - Velocity:(%4.2f,%4.2f,%4.2f)", 
                          m_StartPos.GetX(), m_StartPos.GetY(), m_StartPos.GetZ(),
                          m_StartVel.GetX(), m_StartVel.GetY(), m_StartVel.GetZ() );

//          Bits |= DIRTY_START;
        }

        if( BS.ReadFlag() )
        {
            BS.ReadF32( m_TimeT );

            CLOG_MESSAGE( ENABLE_LOGGING, 
                          "net_proj::net_AcceptUpdate", 
                          "[TimeT] TimeT:%4.2f", m_TimeT );

//          Bits |= DIRTY_TIME_T;
        }

        if( BS.ReadFlag() )
        {
            // Set to "not attached"
            m_AttachedObjectGuid = 0;
            m_AttachedBoneIndex  = -1;
            m_AttachedLocalPos.Zero();
        
            // Try attach?
            if( BS.ReadFlag( m_bIsAttached ) )
            {
                // Read into locals in case of fail
                s32     AttachedNetSlot   = -1;
                s32     AttachedBoneIndex = -1;
                vector3 AttachedLocalPos( 0.0f, 0.0f, 0.0f );

                // Read info
                BS.ReadS32   ( AttachedNetSlot   );
                BS.ReadS32   ( AttachedBoneIndex );
                if( AttachedBoneIndex != -1 )
                    BS.ReadVector( AttachedLocalPos  );
                
                // Does object exist?
                ASSERT( AttachedNetSlot != -1 );
                netobj* pNetObj = NetObjMgr.GetObjFromSlot( AttachedNetSlot );
                if( pNetObj )
                {
                    // Attach
                    m_AttachedObjectGuid = pNetObj->GetGuid();
                    if( pNetObj->IsKindOf( player::GetRTTI() ) )
                    {
                        m_AttachedBoneIndex = -1;
                        m_AttachedLocalPos.Set( 0.0f, 100.0f, 0.0f );
                    }
                    else
                    {
                        m_AttachedBoneIndex = AttachedBoneIndex;
                        m_AttachedLocalPos  = AttachedLocalPos;
                    }
                }
                else
                {
                    // Failed, so set to "not attached"
                    m_bIsAttached = FALSE;
                }                
            }
            
            // Send to other clients
            m_NetDirtyBits |= DIRTY_ATTACH;
            
//          Bits |= DIRTY_ATTACH;
        }
    }

    xbool Exploded;
    if( BS.ReadFlag( Exploded ) )
    {
        vector3 Pos;
        BS.ReadVector( Pos );

        // Put the projectile where it exploded
        //if( !m_bIsAttached )
        {
            OnMove( Pos );
            m_ImpactPoint = Pos;
        }

        //LOG_MESSAGE( "_net_proj::net_AcceptUpdate", "Explode %.3f, %.3f, %.3f", Pos.GetX(), Pos.GetY(), Pos.GetZ() );
        //LOG_FLUSH();

        OnExplode();
    }

//  LOG_MESSAGE( "net_proj::net_AcceptUpdate", "DirtyBits:%08X", Bits );
//  LOG_FLUSH();

    BS.ReadMarker();
}

//==============================================================================

void net_proj::net_ProvideUpdate( bitstream& BS, u32& DirtyBits )
{   
    // LOG_MESSAGE( "net_proj::net_ProvideUpdate", "DirtyBits:%08X", DirtyBits );

//  u32 Keep = DirtyBits;
//  u32 Bits = 0x00000000;

    if( BS.WriteFlag( DirtyBits & netobj::ACTIVATE_BIT ) )
    {
        BS.WriteRangedS32( m_OriginNetSlot, -1, 31 );
        net_ProvideActivate( BS );
        DirtyBits &= ~netobj::ACTIVATE_BIT;
//      Bits |= netobj::ACTIVATE_BIT;

        DirtyBits |= DIRTY_ALL;
        DirtyBits &= ~DIRTY_EXPLODED;
    }

    if( BS.WriteFlag( m_AtRest ) )
    {
        DirtyBits &= ~DIRTY_START;
        DirtyBits &= ~DIRTY_TIME_T;
        DirtyBits &= ~DIRTY_ATTACH;

        if( BS.WriteFlag( DirtyBits & DIRTY_POSITION ) )
        {
            radian3 Orientation = GetL2W().GetRotation();

            BS.WriteVector( GetPosition()     );
            BS.WriteF32   ( Orientation.Pitch );
            BS.WriteF32   ( Orientation.Yaw   );
            BS.WriteF32   ( Orientation.Roll  );
            BS.WriteRangedS32( GetZone1(), 0, 255 );
            BS.WriteRangedS32( GetZone2(), 0, 255 );
            DirtyBits &= ~DIRTY_POSITION;

            CLOG_MESSAGE( ENABLE_LOGGING, 
                          "net_proj::net_ProvideUpdate", 
                          "[At Rest] Position:(%4.2f,%4.2f,%4.2f)", 
                          GetPosition().GetX(), GetPosition().GetY(), GetPosition().GetZ() );

//          Bits |= DIRTY_POSITION;
        }
    }
    else
    {
        DirtyBits &= ~DIRTY_POSITION;

        if( BS.WriteFlag( DirtyBits & DIRTY_START ) )
        {
            BS.WriteVector( m_StartPos );
            BS.WriteVector( m_StartVel );
            BS.WriteRangedS32( GetZone1(), 0, 255 );
            BS.WriteRangedS32( GetZone2(), 0, 255 );
            net_ProvideStart( BS );
            BS.WriteFlag( m_ImpactCount > 0 );
            DirtyBits &= ~DIRTY_START;

            CLOG_MESSAGE( ENABLE_LOGGING, 
                          "net_proj::net_ProvideUpdate", 
                          "[Start] Position:(%4.2f,%4.2f,%4.2f) - Velocity:(%4.2f,%4.2f,%4.2f)", 
                          m_StartPos.GetX(), m_StartPos.GetY(), m_StartPos.GetZ(),
                          m_StartVel.GetX(), m_StartVel.GetY(), m_StartVel.GetZ() );

//          Bits |= DIRTY_START;
        }

        if( BS.WriteFlag( DirtyBits & DIRTY_TIME_T ) )
        {
            BS.WriteF32( m_TimeT );
            DirtyBits &= ~DIRTY_TIME_T;

            CLOG_MESSAGE( ENABLE_LOGGING, 
                          "net_proj::net_ProvideUpdate", 
                          "[TimeT] TimeT:%4.2f", m_TimeT );

//          Bits |= DIRTY_TIME_T;
        }

        if( BS.WriteFlag( DirtyBits & DIRTY_ATTACH ) )
        {
            // Does object exist?
            s32 NetSlot = -1;
            object* pObject = g_ObjMgr.GetObjectByGuid( m_AttachedObjectGuid );
            if( pObject )
            {
                NetSlot = pObject->net_GetSlot();
            }
        
            // Attached?
            if( BS.WriteFlag( ( NetSlot != -1 ) && ( m_bIsAttached ) ) )
            {
                BS.WriteS32( NetSlot );
                BS.WriteS32( m_AttachedBoneIndex );
                if( m_AttachedBoneIndex != -1 )
                    BS.WriteVector( m_AttachedLocalPos );
            }

            DirtyBits &= ~DIRTY_ATTACH;
//          Bits |= DIRTY_ATTACH;
        }
    }

    if( BS.WriteFlag( m_Exploded ) )
    {
        if( m_ImpactCount > 0 )
        {
            BS.WriteVector( m_ImpactPoint );
        }
        else
        {
            BS.WriteVector( GetPosition() );
        }

    //  LOG_MESSAGE( "net_proj::net_ProvideUpdate", "Explode" );
    }
    DirtyBits &= ~DIRTY_EXPLODED;

//  LOG_MESSAGE( "net_proj::net_ProvideUpdate", "DirtyBits:%08X - Wrote:%08X", Keep, Bits );
//  LOG_FLUSH();

    BS.WriteMarker();
}

//==============================================================================

void net_proj::net_Deactivate( void )
{
//    if( (GameMgr.GameInProgress()) && 
//        (m_OwningClient != g_NetworkMgr.GetClientIndex()) )
//    {
//        OnExplode();
//    }
}

//==============================================================================

void net_proj::net_AcceptActivate( const bitstream& BS )
{
    (void)BS;
}

//==============================================================================

void net_proj::net_ProvideActivate( bitstream& BS )
{
    (void)BS;
}

//==============================================================================

void net_proj::net_AcceptStart( const bitstream& BS )
{
    (void)BS;
}

//==============================================================================

void net_proj::net_ProvideStart( bitstream& BS )
{
    (void)BS;
}

//==============================================================================

vector3 net_proj::net_GetBlendOffset( void )
{
    if( m_BlendTimer > 0.0f )
    {
        return m_BlendVector * (m_BlendTimer / m_BlendTotalTime);
    }
    else
    {
        return vector3( 0.0f, 0.0f, 0.0f );
    }
}

//==============================================================================
#endif // ifndef X_EDITOR
//==============================================================================
