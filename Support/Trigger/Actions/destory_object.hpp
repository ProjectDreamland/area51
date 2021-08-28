///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_DESTORY_OBJECT
#define _TRIGGER_ACTIONS_DESTORY_OBJECT

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// DESTORY_OBJECT : destroy a specific object
//=========================================================================

class destory_object : public actions_base
{
public:
                    destory_object                  ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Destroy an Object"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Destroys an object."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_DESTORY_OBJECT;}

protected:
    
    guid    m_ObjectGuid;          //Guid of object to destory...
};

#endif
