///////////////////////////////////////////////////////////////////////////////
//
//  condition_check_property.hpp
//
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _condition_check_property_
#define _condition_check_property_

//=========================================================================
// INCLUDES
//=========================================================================

#include "x_types.hpp"
#include "Auxiliary\MiscUtils\PropertyEnum.hpp"
#include "..\TriggerEx_Conditionals.hpp"
#include "..\Affecters\object_affecter.hpp"

//=========================================================================
// Check Property
//=========================================================================

class condition_check_property : public conditional_ex_base
{
public:
    
    enum property_compare_modes
    { 
        INVALID_PCOMP_CODES = -1,
        PCOMP_CODE_EQUAL,
        PCOMP_CODE_NOT_EQUAL,
        PCOMP_CODE_GREATER_INCLUSIVE,
        PCOMP_CODE_LESSER_INCLUSIVE,
        PCOMP_CODE_GREATER,
        PCOMP_CODE_LESSER,
        PCOMP_CODES_END
    };
    
                    condition_check_property                ( conditional_affecter* pParent );

    virtual         conditional_ex_types    GetType         ( void )   { return GetTypeStatic(); }
    static          conditional_ex_types    GetTypeStatic   ( void )   { return TYPE_CONDITION_CHECK_PROPERTY;}
    virtual         const char*             GetTypeInfo     ( void )   { return "Checks an exposed property on an object."; }
    virtual         const char*             GetDescription  ( void );

    virtual         xbool                   Execute         ( guid TriggerGuid );    
    virtual			void	                OnEnumProp	    ( prop_enum& rList );
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery );

#ifdef X_EDITOR
    virtual         object_affecter*        GetObjectRef0   ( xstring& Desc ) { Desc = "Object error: "; return &m_ObjectAffecter; }
    virtual         s32*                    GetPropertyRef  ( xstring& Desc, s32& PropertyType ) { Desc = "Property error: "; PropertyType = m_PropertyType; return &m_PropertyName;  }
#endif


protected:
    
    void            GetExternalPropTag  ( char* pTag );
    void            GetEnumForProperty  ( xstring& strEnum );
    void            SetPropertyType     ( void );
    
    object_affecter m_ObjectAffecter;
    s32             m_VarRaw;             // Raw data of variable is cast into proper type (f32, u32, s32, xbool) at runtime...
    s32             m_RawString;
    s32             m_Code;               // mode code
    s32             m_PropertyName;
    s32             m_PropertyType;
    s32             m_ObjectType;

    static enum_table<property_compare_modes>    m_CodeTableFull;       // Enumeration of the op types..
    static enum_table<property_compare_modes>    m_CodeTableSmall;      // Enumeration of the op types..
};

#endif
