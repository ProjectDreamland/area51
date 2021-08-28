//==============================================================================
//
//  Corpse.cpp
//
//==============================================================================

//==============================================================================
// INCLUDES
//==============================================================================


#include "Entropy\Entropy.hpp"
#include "Characters\Character.hpp"
#include "Loco\Loco.hpp"
#include "Objects\BaseProjectile.hpp"
#include "NetworkMgr\NetObj.hpp"
#include "Decals\DecalMgr.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Characters\ActorEffects.hpp"
#include "Corpse.hpp"
#include "CorpsePain.hpp"
#include "PhysicsMgr\PhysicsMgr.hpp"
#include "Objects\ParticleEmiter.hpp"
#include "Objects\Player.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Objects\DamageField.hpp"
#include "Objects\Actor\Actor.hpp"



//=========================================================================
// DEFINES
//=========================================================================

static  s32    CORPSE_MAX_DYNAMIC_COUNT         = 3;
static  s32    CORPSE_MAX_ACTIVE_COUNT          = 4;
static  f32    CORPSE_FADEOUT_START_TIME        = 8.0f;
static  f32    CORPSE_FADEOUT_TIME              = 1.0f;
static  f32    CORPSE_IMPACT_SFX_SPEED_SQR      = 300.0f * 300.0f;
static  f32    CORPSE_IMPACT_SFX_INTERVAL_TIME  = 0.1f;
static  f32    CORPSE_BLEND_CONSTRAINTS_TIME    = 1.0f;


//=========================================================================
// DATA
//=========================================================================

s32 corpse::m_ActiveCount = 0;          // # of active (moving) corpses

// Workspace data for corpses for initialization. Using scratchmem would
// be preferable, but this needs to be used in the loading screen where
// smem isn't available to us.
static matrix4  s_InitMatrices[MAX_ANIM_BONES] PS2_ALIGNMENT(16);
static anim_key s_InitAnimKeys[MAX_ANIM_BONES] PS2_ALIGNMENT(16);


// Material type enum
typedef enum_pair<s32> corpse_material_enum_pair;
static corpse_material_enum_pair s_CorpseMaterialEnumPair[] = 
{
    // Available materials
    corpse_material_enum_pair( "FLESH",     object::MAT_TYPE_FLESH ),
    corpse_material_enum_pair( "FABRIC",    object::MAT_TYPE_FABRIC ),

    //**MUST BE LAST**//    
    corpse_material_enum_pair( k_EnumEndStringConst, object::MAT_TYPE_NULL )
};
enum_table<s32>  s_CorpseMaterialEnum( s_CorpseMaterialEnumPair );              


//=========================================================================
// EXTERNS
//=========================================================================

extern xbool    g_level_loading;
extern xbool    g_bBloodEnabled;
extern xbool    g_bRagdollsEnabled;


#ifdef X_EDITOR
extern xbool g_game_running;
#endif // X_EDITOR

//=========================================================================

struct corpse_table_entry
{
    s32         Item;
    const char* pName;
    const char* pIdentifier;
};

static corpse_table_entry s_CorpseTable[] =
{
    { CORPSE_GENERIC,          "Generic Corpse"         ,  "GENERIC"    },
    { CORPSE_BRIDGES,          "Bridges Corpse"         ,  "BRIDGES"    },
    { CORPSE_CARSON,           "Carson Corpse"          ,  "CARSON"     },
    { CORPSE_CRISPY,           "Crispy Corpse"          ,  "CRISPY"     },
    { CORPSE_CRISPY_MUTATED,   "Crispy-MUTATED Corpse"  ,  "CRISPY_MUT" },
    { CORPSE_CHEW,             "Chew Corpse"            ,  "CHEW"       },
    { CORPSE_DRCRAY,           "Dr. Cray Corpse"        ,  "DRCRAY"     },
    { CORPSE_FERRI,            "Ferri Corpse"           ,  "FERRI"      },
    { CORPSE_LEONARD,          "Leonard Corpse"         ,  "LEONARD"    },
    { CORPSE_MCCANN,           "McCann Corpse"          ,  "MCCANN"     },
    { CORPSE_MRWHITE,          "Mr. White Corpse"       ,  "MRWHITE"    },
    { CORPSE_RAMIREZ,          "Ramirez Corpse"         ,  "RAMIREZ"    },
    { CORPSE_VICTOR,           "Victor Corpse"          ,  "VICTOR"     },
};

#define NUM_CORPSE_TABLE_ENTRIES   ((s32)( sizeof(s_CorpseTable) / sizeof(corpse_table_entry) ))

#if defined( ksaffel )
xbool g_LogCorpseNames = TRUE;
#define CORPSE_LOGGING g_LogCorpseNames
#else
#define CORPSE_LOGGING 0
#endif

//=========================================================================
// OBJECT DESC
//=========================================================================

static struct corpse_desc : public object_desc
{
    corpse_desc( void ) : object_desc( object::TYPE_CORPSE, 
                                        "Dead Body",    //!!!!
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

    virtual object* Create( void ) { return new corpse; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

} s_CorpseDesc;

//=========================================================================

const object_desc& corpse::GetTypeDesc( void ) const
{
    return s_CorpseDesc;
}

//=========================================================================

const object_desc& corpse::GetObjectType( void )
{
    return s_CorpseDesc;
}

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

corpse::corpse( void ) :
    m_OriginGuid        ( 0     ),
    m_bActive           ( FALSE ),
    m_bCreatedBlood     ( FALSE ),
    m_bCanDelete        ( FALSE ),
    m_bPermanent        ( FALSE ),
    m_bDestroy          ( FALSE ),
    m_bDrainable        ( FALSE ),
    m_bActorCollision   ( FALSE ),
    m_bWorldCollision   ( TRUE  ),
    m_bActiveWhenVisible( FALSE ),
    m_TimeAlive         ( 0.0f  ),
    m_pActorEffects     ( NULL  ),
    m_AnimGroupName     ( -1    ),
    m_AnimName          ( -1    ),
    m_AnimFrame         (  0    ),
    m_SimulationTime    ( 0.0f  ),
    m_Material          ( object::MAT_TYPE_FLESH ),
	m_BloodDecalGroup   ( -1    ),
    m_CorpseName        ( CORPSE_GENERIC ),
    m_ImpactSfxTimer    ( 0.0f )
{
    PHYSICS_DEBUG_DYNAMIC_MEM_ALLOC( sizeof( corpse ) );
    
    m_FloorProperties.Init( 100.0f, 0.5f );
    
    // This assumes that the dead body is about 2.5 meters tall.  Then is about 2 
    // meters around.  The bbox is center around the eyes of the player.
    m_ZoneTracker.SetBBox( bbox( vector3( -100, -200, -100 ), 
                                 vector3(  100,   50,  100 ) ) );

    m_FadeOutTime = CORPSE_FADEOUT_TIME;
}

//=========================================================================

corpse::~corpse()
{
    PHYSICS_DEBUG_DYNAMIC_MEM_FREE( sizeof( corpse ) );

    if( m_pActorEffects )
    {
        delete m_pActorEffects;
        m_pActorEffects = NULL;
    }
}

//=========================================================================

void corpse::OnKill( void )
{
    if ( m_FlamingDamageField != NULL_GUID )
    {
        g_ObjMgr.DestroyObject( m_FlamingDamageField );
    }
}

//=========================================================================

void corpse::OnMove( const vector3& NewPos )
{
    // Call base class
    object::OnMove( NewPos );

#ifdef X_EDITOR
    // If being moved in the editor, re-initialize
    if( ( !g_game_running ) && ( GetAttrBits() & ( object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT ) ) )
        InitializeEditorPlaced();
#endif // X_EDITOR

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, NewPos );
    m_PhysicsInst.SetZone( m_ZoneTracker.GetMainZone() );

    // Move flaming damage field
    if ( m_FlamingDamageField != NULL_GUID )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_FlamingDamageField );

        if ( pObject )
        {
            pObject->OnMove( GetBBox().GetCenter() );
        }
    }

}

//=========================================================================

void corpse::OnTransform( const matrix4& L2W )
{
    // Call base class
    object::OnTransform( L2W );

#ifdef X_EDITOR
    // If being moved in the editor, re-initialize
    if( ( !g_game_running ) && ( GetAttrBits() & ( object::ATTR_EDITOR_SELECTED | object::ATTR_EDITOR_PLACEMENT_OBJECT ) ) )
        InitializeEditorPlaced();
#endif // X_EDITOR

    // Update zone tracking
    g_ZoneMgr.UpdateZoneTracking( *this, m_ZoneTracker, L2W.GetTranslation() );
    m_PhysicsInst.SetZone( m_ZoneTracker.GetMainZone() );
}

//=========================================================================

bbox corpse::GetLocalBBox( void ) const
{
    // Get bbox from geom? (Fixes punch bag collision since it's so tall!)
    const geom* pGeom = GetSkinInst().GetGeom();
    if( pGeom )
    {
        bbox BBox( vector3( 0.0f, 0.0f, 0.0f ), pGeom->m_BBox.GetRadius() );
        return BBox;
    }
    else
    {
        // Use generic bbox
        return bbox( vector3(-150, -150, -150), vector3(150,150,150) );
    }
}        

//===========================================================================

// This function gets called once per game loop from god::OnAdvance
void corpse::LimitCount( void )
{
    // Keep looping until active count is reduced
    xbool bActiveCountValid;
    do
    {
        // Default to valid count
        bActiveCountValid = TRUE;
    
        // Clear global active count
        m_ActiveCount = 0;
        
        // Search for dynamically created corpses
        s32     DynamicCorpseCount = 0;

        // Search for slowest moving active corpse
        corpse* pActiveCorpse      = NULL;
        f32     ActiveSpeedSqr     = F32_MAX;

        // Search for oldest inactive corpse
        corpse* pInactiveCorpse    = NULL;
        f32     InactiveTimeAlive  = -F32_MAX;
        
        // Loop through all corpses in the world looking for oldest dynamic corpses
        for( slot_id Slot = g_ObjMgr.GetFirst( object::TYPE_CORPSE ); Slot != SLOT_NULL; Slot = g_ObjMgr.GetNext( Slot ) )
        {
            // Lookup corpse
            corpse* pCorpse = (corpse*)g_ObjMgr.GetObjectBySlot( Slot );

            // Skip if already flagged to be destroyed
            if( pCorpse->m_bDestroy )
                continue;

            // Update global active count
            if( pCorpse->m_PhysicsInst.IsActive() )
                m_ActiveCount++;

            // Skip if already fading out
            if( pCorpse->m_TimeAlive >= CORPSE_FADEOUT_START_TIME )
                continue;
                
            // Skip if permanent
            if( pCorpse->m_bPermanent )
                continue;
                
            // Update dynamic (created by killing an npc ie. not editor placed) count
            DynamicCorpseCount++;
                
            // Active?
            if( pCorpse->m_PhysicsInst.IsActive() )
            {          
                // Keep least moving active dynamic corpse
                f32 CorpseSpeedSqr = pCorpse->m_PhysicsInst.GetSpeedSqr();
                if( CorpseSpeedSqr < ActiveSpeedSqr )
                {
                    pActiveCorpse  = pCorpse;
                    ActiveSpeedSqr = CorpseSpeedSqr;
                }                
            }
            else
            {
                // Keep oldest inactive dynamic corpse
                f32 CorpseTimeAlive = pCorpse->m_TimeAlive;
                if( CorpseTimeAlive > InactiveTimeAlive )
                {
                    pInactiveCorpse   = pCorpse;
                    InactiveTimeAlive = CorpseTimeAlive;                
                }
            }                
        }
        
        // If there are too many dynamic corpses or too many active corpses, fade out the oldest
        if(     ( DynamicCorpseCount > CORPSE_MAX_DYNAMIC_COUNT ) 
            ||  ( m_ActiveCount > CORPSE_MAX_ACTIVE_COUNT ) 
            ||  ( g_PhysicsMgr.GetAwakeInstanceCount() > CORPSE_MAX_ACTIVE_COUNT ) )
        {
            // Favor inactive corpses first (if there are any)
            corpse* pOldestCorpse = ( pInactiveCorpse != NULL ) ? pInactiveCorpse : pActiveCorpse;
            
            // Corpse to fade out?
            if( pOldestCorpse )
            {
                // Should be valid
                ASSERT( pOldestCorpse );
                ASSERT( !pOldestCorpse->m_bPermanent );
                ASSERT( !pOldestCorpse->m_bDestroy );
                ASSERT( pOldestCorpse->m_TimeAlive < CORPSE_FADEOUT_START_TIME );

                // Instead of popping away, start the fade
                pOldestCorpse->m_TimeAlive = CORPSE_FADEOUT_START_TIME;
                
                // Put the rigid bodies to sleep so that the physics system does not run out of constraints
                pOldestCorpse->m_PhysicsInst.Deactivate();
                pOldestCorpse->m_PhysicsInst.SetInstCollision( FALSE );
                
                // Keep checking
                bActiveCountValid = FALSE;
            }
        }
    }
    while( !bActiveCountValid );    // Keep limiting until count is acceptable
}

//===========================================================================

xbool corpse::ReachedMaxActiveLimit( void )
{
    return ( m_ActiveCount >= CORPSE_MAX_ACTIVE_COUNT ) ||
           ( g_PhysicsMgr.GetAwakeInstanceCount() > CORPSE_MAX_ACTIVE_COUNT );
}

//===========================================================================

xbool corpse::Initialize( actor& Actor, xbool bDoBodyFade, actor_effects* pActorEffects )
{
    // Keep owner
    m_OriginGuid = Actor.GetGuid();

    // copy in the actor effects
    if( m_pActorEffects )
    {
        delete m_pActorEffects;
    }
    m_pActorEffects = pActorEffects;
    m_bPermanent = !bDoBodyFade;
    
    // Initialize the ragdoll with pop fixing and blending in the constraints
    if( !m_PhysicsInst.Init( Actor.GetSkinInst(), TRUE, CORPSE_BLEND_CONSTRAINTS_TIME ) )
        return FALSE;

    // Set the zone info
    m_ZoneTracker = Actor.GetZoneTracker();
    SetZone1( Actor.GetZone1() );
    SetZone2( Actor.GetZone2() );
    
    // Copy the time when It was created
    m_TimeAlive = 0.0f;
    
    // Copy floor properties
    m_FloorProperties = Actor.GetFloorProperties();

    // Now setup the matrices of the ragdoll from the current animation to inherit velocities
    loco* pLoco = Actor.GetLocoPointer();
    if( pLoco )
        m_PhysicsInst.SetMatrices( pLoco->m_Player, pLoco->GetDeltaPos() );

    // Now rigid body matrices have been setup, update the object transform, which will
    // correctly update the world bounding box and zone
    OnMove( m_PhysicsInst.GetPosition() );

    // copy the decal data
    m_hBloodDecalPackage.SetName( Actor.GetBloodDecalPackage() );
    m_BloodDecalGroup = Actor.GetBloodDecalGroup();

    // If we're on fire, create a damage field
    if ( m_pActorEffects && m_pActorEffects->IsEffectOn( actor_effects::FX_FLAME ) )
    {
        if ( m_FlamingDamageField != NULL_GUID )
        {
            object* pObject = g_ObjMgr.GetObjectByGuid( m_FlamingDamageField );
            if ( pObject )
            {
                g_ObjMgr.DestroyObject( pObject->GetSlot() );
            }
        }
        m_FlamingDamageField = g_ObjMgr.CreateObject( damage_field::GetObjectType() );
        damage_field* pDamageField = (damage_field*)g_ObjMgr.GetObjectByGuid( m_FlamingDamageField );

        if ( pDamageField )
        {
            damage_field& DF = *pDamageField;
            DF.SetSpatialType       ( damage_field::SPATIAL_TYPE_SPHERICAL  );
            DF.SetSpatialTargets    ( damage_field::DF_TARGET_PLAYER        );
            DF.SetActive            ( TRUE                                  );
            DF.SetDimension         ( 0, 50.0f                              );
        }
    }

    // Success
    return TRUE;
}

//===============================================================================

// This function is only called when creating permanent dead bodies in the editor
xbool corpse::Initialize( const char*           pGeomName,
                          const char*           pAnimGroupName,
                          const char*           pAnimName,
                                s32             AnimFrame )
{
    // Must be valid
    if( ( !pGeomName ) || ( !pAnimGroupName ) || ( !pAnimName ) )
        return FALSE;

    // Initialize the ragdoll (no need to fix popping)
    if( !m_PhysicsInst.Init( pGeomName, FALSE, 0.0f ) )
        return FALSE;

    // Lookup geom
    const skin_geom* pGeom = GetSkinInst().GetSkinGeom();
    if( !pGeom )
        return FALSE;

    // Copy the time when It was created
    m_TimeAlive = 0.0f;

    // Lookup animation group
    anim_group::handle& hAnimGroup = m_PhysicsInst.GetAnimGroupHandle();
    hAnimGroup.SetName( pAnimGroupName );
    const anim_group* pAnimGroup = hAnimGroup.GetPointer();
    if( !pAnimGroup )
        return FALSE;

    // Lookup animation info
    s32 nBones = pAnimGroup->GetNBones();
    s32 iAnim  = pAnimGroup->GetAnimIndex( pAnimName );
    if( iAnim == -1 )
        return FALSE;
    const anim_info& AnimInfo = pAnimGroup->GetAnimInfo( iAnim );

    // Clamp frame
    s32 LastFrame = x_max(0, AnimInfo.GetNFrames() - 2);
    if( AnimFrame > LastFrame )
        AnimFrame = LastFrame;

    // Setup ptrs
    matrix4*  pMatrices = s_InitMatrices;
    anim_key* pKeys     = s_InitAnimKeys;

    // Compute matrices
    AnimInfo.GetInterpKeys( (f32)AnimFrame, pKeys, nBones );
    pAnimGroup->ComputeBonesL2W( GetL2W(), pKeys, nBones, pMatrices, TRUE );

    // Setup the ragdoll bodies
    m_PhysicsInst.SetMatrices( pMatrices, nBones, FALSE );

    // Create body -> world constraints from events in animation
    for( s32 iEvent = 0; iEvent < AnimInfo.GetNEvents(); iEvent++ )
    {
        // Lookup event
        const anim_event& Event = AnimInfo.GetEvent( iEvent );
        
        // Is this a generic super event?
        const char* pEventType = Event.GetType();
        if( x_stricmp( pEventType, "Generic" ) == 0 )
        {
            // Is this a "PinToWorld" event?
            const char* pType = Event.GetString( anim_event::STRING_IDX_GENERIC_TYPE );
            if( x_stricmp( pType, "Pin_To_World" ) == 0 )
            {
                // Walk up hierarchy to get a valid bone
                s32 iBone = Event.GetInt( anim_event::INT_IDX_BONE );
                while( iBone >= pGeom->m_nBones )
                    iBone = pAnimGroup->GetBoneParent( iBone );
            
                // Compute event world position
                matrix4& BoneL2W  = pMatrices[ iBone ];
                vector3  WorldPos = BoneL2W * Event.GetPoint( anim_event::POINT_IDX_OFFSET );
                
                // Finally, create the constraint
                m_PhysicsInst.AddBodyWorldConstraint( pGeom->m_pBone[ iBone ].iRigidBody, WorldPos, 0.0f );
            }
        }
    }

    // Success
    return TRUE;
}

//===============================================================================

xbool corpse::InitializeEditorPlaced( void )
{
    // Skip if loading since the static world is not yet loaded and corpses will fall forever! -
    // the main app or editor will call this function again after loading.
    if( g_level_loading )
        return FALSE;
    
    // Valid?
    if(     ( !GetSkinInst().GetSkinGeomName() )
         || ( !GetSkinInst().GetSkinGeom() ) 
         || ( m_AnimGroupName == -1 ) 
         || ( m_AnimName == -1 ) )
        return FALSE;

    // Never delete
    m_bPermanent = TRUE;

    // Make sure properties are included in game save/load
    SetAttrBits( GetAttrBits() & ~object::ATTR_NO_RUNTIME_SAVE);

    // Initialize from properties
    if( !Initialize( GetSkinInst().GetSkinGeomName(),
                     g_StringMgr.GetString( m_AnimGroupName ),
                     g_StringMgr.GetString( m_AnimName ),
                     m_AnimFrame ) )
    {
        return FALSE;
    }
    
    // Run the physics for the specified simulation time...
    f32 Step = 1.0f / 30.0f;
    for ( f32 Time = 0; Time < m_SimulationTime; Time += Step )
    {
        // Force activation incase bodies deactivate
        m_PhysicsInst.Activate();
        
        // Advance simulation
        g_PhysicsMgr.Advance( Step );
    }
    g_PhysicsMgr.ClearDeltaTime();
    
    // Finally activate or deactivate physics instance (since sim time maybe zero)?
    if( m_bActive )
    {
        m_PhysicsInst.Activate();   
    }                        
    else        
    {    
        m_PhysicsInst.Deactivate();
    }        

    // Setup floor color
    m_FloorProperties.ForceUpdate( m_PhysicsInst.GetPosition() );
    
#ifdef X_EDITOR
    // Only update position if game is running
    if( g_game_running )
        OnMove( m_PhysicsInst.GetPosition() );
#else
    // Update position ready for correct rendering
    OnMove( m_PhysicsInst.GetPosition() );
#endif
    
    // Success
    return TRUE;
}

//===============================================================================

void corpse::OnRenderShadowCast( u64 ProjMask )
{
    // Compute LOD mask for the shadow render (by putting zero in for screen size
    // we force the lowest lod)
    u64 ShadLODMask = GetSkinInst().GetLODMask(0);
    if( ShadLODMask == 0 )
        return;

    // Compute matrices
    u64 LODMask;
    s32 nActiveBones;
    const matrix4* pMatrices = m_PhysicsInst.GetBoneL2Ws( LODMask, nActiveBones );
    if( !pMatrices )
        return;

    // Setup render flags
    u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

    // Render that puppy!
    GetSkinInst().RenderShadowCast( &GetL2W(), 
                             pMatrices, 
                             nActiveBones,
                             Flags,
                             ShadLODMask,
                             ProjMask );
}

//===============================================================================

void corpse::OnRender( void )
{
#ifdef X_EDITOR
    // Just render physics if selected and playing the game in the editor
    if( ( g_game_running ) && ( GetAttrBits() & object::ATTR_EDITOR_SELECTED ) )
    {    
        // Activate all bodies?
        if( m_bActiveWhenVisible )
            m_PhysicsInst.Activate();
    
        // Render physics
        m_PhysicsInst.RenderCollision();
        OnDebugRender();
        return;
    }        
#endif

    // render any actor effects
    if( m_pActorEffects )
        m_pActorEffects->Render( this );

    // Setup render flags and get the floor color
    xcolor Ambient = GetFloorColor();
    u32    Flags   = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
    
    // Render fading out?
    if( ( !m_bPermanent ) && ( m_TimeAlive > CORPSE_FADEOUT_START_TIME ) )
    {
        Flags |= render::FADING_ALPHA;
        f32 Alpha = 1.0f - ((m_TimeAlive - CORPSE_FADEOUT_START_TIME) / m_FadeOutTime);
        Alpha = MIN( Alpha, 1.0f );
        Alpha = MAX( Alpha, 0.0f );
        Ambient.A  = (u8)(Alpha*255.0f);
    }

    // Render that puppy!
    m_PhysicsInst.Render( Flags, Ambient );
}

//===============================================================================

#ifndef X_RETAIL

void corpse::OnDebugRender( void )
{
    draw_ClearL2W();
    
    // Loop through constraints
    for( s32 i = 0; i < m_PhysicsInst.GetNBodyWorldConstraints(); i++ )
    {
        // Lookup constraint
        constraint& Constraint = m_PhysicsInst.GetBodyWorldConstraint( i );
            
        // Render
        draw_Sphere( Constraint.GetWorldPos( 0 ), 5.0f, XCOLOR_RED );             
        draw_Sphere( Constraint.GetWorldPos( 1 ), 5.0f, XCOLOR_YELLOW );             
    }
}

//===============================================================================

void corpse::OnColRender( xbool bRenderHigh )
{
#ifdef ENABLE_PHYSICS_DEBUG
    m_PhysicsInst.RenderCollision();
#endif
    
    // Call base class
    object::OnColRender( bRenderHigh );
}

#endif

//===============================================================================

void corpse::OnRenderTransparent( void )
{
    if( m_pActorEffects )
    {
        f32 Alpha = 1.0f;
        if( ( !m_bPermanent ) && ( m_TimeAlive > CORPSE_FADEOUT_START_TIME ) )
        {
            Alpha = 1.0f - ( ( m_TimeAlive - CORPSE_FADEOUT_START_TIME ) / m_FadeOutTime );
            Alpha = MIN( Alpha, 1.0f );
            Alpha = MAX( Alpha, 0.0f );
        }
        m_pActorEffects->RenderTransparent( this, Alpha );
    }
}

//===============================================================================

void corpse::OnAdvanceLogic( f32 DeltaTime )
{
    CONTEXT( "corpse::OnAdvanceLogic" );

    // update any actor effects
    if( m_pActorEffects )
        m_pActorEffects->Update( this, DeltaTime );

    // Flagged to be destroyed?
    if( m_bDestroy )
    {
        ASSERT( !m_bPermanent );
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }

    // Time out and delete his body?
    if( ( m_bCanDelete ) && ( !m_bPermanent ) && ( m_TimeAlive >= ( CORPSE_FADEOUT_START_TIME + m_FadeOutTime ) ) )
    {
        g_ObjMgr.DestroyObject( GetGuid() );
        return;
    }

    // Update the timer.
    if( m_bPermanent )
    {
        // Reset time
        m_TimeAlive = 0.0f;
    }        
    else
    {
        // Update time
        m_TimeAlive += DeltaTime;
        
        // If fading out, put the rigid bodies to sleep so that the physics system does not run out of constraints
        if( m_TimeAlive >= CORPSE_FADEOUT_START_TIME )
        {
            m_PhysicsInst.Deactivate();
            m_PhysicsInst.SetInstCollision( FALSE );
        }        
    }
    
    // Okay lets assume next time we can be deleted
    m_bCanDelete = TRUE;

    // Get current position from physics instance
    vector3 NewPos = m_PhysicsInst.GetPosition();
    
    // Update base object position?
    if( NewPos != GetPosition() )
        OnMove( NewPos );

    // Update ambient color for rendering
    m_FloorProperties.Update( NewPos, DeltaTime );

    // Update instance house keeping
    m_PhysicsInst.Advance( DeltaTime );

    // Possibly play an impact?
    if( m_ImpactSfxTimer == 0.0f )
    {
        // Are rigid bodies active?
        physics_inst& PhysicsInst = GetPhysicsInst();
        if( PhysicsInst.IsActive() )
        {
            // Loop through all rigid bodies looking for fastest collision
            f32 ImpactSpeedSqr = 0.0f;
            s32 iImpactBody     = -1;
            for( s32 i = 0; i < PhysicsInst.GetNRigidBodies(); i++ )
            {
                // Lookup body
                rigid_body& Body = PhysicsInst.GetRigidBody(i);
                
                // Collision occurred?
                if( Body.HasCollided() )
                {
                    // Biggest so far?
                    f32 CollisionSpeedSqr = Body.GetCollisionSpeedSqr();
                    if(     ( CollisionSpeedSqr > CORPSE_IMPACT_SFX_SPEED_SQR ) 
                        &&  ( CollisionSpeedSqr > ImpactSpeedSqr ) )
                    {
                        // Record
                        iImpactBody    = i;
                        ImpactSpeedSqr = CollisionSpeedSqr;
                    }                    
                }
            }
            
            // Play impact?                            
            if( iImpactBody != -1 ) 
            {
                // Play impact sfx
                g_AudioMgr.Play( "TerroristA_Bodyfall", 
                                 PhysicsInst.GetRigidBody( iImpactBody ).GetPosition(), 
                                 GetZone1(), 
                                 TRUE );
                                 
                // Setup delay before next impact
                m_ImpactSfxTimer = CORPSE_IMPACT_SFX_INTERVAL_TIME;                           
            }
        }
    }
    else
    {
        // Update impact sfx timer
        m_ImpactSfxTimer = x_max( 0.0f, m_ImpactSfxTimer - DeltaTime );
    }    
}

//===============================================================================

void corpse::OnActivate( xbool Flag )
{
    // Wake up or put physics to sleep?
    if( Flag )
    {
        m_PhysicsInst.Activate();
    }        
    else        
    {    
        m_PhysicsInst.Deactivate();
    }
    
    // Call base class
    object::OnActivate( Flag );
}

//===============================================================================

xbool corpse::IsBloodEnabled( void ) const
{
    // TO DO: Add property and detect german version if this needs to be on
    //        a per character basis
    return g_bBloodEnabled;
}

//===============================================================================

xbool corpse::IsRagdollEnabled( void ) const
{
    // TO DO: Add property and detect german version if this needs to be on
    //        a per character basis
    return g_bRagdollsEnabled;
}

//===============================================================================

void corpse::CreateSplatDecalOnGround( void )
{
    // Skip?
    if( !IsBloodEnabled() )
        return;

    // Just create once to keep limit down
    if( m_bCreatedBlood )
        return;

    // Lookup package
    decal_package* pPackage = m_hBloodDecalPackage.GetPointer();
    if ( !pPackage )
        return;

    // Valid group?
    if ( (m_BloodDecalGroup<0) || (m_BloodDecalGroup>=pPackage->GetNGroups()) )
        return;

    // Valid decal def?
    s32 nDecalDefs = pPackage->GetNDecalDefs( m_BloodDecalGroup );
    if ( nDecalDefs == 0 )
        return;

    // choose a random decal to paste
    s32 DecalIndex = (nDecalDefs == 1) ? 0 : x_rand() % (nDecalDefs-1);
    decal_definition& DecalDef = pPackage->GetDecalDef( m_BloodDecalGroup, DecalIndex );

    // create a ray that is biased towards the ground
    radian3 BloodOrient;
    BloodOrient.Pitch = x_frand(R_80, R_100);
    BloodOrient.Yaw   = x_frand(R_0,  R_360);
    BloodOrient.Roll  = R_0;

    vector3 RayStart = GetPosition();
    RayStart.GetY() += 10.0f;

    vector3 RayEnd( 0.0f, 0.0f, 1.0f );
    RayEnd.Rotate( BloodOrient );
    RayEnd  = 500.0f * RayEnd;
    RayEnd += RayStart;

    // generate the splat decal
    g_DecalMgr.CreateDecalFromRayCast( DecalDef, RayStart, RayEnd );

    // Don't do again...        
    m_bCreatedBlood = TRUE;
}

//===============================================================================

void corpse::CreateImpactEffect( const pain& Pain )
{
    // Skip if not a direct hit
    if( !Pain.IsDirectHit() )
        return;
        
    // Create blood impact?
    if( m_Material == object::MAT_TYPE_FLESH )
    {
        // Only do if blood is on
        if( IsBloodEnabled() )
        {
            // Create blood impact if blood decals are assigned
            const decal_package* pBloodDecalPackage = m_hBloodDecalPackage.GetPointer();
            if( pBloodDecalPackage )
            {
                // Create blood based on pain type and use color of assigned blood decal group
                particle_emitter::CreateOnPainEffect( Pain, 
                    0.0f, 
                    particle_emitter::UNINITIALIZED_PARTICLE, 
                    pBloodDecalPackage->GetGroupColor( m_BloodDecalGroup ) );
            }                                                  
        }
    }
    // Create dust puff?
    else if( m_Material == object::MAT_TYPE_FABRIC )
    {
        // Create fabric impact effect
        particle_emitter::CreateOnPainEffect( Pain, 
            0.0f, 
            particle_emitter::IMPACT_FABRIC_HIT, 
            XCOLOR_WHITE );
    }
}

//===============================================================================

void corpse::OnPain( const pain& Pain )
{
    // Skip?
    if( !IsRagdollEnabled() )
        return;
        
    // If we've already starting fading the body out, don't do any more splatting
    if( m_TimeAlive >= CORPSE_FADEOUT_START_TIME )
        return;

    // If corpse is not active and there are already too many active, leave alone
    if( ( m_PhysicsInst.IsActive() == FALSE ) && ( ReachedMaxActiveLimit() ) )
        return;

    // Triggers can create npc ragdolls that do not have any pain setup, but still
    // go through through character death state which always calls this function upon
    // creating a ragdoll.
    if( Pain.SetupCalled() == FALSE )
        return;

    // Setup corpse pain and apply to self
    corpse_pain CorpsePain;
    CorpsePain.Setup( Pain, *this );
    CorpsePain.Apply( *this );
    
    // Record pain in origin actor so it can be sent over the net in MP?
    object_ptr<actor> pActor( m_OriginGuid );
    if( pActor)
    {
        // Keep
        pActor->GetCorpseDeathPain() = CorpsePain;
        
        // Only record once so death pain is not overwritten
        m_OriginGuid = 0;
    }
            
    // Create blood on ground?
    if( m_Material == object::MAT_TYPE_FLESH )
        CreateSplatDecalOnGround();
}

//===============================================================================

void corpse::OnColCheck( void )
{
    // Get moving object
    guid    MovingGuid = g_CollisionMgr.GetMovingObjGuid();
    object* pObject    = g_ObjMgr.GetObjectByGuid(MovingGuid);
    
#ifdef X_EDITOR
    // Allow ragdoll to be selected in the editor?
    if( g_CollisionMgr.IsEditorSelectRay() )
    {
        // Let physics instance check for collision...
        m_PhysicsInst.OnColCheck( GetGuid() );
    }
    
#endif // X_EDITOR

    // Only collide with bullets/grenades/player/character melee during game
    if(     ( pObject ) 
        &&  (       ( pObject->IsKindOf( net_proj::GetRTTI()        ) )
                ||  ( pObject->IsKindOf( base_projectile::GetRTTI() ) )
                ||  ( pObject->IsKindOf( character::GetRTTI()       ) )
                ||  ( pObject->IsKindOf( player::GetRTTI()          ) ) ) )
    {
        m_PhysicsInst.OnColCheck( GetGuid() );
    }
}

//===============================================================================

void corpse::ChangeObjectGuid( guid NewGuid )
{
    // Update dead body object
    g_ObjMgr.ChangeObjectGuid( GetGuid(), NewGuid );
}


//===============================================================================
// Editor functions
//===============================================================================

void corpse::OnEnumProp( prop_enum&    List )
{
    // Call base class
    object::OnEnumProp(List);

    // Character properties - lets trigger system check the health
    List.PropEnumHeader  ( "Character",         "This is here so you can check the health with triggers", 0 );
    List.PropEnumFloat   ( "Character\\Health", "This is here so you can check the health with triggers", PROP_TYPE_EXPOSE ); 

    // Dead body
    List.PropEnumHeader  ("DeadBody", "Editor placed ragdoll deadbody", 0 ); 

    // Corpse's name/material type
    List.PropEnumEnum  ( "DeadBody\\CorpseName",  GetEnumStringCorpse(),  "Which NPCs corpse is this (Generic for NPCs with no name).", PROP_TYPE_EXPOSE );
    List.PropEnumEnum  ( "DeadBody\\Material" ,   s_CorpseMaterialEnum.BuildString(), "Type of material of corpse - drives impact effect.", 0 );

    // Ragdoll properties
    List.PropEnumBool    ( "DeadBody\\Active",            "Start level with physics active? (FALSE = frozen, TRUE = moving!)\nFor best performance try leaving this to FALSE!", PROP_TYPE_MUST_ENUM);
    List.PropEnumBool    ( "DeadBody\\ActorCollision",    "Should player/npcs push ragdoll out of the way?",    PROP_TYPE_MUST_ENUM );
    List.PropEnumBool    ( "DeadBody\\WorldCollision",    "Should corpse collide with the world?",              PROP_TYPE_MUST_ENUM );
    List.PropEnumBool    ( "DeadBody\\ActiveWhenVisible", "Should rigid bodies always be active when visible?", PROP_TYPE_MUST_ENUM );
    List.PropEnumFloat   ( "DeadBody\\SimulationTime",    "Physics time to simulate before level starts.",      PROP_TYPE_MUST_ENUM );

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

xbool corpse::OnProperty( prop_query&   I )
{
    // Call base class
    if( object::OnProperty( I ) )
    {
        // Initialize zone tracker?
        if( I.IsVar( "Base\\Position" ) )
        {
            g_ZoneMgr.InitZoneTracking( *this, m_ZoneTracker );
        }

        // Update physics instance zone                    
        m_PhysicsInst.SetZone( m_ZoneTracker.GetMainZone() );
                    
        return TRUE;
    }
    
    // Health?
    if( I.IsVar( "Character\\Health" ) )
    {
        if( I.IsRead() )
            I.SetVarFloat( 0.0f );

        return TRUE;
    }

    if( I.IsVar("DeadBody\\CorpseName" ) )
    {
        if( I.IsRead() )
        {
            I.SetVarEnum( EnumToName( m_CorpseName ) );
        }
        else
        {
            m_CorpseName = (eCorpseName)NameToEnum( I.GetVarEnum() );
        }

        return TRUE;
    }

    // Material
    if ( I.IsVar( "DeadBody\\Material" ) ) 
    {
        // Updating UI?
        if( I.IsRead() )
        {
            if ( s_CorpseMaterialEnum.DoesValueExist( m_Material ) )
            {
                I.SetVarEnum( s_CorpseMaterialEnum.GetString( m_Material ) );
            }
            else
            {
                I.SetVarEnum( "NULL" );
            } 
        }
        else
        {
            // Reading from UI/File
            const char* pValue = I.GetVarEnum();

            // Found?
            if( !s_CorpseMaterialEnum.GetValue( pValue, m_Material ) )
                m_Material = object::MAT_TYPE_NULL;
        }

        return TRUE;      
    }        

    // Active physics?
    if( I.IsVar( "DeadBody\\Active" ) )
    {
        if( I.IsRead() )
            I.SetVarBool( m_bActive );
        else
            m_bActive = I.GetVarBool();

        return TRUE;
    }

    // Actor collision?
    if( I.IsVar( "DeadBody\\ActorCollision" ) )
    {
        if( I.IsRead() )
            I.SetVarBool( m_bActorCollision );
        else
            m_bActorCollision = I.GetVarBool();

        m_PhysicsInst.SetActorCollision( m_bActorCollision );

        return TRUE;
    }
    
    // World collision?
    if( I.IsVar( "DeadBody\\WorldCollision" ) )
    {
        if( I.IsRead() )
            I.SetVarBool( m_bWorldCollision );
        else
            m_bWorldCollision = I.GetVarBool();

        m_PhysicsInst.SetWorldCollision( m_bWorldCollision );

        return TRUE;
    }
    
    // ActiveWhenVisible?
    if( I.IsVar( "DeadBody\\ActiveWhenVisible" ) )
    {
        if( I.IsRead() )
            I.SetVarBool( m_bActiveWhenVisible );
        else
            m_bActiveWhenVisible = I.GetVarBool();

        m_PhysicsInst.SetActiveWhenVisible( m_bActiveWhenVisible );

        return TRUE;
    }

    // SimulationTime
    if( I.VarFloat( "DeadBody\\SimulationTime", m_SimulationTime, 0.0f, 5.0f ) )
    {
        #ifdef X_EDITOR
            // Re-initialize
            if( !I.IsRead() )
                InitializeEditorPlaced();
        #endif // X_EDITOR

        return TRUE;
    }

    // AnimGroup, AnimName?
    if( SMP_UTIL_IsAnimVar( I, 
                            "DeadBody\\AnimGroupName",
                            "DeadBody\\AnimName",
                            m_PhysicsInst.GetAnimGroupHandle(),
                            m_AnimGroupName,
                            m_AnimName ) )
    {
#ifdef X_EDITOR
        // Re-initialize
        if( I.IsRead() == FALSE )                            
            InitializeEditorPlaced();
#endif            
        return TRUE;
    }
    
    // AnimFrame
    if (I.VarInt( "DeadBody\\AnimFrame", m_AnimFrame, 0 ))
    {
        #ifdef X_EDITOR
            // Re-initialize
            if ( !I.IsRead() )
                InitializeEditorPlaced();
        #endif // X_EDITOR

        return TRUE;
    }

    // Geometry
    if( GetSkinInst().OnProperty( I ) )
    {
        #ifdef X_EDITOR
            // Re-initialize if geometry is selected
            if ( ( !I.IsRead() ) && ( I.IsVar( "RenderInst\\File" ) ) )
                InitializeEditorPlaced();
        #endif // X_EDITOR

        return TRUE;
    }

    // Decal package
    if( I.IsVar( "BloodDecals\\Package" ) )
    {
        if( I.IsRead() )
            I.SetVarExternal( m_hBloodDecalPackage.GetName(), RESOURCE_NAME_SIZE );
        else
            m_hBloodDecalPackage.SetName( I.GetVarExternal() );

        return TRUE;
    }

    // Decal group
    if( I.VarInt( "BloodDecals\\Group", m_BloodDecalGroup ) )
    {
        return TRUE;
    }

    return FALSE;
}

//=========================================================================

const char* corpse::EnumToName( eCorpseName theCorpse )
{
    ASSERT( (theCorpse >= 0) && (theCorpse < CORPSE_NAME_MAX) );
    const char* pName = s_CorpseTable[theCorpse].pName;
    CLOG_MESSAGE( CORPSE_LOGGING, "corpse::EnumToName", "%d = '%s'", theCorpse, pName );
    return pName;
}

//=========================================================================

xstring corpse::GetEnumStringCorpse( void )
{
    xstring EnumString;

    for( s32 i=0 ; i<NUM_CORPSE_TABLE_ENTRIES ; i++ )
    {
        EnumString += EnumToName( (eCorpseName)i );
        EnumString += '\0';
    }

    return EnumString;
}

//=========================================================================

eCorpseName corpse::NameToEnum( const char* pName )
{
    // Search the table for the corpse name
    for( s32 i=0 ; i<NUM_CORPSE_TABLE_ENTRIES ; i++ )
    {
        if( x_strcmp( pName, s_CorpseTable[i].pName ) == 0 )
        {
            ASSERT( i == s_CorpseTable[i].Item );
            CLOG_MESSAGE( CORPSE_LOGGING, "corpse::NameToEnum", "Found '%s' = %d", pName, i );
            return (eCorpseName)i;
        }
    }

    // Not found so NULL
    CLOG_ERROR( CORPSE_LOGGING, "corpse::NameToEnum", " Not Found '%s'", pName );
    return CORPSE_GENERIC;
}

//=========================================================================

const char* corpse::GetScanIdentifier( void )
{
    eCorpseName theCorpse = m_CorpseName;
    ASSERT( (theCorpse >= 0) && (theCorpse < CORPSE_NAME_MAX) );
    const char* pIdentifier = s_CorpseTable[theCorpse].pIdentifier;
    CLOG_MESSAGE( CORPSE_LOGGING, "corpse::GetScanIdentifier", "%d = '%s'", theCorpse, pIdentifier );
    return pIdentifier;
}

//=========================================================================

void corpse::StartFading( void )
{
    // Start the fade
    m_TimeAlive = CORPSE_FADEOUT_START_TIME;
    
    // Put the rigid bodies to sleep so that the physics system does not run out of constraints
    m_PhysicsInst.Deactivate();
    m_PhysicsInst.SetInstCollision( FALSE );
}

//===============================================================================

void corpse::StartFading( f32 FadeOutTime )
{
    ASSERT( FadeOutTime > 0.0f );
    m_FadeOutTime = FadeOutTime;
    StartFading();
}

//===============================================================================
