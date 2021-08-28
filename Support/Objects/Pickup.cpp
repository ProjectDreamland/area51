//==============================================================================
//
//  Pickup.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Pickup.hpp"
#include "CollisionMgr\CollisionMgr.hpp"
#include "Entropy.hpp"
#include "Inventory/Inventory2.hpp"
#include "Objects/Player.hpp"
#include "../Support/NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/GameMgr.hpp"

//=========================================================================

#ifdef cgalley
#define ENABLE_LOGGING  1
#else
#define ENABLE_LOGGING  0
#endif

#define FADEIN_TIME  1.0f
#define FADEOUT_TIME 0.4f

//=========================================================================

static struct pickup_desc : public object_desc
{
    pickup_desc( void ) : object_desc( object::TYPE_PICKUP, 
        "Pickup",
        "PICKUPS",
        object::ATTR_SPACIAL_ENTRY      |
        object::ATTR_NEEDS_LOGIC_TIME   |
        object::ATTR_SOUND_SOURCE		|
        object::ATTR_RENDERABLE			|
        object::ATTR_COLLIDABLE         |
        object::ATTR_COLLISION_PERMEABLE|
        object::ATTR_BLOCKS_PLAYER,
        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC            |
        FLAGS_BURN_VERTEX_LIGHTING )
    {
    }

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new pickup; }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "RigidGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

} s_PickupDesc;

//---------------------------------------------------------------------------------

const object_desc& pickup::GetObjectType( void )
{
    return s_PickupDesc;
}

//---------------------------------------------------------------------------------

const object_desc& pickup::GetTypeDesc( void ) const
{
    return s_PickupDesc;
}

//==============================================================================

//==============================================================================

//==============================================================================

pickup* pickup::CreatePickup( guid              OriginGuid,
                              s32               OriginNetSlot,
                              inven_item        Item,
                              f32               Amount,
                              f32               DecayTime,
                              const vector3&    Position,
                              const radian3&    Orientation,
                              const vector3&    Velocity,
                              s32               Zone1,
                              s32               Zone2 )
{
    // Get the name of the geom for the pickup to create
    const char* pGeomName = inventory2::ItemToPickupGeomName( Item, Amount );
    ASSERT( pGeomName );

    // Did we get a valid name?
    if( pGeomName )
    {
        // Create a pickup
        guid PickupGuid = CREATE_NET_OBJECT( pickup::GetObjectType(), TYPE_PICKUP );
        pickup* pPickup = (pickup*)g_ObjMgr.GetObjectByGuid( PickupGuid );
        ASSERT( pPickup );
        if( pPickup )
        {
            pPickup->m_Item   = Item;
            pPickup->m_Amount = Amount;

            // net_proj setup
            pPickup->SetOrigin( OriginGuid, OriginNetSlot );
            pPickup->SetStart( Position, Orientation, Velocity, Zone1, Zone2, 980.0f );

            // Get a Geom to render
            pPickup->SetupRigidGeom( pGeomName );

            // Setup either Decaying or Idle
            pPickup->m_DecayTime = DecayTime;
            if( pPickup->m_DecayTime > 0.0f )
            {
                pPickup->m_Timer = pPickup->m_DecayTime;
                pPickup->m_State = STATE_DECAYING;
            }
            else
            {
                pPickup->m_Timer = 0.0f;

#ifndef X_EDITOR
                if( GameMgr.IsGameMultiplayer() )
                {
                    pPickup->m_State = STATE_WAIT_RESPAWN;
                }
                else
#endif
                {
                    pPickup->m_State = STATE_IDLE;
                }
            }

            // This is a dynamically created pickup
            pPickup->m_IsDynamic = TRUE;
            pPickup->m_bTakeable = TRUE;
        }

#ifndef X_EDITOR
        if( GameMgr.IsGameMultiplayer() )
        {
            // Real fast now.
            pPickup->m_Inst.StartFade( 1, 0.1f );
        }
        pPickup->m_NetDirtyBits |= DIRTY_ALL;
#endif

        // Return it
        return pPickup;
    }
    else
    {
        LOG_ERROR( "pickup::CreatePickup", "Attempting to create a pickup for '%s'", inventory2::ItemToName( Item ) );
    }

    // Failed
    return NULL;
}

//==============================================================================

pickup::pickup( void ) : 
    m_Item              ( INVEN_NULL ),
    m_Amount            ( 0 ),
    m_State             ( pickup::STATE_WAIT_RESPAWN ),
    m_IsDynamic         ( FALSE ),
	m_RespawnTime       ( -1.0f ),
    m_DecayTime         ( -1.0f ),
    m_Timer             ( 0.1f  ),
    m_PlayerIndex       ( -1    ),
	m_bIsActive         ( TRUE  ),
    m_bHideWhileInactive( FALSE ),
    m_bHasBeenPickedup  ( FALSE ),
	m_bTakeable         ( TRUE  ),
    m_MinPlayers        ( 0     ),
    m_MaxPlayers        ( 32    ),
    m_bSpins            ( TRUE  ),
    m_Rotation          ( R_0   )
{
    m_bIsLargeProjectile = TRUE;
#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        m_Inst.StartFade( 1, 0.1f );
    }
#endif 
    m_GeomL2W.Identity();
}

//==============================================================================

pickup::~pickup(void)
{
}

//==============================================================================

void pickup::OnEnumProp( prop_enum&  rPropList )
{
    net_proj::OnEnumProp( rPropList );
    m_Inst.OnEnumProp ( rPropList );

    rPropList.PropEnumHeader( "Pickup", "Pickup items properties", PROP_TYPE_HEADER );
    s32 iHeader = rPropList.PushPath( "Pickup\\" );

    rPropList.PopPath( iHeader );

	//object info
	rPropList.PropEnumString( "Pickup", "General pickup information", PROP_TYPE_HEADER );
	rPropList.PropEnumBool	( "Pickup\\DoesItemRespawn" , "Does this item respawn?", 0 );
	rPropList.PropEnumFloat	( "Pickup\\RespawnTime", "How long after collection does it take for this item to respawn?", 0 );
    
    rPropList.PropEnumBool	( "Pickup\\IsActive" , "Is this item active (defaults to TRUE)?", PROP_TYPE_EXPOSE );
    rPropList.PropEnumBool	( "Pickup\\HideWhileInactive" , "If this item is inactive, hide it (defaults to FALSE)", PROP_TYPE_EXPOSE );

    rPropList.PropEnumBool	( "Pickup\\HasBeenPickedup" , "Has this item been picked up? (resets to FALSE after respawn)", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SHOW );

    rPropList.PropEnumEnum  ( "Pickup\\Item"  , inventory2::GetEnumString(), "The type of this pickup.", 0 );
    rPropList.PropEnumFloat ( "Pickup\\Amount", "The amount in this pickup.", 0 );

    rPropList.PropEnumExternal( "Pickup\\FxFile" ,       "Resource\0fxo",     "File of particle type" , PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumExternal( "Pickup\\Audio Package", "Resource\0audiopkg","The audio package associated with this pickup object.", 0 );

    rPropList.PropEnumInt( "Pickup\\MinPlayers", "What's the min number of players this pickup will spawn for?", 0 );
    rPropList.PropEnumInt( "Pickup\\MaxPlayers", "What's the max number of players this pickup will spawn for?", 0 );

    rPropList.PropEnumBool    ( "Pickup\\Spins", "Does this pickup spin in place?", 0 );
}

//==============================================================================

xbool pickup::OnProperty( prop_query& rPropQuery )
{
    if( net_proj::OnProperty( rPropQuery ) )
	{
        return TRUE;
	}

    else if( m_Inst.OnProperty( rPropQuery ) )
	{
        SetupRigidGeom();
        return TRUE;
	}

    else if( rPropQuery.VarFloat( "Pickup\\RespawnTime", m_RespawnTime ) )
	{
        return TRUE;
	}

    else if( rPropQuery.VarBool( "Pickup\\Spins", m_bSpins ) )
    {
        return TRUE;
    }
    else if( rPropQuery.VarBool( "Pickup\\IsActive", m_bIsActive ) )
    {
        return TRUE;
    }
    else if( rPropQuery.IsVar( "Pickup\\HasBeenPickedup" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarBool( m_bHasBeenPickedup );
        }

        return TRUE;
    }
	else if( rPropQuery.VarBool( "Pickup\\HideWhileInactive", m_bHideWhileInactive ) )
	{
		return TRUE;
	}
    else if( rPropQuery.IsVar( "Pickup\\Item" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( inventory2::ItemToName( m_Item ) );
        }
        else
        {
            m_Item = (inven_item)inventory2::NameToItem( rPropQuery.GetVarEnum() );
            //SetupRigidGeom();
        }

        return TRUE;
    }

    else if( rPropQuery.VarFloat( "Pickup\\Amount", m_Amount ) )
    {
        SetupRigidGeom();
        return TRUE;
    }

    else if( rPropQuery.VarInt( "Pickup\\MinPlayers", m_MinPlayers ) )
    {
        return TRUE;
    }
    else if( rPropQuery.VarInt( "Pickup\\MaxPlayers", m_MaxPlayers ) )
    {
        return TRUE;
    }

    else if( rPropQuery.IsVar( "Pickup\\FxFile" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_hParticleEffect.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = rPropQuery.GetVarExternal();

            if( pString && pString[0] )
            {
                m_hParticleEffect.SetName( pString );                

                // Load the particle effect.
                if( m_hParticleEffect.IsLoaded() == FALSE )
                    m_hParticleEffect.GetPointer();
            }
        }
        return( TRUE );
    }

    else if( rPropQuery.IsVar( "Pickup\\Audio Package" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_hAudioPackage.GetName(), RESOURCE_NAME_SIZE );
        }
        else
        {
            // Get the FileName
            const char* pString = rPropQuery.GetVarExternal();

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
        return( TRUE );
    }
	else 
	{
	    return FALSE;
	}
	
	return TRUE;
}

//==============================================================================

xbool pickup::ShouldHidePickup( void )
{
    if( m_bHideWhileInactive && (!m_bIsActive) )
    {
        return TRUE;
    }

    return FALSE;
}

//==============================================================================

void pickup::OnRender ( void )
{
    CONTEXT( "pickup::OnRender" );

    rigid_geom* pRigidGeom = m_Inst.GetRigidGeom();

    if( pRigidGeom )
    {
        u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;

        // Campaign games should only render one vmesh, whereas multiplayer games
        // will have two vmeshes. The first one is the normal pickup geometry and
        // the second one is a glowy outer shell that makes the pickup easier to
        // see in a fast-paced multiplayer game.
        u32 VMeshMask = 0x1;
#ifndef X_EDITOR
        if( GameMgr.IsGameMultiplayer() && (m_Inst.GetAlpha() == 255) )
            VMeshMask = 0x3;
#endif
        m_Inst.SetVMeshMask( VMeshMask );

#ifdef X_EDITOR
        
        // draw the BBox so we can still see it, then get out
        if( ShouldHidePickup() )
        {
            draw_BBox(GetBBox());
            return;
        }

        m_Inst.Render( &GetL2W(), Flags | GetRenderMode() );        
#else  
        // don't render, hidden
        if( ShouldHidePickup() )
        {
            return;
        }

        m_Inst.Render( (m_bSpins && GameMgr.IsGameMultiplayer()) ? &m_GeomL2W : &GetL2W(), Flags | GetRenderMode() );
#endif

    }
    else
    {
#ifdef X_EDITOR
        draw_BBox( GetBBox() );
#endif
    }
}

//==============================================================================

void pickup::OnInit(void)
{
    net_proj::OnInit();
}

//==============================================================================

void pickup::OnActivate( xbool Flag )
{    
    m_bIsActive = Flag;
}

//==============================================================================

bbox pickup::GetLocalBBox( void ) const 
{ 
    if( m_Inst.GetRigidGeom() )
    {
        bbox BBox = m_Inst.GetBBox();
        BBox.Inflate( 75.0f, 75.0f, 75.0f );
        return BBox;
    }
    else
    {
        return bbox( vector3(-75, -75, -75), vector3(75,75,75) );
    }
}

//==============================================================================

void pickup::OnKill( void )
{
    net_proj::OnKill();
}

//==============================================================================

void pickup::OnMove ( const vector3& NewPos )
{
    net_proj::OnMove(NewPos);
}

//==============================================================================

void pickup::OnTransform ( const matrix4& L2W )
{ 
    net_proj::OnTransform(L2W);
}

//==============================================================================

void pickup::OnColCheck ( void )
{
    g_CollisionMgr.StartApply( GetGuid() );
    g_CollisionMgr.ApplyAABBox( GetBBox() );
    g_CollisionMgr.EndApply();
}

//==============================================================================

void pickup::OnAdvanceLogic  ( f32 DeltaTime )
{
    m_Inst.OnAdvanceLogic( DeltaTime );

#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        if( m_AtRest )
        {
            m_Rotation += DeltaTime;
        }
            
        if( m_Rotation > (2.0f * PI) )
        {
            m_Rotation -= (2.0f * PI);
        }

        if( m_bSpins  )
        {
            bbox    BBox       = GetLocalBBox();
            vector3 BBoxCenter = BBox.GetCenter();

            matrix4 RotMatrix;
            f32     Sin, Cos;

            // We'd like to spin the pickup to make it more noticeable, but
            // the pickup is not always centered on the origin, so we'll need
            // to use the bbox center instead. So...move the object by
            // -BBoxCenter, rotate it, then move it back by the BBoxCenter.
            // Then apply the normal L2W matrix on it. For performance
            // reasons we'll factor some of the math out rather than
            // using pure x_files math. That's because this will be called
            // a lot and was showing up on the profile radar.
            x_sincos( m_Rotation, Sin, Cos );
            RotMatrix.Identity();
            RotMatrix(0,0) =  Cos;
            RotMatrix(2,0) =  Sin;
            RotMatrix(0,2) = -Sin;
            RotMatrix(2,2) =  Cos;
            RotMatrix.PreTranslate( -BBoxCenter );
            RotMatrix.Translate( BBoxCenter );
            m_GeomL2W = GetL2W() * RotMatrix;
            m_GeomL2W.Translate( vector3( 0.0f, (10.0f * Sin) + 20.0f, 0.0f ) );
        }
    }

#endif

    if( m_State != STATE_DESTROY )
        net_proj::OnAdvanceLogic( DeltaTime );

    // Fallen out of world?
    vector3 Position  = GetPosition();
    bbox    WorldBBox = g_ObjMgr.GetSafeBBox();
    WorldBBox.Inflate( 500.0f, 500.0f, 500.0f );
    if( !WorldBBox.Intersect( Position ) )
    {
#ifndef X_EDITOR
        // If this is a server, simply destroy and exit now...
        if( g_NetworkMgr.IsServer() )
        {
            DESTROY_NET_OBJECT( this );
            return;        
        }            
#endif
        
        // Clamp position to world bounds
        if( Position.GetX() < WorldBBox.Min.GetX() )
            Position.GetX() = WorldBBox.Min.GetX();
        if( Position.GetX() > WorldBBox.Max.GetX() )
            Position.GetX() = WorldBBox.Max.GetX();

        if( Position.GetY() < WorldBBox.Min.GetY() )
            Position.GetY() = WorldBBox.Min.GetY();
        if( Position.GetY() > WorldBBox.Max.GetY() )
            Position.GetY() = WorldBBox.Max.GetY();

        if( Position.GetZ() < WorldBBox.Min.GetZ() )
            Position.GetZ() = WorldBBox.Min.GetZ();
        if( Position.GetZ() > WorldBBox.Max.GetZ() )
            Position.GetZ() = WorldBBox.Max.GetZ();
            
        // Update            
        OnMove( Position );            
    }


    // No local logic for clients
#ifndef X_EDITOR
    if( g_NetworkMgr.IsClient() )
        return;
#endif

    // Update the state timer
    if( m_Timer > 0.0f )
        m_Timer -= DeltaTime;

    // Run state logic
    switch( m_State )
    {
    case STATE_DECAYING:
        if( m_Timer <= 0.0f )
        {
            m_State = STATE_FADE_OUT;
            m_Timer = FADEOUT_TIME + 0.5f; // Just a little cushion for variable latency.
#ifndef X_EDITOR
            m_NetDirtyBits |= DIRTY_STATE;
#endif
        }
        break;

    case STATE_FADE_OUT:
        m_Inst.StartFade( -1, FADEOUT_TIME );

        if( m_Timer <= 0.0f )
        {
            m_State = STATE_DESTROY;
            m_Timer = 5.0f;
#ifndef X_EDITOR
            m_NetDirtyBits |= DIRTY_STATE;
#endif
        }
        break;

    case STATE_WAIT_RESPAWN:
        if( m_Timer <= 0.0f )
        {
            m_Timer = 0.0f; // TODO: CJ: Currently no time taken by the respawning process, this will change
#ifndef X_EDITOR
            if( IN_RANGE( m_MinPlayers, GameMgr.GetScore().NPlayers, m_MaxPlayers ) )
            {                   
                m_State = STATE_RESPAWNING;
                m_NetDirtyBits |= DIRTY_STATE;
            }
#else
            m_State = STATE_RESPAWNING;
#endif
        }
        break;

    case STATE_RESPAWNING:
        m_bTakeable = TRUE;
        if( m_Timer <= 0.0f )
        {
            m_Timer = 0.0f;
            m_State = STATE_IDLE;

            // we respawned, reset picked up flag
            m_bHasBeenPickedup = FALSE;
#ifndef X_EDITOR
            m_NetDirtyBits |= DIRTY_STATE;
#endif

            m_Inst.StartFade( 1, 1.0f );
/*
            SetAttrBits( object::ATTR_SPACIAL_ENTRY         |
                         object::ATTR_SOUND_SOURCE		    |
                         object::ATTR_RENDERABLE			|
                         object::ATTR_TRANSPARENT           |
                         object::ATTR_COLLIDABLE            |
                         object::ATTR_COLLISION_PERMEABLE   |
                         object::ATTR_BLOCKS_PLAYER );
*/
        }
        break;

    case STATE_DESTROY:
        if( m_Timer <= 0.0f )
        {
            DESTROY_NET_OBJECT( this );
        }
        break;
    }
}

//==============================================================================

void pickup::OnColNotify( object& Object )
{
    // Exit if we have been collected already
    if( (m_State != STATE_IDLE) && (m_State != STATE_DECAYING) )
        return;

    // have we been deactivated somehow?
    if( !m_bIsActive )
    {
        return;
    }

    // if the pickup is hidden, don't pick it up
    if( ShouldHidePickup() )
    {
        return;
    }

    // Add the pickup to the inventory of the colliding object
    if( Object.IsKindOf( actor::GetRTTI() ) )
    {
        // Cast to actor.
        actor& Actor = actor::GetSafeType( Object );

        // Can the actor accept this pickup?
        if( Actor.OnPickup( *this ) )
        {
            m_bTakeable = FALSE;

            // someone grabbed us! (exposed property)
            m_bHasBeenPickedup = TRUE;

#ifndef X_EDITOR
            if( g_NetworkMgr.IsServer() )
#endif
            {
                #ifndef X_EDITOR
                m_PlayerIndex   = 0;
                m_NetDirtyBits |= DIRTY_ASSIGN; // So that clients know to play the sound.
                #endif
                ProcessTake( Actor );
            }
        }
    }
}

//==============================================================================

void pickup::ProcessTake( actor& Actor )
{
    #ifndef X_EDITOR

    ASSERT( g_NetworkMgr.IsServer() );

    /*
    LOG_MESSAGE( "pickup::ProcessTake", 
                 "Player:%d - Pickup:%d - Type:%d",
                 Actor.net_GetSlot(),
                 m_NetSlot,
                 (s32)m_Item );
                 */
    #endif

    // Play sound 2D for local players, 3D for others, only local players in the editor
#if defined( X_EDITOR )
    g_AudioMgr.Play( GetSound() );
#else
    if( g_NetworkMgr.IsLocal( Actor.net_GetSlot() ) )
        g_AudioMgr.Play( GetSound() );
    else
        g_AudioMgr.PlayVolumeClipped( GetSound(), GetPosition(), GetZone1(), TRUE );
#endif

    if( m_Item == INVEN_GRENADE_FRAG )
    {
        for( s32 i = 0; i < GetAmount(); i++ )
        {
            if( Actor.net_GetSlot() != 0 )
            {
                Actor.AddItemToInventory2( INVEN_GRENADE_FRAG );
            }
        }
    }

    if( m_Item == INVEN_GRENADE_JBEAN )
    {
        for( s32 i = 0; i < GetAmount(); i++ )
        {
            if( Actor.net_GetSlot() != 0 )
            {
                Actor.AddItemToInventory2( INVEN_GRENADE_JBEAN );
            }
        }
    }

    // If we're supposed to respawn after a certain amount of time setup for respawn
    if( m_RespawnTime > 0.0f )
    {
        // Setup to respawn
        m_State = STATE_WAIT_RESPAWN;
        m_Timer = m_RespawnTime;
        #ifndef X_EDITOR
        m_NetDirtyBits |= DIRTY_STATE;
        #endif

        // Hide the pickup
        m_Inst.StartFade( -1, FADEOUT_TIME );
//      SetAttrBits( ATTR_NEEDS_LOGIC_TIME | ATTR_SPACIAL_ENTRY );
    }
    // Otherwise destroy this pickup
    else
    {
        m_State = STATE_FADE_OUT;
        m_Timer = FADEOUT_TIME + 0.5f;

        #ifndef X_EDITOR
        m_NetDirtyBits |= DIRTY_STATE;
        #endif

        //SetAttrBits( ATTR_NEEDS_LOGIC_TIME );
    }
}

//==============================================================================

void pickup::SetupRigidGeom( const char* pGeomName /* = NULL */ )
{
    // Get a Geom to render
#ifndef X_EDITOR
    if( GameMgr.IsGameMultiplayer() )
    {
        const char* pGeomName = inventory2::ItemToPickupGeomName( m_Item, m_Amount );
        m_Inst.SetUpRigidGeom( pGeomName );
    }
    else
#endif
    if( pGeomName )
    {
        m_Inst.SetUpRigidGeom( pGeomName );
    }
}
    

//==============================================================================
#ifndef X_EDITOR
//------------------------------------------------------------------------------

void pickup::net_AcceptUpdate( const bitstream& BS )

{
    if( g_NetworkMgr.IsClient() )
    {
        m_bTakeable = TRUE;
    }
    
    if( BS.ReadFlag() )
    {
        s32 Int;
        BS.ReadRangedS32( Int, INVEN_NULL, INVEN_GLOVES );
        m_Item = (inven_item)Int;
        BS.ReadVariableLenS32( Int );
        m_Amount = (f32)Int;

        BS.ReadFlag( m_bSpins );

        SetupRigidGeom();
    }

    // DIRTY_STATE
    if( BS.ReadFlag() )
    {
        ASSERT( g_NetworkMgr.IsClient() );
        s32 NewState;
        BS.ReadRangedS32( NewState, STATE_FIRST, STATE_LAST );
        m_State = (state)NewState;

        // Fade in
        if( (m_State == STATE_RESPAWNING) || (m_State == STATE_IDLE) )
            m_Inst.StartFade( 1, FADEIN_TIME );

        // Blink out
        if( (m_State == STATE_WAIT_RESPAWN) || (m_State == STATE_DESTROY) )
            m_Inst.StartFade( -1, FADEOUT_TIME );

        m_NetDirtyBits |= DIRTY_STATE;
    }

    // DIRTY_REQUEST
    if( BS.ReadFlag() )
    {
        ASSERT( g_NetworkMgr.IsServer() );
        BS.ReadRangedS32( m_PlayerIndex, 0, 31 );
    //  LOG_MESSAGE( "pickup::net_AcceptUpdate", "Requested By:%d", m_PlayerIndex );

        // A request has come in from a client player for this pickup.
        // If the pickup is available, then we will assign it.

        if( (GameMgr.GetScore().Player[m_PlayerIndex].IsInGame) &&
            m_bTakeable &&
            ( (m_State == STATE_IDLE) ||
              (m_State == STATE_DECAYING) ||
              (m_State == STATE_RESPAWNING) ) )
        {
            // Pickup is available, so the player can have it.  Except for
            // health pickups, we simply send back an ASSIGN to the client
            // and let him deal with it.  For health pickups, we also have to
            // bump up his health here.  We could wait and just let the client 
            // player process it, but (a) that adds latency to the health boost,
            // and (b) we are currently not allowing clients to boost their own
            // health.

            m_bTakeable = FALSE;

            object* pObject = NetObjMgr.GetObjFromSlot( m_PlayerIndex );
            if( pObject )
            {
                ASSERT( pObject->IsKindOf( actor::GetRTTI() ) );
                actor& Actor = *((actor*)pObject);

                m_NetDirtyBits |= DIRTY_ASSIGN;
                ProcessTake( Actor );

                if( m_Item == INVEN_HEALTH )
                {
                    Actor.AddHealth( m_Amount );
                }
            }

            /*
            LOG_MESSAGE( "pickup::net_AcceptUpdate",
                         "Assigned To:%d", m_PlayerIndex );
            */
        }
    }

    // DIRTY_ASSIGN
    if( BS.ReadFlag() )
    {
        ASSERT( g_NetworkMgr.IsClient() );
        BS.ReadRangedS32( m_PlayerIndex, 0, 31 );

        /*
        LOG_MESSAGE( "pickup::net_AcceptUpdate",
                     "Assigned To:%d", m_PlayerIndex );
        */

        // The server has assigned this pickup to a player.  If the player is
        // local on this client, then let him have it!

        if( g_NetworkMgr.IsLocal( m_PlayerIndex ) )
        {
            object* pObject = NetObjMgr.GetObjFromSlot( m_PlayerIndex );
            if( (GameMgr.GetScore().Player[m_PlayerIndex].IsInGame) && pObject )
            {
                ASSERT( pObject->IsKindOf( player::GetRTTI() ) );
                player& Player = *((player*)pObject);
                Player.TakePickup( *this );

                g_AudioMgr.Play( GetSound() );

                /*
                LOG_MESSAGE( "pickup::net_AcceptUpdate",
                             "Taken By:%d", m_PlayerIndex );
                */
            }
        }    
        else
        {
            // Play the pickup sound.
            g_AudioMgr.PlayVolumeClipped( GetSound(), GetPosition(), GetZone1(), TRUE );
        }
    }

    // Ancestor class.
    if( BS.ReadFlag() )
    {
        net_proj::net_AcceptUpdate( BS );
    }
}

//==============================================================================

void pickup::net_ProvideUpdate( bitstream& BS, u32& DirtyBits )
{
    if( BS.WriteFlag( DirtyBits & ACTIVATE_BIT ) )
    {
        DirtyBits |= DIRTY_STATE;

        BS.WriteRangedS32( m_Item, INVEN_NULL, INVEN_GLOVES );
        BS.WriteVariableLenS32( (s32)m_Amount );

        BS.WriteFlag( m_bSpins );

        // Don't clear the dirty bit, let the net_proj do that when it does its
        // own activation processing.
    }

    // DIRTY_STATE
    if( BS.WriteFlag( DirtyBits & DIRTY_STATE ) )
    {
        ASSERT( g_NetworkMgr.IsServer() );
        BS.WriteRangedS32( m_State, STATE_FIRST, STATE_LAST );
        DirtyBits &= ~DIRTY_STATE;
    }

    // DIRTY_REQUEST
    if( BS.WriteFlag( DirtyBits & DIRTY_REQUEST ) )
    {
        ASSERT( g_NetworkMgr.IsClient() );
        BS.WriteRangedS32( m_PlayerIndex, 0, 31 );
        DirtyBits &= ~DIRTY_REQUEST;
        /*
        LOG_MESSAGE( "pickup::net_ProvideUpdate",
                     "Requested By:%d", m_PlayerIndex );
        */
    }

    // DIRTY_ASSIGN
    if( BS.WriteFlag( DirtyBits & DIRTY_ASSIGN ) )
    {
        ASSERT( g_NetworkMgr.IsServer() );
        BS.WriteRangedS32( m_PlayerIndex, 0, 31 );
        DirtyBits &= ~DIRTY_ASSIGN;
        /*
        LOG_MESSAGE( "pickup::net_ProvideUpdate",
                     "Assigned To:%d", m_PlayerIndex );
        */
    }

    // Ancestor class.
    if( BS.WriteFlag( DirtyBits ) )
    {
        net_proj::net_ProvideUpdate( BS, DirtyBits );
    }
}

//==============================================================================

u32 pickup::net_GetUpdateMask( s32 TargetClient ) const
{
    u32 Mask = 0x00000000;

    if( TargetClient == -1 )
    {
        // Bits going TO the server, FROM a client.
        // Only the REQUEST bit.
        Mask = DIRTY_REQUEST;
    }
    else
    {
        // Bits going TO a client, FROM the server.
        // All but the REQUEST bit.
        Mask = ~DIRTY_REQUEST;
    }

    return( Mask );
}

//------------------------------------------------------------------------------
#endif
//==============================================================================

void pickup::PlayerRequest( s32 PlayerIndex )
{
    (void)PlayerIndex;

    #ifndef X_EDITOR
    ASSERT( g_NetworkMgr.IsClient() );

    m_PlayerIndex   = PlayerIndex;
    m_NetDirtyBits |= DIRTY_REQUEST;
    #endif
}

//==============================================================================

const char* pickup::GetSound()
{
    const char* pSoundName;

    switch( m_Item )
    {
    // TODO: Add additional sounds here.
    case INVEN_HEALTH:
        pSoundName = "HealthPickup";
        break;
    default:
        pSoundName = "HealthPickup";
        break;
    }

    return pSoundName;
}

//==============================================================================

