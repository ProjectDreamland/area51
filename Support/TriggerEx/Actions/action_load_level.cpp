///////////////////////////////////////////////////////////////////////////
//
//  action_load_level.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_load_level.hpp"
#include "Dictionary\global_dictionary.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "Entropy.hpp"
#include "NetworkMgr\GameMgr.hpp"
#include "StateMgr/StateMgr.hpp"

#ifdef X_EDITOR
extern g_EditorBreakpoint;
#endif

static const xcolor s_LoadLevelColor   (255,255,255);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_load_level::action_load_level ( guid ParentGuid ) : actions_ex_base( ParentGuid ),
m_StorageIndex(-1)
{
}

//=============================================================================

xbool action_load_level::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;
#ifdef X_EDITOR
    g_EditorBreakpoint = TRUE;
#else 
#ifdef LAN_PARTY_BUILD
    if( GameMgr.GetGameType() == GAME_CAMPAIGN )
    {
        // force exit condition
        g_ActiveConfig.SetExitReason( GAME_EXIT_PLAYER_QUIT );
    }
    else
#endif
    {
        GameMgr.EndGame();
    }

#endif // X_EDITOR
    return TRUE;
}

//=============================================================================

#ifndef X_RETAIL
void action_load_level::OnDebugRender ( s32 Index )
{
    (void)Index;

    if (m_StorageIndex >= 0)
    {
        draw_Label( GetPositionOwner(), s_LoadLevelColor, xfs("Load Level %s", g_StringMgr.GetString(m_StorageIndex)) );
    }
}
#endif // X_RETAIL

//=============================================================================

void action_load_level::OnEnumProp	( prop_enum& rPropList )
{
    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_load_level::OnProperty	( prop_query& rPropQuery )
{ 
    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char* action_load_level::GetDescription( void )
{
    static big_string   Info;
    
    if ( m_StorageIndex < 0 )
    {
        Info.Set("Load Unspecified Level");
    }
    else
    {
        Info.Set(xfs("Load \"%s\"", g_StringMgr.GetString( m_StorageIndex )));
    }   

    return Info.Get();
}
