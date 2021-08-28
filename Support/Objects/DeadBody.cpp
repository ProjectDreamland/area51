#include "DeadBody.hpp"
#include "e_ScratchMem.hpp"

#include "Entropy\Entropy.hpp"
#include "Characters\Character.hpp"
#include "Loco\Loco.hpp"
#include "Objects\BaseProjectile.hpp"
#include "Decals\DecalMgr.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Characters\ActorEffects.hpp"

static const f32    FLOOR_RAY_TEST_EXTENT         = 50.0f;
static const s32    MAX_DEAD_BODIES               = 4;
static const f32    FADEOUT_START_TIME            = 8.0f;
static const f32    FADEOUT_TIME                  = 2.0f;

//=========================================================================
// EXTERNS
//=========================================================================

#ifdef X_EDITOR
extern xbool g_game_running ;
#endif // X_EDITOR

//=========================================================================
// OBJECT DESC
//=========================================================================

static struct dead_body_desc : public object_desc
{
    dead_body_desc( void ) : object_desc( object::TYPE_DEAD_BODY, 
                                        "Dead Body",
                                        "AI",

                                        object::ATTR_SPACIAL_ENTRY          |
										object::ATTR_NEEDS_LOGIC_TIME		|
                                        object::ATTR_SOUND_SOURCE			|
                                        object::ATTR_COLLIDABLE             | 
                                        object::ATTR_BLOCKS_ALL_PROJECTILES | 
                                        object::ATTR_BLOCKS_RAGDOLL         | 
                                        object::ATTR_BLOCKS_SMALL_DEBRIS    | 
                                        object::ATTR_DAMAGEABLE             |
                                        object::ATTR_NO_RUNTIME_SAVE        |
                                        object::ATTR_RENDERABLE             |
                                        object::ATTR_TRANSPARENT,

                                        FLAGS_GENERIC_EDITOR_CREATE | FLAGS_NO_ICON |
                                        FLAGS_IS_DYNAMIC ) {}

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new dead_body; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

} s_DeadBodyDesc;

//=========================================================================

const object_desc& dead_body::GetTypeDesc( void ) const
{
    return s_DeadBodyDesc;
}

//=========================================================================

const object_desc& dead_body::GetObjectType( void )
{
    return s_DeadBodyDesc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

dead_body::dead_body( void ) :
    m_bPhysics(FALSE),
    m_bCreatedBlood(FALSE),
    m_bCanDelete(FALSE),
    m_bPermanent(FALSE),
    m_bDestroy(FALSE),
    m_bDrainable(FALSE),
    m_TimeAlive(0.0f),
    m_NoEnergyTimer(0),
    m_pActorEffects(NULL),
    m_AnimGroupName(-1),
    m_AnimName(-1),
    m_AnimFrame(0),
    m_SimulationTime(0.0f),
    m_RagdollType(ragdoll::TYPE_CIVILIAN),
	m_BloodDecalGroup(-1)
{
    m_FloorProperties.Init( 100.0f, 0.5f );
    
    // This assumes that the dead body is about 2.5 meters tall.  Then is about 2 
    // meters around.  The bbox is center around the eyes of the player.
    m_ZoneTracker.SetBBox( bbox( vector3( -100, -200, -100 ), 
                                 vector3(  100,   50,  100 ) ) );
}

//=========================================================================

dead_body::~dead_body()
{
    if( m_pActorEffects )
    {
        delete m_pActorEffects;
        m_pActorEffects = NULL;
    }
}

//=========================================================================

void dead_body::OnKill( void )
{
}

//=========================================================================

void dead_body::OnMove( const vector3& NewPos )
{
    // Call base class
    object::OnMove( NewPos );

#ifdef X_EDITOR
    // If being moved in the editor, re-initialize
    if ( (!g_game_running) && (GetAttrBits() & (object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT)) )
        InitializeEditorPlaced() ;
#endif // X_EDITOR

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, NewPos );
}

//=========================================================================

void dead_body::OnTransform( const matrix4& L2W )
{
    // Call base class
    object::OnTransform( L2W );

#ifdef X_EDITOR
    // If being moved in the editor, re-initialize
    if ( (!g_game_running) && (GetAttrBits() & (object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT)) )
        InitializeEditorPlaced() ;
#endif // X_EDITOR

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, L2W.GetTranslation() );
}

//=========================================================================

bbox dead_body::GetLocalBBox( void ) const
{
    return bbox( vector3(-150, -150, -150), vector3(150,150,150) );
}

//===========================================================================

void dead_body::LimitDeadBodyCount( void )
{
    // Should only be called when a non-permanent body is created.
    ASSERT( !m_bPermanent );
    ASSERT( !m_bDestroy );

    // Find the oldest non-permanent dead body, and non-permanent count.
    dead_body*              pOldestDeadBody = NULL;
    s32                     nDeadBodies     = 0;
    select_slot_iterator    SlotIter;

    g_ObjMgr.SelectByAttribute( object::ATTR_ALL, object::TYPE_DEAD_BODY );

    for( SlotIter.Begin(); !SlotIter.AtEnd(); SlotIter.Next() )
    {
        // Get body.
        dead_body* pDeadBody = (dead_body*)SlotIter.Get();
        ASSERT( pDeadBody );

        // Don't consider self.
        if( pDeadBody == this )
            continue;

        // Leave permanent bodies.
        if( pDeadBody->m_bPermanent )
            continue;

        // Skip bodies that have already been flagged to be destroyed.
        if( pDeadBody->m_bDestroy )
            continue;

        // Update count.
        nDeadBodies++;

        // Is this the oldest?  (Or the first to make it this far?)
        if( pOldestDeadBody )
        {
            // Is this the oldest?
            if( pDeadBody->m_TimeAlive > pOldestDeadBody->m_TimeAlive )
                pOldestDeadBody = pDeadBody;
        }
        else
        {
            // First to make it this far.
            pOldestDeadBody = pDeadBody;
        }
    }
    SlotIter.End();

    // Flag oldest to be deleted if there are too many.
    if( nDeadBodies > MAX_DEAD_BODIES )
    {
        ASSERT(  pOldestDeadBody != this );
        ASSERT( !pOldestDeadBody->m_bPermanent );
        ASSERT( !pOldestDeadBody->m_bDestroy   );

        pOldestDeadBody->m_bDestroy = TRUE;
    }
}

//===========================================================================

xbool dead_body::Initialize( actor& Actor, xbool doBodyFade, actor_effects* pActorEffects )
{
    // copy in the actor effects
    if( m_pActorEffects )
    {
        delete m_pActorEffects;
    }
    m_pActorEffects = pActorEffects;

    // Lookup geometry and animation info
          skin_inst& SkinInst = Actor.GetSkinInst() ;
    const char* pGeomFileName = SkinInst.GetSkinGeomName() ;
    const char* pAnimFileName = Actor.GetAnimGroupHandle().GetName() ;

    m_bPermanent = !doBodyFade;
    // Initialize the ragdoll
    if ( !m_Ragdoll.Init( pGeomFileName, pAnimFileName, Actor.GetRagdollType(), GetGuid() ) )
        return FALSE;

    // Copy all the skin data
    GetSkinInst() = Actor.GetSkinInst() ;

    // Set the zone info
    m_ZoneTracker = Actor.GetZoneTracker();
    SetZone1( Actor.GetZone1() ) ;
    SetZone2( Actor.GetZone2() ) ;
    
    // Make sure to tell the spatial database where we are
    OnTransform( Actor.GetL2W() );

    // Copy the time when It was created
    m_TimeAlive = 0.0f;
    
    // Copy floor properties
    m_FloorProperties = Actor.GetFloorProperties() ;

    // Now setup the matrices of the ragdoll from the current animation to inherit velocities
    loco* pLoco = Actor.GetLocoPointer() ;
    if (pLoco)
        m_Ragdoll.SetMatrices(pLoco->m_Player, pLoco->GetDeltaPos()) ;

    // Make active
    m_bPhysics = TRUE ;

    // copy the decal data
    m_hBloodDecalPackage.SetName( Actor.GetBloodDecalPackage() );
    m_BloodDecalGroup = Actor.GetBloodDecalGroup();

    // Delete oldest if too many
    if( doBodyFade )
    {
        LimitDeadBodyCount() ;
    }

    // Success
    return TRUE;
}

//===============================================================================

// NOTE: "LimitDeadBodyCount" is not called when using this initialization function
//       (it is only called when creating permanent dead bodies in the editor)
xbool dead_body::Initialize( const char*           pGeomName,
                            const char*           pAnimGroupName,
                            const char*           pAnimName,
                                  s32             AnimFrame,
                                  ragdoll::type   RagdollType )
{
    // Must be valid
    if ( (!pGeomName) || (!pAnimGroupName) || (!pAnimName) )
        return FALSE;

    // Initialize the ragdoll
    if ( !m_Ragdoll.Init( pGeomName, pAnimGroupName, RagdollType, GetGuid() ) )
        return FALSE;

    // Copy the time when It was created
    m_TimeAlive = 0.0f;

    // Lookup animation group
    const anim_group::handle& hAnimGroup = m_Ragdoll.GetAnimGroupHandle() ;
    const anim_group* pAnimGroup = hAnimGroup.GetPointer() ;
    if (!pAnimGroup)
        return FALSE;

    // Lookup animation info
    s32 nBones = pAnimGroup->GetNBones() ;
    s32 iAnim  = pAnimGroup->GetAnimIndex( pAnimName ) ;
    if ( iAnim == -1 )
        return FALSE;
    const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( iAnim ) ;

    // Clamp frame
    s32 LastFrame = x_max(0, AnimInfo.GetNFrames() - 2) ;
    if (AnimFrame > LastFrame)
        AnimFrame = LastFrame ;

    // Allocate temporary memory for keys and matrices
    smem_StackPushMarker();
    byte* pData = smem_StackAlloc( nBones * ( sizeof(anim_key) + sizeof(matrix4) ) );
    if (!pData)
    {
        smem_StackPopToMarker();
        return FALSE;
    }

    // Setup ptrs
    matrix4*  pMatrices = (matrix4*)pData;
    anim_key* pKeys     = (anim_key*)(pData + (nBones * sizeof(matrix4)) );

    // Compute matrices
    AnimInfo.GetInterpKeys( (f32)AnimFrame, pKeys, nBones );
    pAnimGroup->ComputeBonesL2W( GetL2W(), pKeys, nBones, pMatrices, TRUE ) ;

    // Finally, setup the ragdoll particles
    m_Ragdoll.SetMatrices( pMatrices, nBones );

    // Free alloced memory
    smem_StackPopToMarker();

    // Stop any velocity due to constraints being met
    m_Ragdoll.ZeroVelocity() ;

    // Success
    return TRUE;
}

//===============================================================================

xbool dead_body::InitializeEditorPlaced( void )
{
    // Valid?
    if ((!GetSkinInst().GetSkinGeomName()) || (!GetSkinInst().GetSkinGeom()) || (m_AnimGroupName == -1) || (m_AnimName == -1))
        return FALSE;

    // Never delete
    m_bPermanent = TRUE ;

    // Make sure properties are included in game save/load
    SetAttrBits( GetAttrBits() & ~object::ATTR_NO_RUNTIME_SAVE) ;

    // Initialize from properties
    if ( !Initialize(GetSkinInst().GetSkinGeomName(),
                     g_StringMgr.GetString(m_AnimGroupName),
                     g_StringMgr.GetString(m_AnimName),
                     m_AnimFrame,
                     m_RagdollType) )
    {
        return FALSE;
    }

    // Run the physics to make the ragdoll settle and setup floor color
    m_Ragdoll.ZeroDeltaTime() ;
    f32 Step = 1.0f / 30.0f ;
    for ( f32 Time = 0 ; Time < m_SimulationTime ; Time += Step )
    {
        m_Ragdoll.Advance( Step ) ;
    }
    m_Ragdoll.ZeroDeltaTime() ;

    // Turn off physics so body is frozen
    m_Ragdoll.ZeroVelocity() ;

    m_FloorProperties.ForceUpdate(m_Ragdoll.GetCenterPos());
    // Success
    return TRUE;
}

//===============================================================================

matrix4* dead_body::GetBonesForRender( u64 LODMask, s32& nActiveBones )
{
    // Get number of bones to render
    nActiveBones = GetSkinInst().GetNActiveBones(LODMask);

    // are the current matrices valid?
    if ( m_CachedL2Ws.IsValid(nActiveBones) )
        return m_CachedL2Ws.GetMatrices();

    // allocate room for the new matrices
    matrix4* pMatrices = m_CachedL2Ws.GetMatrices(nActiveBones);

    // Compute render matrices
    if (m_Ragdoll.IsInitialized())
        m_Ragdoll.ComputeMatrices(pMatrices, nActiveBones);
    else
    {
        // Ragdoll not present (must be creating a new ragdoll in editor), so just use bind pose
        for (s32 i = 0 ; i < nActiveBones ; i++)
            pMatrices[i] = GetL2W() ;
    }

    // the matrices are no longer dirty
    m_CachedL2Ws.SetDirty(FALSE);

    return m_CachedL2Ws.GetMatrices();
}

//===============================================================================

void dead_body::OnRenderShadowCast( u64 ProjMask )
{
    // Geometry present?
    skin_geom* pSkinGeom = GetSkinInst().GetSkinGeom() ;
    if (!pSkinGeom)
        return ;

    // Compute LOD mask
    u64 LODMask = GetSkinInst().GetLODMask(GetL2W()) ;
    if (LODMask == 0)
        return;

    // Compute LOD mask for the shadow render (by putting zero in for screen size
    // we force the lowest lod)
    u64 ShadLODMask = GetSkinInst().GetLODMask(0);
    if (ShadLODMask == 0)
        return ;

    // get the matrices
    s32 nActiveBones = 0;
    matrix4* pMatrices = GetBonesForRender( LODMask|ShadLODMask, nActiveBones );
    if ( !pMatrices )
        return;

    // Setup render flags
    u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0 ;

    // Render that puppy!
    GetSkinInst().RenderShadowCast( &GetL2W(), 
                             pMatrices, 
                             nActiveBones,
                             Flags,
                             ShadLODMask,
                             ProjMask ) ;
}

//===============================================================================

void dead_body::OnRender( void )
{
    // render any actor effects
    if( m_pActorEffects )
        m_pActorEffects->Render( this );

    // Geometry present?
    skin_geom* pSkinGeom = GetSkinInst().GetSkinGeom() ;
    if (!pSkinGeom)
        return ;

    // Compute LOD mask
    u64 LODMask = GetSkinInst().GetLODMask(GetL2W()) ;
    if (LODMask == 0)
        return;

    // get the matrices
    s32 nActiveBones = 0;
    matrix4* pMatrices = GetBonesForRender( LODMask, nActiveBones );
    if ( !pMatrices )
        return;

    // Setup render flags and fill in the fade amount
    xcolor Ambient = GetFloorColor();
    u32    Flags   = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0 ;
    if ( (!m_bPermanent) && (m_TimeAlive > FADEOUT_START_TIME) )
    {
        Flags |= render::FADING_ALPHA;
        f32 Alpha = 1.0f - ((m_TimeAlive - FADEOUT_START_TIME) / FADEOUT_TIME);
        Alpha = MIN( Alpha, 1.0f );
        Alpha = MAX( Alpha, 0.0f );
        Ambient.A  = (u8)(Alpha*255.0f);
    }

    // Render that puppy!
    GetSkinInst().Render( &GetL2W(), 
                   pMatrices, 
                   nActiveBones,
                   Flags,
                   LODMask,
                   Ambient ) ;
}

//===============================================================================

void dead_body::OnRenderTransparent( void )
{
    if( m_pActorEffects )
    {
        f32 Alpha = 1.0f;
        if ( (!m_bPermanent) && (m_TimeAlive > FADEOUT_START_TIME) )
        {
            Alpha = 1.0f - ((m_TimeAlive - FADEOUT_START_TIME) / FADEOUT_TIME);
            Alpha = MIN( Alpha, 1.0f );
            Alpha = MAX( Alpha, 0.0f );
        }
        m_pActorEffects->RenderTransparent( this, Alpha );
    }
}

//===============================================================================
static f32 RAGDOLL_KINETIC_ENERGY_LIMIT = 1.0f;
static f32 RAGDOLL_NO_ENERGY_TIMEOUT  = 1.0f;

void dead_body::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "dead_body::OnAdvanceLogic" );

    // If no ragdoll, then delete to stop crashes...
    if (!m_Ragdoll.IsInitialized())
    {
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }

    // update any actor effects
    if( m_pActorEffects )
        m_pActorEffects->Update( this, DeltaTime );

    // Flagged to be destroyed?
    if (m_bDestroy)
    {
        ASSERT(!m_bPermanent) ;
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }

    // Time out and delete his body?
    if( (m_bCanDelete) && (!m_bPermanent) && (m_TimeAlive >= (FADEOUT_START_TIME+FADEOUT_TIME)) )
    {
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }

    // Update the timer.
    if ( m_bPermanent )
        m_TimeAlive = 0.0f;
    else
        m_TimeAlive += DeltaTime;

    // Okay lets assume next time we can be deleted
    m_bCanDelete = TRUE;

    // mreed: commented this out so the bodies settle faster and there is less processor draw.
    // Make characters collide with ragdoll
    //m_Ragdoll.ApplyCharacterConstraints() ;

    // Has ragdoll stopped moving?
    if (m_Ragdoll.GetKineticEnergy() < RAGDOLL_KINETIC_ENERGY_LIMIT)
    {
        // Increase timer
        m_NoEnergyTimer += DeltaTime ;

        // Has the ragdoll been idle for over 2 seconds?
        if ( m_NoEnergyTimer > RAGDOLL_NO_ENERGY_TIMEOUT )
        {
            // Stop the physics
            m_bPhysics = FALSE;
            
            // Generate the blood pool just once, after the deadbody has stopped..
            if (!m_bCreatedBlood)
            {
                CreateBloodPool();
                m_bCreatedBlood = TRUE ;
            }
        }
    }
    else
    {
        // Keep updating
        m_NoEnergyTimer = 0 ;
        m_bPhysics      = TRUE ;
    }
    
    // Advance the physics?
    if ( m_bPhysics )
        m_Ragdoll.Advance(DeltaTime) ;

    // Move the object to the new spot
    OnMove( m_Ragdoll.GetCenterPos() );

    // set up the ambient color for rendering
    m_FloorProperties.Update( GetPosition(), DeltaTime );

    // since the ragdoll has been updated, the matrices are no longer valid
    m_CachedL2Ws.SetDirty(TRUE);

    // x_DebugMsg( "Rag: %f\n", m_Ragdoll.GetKineticEnergy() );
}

//===============================================================================

void dead_body::CreateBloodPool( void )
{
    // DBS Jan. 7, 04: I think this was meant to a be a blood pool that would
    // grow as time went on and possibly animate. It was never implemented
}

//===========================================================================

void dead_body::CreateSplatDecalOnGround( const pain &Pain )
{
    (void)Pain;

    // create a splat decal on the ground
    decal_package* pPackage = m_hBloodDecalPackage.GetPointer();
    if ( !pPackage )
        return;

    if ( (m_BloodDecalGroup<0) || (m_BloodDecalGroup>=pPackage->GetNGroups()) )
        return;

    s32 nDecalDefs = pPackage->GetNDecalDefs( m_BloodDecalGroup );
    if ( nDecalDefs == 0 )
        return;
/*
    // figure out what size blood we'd like. The group is ordered: sml, med, big, smear, pool
    s32 iDecalDef;
    f32 Size = 250.0f;
    if ( Pain.DamageR0 > 80.0f )
    {
        iDecalDef = 2;
    }
    else
    {
        if ( x_rand() & 1 )
            iDecalDef = 1;
        else
            iDecalDef = 3;

        if ( Pain.DamageR0 < 25.0f )
            Size = 75.0f;
        else
            Size = 150.0f;
    }

    iDecalDef = MIN(nDecalDefs-2, iDecalDef);
    Size      = x_frand( Size*0.25f, Size );


    // figure out a random orientation for the blood splat
    radian3 BloodOrient;
    BloodOrient.Pitch = x_frand(R_80, R_100);
    BloodOrient.Yaw   = x_frand(R_0,  R_360);
    BloodOrient.Roll  = R_0;
    
    vector3 RayStart = Pain.PtOfImpact;
    RayStart.GetY()      += 10.0f;

    vector3 RayEnd( 0.0f, 0.0f, 1.0f );
    RayEnd.Rotate( BloodOrient );
    RayEnd  = 40.0f * RayEnd;
    RayEnd += RayStart;

    // generate the decal
    const decal_definition& DecalDef = pPackage->GetDecalDef( m_BloodDecalGroup, iDecalDef );
    g_DecalMgr.CreateDecalFromRayCast( DecalDef,
                                       RayStart,
                                       RayEnd,
                                       vector2( Size, Size ),
                                       DecalDef.RandomRoll() );
*/
}

//===============================================================================

void dead_body::OnPain( const pain& Pain )
{
    // if we've already starting fading the body out, don't do any more splatting
    if ( m_TimeAlive > FADEOUT_START_TIME )
        return;

    health_handle Handle(GetLogicalName());
    Pain.ComputeDamageAndForce( Handle, GetGuid(), GetPosition() );

    // Create blood
    CreateSplatDecalOnGround(Pain);

    // Bring back to life...
    m_bCanDelete    = FALSE;  
    m_bPhysics      = TRUE;
    m_NoEnergyTimer = TRUE;

    // Reset creation time so it's put at back of deletion
    m_TimeAlive = 0.0f;

    // Use the hit_type to determine how to react
    switch( Pain.GetHitType() )
    {
    case 0: 
        // Small impact like bullets
        m_Ragdoll.ApplyBlast( Pain.GetPosition(), Pain.GetDirection(), 60, 1000) ;
        particle_emitter::CreateOnPainEffect( Pain, 0, particle_emitter::BLOODY_MESS, FALSE );
        break;
    case 1:
    case 2:
    case 3:
        // Explosive grenade
        m_Ragdoll.ApplyBlast( Pain.GetPosition(), 100.0f, 500.0f );
        m_Ragdoll.ApplyBlast( Pain.GetPosition(), vector3(0,1,0), 100.0f, 1000.0f ) ;
        break;
    case 4:
        // Melee
        m_Ragdoll.ApplyVectorForce( Pain.GetDirection(), 1000.0f );
        particle_emitter::CreateOnPainEffect( Pain, 0, particle_emitter::BLOODY_MESS, FALSE );
        break;
    }
}

//===============================================================================

void dead_body::OnColCheck( void )
{
    // Get moving object
    guid    MovingGuid = g_CollisionMgr.GetMovingObjGuid() ;
    object* pObject    = g_ObjMgr.GetObjectByGuid(MovingGuid) ;
    
#ifdef X_EDITOR
    // Allow ragdoll to be selected in the editor?
    if (g_CollisionMgr.IsEditorSelectRay())
    {
        // Use ragdoll if possible
        if (m_Ragdoll.IsInitialized())
        {
            m_Ragdoll.OnColCheck() ;
            return ;
        }

        // Start with object bbox incase geometry is not present
        bbox BBox = GetBBox() ;

        // Use geometry bbox?
        geom* pGeom = GetSkinInst().GetGeom() ;
        if (pGeom)
        {
            BBox = pGeom->m_BBox ;
            BBox.Transform(GetL2W()) ;
        }
        
        // Apply
        g_CollisionMgr.StartApply( GetGuid() ) ;
        g_CollisionMgr.ApplyAABBox( BBox ) ;
        g_CollisionMgr.EndApply() ;
    }
#endif // X_EDITOR

    // Only collide with bullets
    if ( (pObject) && (pObject->IsKindOf( base_projectile::GetRTTI())) )
    {
        m_Ragdoll.OnColCheck() ;
    }

}

//===============================================================================

void dead_body::ChangeObjectGuid( guid NewGuid )
{
    // Update dead body object
    g_ObjMgr.ChangeObjectGuid( GetGuid(), NewGuid ) ;

    // Update ragdoll so collision works
    m_Ragdoll.SetObjectGuid( NewGuid ) ;
}


//===============================================================================
// Editor functions
//===============================================================================

void dead_body::OnEnumProp( prop_enum&    List )
{
    // Call base class
    object::OnEnumProp(List);

    // Character properties - lets trigger system check the health
    List.PropEnumHeader( "Character",         "This is here so you can check the health with triggers", 0 ) ;
    List.PropEnumFloat ( "Character\\Health", "This is here so you can check the health with triggers", PROP_TYPE_EXPOSE ) ; 

    // Dead body
    List.PropEnumHeader("DeadBody", "Editor placed ragdoll deadbody", 0 ) ; 

    // Ragdoll properties
    List.PropEnumBool    ( "DeadBody\\Physics",       "Start with physics active? (FALSE = frozen, TRUE = moving!)\nFor performance try leaving this to FALSE!", PROP_TYPE_MUST_ENUM);
    List.PropEnumEnum    ( "DeadBody\\RagdollType",    ragdoll::s_TypeList.BuildString(), "Type of ragdoll rig to use.", PROP_TYPE_MUST_ENUM);
    List.PropEnumFloat   ( "DeadBody\\SimulationTime", "Physics time to simulate before level starts.", PROP_TYPE_MUST_ENUM);

    // Animation properties
    List.PropEnumExternal( "DeadBody\\AnimGroupName",  "Resource\0animexternal",          "Select the animation group and animation.", PROP_TYPE_MUST_ENUM | PROP_TYPE_EXTERNAL );
    List.PropEnumString  ( "DeadBody\\AnimName",       "Name of the animation to play.",  PROP_TYPE_MUST_ENUM);
    List.PropEnumInt     ( "DeadBody\\AnimFrame",      "Frame of animation to start at.", PROP_TYPE_MUST_ENUM);

    // Geometry properties
    GetSkinInst().OnEnumProp(List);

    // Decals
    List.PropEnumHeader  ( "BloodDecals",          "Which blood decals this actor will leave.", 0 );
    List.PropEnumExternal( "BloodDecals\\Package", "Decal Resource\0decalpkg\0", "Which blood decal package this actor uses.", 0 );
    List.PropEnumInt     ( "BloodDecals\\Group",   "Within the decal package, which group of bloud this actor uses.", 0 );
}

//===============================================================================

xbool dead_body::OnProperty( prop_query&   I )
{
    // Call base class
    if (object::OnProperty(I))
    {
        // Initialize zone tracker?
        if( I.IsVar( "Base\\Position" ) )
        {
            g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
        }
                    
        return TRUE;
    }
    
    // Health?
    if ( I.IsVar( "Character\\Health" ) )
    {
        if (I.IsRead())
            I.SetVarFloat(0) ;

        return TRUE ;
    }

    // Physics?
    if ( I.IsVar( "DeadBody\\Physics" ) )
    {
        if (I.IsRead())
            I.SetVarBool(m_bPhysics) ;
        else
            m_bPhysics = I.GetVarBool() ;

        return TRUE ;
    }

    // RagdollType?
    if( I.IsVar( "DeadBody\\RagdollType" ) )
    {
        RagdollType_OnProperty( I, m_RagdollType );
        
        #ifdef X_EDITOR
            // Re-initialize
            if ( !I.IsRead() )
                InitializeEditorPlaced() ;
        #endif // X_EDITOR

        return TRUE;
    }

    // SimulationTime
    if (I.VarFloat( "DeadBody\\SimulationTime", m_SimulationTime, 0.0f, 5.0f ))
    {
        #ifdef X_EDITOR
            // Re-initialize
            if ( !I.IsRead() )
                InitializeEditorPlaced() ;
        #endif // X_EDITOR

        return TRUE ;
    }

    // AnimGroup, AnimName
    if( I.IsVar( "DeadBody\\AnimGroupName" ) )
    {
        if( I.IsRead() )
        {
            if ( m_AnimGroupName >= 0 )
                I.SetVarExternal( g_StringMgr.GetString(m_AnimGroupName), 256 );
            else
                I.SetVarExternal("", 256);
        }
        else
        {
            // Get the FileName
            xstring String = I.GetVarExternal();
            if( !String.IsEmpty() )
            {
                s32 PkgIndex = String.Find( '\\', 0 );
                if( PkgIndex != -1 )
                {
                    xstring Pkg = String.Left( PkgIndex );
                    Pkg += "\0\0";
                    m_AnimName = g_StringMgr.Add( String.Right( String.GetLength() - PkgIndex - 1) );
                    m_AnimGroupName = g_StringMgr.Add( Pkg );
                }
                else
                {
                    m_AnimGroupName = g_StringMgr.Add( String );
                }
            }

            #ifdef X_EDITOR
                // Re-initialize
                InitializeEditorPlaced() ;
            #endif // X_EDITOR
        }
        return TRUE;
    }

    // AnimName
    if (I.IsVar( "DeadBody\\AnimName" ))
    {
        if( I.IsRead() )
        {
            if (m_AnimName >= 0)
                I.SetVarString( g_StringMgr.GetString(m_AnimName), 256 );
            else
                I.SetVarString( "", 256);
        }
        else
        {
            if (x_strlen(I.GetVarString()) > 0)
                m_AnimName = g_StringMgr.Add( I.GetVarString() );

            #ifdef X_EDITOR
                // Re-initialize
                InitializeEditorPlaced() ;
            #endif // X_EDITOR
        }

        return TRUE;
    }

    // AnimFrame
    if (I.VarInt( "DeadBody\\AnimFrame", m_AnimFrame, 0 ))
    {
        #ifdef X_EDITOR
            // Re-initialize
            if ( !I.IsRead() )
                InitializeEditorPlaced() ;
        #endif // X_EDITOR

        return TRUE ;
    }

    // Geometry
    if (GetSkinInst().OnProperty(I))
    {
        #ifdef X_EDITOR
            // Re-initialize if geometry is selected
            if ( ( !I.IsRead() ) && ( I.IsVar( "RenderInst\\File" ) ) )
                InitializeEditorPlaced() ;
        #endif // X_EDITOR

        return TRUE ;
    }

    // Decal package
    if ( I.IsVar( "BloodDecals\\Package" ) )
    {
        if ( I.IsRead() )
            I.SetVarExternal( m_hBloodDecalPackage.GetName(), RESOURCE_NAME_SIZE );
        else
            m_hBloodDecalPackage.SetName( I.GetVarExternal() );

        return TRUE;
    }

    // Decal group
    if ( I.VarInt( "BloodDecals\\Group", m_BloodDecalGroup ) )
    {
        return TRUE;
    }

    return FALSE ;
}

//===============================================================================

