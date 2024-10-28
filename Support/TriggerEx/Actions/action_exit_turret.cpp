///////////////////////////////////////////////////////////////////////////////
//
//  action_exit_turret.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_exit_turret.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "objects\player.hpp"

static const xcolor s_LogicColor          (200,200,0);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_exit_turret::action_exit_turret( guid ParentGuid ):  
actions_ex_base     ( ParentGuid    )
{
}

//=============================================================================

xbool action_exit_turret::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    player* pPlayerObj = SMP_UTIL_GetActivePlayer();
    if ( pPlayerObj )
    {
        pPlayerObj->ExitTurret();
        return TRUE;
    }

    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}    

//=============================================================================

const char* action_exit_turret::GetDescription( void )
{
    static big_string   Info;
    Info.Set("Exit Turret");
    return Info.Get();
}

//=============================================================================
