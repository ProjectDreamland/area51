//==============================================================================
//
//  JumpPad.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "JumpPad.hpp"
#include "Objects\Player.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "AudioMgr\AudioMgr.hpp"

//==============================================================================
//  OBJECT DESCRIPTION
//==============================================================================

static struct jump_pad_desc : public object_desc
{
    jump_pad_desc( void ) 
        :   object_desc( object::TYPE_JUMP_PAD, 
                         "Jump Pad",
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
        return( new jump_pad ); 
    }

    //--------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32 OnEditorRender( object& Object ) const
    {
        if( Object.GetAttrBits() & object::ATTR_EDITOR_SELECTED )
        {
            bbox BBox = Object.GetLocalBBox();
            BBox.Translate( Object.GetPosition() );
            draw_BBox( BBox );

            jump_pad& JumpPad = (jump_pad&)Object;

            xbool   Toggle   = TRUE;
            vector3 P0       = JumpPad.GetPosition();
            vector3 P1;
            vector3 Velocity = JumpPad.m_Velocity;
            f32     Time     = JumpPad.m_ArcTime;
            if( Time > 10.0f )  Time = 10.0f;
            if( Time <  0.0f )  Time =  1.0f;

            while( Time > 0.0f )
            {
                P1               = P0 + Velocity * (1.0f/30.0f);
                Time            -= (1.0f/30.0f);
                Velocity.GetY() -= (1000.0f    * g_MPTweaks.Gravity) * (1.0f/30.0f);
                                // (Grav=cm/s) * (MP)                * (1/30th second);

                draw_Line( P0, P1, Toggle ? XCOLOR_YELLOW : XCOLOR_RED );
                draw_Point( P1 );

                Toggle = !Toggle;
                P0     = P1;
            }
        }

        return( EDITOR_ICON_JUMP_PAD );
    }

#endif // X_EDITOR

} s_jump_pad_Desc;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& jump_pad::GetTypeDesc( void ) const
{
    return( s_jump_pad_Desc );
}

//==============================================================================

const object_desc& jump_pad::GetObjectType( void )
{
    return( s_jump_pad_Desc );
}

//==============================================================================

jump_pad::jump_pad( void )
{
    m_TriggerSize( 100.0f, 100.0f, 100.0f );
    m_Pitch         = 45;
    m_Yaw           =  0;
    m_Speed         = 1000.0f;
    m_ArcTime       =    1.0f;
    m_AirControl    =    0.0f;
    m_BoostOnly     = FALSE;
    m_ReboostOnly   = FALSE;
    m_Instantaneous = TRUE;

    ComputeTrigger();
    ComputeVelocity();
}

//==============================================================================

jump_pad::~jump_pad( void )
{
    m_FXHandle.KillInstance();
}

//==============================================================================

void jump_pad::ComputeTrigger( void )
{
    vector3 Corner = m_TriggerSize * 0.5f;
    m_WorldTrigger.Set( Corner, -Corner );
    m_WorldTrigger.Translate( GetPosition() );
}

//==============================================================================

void jump_pad::ComputeVelocity( void )
{
    m_Velocity.Set( DEG_TO_RAD( -m_Pitch ), DEG_TO_RAD( m_Yaw ) );
    m_Velocity.Scale( m_Speed );
}

//==============================================================================

void jump_pad::OnMove( const vector3& NewPos )   
{
    object::OnMove( NewPos );
    ComputeTrigger();
}

//==============================================================================

void jump_pad::OnMoveRel( const vector3& DeltaPos )
{
    object::OnMoveRel( DeltaPos );
    ComputeTrigger();
}

//==============================================================================

void jump_pad::OnTransform( const matrix4& L2W )
{
    object::OnTransform( L2W );
    ComputeTrigger();
}

//==============================================================================

bbox jump_pad::GetLocalBBox( void ) const 
{ 
    bbox BBox;
    vector3 Corner = m_TriggerSize * 0.5f;
    BBox.Set( Corner, -Corner );
    return( BBox );
}

//==============================================================================

void jump_pad::OnAdvanceLogic( f32 DeltaTime )
{
    m_FXHandle.AdvanceLogic( DeltaTime );
    if( m_FXHandle.IsFinished() )
        m_FXHandle.KillInstance();

    slot_id Slot = SLOT_NULL;
    g_ObjMgr.SelectBBox( ATTR_LIVING, m_WorldTrigger, TYPE_PLAYER );
    Slot = g_ObjMgr.StartLoop();
    while( Slot != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( Slot );
        ASSERT( pObject );
        ASSERT( pObject->GetType() == TYPE_PLAYER );
        player* pPlayer = (player*)pObject;

        if( m_Instantaneous && !m_ReboostOnly )
        {
            pPlayer->SetWayPoint( 0, GetPosition() );
        }

        pPlayer->HitJumpPad( m_Velocity, 
                             DeltaTime, 
                             m_AirControl*0.01f, 
                             m_BoostOnly,
                             m_ReboostOnly, 
                             m_Instantaneous,
                             GetGuid() );

        Slot = g_ObjMgr.GetNextResult( Slot );
    }
    g_ObjMgr.EndLoop();
}

//==============================================================================

void jump_pad::OnEnumProp( prop_enum& rPropList )
{
    object::OnEnumProp( rPropList );
    rPropList.PropEnumHeader ( "Jump Pad",                  "What's an online game without jump pads?",             0 );
    rPropList.PropEnumVector3( "Jump Pad\\Trigger Size",    "Enter the trigger size (centered about position).",    0 );
    rPropList.PropEnumFloat  ( "Jump Pad\\Boost Pitch",     "Kick pitch.",                                          PROP_TYPE_DONT_EXPORT );
    rPropList.PropEnumFloat  ( "Jump Pad\\Boost Yaw",       "Kick yaw.",                                            PROP_TYPE_DONT_EXPORT );
    rPropList.PropEnumFloat  ( "Jump Pad\\Boost Speed",     "Kick velocity (cm/s).",                                PROP_TYPE_DONT_EXPORT );
    rPropList.PropEnumFloat  ( "Jump Pad\\Editor Arc Time", "Time in seconds to show arc path.  Editor only.",      PROP_TYPE_DONT_EXPORT );
    rPropList.PropEnumFloat  ( "Jump Pad\\Air Control",     "Percentage (0-100) 'air control' allowed player.",     0 );
    rPropList.PropEnumBool   ( "Jump Pad\\Boost Only",      "Only boost player if NOT airborn from another boost?", 0 );
    rPropList.PropEnumBool   ( "Jump Pad\\Reboost Only",    "Only boost player if airborn from another boost?",     0 );
    rPropList.PropEnumBool   ( "Jump Pad\\Instantaneous",   "Apply velocity instantaneously (jump pad) or not (gravity field)?", 0 );
    rPropList.PropEnumVector3( "Jump Pad\\Boost Vector",    "Vector of boost, includes pitch, yaw, velocity.",      PROP_TYPE_DONT_SHOW );
}
    
//==============================================================================

xbool jump_pad::OnProperty( prop_query& rPropQuery )
{
    if( object::OnProperty( rPropQuery ) )
    {
        return( TRUE );
    }

    if( rPropQuery.VarVector3( "Jump Pad\\Trigger Size", m_TriggerSize ) )
    {
        if( !rPropQuery.IsRead() )  ComputeTrigger();
        return( TRUE );
    }

    if( rPropQuery.VarFloat( "Jump Pad\\Boost Pitch", m_Pitch, -90, +90 ) )
    {
        if( !rPropQuery.IsRead() )  ComputeVelocity();
        return( TRUE );
    }

    if( rPropQuery.VarFloat( "Jump Pad\\Boost Yaw", m_Yaw, -360, 360 ) )
    {
        if( !rPropQuery.IsRead() )  ComputeVelocity();
        return( TRUE );
    }

    if( rPropQuery.VarFloat( "Jump Pad\\Boost Speed", m_Speed ) )
    {
        if( !rPropQuery.IsRead() )  ComputeVelocity();
        return( TRUE );
    }

    if( rPropQuery.VarFloat  ( "Jump Pad\\Air Control",     m_AirControl    ) ) return( TRUE );
    if( rPropQuery.VarBool   ( "Jump Pad\\Boost Only",      m_BoostOnly     ) ) return( TRUE );
    if( rPropQuery.VarBool   ( "Jump Pad\\Reboost Only",    m_ReboostOnly   ) ) return( TRUE );
    if( rPropQuery.VarBool   ( "Jump Pad\\Instantaneous",   m_Instantaneous ) ) return( TRUE );
    if( rPropQuery.VarFloat  ( "Jump Pad\\Editor Arc Time", m_ArcTime       ) ) return( TRUE );
    if( rPropQuery.VarVector3( "Jump Pad\\Boost Vector",    m_Velocity      ) ) return( TRUE );

    return( FALSE );
}

//==============================================================================

void jump_pad::PlayJump( void )
{
    SMP_UTIL_InitFXFromString( "MP_JumpPad2.fxo", m_FXHandle );
    ASSERT( m_FXHandle.Validate() );
    m_FXHandle.SetTransform( GetL2W() );

    g_AudioMgr.Play( "spawn", GetPosition(), GetZone1(), TRUE );
}

//==============================================================================

void jump_pad::OnRender( void )
{
}

//==============================================================================

void jump_pad::OnRenderTransparent( void )
{
    m_FXHandle.Render();
}

//==============================================================================
