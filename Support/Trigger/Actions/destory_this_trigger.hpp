///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_DESTORY_THIS_TRIGGER
#define _TRIGGER_ACTIONS_DESTORY_THIS_TRIGGER

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// DESTORY_THIS_TRIGGER : modifies a timer
//=========================================================================

class destory_this_trigger : public actions_base
{
public:
                    destory_this_trigger            ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Destroy This Trigger"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Destroys this trigger."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_ACTIVATE_OBJECT;}
};

#endif
