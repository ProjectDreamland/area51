//==============================================================================
//
// CokeCan.cpp
//
// A cheap verlet particle modelled coke can - very similar to ragdoll -
//
// The can is modelled with 2 spheres connected by an equal distance constraint.
// Rolling is faked - the correct roll speed is computed when in contact
// with the ground.
//
// Now because this is a very cheap cpu physics model, there is no stacking as such.
// but because cans are initially inactive, you can place them stacked - then when
// one of them becomes active, it makes any cans close by also become active,
// and the stack will all fall down. Cheeky. 
//
// Collision if performed as follows:
// 1 ) Can collision:   Spheres of each can are kept a set distance from each other.
// 2 ) World collision: Each sphere is just projected out of the colliding plane.
// 3 ) Actor collision: Each sphere is kept a set distance from the actor cylinder.
//
//==============================================================================

#include "CokeCan.hpp"
#include "Entropy\Entropy.hpp"
#include "Objects\BaseProjectile.hpp"
#include "Ragdoll\VerletCollision.hpp"
#include "Objects\Player.hpp"

#ifdef X_EDITOR
#include "CollisionMgr\PolyCache.hpp"
#include "..\..\Apps\WorldEditor\WorldEditor.hpp"
#endif

//==============================================================================
// DEFINES
//==============================================================================

struct coke_can_tweaks
{
    f32         INVERSE_MASS               ;
    f32         ACTOR_COLL_VEL_PERP_SCALE  ;
    f32         ACTOR_COLL_VEL_PARA_SCALE  ;
    f32         GRAVITY                    ;
    f32         FRICTION                   ;
    f32         MIN_FRICTION               ;
    f32         MAX_FRICTION               ;
    f32         MAJOR_AXIS_FRICTION        ;
    f32         BOUNCY                     ;
    f32         TIME_STEP                  ;
    f32         COLLISION_BACKOFF          ;
    f32         MIN_COLL_DIST              ;
    f32         COLL_BBOX_INFLATE          ;
    f32         AIR_LINEAR_DAMPEN          ;
    f32         AIR_ANGULAR_DAMPEN         ;
    f32         GROUND_LINEAR_DAMPEN       ;
    f32         GROUND_ANGULAR_DAMPEN      ;
    f32         MAX_SPEED                  ;
    f32         ACTIVE_ENERGY              ;
    f32         PAIN_BULLET_FORCE_SCALE    ;
    f32         PAIN_EXPLOSION_FORCE_SCALE ;
    s32         ACTIVE_FRAMES              ;
    f32         AUDIO_IMPACT_SPEED         ;
    f32         AUDIO_MAX_ROLLING_SPEED    ;
    f32         AUDIO_MIN_ROLLING_SPEED    ;
    f32         AUDIO_COLLIDE_SPEED        ;
    const char* SFX_BULLET_IMPACT          ;
    const char* SFX_ROLLING                ;
    const char* SFX_IMPACT_WORLD           ;
    const char* SFX_IMPACT_CAN             ;
};

static coke_can_tweaks k_CokeCanTweak[ coke_can::PROFILE_COUNT ] = {

    // SMALL CAN

   {  1.0f / 1.0f,                  // m_InverseMass
      1.0f,                         // ACTOR_COLL_VEL_PERP_SCALE
      1.0f,                         // ACTOR_COLL_VEL_PARA_SCALE
      -9.8f * 100 * 2.0f,           // GRAVITY                      
      0.1f,                         // FRICTION                     
      0.010f,                       // MIN_FRICTION                 
      0.030f,                       // MAX_FRICTION                 
      0.1f,                         // MAJOR_AXIS_FRICTION          
      0.70f,                        // BOUNCY                       
      1.0f / 30.0f,                 // TIME_STEP                    
      0.1f,                         // COLLISION_BACKOFF            
      0.1f,                         // MIN_COLL_DIST                
      20.0f,                        // COLL_BBOX_INFLATE            
      0.001f,                       // AIR_LINEAR_DAMPEN                
      0.001f,                       // AIR_ANGULAR_DAMPEN               
      0.01f,                        // GROUND_LINEAR_DAMPEN                
      0.1f,                         // GROUND_ANGULAR_DAMPEN               
      100.0f,                       // MAX_SPEED                    
      4.0f,                         // ACTIVE_ENERGY                
      625.0f,                       // PAIN_BULLET_FORCE_SCALE             
      1000.0f,                      // PAIN_EXPLOSION_FORCE_SCALE
      4,                            // ACTIVE_FRAMES                
      3.0f,                         // AUDIO_IMPACT_SPEED           
      2.0f,                         // AUDIO_MAX_ROLLING_SPEED      
      0.05f,                        // AUDIO_MIN_ROLLING_SPEED      
      3.0f,                         // AUDIO_COLLIDE_SPEED 
      "BulletImpactMetal",          // SFX_BULLET_IMPACT
      "Can_Roll_Loop",              // SFX_ROLLING
      "Can_Impact_World",           // SFX_IMPACT_WORLD
      "Can_Impact_Can",             // SFX_IMPACT_CAN
    },

    // BARREL SIZED CAN

   {  1.0f / 20.0f,                 // m_InverseMass
    0.1f,                           // ACTOR_COLL_VEL_PERP_SCALE
    0.5f,                           // ACTOR_COLL_VEL_PARA_SCALE
   -9.8f * 100 * 2.0f,              // GRAVITY                      
    0.2f,                           // FRICTION                     
    0.010f,                         // MIN_FRICTION                 
    0.040f,                         // MAX_FRICTION                 
    0.1f,                           // MAJOR_AXIS_FRICTION
    0.25f,                          // BOUNCY                       
    1.0f / 30.0f,                   // TIME_STEP                    
    0.1f,                           // COLLISION_BACKOFF            
    0.1f,                           // MIN_COLL_DIST                
    20.0f,                          // COLL_BBOX_INFLATE            
    0.001f,                         // AIR_LINEAR_DAMPEN                
    0.0015f,                        // AIR_ANGULAR_DAMPEN               
    0.04f,                          // GROUND_LINEAR_DAMPEN                
    0.5f,                           // GROUND_ANGULAR_DAMPEN               
    75.0f,                          // MAX_SPEED
    4.0f,                           // ACTIVE_ENERGY                
    50.0f,                          // PAIN_BULLET_FORCE_SCALE             
    300.0f,                         // PAIN_EXPLOSION_FORCE_SCALE
    4,                              // ACTIVE_FRAMES                
    3.0f,                           // AUDIO_IMPACT_SPEED           
    2.0f,                           // AUDIO_MAX_ROLLING_SPEED      
    0.05f,                          // AUDIO_MIN_ROLLING_SPEED      
    3.0f,                           // AUDIO_COLLIDE_SPEED 
    "BulletImpactRubber",           // SFX_BULLET_IMPACT
    "Barrel_Roll_Loop",             // SFX_ROLLING
    "Barrel_Impact_World",          // SFX_IMPACT_WORLD
    "Barrel_Impact_Barrel",         // SFX_IMPACT_CAN
    }
};


#ifndef X_RETAIL
static xbool    DEBUG_COKE_CAN                  = FALSE;
#endif

//=========================================================================
// EXTERNS
//=========================================================================

#ifdef X_EDITOR
extern xbool g_game_running;
extern xbool g_level_loading;
#endif


//=========================================================================
// EDITOR UTILITY FUNCTIONS
//=========================================================================

#ifdef X_EDITOR

static
xbool SphereIntersectsNGon( const vector3& ObjectCenter,
                            const vector3& SpherePos, f32 SphereRadius, f32 CollisionBackOff,
                            const vector3* pVerts, s32 nVerts,
                                  f32&     Depth,
                                  vector3& Normal )
{
    ASSERT( pVerts );
    ASSERT( nVerts >= 3 );

    // Compute plane for NGon
    plane Plane;
    Plane.Setup( pVerts[0], pVerts[1], pVerts[2] );

    // Far enough away from the plane?
    f32 DistFromPlane = Plane.Distance( SpherePos );
    if( x_abs( DistFromPlane ) > ( SphereRadius + CollisionBackOff ) )
        return FALSE;

    // Compute the closest point on the plane to the sphere
    vector3 PointOnPlane = SpherePos + ( DistFromPlane * Plane.Normal );

    // Check to see if point on plane is inside NGon
    for( s32 v = 0; v < nVerts; v++ )
    {
        // Lookup edge end pts
        const vector3& EdgeStart = pVerts[( v == 0 ) ? nVerts-1 : v-1 ];
        const vector3& EdgeEnd   = pVerts[ v ];

        // Exit loop if point is outside of edge
        vector3 EdgeDir        = EdgeEnd - EdgeStart;
        vector3 EdgeNormal     = EdgeDir.Cross( Plane.Normal );
        vector3 EdgePointDelta = EdgeStart - SpherePos;

        // Outside of edge?
        if( EdgeNormal.Dot( EdgePointDelta ) < 0.0f )
            return FALSE;
    }

    // Compute sphere top and bottom points with respect to plane
    vector3 SphereTopPoint     = SpherePos + ( SphereRadius * Plane.Normal );
    vector3 SphereBotPoint     = SpherePos - ( SphereRadius * Plane.Normal );
    f32     TopDist            = -Plane.Distance( SphereTopPoint );
    f32     BotDist            = -Plane.Distance( SphereBotPoint );
    f32     TopDistToCenterSqr = ( ObjectCenter - SphereTopPoint ).LengthSquared();
    f32     BotDistToCenterSqr = ( ObjectCenter - SphereBotPoint ).LengthSquared();
    
    // Choose point furthest away from object center so that coke is not pushed half way through plane
    f32 IntersectionDist = ( TopDistToCenterSqr > BotDistToCenterSqr ) ? TopDist : BotDist;
    
    // Move a tad further away from the plane for float safety
    IntersectionDist += ( CollisionBackOff * 1.5f ) * x_sign( IntersectionDist );
    
    // Smallest intersection so far?
    if( x_abs( IntersectionDist ) < x_abs( Depth ) )
    {
        // Record
        Depth  = IntersectionDist;
        Normal = Plane.Normal; 
    }
        
    // Point is inside all of NGon edges so sphere intersects
    return TRUE;
}

//==============================================================================

static
xbool SphereIntersectsWorld( const vector3& ObjectCenter,
                             const vector3& SpherePos, f32 SphereRadius, f32 CollisionBackOff,
                             f32& Depth, vector3& Normal )
{
    // Compute bbox of sphere with room for float error
    bbox SphereBBox( SpherePos, SphereRadius + ( CollisionBackOff * 2.0f ) );

    // Collect clusters
    g_PolyCache.BuildClusterList( SphereBBox, 
        object::TYPE_ALL_TYPES, 
        object::ATTR_COLLIDABLE, 
        object::ATTR_COLLISION_PERMEABLE | object::ATTR_LIVING );

    // Clear results
    xbool bIntersect = FALSE;
    
    // Loop over all clusters
    for( s32 iCL = 0; iCL < g_PolyCache.m_nClusters; iCL++ )
    {
        // Lookup cluster
        poly_cache::cluster& CL = *g_PolyCache.m_ClusterList[iCL];

        // Skip cluster if it doesn't intersect the sphere bbox
        if ( !SphereBBox.Intersect( CL.BBox ) )
            continue;

        // Loop over all quads in cluster
        for( s32 iQ = 0; iQ < (s32)CL.nQuads; iQ++ )
        {
            // Skip if bbox does not intersect sphere
            bbox* pBBox = (bbox*)(&CL.pBounds[iQ]);
            if( !SphereBBox.Intersect( *pBBox ) ) 
                continue;

            // Lookup quad vertices
            poly_cache::cluster::quad& QD = CL.pQuad[iQ];
            vector3 Vertices[4];
            Vertices[0] = CL.pPoint[ QD.iP[0] ];
            Vertices[1] = CL.pPoint[ QD.iP[1] ];
            Vertices[2] = CL.pPoint[ QD.iP[2] ];
            Vertices[3] = CL.pPoint[ QD.iP[3] ];
            s32 nVertices = ( CL.pBounds[iQ].Flags & BOUNDS_IS_QUAD ) ? 4 : 3;

            // Does sphere intersect NGon?
            bIntersect |= SphereIntersectsNGon( ObjectCenter, 
                                                SpherePos, SphereRadius, CollisionBackOff,
                                                Vertices, nVertices, Depth, Normal );
        }
    }

    // Return result
    return bIntersect;
}

//=========================================================================

xbool CokeCanIntersectsWorld( const vector3& ParticlePos0, 
                              const vector3& ParticlePos1, 
                                    f32      ParticleRadius, 
                                    f32      CollisionBackOff,
                                    f32&     Depth, 
                                    vector3& Normal )
{
    // Clear results
    Depth = F32_MAX;
    Normal.Zero();
    
    // Check for intersection and setup smallest depth
    xbool   bIntersect = FALSE;
    vector3 Center( ( ParticlePos0 + ParticlePos1 ) * 0.5f );
    bIntersect  = SphereIntersectsWorld( Center, ParticlePos0, ParticleRadius, CollisionBackOff, Depth, Normal );
    bIntersect |= SphereIntersectsWorld( Center, ParticlePos1, ParticleRadius, CollisionBackOff, Depth, Normal );
    
    // Return result
    return bIntersect;
}    

#endif  //#ifdef X_EDITOR


//=========================================================================
// OBJECT DESC
//=========================================================================

static struct coke_can_desc : public object_desc
{
    coke_can_desc( void ) : object_desc( object::TYPE_COKE_CAN, 
                                        "CokeCan",
                                        "PROPS",

                                        object::ATTR_SPACIAL_ENTRY          |
										object::ATTR_NEEDS_LOGIC_TIME		|
                                        object::ATTR_SOUND_SOURCE			|
                                        object::ATTR_COLLIDABLE             | 
                                        object::ATTR_BLOCKS_ALL_PROJECTILES | 
                                        object::ATTR_BLOCKS_ALL_ACTORS      | 
                                        object::ATTR_BLOCKS_RAGDOLL         | 
                                        object::ATTR_DAMAGEABLE             |
                                        object::ATTR_NO_RUNTIME_SAVE        |
                                        object::ATTR_RENDERABLE,

                                        FLAGS_GENERIC_EDITOR_CREATE | FLAGS_NO_ICON |
                                        FLAGS_IS_DYNAMIC ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new coke_can; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

} s_CokeCanDesc;

//=========================================================================

const object_desc& coke_can::GetTypeDesc( void ) const
{
    return s_CokeCanDesc;
}

//=========================================================================

const object_desc& coke_can::GetObjectType( void )
{
    return s_CokeCanDesc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

coke_can::coke_can( void ) :
    m_bInitialized      ( FALSE ),  // TRUE if initialized
    m_bOnGround         ( TRUE ),   // TRUE if lying on the ground
    m_ActiveCount       ( 0 ),      // Forces physics to update
    m_ParticleRadius    ( 0 ),      // Radius of particles
    m_ParticleDist      ( 0 ),      // Constraint distance
    m_Roll              ( 0 ),      // Roll of can
    m_RollSpeed         ( 0 ),      // Roll speed of can
    m_DeltaTime         ( 0 ),      // Accumulated delta time
    m_iMajorAxis        ( 0 ),      // Longest axis of can
    m_MinInitVel        ( 0,0,0 ),  // Min initial velocity
    m_MaxInitVel        ( 0,0,0 ),  // Max initial velocity
    m_RollAudioID       ( 0 )       // Can rolling audio id
{
    m_FloorProperties.Init( 100.0f, 0.5f );
    m_iProfile = PROFILE_CAN;
}

//=========================================================================

coke_can::~coke_can()
{
}

//=========================================================================

void coke_can::OnMove( const vector3& NewPos )
{
    // Call base class
    object::OnMove( NewPos );

#ifdef X_EDITOR
    // If being moved in the editor, re-initialize
    if( ( !g_game_running ) && ( GetAttrBits() & ( object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT ) ) )
    {
        InitPhysics();
    }
#endif
}

//=========================================================================

void coke_can::OnTransform( const matrix4& L2W )
{
    // Call base class
    object::OnTransform( L2W );

#ifdef X_EDITOR
    // If being moved in the editor, re-initialize
    if( ( !g_game_running ) && ( GetAttrBits() & ( object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT ) ) )
    {
        InitPhysics();
    }
#endif
}

//=========================================================================

bbox coke_can::GetLocalBBox( void ) const
{
    // Grab from geometry?
    geom* pGeom = m_SkinInst.GetGeom();
    if( pGeom )
    {
        return bbox( pGeom->m_BBox.GetCenter(), pGeom->m_BBox.GetRadius() );
    }

    return bbox( vector3( -50, -50, -50 ), vector3( 50,50,50 ) );
}

//===========================================================================

bbox coke_can::GetGeomBBox( void ) const
{
    // Grab from geometry?
    geom* pGeom = m_SkinInst.GetGeom();
    if( pGeom )
        return pGeom->m_BBox;

    return bbox( vector3( -50, -50, -50 ), vector3( 50,50,50 ) );
}

//===========================================================================

void coke_can::OnRender( void )
{
    // Geometry present?
    skin_geom* pSkinGeom = m_SkinInst.GetSkinGeom();
    if( !pSkinGeom )
        return;
        
    // Can only support 1 boned cans!
    ASSERTS( pSkinGeom->m_nBones == 1, "Coke cans can only have 1 bone!!" );        

    // Compute LOD mask
    u64 LODMask = m_SkinInst.GetLODMask( GetL2W() );
    if( LODMask == 0 )
        return;

    // Setup render flags
    u32    Flags   = ( GetFlagBits() & object::FLAG_CHECK_PLANES ) ? render::CLIPPED : 0;
    xcolor Ambient = m_FloorProperties.GetColor();
    
    // Render that puppy!
    matrix4* pBackedUpMtx = ( matrix4* )smem_BufferAlloc( sizeof( matrix4 ) );
    ASSERT( pBackedUpMtx );
    *pBackedUpMtx = GetL2W();

#ifdef X_EDITOR
    // Render transparent if selected in editor so you can see collision
    if ( GetAttrBits() & object::ATTR_EDITOR_SELECTED )
    {
        // Render collision now before the z buffer is primed which will stop the coll render!
        OnColRender( TRUE );

        // Render can transparent        
        Flags |= render::FADING_ALPHA;
        Ambient.A  = 192;
    }
#endif

    m_SkinInst.Render( pBackedUpMtx,
                       pBackedUpMtx,
                       1, Flags | GetRenderMode(),
                       LODMask, 
                       Ambient );

#ifndef X_RETAIL
    // Lookup profile
    const coke_can_tweaks& Profile = GetProfile();

    // Use this to show when cans are active
    if( ( DEBUG_COKE_CAN ) && ( GetEnergy() > Profile.ACTIVE_ENERGY ) )
        OnColRender( FALSE );
#endif
}

//===============================================================================

#ifndef X_RETAIL
void coke_can::OnColRender( xbool bRenderHigh )
{
    ( void )bRenderHigh;

    // Render world bbox
    draw_SetL2W( GetL2W() );
    draw_BBox( GetGeomBBox(), XCOLOR_YELLOW );

    // Render the particles
    draw_ClearL2W();
    draw_Sphere( m_Particles[0].m_Pos, m_ParticleRadius, XCOLOR_GREEN );
    draw_Sphere( m_Particles[1].m_Pos, m_ParticleRadius, XCOLOR_GREEN );

    // Render constraint
    draw_Line( m_Particles[0].m_Pos, m_Particles[1].m_Pos, XCOLOR_RED );

    // Show energy
    //draw_Label( GetPosition(), XCOLOR_BLUE, "Energy:%f", GetEnergy() );

    // Show 1 of the particle speed
    //draw_Label( m_Particles[0].m_Pos, XCOLOR_RED, "Speed:%f", m_Particles[0].GetVelocity().Length() );
    draw_Label( GetPosition(), XCOLOR_BLUE, "RollSpeed:%f", x_abs( m_RollSpeed ) );
}
#endif // X_RETAIL

//===============================================================================

void coke_can::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "coke_can::OnAdvanceLogic" );

    // This fixes blue-printed cans from not working properly
    // Initialized physics?
    if( !m_bInitialized )
    {
        m_bInitialized = TRUE;
        InitPhysics();
    }

    // Lookup profile
    const coke_can_tweaks& Profile = k_CokeCanTweak[ m_iProfile ];
    
    // Only update physics if can is active or moving
    if( ( m_ActiveCount > 0 ) || ( GetEnergy() > Profile.ACTIVE_ENERGY ) )
    {
        // Update physics?
        m_DeltaTime += DeltaTime;
        if( m_DeltaTime >= Profile.TIME_STEP )
        {
            // Update time
            m_DeltaTime -= Profile.TIME_STEP;

            // Update active count
            if( m_ActiveCount > 0 )
                m_ActiveCount--;

            // Update?
            ApplyDamping();
            Integrate( Profile.TIME_STEP );
            ApplyConstraints();
            UpdateL2W();

            // Update roll
            m_Roll      += m_RollSpeed;
            m_RollSpeed -= m_RollSpeed * 0.5f; // Dampen ( this slows down spinning in air ) 

            // Keep active?
            if( GetEnergy() > Profile.ACTIVE_ENERGY )
                m_ActiveCount = Profile.ACTIVE_FRAMES;
            else
            {
                // Turn off roll audio?
                if( ( m_ActiveCount == 0 ) && m_RollAudioID )
                {
                    g_AudioMgr.Release( m_RollAudioID, 0.5f );
                    m_RollAudioID = 0;
                }
            }
        }

        // Keep position up to date
        vector3 NewPos = ( m_Particles[0].m_Pos + m_Particles[1].m_Pos ) * 0.5f;
        OnMove( NewPos );
    }

    // Update floor tracking
    m_FloorProperties.Update( GetPosition(), DeltaTime );
}

//===============================================================================

void coke_can::OnPain( const pain& Pain )
{
    // Lookup profile
    const coke_can_tweaks& Profile = GetProfile();

    // Prepare pain
    Pain.ComputeDamageAndForce( GetLogicalName(), GetGuid(), GetBBox().GetCenter() );
    
    // Lookup pain info
    const vector3& Pos   = Pain.GetPosition();
          vector3  Dir   = Pain.GetDirection();
          f32      Force = Pain.GetForce() * Profile.TIME_STEP;

    // Compute main axis between 2 spheres
    f32     T    = 0.5f;
    vector3 Axis = m_Particles[1].m_Pos - m_Particles[0].m_Pos;
    if( Axis.SafeNormalizeAndScale( m_ParticleRadius ) )
    {   
        // Compute far ends of can
        vector3 S = m_Particles[0].m_Pos - Axis;     
        vector3 E = m_Particles[1].m_Pos + Axis;     
        
        // Get distance ratio of pain between ends of can
        T = Pos.GetClosestPToLSegRatio( S, E );
    }

    // Hit by bullet?
    if( Pain.IsDirectHit() )
    {
        // Scale force
        Force *= Profile.PAIN_BULLET_FORCE_SCALE;
    
        // Compute impulse based 100% pain direction
        vector3 Impulse = Force * Dir;
        
        // Apply to particles
        m_Particles[0].m_LastPos -= Impulse * ( 1.0f - T );
        m_Particles[1].m_LastPos -= Impulse * T;
    }
    else    
    {
        // Scale force
        Force *= Profile.PAIN_EXPLOSION_FORCE_SCALE;
    
        // Compute direction from explosion
        Dir = GetPosition() - Pos;
        if( !Dir.SafeNormalize() )
            Dir.Set( 0.0f, 1.0f, 0.0f );

        // Apply directional + upwards blast to particles
        vector3 Impulse = Force * ( ( 0.25f * Dir ) + ( vector3( 0.0f, 0.75f, 0.0f ) ) );
        m_Particles[0].m_LastPos -= Impulse;
        m_Particles[1].m_LastPos -= Impulse;

        // Apply spinning blast to particles
        Impulse = Force * 0.125f * ( Dir + vector3( 0.0f, 1.0f, 0.0f ) );
        m_Particles[0].m_LastPos -= Impulse * ( 1.0f - T );
        m_Particles[1].m_LastPos -= Impulse * T;
        
        // Flag as in air so damping doesn't happen until it lands again
        m_bOnGround = FALSE;
    }
    
    // Activate physics
    m_ActiveCount = Profile.ACTIVE_FRAMES;    

    // Audio
    voice_id VoiceID = g_AudioMgr.Play( Profile.SFX_BULLET_IMPACT, Pos, GetZone1(), TRUE );
    g_AudioManager.NewAudioAlert( VoiceID, audio_manager::BULLET_IMPACTS, GetPosition(), GetZone1(), GetGuid() ); 
    
}
    
//===============================================================================

void coke_can::OnColCheck( void )
{
#ifdef X_EDITOR    
    // Editor select?
    if( g_CollisionMgr.IsEditorSelectRay() )
    {
        // Apply object orientated bounding box test
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplyOOBBox( GetGeomBBox(), GetL2W() );
        g_CollisionMgr.EndApply();
        return;
    }
#endif
    
    // Get moving object
    guid    MovingGuid = g_CollisionMgr.GetMovingObjGuid() ;
    object* pObject    = g_ObjMgr.GetObjectByGuid(MovingGuid) ;

    // Collide with bullets, projectiles, or melee?
    if (        ( pObject ) 
            &&  (       ( pObject->IsKindOf( base_projectile::GetRTTI() ) )     // Normal projectiles
                    ||  ( pObject->IsKindOf( net_proj::GetRTTI() ) )            // Net projectiles
                    ||  ( pObject->IsKindOf( actor::GetRTTI() ) ) ) )           // For melee
    {
        // Apply object orientated bounding box test
        g_CollisionMgr.StartApply( GetGuid() );
        g_CollisionMgr.ApplyOOBBox( GetGeomBBox(), GetL2W() );
        g_CollisionMgr.EndApply();
    }
}

//===============================================================================
// Editor functions
//===============================================================================

void coke_can::OnEnumProp( prop_enum&    List )
{
    // Call base class
    object::OnEnumProp( List );

    // Geometry properties
    m_SkinInst.OnEnumProp( List );

    // Coke can properies
    List.PropEnumHeader ( "CokeCan", "Physically coke can", 0 ); 
    List.PropEnumEnum   ( "CokeCan\\Physics Model",  "CAN\0BARREL\0", "Physics model of the object.", PROP_TYPE_EXPOSE );
    List.PropEnumVector3( "CokeCan\\MinInitVel", "Minimum initial velocity of can in local space", 0 );
    List.PropEnumVector3( "CokeCan\\MaxInitVel", "Maximum initial velocity of can in local space", 0 );

#ifdef X_EDITOR    
    List.PropEnumFloat ( "CokeCan\\ParticleRadius",    "Radius of collision particles inside coke can", PROP_TYPE_MUST_ENUM | PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SAVE | PROP_TYPE_DONT_EXPORT );
    List.PropEnumButton( "CokeCan\\FixWorldIntersection", "Puts can in collision free position", PROP_TYPE_MUST_ENUM );
#endif    
}

//===============================================================================

xbool coke_can::OnProperty( prop_query&   I )
{
    // Call base class
    if( object::OnProperty( I ) )
        return TRUE;

    // Physics model
    if( I.IsVar( "CokeCan\\Physics Model" ) )
    {
        if( I.IsRead() )
        {
            switch( m_iProfile )
            {
            case PROFILE_CAN:       I.SetVarEnum( "CAN" );        break;
            case PROFILE_BARREL:    I.SetVarEnum( "BARREL" );     break;
            }
        }
        else
        {
            if( !x_stricmp( I.GetVarEnum(), "CAN" ) )
                m_iProfile = PROFILE_CAN;
            else if( !x_stricmp( I.GetVarEnum(), "BARREL" ) )
                m_iProfile = PROFILE_BARREL;            
        }
        return TRUE;
    }


    // Geometry
    if( m_SkinInst.OnProperty( I ) )
    {
        // Was geometry just selected?
        if( ( !I.IsRead() ) && ( I.IsVar( "RenderInst\\File" ) ) )
        {
            // Setup physics again
            InitPhysics();
        }

        return TRUE;
    }

    // Min init velocity?
    if( I.VarVector3( "CokeCan\\MinInitVel", m_MinInitVel ) )
    {
        // Was velocity just updated?
        if( !I.IsRead() )
        {
            // Setup physics again
            InitPhysics();
        }

        return TRUE;
    }

    // Max init velocity?
    if( I.VarVector3( "CokeCan\\MaxInitVel", m_MaxInitVel ) )
    {
        // Was velocity just updated?
        if( !I.IsRead() )
        {
            // Setup physics again
            InitPhysics();
        }

        return TRUE;
    }

#ifdef X_EDITOR    

    // Show collision radius
    if( I.IsVar( "CokeCan\\ParticleRadius" ) )
    {
        // Update UI?
        if( I.IsRead() )
        {
            I.SetVarFloat( m_ParticleRadius );
        }
        
        return TRUE;
    }

    // Move to collision free?
    if( I.IsVar( "CokeCan\\FixWorldIntersection" ) )
    {
        // Update UI?
        if( I.IsRead() )
        {
            I.SetVarButton( "FixWorldIntersection" );
        }            
        else
        {
            // Can only apply if just editing the coke can
            if(        ( !g_game_running  ) 
                    && ( !g_level_loading ) 
                    && ( GetAttrBits() & object::ATTR_EDITOR_SELECTED ) )
            {
                // Clear collision results
                f32     CollisionBackOff = GetProfile().COLLISION_BACKOFF;
                f32     Depth      = F32_MAX;
                vector3 Normal( 0.0f, 0.0f, 0.0f );
                vector3 DeltaPos( 0.0f, 0.0f, 0.0f );
                s32     MaxIters = 100;
                    
                // Does coke can intersect world?
                while( ( MaxIters-- ) && ( CokeCanIntersectsWorld( m_Particles[0].m_Pos + DeltaPos,    // ParticlePos0
                                                                   m_Particles[1].m_Pos + DeltaPos,    // ParticlePos1
                                                                   m_ParticleRadius,                   // ParticleRadius
                                                                   CollisionBackOff,                   // Collision back off dist
                                                                   Depth,                              // Intersection depth
                                                                   Normal ) ) )                        // Intersection normal
                {     
                    // Project out of collision
                    DeltaPos += Depth * Normal;
                }
                
                // Moved?
                if( DeltaPos != vector3( 0.0f, 0.0f, 0.0f ) )
                {
                    // Lookup current position
                    vector3 CurrPos = GetPosition();

                    // If this is a blue-print, use the anchor position!
                    editor_blueprint_ref* pBlueprintRef = NULL;
                    g_WorldEditor.GetBlueprintRefContainingObject2( GetGuid(), &pBlueprintRef );
                    if( pBlueprintRef )
                    {
                        // Use anchor position
                        object* pAnchor = g_ObjMgr.GetObjectByGuid( pBlueprintRef->Anchor );
                        if( pAnchor )
                            CurrPos = pAnchor->GetPosition();
                    }                

                    // Move the position via the world editor so that blue-prints are correctly saved/undo/redo etc
                    prop_query PropQuery;
                    PropQuery.WQueryVector3( "Base\\Position", CurrPos + DeltaPos );
                    g_WorldEditor.OnProperty( PropQuery );
                }                
            }            
        }
                    
        return TRUE;
    }
#endif  //#ifdef X_EDITOR    

    return FALSE;
}

//===============================================================================

// Misc
const coke_can_tweaks& coke_can::GetProfile( void )
{
    // Lookup profile
    ASSERT( m_iProfile >= 0 );
    ASSERT( m_iProfile < PROFILE_COUNT );
    return k_CokeCanTweak[ m_iProfile ];
}

//===============================================================================

// Simple structure used for computing bbox axis info
struct axis_info
{
    s32 Index;
    f32 Length;
};

// Sorts axis info based on axis length
s32 CompareAxisInfo( const void* pA, const void* pB )
{
    // Lookup anims
    axis_info* pInfoA = ( axis_info* )pA;
    axis_info* pInfoB = ( axis_info* )pB;

    if( pInfoA->Length > pInfoB->Length )
        return 1;
    if( pInfoA->Length < pInfoB->Length )
        return -1;

    return 0;
}

//===============================================================================

void coke_can::InitPhysics( void )
{
    s32 i;

    // Lookup geometry
    geom* pGeom = m_SkinInst.GetGeom();
    if( !pGeom )
        return;

    // Lookup profile
    const coke_can_tweaks& Profile = GetProfile();

    // Lookup geometry info
    bbox& BBox = pGeom->m_BBox;

    // Compute axis info
    axis_info AxisInfo[3];
    for ( i = 0; i < 3; i++ )
    {
        AxisInfo[i].Index  = i;
        AxisInfo[i].Length = BBox.Max[i] - BBox.Min[i];
    }

    // Sort from smallest to largest
    x_qsort( AxisInfo, 3, sizeof( axis_info ), CompareAxisInfo );
    
    // Lookup axis info
    s32 iAxis0 = AxisInfo[0].Index;
    s32 iAxis1 = AxisInfo[1].Index;
    s32 iAxis2 = AxisInfo[2].Index;

    // Compute radius to use
    m_ParticleRadius = 0.5f * x_max( AxisInfo[0].Length, AxisInfo[1].Length );

    // Compute height
    //f32 Height = BBox.Max[iAxis2] - BBox.Min[iAxis2];
    
    // Setup positions in local space
    vector3 LocalTop;
    LocalTop[iAxis0] = ( BBox.Min[iAxis0] + BBox.Max[iAxis0] ) * 0.5f;
    LocalTop[iAxis1] = ( BBox.Min[iAxis1] + BBox.Max[iAxis1] ) * 0.5f;
    LocalTop[iAxis2] = BBox.Max[iAxis2] - m_ParticleRadius;

    vector3 LocalBot;
    LocalBot[iAxis0] = ( BBox.Min[iAxis0] + BBox.Max[iAxis0] ) * 0.5f;
    LocalBot[iAxis1] = ( BBox.Min[iAxis1] + BBox.Max[iAxis1] ) * 0.5f;
    LocalBot[iAxis2] = BBox.Min[iAxis2] + m_ParticleRadius;

    // Compute distance constraint
    m_ParticleDist = x_abs( LocalTop[iAxis2] - LocalBot[iAxis2] );

    // Put into world space
    const matrix4& L2W = GetL2W();
    vector3 WorldTop = L2W * LocalTop;
    vector3 WorldBot = L2W * LocalBot;

    // Init particles
    m_Particles[0].m_BindPos = LocalTop;
    m_Particles[0].m_Pos = 
    m_Particles[0].m_LastPos = 
    m_Particles[0].m_LastCollPos = WorldTop;

    m_Particles[1].m_BindPos = LocalBot;
    m_Particles[1].m_Pos = 
    m_Particles[1].m_LastPos = 
    m_Particles[1].m_LastCollPos = WorldBot;

    // Compute local space random init vel
    vector3 InitLocalVel( x_frand( m_MinInitVel.GetX(), m_MaxInitVel.GetX() ),
                          x_frand( m_MinInitVel.GetY(), m_MaxInitVel.GetY() ),
                          x_frand( m_MinInitVel.GetZ(), m_MaxInitVel.GetZ() ) );

    // Compute world space velocity and take time step into account to match integration
    vector3 InitWorldVel = L2W.RotateVector( InitLocalVel ) * Profile.TIME_STEP;

    // Setup init velocity
    m_Particles[0].SetVelocity( InitWorldVel );
    m_Particles[1].SetVelocity( InitWorldVel );

    // Keep major axis
    m_iMajorAxis = iAxis2;

    // Clear roll
    m_Roll      = 0.0f;
    m_RollSpeed = 0.0f;

    // Force local bbox to recompute
    SetFlagBits( GetFlagBits() | object::FLAG_DIRTY_TRANSFORM );
}

//===============================================================================

f32 coke_can::GetEnergy( void )
{
    // Accumulate velocities squared
    f32 E;
    E  = m_Particles[0].GetVelocity().LengthSquared();
    E += m_Particles[1].GetVelocity().LengthSquared();
    
    return E;
}

//===============================================================================

void coke_can::UpdateL2W( void )
{
    // Clear L2W
    matrix4 L2W;
    L2W.Identity();

    // Compute initial direction
    vector3 InitDir = m_Particles[1].m_BindPos - m_Particles[0].m_BindPos;
    vector3 CurrDir = m_Particles[1].m_Pos     - m_Particles[0].m_Pos;

    // Compute rotation
    quaternion Rot;
    Rot.Setup( InitDir, CurrDir );
    L2W.SetRotation( Rot );

    // Compute translation
    L2W.SetTranslation( 0.5f * ( m_Particles[0].m_Pos + m_Particles[1].m_Pos ) );

    // Apply roll
    switch( m_iMajorAxis )
    {
        case 0: L2W.PreRotateX( m_Roll );  break;
        case 1: L2W.PreRotateY( m_Roll );  break;
        case 2: L2W.PreRotateZ( m_Roll );  break;
    }

    // Update
    OnTransform( L2W );
}

//===============================================================================

void coke_can::Integrate( f32 DeltaTime )
{
    CONTEXT( "coke_can::Integrate" );

    // Nothing to do?
    if( DeltaTime == 0 )
        return;

    // Lookup profile
    const coke_can_tweaks& Profile = GetProfile();

    // Setup constants
    vector3 Gravity( 0,Profile.GRAVITY,0 );
    f32     DeltaTimeSquared = x_sqr( DeltaTime );

    // Apply verlet integration to all particles
    for ( s32 i = 0; i < 2; i++ )
    {
        // Lookup particle
        particle& Particle = m_Particles[i];

        // Compute movement
        vector3 Pos   = Particle.m_Pos;
        vector3 Vel   = Particle.m_Pos - Particle.m_LastPos;
        vector3 Accel = ( Gravity * DeltaTimeSquared );

        // Move
        Particle.m_Pos += Vel + Accel;

        // Update last position
        Particle.m_LastPos = Pos;
    }
}

//===============================================================================

void coke_can::ApplyEqualDistConstraint( particle& ParticleA, particle& ParticleB, f32 EqualDist )
{
    // Get distance between particles
    vector3 Delta   = ParticleB.m_Pos - ParticleA.m_Pos;
    f32     DistSqr = Delta.LengthSquared();
    if( DistSqr < 0.001f )
        return;

    // Move to target dist
    f32 Dist = x_sqrt( DistSqr );
    f32 Diff = ( Dist - EqualDist ) / Dist;

    // Scale
    Delta *= Diff * 0.5f;

    // Apply deltas
    ParticleA.m_Pos += Delta;
    ParticleB.m_Pos -= Delta;
}

//===============================================================================

f32 coke_can::ApplyMinDistConstraint( particle& ParticleA, particle& ParticleB, f32 MinDist, 
                                      f32 TotalInvMass, f32 InvMassA, f32 InvMassB )
{
    // Make sure mass is setup correctly
    ASSERT( TotalInvMass != 0.0f );
    
    // Get distance between particles
    vector3 Delta   = ParticleB.m_Pos - ParticleA.m_Pos;
    f32     DistSqr = Delta.LengthSquared();

    // No collision
    if( DistSqr < 0.001f )
        return 0.0f;

    // Particles too close?
    f32 Dist = x_sqrt( DistSqr );
    if( Dist < MinDist )
    {
        // Move apart
        f32 Diff = ( Dist - MinDist ) / ( Dist * TotalInvMass );

        // Scale
        Delta *= Diff;

        // Apply deltas
        ParticleA.m_Pos += InvMassA * Delta;
        ParticleB.m_Pos -= InvMassB * Delta;

        // Compute relative impact velocity
        vector3 Vel = ParticleA.GetVelocity() - ParticleB.GetVelocity();

        // Return speed squared
        return Vel.LengthSquared();
    }

    // No collision
    return 0.0f;
}

//===============================================================================

void coke_can::ApplyDistConstraints( void )
{
    // Keep particles at set distance
    ApplyEqualDistConstraint( m_Particles[0], m_Particles[1], m_ParticleDist );
}

//==============================================================================

xbool coke_can::ApplyCylinderConstraint ( const vector3& Bottom, const vector3& Top, f32 Radius, vector3& CollNorm )
{
    // Compute radius info taking particle radius into account
    Radius += m_ParticleRadius;
    f32 RadiusSqr = Radius * Radius;

    // Loop through all particles looking for collision
    xbool bCollision = FALSE;
    for ( s32 i = 0; i < 2; i++ )
    {
        // Lookup particle
        particle& Particle = m_Particles[i];

        // Get vector and distance to line down middle of cylinder
        vector3 Delta   = Particle.m_Pos.GetClosestVToLSeg( Bottom, Top );
        f32     DistSqr = Delta.LengthSquared();
        if( DistSqr < 0.001f )
            continue;

        // Project out of capped cylinder?
        if( DistSqr < RadiusSqr )
        {
            // Compute distance from cylinder
            f32 Dist    = x_sqrt( DistSqr );
            f32 InvDist = 1.0f / Dist;

            // Record collision
            bCollision = TRUE;
            CollNorm   = Delta * InvDist;

            // Scale and dampen
            f32 Diff = ( Dist - Radius ) * InvDist;
            Delta *= Diff;

            // Project particle out of cylinder
            Particle.m_Pos += Delta;
        }
    }
    
    return bCollision;
}

//==============================================================================

void coke_can::ApplyCollConstraints( void )
{
    CONTEXT( "coke_can::ApplyCollConstraints" );

    s32   i;

    // Lookup profile
    const coke_can_tweaks& Profile = GetProfile();

    // Compute world bbox taking particle velocities into account
    bbox WorldBBox;
    WorldBBox.Clear();
    for ( i = 0; i < 2; i++ )
    {
        // Get particle
        particle& Particle = m_Particles[i];

        // Compute movement distance of particle (min of 1.0f)
        vector3 Delta   = Particle.m_Pos - Particle.m_LastCollPos;
        f32     DistSqr = Delta.LengthSquared();
        f32     Dist    = 1.0f; 
        if( DistSqr > 1.0f )
            Dist = x_sqrt( DistSqr );
            
        // Compute particle movement bbox, taking projection into account
        bbox ParticleBBox( Particle.m_Pos, Dist );
        
        // Accumulate to world bounds
        WorldBBox += ParticleBBox;
    }
    
    // Clear on ground flag
    m_bOnGround = FALSE;

    // Inflate to take particle radius into account and for a bit of safety
    f32 InflateAmt = m_ParticleRadius + Profile.COLL_BBOX_INFLATE + Profile.MIN_COLL_DIST;
    WorldBBox.Inflate( InflateAmt,InflateAmt,InflateAmt ); 

    // Prepare verlet collision by collecting possible collision objects
    VerletCollision_CollectObjects( WorldBBox );

    // Compute world space major axis and roll axis
    vector3 LocalMajorAxis( 0,0,0 );
    LocalMajorAxis[m_iMajorAxis] = 1.0f;
    vector3 MajorAxis = GetL2W().RotateVector( LocalMajorAxis );
    plane   MajorAxisPlane( MajorAxis, 0 );
    vector3 RollAxis   = v3_Cross( vector3( 0, -1, 0 ), MajorAxis );
    
    // Clear max values
    f32     MaxRollSpeed      = 0.0f;
    f32     MaxImpactSpeedSqr = 0.0f;

    // Apply collision constraints
    for ( i = 0; i < 2; i++ )
    {
        // Get particle
        particle& Particle = m_Particles[i];

        // Compute start and end pts
        vector3 S = Particle.m_LastCollPos;
        vector3 E = Particle.m_Pos;
        vector3 Delta = E-S;
        f32     DistSq = Delta.LengthSquared();
        if( DistSq < x_sqr( Profile.MIN_COLL_DIST ) )
            continue;

        sphere_cast Cast;

        // Collision?
        if( VerletCollision_SphereCast( S, E, m_ParticleRadius, Cast ) )
        {
            // Pull back from collision a tad
            f32     T     = Cast.m_CollT;
            f32     Dist  = x_sqrt( DistSq );
            T -= Profile.COLLISION_BACKOFF / Dist;
            if( T < 0 )
                T = 0;

            // Put new start pos at collision pos
            S = S + ( T * Delta );

            // On the ground?
            if( Cast.m_CollPlane.Normal.GetY() > 0.5f )
                m_bOnGround = TRUE;

            // Get penetration depth of the end point we wanted to reach
            Dist = Cast.m_CollPlane.Distance( E ) - m_ParticleRadius;

            // Friction is proportional to the penetration distance and mass
            f32 Friction = -Dist * Profile.FRICTION;
            if( Friction < Profile.MIN_FRICTION )
                Friction = Profile.MIN_FRICTION;
            else                
            if( Friction > Profile.MAX_FRICTION )
                Friction = Profile.MAX_FRICTION;

            // Split vel into components
            vector3 Vel = Particle.GetVelocity();
            vector3 Perp, Para;
            Cast.m_CollPlane.GetComponents( Vel, Para, Perp );

            // Compute impact speed squared into plane and update max
            f32 ImpactSpeedSqr = Perp.LengthSquared();
            MaxImpactSpeedSqr = x_max( MaxImpactSpeedSqr, ImpactSpeedSqr );

            // Get components along major axis
            vector3 MajorPerp, MajorPara;
            MajorAxisPlane.GetComponents( Para, MajorPara, MajorPerp );

            // Apply friction along major axis
            Para = MajorPara + ( ( 1.0f - Profile.MAJOR_AXIS_FRICTION ) * MajorPerp );

            // Apply friction along roll axis
            Para -= Para * Friction;

            // Bounce
            Perp -= Perp * Profile.BOUNCY;

            // Compute new vel
            Vel = Perp + Para;

            // Compute roll speed
            f32 RollSpeed = R_360 * v3_Dot( Para, RollAxis ) / ( 2.0f * PI * m_ParticleRadius );
            if( x_abs( RollSpeed ) > x_abs( MaxRollSpeed ) )
                MaxRollSpeed = RollSpeed;

            // Project end point out of plane that was collided with
            E += Cast.m_CollPlane.Normal * ( -Dist + Profile.COLLISION_BACKOFF );

            // Now see how close we can get to the final projected pos
            if( VerletCollision_SphereCast( S, E, m_ParticleRadius, Cast ) )
            {
                // Pull back from collision a tad
                T     = Cast.m_CollT;
                Delta = E - S;
                Dist  = Delta.Length();
                T -= Profile.COLLISION_BACKOFF / Dist;
                if( T < 0 )
                    T = 0;

                // Setup new end pt
                E = S + ( T * ( E - S ) );
            }

            // Set new velocity
            Particle.SetVelocity( Vel );

            // Set new position
            Particle.m_Pos = E;
        }

        // Update last collision free pos
        Particle.m_LastCollPos = Particle.m_Pos;
    }

    // Update roll speed?
    if( MaxRollSpeed != 0.0f )
    {
        m_RollSpeed = MaxRollSpeed;
    }
    
    // Audio for roll
    f32 RollMagnitude = x_abs( MaxRollSpeed );
    if( RollMagnitude > Profile.AUDIO_MIN_ROLLING_SPEED )
    {
        // Is sound not playing?
        if( m_RollAudioID == 0 )
        {
            // Play the roll!
            m_RollAudioID = g_AudioMgr.PlayVolumeClipped( Profile.SFX_ROLLING, GetPosition(), GetZone1(), TRUE );
        }

        // Now adjust volume and pitch based on the velocity.
        f32 Velocity = x_min( RollMagnitude, Profile.AUDIO_MAX_ROLLING_SPEED ) - Profile.AUDIO_MIN_ROLLING_SPEED;
        f32 Range    = Profile.AUDIO_MAX_ROLLING_SPEED - Profile.AUDIO_MIN_ROLLING_SPEED;
        f32 Scale    = Velocity / Range;
        f32 Volume   = 0.1f  + Scale * 0.9f;  // volume range is [0.10..1.0]
        f32 Pitch    = 0.94f + Scale * 0.06f; // pitch range is  [0.94..1.0]
        g_AudioMgr.SetVolume( m_RollAudioID, Volume );
        g_AudioMgr.SetPitch( m_RollAudioID, Pitch );
    }
    else
    {
        if( m_RollAudioID )
        {
            g_AudioMgr.Release( m_RollAudioID, 0.5f );
            m_RollAudioID = 0;
        }
    }

    // Play audio impact?
    if( MaxImpactSpeedSqr > x_sqr( Profile.AUDIO_IMPACT_SPEED ) )
    {
        // Rob - hookup volume control here if you want to...
        //f32 ImpactSpeed = x_sqrt( ImpactSpeedSqr );
        //f32 Volume = x_min( 1.0f, ImpactSpeed * ?? );

        // Play audio
        g_AudioMgr.PlayVolumeClipped( Profile.SFX_IMPACT_WORLD, GetPosition(), GetZone1(), TRUE );
    }
}

//==============================================================================

void coke_can::ApplyCanConstraints( coke_can& CokeCan )
{
    // Lookup profile
    const coke_can_tweaks& Profile = GetProfile();

    // Compute dist to keep particles away
    f32 MinDist = m_ParticleRadius + CokeCan.m_ParticleRadius;

    // Make sure masses are valid
    ASSERT( Profile.INVERSE_MASS != 0.0f );
    ASSERT( CokeCan.GetProfile().INVERSE_MASS != 0.0f );
    
    // Compute mass info
    f32 InvMassA = Profile.INVERSE_MASS;
    f32 InvMassB = CokeCan.GetProfile().INVERSE_MASS;
    f32 TotalInvMass = InvMassA + InvMassB;
    
    // Keep particles a set distance from each other
    f32 MaxImpactSpeedSqr = 0.0f;
    
    MaxImpactSpeedSqr = x_max( MaxImpactSpeedSqr, ApplyMinDistConstraint( m_Particles[0], CokeCan.m_Particles[0], MinDist,
                                                                          TotalInvMass, InvMassA, InvMassB ) );
    
    MaxImpactSpeedSqr = x_max( MaxImpactSpeedSqr, ApplyMinDistConstraint( m_Particles[0], CokeCan.m_Particles[1], MinDist,
                                                                          TotalInvMass, InvMassA, InvMassB ) );
    
    MaxImpactSpeedSqr = x_max( MaxImpactSpeedSqr, ApplyMinDistConstraint( m_Particles[1], CokeCan.m_Particles[0], MinDist,
                                                                          TotalInvMass, InvMassA, InvMassB ) );
    
    MaxImpactSpeedSqr = x_max( MaxImpactSpeedSqr, ApplyMinDistConstraint( m_Particles[1], CokeCan.m_Particles[1], MinDist,
                                                                          TotalInvMass, InvMassA, InvMassB ) );

    // Play audio?
    if( MaxImpactSpeedSqr > x_sqr( Profile.AUDIO_COLLIDE_SPEED ) )
    {
        // Play some coke can on coke can collision audio!
        g_AudioMgr.PlayVolumeClipped( Profile.SFX_IMPACT_CAN, GetPosition(), GetZone1(), TRUE );
    }
}

//==============================================================================

void coke_can::ApplyCanConstraints( void )
{
    // Lookup profile
    const coke_can_tweaks& Profile = GetProfile();

    // Are we still active?
    xbool bActive = ( GetEnergy() > Profile.ACTIVE_ENERGY );

    // Find all cans
    g_ObjMgr.SelectBBox( object::ATTR_COLLIDABLE, GetBBox(), object::TYPE_COKE_CAN );
    slot_id SlotID = g_ObjMgr.StartLoop();
    while( SlotID != SLOT_NULL )
    {
        // Lookup object
        coke_can* pCokeCan = ( coke_can* )g_ObjMgr.GetObjectBySlot( SlotID );
        ASSERT( pCokeCan );

        // Collide with the coke can ( but not self )
        if( this != pCokeCan )
        {
            // Make the other can active ( could be stacked on top )?
            if( bActive ) 
                pCokeCan->m_ActiveCount = Profile.ACTIVE_FRAMES;

            // Do collision
            ApplyCanConstraints( *pCokeCan );
        }

        // Check next object
        SlotID = g_ObjMgr.GetNextResult( SlotID );
    }
    g_ObjMgr.EndLoop();
}

//==============================================================================

void coke_can::ApplyActorConstraints( actor& Actor )
{
    // Get loco
    loco* pLoco = Actor.GetLocoPointer();
    if( !pLoco )
        return;

    // Get physics
    character_physics& Physics = pLoco->m_Physics;

    // Compute capped collision cylinder
    vector3 Bottom = Physics.GetPosition();
    vector3 Top    = Bottom;
    Top.GetY() += Physics.GetColHeight();
    f32     Radius = Physics.GetColRadius();

    // Collide with can?
    vector3 CollNorm;
    if( ApplyCylinderConstraint( Bottom, Top, Radius, CollNorm ) )
    {
        // Lookup profile
        const coke_can_tweaks& Profile = GetProfile();
    
        // Apply a fake impact response by slowing down the player
        Actor.ScaleVelocity( CollNorm, 
                             Profile.ACTOR_COLL_VEL_PERP_SCALE, 
                             Profile.ACTOR_COLL_VEL_PARA_SCALE );
    }
}

//==============================================================================

void coke_can::ApplyActorConstraints( void )
{
    // Check all players
    g_ObjMgr.SelectBBox( object::ATTR_COLLIDABLE, GetBBox(), object::TYPE_PLAYER );
    slot_id SlotID = g_ObjMgr.StartLoop();
    while( SlotID != SLOT_NULL )
    {
        // Lookup object
        player* pPlayer = ( player* )g_ObjMgr.GetObjectBySlot( SlotID );
        ASSERT( pPlayer );

        // Collide
        ApplyActorConstraints( *pPlayer );

        // Check next object
        SlotID = g_ObjMgr.GetNextResult( SlotID );
    }
    g_ObjMgr.EndLoop();
}

//==============================================================================

void coke_can::ApplyConstraints( void )
{
    // Collide with other cans
    ApplyCanConstraints();

    // Apply collision
    ApplyCollConstraints();

    // Apply distance contraint
    ApplyDistConstraints();
}

//==============================================================================

void coke_can::ApplyDamping( void )
{
    s32 i;

    // Lookup profile
    const coke_can_tweaks& Profile = GetProfile();

    // Clamp speed
    for ( i = 0; i < 2; i++ )
    {
        // Get particle
        particle& Particle = m_Particles[i];

        // Compute vel and speed
        vector3 Vel      = Particle.GetVelocity();
        f32     SpeedSqr = Vel.LengthSquared();
        
        // Clamp speed?
        if( SpeedSqr > x_sqr( Profile.MAX_SPEED ) )
        {
            // Scale down speed
            f32 Speed = x_sqrt( SpeedSqr );
            f32 Scale = Profile.MAX_SPEED / Speed;
            Vel *= Scale;

            // Set new vel
            Particle.SetVelocity( Vel );
        }
    }

    // Compute center of mass velocity
    vector3 CenterVel( 0,0,0 );
    for ( i = 0; i < 2; i++ )
        CenterVel += m_Particles[i].GetVelocity();
    CenterVel *= 0.5f;

    // Apply damping
    for ( i = 0; i < 2; i++ )
    {
        // Get particle
        particle& Particle = m_Particles[i];

        // Compute angular and linear velocity
        vector3 Vel        = Particle.GetVelocity();
        vector3 AngularVel = Vel - CenterVel;
        vector3 LinearVel  = Vel - AngularVel;
            
        // Dampen
        if( m_bOnGround )
        {
            LinearVel  -= LinearVel  * Profile.GROUND_LINEAR_DAMPEN;
            AngularVel -= AngularVel * Profile.GROUND_ANGULAR_DAMPEN;
        }
        else
        {
            LinearVel  -= LinearVel  * Profile.AIR_LINEAR_DAMPEN;
            AngularVel -= AngularVel * Profile.AIR_ANGULAR_DAMPEN;
        }
        
        // Compute new vel
        Vel = LinearVel + AngularVel;
        
        // Set new vel
        Particle.SetVelocity( Vel );
    }
}

//==============================================================================

void coke_can::OnColNotify( object& Obj )
{
    
    if( Obj.IsKindOf( actor::GetRTTI() ) )
    {
        actor& Actor = actor::GetSafeType( Obj );
        ApplyActorConstraints( Actor );
    }
    else
    {
    }
}

//==============================================================================

#ifdef X_EDITOR

s32 coke_can::OnValidateProperties( xstring& ErrorMsg )
{
    // Does coke can intersect world?
    f32     Depth;
    vector3 Normal;
    f32     CollisionBackOff = GetProfile().COLLISION_BACKOFF;
    if( CokeCanIntersectsWorld( m_Particles[0].m_Pos,   // ParticlePos0
                                m_Particles[1].m_Pos,   // ParticlePos1
                                m_ParticleRadius,       // ParticleRadius
                                CollisionBackOff,       // Collision back off dist
                                Depth,                  // Intersection depth
                                Normal ) )              // Intersection normal
    {    
        // Report error
        ErrorMsg += "COKE CAN IS INTERSECTING THE WORLD!\n\nThis will cause the coke can to fall half way though the floor etc.\n\nUse the \"FixWorldIntersection\" property button or re-position manually to resolve this issue.\n";
        return 1;
    }
    
    // No error
    return 0;
}
#endif // X_EDITOR

//==============================================================================
