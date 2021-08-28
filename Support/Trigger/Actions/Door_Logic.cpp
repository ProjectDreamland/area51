///////////////////////////////////////////////////////////////////////////
//
//  door_logic.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\door_logic.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "Entropy.hpp"

static const xcolor s_DoorLogicColor             (0,255,0);

//=========================================================================
// OPEN_AND_LOCK_DOOR
//=========================================================================

door_logic::door_logic ( guid ParentGuid ) : 
    actions_base        ( ParentGuid ),
    m_DoorObjectGuid    (NULL),
    m_State             (door::CLOSED),
    m_Logic             (TRUE)
{
}

//=============================================================================

void door_logic::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * move_object::Execute" );

    (void) pParent;


    if( m_DoorObjectGuid )
    {
        object* pObj = g_ObjMgr.GetObjectByGuid( m_DoorObjectGuid );
        if( pObj )
        {
            door& Door = door::GetSafeType( *pObj );
            Door.SetTargetState( m_State );
            Door.OverRideLogic( m_Logic );
        }
    }
/*
    object_ptr<door>  DoorPtr( m_DoorObjectGuid );
    
    if ( !DoorPtr.IsValid() )
    {
        return;
    }
*/
//    DoorPtr.m_pObject->TriggerSetState( door::DOOR_CLOSING_LOCKED );
}

//=============================================================================

void door_logic::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_DoorObjectGuid);
    
    if ( !ObjectPtr.IsValid() )
        return;

#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_DoorLogicColor );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_DoorLogicColor );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_DoorLogicColor, GetTypeName() );
#endif
}

//=============================================================================

void door_logic::OnEnumProp	( prop_enum& rPropList )
{
    //object info 
    rPropList.AddGuid   ( "Door Guid" , "GUID of the door that you want to open and lock" );

    rPropList.AddEnum   ( "Target State", 
                          "CLOSED\0LOCKED\0OPEN\0", 
                          "The target state for the door ");
    
    rPropList.AddBool   ( "Run Door Logic", "Let the door run its logic" );

    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool door_logic::OnProperty	( prop_query& rPropQuery )
{
    if ( rPropQuery.VarGUID ( "Door Guid"  , m_DoorObjectGuid ) )
        return TRUE;

    // The Initial state.
    if( rPropQuery.IsVar( "Target State" ) )
    {
        if( rPropQuery.IsRead () )
        {
            switch( m_State )
            {
                case door::CLOSED     : rPropQuery.SetVarEnum( "CLOSED" ); break;
                case door::LOCKED     : rPropQuery.SetVarEnum( "LOCKED" ); break;
                case door::OPEN       : rPropQuery.SetVarEnum( "OPEN" ); break;
                default:    
                {
                ASSERTS( FALSE, "Didn't set the type"  );
                }
            } 
        }
        else
        {
            if( !x_stricmp( "CLOSED", rPropQuery.GetVarEnum()) )
            {
                m_State = door::CLOSED;
            }
            else if( !x_stricmp( "LOCKED", rPropQuery.GetVarEnum() ) )
            {
                m_State = door::LOCKED;
            }
            else if( !x_stricmp( "OPEN", rPropQuery.GetVarEnum() ) )
            {
                m_State = door::OPEN;
            }
        }

        return TRUE;
    }
    
    if( rPropQuery.VarBool( "Run Door Logic", m_Logic ) )
        return TRUE;


    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}