//==============================================================================
//  DecalPackage.hpp
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

#ifndef DECALPACKAGE_HPP
#define DECALPACKAGE_HPP

//==============================================================================
// Includes
//==============================================================================

#include "Auxiliary\MiscUtils\Fileio.hpp"
#include "DecalDefinition.hpp"

//==============================================================================
// Functions used to force the decal loader to link
//==============================================================================
void ForceDecalLoaderLink( void );

//==============================================================================
// The class
//==============================================================================

class decal_package
{
public:
    enum
    {
        DECAL_PACKAGE_VERSION = 0x0004
    };

    //==========================================================================
    // Constructors/destructors
    //==========================================================================
            decal_package       ( void );
            decal_package       ( fileio& File );
            ~decal_package      ( void );
    void    FileIO              ( fileio& File );

    //==========================================================================
    // Get/Set functions
    //==========================================================================
    s32                 GetNGroups      ( void                  )  const;
    const char*         GetGroupName    ( s32         iGroup    )  const;
    void                SetGroupName    ( s32         iGroup,
                                          const char* Name      );
    xcolor              GetGroupColor   ( s32         iGroup    )  const;
    void                SetGroupColor   ( s32         iGroup, 
                                          xcolor      Color     );
    s32                 GetNDecalDefs   ( void                  )  const;
    decal_definition&   GetDecalDef     ( s32         iDecalDef )  const;
    s32                 GetNDecalDefs   ( s32         iGroup    )  const;
    decal_definition&   GetDecalDef     ( s32         iGroup,
                                          s32         iDecalDef )  const;
    decal_definition*   GetDecalDef     ( const char* Name,
                                          s32         iDecalDef )  const;

    //==========================================================================
    // Functions used by the decal compiler.
    //==========================================================================
    #ifdef TARGET_PC
    void    AllocGroups          ( s32 nGroups    ); // DO NOT USE IN-GAME!!!!
    void    AllocDecals          ( s32 nDecals    ); // DO NOT USE IN-GAME!!!!
    void    SetGroupDecalDefStart( s32 iGroup,
                                   s32 iDecalDef  ); // DO NOT USE IN-GAME!!!!
    void    SetGroupDecalDefCount( s32 iGroup,
                                   s32 nDecalDefs ); // DO NOT USE IN-GAME!!!!
    #endif
        
protected:
    //==========================================================================
    // Internal structures
    //==========================================================================
    struct group
    {
        void    FileIO  ( fileio& File );

        char    Name[32];
        xcolor  Color;
        s32     nDecalDefs;
        s32     iDecalDef;
    };

    //==========================================================================
    // Decal data
    //==========================================================================
    s32                 m_Version;
    s32                 m_nGroups;
    group*              m_pGroups;
    s32                 m_nDecalDefs;
    decal_definition*   m_pDecalDefs;
};

//==============================================================================
// Inlines
//==============================================================================

inline s32 decal_package::GetNGroups( void ) const
{
    return m_nGroups;
}

//==============================================================================

inline const char* decal_package::GetGroupName( s32 iGroup ) const
{
    ASSERT( (iGroup>=0) && (iGroup < m_nGroups) );
    return m_pGroups[iGroup].Name;
}

//==============================================================================

inline void decal_package::SetGroupName( s32 iGroup, const char* Name )
{
    ASSERT( (iGroup>=0) && (iGroup < m_nGroups) );
    x_strsavecpy( m_pGroups[iGroup].Name, Name, 32 );
}

//==============================================================================

inline xcolor decal_package::GetGroupColor( s32 iGroup )  const
{
    ASSERT( (iGroup>=0) && (iGroup < m_nGroups) );
    return m_pGroups[iGroup].Color;
}

//==============================================================================

inline void decal_package::SetGroupColor( s32 iGroup, xcolor Color )
{
    ASSERT( (iGroup>=0) && (iGroup < m_nGroups) );
    m_pGroups[iGroup].Color = Color;
}

//==============================================================================

inline s32 decal_package::GetNDecalDefs( void ) const
{
    return m_nDecalDefs;
}

//==============================================================================

inline decal_definition& decal_package::GetDecalDef( s32 iDecalDef ) const
{
    ASSERT( (iDecalDef>=0) && (iDecalDef < m_nDecalDefs) );
    return m_pDecalDefs[iDecalDef];
}

//==============================================================================

inline s32 decal_package::GetNDecalDefs( s32 iGroup ) const
{
    ASSERT( (iGroup>=0) && (iGroup < m_nGroups) );
    return m_pGroups[iGroup].nDecalDefs;
}

//==============================================================================

inline decal_definition& decal_package::GetDecalDef( s32 iGroup, s32 iDecalDef ) const
{
    ASSERT( (iGroup>=0) && (iGroup < m_nGroups) );
    ASSERT( (iDecalDef>=0) && (iDecalDef < m_pGroups[iGroup].nDecalDefs) );
    return m_pDecalDefs[m_pGroups[iGroup].iDecalDef+iDecalDef];
}

//==============================================================================

inline decal_definition* decal_package::GetDecalDef( const char* Name,
                                                     s32         iDecalDef )  const
{
    s32 i;
    for ( i = 0; i < m_nGroups; i++ )
    {
        if ( !x_strcmp( GetGroupName(i), Name ) )
        {
            if ( (iDecalDef>=0) && (iDecalDef<m_pGroups[i].nDecalDefs) )
                return &m_pDecalDefs[m_pGroups[i].iDecalDef+iDecalDef];
        }
    }

    return NULL;
}

//==============================================================================

#endif // DECALPACKAGE_HPP