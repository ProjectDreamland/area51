///////////////////////////////////////////////////////////////////////////////
//
//  close_and_lock_door.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_RETURN_DOOR_TO_NORMAL_
#define _TRIGGER_ACTIONS_RETURN_DOOR_TO_NORMAL_

//=========================================================================
// INCLUDES
//=========================================================================
 
#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// MOVE_OBJECT
//=========================================================================

class return_door_to_normal : public actions_base
{
public:
                    return_door_to_normal           ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Unlocks door"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Unlocks door that has been triggered to lock."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_RESTORE_DOOR;}

protected:
    
    guid    m_DoorObjectGuid;            //GUID of affected object..
};


#endif
