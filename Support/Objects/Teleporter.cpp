//==============================================================================
//
//  Teleporter.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "Teleporter.hpp"
#include "Objects\Player.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

//==============================================================================
//  OBJECT DESCRIPTION
//==============================================================================

static struct teleporter_desc : public object_desc
{
    teleporter_desc( void ) 
        :   object_desc( object::TYPE_TELEPORTER, 
                         "Teleporter",
                         "Multiplayer",
                         object::ATTR_RENDERABLE      |
                         object::ATTR_TRANSPARENT     |
                         object::ATTR_NEEDS_LOGIC_TIME,
                         FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC )
    {
        // Empty function body.
    }

    //--------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        return( new teleporter ); 
    }

    //--------------------------------------------------------------------------

#ifdef X_EDITOR

    void DrawArc( const vector3& Start, const vector3& End, xbool Arc ) const
    {
        vector3 Edge = (End - Start);
        vector3 P0   = Start;
        vector3 P1;
        vector3 Offset;
        xcolor  Color;

        if( Arc )
        {
            vector3 Nudge = Edge;
            Nudge.NormalizeAndScale( 100.0f );

            f32 X = ABS( Edge.GetX() );
            f32 Y = ABS( Edge.GetY() ) * 3.0f;
            f32 Z = ABS( Edge.GetZ() );

            if( (Y > X) && (Y > Z) )
            {
                Offset.Set( 200.0f, 0.0f, 200.0f );
                Nudge.RotateX( R_90 );
            }
            else
            {
                Offset.Set( 0.0f, 500.0f, 0.0f );
                Nudge.RotateY( R_90 );
            }

            Offset += Nudge;
        }

        for( f32 T = 0.0f; T < 1.0f; T += 0.05f )
        {
            Color.Set( (u8)(T * 255), (u8)((1-T) * 255), 0 );

            P1  = Start;
            P1 += Edge * T;

            if( Arc )
                P1 += Offset * x_sin( T * PI );

            draw_Line( P0, P1, Color );
            draw_Point( P1, Color );

            P0 = P1;
        }

        draw_Line( P0, End, Color );

        if( !Arc )
            draw_Marker( End, XCOLOR_YELLOW );
    }

    //--------------------------------------------------------------------------

    virtual s32 OnEditorRender( object& Object ) const
    {
        teleporter& Teleporter = (teleporter&)Object;

        if( Object.GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        {
            bbox BBox = Object.GetLocalBBox();
            BBox.Translate( Object.GetPosition() );
            draw_BBox( BBox );

            vector3 Start = Teleporter.GetPosition();

            for( s32 i = 0; i < 8; i++ )
            {
                if( Teleporter.m_Target[i] != 0 )
                {
                    object* pObject = g_ObjMgr.GetObjectByGuid( Teleporter.m_Target[i] );
                    if( pObject && (pObject->GetType() == object::TYPE_TELEPORTER) )
                    {
                        const teleporter& Target = (teleporter&)(*pObject);
                        vector3 End = Target.GetPosition();
                        DrawArc( Start, End, TRUE );
                    }
                    else
                    {
                        vector3 End = Start;
                        End.GetY() += 1000.0f;
                        DrawArc( Start, End, FALSE );
                    }
                }
            }

            if( Teleporter.m_BoostOut )
            {
                xbool   Toggle   = TRUE;
                f32     Time     = 1.0f;
                vector3 P0       = Teleporter.GetPosition();
                vector3 P1;
                vector3 Velocity( DEG_TO_RAD( -Teleporter.m_BoostPitch ), 
                                  DEG_TO_RAD(  Teleporter.m_BoostYaw   ) );
                Velocity.Scale( Teleporter.m_BoostSpeed );

                while( Time > 0.0f )
                {
                    P1               = P0 + Velocity * (1.0f/30.0f);
                    Time            -= (1.0f/30.0f);
                    Velocity.GetY() -= (1000.0f    * g_MPTweaks.Gravity) * (1.0f/30.0f);
                                    // (Grav=cm/s) * (MP)                * (1/30th second);

                    draw_Line( P0, P1, Toggle ? XCOLOR_YELLOW : XCOLOR_BLUE );
                    draw_Point( P1 );

                    Toggle = !Toggle;
                    P0     = P1;
                }
            }
        }

        EditorIcon_Draw( EDITOR_ICON_MARKER, 
                         Teleporter.GetL2W(), 
                         !!(Teleporter.GetAttrBits() & object::ATTR_EDITOR_SELECTED), 
                         XCOLOR_WHITE );

        return( -1 );
    }

#endif // X_EDITOR

} s_teleporter_Desc;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& teleporter::GetTypeDesc( void ) const
{
    return( s_teleporter_Desc );
}

//==============================================================================

const object_desc& teleporter::GetObjectType( void )
{
    return( s_teleporter_Desc );
}

//==============================================================================

teleporter::teleporter( void )
{
    m_TriggerSize( 100.0f, 100.0f, 100.0f );
    ComputeTrigger();

    for( s32 i = 0; i < 8; i++ )
        m_Target[i] = 0;

    m_BoostPitch  = 0.0f;
    m_BoostYaw    = 0.0f;
    m_BoostSpeed  = 0.0f;

    m_IgnoreBits  = 0x00000000;
    m_Relative    = FALSE;

    m_Audio = 0;
}

//==============================================================================

teleporter::~teleporter( void )
{
    m_FXHandle.KillInstance();
}

//==============================================================================

void teleporter::ComputeTrigger( void )
{
    vector3 Corner = m_TriggerSize * 0.5f;
    m_WorldTrigger.Set( Corner, -Corner );
    m_WorldTrigger.Translate( GetPosition() );
}

//==============================================================================

void teleporter::OnMove( const vector3& NewPos )   
{
    object::OnMove( NewPos );
    ComputeTrigger();
}

//==============================================================================

void teleporter::OnMoveRel( const vector3& DeltaPos )
{
    object::OnMoveRel( DeltaPos );
    ComputeTrigger();
}

//==============================================================================

void teleporter::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );
    ComputeTrigger();
}

//==============================================================================

bbox teleporter::GetLocalBBox( void ) const 
{ 
    bbox BBox;
    vector3 Corner = m_TriggerSize * 0.5f;
    BBox.Set( Corner, -Corner );
    return( BBox );
}

//==============================================================================

void teleporter::OnAdvanceLogic( f32 DeltaTime )
{
    slot_id Loop    = SLOT_NULL;
    object* pObject = NULL;
    player* pPlayer = NULL;
    s32 PlayerIndex = -1;

    m_FXHandle.AdvanceLogic( DeltaTime );
    if( m_FXHandle.IsFinished() )
        m_FXHandle.KillInstance();

    g_ObjMgr.SelectBBox( ATTR_LIVING, m_WorldTrigger, TYPE_PLAYER );
    Loop = g_ObjMgr.StartLoop();

    while( Loop != SLOT_NULL )
    {
        // Get the player touching the trigger box.
        pObject = g_ObjMgr.GetObjectBySlot( Loop );
        ASSERT( pObject );
        ASSERT( pObject->GetType() == TYPE_PLAYER );
        pPlayer = (player*)pObject;

        // Is this player in the ignore list?
        PlayerIndex = pPlayer->net_GetSlot();
        if( m_IgnoreBits & (1<<PlayerIndex) )
            PlayerIndex = -1;

        // Player was not in the ignore list.  Teleport!
        if( PlayerIndex != -1 )
        {
            s32  Count = 0;
            guid List[8];
            for( s32 i = 0; i < 8; i++ )
            {
                if( m_Target[i] != 0 )
                {
                    pObject = g_ObjMgr.GetObjectByGuid( m_Target[i] );
                    if( pObject && (pObject->GetType() == object::TYPE_TELEPORTER) )
                    {
                        // TO DO - Make sure teleporter is in an active zone.
                        List[Count] = m_Target[i];
                        Count++;
                    }
                    else
                    {
                        m_Target[i] = 0;
                    }
                }
            }

            // Pick a teleporter.
            s32 T = x_irand( 0, Count );
            pObject = g_ObjMgr.GetObjectByGuid( List[T] );
            break;
        }

        pObject     = NULL;
        pPlayer     = NULL;
        PlayerIndex = -1;

        Loop = g_ObjMgr.GetNextResult( Loop );
    }
    g_ObjMgr.EndLoop();

    if( pObject && (PlayerIndex != -1) )
    {          
        matrix4 L2W         = GetL2W();
        radian3 Orientation = L2W.GetRotation();
        f32     RelYaw      = pPlayer->GetYaw() - Orientation.Yaw;
        vector3 RelPos      = pPlayer->GetPosition() - GetPosition();
        RelPos.RotateY( -Orientation.Yaw );        

        pPlayer->SetWayPoint( 0, GetPosition() );

        // Send the player to the teleporter.
        teleporter& Teleporter = (teleporter&)(*pObject);
        Teleporter.Receive( PlayerIndex, RelPos, RelYaw );
    }

    // If there are any players in the ignore list, see if they can be taken
    // off that list.
    #ifndef X_EDITOR

    bbox SafeZone = m_WorldTrigger;
    SafeZone.Inflate( 50.0f, 100.0f, 50.0f );

    for( s32 i = 0; i < 32; i++ )
    {
        if( m_IgnoreBits & (1 << i) )
        {
            pObject = NetObjMgr.GetObjFromSlot( i );
            if( pObject )
            {
                if( !SafeZone.Intersect( pObject->GetBBox() ) )
                {
                    m_IgnoreBits &= ~(1<<i);
                }
            }
        }
    }
    #endif
}

//==============================================================================

void teleporter::Receive(       s32      PlayerIndex, 
                          const vector3& RelPos, 
                                radian   RelYaw )
{
    (void)PlayerIndex;

    #ifndef X_EDITOR

    object* pObject = NetObjMgr.GetObjFromSlot( PlayerIndex );
    if( !pObject )
        return;

    if( pObject->GetType() != object::TYPE_PLAYER )
        return;

    player* pPlayer = (player*)pObject;

    radian  NewPitch;
    radian  NewYaw;
    vector3 NewPos;

    matrix4 L2W         = GetL2W();
    radian3 Orientation = L2W.GetRotation();

    if( m_Relative )
    {
        NewPitch = pPlayer->GetPitch();
        NewYaw   = Orientation.Yaw + RelYaw;
        NewPos   = RelPos;

        NewPos.RotateY( Orientation.Yaw );
        NewPos.GetY() += 1.0f;
        NewPos        += GetPosition();
    }
    else
    {
        NewPitch = Orientation.Pitch;
        NewYaw   = Orientation.Yaw;
        NewPos   = GetPosition();
    }

    // Do the actual player teleport.
    pPlayer->SetWayPoint( 1, NewPos );
    pPlayer->Teleport( NewPos, NewPitch, NewYaw, FALSE, TRUE );
    pPlayer->SetZone1( GetZone1() );
    pPlayer->SetZone2( GetZone2() );
    g_ZoneMgr.InitZoneTracking( *pPlayer, (zone_mgr::tracker&)pPlayer->GetZoneTracker() );

    vector3 Boost( DEG_TO_RAD( -m_BoostPitch ), DEG_TO_RAD( m_BoostYaw ) );

    if( m_BoostOut )
    {
        Boost.Scale( m_BoostSpeed );
    }
    else
    {
        Boost.Zero();
    }

    pPlayer->HitJumpPad( Boost, 0.0f, 0.0f, FALSE, FALSE, m_BoostOut, GetGuid() );

    // Add this player to the ignore list.
    m_IgnoreBits |= (1 << PlayerIndex);

    #endif
}

//==============================================================================

void teleporter::OnEnumProp( prop_enum& rPropList )
{
    u32 Flag = m_BoostOut ? 0 : PROP_TYPE_DONT_SHOW;

    object::OnEnumProp( rPropList );

    rPropList.PropEnumHeader ( "Teleporter",                   "What's an online game without teleporters?", 0 );
    rPropList.PropEnumVector3( "Teleporter\\Trigger Size",     "Enter the trigger size (centered about position).", 0 );
    rPropList.PropEnumBool   ( "Teleporter\\Arrive Relative",  "(Set on receiver) Retain player's relative orientation, velocity, and position.", 0 );
    rPropList.PropEnumBool   ( "Teleporter\\Arrive Boost Out", "(Set on receiver) Boost player upon arrival like a jump pad.", PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumFloat  ( "Teleporter\\Boost Pitch",      "Boost pitch.", Flag );
    rPropList.PropEnumFloat  ( "Teleporter\\Boost Yaw",        "Boost yaw.",   Flag );
    rPropList.PropEnumFloat  ( "Teleporter\\Boost Speed",      "Boost velocity (cm/s).", Flag );

    // Pack all of the values down into the lower end of the array.
    for( s32 i = 0; i < 8; i++ )
    {
        if( m_Target[i] == 0 )
        {
            for( s32 j = i+1; j < 8; j++ )
            {
                if( m_Target[j] != 0 )
                {
                    m_Target[i] = m_Target[j];
                    m_Target[j] = 0;
                }
            }
        }
    }

    for( s32 i = 0; i < 8; i++ )
    {
        char Name[64];
        x_sprintf( Name, "Teleporter\\Target%02d", i );

        rPropList.PropEnumGuid( Name, "Select a target teleporter object.", 0 );

        if( m_Target[i] == 0 )
            break;
    } 

    SMP_UTIL_EnumHiddenManualResource( rPropList, "Teleporter\\TeleportOut", SMP_FXO );
    SMP_UTIL_EnumHiddenManualResource( rPropList, "Teleporter\\TeleportIn",  SMP_FXO );
}
    
//==============================================================================

xbool teleporter::OnProperty( prop_query& rPropQuery )
{
    if( object::OnProperty( rPropQuery ) )
    {
        return( TRUE );
    }

    if( rPropQuery.VarVector3( "Teleporter\\Trigger Size", m_TriggerSize ) )
    {
        if( !rPropQuery.IsRead() )  ComputeTrigger();
        return( TRUE );
    }

    if( rPropQuery.VarBool( "Teleporter\\Arrive Relative",  m_Relative ) )  return( TRUE );
    if( rPropQuery.VarBool( "Teleporter\\Arrive Boost Out", m_BoostOut ) )  return( TRUE );

    if( rPropQuery.VarFloat( "Teleporter\\Boost Pitch", m_BoostPitch, -90,  +90 ) )  return( TRUE );
    if( rPropQuery.VarFloat( "Teleporter\\Boost Yaw",   m_BoostYaw,  -360, +360 ) )  return( TRUE );
    if( rPropQuery.VarFloat( "Teleporter\\Boost Speed", m_BoostSpeed            ) )  return( TRUE );

    for( s32 i = 0; i < 8; i++ )
    {
        char Name[64];
        x_sprintf( Name, "Teleporter\\Target%02d", i );

        if( rPropQuery.VarGUID( Name, m_Target[i] ) )
        {
            return( TRUE );
        }
    }

    if( SMP_UTIL_IsHiddenManualResource( rPropQuery, "Teleporter\\TeleportOut", "MP_Teleport_Out.fxo" ) )
        return( TRUE );

    if( SMP_UTIL_IsHiddenManualResource( rPropQuery, "Teleporter\\TeleportIn", "MP_Teleport_In.fxo" ) )
        return( TRUE );

    return( FALSE );
}

//==============================================================================

void teleporter::PlayTeleportIn( void )
{
    rhandle<char> Resource;
    Resource.SetName( "MP_Teleport_In.fxo" );

    char* pPtr = Resource.GetPointer();
    ASSERT( pPtr );
    if( !pPtr )
        return;

    m_FXHandle.InitInstance( pPtr );
    m_FXHandle.SetTransform( GetL2W() );

    g_AudioMgr.Play( "teleport_in", GetPosition(), GetZone1(), TRUE );
}

//==============================================================================

void teleporter::PlayTeleportOut( void )
{
    rhandle<char> Resource;
    Resource.SetName( "MP_Teleport_Out.fxo" );

    char* pPtr = Resource.GetPointer();
    ASSERT( pPtr );
    if( !pPtr )
        return;

    m_FXHandle.InitInstance( pPtr );
    m_FXHandle.SetTransform( GetL2W() );

    g_AudioMgr.Play( "teleport_out", GetPosition(), GetZone1(), TRUE );
}

//==============================================================================

void teleporter::OnRender( void )
{
}

//==============================================================================

void teleporter::OnRenderTransparent( void )
{
    m_FXHandle.Render();
}

//==============================================================================
