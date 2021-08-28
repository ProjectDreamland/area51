#ifndef SKINGEOM_DATA_HPP
#define SKINGEOM_DATA_HPP

#include "GeomDesc.hpp"

class skingeom_rsc_desc : public geom_rsc_desc
{
public:
                        skingeom_rsc_desc           ( void );
    virtual void        OnCheckIntegrity            ( void );
    virtual void        OnEnumProp                  ( prop_enum& List        );
    virtual xbool       OnProperty                  ( prop_query& I          );
    virtual void        OnGetCompilerDependencies   ( xarray<xstring>& List  );
    virtual void        OnGetCompilerRules          ( xstring& CompilerRules );
    virtual void        OnGetFinalDependencies      ( xarray<xstring>& List, platform Platform, const char* pDirectory );
};

#endif