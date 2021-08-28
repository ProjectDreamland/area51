//==============================================================================
//
//  condition_line_of_sight.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  define HERE
//
//==============================================================================
#ifndef __CONDITION_LINE_OF_SIGHT__ 
#define __CONDITION_LINE_OF_SIGHT__ 

//==------------------------------------------------------------------------
// Includes
//==------------------------------------------------------------------------
#include "..\TriggerEx_Conditionals.hpp"
#include "..\Affecters\object_affecter.hpp"

//==------------------------------------------------------------------------
// Defines
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Globals
//==------------------------------------------------------------------------

//==------------------------------------------------------------------------
// Prototypes
//==------------------------------------------------------------------------


// Class condition_line_of_sight
class condition_line_of_sight : public conditional_ex_base
{

public:
	condition_line_of_sight                 ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_LINE_OF_SIGHT;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Check line of sight ( PROGRAMMER USE ONLY!!!! )."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*        GetObjectRef0   ( xstring& Desc ) { Desc = "Object1 error: "; return &m_Affecter1; }
    virtual         object_affecter*        GetObjectRef1   ( xstring& Desc ) { Desc = "Object2 error: "; return &m_Affecter2; }
#endif

#ifndef X_RETAIL
    virtual         void                    OnDebugRender   ( s32 Index );
#endif // X_RETAIL

protected:

    enum check_codes
    { 
        INVALID_CHECK_CODES = -1,
        CODE_CAN_SEE_OBJECT,
        CODE_CANT_SEE_OBJECT,
        CHECK_CODES_END
    };

    s32              m_CheckCode;        // Used to determine what type of conditional check to perform
    object_affecter  m_Affecter1;
    object_affecter  m_Affecter2;
    xbool            m_CanSee;
};


#endif // CONDITION_LINE_OF_SIGHT
