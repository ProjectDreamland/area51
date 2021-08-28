///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_OPEN_LOCK_DOOR_
#define _TRIGGER_ACTIONS_OPEN_LOCK_DOOR_

//=========================================================================
// INCLUDES
//=========================================================================
 
#include "..\Support\Trigger\Trigger_Actions.hpp"
#include "..\Support\Objects\Door.hpp"

//=========================================================================
// MOVE_OBJECT
//=========================================================================

class open_and_lock_door : public actions_base
{
public:
                    open_and_lock_door                     ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Open and lock Door"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Opens and locks a door in the open position."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_OPEN_AND_LOCK_DOOR;}

protected:
    
    guid        m_DoorObjectGuid;            //GUID of affected object..
};


#endif
