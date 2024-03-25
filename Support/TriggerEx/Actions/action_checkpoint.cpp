///////////////////////////////////////////////////////////////////////////
//
//  action_checkpoint.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_checkpoint.hpp"

#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Objects\HudObject.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#include "CheckPointMgr\CheckPointMgr.hpp"
#include "StateMgr/StateMgr.hpp" 

//=========================================================================
//=========================================================================

action_checkpoint::action_checkpoint ( guid ParentGuid ) : actions_ex_base( ParentGuid )
{
    m_CPTableName = -1;
    m_CPTitleName = -1;
}

//=============================================================================

xbool action_checkpoint::Execute ( f32 DeltaTime )
{
    (void)DeltaTime;

    // Display the text to the hud
    if( (m_CPTableName != -1) && (m_CPTitleName != -1 ) )
        g_GameTextMgr.DisplayMessage( g_StringMgr.GetString( m_CPTableName ), g_StringMgr.GetString( m_CPTitleName ) );   

    // Set the checkpoint!
    g_CheckPointMgr.SetCheckPoint( m_TriggerGuidRestore, m_TriggerGuidDebugSkip, m_CPTableName, m_CPTitleName );
    g_AudioMgr.Play("Dialog");

#if !defined(X_EDITOR)
    // Auto save any changes made to the profile
    g_StateMgr.SilentSaveProfile();
#endif
    return TRUE;
}

//=============================================================================

void action_checkpoint::OnEnumProp ( prop_enum& List )
{
    x_try;

    xstring TableString;
    TableString.Clear();

    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

    if( SlotID == SLOT_NULL )
        x_throw( "No Hud Object specified in this project." );

    hud_object& Hud = hud_object::GetSafeType( *g_ObjMgr.GetObjectBySlot( SlotID ) );
    Hud.GetBinaryResourceName(TableString);

    List.PropEnumEnum    ( "String Table", (const char*)TableString, "Which table to get the string from.", 0 );
    List.PropEnumString  ( "Checkpoint Name", "The name of the title from the stringtable for the checkpoint.", 0 );
    List.PropEnumGuid    ( "Restore Trigger", "This GUID should point to the trigger that is to be executed upon restoration of this checkpoint.", 0 );
    List.PropEnumGuid    ( "DebugSkip Trigger", "This GUID should point to the trigger that is to be executed when you skip to checkpoint from the debug menu (happens before Restoration trigger and used primarily for player inventory changes).", 0 );

    //object info
    actions_ex_base::OnEnumProp( List );

    x_catch_display;
}

//=============================================================================

xbool action_checkpoint::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID( "Restore Trigger", m_TriggerGuidRestore ) )
    {
        return TRUE;
    }

    if ( rPropQuery.VarGUID( "DebugSkip Trigger", m_TriggerGuidDebugSkip ) )
    {
        return TRUE;
    }

    if( rPropQuery.IsVar( "String Table" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_CPTableName >= 0 )
                rPropQuery.SetVarEnum( g_StringMgr.GetString( m_CPTableName ) );
            else
                rPropQuery.SetVarEnum( "" );
        }
        else
        {

            m_CPTableName = g_StringMgr.Add( rPropQuery.GetVarEnum() );
        }

        return TRUE;
    }
    
    if( rPropQuery.IsVar( "Checkpoint Name" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_CPTitleName >= 0 )
            {
                const char* pString = g_StringMgr.GetString( m_CPTitleName );
                rPropQuery.SetVarString( pString, x_strlen( pString )+1 );
            }
            else
            {
                rPropQuery.SetVarString( "", 1 );
            }
        }
        else
        {
            m_CPTitleName = g_StringMgr.Add( rPropQuery.GetVarString() );
        }

        return TRUE;
    }

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;

}

//=============================================================================

const char* action_checkpoint::GetDescription( void )
{
    return "Checkpoint!";
}

//=============================================================================


