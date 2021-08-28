///////////////////////////////////////////////////////////////////////////
//
//  action_save_game.cpp
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_save_game.hpp"
#include "Entropy.hpp"

static const xcolor s_SaveGameColor   (255,255,255);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_save_game::action_save_game ( guid ParentGuid ) : actions_ex_base( ParentGuid )
{
}

//=============================================================================

xbool action_save_game::Execute ( f32 DeltaTime )
{
    (void) DeltaTime;
#ifdef OLD_SAVE
    g_SaveMgr.RequestSave();
#endif
    return TRUE;
}

//=============================================================================

#ifndef X_RETAIL
void action_save_game::OnDebugRender ( s32 Index )
{
    (void)Index;

    draw_Label( GetPositionOwner(), s_SaveGameColor, "Save Game" );
}
#endif // X_RETAIL

//=============================================================================

void action_save_game::OnEnumProp	( prop_enum& rPropList )
{
    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_save_game::OnProperty	( prop_query& rPropQuery )
{    
    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;
  
    return FALSE;
}

//=============================================================================

const char* action_save_game::GetDescription( void )
{
    static big_string   Info;
    Info.Set("Save Game");
    return Info.Get();
}
