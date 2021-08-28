//=========================================================================
//
//  Gray.cpp
//
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================

#include "Gray.hpp"

//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

static struct gray_desc : public object_desc
{
        gray_desc( void ) : object_desc( 
            object::TYPE_GRAY, 
            "NPC - Gray", 
            "AI",
            object::ATTR_NEEDS_LOGIC_TIME       |
            object::ATTR_COLLIDABLE             | 
            object::ATTR_BLOCKS_ALL_PROJECTILES | 
            object::ATTR_BLOCKS_RAGDOLL         | 
            object::ATTR_BLOCKS_CHARACTER_LOS   | 
            object::ATTR_BLOCKS_PLAYER_LOS      | 
            object::ATTR_BLOCKS_SMALL_DEBRIS    | 
            object::ATTR_RENDERABLE             | 
            object::ATTR_TRANSPARENT            |
            object::ATTR_SPACIAL_ENTRY          |
            object::ATTR_CHARACTER_OBJECT       |
            object::ATTR_DAMAGEABLE             |
            object::ATTR_LIVING                 |
            object::ATTR_CAST_SHADOWS,

            FLAGS_TARGETS_OBJS |
            FLAGS_IS_DYNAMIC |
            FLAGS_NO_ICON )   { }

    //-------------------------------------------------------------------------

    virtual object* Create( void ) { return new gray; } 

    //-------------------------------------------------------------------------

    virtual const char* QuickResourceName( void ) const
    {
        return "SkinGeom";
    }

    //-------------------------------------------------------------------------

    virtual const char* QuickResourcePropertyName( void ) const 
    {
        return "RenderInst\\File";
    }

    //-------------------------------------------------------------------------

#ifdef X_EDITOR

    virtual s32  OnEditorRender( object& Object ) const
    {
        object_desc::OnEditorRender( Object );
        return -1;
    }

#endif // X_EDITOR

} s_gray_Desc;

//=============================================================================

const object_desc& gray::GetTypeDesc( void ) const
{
    return s_gray_Desc;
}

//=============================================================================

const object_desc& gray::GetObjectType( void )
{
    return s_gray_Desc;
}




//=========================================================================
// GRAY CHARACTER
//=========================================================================

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( push )
    #pragma warning( disable:4355 ) // 'this' : used in base member initializer list
#endif

gray::gray() :
    m_Idle   ( *this, character_state::STATE_IDLE    ),
/*    m_Alert  ( *this, character_state::STATE_ALERT   ),
    m_Attack ( *this, character_state::STATE_ATTACK  ),
    m_Search ( *this, character_state::STATE_SEARCH  ),
    m_Cover  ( *this, character_state::STATE_COVER   ),*/
    m_Death  ( *this, character_state::STATE_DEATH   )
{
    // Setup pointer to loco for base class to use
    m_pLoco             = &m_Loco;

    m_Faction      = FACTION_GRAY;
    m_FriendFlags |= ( FACTION_GRAY ) ;
    m_gShield      = NULL_GUID;
}

#if defined TARGET_XBOX && _MSC_VER >= 1300
    #pragma warning( pop )
#endif

//=========================================================================

gray::~gray()
{
}

//=========================================================================

//=========================================================================
// EDITOR FUNCTIONS
//=========================================================================

void gray::OnEnumProp ( prop_enum&    List )
{
    // Call base class
    character::OnEnumProp(List);

    // Header
    List.PropEnumHeader( "Gray","Gray NPC", 0 );
}

//=============================================================================

void gray::OnPain( const pain& Pain )
{
    character::OnPain(Pain);
}

//=============================================================================

void gray::OnRender( void )
{
    character::OnRender();
}

//=============================================================================

xbool gray::OnProperty ( prop_query& I )
{
    // Call base class
    if (character::OnProperty(I))
        return TRUE;

    // Not found
    return FALSE;
}

//=============================================================================
//  OnInit
//
//      For now, hit locations are going to be added here.  They will get the
//      proper boneID set when the model is actually loaded.
//=============================================================================
void gray::OnInit( void )
{
    // Call base class
    character::OnInit();
}

//=============================================================================

xbool gray::OnAnimEvent( const anim_event& Event, const vector3& WorldPos )
{
    s32 ZoneID = GetZone1();

    // Call base class
    if (character::OnAnimEvent(Event, WorldPos))
        return TRUE;

    // Old event?
    if (x_stricmp(Event.GetType(), "Old Event") == 0)
    {
        switch(Event.GetInt( anim_event::INT_IDX_OLD_TYPE ))
        {
            case ANIM_EVENT_SFX_FOOT_HEEL:
            {
                switch( m_GroundMaterial )
                {
                    case MAT_TYPE_NULL:
                    
                    break;
                    case MAT_TYPE_EARTH:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Earth_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_ROCK:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Rock_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_CONCRETE:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Concrete_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_SOLID_METAL:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Metal_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_HOLLOW_METAL:    
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Metal_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_METAL_GRATE:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Grate_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_PLASTIC:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Plastic_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_WATER:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Water_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_WOOD:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Wood_Heel", WorldPos, ZoneID, TRUE );
                    break;
                
                    case MAT_TYPE_ENERGY_FIELD:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Concrete_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_BULLET_PROOF_GLASS:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Concrete_Heel", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_ICE:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Concrete_Heel", WorldPos, ZoneID, TRUE );
                    break;
                
                    default:
                    
                        ASSERTS( 0, "Unknown sound material" );
                    break;
                };   
                return TRUE;
            }
            break;
            case ANIM_EVENT_SFX_FOOT_TOE:
            {   
                switch( m_GroundMaterial )
                {
                    case MAT_TYPE_NULL:
                    
                    break;
                    case MAT_TYPE_EARTH:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Earth_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_ROCK:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Rock_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_CONCRETE:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Concrete_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_SOLID_METAL:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Metal_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_HOLLOW_METAL:    
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Metal_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_METAL_GRATE:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Grate_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_PLASTIC:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Plastic_Toe", WorldPos , ZoneID, TRUE);
                    break;
                    case MAT_TYPE_WATER:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Water_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_WOOD:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Wood_Toe", WorldPos, ZoneID, TRUE );
                    break;
                
                    case MAT_TYPE_ENERGY_FIELD:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Concrete_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_BULLET_PROOF_GLASS:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Concrete_Toe", WorldPos, ZoneID, TRUE );
                    break;
                    case MAT_TYPE_ICE:
                        g_AudioMgr.PlayVolumeClipped( "FF_Boot_Concrete_Toe", WorldPos, ZoneID, TRUE );
                    break;
                
                    default:

                        ASSERTS( 0, "Unknown sound material" );
                    break;
                };
                return TRUE;
            }
            break;
        }
    }

    // Event not handled
    return FALSE;
}

