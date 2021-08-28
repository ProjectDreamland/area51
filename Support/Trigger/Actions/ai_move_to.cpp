///////////////////////////////////////////////////////////////////////////
//
//  ai_move_to.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\ai_move_to.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"
#include "..\Support\Characters\Character.hpp"

#include "Entropy.hpp"

static const xcolor s_AIColor               (0,255,255);

//=========================================================================
// AI_MOVE_TO
//=========================================================================

ai_move_to::ai_move_to ( guid ParentGuid ) : actions_base( ParentGuid ),
m_AIGuid(NULL),
m_BeginState(0),
m_EndState(0),
m_Desintation(0.0f,0.0f,0.0f),
m_MoveStyle( 0 ),
m_bOverrideAI( FALSE ),
m_ModifyGroup( ONE_NPC )
{
}

//=============================================================================

void ai_move_to::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * ai_move_to::Execute" );
      
    (void) pParent;
    guid    ToTriggerGuid = NULL;

    switch ( m_ModifyGroup )
    {
        case ONE_NPC:
        {
            if (m_AIGuid == NULL)
                return;
            ToTriggerGuid = m_AIGuid;       
            break;
        }

/*
        case NPC_CLASS:
        {
            if ( pParent->GetTriggerActor() == NULL )
                return;

            // Need some logic here to determine the class of the character.
            break;
        }
*/

        case ALL_NPC:
        {
            if ( pParent->GetTriggerActor() == NULL )
            return;

            ToTriggerGuid = *pParent->GetTriggerActor();
            break;
        }

        default:
            break;
    }


    object_ptr<character> ObjectPtr(ToTriggerGuid);

    if (!ObjectPtr.IsValid())
        return;

//    ObjectPtr.m_pObject->OnTrigger( m_Desintation, (loco::move_style) m_MoveStyle, (states) m_EndState, m_bOverrideAI );
    
/*
    ObjectPtr.m_pObject->OnTrigger( 
        (character::states) m_BeginState, 
        (character::states) m_EndState, 
        m_Desintation );
*/
}

//=============================================================================

void ai_move_to::OnRender ( void )
{
    object_ptr<object> ObjectPtr(m_AIGuid);
    
    if ( !ObjectPtr.IsValid() )
        return;
#ifdef TARGET_PC
    vector3 MyPosition =  GetPositionOwner() + SMP_UTIL_RandomVector(k_rand_draw_displace_amt);
    draw_Line( MyPosition, ObjectPtr.m_pObject->GetPosition(), s_AIColor );
    draw_BBox( ObjectPtr.m_pObject->GetBBox(), s_AIColor );
    draw_Label( ObjectPtr.m_pObject->GetPosition(), s_AIColor, GetTypeName() );
#endif
}

//=============================================================================

void ai_move_to::OnEnumProp ( prop_enum& rPropList )
{    
//  rPropList.AddEnum( "Who To Move", "Specifiecd NPC\0NPC Class\0All NPCs\0",
//                      "Tells which NPCs this action moves", PROP_TYPE_MUST_ENUM|PROP_TYPE_DONT_SAVE );

    rPropList.AddEnum( "Who To Move", "Specifiecd NPC\0All NPCs\0",
                       "Tells which NPCs this action moves", PROP_TYPE_MUST_ENUM );
        
    //object info
    rPropList.AddGuid ( "AI Guid" , "Guid of the AI state to modify." );
     
    rPropList.AddEnum ( "Begin State" , character::GetStatesEnum(), "State of the AI to be changed too at the start of the move." );
    
    rPropList.AddEnum ( "End State" ,   character::GetStatesEnum(), "State of the AI to be changed too at the end of the move." );
    
    rPropList.AddVector3 ( "Destination" ,  "End position of the move." );

    rPropList.AddEnum ( "Move Style", loco::GetMoveStyleEnum(), "Move style that the AI uses when traveling" ) ;

    rPropList.AddBool ( "Override AI", "Set this to override AI while moving.  Be careful." ) ;
    
    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool ai_move_to::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.IsVar( "Who To Move"  ) )
    {
        if( rPropQuery.IsRead() )
        {
            switch ( m_ModifyGroup )
            {
            case ONE_NPC:          
                rPropQuery.SetVarEnum( "Specifiecd NPC" );                    
                break;
            case NPC_CLASS:           
                rPropQuery.SetVarEnum( "NPC Class" );                   
                break;
            case ALL_NPC:
                rPropQuery.SetVarEnum( "All NPCs" );
                break;
              

            default:
                ASSERT(0);
                break;
            }
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            if( x_stricmp( pString, "One NPC" ) ==0 ) 
            {                
                m_ModifyGroup = ONE_NPC;
            }
            if( x_stricmp( pString, "NPC Class" )==0)
            {
                m_ModifyGroup = NPC_CLASS;
            }
            if ( x_stricmp( pString, "All NPCs" ) == 0 )
            {
                m_ModifyGroup = ALL_NPC;
            }
            
            return( TRUE );
        }
    }

    if ( rPropQuery.VarGUID ( "AI Guid"  , m_AIGuid ) )
    {
        return TRUE;
    }
    
    if ( rPropQuery.VarVector3 ( "Destination"  , m_Desintation ) )
        return TRUE;

    if ( rPropQuery.IsVar( "Begin State" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( character::GetStateName(m_BeginState) ); 
          
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();

            m_BeginState = (s32) character::GetStateByName  ( pString) ;

            return( TRUE );
        }
    }
    
    if ( rPropQuery.IsVar( "End State" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( character::GetStateName(m_EndState) ); 
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            m_EndState = (s32) character::GetStateByName  ( pString) ;
            
            return( TRUE );
        }
    }

    if ( rPropQuery.IsVar( "Move Style" ) ) 
    {
        if( rPropQuery.IsRead() )
        {
            rPropQuery.SetVarEnum( loco::GetMoveStyleName( m_MoveStyle ) ); 
            
            return( TRUE );
        }
        else
        {
            const char* pString = rPropQuery.GetVarEnum();
            
            m_MoveStyle = (s32) loco::GetMoveStyleByName  ( pString) ;
            
            return( TRUE );
        }
    }

    if ( rPropQuery.VarBool( "Override AI" , m_bOverrideAI ) )
        return TRUE ;

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;

    return FALSE;
}



