///////////////////////////////////////////////////////////////////////////
//
//  action_player_inventory.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_player_inventory.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Objects\Player.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Inventory/Inventory2.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_player_inventory::action_player_inventory ( guid ParentGuid ) : actions_ex_base( ParentGuid ), 
m_Code(PLAYER_ITEM_CODE_GIVE),
m_Item( INVEN_NULL ),
m_RemoveAllItem(TRUE)
{
}

//=============================================================================

xbool action_player_inventory::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if (pPlayer )
    {
        switch (m_Code)
        {
        case PLAYER_ITEM_CODE_GIVE:
            {
                pPlayer->AddItemToInventory2( m_Item );
                return TRUE;
            }
            break;
        case PLAYER_ITEM_CODE_TAKE:
            {
                pPlayer->RemoveItemFromInventory2( m_Item, m_RemoveAllItem );
                return TRUE;
            }
            break;
        default:
            ASSERT(0);
            break;
        }
    }

    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}

//=============================================================================

void action_player_inventory::OnEnumProp	( prop_enum& rPropList )
{
    rPropList.PropEnumEnum( "Operation",         "Give\0Take\0", "Do we give or take the item.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumInt ( "Code" ,             "",  PROP_TYPE_DONT_SHOW  );
    
    if( m_Code == PLAYER_ITEM_CODE_TAKE )
        rPropList.PropEnumBool( "Remove All", "Do you want to remove all of the item with that type or just one.", 0 );

    rPropList.PropEnumEnum(     "Item",     inventory2::GetEnumString(), "Item to remove.", PROP_TYPE_MUST_ENUM ) ;
    
    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_player_inventory::OnProperty	( prop_query& rPropQuery )
{ 
    if( rPropQuery.IsVar( "Item" ) )
    {
        if ( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( inventory2::ItemToName( m_Item ) );
        }
        else
        {
            m_Item = inventory2::NameToItem( rPropQuery.GetVarEnum() );
        }
        return TRUE;
    }

    if ( rPropQuery.VarInt ( "Code"  , m_Code ) )
        return TRUE;
    
    if ( rPropQuery.IsVar  ( "Operation" ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case PLAYER_ITEM_CODE_GIVE:   rPropQuery.SetVarEnum( "Give" );     break;
            case PLAYER_ITEM_CODE_TAKE:   rPropQuery.SetVarEnum( "Take" );     break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "Give" )==0)        { m_Code = PLAYER_ITEM_CODE_GIVE;}
            if( x_stricmp( pString, "Take" )==0)        { m_Code = PLAYER_ITEM_CODE_TAKE;}
            
            return( TRUE );
        }
    }
    
    if( rPropQuery.VarBool( "Remove All", m_RemoveAllItem ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char* action_player_inventory::GetDescription( void )
{
    static big_string   Info;

    switch (m_Code)
    {
    case PLAYER_ITEM_CODE_GIVE:
        Info.Set(xfs("Give %s to Player", inventory2::ItemToName( m_Item ) ));
        break;
    case PLAYER_ITEM_CODE_TAKE:
        Info.Set(xfs("Take %s from Player", inventory2::ItemToName( m_Item ) ));
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}
