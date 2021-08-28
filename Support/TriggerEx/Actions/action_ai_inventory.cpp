///////////////////////////////////////////////////////////////////////////
//
//  action_ai_inventory.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_ai_inventory.hpp"
#include "TemplateMgr\TemplateMgr.hpp"
#include "Characters\Character.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Inventory/Inventory2.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_ai_inventory::action_ai_inventory ( guid ParentGuid ) : action_ai_base( ParentGuid ), 
m_Code(AI_ITEM_CODE_GIVE),
m_TemplateIndex(-1),
m_Item( INVEN_NULL ),
m_RemoveAllItem(TRUE)
{
}

//=============================================================================

xbool action_ai_inventory::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;

    object* pObject = m_CharacterAffecter.GetObjectPtr();
    if (pObject &&  pObject->IsKindOf(character::GetRTTI()))
    {
        character &characterSource = character::GetSafeType( *pObject );
        switch (m_Code)
        {
        case AI_ITEM_CODE_GIVE:
            {
                characterSource.AddItemToInventory2( m_Item );
                return TRUE;
            }
            break;
        case AI_ITEM_CODE_TAKE:
            {
                characterSource.RemoveItemFromInventory2( m_Item );
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

void action_ai_inventory::OnEnumProp	( prop_enum& rPropList )
{
    rPropList.PropEnumEnum( "Operation",         "Give\0Take\0", "Do we give or take the item.", PROP_TYPE_DONT_SAVE | PROP_TYPE_MUST_ENUM );
    rPropList.PropEnumInt ( "Code" ,             "",  PROP_TYPE_DONT_SHOW  );
    
    if( m_Code == AI_ITEM_CODE_TAKE )
        rPropList.PropEnumBool( "Remove All", "Do you want to remove all of the item with that type or just one.", 0 );

    if (m_Code == AI_ITEM_CODE_GIVE)
        rPropList.PropEnumFileName( "TemplateBPX" ,  "template blueprints (*.bpx)|*.bpx|All Files (*.*)|*.*||", "Template Name, to determine what type of object to create.", 0 );

    if (m_Code == AI_ITEM_CODE_TAKE)
        rPropList.PropEnumEnum(     "Item",     inventory2::GetEnumString(), "Item to remove.", PROP_TYPE_MUST_ENUM ) ;
    
    action_ai_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_ai_inventory::OnProperty	( prop_query& rPropQuery )
{

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
            case AI_ITEM_CODE_GIVE:   rPropQuery.SetVarEnum( "Give" );     break;
            case AI_ITEM_CODE_TAKE:   rPropQuery.SetVarEnum( "Take" );     break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            if( x_stricmp( pString, "Give" )==0)        { m_Code = AI_ITEM_CODE_GIVE;}
            if( x_stricmp( pString, "Take" )==0)        { m_Code = AI_ITEM_CODE_TAKE;}
            
            return( TRUE );
        }
    }
    
    if( rPropQuery.VarBool( "Remove All", m_RemoveAllItem ) )
        return TRUE;

    if( action_ai_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char* action_ai_inventory::GetDescription( void )
{
    static big_string   Info;

    switch (m_Code)
    {
    case AI_ITEM_CODE_GIVE:
        {
            big_string ItemName;
            ItemName.Set("Unknown Item");
            if ( m_TemplateIndex >= 0 )
            {
                ItemName.Set(g_TemplateStringMgr.GetString(m_TemplateIndex));
            }
            Info.Set(xfs("%s gets %s", GetAIName(), ItemName.Get() ));
        }
        break;
    case AI_ITEM_CODE_TAKE:
        Info.Set(xfs("%s loses %s", GetAIName(), inventory2::ItemToName( m_Item )));
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}

//===========================================================================

#ifdef X_EDITOR
void action_ai_inventory::EditorPreGame( void )
{
    matrix4 rMat;
    rMat.Identity();

    if (m_TemplateIndex != -1)
    {
        guid ObjectGuid = g_TemplateMgr.EditorCreateSingleTemplateFromPath( g_TemplateStringMgr.GetString( m_TemplateIndex ), rMat.GetTranslation(), rMat.GetRotation(), -1, -1 ); 
    
        object* pObject = g_ObjMgr.GetObjectByGuid( ObjectGuid );
        if ( pObject )
        {
            pObject->EditorPreGame();
            g_ObjMgr.DestroyObject(ObjectGuid);
        }
    }
}

//=========================================================================

s32* action_ai_inventory::GetTemplateRef ( xstring& Desc )
{
    // Give blueprint?
    if( m_Code == AI_ITEM_CODE_GIVE )
    {
        Desc = "Item blueprint error: ";
        return &m_TemplateIndex;
    }

    return NULL;
}

//===========================================================================


#endif // X_EDITOR

