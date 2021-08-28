///////////////////////////////////////////////////////////////////////////
//
//  give_player_item.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\give_player_item.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Objects\Player.hpp"
#include "Dictionary\global_dictionary.hpp"

#include "Entropy.hpp"

//=========================================================================
// GIVE_PLAYER_ITEM
//=========================================================================

give_player_item::give_player_item ( guid ParentGuid ) : actions_base( ParentGuid ), m_TemplateIndex(-1)
{
}

//=============================================================================

void  give_player_item::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * give_player_item::Execute" );

    ( void ) pParent;

    if ( m_TemplateIndex >= 0 )
    {
        // Get the player and all necessary information for the object we're going to give him.
        player* pPlayer = SMP_UTIL_GetActivePlayer();

        ASSERT( pPlayer );
        if ( pPlayer == NULL )
        {
            return;
        }

        guid CreatedItemGuid = g_TemplateMgr.CreateSingleTemplate( g_TemplateStringMgr.GetString(m_TemplateIndex), pPlayer->GetPosition(), radian3( 0,1,0 ), pPlayer->GetZone1(), pPlayer->GetZone2() );
        object_ptr<inventory_item> pInventoryItem( CreatedItemGuid );

        ASSERT( pInventoryItem.IsValid() );
        if ( !pInventoryItem.IsValid() )
        {
            return;
        }

        pPlayer->AddItemToInventory( pInventoryItem.m_pObject );
        pInventoryItem.m_pObject->DeactivateItem();
    }
}

//=============================================================================

void give_player_item::OnEnumProp ( prop_enum& rPropList )
{
    //object info
/*    
    rPropList.AddExternal( "Inventory Blueprint",
                      "File\0blueprint\0",
                      "Resource for this item",
                       PROP_TYPE_MUST_ENUM | PROP_TYPE_EXTERNAL );
*/
    rPropList.AddFileName( "TemplateBPX" ,  "template blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||", "Template Name, to determine what type of object to create." );
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool give_player_item::OnProperty ( prop_query& rPropQuery )
{
/*
    if ( rPropQuery.IsVar( "Inventory Blueprint" ) )
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarExternal( m_TemplateName, 32 );
        }
        else
        {
            x_strcpy( m_TemplateName, rPropQuery.GetVarExternal() );

           //add new blueprint
#ifdef TARGET_PC
            g_TemplateMgr.EditorAddBlueprintRef(rPropQuery.GetVarExternal());
#endif
        }
        
        return TRUE;
    }
*/
    if ( rPropQuery.IsVar( "TemplateBPX" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if ( m_TemplateIndex < 0 )
            {
                rPropQuery.SetVarFileName( "", 256 );
            }
            else
            {
                rPropQuery.SetVarFileName( g_TemplateStringMgr.GetString( m_TemplateIndex ), 256 );
            }
        }
        else
        {
            if ( x_strlen( rPropQuery.GetVarFileName() ) > 0 )
            {
                m_TemplateIndex = g_TemplateStringMgr.Add( rPropQuery.GetVarFileName() );
            }
        }

        return TRUE;
     }
    
    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}


