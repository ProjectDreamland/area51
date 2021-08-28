///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_ACTIVATE_OBJECT
#define _TRIGGER_ACTIONS_ACTIVATE_OBJECT

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================== ==============================================
// ACTIVATE_OBJECT
//=========================================================================

class activate_object : public actions_base
{
public:
                    activate_object                 ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Activate Object"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Activates an object."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender        ( void );
    
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_ACTIVATE_OBJECT;}

protected:
    
    guid    m_ObjectGuid;                           //GUID of object to call OnActivate for
};


#endif
