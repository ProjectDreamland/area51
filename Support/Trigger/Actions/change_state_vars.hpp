///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_CHANGE_STATE_VARS
#define _TRIGGER_ACTIONS_CHANGE_STATE_VARS

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// CHANGE_STATE_VARS : change a global variables vaule by an x amount..
//=========================================================================

class change_state_vars : public actions_base
{
    
public:
                   change_state_vars                 ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )   { return "Change a Global Variable"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Changes a global variable."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );
   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_CHANGE_STATE_VARIABLE;}
        
protected:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_ADD,
            CODE_SUBTRACT,
            CODE_SET,
            
            CODES_END
    };
    
protected:

    s32                         m_Code;                                     // Used to determine what type of conditional check to perform
    sml_string                  m_VariableName;                             // Name of the variable
    xhandle                     m_VarHandle;                                // Gloabal variable handle..
    var_mngr::variable_types    m_Type;                                     // Type of variable...
    s32                         m_VarRaw;                                   // Raw data of variable is cast into proper type (f32, u32, s32, xbool) at runtime...
};

#endif
