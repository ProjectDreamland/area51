///////////////////////////////////////////////////////////////////////////////
//
//  Trigger_Actions.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _TRIGGER_ACTIONS_SET_TIMER
#define _TRIGGER_ACTIONS_SET_TIMER

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\Support\Trigger\Trigger_Actions.hpp"

//=========================================================================
// SET_TIMER : modifies a timer
//=========================================================================

class set_timer : public actions_base
{
public:
    
    enum codes
    { 
        INVALID_CODES = -1,
            
            CODE_RESET_TIMER,
            CODE_START_TIMER,
            CODE_STOP_TIMER,

            CODES_END
    };

public:
                    set_timer                       ( guid ParentGuid );

    virtual         const char*         GetTypeName ( void )    { return "Set Timer"; } 
    virtual         const char*         GetTypeInfo ( void )    { return "Operates upon a global timer."; } 
    virtual         void                Execute     ( trigger_object* pParent );    
    virtual			void	            OnEnumProp	( prop_enum& rList );
    virtual			xbool	            OnProperty	( prop_query& rPropQuery );

    virtual         void                OnRender    ( void );

   
    virtual         action_types        GetType         ( void ) { return GetTypeStatic();}
    static          action_types        GetTypeStatic   ( void ) { return TYPE_ACTION_SET_TIMER;}

protected:
 
    sml_string        m_TimerName;          //Name of Timer to operate upon
    xhandle           m_TimerHandle;        //Hnadle to the timer
    s32               m_Code;               //Action code

protected:
    
    static enum_table<codes>     s_CodeEnum;      // Enumeration of the code types..
};

#endif
