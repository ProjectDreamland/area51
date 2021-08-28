//=========================================================================
// Font Editor
//=========================================================================

#ifndef FONT_EDITOR_HPP
#define FONT_EDITOR_HPP

#include "..\EDRscDesc\RSCDesc.hpp"

//=========================================================================
// FONT DESC.
//=========================================================================
struct font_desc : public rsc_desc
{
    CREATE_RTTI( font_desc, rsc_desc, rsc_desc )

                        font_desc                   ( void );
    
    virtual void        OnEnumProp                  ( prop_enum&  List          );
    virtual xbool       OnProperty                  ( prop_query& I             );
    virtual void        OnCheckIntegrity            ( void );
    virtual void        OnGetCompilerDependencies   ( xarray<xstring>& List     );
    virtual void        OnGetCompilerRules          ( xstring& CompilerRules    );
    virtual void        OnGetFinalDependencies      ( xarray<xstring>& List, platform Platform, const char* pDirectory );

    char                m_SourceFileName[256];
    char                m_MapFileName[256];
    char                m_format[256];
};

//=========================================================================
// END
//=========================================================================
#endif