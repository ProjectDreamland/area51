///////////////////////////////////////////////////////////////////////////////
//
//  safe_spot_trigger.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _safe_spot_trigger_hpp
#define _safe_spot_trigger_hpp

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// safe_spot_trigger : When hit, marks this as the last safe spot
//=========================================================================

class safe_spot_trigger : public actions_base
{
public:
                    safe_spot_trigger               ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Safe Spot"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "When hit, marks this as the last safe spot"; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_SAFE_SPOT_TRIGGER;}

protected:
    
    
};

#endif
