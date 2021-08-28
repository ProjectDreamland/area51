//==============================================================================
//
//  GameProp.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "GameProp.hpp"

//==============================================================================
//  OBJECT DESCRIPTION
//==============================================================================

static struct game_prop_desc : public object_desc
{
    game_prop_desc( void ) 
        :   object_desc( object::TYPE_GAME_PROP, 
                         "GameProp",
                         "Multiplayer",
                         object::ATTR_NULL + object::ATTR_NO_RUNTIME_SAVE,
                         FLAGS_GENERIC_EDITOR_CREATE | FLAGS_IS_DYNAMIC )
    {
        // Empty function body.
    }

    //--------------------------------------------------------------------------

    virtual object* Create( void ) 
    { 
        return( new game_prop ); 
    }

    //--------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        if( Object.IsKindOf( game_prop::GetRTTI() ) )
        {
            game_prop GameProp = game_prop::GetSafeType( Object );

            EditorIcon_Draw( EDITOR_ICON_CTF_FLAG, 
                GameProp.GetL2W(), 
                !!(GameProp.GetAttrBits() & object::ATTR_EDITOR_SELECTED), 
                GameProp.GetCircuit().GetColor() );

            if( mp_settings::s_Selected )
                GameProp.GetCircuit().SpecialRender( GameProp.GetPosition() );

            if( !!(GameProp.GetAttrBits() & object::ATTR_EDITOR_SELECTED) )
            {
                if( GameProp.m_Kind == game_prop::CAPTURE_POINT )
                {
                    xcolor  Color = GameProp.GetCircuit().GetColor();
                    vector3 Center( 0, GameProp.m_Elevation, 0 );
                    Center = GameProp.GetL2W().Transform( Center );
                    draw_Marker( Center, Color );
                    draw_Sphere( Center, GameProp.m_Radius, Color );
                    draw_Line  ( Center, GameProp.GetPosition(), Color );
                }
            }
        }
        else
        {
            ASSERT( FALSE );
        }

        return( -1 );
    }

#endif // X_EDITOR

} s_game_prop_Desc;

//==============================================================================
//  FUNCTIONS
//==============================================================================

const object_desc& game_prop::GetTypeDesc( void ) const
{
    return( s_game_prop_Desc );
}

//==============================================================================

const object_desc& game_prop::GetObjectType( void )
{
    return( s_game_prop_Desc );
}

//==============================================================================

game_prop::game_prop( void )
{
    m_Kind      = (kind)0;
    m_Radius    = 750.0f;
    m_Elevation = 200.0f;
}

//==============================================================================

game_prop::~game_prop( void )
{
}

//==============================================================================

bbox game_prop::GetLocalBBox( void ) const 
{ 
    bbox BBox( vector3(0,0,0), 50.0f );
    BBox += vector3(0,300,0);
    return( BBox );
}

//==============================================================================

void game_prop::OnEnumProp( prop_enum& List )
{
    List.PropEnumHeader( "GameProp", 
                        "Provides position for game type specific objects.", 
                        PROP_TYPE_DONT_SAVE | 
                        PROP_TYPE_DONT_EXPORT | 
                        PROP_TYPE_DONT_SAVE_MEMCARD );
    List.PropEnumEnum( "GameProp\\Kind", 
                        "Flag Base\0Capture Point\0", 
                        "What kind of game prop?", 
                        PROP_TYPE_DONT_SAVE_MEMCARD |
                        PROP_TYPE_MUST_ENUM );
    List.PropEnumEnum( "GameProp\\Team", 
                        "Team 0 (Alpha)\0Team 1 (Omega)\0All\0None\0", 
                        "Which team is this prop affiliated with?", 
                        PROP_TYPE_DONT_SHOW |
                        PROP_TYPE_DONT_SAVE | 
                        PROP_TYPE_DONT_EXPORT | 
                        PROP_TYPE_DONT_SAVE_MEMCARD );

    // Capture point specific.
    {
        u32 Show = (m_Kind == CAPTURE_POINT) ? 0 : PROP_TYPE_DONT_SHOW;

        List.PropEnumHeader( "Capture Point", "Capture Point properties.",
                        Show |
                        PROP_TYPE_DONT_SAVE | 
                        PROP_TYPE_DONT_EXPORT | 
                        PROP_TYPE_DONT_SAVE_MEMCARD );
        List.PropEnumFloat( "Capture Point\\Radius", 
                        "Range of influence.",
                        Show |
                        PROP_TYPE_DONT_SAVE_MEMCARD );
        List.PropEnumFloat( "Capture Point\\Elevation", 
                        "Elevation of sphere from base.",
                        Show |
                        PROP_TYPE_DONT_SAVE_MEMCARD );
    }

    m_Circuit.OnEnumProp( List );
    object::OnEnumProp( List );
}
    
//==============================================================================

xbool game_prop::OnProperty( prop_query& Query )
{
    if( object::OnProperty( Query ) )
    {
        return( TRUE );
    }

    if( m_Circuit.OnProperty( Query ) )
    {
        return( TRUE );
    }

    if( Query.IsVar( "GameProp\\Kind" ) )
    {
        if( Query.IsRead() )
        {
            switch( m_Kind )
            {
            case FLAG_BASE:     Query.SetVarEnum( "Flag Base"     );   break;
            case CAPTURE_POINT: Query.SetVarEnum( "Capture Point" );   break;
            default:            ASSERT( FALSE );                       break;
            } 
            return( TRUE );
        }
        else
        {
            const char* pString = Query.GetVarEnum();
            if( x_stricmp( pString, "Flag Base"     ) == 0 )    { m_Kind = FLAG_BASE;     }
            if( x_stricmp( pString, "Capture Point" ) == 0 )    { m_Kind = CAPTURE_POINT; }
            return( TRUE );
        }
    }

    if( Query.VarFloat( "Capture Point\\Radius",    m_Radius    ) )  
    {
        return( TRUE );
    }
    if( Query.VarFloat( "Capture Point\\Elevation", m_Elevation ) )  return( TRUE );

    // NOTE - This field is defunct.  Only here for backwards compatability.
    if( Query.IsVar( "GameProp\\Team" ) )
    {
        if( Query.IsRead() )
        {
            /*
            switch( m_TeamBits )
            {
            case 0xFFFFFFFF:    Query.SetVarEnum( "All" );             break;
            case 0x00000000:    Query.SetVarEnum( "None" );            break;
            case 0x00000001:    Query.SetVarEnum( "Team 0 (Alpha)" );  break;
            case 0x00000002:    Query.SetVarEnum( "Team 1 (Omega)" );  break;
            default:            ASSERT( FALSE );                       break;
            }    
            return( TRUE );
            */
        }
        else
        {
            const char* pString = Query.GetVarEnum();
            if( x_stricmp( pString, "None"           ) == 0 )    { m_Circuit.SetCircuit(0); }
            if( x_stricmp( pString, "Team 0 (Alpha)" ) == 0 )    { m_Circuit.SetCircuit(1); }
            if( x_stricmp( pString, "Team 1 (Omega)" ) == 0 )    { m_Circuit.SetCircuit(2); }
            if( x_stricmp( pString, "All"            ) == 0 )    { m_Circuit.SetCircuit(3); }
            return( TRUE );
        }
    }

    return( FALSE );
}

//==============================================================================
