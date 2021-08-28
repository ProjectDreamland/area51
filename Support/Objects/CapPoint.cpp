//==============================================================================
//
//  CapPoint.cpp
//
//==============================================================================
/*
    TO DO:

    - Render real graphics.  Maybe use a game prop...?
    - Proper bounding box.
    - Collision.  Player should not be able to walk through this.
    - If within a locked zone, go neutral immediately.
    - HUD icons.  (Unless in a locked zone.)

*/
//==============================================================================
//  INCLUDES
//==============================================================================

#include "CapPoint.hpp"
#include "GameLib/RenderContext.hpp"
#include "TemplateMgr/TemplateMgr.hpp"
#include "Entropy/e_Draw.hpp"
#include "Objects/Actor/Actor.hpp"
#include "Objects\HudObject.hpp"

#ifndef X_EDITOR
#include "NetworkMgr/NetworkMgr.hpp"
#include "NetworkMgr/logic_Base.hpp"
#endif  

//==============================================================================
//  DEFINITIONS
//==============================================================================

#define COLUMN_OPACITY  127

//==============================================================================
//  OBJECT DESCRIPTION
//==============================================================================

static struct cap_point_desc : public object_desc
{
    cap_point_desc( void ) 
        :   object_desc( object::TYPE_CAP_POINT, 
                         "CapPoint",
                         "Multiplayer",
                         object::ATTR_NEEDS_LOGIC_TIME     |
                         object::ATTR_COLLIDABLE           |
                         object::ATTR_BLOCKS_ALL_PROJECTILES |
                         object::ATTR_BLOCKS_ALL_ACTORS    |
                         object::ATTR_BLOCKS_RAGDOLL       |
                         object::ATTR_BLOCKS_SMALL_DEBRIS  |
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
        return( new cap_point ); 
    }

} s_cap_point_Desc;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& cap_point::GetTypeDesc( void ) const
{
    return( s_cap_point_Desc );
}

//==============================================================================

const object_desc& cap_point::GetObjectType( void )
{
    return( s_cap_point_Desc );
}

//==============================================================================

cap_point::cap_point( void )
{
    m_Circuit     = 0;
    m_Award       = 0;
    m_Mode        = 0;
    m_Elevation   = 200.0f;
    m_Radius      = 750.0f;
    m_Value       =   0.0f;
    m_AwardTimer  =   0.0f;
    m_AlphaMask   = 0x00000000;
    m_OmegaMask   = 0x00000000;
    m_Alpha       = 0.0f;
    m_IconOpacity = 0.0f;
    m_bRendered   = FALSE;

    m_bInitialized = FALSE;

    for( s32 i = 0; i < 32; i++ )
    {
        m_bArcs[ i ] = FALSE;
    }

    #ifndef X_EDITOR
    m_NetTeamBits = 0x00000000;
    #endif
}

//==============================================================================

void cap_point::Init( void )
{
    if( !m_bInitialized )
    {
        // Base.
        {
            m_BaseGuid = g_ObjMgr.CreateObject( team_prop::GetObjectType() );
            object* pBase = g_ObjMgr.GetObjectByGuid( m_BaseGuid );

            if( pBase )
            {
                ((team_prop*)pBase)->SetGeom( "MP_Cap_Point_Base001.rigidgeom" );
                ((team_prop*)pBase)->SetCircuit( m_Circuit );
                ((team_prop*)pBase)->SetL2W( GetL2W() );
                ((team_prop*)pBase)->OnMove( GetPosition() );
            }
        }
        
        // Core.
        {
            m_BallCore = g_ObjMgr.CreateObject( team_prop::GetObjectType() );
            object* pCore = g_ObjMgr.GetObjectByGuid( m_BallCore );

            if( pCore )
            {
                ((team_prop*)pCore)->SetGeom( "MP_Cap_Point_001.rigidgeom" ); 
                ((team_prop*)pCore)->SetCircuit( m_Circuit );
                ((team_prop*)pCore)->OnMove( m_Center );
            }
        }
      
        // Anim.
        {
            m_BallAnim = g_TemplateMgr.CreateSingleTemplate( "C:\\GameData\\A51\\Source\\Themes\\Multiplayer\\Assets\\Blueprint\\MP_Cap_Point_000.bpx", 
                                                    m_Center, radian3(R_0,R_0,R_0), 0, 0 );
        }

        // Column.
        {
            rhandle<char> Resource;
            Resource.SetName( "MP_CapturePtBeam.fxo" );

            m_Top.InitInstance( Resource.GetPointer() );
            m_Top.Restart();

            m_Top.SetTranslation( GetPosition() );
            xcolor Color;
            Color = XCOLOR_GREEN;
            Color.A = COLUMN_OPACITY;
            m_Top.SetColor( Color );

            m_Bottom.InitInstance( Resource.GetPointer() );
            m_Bottom.Restart();

            m_Bottom.SetTranslation( GetPosition() );

            Color = XCOLOR_YELLOW;
            Color.A = COLUMN_OPACITY;
            m_Bottom.SetColor( Color );

            Resource.Destroy();
        }

        // Sound effect.
        {
            m_Hum = g_AudioMgr.Play( "CNH_CAPTURE_POINT_HUM", m_Center, GetZone1(), TRUE, FALSE );
        }
    }

    m_bInitialized = TRUE;
}

//==============================================================================

cap_point::~cap_point( void )
{
    // Dispose of geometry and effects.
    g_ObjMgr.DestroyObject( m_BaseGuid );
    g_ObjMgr.DestroyObject( m_BallAnim );
    g_ObjMgr.DestroyObject( m_BallCore );
    g_AudioMgr.Pause( m_Hum );

    m_Top.KillInstance();
    m_Bottom.KillInstance();
    for( s32 i = 0; i < 32; i++ )
    {
        m_Arcs[ i ].KillInstance();
    }    
}

//==============================================================================

bbox cap_point::GetLocalBBox( void ) const 
{ 
    bbox BBox( vector3( 0.0f, m_Elevation, 0.0f ), 
               x_max( m_Radius + 100.0f, m_Elevation + 100.0f ) );
    return( BBox );
}

//==============================================================================

void cap_point::SetCircuit( s32 Circuit )
{
    m_Circuit = Circuit;
    #ifndef X_EDITOR
    m_NetTeamBits = GameMgr.GetCircuitBits( m_Circuit );
    switch( m_NetTeamBits )
    {
    case 0x00000001:
        m_Mode  = -1;
        m_Value = -1.0f;
        break;
    case 0x00000002:
        m_Mode  = +1;
        m_Value = +1.0f;
        break;
    default:
        m_Mode  =  0;
        m_Value =  0.0f;
        break;
    }
    #endif
}

//==============================================================================

void cap_point::SetL2W( const matrix4& L2W )
{
    SetTransform( L2W );
    m_Center.Set( 0, m_Elevation, 0 );
    m_Center = L2W.Transform( m_Center );
}

//==============================================================================

void cap_point::OnRender( void )
{
    // If we were using normal geometry, we would render using the following
    // code.  Since we have switch to a cloth based system, this code is now
    // defunct.  I'll leave it in, though, for reference.
    //
    //  if( m_Inst.GetInst() != HNULL )
    //  {
    //      m_Inst.Render( &GetL2W(), Flags | render::NORMAL );
    //  }
    //  else
    //  {
    //      #ifdef X_EDITOR
    //      draw_BBox( GetBBox() );
    //      #endif
    //  }

    //--------------------------------------------------------------------------
    #ifdef X_EDITOR
    //--------------------------------------------------------------------------

    //--------------------------------------------------------------------------
    #endif // X_EDITOR
    //--------------------------------------------------------------------------
}

//==============================================================================

void cap_point::OnRenderTransparent( void )
{
    //--------------------------------------------------------------------------
    #ifndef X_EDITOR
    //--------------------------------------------------------------------------

    xcolor Color;
    m_bRendered = TRUE;

    if( (m_NetTeamBits == 0x00000000) || (m_NetTeamBits == 0xFFFFFFFF) )
    {
        Color = XCOLOR_YELLOW;
    }
    else
    {
        if( m_NetTeamBits & g_RenderContext.TeamBits )     
        {
            Color = XCOLOR_GREEN;
        }
        else
        {
            Color = XCOLOR_RED;
        }
    }

    // Column.
    {
        f32 TopPercentage = x_abs( m_Value );
        xcolor TopColor = XCOLOR_GREEN;     

        if( ((g_RenderContext.TeamBits == 0x00000001) && (m_Value > 0.0f)) ||
            ((g_RenderContext.TeamBits == 0x00000002) && (m_Value < 0.0f)) )
        {
            TopColor = XCOLOR_RED; 
        }

        TopColor.A = COLUMN_OPACITY;

        ASSERT( IN_RANGE( 0.0f, TopPercentage, 1.0f ) );

        // Top Portion.
        if( TopPercentage > 0.0f )
        {
            matrix4 L2W = GetL2W();
            L2W.PreTranslate( vector3( 0.0f, (1.0f - TopPercentage) * m_Elevation, 0.0f ) );
            L2W.PreScale( vector3( 1.0f, TopPercentage * (m_Elevation / 100.0f), 1.0f ) );
            m_Top.SetColor( TopColor );
            m_Top.SetTransform( L2W );
            m_Top.Render();
        }

        // Bottom Portion (yellow).
        if( TopPercentage < 1.0f )
        {
            matrix4 L2W = GetL2W();
            L2W.PreScale( vector3( 1.0f, (1.0f - TopPercentage) * (m_Elevation / 100.0f), 1.0f ) );
            m_Bottom.SetTransform( L2W );
            m_Bottom.Render();
        }
    }

    // Render "arcs".
    for( s32 i = 0; i < 32; i++ )
    {
        if( m_bArcs[ i ] )
        {
            xcolor Color = XCOLOR_WHITE;
            
            u32 GMask, RMask;

            if( g_RenderContext.TeamBits == 0x00000001 )
            {
                GMask = m_AlphaMask;
                RMask = m_OmegaMask;
            }
            else
            {
                GMask = m_OmegaMask;
                RMask = m_AlphaMask;
            }

            if( GMask & (1 << i) )
            {
                Color = XCOLOR_GREEN;
                Color.A = 127;
                m_Arcs[ i ].SetColor( Color );
            }
            else if( RMask & (1 << i) )
            {
                Color = XCOLOR_RED;
                Color.A = 127;
                m_Arcs[ i ].SetColor( Color );
            }
            
            m_Arcs[ i ].Render();
        }
    }
            
    //--------------------------------------------------------------------------
    #endif // X_EDITOR
    //--------------------------------------------------------------------------
}

//==============================================================================

void cap_point::OnAdvanceLogic( f32 DeltaTime )
{
    //--------------------------------------------------------------------------
    #ifndef X_EDITOR
    //--------------------------------------------------------------------------

    m_NetTeamBits = GameMgr.GetCircuitBits( m_Circuit );

    s32 Count = 0;

    m_AlphaMask = 0x00000000;
    m_OmegaMask = 0x00000000;

    xbool bContesting = FALSE;

    // Scan for players in the area.
    for( s32 i = 0; i < 32; i++ )
    {
        actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
        if( !pActor )
            continue;
        if( pActor->IsDead() )
            continue;
        vector3 Position = pActor->GetPosition() + vector3(0,100,0);
        f32 Distance = (Position - m_Center).Length();
        if( Distance > m_Radius )
            continue;
        if( g_ObjMgr.HasLOS( GetGuid(), m_Center, pActor->GetGuid(), Position ) )
        {
            bContesting = TRUE;
            
            if( pActor->net_GetTeamBits() == 0x00000001 )
            {
                Count -= 1;
                m_AlphaMask |= (1 << i);
            }
            if( pActor->net_GetTeamBits() == 0x00000002 )
            {
                Count += 1;
                m_OmegaMask |= (1 << i);
            }
        }
    }

    xbool bInActiveZone = !GameMgr.IsZoneLocked( GetZone1() );

    // If the point is in a dead zone and it belongs to one team, apply a 
    // counter influence.  Intent is to neutralize points in dead zones, but not
    // to do so instantly.
    if( m_Mode && !bInActiveZone)
    {
        Count -= m_Mode;
    }

    //
    // Icon Stuff.
    //
    {
        // LoS checks (icon should only be visible when cap point isn't).
        {
            player* pPlayer = SMP_UTIL_GetActivePlayer();
            if( !pPlayer )
                return;

            xbool bHasLOS = !bInActiveZone;
         
            // If it wasn't rendered last frame, we can assume there is no LoS.
            if( m_bRendered && bInActiveZone )
            {
                g_CollisionMgr.LineOfSightSetup( pPlayer->GetGuid(), m_Center, pPlayer->GetEyesPosition() );

                // Only need one collision to block LOS.
                g_CollisionMgr.SetMaxCollisions(1);

                g_CollisionMgr.CheckCollisions( object::TYPE_ALL_TYPES, 
                    object::ATTR_BLOCKS_PLAYER_LOS,
                    (object::object_attr)(object::ATTR_COLLISION_PERMEABLE|object::ATTR_LIVING) );

                bHasLOS = (g_CollisionMgr.m_nCollisions == 0);
            }

            // Clear this so it's ready for the next render pass to set.
            m_bRendered = FALSE;

            // Increment the opacity if you can't see it, otherwise decrement it.
            m_IconOpacity += (2.0f * DeltaTime) * (bHasLOS ? -1.0f : 1.0f);
            m_IconOpacity = MINMAX( 0.0f, m_IconOpacity, 1.0f );
        }

        // Step the alpha.
        m_Alpha += 2.5f * DeltaTime;

        if( m_Alpha > (10.0f * PI) )
            m_Alpha -= (10.0f * PI);

        slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

        if( SlotID != SLOT_NULL )
        {
            player* pActor = SMP_UTIL_GetActivePlayer();
            if( pActor )
            {
                u32 TeamBits = pActor->net_GetTeamBits();

                object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
                hud_object& Hud = hud_object::GetSafeType( *pObj );

                if( Hud.m_Initialized )
                {
                    xcolor Red   ( 127,   0, 0 );
                    xcolor Green (   0, 127, 0 );
                    xcolor Yellow( 127, 127, 0 );

                    xcolor Color2 = Yellow;

                    if( ((m_Mode == -1) && (TeamBits & 0x00000001)) ||
                        ((m_Mode ==  1) && (TeamBits & 0x00000002)) )
                    {
                        Color2 = Green;
                    }
                    else if( m_Mode != 0 )
                    {
                        Color2 = Red;
                    }

                    Hud.GetPlayerHud( 0 ).m_Icon.AddIcon( ICON_CNH_OUTER, 
                                                          m_Center, m_Center, 
                                                          FALSE, FALSE, 
                                                          GUTTER_NONE, Color2, 
                                                          NULL, FALSE, FALSE, 
                                                          m_IconOpacity );
                    
                    if( bContesting )
                    {
                        xcolor Color1 = Yellow;

                        if( Count != 0 )
                        {
                            Color1 = Green;
                        }

                        if( ((TeamBits & 0x00000001) && (Count > 0)) ||
                            ((TeamBits & 0x00000002) && (Count < 0)) )
                        {
                            Color1 = Red;
                        }

                        Hud.GetPlayerHud( 0 ).m_Icon.AddIcon( ICON_CNH_INNER, 
                                m_Center, 
                                m_Center, 
                                FALSE, 
                                FALSE, 
                                GUTTER_NONE, 
                                Color1, 
                                NULL, 
                                FALSE, 
                                FALSE, 
                                0.25f * MINMAX( 2.01f, 3.0f + x_sin( m_Alpha ), 3.99f ) * m_IconOpacity );
                    }
                }
            }
        }
    }

    // Make value contributions from the local players.
    m_Value += (Count * DeltaTime * 0.20f);

    // Resist change.
    if( m_Mode )
    {
        // If one team already owns the node, give them a little free recovery.
        m_Value += (m_Mode * DeltaTime * 0.05f);
    }
    else if( Count == 0 )
    {
        // If the node is neutral and there are no players about, gravitate
        // towards a 0 value.
        if( m_Value > 0.0f )
        {
            m_Value -= (DeltaTime * 0.10f);
            m_Value  = MAX( 0.0f, m_Value );
        }
        if( m_Value < 0.0f )
        {
            m_Value += (DeltaTime * 0.10f);
            m_Value  = MIN( 0.0f, m_Value );
        }
    }

    // Clip the value.
    m_Value = MINMAX( -1.0f, m_Value, +1.0f );

    // Effects.
    m_Top.AdvanceLogic( DeltaTime );
    m_Bottom.AdvanceLogic( DeltaTime );

    // Now do the per-player check.
    for( s32 i = 0; i < 32; i++ )
    {
        // Check to see if either alpha or omega arcs are being drawn.
        if( (m_AlphaMask & (1 << i)) | (m_OmegaMask & (1 << i)) )
        {
            if( !m_bArcs[ i ] )
            {
                rhandle<char> Resource;
                Resource.SetName( "MP_ElecTeather_000.fxo" );

                m_Arcs[ i ].InitInstance( Resource.GetPointer() );
                m_Arcs[ i ].Restart();

                Resource.Destroy();

                m_bArcs[ i ] = TRUE;
            }
            
            m_Arcs[ i ].SetSuspended( FALSE );

            actor* pActor = (actor*)NetObjMgr.GetObjFromSlot( i );
            if( pActor )
            {
                vector3 Point = pActor->GetPosition();
                Point += vector3(0,100,0);

                {
                    vector3 Axis = (m_Center - Point);
                    radian  Pitch;
                    radian  Yaw;

                    Axis.GetPitchYaw( Pitch, Yaw );

                    matrix4 L2W( vector3( 1.0f, 1.0f, (Axis.Length() / 500.0f) ), 
                                 radian3( Pitch, Yaw, R_0 ), 
                                 m_Center );

                    m_Arcs[ i ].SetTransform( L2W );
                }
            }
        }
        else
        {
            m_Arcs[ i ].SetSuspended( TRUE );
        }

        if( m_bArcs[ i ] )
        {
            m_Arcs[ i ].AdvanceLogic( DeltaTime );
            if( m_Arcs[ i ].IsFinished() )
            {
                m_Arcs[ i ].KillInstance();
                m_bArcs[ i ] = FALSE;
            }
        }
    }

    // End of the line for clients.
    if( g_NetworkMgr.IsClient() )
        return;

    //
    // Check for mode change.
    //

    if( (m_Value <= -1.0f) && (m_Mode == 0) )
    {
        // Conversion to Alpha!
        m_Mode = -1;
        pGameLogic->CircuitChanged( m_Circuit, 0x00000001 );
        m_NetDirtyBits |= DIRTY_MODE;
    }

    if( (m_Value >= 1.0f) && (m_Mode == 0) )
    {
        // Conversion to Omega!
        m_Mode = 1;
        pGameLogic->CircuitChanged( m_Circuit, 0x00000002 );
        m_NetDirtyBits |= DIRTY_MODE;
    }

    if( (m_Value >= 0.0f) && (m_Mode == -1) )
    {
        // Alpha lost the node!
        m_Mode  = 0;
        m_Value = 0.0f;
        pGameLogic->CircuitChanged( m_Circuit, 0x00000000 );
        m_NetDirtyBits |= DIRTY_MODE;
    }

    if( (m_Value <= 0.0f) && (m_Mode == 1) )
    {
        // Omega lost the node!
        m_Mode  = 0;
        m_Value = 0.0f;
        pGameLogic->CircuitChanged( m_Circuit, 0x00000000 );
        m_NetDirtyBits |= DIRTY_MODE;
    }

    // And now let's take care of scoring.
    if( m_Mode )
    {
        m_AwardTimer -= (DeltaTime * ABS(m_Value));
        if( m_AwardTimer <= 0.0f )
        {
            // Give the lucky team a point!
            m_Award      += m_Mode;
            m_AwardTimer += 1.0f;
        }
    }
    else
    {
        m_AwardTimer = 0.0f;
    }

    //--------------------------------------------------------------------------
    #endif
    //--------------------------------------------------------------------------
}

//==============================================================================

s32 cap_point::GetAward( void )
{
    s32 Award = m_Award;
    m_Award = 0;
    return( Award );
}

//==============================================================================

xbool cap_point::PlayerInfluence( s32 PlayerIndex )
{
    if( (1 << PlayerIndex) & (m_AlphaMask | m_OmegaMask) )
        return( TRUE );
    else
        return( FALSE );
}

//==============================================================================

s32 cap_point::GetCircuit( void )
{
    return( m_Circuit );
}

//==============================================================================
#ifndef X_EDITOR
//==============================================================================

void cap_point::net_AcceptUpdate( const bitstream& BS )
{
    // netobj::ACTIVATE_BIT
    if( BS.ReadFlag() )
    {
        vector3 Position;
        radian3 Orientation;
        s32     Zone1;
        s32     Zone2;

        BS.ReadVector( Position );
        BS.ReadF32( m_Elevation );
        BS.ReadF32( m_Radius    );
        BS.ReadF32( Orientation.Pitch );
        BS.ReadF32( Orientation.Yaw   );
        BS.ReadF32( Orientation.Roll  );
        BS.ReadRangedS32( Zone1, 0, 255 );
        BS.ReadRangedS32( Zone2, 0, 255 );
        BS.ReadRangedS32( m_Circuit, 0, 15 );

        matrix4 L2W;
        L2W.Identity();
        L2W.SetRotation( Orientation );
        L2W.SetTranslation( Position );

        SetL2W( L2W );
        SetZone1( Zone1 );
        SetZone2( Zone2 );
        
        Init();
    }

    // DIRTY_MODE
    if( BS.ReadFlag() )
    {
        BS.ReadRangedS32( m_Mode, -1, +1 );
        BS.ReadRangedF32( m_Value, 8, -1.0f, +1.0f );
    }
}

//==============================================================================

void cap_point::net_ProvideUpdate( bitstream& BS, u32& DirtyBits )
{
    if( DirtyBits & netobj::ACTIVATE_BIT )
        DirtyBits |= DIRTY_ALL;

    if( BS.WriteFlag( DirtyBits & netobj::ACTIVATE_BIT ) )
    {
        vector3 Position    = GetL2W().GetTranslation();
        radian3 Orientation = GetL2W().GetRotation();
        BS.WriteVector( Position );
        BS.WriteF32( m_Elevation );
        BS.WriteF32( m_Radius    );
        BS.WriteF32( Orientation.Pitch );
        BS.WriteF32( Orientation.Yaw   );
        BS.WriteF32( Orientation.Roll  );
        BS.WriteRangedS32( GetZone1(), 0, 255 );
        BS.WriteRangedS32( GetZone2(), 0, 255 );
        BS.WriteRangedS32( m_Circuit, 0, 15 );
        
        DirtyBits &= ~netobj::ACTIVATE_BIT;
    }

    if( BS.WriteFlag( DirtyBits & DIRTY_MODE ) )
    {
        BS.WriteRangedS32( m_Mode, -1, +1 );
        BS.WriteRangedF32( m_Value, 8, -1.0f, +1.0f );
        
        DirtyBits &= ~DIRTY_MODE;
    }
}

//==============================================================================
#endif // X_EDITOR
//==============================================================================
