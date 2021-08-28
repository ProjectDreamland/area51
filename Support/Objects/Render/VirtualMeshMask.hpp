//==============================================================================
//  VirtualMeshMask.hpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This file contains a struct for dealing with virtual meshes. The most
//  important part of this struct that makes it better than just a u32 is that
//  it can handle enumerating of properties on the editor side, but compiles
//  into a simple mask on the game side
//==============================================================================

#ifndef _VIRTUALMESHMASK_HPP_INCLUDED_
#define _VIRTUALMESHMASK_HPP_INCLUDED_

//==============================================================================
// INCLUDES
//==============================================================================

#include "x_types.hpp"
#include "Auxiliary\MiscUtils\Dictionary.hpp"

//==============================================================================
// FORWARD REFERENCES
//==============================================================================

struct geom;
class  prop_enum;
class  prop_query;

//==============================================================================
// STRUCT DEFINITION
//==============================================================================

struct virtual_mesh_mask
{
    enum { MAX_VMESHES = 32 };

    // init
    virtual_mesh_mask( void );

    // useful helper functions
            operator u32( void ) const                       { return VMeshMask; }
    void    OnEnumProp  ( prop_enum&  List, geom* pGeom );
    xbool   OnProperty  ( prop_query& I,    geom* pGeom );
    
    void    SyncVMeshes ( geom* pGeom );

    // editor-side data
#if defined(X_EDITOR)
    s32   nVMeshes;
    s32   VMeshNameIds[MAX_VMESHES];

    static dictionary VMeshDictionary;
#endif

    // run-time data
    u32 VMeshMask;
};

//==============================================================================

#endif // _VIRTUALMESHMASK_HPP_INCLUDED_