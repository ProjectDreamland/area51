// friendly_scientist : implementation file
/////////////////////////////////////////////////////////////////////////////

#include "FriendlyScientist.hpp"
//#include "TaskSystem\character_sub_task.hpp"

//===========================================================================
// friendly_scientist

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

friendly_scientist::friendly_scientist() :
    m_Idle              ( *this, character_state::STATE_IDLE    ),
    m_Alert             ( *this, character_state::STATE_ALERT   ),
    m_Flee              ( *this, character_state::STATE_FLEE    ),
    m_Cover             ( *this, character_state::STATE_COVER   ),
    m_Alarm             ( *this, character_state::STATE_ALARM   ),
    m_Death             ( *this, character_state::STATE_DEATH   )
{
    // Setup pointer to loco for base class to use
    m_pLoco             = &m_Loco ;
    m_Faction = FACTION_WORKERS ;
    m_FriendFlags |= ( FACTION_WORKERS | FACTION_GRAY | FACTION_MILITARY | FACTION_PLAYER_NORMAL ) ;
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif

//===========================================================================

friendly_scientist::~friendly_scientist()
{
}

//===========================================================================

static struct friendly_scientist_desc : public object_desc
{
    friendly_scientist_desc( void ) : object_desc( 
        object::TYPE_FRIENDLY_SCIENTIST, 
        "Friendly Scientist",
        "AI",
        object::ATTR_NEEDS_LOGIC_TIME       |
        object::ATTR_RENDERABLE             |
        object::ATTR_TRANSPARENT            |
        object::ATTR_CHARACTER_OBJECT       |
        object::ATTR_SPACIAL_ENTRY          |
        object::ATTR_COLLIDABLE             |
        object::ATTR_BLOCKS_ALL_PROJECTILES | 
        object::ATTR_BLOCKS_RAGDOLL         | 
        object::ATTR_BLOCKS_CHARACTER_LOS   | 
        object::ATTR_BLOCKS_PLAYER_LOS      | 
        object::ATTR_BLOCKS_SMALL_DEBRIS    | 
        object::ATTR_DAMAGEABLE             |
        object::ATTR_LIVING                 |
        object::ATTR_CAST_SHADOWS,

#ifdef sbroumley
        FLAGS_GENERIC_EDITOR_CREATE |
#endif
        FLAGS_TARGETS_OBJS |
        FLAGS_IS_DYNAMIC            |
        FLAGS_NO_ICON
        ) {}
    
    //-------------------------------------------------------------------------
    
    virtual object* Create( void ) { return new friendly_scientist; }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

} s_friendly_scientist_desc ;

//=========================================================================

const object_desc& friendly_scientist::GetTypeDesc( void ) const
{
    return s_friendly_scientist_desc;
}

//=========================================================================

const object_desc&  friendly_scientist::GetObjectType   ( void )
{
    return s_friendly_scientist_desc;
}

//=============================================================================

void friendly_scientist::OnInit( void )
{
    character::OnInit() ;
}

//=============================================================================

void friendly_scientist::OnEnumProp( prop_enum& rList )
{
    rList.PropEnumHeader( "Scientist", "TEST", PROP_TYPE_HEADER ) ;
    rList.PropEnumHeader( "Scientist\\Conversation Values", "TEST", PROP_TYPE_HEADER ) ;

    character::OnEnumProp( rList ) ;
}

//=============================================================================

xbool friendly_scientist::OnProperty( prop_query& rPropQuery )
{
    if ( character::OnProperty( rPropQuery ) )
    {
        return TRUE ;
    }

    return FALSE ;
}

