//==============================================================================
//  DecalPackage.cpp
//
//  Copyright (c) 2004 Inevitable Entertainment Inc. All rights reserved.
//
//  This class contains groups of decals the game can interface with. For
//  example, a single package may contain all of the blood decals used in the
//  game and groups within the package could correspond with each type of blood
//  (human, mutant, gray, etc.) Another package could describe bullet holes, and
//  each group could correspond to the material it likes to stick on (say wood
//  or concrete).
//==============================================================================

#include "DecalPackage.hpp"

//==============================================================================
// Implementation
//==============================================================================

void decal_package::group::FileIO( fileio& File )
{
    File.Static( Name, 32 );
    File.Static( Color );
    File.Static( nDecalDefs );
    File.Static( iDecalDef );
}

//==============================================================================

decal_package::decal_package( void ) :
    m_Version( DECAL_PACKAGE_VERSION ),
    m_nGroups( 0 ),
    m_pGroups( NULL ),
    m_nDecalDefs( 0 ),
    m_pDecalDefs( NULL )
{
}

//==============================================================================

decal_package::decal_package( fileio& File )
{
    (void)File;

    if ( m_Version != DECAL_PACKAGE_VERSION )
    {
        x_throw( xfs( "Decal package versions do not match. App wants %d, package is %d", DECAL_PACKAGE_VERSION, m_Version ) );
        decal_package();
        return;
    }

    ForceDecalLoaderLink();
}

//==============================================================================

decal_package::~decal_package( void )
{
}

//==============================================================================

void decal_package::FileIO( fileio& File )
{
    File.Static( m_Version );
    File.Static( m_nGroups );
    File.Static( m_pGroups, m_nGroups );
    File.Static( m_nDecalDefs );
    File.Static( m_pDecalDefs, m_nDecalDefs );
}

//==============================================================================
// Functions for the compiler
//==============================================================================

#ifdef TARGET_PC

void decal_package::AllocGroups( s32 nGroups )
{
    if ( m_nGroups )
        delete []m_pGroups;

    m_nGroups = nGroups;
    m_pGroups = new group[m_nGroups];
}

//==============================================================================

void decal_package::AllocDecals( s32 nDecalDefs )
{
    if ( m_nDecalDefs )
        delete []m_pDecalDefs;

    m_nDecalDefs = nDecalDefs;
    m_pDecalDefs = new decal_definition[m_nDecalDefs];
}

//==============================================================================

void decal_package::SetGroupDecalDefStart( s32 iGroup, s32 iDecalDef )
{
    ASSERT( (iGroup>=0) && (iGroup < m_nGroups) );
    ASSERT( (iDecalDef>=0) && (iDecalDef < m_nDecalDefs) );

    m_pGroups[iGroup].iDecalDef = iDecalDef;
}

//==============================================================================

void decal_package::SetGroupDecalDefCount( s32 iGroup, s32 nDecalDefs )
{
    ASSERT( (iGroup>=0) && (iGroup < m_nGroups) );
    ASSERT( (nDecalDefs>=0) && (nDecalDefs <= m_nDecalDefs) );

    m_pGroups[iGroup].nDecalDefs = nDecalDefs;
}

#endif TARGET_PC

//==============================================================================
