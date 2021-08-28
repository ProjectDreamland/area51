//=========================================================================
//
//  LoreList.cpp
//
//=========================================================================

#include "x_files/x_types.hpp"

#include "LoreList.hpp"
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
lore_list g_LoreList;

//=========================================================================
//  Functions
//=========================================================================

void lore_list::Init( void )
{
    xstring Manifest;

    (void)PRELOAD_FILE("LoreList.txt");

    Clear();
    VERIFY( Manifest.LoadFile( xfs("%s/LoreList.txt", g_RscMgr.GetRootDirectory()) ) );
    Append( (const char*)Manifest );

#if defined (mbillington)
    for( s32 i=0; i<LORE_TABLE_SIZE; i++ )
    {
        lore_entry& Current = *g_LoreList.GetByIndex(i);
        if( Current.LoreID == -1 )
            break;

        LOG_MESSAGE( "lore_list::Init",
            "LoreID:%d, Type:%d, FileName:%s, Clue:%s, ShortDesc:%s, LongDesc:%s, Name:%s",
                     Current.LoreID, Current.LoreType, Current.FileName, 
                     Current.Clue, Current.ShortDesc, Current.FullDesc,
                     Current.FullName);
    }
#endif
}

//=========================================================================

void lore_list::Kill( void )
{
    Clear();
}

//=========================================================================

void lore_list::Clear( void )
{
    // clear the list
    x_memset( m_LoreList, 0, sizeof( m_LoreList ) );
    m_LoreList[0].LoreID = -1;

    // clear the vault 
    x_memset( m_LoreVault, 0, sizeof( m_LoreVault) );
    m_LoreVault[0].MapID = -1;
}

//=========================================================================

xbool lore_list::Append( const lore_entry& Entry )
{
    lore_entry* pLoreEntry;
    s32         Count;
    
    pLoreEntry = m_LoreList;
    Count = 0;

    while( (Count < LORE_TABLE_SIZE) && (pLoreEntry->LoreID != -1) )
    {
        Count++;
        pLoreEntry++;
    }

    // No space, don't append.
    if( Count >= LORE_TABLE_SIZE )
    {
        ASSERTS( FALSE, "Too many entries in lore data file.  Increase max size or reduce number of entries." );
        return FALSE;
    }

    *pLoreEntry++=Entry;

    if( ++Count < LORE_TABLE_SIZE )
    {
        pLoreEntry->LoreID = -1;
    }

    return TRUE;
}

//=========================================================================

xbool lore_list::AddToVault( const lore_entry& Entry )
{
    lore_vault* pVault = m_LoreVault;
    s32 Count = 0;

    while( ( Count < NUM_VAULTS ) && ( pVault->MapID != -1) )
    {
        // see if we already have a vault for this mapID
        if( pVault->MapID == Entry.MapID )
        {
            // OK, look for a space
            for( s32 i=0; i<NUM_PER_VAULT; i++ )
            {
                if( pVault->LoreID[i] == -1 )
                {
                    // found a space
                    pVault->LoreID[i] = Entry.LoreID;
                    return TRUE;
                }
            }
            // no space available!
            ASSERTS( FALSE, xfs("Too many entries for vault.  LoreID : %d    MapID : %d", Entry.LoreID, Entry.MapID ) );
        }
        pVault++;
        Count++;
    }

    // No vault for this mapID - set one up
    if( Count < NUM_VAULTS )
    {
        // found a free vault - store the data
        pVault->MapID     = Entry.MapID;
        pVault->LoreID[0] = Entry.LoreID;
        
        // clear remaining entries
        for( s32 v=1; v<NUM_PER_VAULT; v++ )
        {
            pVault->LoreID[v] = -1;
        }

        // flag the next entry free
        if( ++Count < NUM_VAULTS)
        {
            pVault++;
            pVault->MapID = -1;
        }

        return TRUE;
    }

    // could not add!
    ASSERTS( FALSE, "Ran out of lore vaults!" );
    return FALSE;
}

//=========================================================================

const lore_type lore_list::GetType( s32 LoreID )
{
    const lore_entry* pEntry;

    pEntry = Find( LoreID );
    if( pEntry==NULL )
    {
        return LORE_TYPE_UNKNOWN;
    }
    return pEntry->LoreType;
}

//=========================================================================

const char* lore_list::GetFileName( s32 LoreID )
{
    const lore_entry* pEntry;

    pEntry = Find( LoreID );
    if( pEntry==NULL )
    {
        return NULL;
    }
    return pEntry->FileName;
}

//=========================================================================

const lore_entry* lore_list::Find( s32 LoreID )
{
    if( LoreID == -1 )
    {
        return NULL;
    }

    lore_entry* pLoreList = m_LoreList;

    while( pLoreList->LoreID != -1 )
    {
        if( pLoreList->LoreID == LoreID )
        {
            return pLoreList;
        }
        pLoreList++;
    }

    return NULL;
}

//=========================================================================

lore_entry* lore_list::GetByIndex( s32 Index )
{
    return &m_LoreList[Index];
}


//=========================================================================

const xwchar* lore_list::GetLoreName( s32 LoreID )
{
    lore_entry* pLoreEntry;
    s32         Count;

    pLoreEntry = m_LoreList;
    Count = 0;

    while( (Count < LORE_TABLE_SIZE) && (pLoreEntry->LoreID != -1) )
    {
        if( pLoreEntry->LoreID == LoreID )
        {
            break;
        }

        Count++;
        pLoreEntry++;
    }

    // No space, don't append.
    if( Count >= LORE_TABLE_SIZE )
    {
        ASSERTS( FALSE, "Count is too high in lore_list::GetLoreName().  Probably an invalid LoreID" );
        return NULL;
    }

    pLoreEntry = &m_LoreList[Count];
    return g_StringTableMgr( "lore_ingame", pLoreEntry->FullName);
}

//=========================================================================

lore_vault* lore_list::GetVaultByMapID( s32 MapID, s32& Index )
{
    lore_vault* pVault = m_LoreVault;
    Index = 0;

    while( ( Index < NUM_VAULTS ) && ( pVault->MapID != -1) )
    {
        // check for a match
        if( pVault->MapID == MapID )
        {
            return pVault;
        }
        pVault++;
        Index++;
    }

    ASSERTS( FALSE, "Could not look up vault from mapID" );
    Index = -1;
    return NULL;
}

//=========================================================================

lore_vault* lore_list::GetVaultByLoreID( s32 LoreID, s32& Index, s32& LoreIndex )
{
    lore_vault* pVault = m_LoreVault;
    
    Index     = 0;
    LoreIndex = 0;

    while( ( Index < NUM_VAULTS ) && ( pVault->MapID != -1) )
    {
        // scan all the lore entries in the vault
        for( s32 i=0; i<NUM_PER_VAULT; i++ )
        {
            // check for a match
            if( pVault->LoreID[i] == LoreID )
            {
                // found it!
                LoreIndex = i;
                return pVault;
            }
        }
        pVault++;
        Index++;
    }

    ASSERTS( FALSE, "Could not look up vault from LoreID" );
    Index     = -1;
    LoreIndex = -1;
    
    return NULL;
}

s32 lore_list::GetLoreIDByVault( lore_vault *pVault, s32 Index)
{
    if( Index < NUM_PER_VAULT )
    {
        // get the actual lore ID
        return pVault->LoreID[Index];
    }

    return -1;
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

void lore_list::Append( const char* pManifest )
{
    s32 Version = 0;
    char LoreTypeString[16];

    // Get the version of the manifest
    Version = GetNumber( pManifest );
    
    if( Version != LORE_VERSION )
    {
        ASSERTS( FALSE, "Version mismatch in lore data file!" );
        return;
    }

    lore_entry Entry;

    while( TRUE )
    {
        x_memset(&Entry, 0, sizeof(Entry) );
        x_memset(&LoreTypeString, 0, sizeof(LoreTypeString) );

        // read lore ID
        Entry.LoreID = GetNumber( pManifest );
        if( (Entry.LoreID == -1) || (*pManifest==0x0) )
        {
            break;
        }

        // read map ID
        Entry.MapID = GetNumber( pManifest );

        // read number of items in set
        Entry.NumItems = GetNumber( pManifest );

        // get lore type
        GetString( pManifest, LoreTypeString, sizeof(LoreTypeString) );
        if( x_stricmp( LoreTypeString, "VIDEO" )==0 )
        {
            Entry.LoreType = LORE_TYPE_VIDEO;
        }
        else if( x_stricmp( LoreTypeString, "AUDIO" )==0 )
        {
            Entry.LoreType = LORE_TYPE_AUDIO;
        }
        else if( x_stricmp( LoreTypeString, "STILL" )==0 )
        {
            Entry.LoreType = LORE_TYPE_STILL;
        }
        else if( x_stricmp( LoreTypeString, "TEXT" )==0 )
        {
            Entry.LoreType = LORE_TYPE_TEXT;
        }
        else
        {
            ASSERTS( FALSE, "Unexpected lore type read from lore list!" );
        }

        GetString( pManifest, Entry.FileName,   sizeof(Entry.FileName ) );
        GetString( pManifest, Entry.Clue,       sizeof(Entry.Clue     ) );
        GetString( pManifest, Entry.ShortDesc,  sizeof(Entry.ShortDesc) );
        GetString( pManifest, Entry.FullDesc,   sizeof(Entry.FullDesc ) );
        GetString( pManifest, Entry.FullName,   sizeof(Entry.FullName ) );

        Append( Entry );

        AddToVault( Entry );
    }
}

