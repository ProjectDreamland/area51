///////////////////////////////////////////////////////////////////////////////
//
//  action_mission_failed.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_mission_failed.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"
#include "Objects\HudObject.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "objects\player.hpp"

static const xcolor s_LogicColor          (200,200,0);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_mission_failed::action_mission_failed( guid ParentGuid ):  
    actions_ex_base( ParentGuid ),
    m_TableName( -1 ),
    m_ReasonName( -1 )
{
}

//=============================================================================

xbool action_mission_failed::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    player* pPlayerObj = SMP_UTIL_GetActivePlayer();
    if ( pPlayerObj && (m_TableName > -1) && (m_ReasonName > -1) )
    {
        // set player up for mission failed state here
        pPlayerObj->OnMissionFailed( m_TableName, m_ReasonName );
        return TRUE;
    }

    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}    

//=============================================================================

void action_mission_failed::OnEnumProp ( prop_enum& rPropList )
{ 
    x_try;

    xstring TableString;
    TableString.Clear();

    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID == SLOT_NULL )
        x_throw( "No Hud Object specified in this project." );

    hud_object& Hud = hud_object::GetSafeType( *g_ObjMgr.GetObjectBySlot( SlotID ) );
    Hud.GetBinaryResourceName(TableString);

    rPropList.PropEnumEnum    ( "Table Name", (const char*)TableString, "Which table to get the string from.", 0 );
    rPropList.PropEnumString  ( "Reason Name", "The name of the mission failed reason from which to get the string.", 0 );

    //object info
    actions_ex_base::OnEnumProp( rPropList );

    x_catch_display;
}

//=============================================================================

xbool action_mission_failed::OnProperty ( prop_query& rPropQuery )
{
    if( rPropQuery.IsVar( "Table Name" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_TableName >= 0 )
                rPropQuery.SetVarEnum( g_StringMgr.GetString( m_TableName ) );
            else
                rPropQuery.SetVarEnum( "" );
        }
        else
        {

            m_TableName = g_StringMgr.Add( rPropQuery.GetVarEnum() );
        }

        return TRUE;
    }

    if( rPropQuery.IsVar( "Reason Name" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_ReasonName >= 0 )
            {
                const char* pString = g_StringMgr.GetString( m_ReasonName );
                rPropQuery.SetVarString( pString, x_strlen( pString )+1 );
            }
            else
            {
                rPropQuery.SetVarString( "", 1 );
            }
        }
        else
        {
            m_ReasonName= g_StringMgr.Add( rPropQuery.GetVarString() );
        }

        return TRUE;
    }

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_mission_failed::GetDescription( void )
{
    static big_string   Info;
    Info.Set("Set off the \"mission failed\" death sequence");
    return Info.Get();
}

//=============================================================================
