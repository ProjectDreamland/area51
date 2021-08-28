///////////////////////////////////////////////////////////////////////////////
//
//  Door_Logic.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef TRIGGER_ACTIONS_DOOR_LOGIC
#define TRIGGER_ACTIONS_DOOR_LOGIC

//=========================================================================
// INCLUDES
//=========================================================================
 
#include "..\Support\Trigger\Trigger_Actions.hpp"
#include "..\Support\Objects\Door.hpp "

//=========================================================================
// MOVE_OBJECT
//=========================================================================

class door_logic : public actions_base
{
public:
                    door_logic                      ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Door Logic"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Tells door what state to go to and if it needs to run its logic"; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_DOOR_LOGIC;}

protected:
    
    guid            m_DoorObjectGuid;            //GUID of affected object..
    door::state     m_State;
    xbool           m_Logic;
};


#endif
