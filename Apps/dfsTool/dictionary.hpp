//==============================================================================
//
//  dictionary.hpp
//
//==============================================================================

#ifndef DICTIONARY_HPP
#define DICTIONARY_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#ifndef X_TYPES_HPP
#include "x_types.hpp"
#endif

#ifndef X_ARRAY_HPP
#include "x_array.hpp"
#endif

#include "x_bytestream.hpp"

//==============================================================================
//  CLASS dictionary
//==============================================================================

class dictionary
{
protected:
    struct entry
    {
        s32         Offset;                 // Offset of this entry into the buffer
        u32         Hash;                   // Hash key for this entry
    };

    xarray<entry>   m_Entries;              // Array of entries
    char*           m_pBuffer;              // Pointer to buffer for storing strings
    s32             m_BufferLength;         // Length of buffer
    s32             m_NextOffset;           // Next offset to write to in buffer

    s32*            m_pHashTable;           // Pointer to hash table
    s32             m_HashTableSize;        // Number of entries in hash table (each entry is an s32)

    void            ResizeHashTable ( s32 NewSize );
    void            HashString      ( const char* pString, u32& Hash, s32& Length ) const;
    s32             Find            ( const char* pString, u32& Hash, s32& Length ) const;

public:
                    dictionary      ( );
                   ~dictionary      ( );

    s32             Find            ( const char* pString ) const;          // Find entry in dictionary, -1 = failed, otherwise index
    s32             Add             ( const char* pString );                // Add entry to dictionary, -1 = failed, otherwise index
    const char*     GetString       ( s32 iEntry ) const;                   // Get string given index
    s32             GetOffset       ( s32 iEntry ) const;                   // Get offset into dictionary given index
    s32             GetCount        ( void ) const;                         // Get count of entries in dictionary
    s32             GetSaveSize     ( void ) const;                         // Get number of bytes to save dictionary
    
    s32             Save            ( X_FILE* pFile ) const;                // Save dictionary to file
    s32             Save            ( xbytestream& ByteStream ) const;      // Save dictionary to bytestream
    xbool           Load            ( X_FILE* pFile, s32 nBytes );          // Load dictionary from file
};

//==============================================================================
#endif // DICTIONARY_HPP
//==============================================================================
