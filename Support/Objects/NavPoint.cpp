//=========================================================================
// NAV_POINT.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "NavPoint.hpp"
#include "e_Draw.hpp"
#include "e_View.hpp"
#include "Entropy.hpp"
#include "x_math.hpp"
#include "Objects\Player.hpp"
#include "Font\Font.hpp"

#include "Objects\HudObject.hpp"

f32 s_BitmapOffsetX = 250.0f;
f32 s_BitmapOffsetY = 150.0f;
//=========================================================================
// OBJECT DESCRIPTION
//=========================================================================

//=========================================================================
static struct nav_point_desc : public object_desc
{
    nav_point_desc( void ) : object_desc( 
        object::TYPE_NAV_POINT, 
        "Nav Point", 
        "HUD",
        object::ATTR_DRAW_2D                     |
        object::ATTR_RENDERABLE,

        FLAGS_GENERIC_EDITOR_CREATE | 
        FLAGS_IS_DYNAMIC  ) {}         

        //---------------------------------------------------------------------

        virtual object* Create          ( void )
        {
            return new nav_point;
        }

        //-------------------------------------------------------------------------

#ifdef X_EDITOR

        virtual s32  OnEditorRender( object& Object ) const
        {
            object_desc::OnEditorRender( Object );
            return EDITOR_ICON_TWO_WAY_ARROW;
        }

#endif // X_EDITOR

} s_NavPoint_Desc;

//=========================================================================

const object_desc&  nav_point::GetTypeDesc( void ) const
{
    return s_NavPoint_Desc;
}

//=========================================================================

const object_desc&  nav_point::GetObjectType( void )
{
    return s_NavPoint_Desc;
}


//=========================================================================
// FUNCTIONS
//=========================================================================

nav_point::nav_point( void )
{
    m_Active        = FALSE;
    m_TargetGuid    = NULL_GUID;
}

//=========================================================================

nav_point::~nav_point( void )
{

}

//=========================================================================

void nav_point::OnRender( void )
{
    if( !m_Active )
        return;

    // Get the active player
    slot_id PlayerSlot  = g_ObjMgr.GetFirst( object::TYPE_PLAYER ) ;
    player* pPlayer     = NULL;

    while ( PlayerSlot != SLOT_NULL )
    {
        pPlayer = (player*)g_ObjMgr.GetObjectBySlot( PlayerSlot ) ;
        if ( pPlayer && pPlayer->IsActivePlayer() )
        {
            break;
        }
        PlayerSlot = g_ObjMgr.GetNext( PlayerSlot ) ;
    }

    if( pPlayer == NULL )
        return;

    vector3 TargetPosition = GetPosition();

    // If we have a target guid use its position.
    if( m_TargetGuid )
    {
        object* pObject = g_ObjMgr.GetObjectByGuid( m_TargetGuid );
        ASSERT( pObject );

        if( pObject && pObject->IsKindOf( actor::GetRTTI() ) )
        {
            actor& Actor = actor::GetSafeType( *pObject );
            vector3 Temp;
            Actor.GetHeadAndRootPosition( TargetPosition, Temp );
            TargetPosition.GetY() += 35.0f;
        }
        else if( pObject )
        {
            TargetPosition = pObject->GetPosition();
        }

    }


    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID != SLOT_NULL )
    {
        object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
        hud_object& Hud = hud_object::GetSafeType( *pObj );

        Hud.m_PlayerHuds[ 0 ].m_Icon.AddIcon(   ICON_WAYPOINT, 
            TargetPosition, 
            TargetPosition,
            FALSE,
            TRUE, 
            GUTTER_ELLIPSE, 
            XCOLOR_GREEN, 
            NULL,
            FALSE,
            TRUE, 
            1.0f );
    }

}

//=========================================================================

bbox nav_point::GetLocalBBox( void ) const
{
    return bbox( vector3(0.0f, 0.0f, 0.0f) , 50.0f );
}

//=========================================================================

void nav_point::OnActivate( xbool Flag )
{
    m_Active = Flag;
}

//=========================================================================

void nav_point::OnEnumProp( prop_enum& List )
{
    object::OnEnumProp      ( List );

    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID != SLOT_NULL )
    {
        object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
        hud_object& Hud = hud_object::GetSafeType( *pObj );

        Hud.m_PlayerHuds[ 0 ].m_Icon.OnEnumProp( List );
    }

    List.PropEnumGuid    ( "Nav Point\\Target Guid", "Guid of an object that we want to target", PROP_TYPE_EXPOSE );
    List.PropEnumBool    ( "Nav Point\\Is Currently Active", "Is this Waypoint currently active?", PROP_TYPE_EXPOSE | PROP_TYPE_READ_ONLY | PROP_TYPE_DONT_SHOW );
}

//=========================================================================

xbool nav_point::OnProperty( prop_query& rPropQuery )
{
    if( object::OnProperty( rPropQuery ) )
        return TRUE;

    slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID != SLOT_NULL )
    {
        object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
        hud_object& Hud = hud_object::GetSafeType( *pObj );

        if( Hud.m_PlayerHuds[ 0 ].m_Icon.OnProperty( rPropQuery ) ) 
            return TRUE;
    }

    if ( rPropQuery.IsVar( "Nav Point\\Is Currently Active" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarBool( m_Active );
        }

        return TRUE;
    }

    if( rPropQuery.IsVar( "Nav Point\\Target Guid" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarGUID( m_TargetGuid );
        }
        else
        {
            m_TargetGuid =  rPropQuery.GetVarGUID();
        }
        return TRUE;
    }

    return FALSE;
}

//=========================================================================
