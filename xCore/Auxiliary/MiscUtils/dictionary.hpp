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

#include "x_bitstream.hpp"

//==============================================================================
//  CLASS dictionary
//==============================================================================
 
struct dictionary_entry
{
    s32         Offset;                 // Offset of this entry into the buffer
    u32         Hash;                   // Hash key for this entry
};

class dictionary
{
protected:

    typedef dictionary_entry       templated_entry;

    xarray<templated_entry>   m_Entries;    // Array of entries

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

    void            Reset           ( void );

    s32             Find            ( const char* pString ) const;          // Find entry in dictionary, -1 = failed, otherwise index
    s32             Add             ( const char* pString );                // Add entry to dictionary, -1 = failed, otherwise index
   
    const char*     GetString       ( s32 iEntry ) const;                   // Get string given index
    s32             GetOffset       ( s32 iEntry ) const;                   // Get offset into dictionary given index
    s32             GetCount        ( void ) const;                         // Get count of entries in dictionary
    s32             GetSaveSize     ( void ) const;                         // Get number of bytes to save dictionary
    
    s32             Save            ( X_FILE* pFile ) const;                // Save dictionary to file
    s32             Save            ( const char* pFileName  ) const;       // Save dictionary to file
   
    xbool           Load            ( X_FILE* pFile, s32 nBytes );          // Load dictionary from file
    xbool           Load            ( const char* pFileName );              // Load dictionary from file

    void            Export          ( bitstream& BitStream );               // Save the dictionary to a bitstream
    void            Import          ( bitstream& BitStream );               // Load the dictionary from a bitstream
    void            Dump            ( void );                               // Dump the dictionary contents

};

//==============================================================================
/*
template<class data_type> dictionary<data_type>::dictionary( s32 TableSize )
{
    m_pBuffer       = NULL;
    m_BufferLength  = 0;
    m_NextOffset    = 0;

    m_HashTableSize = TableSize; 
    m_pHashTable    = (s32*)x_malloc( m_HashTableSize * sizeof(s32) );
    x_memset( m_pHashTable, 0xff, m_HashTableSize * sizeof(s32) );
}

//==============================================================================

template<class data_type> dictionary<data_type>::dictionary()
{
    m_pBuffer       = NULL;
    m_BufferLength  = 0;
    m_NextOffset    = 0;

    m_HashTableSize = 20000; //65599; //32768;
    m_pHashTable    = (s32*)x_malloc( m_HashTableSize * sizeof(s32) );
    x_memset( m_pHashTable, 0xff, m_HashTableSize * sizeof(s32) );
}

//==============================================================================

template<class data_type> dictionary<data_type>::~dictionary()
{
    if( m_pBuffer )
        x_free( m_pBuffer );
}

//==============================================================================

template<class data_type> void  dictionary<data_type>::ResizeHashTable( s32 NewSize )
{
    ASSERT( NewSize >= m_Entries.GetCount() );

    m_HashTableSize = NewSize;
    m_pHashTable    = (s32*)x_realloc( m_pHashTable, m_HashTableSize * sizeof(s32) );
    x_memset( m_pHashTable, 0xff, m_HashTableSize * sizeof(s32) );

    for( s32 i=0 ; i<m_Entries.GetCount() ; i++ )
    {
        s32 Index = m_Entries[i].Hash % m_HashTableSize;

        while( m_pHashTable[Index] != -1 )
        {
            Index = (Index+1) % m_HashTableSize;
        }

        m_pHashTable[Index] = i;
    }
}

//==============================================================================

template<class data_type> void dictionary<data_type>::HashString( const char* pString, u32& Hash, s32& Length ) const
{
    const char* pStringStart = pString;
    Hash                     = 5381;
    Length                   = 0;

    // Process each character to generate the hash key
    while( *pString )
    {
        // These other hash functions are left here for completeness, they have all been tried and
        // are sorted in order of worst to best for performance (performance here usually means a
        // combination of simplicity to calculate and resulting distribution)
        //  Hash = (Hash << 4) + *pString++;
        //  Hash = (Hash << 4) ^ (Hash >> 28) + *pString++;
        //  Hash = *pString++ + (Hash << 6) + (Hash << 16) - Hash;
        Hash = (Hash * 33) ^ *pString++;
    }

    // Calculate length of string
    Length = pString - pStringStart;
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::Find( const char* pString, u32& Hash, s32& Length ) const
{
    // Hash the string for faster comparison, also get the length while we're at it
    HashString( pString, Hash, Length );

    // Try to find existing match in dictionary
    s32 Index = Hash % m_HashTableSize;
    while( m_pHashTable[Index] != -1 )
    {
        if( (m_pHashTable[Index] != -1) &&
            (m_Entries[m_pHashTable[Index]].Hash == Hash) &&
            (x_strcmp( pString, &m_pBuffer[m_Entries[m_pHashTable[Index]].Offset] ) == 0) )
        {
            return m_pHashTable[Index];
        }
        Index = (Index+1) % m_HashTableSize;
    }

    // Return not found
    return -1;
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::Find( const char* pString ) const
{
    u32 Hash;
    s32 Length;

    // Call internal find function
    return( Find( pString, Hash, Length ) );
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::Add( const char* pString )
{
    data_type Empty;
    
    return Add( pString, Empty );
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::Add( const char* pString, const data_type& rData )
{
    s32 i;
    u32 Hash;
    s32 Length;
    
    // Try to find match
    i = Find( pString, Hash, Length );
    if( i != -1 )
        return i;
    
    // Reallocate the buffer if too small to add a new string
    if( (m_BufferLength-m_NextOffset) < (Length+1) )
    {
        m_BufferLength = MAX( (m_BufferLength*2), 128*1024 );
        m_pBuffer = (char*)x_realloc( m_pBuffer, m_BufferLength );
        ASSERT( m_pBuffer );
    }
    
    // Create a new entry
    dictionary_entry<data_type>& Entry = m_Entries.Append();

    Entry.Hash   = Hash;
    Entry.Offset = m_NextOffset;
    Entry.Data   = rData;

    // Add string into buffer
    x_strcpy( &m_pBuffer[m_NextOffset], pString );
    m_pBuffer[m_NextOffset+Length] = 0;
    m_NextOffset += Length+1;
    
    // Add into hash table
    if( (m_HashTableSize/2) < m_Entries.GetCount() )
        ResizeHashTable( m_HashTableSize*2 );
    s32 Index = Hash % m_HashTableSize;
    while( m_pHashTable[Index] != -1 )
    {
        Index = (Index+1) % m_HashTableSize;
    }
    m_pHashTable[Index] = m_Entries.GetCount()-1;
    
    // Return index of string
    return m_Entries.GetCount()-1;
}

//==============================================================================

template<class data_type> const char* dictionary<data_type>::GetString( s32 iEntry ) const
{
    ASSERT( (iEntry >= 0) && (iEntry < m_Entries.GetCount()) );
    return &m_pBuffer[m_Entries[iEntry].Offset];
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::GetOffset( s32 iEntry ) const
{
    ASSERT( (iEntry >= 0) && (iEntry < m_Entries.GetCount()) );
    return m_Entries[iEntry].Offset;
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::GetCount( void ) const
{
    return m_Entries.GetCount();
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::GetSaveSize( void ) const
{
    return m_NextOffset;
}

//==============================================================================

template<class data_type> data_type dictionary<data_type>::GetData( s32 iEntry ) const
{
    ASSERT( (iEntry >= 0) && (iEntry < m_Entries.GetCount()) );
    return m_Entries[iEntry].Data;
}

//==============================================================================

template<class data_type> void dictionary<data_type>::SetData( s32 iEntry, const data_type& rData )
{
    ASSERT( (iEntry >= 0) && (iEntry < m_Entries.GetCount()) );
    m_Entries[iEntry].Data = rData;
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::Save( X_FILE* pFile ) const
{
    ASSERT( pFile );

    // Write the data buffer
    s32 BytesWritten = x_fwrite( m_pBuffer, 1, m_NextOffset, pFile );
    ASSERT( BytesWritten == m_NextOffset );

    // Return number of bytes written
    return BytesWritten;
}

//==============================================================================

template<class data_type> xbool dictionary<data_type>::Load( X_FILE* pFile, s32 nBytes )
{
    ASSERT( pFile );
 
    // Resize the data buffer
    m_pBuffer = (char*)x_realloc( m_pBuffer, nBytes );
    m_BufferLength = nBytes;
    m_NextOffset   = nBytes;
 
    // Read the data into buffer
    s32 BytesRead = x_fread( m_pBuffer, 1, nBytes, pFile );
    ASSERT( BytesRead == nBytes );
    (void)BytesRead;
 
    // Recreate all the dictionary entries
    m_Entries.Clear();
    const char* pString = m_pBuffer;
    while( (pString - m_pBuffer) < m_NextOffset )
    {
        s32     Length;
 
        // Add a new dictionary entry
        dictionary_entry<data_type>&  Entry = m_Entries.Append();
 
        // Get the hash key and length of the string
        HashString( pString, Entry.Hash, Length );
 
        // Set the offset into the entry
        Entry.Offset = (pString - m_pBuffer);
 
        // Advance to next string
        pString += Length+1;
    }
 
    // Rebuild the hash table
    ResizeHashTable( m_Entries.GetCount()*2 );
 
    return TRUE;
}

//==============================================================================

template<class data_type> s32 dictionary<data_type>::Save ( const char* pFileName  ) const
{
    ASSERT( pFileName );
    
    X_FILE* pFile = x_fopen( pFileName, "wb" );
    
    s32 Rval = 0;

    if (pFile != NULL)
    {
        Rval = Save( pFile );
        x_fclose( pFile );
    }
    
    return Rval;
}

//==============================================================================

template<class data_type> xbool dictionary<data_type>::Load ( const char* pFileName )
{    
    ASSERT( pFileName );
    
    X_FILE* pFile = x_fopen( pFileName, "rb" );
    
    if (pFile == NULL)
        return FALSE;
    
    s32     Filesize    = x_flength(pFile);
    xbool   Rval        = FALSE;
    
    if (Filesize > 0)
    {
        Rval = Load( pFile, Filesize );
    }
    
    x_fclose( pFile );
    
    return Rval;
}
*/
//==============================================================================
#endif // DICTIONARY_HPP
//==============================================================================
