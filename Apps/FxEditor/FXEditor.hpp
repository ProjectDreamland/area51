#ifndef DECAL_EDITOR_HPPP
#define DECAL_EDITOR_HPPP

#include "..\EDRscDesc\RSCDesc.hpp"
#include "MeshUtil\RawMesh.hpp"
#include "MeshUtil\RawAnim.hpp"
#include "..\MiscUtils\SimpleUtils.hpp"

//=========================================================================
// ANIMATION DESC.
//=========================================================================
struct fx_desc : public rsc_desc
{
    CREATE_RTTI( fx_desc, rsc_desc, rsc_desc )

                        fx_desc      ( void );
    
    virtual void        OnEnumProp                  ( prop_enum&  List          );
    virtual xbool       OnProperty                  ( prop_query& I             );
    virtual void        OnCheckIntegrity            ( void );
    virtual void        OnGetCompilerDependencies   ( xarray<xstring>& List     );
    virtual void        OnGetCompilerRules          ( xstring& CompilerRules    );
    virtual void        OnGetFinalDependencies      ( xarray<xstring>& List, platform Platform, const char* pDirectory );

   
    big_string   m_FxsFile;
};

//=========================================================================
// END
//=========================================================================
#endif