/*
#include "BaseStdAfx.h"
#include "../EDRscDesc\RSCDesc.hpp"

//=========================================================================
// TYPE
//=========================================================================
class rigidgeom_rsc_desc : public rsc_desc
{
public:
                        rigidgeom_rsc_desc  ( void );
    virtual void        OnEnumProp          ( prop_enum& List );
    virtual xbool       OnProperty          ( prop_query& I );
    virtual void        OnGetDependencies   ( xarray<xstring>& List    )  {};
    virtual void        OnGetCompilerRules  ( xstring& CompilerRules     )  {};
    virtual void        OnSave              ( text_out& Out              )  {};
    virtual void        OnLoad              ( text_out& In               )  {};
    virtual void        OnCheckIntegrity    ( void ) {ASSERT(0); }  
    char m_Dependency[256];
};

DEFINE_RSC_TYPE( s_MeshDcl, rigidgeom_rsc_desc, "rigidgeom", "MATX Files (*.matx)", "Geometry for Rigid Objects" );

//=========================================================================
// FUNCTIONS
//=========================================================================

//=========================================================================

rigidgeom_rsc_desc::rigidgeom_rsc_desc( void ) : rsc_desc( s_MeshDcl ) 
{
}

//=========================================================================

void rigidgeom_rsc_desc::OnEnumProp( prop_enum& List )
{
    List.AddString( "Dependency", "Dependencty of a resource. This is really a test" );
}

//=========================================================================
xbool rigidgeom_rsc_desc::OnProperty( prop_query& I )
{
    if( I.VarString( "Dependency", m_Dependency ) ) return TRUE;

    return FALSE;
}

*/
