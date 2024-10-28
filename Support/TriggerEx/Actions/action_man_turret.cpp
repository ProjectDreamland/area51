///////////////////////////////////////////////////////////////////////////////
//
//  action_man_turret.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_man_turret.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "objects\player.hpp"
#include "objects\turret.hpp"

static const xcolor s_LogicColor          (200,200,0);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_man_turret::action_man_turret( guid ParentGuid ):  
actions_ex_base     ( ParentGuid    ),
m_TurretObject      ( NULL_GUID     )
{
}

//=============================================================================

xbool action_man_turret::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    player* pPlayerObj = SMP_UTIL_GetActivePlayer();
    if ( pPlayerObj )
    {
        guid LeftBoundary  = NULL_GUID;
        guid RightBoundary = NULL_GUID;
        guid LowerBoundary = NULL_GUID;
        guid UpperBoundary = NULL_GUID;

        object* pObj = g_ObjMgr.GetObjectByGuid( m_TurretObject );
        if ( pObj && pObj->IsKindOf( turret::GetRTTI() ) )
        {
            turret* pTurret = (turret*)pObj;
            pTurret->GetMannedBoundaryGuids( LeftBoundary, RightBoundary, UpperBoundary, LowerBoundary );
        }

        pPlayerObj->ManTurret( m_TurretObject, m_TurretObject2, m_TurretObject3, m_AnchorObject, LeftBoundary, RightBoundary, UpperBoundary, LowerBoundary );
        return TRUE;
    }

    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}    

//=============================================================================

void action_man_turret::OnEnumProp ( prop_enum& rPropList )
{ 
    rPropList.PropEnumGuid      ( "Turret Object",          "Turret we're manning, which will stop the turret's rendering while the player is manning it (different turret to render).", 0 );
    rPropList.PropEnumGuid      ( "Turret Object 2",        "Part of turret we're manning, which will stop the object's rendering while the player is manning it.", 0 );
    rPropList.PropEnumGuid      ( "Turret Object 3",        "Part of turret we're manning, which will stop the object's rendering while the player is manning it.", 0 );
    rPropList.PropEnumGuid      ( "Anchor Object",          "This object's position and rotation will be given to the player, and is where he'll plant.", 0 );
    rPropList.PropEnumGuid      ( "Left Boundary",          "No aiming left of this object. The position of this object will represent the left boundary for horizontal rotation of the turret.", 0 );
    rPropList.PropEnumGuid      ( "Right Boundary",         "No aiming right of this object. The position of this object will represent the right boundary for horizontal rotation of the turret.", 0 );
    rPropList.PropEnumGuid      ( "Upper Boundary",         "No aiming above this object. The position of this object will represent the upper boundary for horizontal rotation of the turret.", 0 );
    rPropList.PropEnumGuid      ( "Lower Boundary",         "No aiming below this object. The position of this object will represent the lower boundary for horizontal rotation of the turret.", 0 );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_man_turret::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID( "Turret Object" ,   m_TurretObject  ) )
        return TRUE;

    if ( rPropQuery.VarGUID( "Turret Object 2" , m_TurretObject2 ) )
        return TRUE;

    if ( rPropQuery.VarGUID( "Turret Object 3" , m_TurretObject3 ) )
        return TRUE;

    if ( rPropQuery.VarGUID( "Anchor Object" ,   m_AnchorObject  ) )
        return TRUE;

    if(  rPropQuery.VarGUID( "Left Boundary",     m_LeftBoundary  ) )
        return TRUE;

    if(  rPropQuery.VarGUID( "Right Boundary",    m_RightBoundary ) )
        return TRUE;

    if(  rPropQuery.VarGUID( "Upper Boundary",    m_UpperBoundary ) )
        return TRUE;

    if(  rPropQuery.VarGUID( "Lower Boundary",    m_LowerBoundary ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_man_turret::GetDescription( void )
{
    static big_string   Info;
    Info.Set("Man Turret");
    return Info.Get();
}

//=============================================================================
