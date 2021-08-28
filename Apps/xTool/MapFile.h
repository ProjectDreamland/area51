//==============================================================================
//
//  MapFile.h
//
//==============================================================================

#ifndef MAP_FILE_H
#define MAP_FILE_H

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#ifndef X_ARRAY_HPP
#include "x_array.hpp"
#endif

//==============================================================================
//  CLASS CMapFile
//==============================================================================

class CMapFile : public CObject
{
public:

    struct section
    {
        s32         iName;              // Name of the section
        u32         Address;            // Address of the section
        u32         Size;               // Size of the section in bytes
    };

    struct object
    {
        s32         iSection;           // Index of the section this object file belongs to
        s32         iName;              // Name of the object file
        u32         Address;            // Address of the object file
        u32         Size;               // Size of the object file in bytes
    };

    struct symbol
    {
        s32         iSection;           // Index of the Section this function belongs to
        s32         iObject;            // Index of the Object File this function belongs to
        s32         iName;              // Name of the function
        u32         Address;            // Address of the function
        u32         Size;               // Size of the function in bytes
    };

    xbool           m_Initialized;      // Has been initialized
    xstring         m_FileName;         // Name of the map file
    xstring         m_MapFile;          // TEXT of the map file
    xarray<section> m_Sections;         // Sections from the map file
    xarray<object>  m_Objects;          // Object files from the map file
    xarray<symbol>  m_Symbols;          // Symbol from the map file

public:
                        CMapFile            ( );
                       ~CMapFile            ( );
    DECLARE_DYNCREATE( CMapFile )

    xbool               Init                ( const char* pFileName );
    void                Kill                ( void );

    const char*         AddressToSymbol     ( u32 Address );

    void                Serialize           ( CArchive& ar );

protected:
    xbool               LoadFile            ( const char* pFileName );

    xbool               ishex               ( char c );
    s32                 tohex               ( char c );
    u32                 ReadHex             ( xstring& s, s32& i );
    void                SkipSpaces          ( xstring& s, s32& i );
    void                SkipField           ( xstring& s, s32& i );
    void                SkipToEOL           ( xstring& s, s32& i );
};

//==============================================================================
#endif // MAP_FILE_H
//==============================================================================
