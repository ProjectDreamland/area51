///////////////////////////////////////////////////////////////////////////////
//
//  condition_player_button_state.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "condition_player_button_state.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "..\Support\Objects\Player.hpp"
#include "..\triggerex_object.hpp"

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

condition_player_button_state::condition_player_button_state( conditional_affecter* pParent ):  conditional_ex_base(  pParent ),
m_Code(BUTTON_CODE_PRESSED),
m_ButtonID(ingame_pad::ACTION_USE)
{
}

//=============================================================================

xbool condition_player_button_state::Execute( guid TriggerGuid )
{    
    (void) TriggerGuid;
    
    player* pPlayer = SMP_UTIL_GetActivePlayer();
    if ( !pPlayer )
        return FALSE;

    f32 WasVal = g_IngamePad[ pPlayer->GetActivePlayerPad() ].GetLogical( m_ButtonID ).WasValue;
    f32 IsVal  = g_IngamePad[ pPlayer->GetActivePlayerPad() ].GetLogical( m_ButtonID ).IsValue;
    xbool WasPressed = WasVal > 0.0f;
    xbool IsPressed  = IsVal  > 0.0f;

    switch ( m_Code )
    {
    case BUTTON_CODE_PRESSED:  
        return ( WasPressed || IsPressed );
        break;
    case BUTTON_CODE_NOT_PRESSED: 
        return !( WasPressed || IsPressed );
        break;
    default:
        ASSERT(0);
        break;
    }

    return FALSE;
}    

//=============================================================================

void condition_player_button_state::OnEnumProp ( prop_enum& rPropList )
{ 
    rPropList.PropEnumEnum( "Logical Input", ingame_pad::GetLogicalIDEnum(), "Logical mapping to watch.", 0 ) ;
    rPropList.PropEnumEnum( "Operation",     "Pressed\0NOT Pressed\0",       "Logic available to this condtional.", PROP_TYPE_MUST_ENUM );

    conditional_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool condition_player_button_state::OnProperty ( prop_query& rPropQuery )
{
    if( rPropQuery.IsVar( "Logical Input" ) )
    {
        if ( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( ingame_pad::GetLogicalIDName( m_ButtonID ) );
        }
        else
        {
            m_ButtonID = ingame_pad::GetLogicalIDByName( rPropQuery.GetVarEnum() );
        }
        return TRUE;
    }

    if ( rPropQuery.VarInt ( "Code" , m_Code ) )
        return TRUE;

    if ( rPropQuery.IsVar( "Operation") )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_Code )
            {
            case BUTTON_CODE_PRESSED:      rPropQuery.SetVarEnum( "Pressed" );     break;
            case BUTTON_CODE_NOT_PRESSED:  rPropQuery.SetVarEnum( "NOT Pressed" ); break;
            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "Pressed" )==0)      { m_Code = BUTTON_CODE_PRESSED;}
            if( x_stricmp( pString, "NOT Pressed" )==0)  { m_Code = BUTTON_CODE_NOT_PRESSED;}
            
            return( TRUE );
        }
    }

    if( conditional_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* condition_player_button_state::GetDescription( void )
{
    static big_string   Info;
    switch ( m_Code )
    {
    case BUTTON_CODE_PRESSED:   
        Info.Set(xfs("%s Pressed.", ingame_pad::GetLogicalIDName(m_ButtonID) ) );
        break;
    case BUTTON_CODE_NOT_PRESSED: 
        Info.Set(xfs("%s NOT Pressed.", ingame_pad::GetLogicalIDName(m_ButtonID) ) );
        break;
    default:
        ASSERT(0);
        break;
    }

    return Info.Get();
}
