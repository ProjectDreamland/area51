///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_CHANGE_OBJECT_HEALTH
#define _TRIGGER_ACTIONS_CHANGE_OBJECT_HEALTH

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================== ==============================================
// change_object_health
//=========================================================================

class change_object_health : public actions_base
{
public:
                    change_object_health            ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Change Object Health"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Changes an objects health."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender        ( void );
    
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_CHANGE_OBJECT_HEALTH;}

protected:
    
    f32     m_Health;                               //Health to change by.
    guid    m_ObjectGuid;                           //GUID of object to call OnActivate for
};


#endif
