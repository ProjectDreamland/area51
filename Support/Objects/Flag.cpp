//==============================================================================
//
//  Flag.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Flag.hpp"
#include "GameLib/RenderContext.hpp"
#include "TemplateMgr/TemplateMgr.hpp"
#include "Entropy/e_Draw.hpp"
#include "Objects/Actor/Actor.hpp"
#include "Objects/Player.hpp"
#include "Objects\BaseProjectile.hpp"
#include "Objects\HudObject.hpp"
#include "NetworkMgr\logic_ctf.hpp"
#include "FlagBase.hpp"
#include "Objects/ClothObject.hpp"

#ifndef X_EDITOR
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/logic_Base.hpp"
#endif

//==============================================================================
//  DEFINES
//==============================================================================

static f32 FLAG_OBJECT_AIR_RESISTANCE = 0.25f;



//==============================================================================
//  OBJECT DESCRIPTION
//==============================================================================

static struct flag_desc : public object_desc
{
    flag_desc( void ) 
        :   object_desc( object::TYPE_FLAG, 
        "Flag",
        "Multiplayer",
        object::ATTR_NEEDS_LOGIC_TIME     |
        object::ATTR_COLLIDABLE           |
        object::ATTR_BLOCKS_ALL_PROJECTILES |
        object::ATTR_BLOCKS_ALL_ACTORS    |
        object::ATTR_BLOCKS_RAGDOLL       |
        object::ATTR_BLOCKS_SMALL_DEBRIS  |
        object::ATTR_DAMAGEABLE           |
        object::ATTR_RENDERABLE           |
        object::ATTR_TRANSPARENT          |
        object::ATTR_SPACIAL_ENTRY        |
        object::ATTR_NO_RUNTIME_SAVE,
        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC )
    {
        // Empty function body.
    }

    //--------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        return( new flag ); 
    }

} s_flag_Desc;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& flag::GetTypeDesc( void ) const
{
    return( s_flag_Desc );
}

//==============================================================================

const object_desc& flag::GetObjectType( void )
{
    return( s_flag_Desc );
}

//==============================================================================

flag::flag( void )
{
    m_GeomReady   = FALSE;
    m_Yaw         = R_0;
    m_Age         = 0.0f;
    m_TimeOut     = TRUE;
    m_TimeOutAge  = 20.0f;
    m_Attached    = FALSE;
    m_Player      = -1;
    m_Bone        =  1;
    m_Holes       =  0;
    m_IgnoreBits  = 0x00000000;
    m_IgnoreTimer = 0.0f;
    m_bRendered   = TRUE;
    m_IconOpacity = 0.0f;
    m_BaseAlpha   = 0.0f;

    m_bIsSettled  = TRUE;
    m_Velocity    = 0.0f;

    m_VanishPos.Zero();

    for( s32 i = 0; i < NUM_FLAG_EFFECTS; i++ )
    {
        m_StartEffects[ i ] = FALSE;
        m_EffectActive[ i ] = FALSE;
    }

    // Reset cloth.
    m_Cloth.Reset();
    m_ActiveTimer = 0.0f;
}

//==============================================================================

flag::~flag( void )
{
    m_Cloth.Kill();

    for( s32 i = 0; i < NUM_FLAG_EFFECTS; i++ )
    {
        m_FlagFX[ i ].KillInstance();
    }
}

//==============================================================================

void flag::InitGeometry( u32 TeamBits )
{
    if( !m_GeomReady )
    {
        guid GUID = GetGuid();

        {
            g_TemplateMgr.ApplyTemplateToObject( 
                "C:\\GameData\\A51\\Source\\Themes\\Multiplayer\\Assets\\Blueprint\\Flag00.bpx", 
                GUID );
        }
    }

    // Init effects.
    {
        char* pFlagEffects[] = { "MP_FlagReturned.fxo",          // FLAG_TIMEOUT
                                 "MP_ctf_flag_disappear.fxo",    // FLAG_CAPTURE
                                 "MP_ctf_flag_disappear.fxo",    // FLAG_TELEPORT
                                 "MP_ctf_flag_disappear.fxo"  }; // FLAG_SPAWN                  
        
        for( s32 i = 0; i < NUM_FLAG_EFFECTS; i++ )
        {
            rhandle<char> Resource;

            Resource.SetName( pFlagEffects[ i ] );

            m_FlagFX[ i ].InitInstance( Resource.GetPointer() );
            m_FlagFX[ i ].Restart();

            Resource.Destroy();
        }
    }


#ifndef X_EDITOR
    m_NetTeamBits = TeamBits;
#endif

    m_GeomReady   = TRUE;
}

//==============================================================================

void flag::InitCloth( void )
{
    // Reset cloth
    m_Cloth.SetL2W( GetL2W(), TRUE );

    // Force flag object bounds to update
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
}

//==============================================================================

void flag::SetTeamBits( u32 TeamBits )
{
    InitGeometry( TeamBits );
}

//==============================================================================

void flag::SetTimeOut( f32 TimeOutAge, f32 CurrentAge )
{
    m_TimeOut    = TRUE;
    m_TimeOutAge = TimeOutAge;
    m_Age        = CurrentAge;

#ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_TIMEOUT;
#endif
}           

//==============================================================================

void flag::ClearTimeOut( void )
{
    m_TimeOut    = FALSE;
    m_TimeOutAge = 0.0f;

#ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_TIMEOUT;
#endif
}

//==============================================================================

void flag::AddIgnore( s32 PlayerIndex )
{
    m_IgnoreBits |= (1 << PlayerIndex);
    m_IgnoreTimer = 5.0f;
}

//==============================================================================

void flag::AttachToPlayer( s32 PlayerIndex )
{
    m_Attached = TRUE;
    m_Player   = PlayerIndex;
    m_Bone     = 1;

    InitCloth();

    m_EffectActive[ FLAG_TIMEOUT ] = FALSE;

#ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_POSITION;
#endif
}

//==============================================================================

void flag::SetPosition( const vector3& Position, s32 Zone1, s32 Zone2, xbool bSetDirty /*= TRUE*/ )
{
    // This is a little HACKy.

    matrix4 L2W;

    L2W.Identity();
    L2W.RotateY( m_Yaw );
    L2W.Translate( Position );

    OnTransform( L2W );

    SetZone1( Zone1 );
    SetZone2( Zone2 );

    m_Attached = FALSE;
    m_Player   = -1;

#ifndef X_EDITOR
    if( bSetDirty )
    {
        m_NetDirtyBits |= DIRTY_POSITION;
    }
#endif
}

//==============================================================================

void flag::StartFall( void )
{
    m_Velocity   = 0.0f;
    m_bIsSettled = FALSE;

#ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_POSITION;
#endif
}

//==============================================================================

void flag::StopFall( void )
{
    m_Velocity   = 0.0f;
    m_bIsSettled = TRUE;

#ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_POSITION;
#endif
}

//==============================================================================

void flag::SetYaw( radian Yaw )
{
    m_Yaw = Yaw;

#ifndef X_EDITOR
    m_NetDirtyBits |= DIRTY_POSITION;
#endif
}

//==============================================================================

bbox flag::GetLocalBBox( void ) const 
{ 
    return m_Cloth.GetLocalBBox();
}

//==============================================================================

void flag::OnEnumProp( prop_enum& rPropList )
{
    object::OnEnumProp( rPropList );

    m_Cloth.OnEnumProp( rPropList );
}

//==============================================================================

xbool flag::OnProperty( prop_query& rPropQuery )
{       
    if( object::OnProperty( rPropQuery ) )
        return( TRUE );

    // Check cloth properties
    if( m_Cloth.OnProperty( rPropQuery ) )
    {
        // Was cloth just initialized from geometry?
        if( ( rPropQuery.IsRead() == FALSE ) && ( rPropQuery.IsVar( "RenderInst\\File" ) ) )
        {
            // Setup cloth guid and force bounds of flag to be recomputed
            m_Cloth.SetObjectGuid( GetGuid() );
            SetFlagBits( GetFlagBits() | object::FLAG_DIRTY_TRANSFORM );
        }

        return TRUE;
    }

    return( FALSE );
}


//==============================================================================

s32 flag::GetVTexture( void )
{
    s32 VTexture = 0;

    #ifndef X_EDITOR
    player* pPlayer = (player*)NetObjMgr.GetObjFromSlot( g_RenderContext.NetPlayerSlot );
    u32 PlayerTeamBits = pPlayer->net_GetTeamBits();
    u32 TeamBits       = net_GetTeamBits();


    // We know what the alignment is, just check how that relates to the player viewing it.

    switch( TeamBits )
    {
    case FRIENDLY_ALPHA:
        VTexture = (PlayerTeamBits & TeamBits) ? FRIEND_TO_TEAM_ALPHA : FRIEND_TO_ENEMY_ALPHA;
        break;

    case FRIENDLY_OMEGA:
        VTexture = (PlayerTeamBits & TeamBits) ? FRIEND_TO_TEAM_OMEGA : FRIEND_TO_ENEMY_OMEGA;
        break;

    default:
        ASSERT( FALSE );
        break;
    }
    #endif
    
    return VTexture;
}

//==============================================================================

void flag::OnRender( void )
{
    m_bRendered = TRUE;

    // Don't render the flag for the attached player.
    if( m_Attached && (m_Player == g_RenderContext.NetPlayerSlot) )
        return;

    // Trigger cloth to be active.
    m_ActiveTimer = 5.0f;

    // Must have geometry.
    if( m_Cloth.GetRigidInst().GetGeom() == NULL )
        return;

    // Compute render flags
    u32 Flags = (GetFlagBits() & object::FLAG_CHECK_PLANES) ? render::CLIPPED : 0;
    Flags |= GetRenderMode();

#ifdef TARGET_XBOX

    // Just render the rigid geometry now - the cloth part will get rendered later so fog works
    m_Cloth.RenderRigidGeometry( Flags );

#else

    // Render rigid geometry and cloth geometry
    m_Cloth.RenderRigidGeometry( Flags );
    m_Cloth.RenderClothGeometry( GetVTexture() );

#endif

#ifdef X_EDITOR
    
    // Draw debug info
    if( GetAttrBits() & ATTR_EDITOR_SELECTED )
    {
        m_Cloth.RenderSkeleton() ;
    }        
    
#endif // X_EDITOR
}

//==============================================================================

#ifdef TARGET_XBOX    

void flag::OnRenderCloth( void )
{
    // Don't render the flag for the attached player.
    if( m_Attached && (m_Player == g_RenderContext.NetPlayerSlot) )
        return;

    m_Cloth.RenderClothGeometry( GetVTexture() );
}

#endif    

//==============================================================================

void flag::OnRenderTransparent( void )
{
    /*
    {
        bbox BBox( vector3(-50,25,-50), vector3(50,200,50) );
        BBox.Translate( GetPosition() );
        draw_BBox( BBox, XCOLOR_WHITE );
        draw_BBox( GetBBox(), XCOLOR_YELLOW );
        draw_BBox( m_Cloth.GetWorldBBox(), XCOLOR_BLUE );
    }
    */

#if !defined( CONFIG_RETAIL )

    // Don't render the flag for the attached player.
    if( m_Attached && (m_Player == g_RenderContext.NetPlayerSlot) )
        return;

    //--------------------------------------------------------------------------
    #ifndef X_EDITOR
    //--------------------------------------------------------------------------

    xcolor Color;

    if( m_NetTeamBits & g_RenderContext.TeamBits )
        Color = XCOLOR_GREEN;
    else
        Color = XCOLOR_RED;

    if( m_TimeOut && ((m_Age / m_TimeOutAge) > 0.75f) )
    {
        Color.A = (u8)(255 - (((m_Age / m_TimeOutAge) - 0.75f) * 1024.0f));
        //                     [    0.75 .. 1.00    ]
        //                    [            0.00 .. 0.25      ]
        //                   [                   0 .. 256                ]
    }

    // Draw away.
    //draw_Marker( GetPosition(), Color );

    //--------------------------------------------------------------------------
    #endif // X_EDITOR
    //--------------------------------------------------------------------------

#endif // !defined( CONFIG_RETAIL )

    for( s32 i = 0; i < NUM_FLAG_EFFECTS; i++ )
    {
        if( m_EffectActive[ i ] ) 
        {
            m_FlagFX[ i ].Render();
        }
    }
}

//==============================================================================

void flag::PlayEffect( flag_effect Effect )
{
#ifndef X_EDITOR
    m_StartEffects[ Effect ] = TRUE;

    switch( Effect )
    {
    case FLAG_TELEPORT:
        // Teleporting will always be done to the base, 
        // so we can assume we want to play the spawn effect too. 
        m_StartEffects[ FLAG_SPAWN ] = TRUE;
        m_NetDirtyBits  |= DIRTY_TELEPORT;
        break;

    case FLAG_TIMEOUT:
        m_NetDirtyBits |= DIRTY_TIMEOUT_IMMINENT;
        break;

    case FLAG_CAPTURE:
        m_NetDirtyBits |= DIRTY_CAPTURE;
        break;

    default:
        ASSERT( FALSE );
        break;
    }

    if( m_Attached ) 
    {
        // If the flag is attached to a player, ignore this effect.
        m_StartEffects[ FLAG_TELEPORT ] = FALSE;
        m_StartEffects[ FLAG_TIMEOUT  ] = FALSE;
    }

    m_VanishPos = GetPosition();
#endif
}

//==============================================================================

void flag::OnAdvanceLogic( f32 DeltaTime )
{
    // Fall logic.
    if( !m_bIsSettled && !m_Attached && m_TimeOut )
    {
        vector3 OldPos = GetPosition();
        vector3 NewPos( OldPos );

        m_Velocity += DeltaTime * 980.0f;

        NewPos.GetY() -= m_Velocity * DeltaTime;

        g_CollisionMgr.CylinderSetup( GetGuid(), OldPos, NewPos, 10.0f, 50.0f );

        g_CollisionMgr.SetMaxCollisions( 10 );
        //g_CollisionMgr.AddToIgnoreList( m_OriginGuid );

        g_CollisionMgr.CheckCollisions( 
            object::TYPE_ALL_TYPES, 
            object::ATTR_BLOCKS_LARGE_PROJECTILES);

        //
        // Backup collisions
        //
        s32 nCollisions = g_CollisionMgr.m_nCollisions;
        ASSERT( nCollisions <= MAX_COLLISION_MGR_COLLISIONS );
        if( nCollisions > MAX_COLLISION_MGR_COLLISIONS ) nCollisions = MAX_COLLISION_MGR_COLLISIONS;

        collision_mgr::collision CollBackup[MAX_COLLISION_MGR_COLLISIONS];
        x_memcpy( CollBackup, g_CollisionMgr.m_Collisions, sizeof( collision_mgr::collision )*nCollisions );

        xbool bImpacted = FALSE;

        // Process the collisions in order.
        for( s32 i = 0; i < nCollisions; i++ )
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
                // Do NOT alter the trajectory as a result of the cloth.  
                // Doing so would screw up the networking.
                continue;                
            }

            // Just go through flag objects.
            if( pObject->IsKindOf( flag::GetRTTI() ) )
            {
                // Do NOT alter the trajectory as a result of the cloth.  
                // Doing so would screw up the networking.
                continue;                
            }

            //
            // Is this a piece of glass
            //
            if( (Coll.Flags == object::MAT_TYPE_GLASS) )
            {
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

            // Hit an object, make sure we don't go through it!
            {
                NewPos = OldPos;
                NewPos.GetY() -= Coll.T * DeltaTime * m_Velocity;
                bImpacted = TRUE;
            }
        }

        SetPosition( NewPos, GetZone1(), GetZone2(), FALSE );

        // Collision?  Then bail out of this loop.
        if( bImpacted || (g_CollisionMgr.m_nCollisions >= 10) )
        {
            StopFall();
        }
    }

#ifndef X_EDITOR
    //
    // Icon Stuff.
    //
    {
        xcolor Red   (  90,  10, 10 );
        xcolor Green ( 0,   127,  0 );
        xcolor Yellow( 127, 127,  0 );

        xbool bRenderFlagOutline = FALSE;

        vector3 FlagPos = GetPosition();

        actor* pAttachActor = NULL;
        if( m_Attached )
        {
            actor* pAttachActor = (actor*)NetObjMgr.GetObjFromSlot( m_Player );
            if( pAttachActor )
            {
                vector3 Temp;
                pAttachActor->GetHeadAndRootPosition( FlagPos, Temp );
                FlagPos.GetY() += 70.0f;
            }
        }
        
        player* pPlayer = SMP_UTIL_GetActivePlayer();
        if( !pPlayer )
            return;

        slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );
        if( SlotID == SLOT_NULL )
            return;

        object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
        if( !pObj )
            return;

        hud_object& Hud = hud_object::GetSafeType( *pObj );
        if( !Hud.m_Initialized )
            return;

        // LoS checks (icon should only be visible when cap point isn't).
        {
            // Cast from player eye to flag, skipping player, and flag collisions
            vector3 FlagCenter = GetBBox().GetCenter();
            g_CollisionMgr.LineOfSightSetup( pPlayer->GetGuid(), pPlayer->GetEyesPosition(), FlagCenter );
            g_CollisionMgr.SetMaxCollisions(1);
            g_CollisionMgr.AddToIgnoreList( GetGuid() ) ; // Skip collision from flag itself
            if( pAttachActor )
            {
                // Skip collision from attached player
                g_CollisionMgr.AddToIgnoreList( pAttachActor->GetGuid() );
            }

            g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                object::ATTR_BLOCKS_PLAYER_LOS,
                (object::object_attr)(object::ATTR_COLLISION_PERMEABLE|object::ATTR_LIVING) );

            xbool bHasLOS = (g_CollisionMgr.m_nCollisions == 0);


            // Clear this so it's ready for the next render pass to set.
            m_bRendered = FALSE;

            // Increment the opacity if you can't see it, otherwise decrement it.
            m_IconOpacity += (2.0f * DeltaTime) * (bHasLOS ? -1.0f : 1.0f);
            m_IconOpacity = MINMAX( 0.0f, m_IconOpacity, 1.0f );
        }

        if( pPlayer )
        {
            u32 TeamBits = pPlayer->net_GetTeamBits();

            xcolor InnerColor = Red;
            xcolor OuterColor = Green;

            if( m_NetTeamBits & TeamBits )
            {
                InnerColor = Green;
                OuterColor = Red;
            }

            if( !m_Attached )
            {
                FlagPos.GetY() += 200.0f;
            }
            
            // Flag
            {    
                // If it is NOT attached to the active player, add an icon for the flag.
                if( !(m_Attached && (m_Player == pPlayer->net_GetSlot())) )
                {
                    Hud.GetPlayerHud( 0 ).m_Icon.AddIcon( ICON_FLAG_INNER, FlagPos, FlagPos, FALSE, TRUE, GUTTER_NONE, InnerColor, NULL, FALSE, FALSE, m_IconOpacity );
                }

                if( m_TimeOut || m_Attached )
                {
                    bRenderFlagOutline = TRUE;

                    if( m_TimeOut )
                    {
                        OuterColor = Yellow;
                    }
                }

                // Outline.
                if( bRenderFlagOutline )
                {
                    if( !(m_Attached && (m_Player == pPlayer->net_GetSlot())) )
                    {
                        Hud.GetPlayerHud( 0 ).m_Icon.AddIcon( ICON_FLAG_OUTER, FlagPos, FlagPos, FALSE, TRUE, GUTTER_NONE, OuterColor, NULL, FALSE, FALSE, m_IconOpacity );
                    }
                }
            }

            // Base.
            {
                xbool bBaseHasLOS = TRUE;

                vector3 BasePos( 0.0f, 0.0f, 0.0f );

                g_ObjMgr.SelectByAttribute( object::ATTR_COLLIDABLE, object::TYPE_FLAG_BASE );
                slot_id aID = g_ObjMgr.StartLoop();
                flag_base* pFlagBase = NULL;
                while( (aID != SLOT_NULL) )
                {
                    object* pObject = g_ObjMgr.GetObjectBySlot(aID);

                    flag_base& FlagBase = flag_base::GetSafeType( *pObject );

                    if( (FlagBase.net_GetTeamBits() & m_NetTeamBits) )
                    {
                        pFlagBase = &FlagBase;
                        BasePos = FlagBase.GetBBox().GetCenter();
                        break;
                    }

                    aID = g_ObjMgr.GetNextResult( aID );
                }
                g_ObjMgr.EndLoop();

                if( bRenderFlagOutline )
                {
                    // Cast from player eye to base, skipping player, flag, and base collisions
                    g_CollisionMgr.LineOfSightSetup( pPlayer->GetGuid(), pPlayer->GetEyesPosition(), BasePos );
                    g_CollisionMgr.SetMaxCollisions(1);
                    g_CollisionMgr.AddToIgnoreList( GetGuid() );
                    if( pFlagBase )
                    {
                        g_CollisionMgr.AddToIgnoreList( pFlagBase->GetGuid() );
                    }                        

                    g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                        object::ATTR_BLOCKS_PLAYER_LOS,
                        (object::object_attr)(object::ATTR_COLLISION_PERMEABLE|object::ATTR_LIVING) );

                    bBaseHasLOS = (g_CollisionMgr.m_nCollisions == 0);
                }

                {
                    m_BaseAlpha += (2.0f * DeltaTime) * (bBaseHasLOS ? -1.0f : 1.0f);
                    m_BaseAlpha  = MINMAX( 0.0f, m_BaseAlpha, 1.0f );                    
                }

                BasePos.GetY() += 200.0f;
                Hud.GetPlayerHud( 0 ).m_Icon.AddIcon( ICON_FLAG_OUTER, BasePos, BasePos, FALSE, TRUE, GUTTER_NONE, InnerColor, NULL, FALSE, FALSE, m_BaseAlpha ); 
            }
        }
    } // Icon stuff.

    // Effect Stuff.
    for( s32 i = 0; i < NUM_FLAG_EFFECTS; i++ )
    {
        if( m_StartEffects[ i ] )
        {
            vector3 EffectPos( GetPosition() );
            switch( i )
            {
            case FLAG_TIMEOUT:
                m_FlagFX[ i ].SetTranslation( EffectPos + vector3(0,150,0) );
                m_FlagFX[ i ].SetScale( vector3(1.5f,1.5f,1.5f) );
                break;
            case FLAG_TELEPORT:
                m_FlagFX[ i ].SetTranslation( m_VanishPos );
                break;
            default:
                m_FlagFX[ i ].SetTranslation( EffectPos );
                break;
            }
                
            m_FlagFX[ i ].Restart();
            m_EffectActive[ i ] = TRUE;
            m_StartEffects[ i ] = FALSE;    
        }

        m_FlagFX[ i ].AdvanceLogic( DeltaTime );
        if( m_FlagFX[ i ].IsFinished() )
        {
            m_EffectActive[ i ] = FALSE;
        }
    }

#endif // X_EDITOR

    // Is cloth active?
    m_ActiveTimer -= DeltaTime;
    if( m_ActiveTimer <= 0.0f )
    {
        // Do not update.
        m_ActiveTimer = 0.0f;
        return;
    }

    // Override cloth properties to make flags look heavier and less stretchy
    m_Cloth.m_Gravity.Set( 0.0f, -9.8f * 100.0f * 1.0f, 0.0f );
    m_Cloth.m_Dampen      = 0.1f;
    m_Cloth.m_nIterations = 3;

    // Selects all objects whose bbox intersects the cloth object.
    g_ObjMgr.SelectBBox( object::ATTR_LIVING, GetBBox(), object::TYPE_ALL_TYPES );
    slot_id SlotID = g_ObjMgr.StartLoop();
    while( SlotID != SLOT_NULL )
    {
        // Lookup object.
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        ASSERT( pObject );

        // Only collide with actors.
        if( pObject->IsKindOf( actor::GetRTTI() ) )
        {
            // Get actor.
            actor& Actor = actor::GetSafeType( *pObject );   

            // Get loco.
            loco* pLoco = Actor.GetLocoPointer();
            if( pLoco )
            {
                // Get physics.
                character_physics& Physics = pLoco->m_Physics;

                // Compute capped collision cylinder.
                vector3 Bottom = Physics.GetPosition();
                vector3 Top    = Bottom;
                f32     Radius = Physics.GetColRadius() * 2.0f;
                Top.GetY() += Physics.GetColHeight();

                // Collide with the cloth.
                m_Cloth.ApplyCappedCylinderColl( Bottom, Top, Radius );
            }
        }

        // Check next object.
        SlotID = g_ObjMgr.GetNextResult( SlotID );
    }     

    g_ObjMgr.EndLoop();

    // Update the cloth simulation.
    m_Cloth.Advance( DeltaTime );

    // Force flag object bounds to update
    SetFlagBits( GetFlagBits() | FLAG_DIRTY_TRANSFORM );
}

//==============================================================================

void flag::OnPain( const pain& Pain )
{
    // Prepare pain (just use settings from coke can since the values are good)
    Pain.ComputeDamageAndForce( "COKE_CAN", GetGuid(), GetBBox().GetCenter() );

    // Let cloth handle it
    m_Cloth.OnPain( Pain );
}

//==============================================================================

void flag::OnProjectileImpact( const object&  Projectile,
                               const vector3& Velocity,
                                     u32      CollPrimKey, 
                               const vector3& CollPoint,
                                     xbool    PunchDamageHole,       
                                     f32      ManualImpactForce )                                                                                                   
{
    // Only 100 holes allowed in a flag.
    if( PunchDamageHole )
    {
        if( m_Holes < 100 )
            m_Holes++;
        else
            PunchDamageHole = FALSE;
    }

    m_Cloth.OnProjectileImpact( Projectile, 
                                Velocity,
                                CollPrimKey, 
                                CollPoint, 
                                PunchDamageHole, 
                                ManualImpactForce );
}

//==============================================================================

u32 flag::GetRenderMode( void )
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

void flag::OnColCheck( void )
{
#ifdef X_EDITOR
    // Allow cloth to be selected in the editor?
    if( g_CollisionMgr.IsEditorSelectRay() )
    {
        // Let the cloth do it's thing...
        m_Cloth.OnColCheck( GetGuid(), GetMaterial() );
        return;
    }
#endif // X_EDITOR

    // Get moving object.
    guid    MovingGuid = g_CollisionMgr.GetMovingObjGuid();
    object* pObject    = g_ObjMgr.GetObjectByGuid(MovingGuid);

    // Collide with bullets, projectiles, or melee?
    if (        ( pObject ) 
            &&  (       ( pObject->IsKindOf( base_projectile::GetRTTI() ) )     // Normal projectiles
                    ||  ( pObject->IsKindOf( net_proj::GetRTTI() ) )            // Net projectiles
                    ||  ( pObject->IsKindOf( actor::GetRTTI() ) ) ) )           // For melee

    {
        // Let the cloth do it's thing...
        m_Cloth.OnColCheck( GetGuid(), GetMaterial() ) ;
    }
}

//==============================================================================

void flag::OnMove( const vector3& NewPos )
{
    // Compute delta movement
    vector3 DeltaPos     = NewPos - GetPosition();
    f32     DeltaDistSqr = DeltaPos.LengthSquared();

    // Call base class.
    object::OnMove( NewPos );

    // Update cloth (reset if moving more than 5 meters).
    m_Cloth.SetL2W( GetL2W(), ( DeltaDistSqr > x_sqr( 500.0f ) ), FLAG_OBJECT_AIR_RESISTANCE );
}

//==============================================================================

void flag::OnTransform( const matrix4& L2W )
{
    // Compute delta movement
    vector3 DeltaPos     = L2W.GetTranslation() - GetPosition();
    f32     DeltaDistSqr = DeltaPos.LengthSquared();

    // Call base class.
    object::OnTransform( L2W );

    // Update cloth (reset if moving more than 5 meters).
    m_Cloth.SetL2W( GetL2W(), ( DeltaDistSqr > x_sqr( 500.0f ) ), FLAG_OBJECT_AIR_RESISTANCE );
}

//==============================================================================
#ifndef X_EDITOR
//==============================================================================

void flag::net_Logic( f32 DeltaTime )
{
    xbool DamageField = FALSE;

    m_Age += DeltaTime;

    // If attached to a player, follow him.
    if( m_Attached )
    {
        // Can only be attached to actors
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( m_Player );
        if( !pActor )
            return;

        // Animation loaded?
        loco* pLoco = pActor->GetLocoPointer();
        if( pLoco && pLoco->m_Player.HasAnimGroup() )
        {
            // Set transform to the flag bone
            matrix4 L2W = pLoco->GetFlagL2W();
            OnTransform( L2W );
            
            // Use actors zone info
            SetZone1( pActor->GetZone1() );
            SetZone2( pActor->GetZone2() );
        }
    }

    if( g_NetworkMgr.IsClient() )
        return;

    // See if there is a player nearby.  Or a damage field!
    if( !m_Attached )
    {
        slot_id Slot = SLOT_NULL;

        bbox BBox( vector3(-50,25,-50), vector3(50,200,50) );
        BBox.Translate( GetPosition() );

        g_ObjMgr.SelectBBox( ATTR_LIVING, BBox );
        Slot = g_ObjMgr.StartLoop();

        while( (Slot != SLOT_NULL) && (!m_Attached) )
        {
            object* pObject = g_ObjMgr.GetObjectBySlot( Slot );
            ASSERT( pObject );
            object::type ObjType = pObject->GetType();
            if( (ObjType == object::TYPE_PLAYER) || 
                (ObjType == object::TYPE_NET_GHOST) )
            {
                netobj* pNetObj = pObject->AsNetObj();
                s32     Index   = pNetObj->net_GetSlot();
                if( !(m_IgnoreBits & (1<<Index)) )
                    pGameLogic->FlagTouched( Index, m_NetSlot );
            }
            Slot = g_ObjMgr.GetNextResult( Slot );
        }
        g_ObjMgr.EndLoop();

        if( !m_Attached && m_TimeOut )
        {   
            g_ObjMgr.SelectBBox( ATTR_ALL, BBox, object::TYPE_DAMAGE_FIELD );
            if( g_ObjMgr.StartLoop() != SLOT_NULL )
            {
                m_Age += DeltaTime * 4.0f;
                DamageField = TRUE;
            }
            g_ObjMgr.EndLoop();
        }
    }

    // Clear ignore bits?
    if( m_IgnoreBits )
    {
        m_IgnoreTimer -= DeltaTime;
        if( m_IgnoreTimer < 0.0f )
        {
            m_IgnoreTimer = 0.0f;
            m_IgnoreBits  = 0x00000000;
        }
    }

    if( ((m_Age + 3.0f) > m_TimeOutAge) && 
        !m_EffectActive[ FLAG_TIMEOUT ] && 
        m_TimeOut &&
        !DamageField ) 
    {
        PlayEffect( FLAG_TIMEOUT );
    }

    // Timeout?
    if( (g_NetworkMgr.IsServer()) && 
        (!m_Attached) &&         
        (m_TimeOut) && 
        (m_Age > m_TimeOutAge) )
    {
        pGameLogic->FlagTimedOut( m_NetSlot );
    }
}

//==============================================================================

void flag::net_AcceptUpdate( const bitstream& BS )
{
    // netobj::ACTIVATE_BIT
    if( BS.ReadFlag() )
    {
        u32 TeamBits;
        BS.ReadU32( TeamBits );
        InitGeometry( TeamBits );
    }

    // TIMEOUT IMMINENT BIT
    if( BS.ReadFlag() )
    {
        PlayEffect( FLAG_TIMEOUT );
    }

    // CAPTURE BIT
    if( BS.ReadFlag() )
    {
        PlayEffect( FLAG_CAPTURE );
    }

    // TELEPORT BIT
    if( BS.ReadFlag() )
    {
        PlayEffect( FLAG_TELEPORT );
    }

    // DIRTY_POSITION
    if( BS.ReadFlag() )
    {
        xbool Attached;
        if( BS.ReadFlag( Attached ) )
        {
            s32 Player;
            BS.ReadS32( Player );
            AttachToPlayer( Player );
        }
        else
        {
            vector3 Position;
            radian  Yaw;
            s32     Zone1;
            s32     Zone2;
            xbool   bFalling;

            BS.ReadVector( Position );
            BS.ReadF32   ( Yaw      );
            BS.ReadS32   ( Zone1    );
            BS.ReadS32   ( Zone2    );

            BS.ReadFlag  ( bFalling  );

            SetYaw( Yaw );
            SetPosition( Position, Zone1, Zone2 );

            // This means it wasn't falling, but now it is.
            if( bFalling && m_bIsSettled )
            {
                StartFall();
            }

            // The server can stop the fall anytime it wants to.
            else if( !bFalling )
            {
                StopFall();
            }
        }
    }

    // DIRTY_TIMEOUT
    if( BS.ReadFlag() )
    {
        if( BS.ReadFlag() )
        {
            f32 TimeOutAge;
            f32 CurrentAge;
            BS.ReadF32( TimeOutAge );
            BS.ReadF32( CurrentAge );
            SetTimeOut( TimeOutAge, CurrentAge );
        }
        else
        {
            ClearTimeOut();
        }
    }
}

//==============================================================================

void flag::net_ProvideUpdate( bitstream& BS, u32& DirtyBits )
{
    if( DirtyBits & netobj::ACTIVATE_BIT )
        DirtyBits |= DIRTY_ALL;

    if( BS.WriteFlag( DirtyBits & netobj::ACTIVATE_BIT ) )
    {
        BS.WriteU32( m_NetTeamBits );
        DirtyBits &= ~netobj::ACTIVATE_BIT;
    }

    if( BS.WriteFlag( DirtyBits & DIRTY_TIMEOUT_IMMINENT ) )
    {
        DirtyBits &= ~DIRTY_TIMEOUT_IMMINENT;
    }

    if( BS.WriteFlag( DirtyBits & DIRTY_CAPTURE ) )
    {
        DirtyBits &= ~DIRTY_CAPTURE;
    }

    if( BS.WriteFlag( DirtyBits & DIRTY_TELEPORT ) )
    {
        DirtyBits &= ~DIRTY_TELEPORT;
    }

    if( BS.WriteFlag( DirtyBits & DIRTY_POSITION ) )
    {
        if( BS.WriteFlag( m_Attached ) )
        {
            BS.WriteS32( m_Player );
        }
        else
        {
            const matrix4& L2W = GetL2W();

            BS.WriteVector( L2W.GetTranslation()  );
            BS.WriteF32   ( L2W.GetRotation().Yaw );
            BS.WriteS32   ( GetZone1()            );
            BS.WriteS32   ( GetZone2()            );
            BS.WriteFlag  ( !m_bIsSettled         );
        }

        DirtyBits &= ~DIRTY_POSITION;
    }

    if( BS.WriteFlag( DirtyBits & DIRTY_TIMEOUT ) )
    {
        if( BS.WriteFlag( m_TimeOut ) )
        {
            BS.WriteF32( m_TimeOutAge );
            BS.WriteF32( m_Age        );
        }

        DirtyBits &= ~DIRTY_TIMEOUT;
    }
}

//==============================================================================
#endif // X_EDITOR
//==============================================================================
