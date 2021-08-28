///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_MOVE_OBJECT
#define _TRIGGER_ACTIONS_MOVE_OBJECT

//=========================================================================
// INCLUDES
//=========================================================================

 
#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// MOVE_OBJECT
//=========================================================================

class move_object : public actions_base
{
public:
                    move_object                     ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Move an Object"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Moves an object."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_MOVE_OBJECT;}

protected:
    
    guid        m_ObjectGuid;                       //GUID of affected object..
    vector3     m_Position;                         //Position
    radian3     m_Orientation;                      //Direction
};


#endif
