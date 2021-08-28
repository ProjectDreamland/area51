///////////////////////////////////////////////////////////////////////////
//
//  set_timer.cpp
//
//
///////////////////////////////////////////////////////////////////////////

//=========================================================================
//  INCLUDES
//=========================================================================

#include "..\Support\Trigger\Actions\set_timer.hpp"

#include "..\Support\Trigger\Trigger_Manager.hpp"
#include "..\Support\Trigger\Trigger_Object.hpp"

#include "Entropy.hpp"

//=========================================================================
// SET_TIMER
//=========================================================================

typedef enum_pair<set_timer::codes> action_set_timer_pair;

action_set_timer_pair SetTimerCodeEnum[] = 
{
        action_set_timer_pair("RESET_TIMER",             set_timer::CODE_RESET_TIMER),
        action_set_timer_pair("START_TIMER",             set_timer::CODE_START_TIMER),
        action_set_timer_pair("STOP_TIMER",              set_timer::CODE_STOP_TIMER),
        
        action_set_timer_pair( k_EnumEndStringConst,      set_timer::INVALID_CODES) //**MUST BE LAST**//
};

enum_table<set_timer::codes>  set_timer::s_CodeEnum(SetTimerCodeEnum);

//=============================================================================

set_timer::set_timer ( guid ParentGuid ) : actions_base( ParentGuid ),
m_TimerHandle(HNULL), 
m_Code(CODE_RESET_TIMER)
{
}

//=============================================================================

void set_timer::Execute ( trigger_object* pParent )
{
    TRIGGER_CONTEXT( "ACTION * set_timer::Execute" );

    (void) pParent;
    
    ASSERT( pParent );
   
    //-Get the timmer from the global variable manager
    //-And perform the operation upon the timmer as specificed by the code..

    if ( m_TimerHandle.IsNull() )
        return;

   switch ( m_Code )
   {
   case CODE_RESET_TIMER:
       g_VarMgr.ResetTimer( m_TimerHandle );
       break;
   case CODE_START_TIMER:
       g_VarMgr.StartTimer( m_TimerHandle );
       break;
   case CODE_STOP_TIMER:
       g_VarMgr.StopTimer( m_TimerHandle );
       break;
   default:
       ASSERT(0);
       break;
   };
}

//=============================================================================

void set_timer::OnRender ( void )
{}

//=============================================================================

void set_timer::OnEnumProp ( prop_enum& rPropList )
{  
    rPropList.AddInt    ( "Code" , "",  PROP_TYPE_DONT_SHOW  );
    
    rPropList.AddString ( "Timer Name" , "Name of the timer to operation upon." , PROP_TYPE_MUST_ENUM );

    rPropList.AddEnum   ( "Logic",  s_CodeEnum.BuildString(), "Logic available to this action.", PROP_TYPE_DONT_SAVE  );

    actions_base::OnEnumProp( rPropList );
}

//=============================================================================

xbool set_timer::OnProperty ( prop_query& rPropQuery )
{
    if ( rPropQuery.VarInt ( "Code"  , m_Code ) )
    {
        return TRUE;
    }
    
    if ( rPropQuery.IsVar( "Timer Name" ) ) 
    {
        if( rPropQuery.IsRead() )
        {     
            rPropQuery.SetVarString( m_TimerName.Get() , m_TimerName.MaxLen() );
            
            if ( g_VarMgr.GetTimerHandle( m_TimerName.Get(), &m_TimerHandle) == FALSE )
                m_TimerHandle = HNULL;
        }
        else
        {
            m_TimerName.Set(rPropQuery.GetVarString());
        } 
        
        return( TRUE );
    }
 
    if (m_TimerHandle.IsNonNull())
    {
        if ( SMP_UTIL_IsEnumVar<s32,codes>(rPropQuery, "Logic", m_Code, s_CodeEnum ) )
            return TRUE;
    }
    else
    {
        if ( rPropQuery.IsVar( "Logic" ) ) 
        {
            if( rPropQuery.IsRead() )
            {     
                rPropQuery.SetVarEnum( "INVALID" );
            }
            
            return( TRUE );
        }
    }

    if( actions_base::OnProperty( rPropQuery ) )
        return TRUE;
    
    return FALSE;
}



