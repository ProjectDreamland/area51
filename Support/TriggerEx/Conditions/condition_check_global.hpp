///////////////////////////////////////////////////////////////////////////////
//
//  condition_check_global.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_check_global_
#define _condition_check_global_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\TriggerEx_Conditionals.hpp"
#include "MiscUtils\SimpleUtils.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_check_global : public conditional_ex_base
{
public:

    enum codes
    { 
        INVALID_CVAR_CODES = -1,
        CVAR_CODE_EQUAL,
        CVAR_CODE_NOT_EQUAL,
        CVAR_CODE_GREATER_INCLUSIVE,
        CVAR_CODE_LESSER_INCLUSIVE,
        CVAR_CODE_GREATER,
        CVAR_CODE_LESSER,
        CVAR_CODES_END
    };

                    condition_check_global                  ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_CHECK_GLOBAL;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Check a globals value."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         s32*                    GetGlobalRef    ( xstring& Desc ) { Desc = "Global variable error: "; return &m_GlobalIndex; }
#endif
        
protected:
    
                    s32                     GetVariableType ( void );
 
    static enum_table<codes>    m_CodeTableFull;       // Enumeration of the op types..
    static enum_table<codes>    m_CodeTableSmall;      // Enumeration of the op types..

    s32                         m_Code;                                     // Used to determine what type of conditional check to perform
    s32                         m_GlobalIndex;                              // Name of the variable
    s32                         m_VarRaw;                                   // Raw data of variable is cast into proper type (f32, u32, s32, xbool) at runtime...
    guid                        m_VarGuid;
};

#endif
