///////////////////////////////////////////////////////////////////////////////
//
//  condition_player_has_item.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_player_has_item.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "..\Support\Objects\Player.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Inventory/Inventory2.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

condition_player_has_item::condition_player_has_item( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_Code(ITEM_CODE_HAS_ITEM),
m_Item( INVEN_NULL )
{
}

//=============================================================================

xbool condition_player_has_item::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;

    player* pPlayer = SMP_UTIL_GetActivePlayer();

    if ( pPlayer == NULL )
    {
        return FALSE;
    }

    xbool bHasItem  = TRUE;
    
    bHasItem = pPlayer->GetInventory2().HasItem( m_Item );

    switch (m_Code)
    {
        case ITEM_CODE_HAS_ITEM:
            if ( bHasItem )
            {
                return TRUE;
            }
            break;
        case ITEM_CODE_NOT_HAVE_ITEM:    
            if ( !bHasItem )
            {
                return TRUE;
            }
            break;
        default:
            ASSERT(0);
            break;
    }
    
    return FALSE; 
}    

//=============================================================================

void condition_player_has_item::OnEnumProp ( prop_enum& rPropList )
{ 
    rPropList.PropEnumInt ( "Code" ,     "",  PROP_TYPE_DONT_SHOW  );
    rPropList.PropEnumEnum( "Item", inventory2::GetEnumString(), "Item to check.", PROP_TYPE_MUST_ENUM ) ;
    
    rPropList.PropEnumEnum( "Operation", "Has Item\0Does NOT have item\0", "Logic available to this condtional.", PROP_TYPE_DONT_SAVE  );

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_player_has_item::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;
  
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

    if ( rPropQuery.IsVar( "Operation") )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case ITEM_CODE_HAS_ITEM:      rPropQuery.SetVarEnum( "Has Item" );           break;
            case ITEM_CODE_NOT_HAVE_ITEM: rPropQuery.SetVarEnum( "Does NOT have item" ); break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "Has Item" )==0)            { m_Code = ITEM_CODE_HAS_ITEM;}
            if( x_stricmp( pString, "Does NOT have item" )==0)  { m_Code = ITEM_CODE_NOT_HAVE_ITEM;}
            
            return( TRUE );
        }
    }

    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* condition_player_has_item::GetDescription( void )
{
    static big_string   Info;
    switch ( m_Code )
    {
    case ITEM_CODE_HAS_ITEM:   
        Info.Set(xfs("Player has %s.", inventory2::ItemToName(m_Item) ) );
        break;
    case ITEM_CODE_NOT_HAVE_ITEM: 
        Info.Set(xfs("Player does NOT have %s.", inventory2::ItemToName(m_Item) ) );
        break;
    default:
        ASSERT(0);
        break;
    }
    return Info.Get();
}
