///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_PLAY_SCRIPT
#define _TRIGGER_ACTIONS_PLAY_SCRIPT

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// PLAY_SCRIPT : plays a script
//=========================================================================

class play_script : public actions_base
{
public:
                    play_script                     ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Play the Script"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Players a script."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_ACTIVATE_OBJECT;}

protected:
    
    s32     m_ScriptID;             //Script ID to execute
};


#endif
