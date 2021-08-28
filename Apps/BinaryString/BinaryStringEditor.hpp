//=========================================================================
// Binary String Editor
//=========================================================================

#ifndef BINARY_STRING_EDITOR_HPP
#define BINARY_STRING_EDITOR_HPP

#include "..\EDRscDesc\RSCDesc.hpp"

//=========================================================================
// BIN STRING DESC.
//=========================================================================
struct binstring_desc : public rsc_desc
{
    CREATE_RTTI( binstring_desc, rsc_desc, rsc_desc )

                        binstring_desc              ( void );
    
    virtual void        OnEnumProp                  ( prop_enum&  List          );
    virtual xbool       OnProperty                  ( prop_query& I             );
    virtual void        OnCheckIntegrity            ( void );
    virtual void        OnGetCompilerDependencies   ( xarray<xstring>& List     );
    virtual void        OnGetCompilerRules          ( xstring& CompilerRules    );
    virtual void        OnGetFinalDependencies      ( xarray<xstring>& List, platform Platform, const char* pDirectory );

    char                m_FileName[256];
};

//=========================================================================
// END
//=========================================================================
#endif