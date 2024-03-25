///////////////////////////////////////////////////////////////////////////
//
//  Action Display Text.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_display_text.hpp"

#include "Entropy.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Objects\HudObject.hpp"
#include "Dictionary\Global_Dictionary.hpp"
#include "GameTextMgr\GameTextMgr.hpp"

static const xcolor s_LogicColor          (100,100,0);

//=========================================================================
// MUSIC_INTENSITY
//=========================================================================

action_display_text::action_display_text ( guid ParentGuid ) : actions_ex_base( ParentGuid )
{
    m_TableName = -1;
    m_TitleName = -1;
    m_Objective = FALSE;
    m_RenderNow = TRUE;
}

//=============================================================================

xbool action_display_text::Execute ( f32 DeltaTime )
{
    (void)DeltaTime;    
             
    if( (m_TitleName >= 0) && (m_TableName >= 0) )
    {
        if( m_RenderNow )        
            g_GameTextMgr.DisplayMessage( g_StringMgr.GetString( m_TableName ), g_StringMgr.GetString( m_TitleName ) );   

        if( m_Objective )
        {
            // setup in HudObject that this is the current objective.
            slot_id SlotID  = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

            if( SlotID != SLOT_NULL )
            {
                object* pObj    = g_ObjMgr.GetObjectBySlot( SlotID );
                hud_object& Hud = hud_object::GetSafeType( *pObj );

                //if( !Hud.m_Initialized )
                //{
                //    //return FALSE;
                //    m_bErrorInExecute = TRUE;
                //   return (!RetryOnError());
                //}

                Hud.SetObjectiveText(m_TableName, m_TitleName );
            }
        }
    }

    return TRUE;
}

//=============================================================================

void action_display_text::OnEnumProp ( prop_enum& List )
{
    x_try;

        xstring TableString;
        TableString.Clear();

        slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_HUD_OBJECT );

        if( SlotID == SLOT_NULL )
           x_throw( "No Hud Object specified in this project." );

        hud_object& Hud = hud_object::GetSafeType( *g_ObjMgr.GetObjectBySlot( SlotID ) );
        Hud.GetBinaryResourceName(TableString);

        List.PropEnumEnum    ( "Table Name", (const char*)TableString, "Which table to get the string from.", 0 );
        List.PropEnumString  ( "Title Name", "The name of the title from which to get the string out of.", 0 );
        List.PropEnumBool    ( "Objective",  "Is this string our current objective?",0 );
        List.PropEnumBool    ( "Render",     "When set do you want it to be rendered to the text box?",0 );

        //object info
        actions_ex_base::OnEnumProp( List );
    
    x_catch_display;
}

//=============================================================================

xbool action_display_text::OnProperty ( prop_query& rPropQuery )
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
    
    if( rPropQuery.IsVar( "Title Name" ) )
    {
        if( rPropQuery.IsRead() )
        {
            if( m_TitleName >= 0 )
            {
                const char* pString = g_StringMgr.GetString( m_TitleName );
                rPropQuery.SetVarString( pString, x_strlen( pString )+1 );
            }
            else
            {
                rPropQuery.SetVarString( "", 1 );
            }
        }
        else
        {
            m_TitleName = g_StringMgr.Add( rPropQuery.GetVarString() );
        }

        return TRUE;
    }

    if( rPropQuery.VarBool( "Objective", m_Objective ) )
        return TRUE;

    if( rPropQuery.VarBool( "Render", m_RenderNow) )
        return TRUE;


    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;

}

//=============================================================================

const char* action_display_text::GetDescription( void )
{
    static big_string   Info;
    Info.Set("Display the text from the localized tables.");
    return Info.Get();
}

//=============================================================================


