//==============================================================================
//  DecalLoader.cpp
//
//  Copyright (c) 2003 Inevitable Entertainment Inc. All rights reserved.
//
//  This code handles thte loading of decal resources and adds them to the
//  resource manager
//==============================================================================

#include "DecalDefinition.hpp"
#include "Decals\DecalMgr.hpp"
#include "Decals\DecalPackage.hpp"
#include "ResourceMgr\ResourceMgr.hpp"

//==============================================================================
// Decal loader
//==============================================================================

struct decal_loader : public rsc_loader
{
    //-------------------------------------------------------------------------
    
    decal_loader( void ) : rsc_loader( "DECAL PACKAGE", ".decalpkg" )
    {
    }

    //-------------------------------------------------------------------------
    
    virtual void* PreLoad ( X_FILE* FP )
    {
        MEMORY_OWNER( "DECAL DATA" );
        fileio File;
        return( File.PreLoad( FP ) );
    }

    //-------------------------------------------------------------------------
    
    virtual void* Resolve ( void* pData ) 
    {
        fileio         File;
        decal_package* pDecalPkg = NULL;

        File.Resolved( (fileio::resolve*)pData, pDecalPkg );

        s32 i;
        for ( i = 0; i < pDecalPkg->GetNDecalDefs(); i++ )
        {
            decal_definition& DecalDef = pDecalPkg->GetDecalDef(i);
            DecalDef.m_Handle = HNULL;
            g_DecalMgr.RegisterDefinition( DecalDef );
        }

        return( pDecalPkg );
    }

    //-------------------------------------------------------------------------
    
    virtual void Unload( void* pData )
    {
        decal_package* pDecalPkg = (decal_package*)pData;
        ASSERT( pDecalPkg );

        s32 i;
        for ( i = 0; i < pDecalPkg->GetNDecalDefs(); i++ )
        {
            decal_definition& DecalDef = pDecalPkg->GetDecalDef(i);
            
            if ( DecalDef.m_Handle.IsNonNull() )
                g_DecalMgr.UnregisterDefinition( DecalDef );

            DecalDef.m_Handle = HNULL;
        }
    
        delete pDecalPkg;
    }

};

static decal_loader s_Decal_Loader;

//==============================================================================

void ForceDecalLoaderLink( void )
{
}

//==============================================================================
