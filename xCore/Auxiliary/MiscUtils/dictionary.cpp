//==============================================================================
//
//  dictionary.cpp
//
//==============================================================================

#include "dictionary.hpp"

#define MIN_HASH_TABLE_SIZE         2048
#define MIN_STRING_BUFFER_SIZE      (16*1024)

//==============================================================================

dictionary::dictionary()
{
    m_pBuffer       = NULL;
    m_pHashTable    = NULL;
    Reset();

}

//==============================================================================

dictionary::~dictionary()
{
    Reset();
}

//==============================================================================

void dictionary::Reset( void )
{
    if( m_pBuffer )
        x_free( m_pBuffer );

    if( m_pHashTable )
        x_free( m_pHashTable );

    m_Entries.Clear();

    m_pBuffer = NULL;
    m_pHashTable = NULL;
    m_BufferLength  = 0;
    m_NextOffset    = 0;
    m_HashTableSize = 0;
}

//==============================================================================

void dictionary::ResizeHashTable( s32 NewSize )
{
    MEMORY_OWNER( "dictionary::ResizeHashTable" );
    NewSize = MAX(NewSize,MIN_HASH_TABLE_SIZE);
    NewSize = MAX(NewSize,m_Entries.GetCount()*2);
    ASSERT( NewSize >= m_Entries.GetCount() );

    m_HashTableSize = NewSize;
    m_pHashTable    = (s32*)x_realloc( m_pHashTable, m_HashTableSize * sizeof(s32) );
    x_memset( m_pHashTable, 0xff, m_HashTableSize * sizeof(s32) );

    for( s32 i=0 ; i<m_Entries.GetCount() ; i++ )
    {
        s32 Index = m_Entries[i].Hash % m_HashTableSize;

        while( m_pHashTable[Index] != -1 )
        {
            Index++;
            if( Index == m_HashTableSize ) Index = 0;
        }

        m_pHashTable[Index] = i;
    }
}

//==============================================================================

void dictionary::HashString( const char* pString, u32& Hash, s32& Length ) const
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
//        Hash = (Hash << 4) + *pString++;
//        Hash = (Hash << 4) ^ (Hash >> 28) + *pString++;
//        Hash = *pString++ + (Hash << 6) + (Hash << 16) - Hash;
        Hash = (Hash * 33) ^ *pString++;
    }

    // Calculate length of string
    Length = pString - pStringStart;
}

//==============================================================================

s32 dictionary::Find( const char* pString, u32& Hash, s32& Length ) const
{
    // Hash the string for faster comparison, also get the length while we're at it
    HashString( pString, Hash, Length );

    // If the hash table has no entries then we won't find it!
    if( m_HashTableSize==0 )
        return -1;

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

        Index++;
        if( Index==m_HashTableSize ) Index = 0;
    }

    // Return not found
    return -1;
}

//==============================================================================

s32 dictionary::Find( const char* pString ) const
{
    u32 Hash;
    s32 Length;

    // Call internal find function
    return( Find( pString, Hash, Length ) );
}

//==============================================================================

s32 dictionary::Add( const char* pString )
{
    s32 i;
    u32 Hash;
    s32 Length;

    MEMORY_OWNER( "DICTIONARY" );

    // Try to find match
    i = Find( pString, Hash, Length );
    if( i != -1 )
        return i;

    // Reallocate the buffer if too small to add a new string
    if( (m_BufferLength-m_NextOffset) < (Length+1) )
    {
        m_BufferLength = MAX( (m_BufferLength*2), MIN_STRING_BUFFER_SIZE );
        m_pBuffer = (char*)x_realloc( m_pBuffer, m_BufferLength );
        ASSERT( m_pBuffer );
    }

    // Create a new dictionary_entry
    dictionary_entry& Entry = m_Entries.Append();
    Entry.Hash   = Hash;
    Entry.Offset = m_NextOffset;

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
        Index++;
        if( Index == m_HashTableSize ) Index = 0;
    }
    m_pHashTable[Index] = m_Entries.GetCount()-1;

    // Return index of string
    return m_Entries.GetCount()-1;
}

//==============================================================================

const char* dictionary::GetString( s32 iEntry ) const
{
    ASSERT( (iEntry >= 0) && (iEntry < m_Entries.GetCount()) );
    return &m_pBuffer[m_Entries[iEntry].Offset];
}

//==============================================================================

s32 dictionary::GetOffset( s32 iEntry ) const
{
    ASSERT( (iEntry >= 0) && (iEntry < m_Entries.GetCount()) );
    return m_Entries[iEntry].Offset;
}

//==============================================================================

s32 dictionary::GetCount( void ) const
{
    return m_Entries.GetCount();
}

//==============================================================================

s32 dictionary::GetSaveSize( void ) const
{
    return m_NextOffset;
}

//==============================================================================

s32 dictionary::Save( X_FILE* pFile ) const
{
    ASSERT( pFile );

    // Write the data buffer
    s32 BytesWritten = x_fwrite( m_pBuffer, 1, m_NextOffset, pFile );
    ASSERT( BytesWritten == m_NextOffset );

    // Return number of bytes written
    return BytesWritten;
}

//==============================================================================

xbool dictionary::Load( X_FILE* pFile, s32 nBytes )
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
 
        // Add a new dictionary dictionary_entry
        dictionary_entry&  Entry = m_Entries.Append();
 
        // Get the hash key and length of the string
        HashString( pString, Entry.Hash, Length );
 
        // Set the offset into the dictionary_entry
        Entry.Offset = (pString - m_pBuffer);
 
        // Advance to next string
        pString += Length+1;
    }
 
    // Rebuild the hash table
    ResizeHashTable( m_Entries.GetCount()*2 );
 
    return TRUE;
}

//==============================================================================

s32 dictionary::Save( const char* pFileName  ) const
{
    ASSERT( pFileName );
    
    X_FILE* pFile = x_fopen( pFileName, "wb" );
    
    return Save( pFile );
}

//==============================================================================

xbool dictionary::Load( const char* pFileName )
{    
    ASSERT( pFileName );
    
    X_FILE* pFile = x_fopen( pFileName, "rb" );
    
    if (pFile == NULL)
        return FALSE;
    
    xbool Rval = Load( pFile, x_flength(pFile) );
    
    x_fclose( pFile );
    
    return Rval;
}

//==============================================================================

void dictionary::Export( bitstream& BitStream )
{
    // Save the dictionary to a bitstream

    // write out data size (in bytes)
    BitStream.WriteU32( m_NextOffset );

    // Write the data buffer
    BitStream.WriteBits( m_pBuffer, ( m_NextOffset * 8 ) );
}

//==============================================================================

void dictionary::Import( bitstream& BitStream )
{
    // Load the dictionary from a bitstream

    // read data size
    u32 nBytes;
    BitStream.ReadU32( nBytes );

    // Resize the data buffer
    m_pBuffer = (char*)x_realloc( m_pBuffer, nBytes );
    m_BufferLength = nBytes;
    m_NextOffset   = nBytes;
 
    // Read the data into buffer
    BitStream.ReadBits( m_pBuffer, (nBytes*8) );
 
    // Recreate all the dictionary entries
    m_Entries.Clear();
    const char* pString = m_pBuffer;
    while( (pString - m_pBuffer) < m_NextOffset )
    {
        s32     Length;
 
        // Add a new dictionary dictionary_entry
        dictionary_entry&  Entry = m_Entries.Append();
 
        // Get the hash key and length of the string
        HashString( pString, Entry.Hash, Length );
 
        // Set the offset into the dictionary_entry
        Entry.Offset = (pString - m_pBuffer);
 
        // Advance to next string
        pString += Length+1;
    }
 
    // Rebuild the hash table
    ResizeHashTable( m_Entries.GetCount()*2 );
}

//==============================================================================

void dictionary::Dump( void )
{
    x_DebugMsg( 0, "------------------------\n" );
    x_DebugMsg( 0, "       DICTIONARY       \n" );
    x_DebugMsg( 0, "------------------------\n" );

    // Dump the contents of the dictionary to the debug window
    for( s32 x=0; x<m_Entries.GetCount(); x++ )
    {
        x_DebugMsg( 0, "Dictionary string : %s\n", &m_pBuffer[m_Entries[x].Offset] );
    }
}

//==============================================================================

