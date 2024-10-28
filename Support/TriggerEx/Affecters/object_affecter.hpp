///////////////////////////////////////////////////////////////////////////////
//
//  object_affecter.hpp
//
///////////////////////////////////////////////////////////////////////////////

#ifndef _object_affecter_
#define _object_affecter_

//=========================================================================
// INCLUDES
//=========================================================================

#include "MiscUtils\SimpleUtils.hpp"

//=========================================================================
// Check Property
//=========================================================================

class object_affecter
{
public:
    
    enum guid_codes
    { 
        OBJECT_GUID_CODES = -1,
        OBJECT_CODE_CHECK_BY_STATIC_GUID,
        OBJECT_CODE_CHECK_BY_GLOBAL_GUID,
        OBJECT_GUID_CODES_END
    };

                    object_affecter                         ( void );

    virtual			void	                OnEnumProp	    ( prop_enum& rList, const char* pPropName, u32 Flags = 0);
    virtual			xbool	                OnProperty	    ( prop_query& rPropQuery, const char* pPropName );

                    object*                 GetObjectPtr    ( void );
                    const char*             GetObjectInfo   ( void );
                    
    inline          guid_codes              GetObjectCode   ( void ) { return (guid_codes)m_GuidCode; }
                    void                    SetStaticGuid   ( guid Guid );

                    guid                    GetGuid         ( void );
    inline          s32                     GetGuidVarName  ( void ) { return m_GuidVarName;   }
    inline          xhandle                 GetGuidVarHandle( void ) { return m_GuidVarHandle; }

protected:
    

    s32             m_GuidVarName;      // Name of the variable
    xhandle         m_GuidVarHandle;    // Global variable handle..
    guid            m_ObjectGuid;       // Object ID to see if an object exist...
    s32             m_GuidCode;         // Used to determine what type of conditional check to perform
static big_string   m_Description;
};

#endif
