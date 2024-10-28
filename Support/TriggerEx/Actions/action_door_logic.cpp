///////////////////////////////////////////////////////////////////////////////
//
//  action_door_logic.cpp
//
///////////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "action_door_logic.hpp"
#include "..\xcore\auxiliary\MiscUtils\Property.hpp"
#include "MiscUtils\SimpleUtils.hpp"
#include "Entropy.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"

static const xcolor s_LogicColor          (200,200,0);

//=========================================================================
// CLASS FUNCTIONS
//=========================================================================

action_door_logic::action_door_logic( guid ParentGuid ):  actions_ex_base(  ParentGuid ),
m_State(door::CLOSED),
m_Logic(TRUE)
{
}

//=============================================================================

xbool action_door_logic::Execute( f32 DeltaTime )
{    
    (void) DeltaTime;

    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if( pObject && pObject->IsKindOf( door::GetRTTI() ) == TRUE )
    {
        door& Door = door::GetSafeType( *pObject );
        Door.SetTargetState( m_State );
        Door.OverRideLogic( m_Logic );
        return TRUE;
    }
    else
    {
        x_DebugMsg("action_door_logic::Execute failed!\n");
    }

    m_bErrorInExecute = TRUE;
    return (!RetryOnError());
}    

//=============================================================================

#ifndef X_RETAIL
void action_door_logic::OnDebugRender ( s32 Index )
{
    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if( pObject && pObject->IsKindOf( door::GetRTTI() ) == TRUE )
    {
        draw_Line( GetPositionOwner(), pObject->GetPosition(), s_LogicColor );
        draw_BBox( pObject->GetBBox(), s_LogicColor );
        if (!GetElse())
        {
            draw_Label( pObject->GetPosition(), s_LogicColor, xfs("[%d]Door Logic", Index) );
        }
        else
        {
            draw_Label( pObject->GetPosition(), s_LogicColor, xfs("[Else %d]Door Logic", Index) );
        }
    }
}
#endif // X_RETAIL

//=============================================================================

void action_door_logic::OnEnumProp ( prop_enum& rPropList )
{ 
    //guid specific fields
    m_ObjectAffecter.OnEnumProp( rPropList, "Door" );

    rPropList.PropEnumEnum   ( "Target State", "Closed\0Locked\0Open\0", "The target state for the door ", PROP_TYPE_MUST_ENUM);
    rPropList.PropEnumBool   ( "Run Logic", "Let the door run its logic", 0 );

    actions_ex_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool action_door_logic::OnProperty ( prop_query& rPropQuery )
{
    //guid specific fields
    if( m_ObjectAffecter.OnProperty( rPropQuery, "Door" ) )
        return TRUE;

    // The Initial state.
    if( rPropQuery.IsVar( "Target State" ) )
    {
        if( rPropQuery.IsRead () )
        {
            switch( m_State )
            {
                case door::CLOSED     : rPropQuery.SetVarEnum( "Closed" ); break;
                case door::LOCKED     : rPropQuery.SetVarEnum( "Locked" ); break;
                case door::OPEN       : rPropQuery.SetVarEnum( "Open" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "Closed", rPropQuery.GetVarEnum()) )
            {
                m_State = door::CLOSED;
            }
            else if( !x_stricmp( "Locked", rPropQuery.GetVarEnum() ) )
            {
                m_State = door::LOCKED;
            }
            else if( !x_stricmp( "Open", rPropQuery.GetVarEnum() ) )
            {
                m_State = door::OPEN;
            }
        }

        return TRUE;
    }
    
    if( rPropQuery.VarBool( "Run Logic", m_Logic ) )
        return TRUE;

    if( actions_ex_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}

//=============================================================================

const char* action_door_logic::GetDescription( void )
{
    static big_string   Info;

    object* pObject = m_ObjectAffecter.GetObjectPtr();
    if( pObject  )
    {
        if (pObject->IsKindOf( door::GetRTTI() ) == FALSE)
        {
            //we've got an object but its not a door
            Info.Set(xfs("??? Door Logic on %s", m_ObjectAffecter.GetObjectInfo()));
        }
        else
        {
            switch( m_State )
            {
                case door::CLOSED: Info.Set("Close Door"); break;
                case door::LOCKED: Info.Set("Lock Door");  break;
                case door::OPEN:   Info.Set("Open Door");  break;
                default:           Info.Set("Door Logic"); break;
            } 
        }
    }
    else
    {
        switch( m_State )
        {
            case door::CLOSED: 
                Info.Set(xfs("Door Close on %s", m_ObjectAffecter.GetObjectInfo()));
                break;
            case door::LOCKED: 
                Info.Set(xfs("Door Lock on %s", m_ObjectAffecter.GetObjectInfo()));
                break;
            case door::OPEN: 
                Info.Set(xfs("Door Open on %s", m_ObjectAffecter.GetObjectInfo()));
                break;
            default: 
                Info.Set(xfs("Door Logic on %s", m_ObjectAffecter.GetObjectInfo()));
                break;
        } 
    }

    return Info.Get();
}
