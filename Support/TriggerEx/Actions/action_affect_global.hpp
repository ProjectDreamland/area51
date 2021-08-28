///////////////////////////////////////////////////////////////////////////////
//
//  action_affect_global.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _action_affect_global_
#define _action_affect_global_

//=========================================================================
// INCLUDES
//=========================================================================

#include "..\TriggerEx_Actions.hpp"
#include "Dictionary\global_dictionary.hpp"

//=========================== ==============================================
// action_affect_global
//=========================================================================

class action_affect_global : public actions_ex_base
{
public:
                    action_affect_global                ( guid ParentGuid );

    virtual         action_ex_types     GetType         ( void )    { return GetTypeStatic();}
    static          action_ex_types     GetTypeStatic   ( void )    { return TYPE_ACTION_AFFECT_GLOBAL_VAR;}
    virtual         const char*         GetTypeInfo     ( void )    { return "Alter a Global Variable."; } 
    virtual         const char*         GetDescription  ( void );

    virtual         xbool               Execute         ( f32 DeltaTime );    
    virtual			void	            OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	            OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         s32*                GetGlobalRef    ( xstring& Desc ) { Desc = "Global variable error: "; return &m_GlobalIndex; }
#endif

#ifndef X_RETAIL
    virtual         void                OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:
    
                    s32                 GetVariableType ( void );
                    
    enum variable_codes
    { 
        INVALID_VAR_CODES = -1,
        VAR_CODE_ADD,
        VAR_CODE_SUBTRACT,
        VAR_CODE_SET,
        VAR_CODE_RESET_TIMER,
        VAR_CODE_START_TIMER,
        VAR_CODE_STOP_TIMER,
        VAR_CODES_END
    };

    static enum_table<variable_codes>     s_VarCodeEnum;      // Enumeration of the code types..
    static enum_table<variable_codes>     s_TimerCodeEnum;    // Enumeration of the code types..

    s32               m_VarRaw;             // Raw data of variable is cast into proper type (f32, u32, s32, xbool) at runtime...
    guid              m_VarGuid;
    s32               m_Code;               //Action code
    s32               m_GlobalIndex;    
};


#endif
