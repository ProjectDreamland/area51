//==============================================================================
//  VirtualTextureMask.hpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This file contains a struct for dealing with virtual textures. The most
//  important part of this struct that makes it better than just a u32 is that
//  it can handle enumerating of properties on the editor side, but compiles
//  into a simple mask on the game side
//==============================================================================

#ifndef _VIRTUALTEXTUREMASK_HPP_INCLUDED_
#define _VIRTUALTEXTUREMASK_HPP_INCLUDED_

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

struct virtual_texture_mask
{
    enum { MAX_VTEXTURES = 8 };

    // init
    virtual_texture_mask( void );

    // useful helper functions
    operator u32( void ) const                       { return VTextureMask; }
    void    OnEnumProp  ( prop_enum&  List, geom* pGeom );
    xbool   OnProperty  ( prop_query& I,    geom* pGeom );

    s32         FindFirstMat  ( geom* pGeom, s32 iVTexture );
    const char* BuildEnumList ( geom* pGeom, s32 iVTexture );
    void        SyncVTextures ( geom* pGeom );

    // editor-side data
#if defined(X_EDITOR)
    s32   nVTextures;
    s32   VTextureNameIds[MAX_VTEXTURES];
    s32   VTextureChoiceIds[MAX_VTEXTURES];

    static dictionary VTextureDictionary;
#endif

    // run-time data
    u32 VTextureMask;
};

//==============================================================================

#endif // _VIRTUALTEXTUREMASK_HPP_INCLUDED_