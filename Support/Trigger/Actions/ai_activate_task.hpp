///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_ai_activate_task
#define _TRIGGER_ACTIONS_ai_activate_task

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"


//=========================================================================
// ai_activate_task 
//=========================================================================

class ai_activate_task : public actions_base
{
public:
                    ai_activate_task                 ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Activate Task"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Activates a character task."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& List );
    virtual			xbool	            OnProperty	( prop_query& I );

    virtual         void                OnRender    ( void );
 
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_ACTIVATE_TASK;}

protected:

    guid            m_TaskGuid;
};

#endif
