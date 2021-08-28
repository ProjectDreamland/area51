///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_DEACTIVATE_OBJECT
#define _TRIGGER_ACTIONS_DEACTIVATE_OBJECT

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// DEACTIVATE_OBJECT
//=========================================================================

class deactivate_object : public actions_base
{
public:
                    deactivate_object               ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Deactivate Object"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Deactivates an object."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_DEACTIVATE_OBJECT;}

protected:
    
    guid    m_ObjectGuid;                               //GUID of object to call OnActivate for
};

#endif
