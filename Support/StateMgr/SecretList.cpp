//=========================================================================
//
//  SecretList.cpp
//
//=========================================================================

#include "x_files/x_types.hpp"

#include "SecretList.hpp"
#include "stringmgr/stringmgr.hpp"
#include "ResourceMgr\ResourceMgr.hpp"
#include "StateMgr/StateMgr.hpp"

//=========================================================================
//  Defines
//=========================================================================

//=========================================================================
//  Structs                                                                
//=========================================================================

//=========================================================================
//  Globals                                                                
//=========================================================================
secret_list g_SecretList;

//=========================================================================
//  Functions
//=========================================================================

void secret_list::Init( void )
{
    xstring Manifest;

    (void)PRELOAD_FILE("SecretList.txt");

    Clear();
    VERIFY( Manifest.LoadFile( xfs("%s/SecretList.txt", g_RscMgr.GetRootDirectory()) ) );
    Append( (const char*)Manifest );
}

//=========================================================================

void secret_list::Kill( void )
{
    Clear();
}

//=========================================================================

void secret_list::Clear( void )
{
    // clear the list
    x_memset( m_SecretList, 0, sizeof( m_SecretList ) );
    m_SecretList[0].SecretID = -1;
}

//=========================================================================

xbool secret_list::Append( const secret_entry& Entry )
{
    secret_entry* pSecretEntry;
    s32         Count;
    
    pSecretEntry = m_SecretList;
    Count = 0;

    while( (Count < SECRET_TABLE_SIZE) && (pSecretEntry->SecretID != -1) )
    {
        Count++;
        pSecretEntry++;
    }

    // No space, don't append.
    if( Count >= SECRET_TABLE_SIZE )
    {
        ASSERTS( FALSE, "Too many entries in secret data file.  Increase max size or reduce number of entries." );
        return FALSE;
    }

    *pSecretEntry++=Entry;

    if( ++Count < SECRET_TABLE_SIZE )
    {
        pSecretEntry->SecretID = -1;
    }

    return TRUE;
}

//=========================================================================

secret_entry* secret_list::GetByIndex( s32 Index )
{
    return &m_SecretList[Index];
}

//=========================================================================

static s32 GetNumber( const char*& pManifest )
{
    s32 value;
    s32 IsNegative;

    IsNegative = FALSE;
    while( !x_isdigit(*pManifest) && (*pManifest!='-') )
    {
        if( *pManifest == 0x0 )
        {
            return 0;
        }
        pManifest++;
    }

    if( *pManifest == '-' )
    {
        pManifest++;
        IsNegative = TRUE;
    }

    value = 0;
    while( x_isdigit(*pManifest) )
    {
        if( *pManifest == 0x0 )
        {
            break;
        }
        value = value*10+*pManifest-'0';
        pManifest++;
    }
    if( IsNegative )
    {
        value = -value;
    }
    return value;
}


//=========================================================================

static void GetString( const char*& pManifest, char* pString, s32 MaxLength )
{
    x_memset(pString,0,MaxLength);

    // Look for Opening Quote mark
    while( *pManifest != '"' )
    {
        if( *pManifest == 0x0 )
        {
            return;
        }
        pManifest++;
    }

    // Skip quote mark
    pManifest++;

    // Copy until close quote mark
    while( *pManifest != '"' )
    {
        if( *pManifest == 0x0 )
        {
            return;
        }

        if( MaxLength > 0 )
        {
            *pString++ = *pManifest;
            MaxLength--;
        }
        pManifest++;
    }
    pManifest++;
}

//=========================================================================

void secret_list::Append( const char* pManifest )
{
    s32 Version = 0;
    char SecretTypeString[16];

    // Get the version of the manifest
    Version = GetNumber( pManifest );
    
    if( Version != SECRET_VERSION )
    {
        ASSERTS( FALSE, "Version mismatch in secret data file!" );
        return;
    }

    secret_entry Entry;

    while( TRUE )
    {
        x_memset(&Entry, 0, sizeof(Entry) );
        x_memset(&SecretTypeString, 0, sizeof(SecretTypeString) );

        // read secret ID
        Entry.SecretID = GetNumber( pManifest );
        if( (Entry.SecretID == -1) || (*pManifest==0x0) )
        {
            break;
        }

        // get secret type
        GetString( pManifest, SecretTypeString, sizeof(SecretTypeString) );
        if( x_stricmp( SecretTypeString, "VIDEO" )==0 )
        {
            Entry.SecretType = SECRET_TYPE_VIDEO;
        }
        else if( x_stricmp( SecretTypeString, "CHEAT" )==0 )
        {
            Entry.SecretType = SECRET_TYPE_CHEAT;
        }
        else
        {
            ASSERTS( FALSE, "Unexpected secret type read from secret list!" );
        }

        GetString( pManifest, Entry.FileName,   sizeof(Entry.FileName ) );
        GetString( pManifest, Entry.ShortDesc,  sizeof(Entry.ShortDesc) );
        GetString( pManifest, Entry.FullDesc,   sizeof(Entry.FullDesc ) );

        Append( Entry );
    }
}

