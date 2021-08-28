//==============================================================================
//
//  HeapState.cpp
//
//==============================================================================

#include "stdafx.h"
#include "HeapState.h"
#include "LogData.h"

//==============================================================================

IMPLEMENT_DYNCREATE( CHeapState, CObject );

//==============================================================================

CHeapState::CHeapState()
{
    m_HashTableSize = 20000; //65599; //32768;
    m_pHashTable    = (s32*)malloc( m_HashTableSize * sizeof(s32) );
    memset( m_pHashTable, 0xff, m_HashTableSize * sizeof(s32) );

    m_HeaderSize        = MEMORY_HEADER_SIZE;
    m_MinAddress        = U32_MAX;
    m_MaxAddress        = U32_MIN;
    m_CurrentBytes      = 0;
    m_MaximumBytes      = 0;
}

//==============================================================================

CHeapState::~CHeapState()
{
    if( m_pHashTable )
        free( m_pHashTable );
}

//==============================================================================

void CHeapState::ResizeHashTable( s32 NewSize )
{
    ASSERT( NewSize >= m_Entries.GetCount() );

    if( m_HashTableSize != NewSize )
    {
        m_HashTableSize = NewSize;
        m_pHashTable    = (s32*)realloc( m_pHashTable, m_HashTableSize * sizeof(s32) );
        memset( m_pHashTable, 0xff, m_HashTableSize * sizeof(s32) );

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
}

//==============================================================================

void CHeapState::ApplyOperationMalloc( log_memory* pEntry, bool UpdateEntry )
{
    s32 iEntry      = -1;
    u32 Address     = pEntry->GetAddress();
    u32 Size        = pEntry->GetSize();
    u32 RoundedSize = (Size + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1);

    // Don't log 0 size allocations
    // TODO: Remove this when new memory manager installed
    if( Address == 0xffffffff )
        return;

    // Update memory bounds
    if( m_MinAddress > (Address - m_HeaderSize) )
        m_MinAddress = (Address - m_HeaderSize);
    if( m_MaxAddress < (Address + RoundedSize) )
        m_MaxAddress = (Address + RoundedSize);

    // Check for a free entry
    if( m_FreeEntries.GetCount() > 0 )
    {
        // Use last free entry
        iEntry = m_FreeEntries[m_FreeEntries.GetCount() - 1];
        m_FreeEntries.Delete( m_FreeEntries.GetCount() - 1 );
    }
    else
    {
        // Create a new entry
        iEntry = m_Entries.GetCount();
        m_Entries.Append();

        // Resize the hash table if necessary
        if( (m_HashTableSize/2) < m_Entries.GetCount() )
            ResizeHashTable( m_HashTableSize*2 );
    }

    ASSERT( iEntry != -1 );

    // Setup entry
    entry& Entry = m_Entries[iEntry];
    Entry.pEntry = pEntry;
    Entry.Hash   = Address;

    // Check for occupied memory
    if( UpdateEntry )
    {
        if( IsRangeOccupied( Address, RoundedSize ) )
            pEntry->SetFlags( log_memory::flag_system_error, log_memory::flag_system_error );
    }

    // Insert into hash table
    s32 iHash = Entry.Hash % m_HashTableSize;
    while( m_pHashTable[iHash] != -1 )
    {
        iHash = (iHash+1) % m_HashTableSize;
    }
    m_pHashTable[iHash] = iEntry;

    // Update current bytes allocated
    m_CurrentBytes += m_HeaderSize + RoundedSize;
    m_MaximumBytes = max( m_MaximumBytes, m_CurrentBytes );

    // Update flags for entry
    if( UpdateEntry )
    {
        pEntry->SetFlags( log_memory::flag_memory_active, log_memory::flag_memory_active );
        pEntry->SetCurrentBytes( m_CurrentBytes );
    }
}

//==============================================================================

void CHeapState::ApplyOperationRealloc( log_memory* pEntry, bool UpdateEntry )
{
    u32 OldAddress = pEntry->GetOldAddress();

    // Realloc with OldAddress of 0 or ZEROPTR is like a malloc
    if( (OldAddress == 0) )
    {
        ApplyOperationMalloc( pEntry, UpdateEntry );
    }
    else
    {
        if( pEntry->GetSize() == 0 )
        {
            ASSERTS( 0, "This code has not been tested" );

            // Find the previous allocation in the hash
            u32 Hash  = OldAddress;
            s32 Index = Hash % m_HashTableSize;
            s32 iHash = Index;
            s32 iEntry = -1;
            do
            {
                iEntry = m_pHashTable[iHash];
                if( (iEntry != -1) && (m_Entries[iEntry].Hash == Hash) )
                {
                    break;
                }
                iHash = (iHash+1) % m_HashTableSize;
            } while( iHash != Index );

            ASSERT( m_Entries[iEntry].Hash == Hash );

            // Get old entry and remove from hash
            m_pHashTable[iHash] = -1;
            m_Entries[iEntry].pEntry = NULL;

            // Add Entry to free list
            m_FreeEntries.Append() = iEntry;
        }
        else
        {
            u32 Address     = pEntry->GetAddress();
            u32 Size        = pEntry->GetSize();
            u32 RoundedSize = (Size + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1);

            // Update memory bounds
            if( m_MinAddress > (Address - m_HeaderSize) )
                m_MinAddress = (Address - m_HeaderSize);
            if( m_MaxAddress < (Address + RoundedSize) )
                m_MaxAddress = (Address + RoundedSize);

            // Find the previous allocation in the hash
            u32 Hash  = OldAddress;
            s32 iHash = Hash % m_HashTableSize;
            s32 Index = iHash;
            do
            {
                if( (m_pHashTable[iHash] != -1) &&
                    (m_Entries[m_pHashTable[iHash]].Hash == Hash) )
                {
                    break;
                }
                iHash = (iHash+1) % m_HashTableSize;
            } while( iHash != Index );

            s32 iEntry = m_pHashTable[iHash];
            if( (iEntry != -1) && (m_Entries[iEntry].Hash == Hash) )
            {
                // Get old entry and remove from hash
                entry& Entry = m_Entries[iEntry];
                m_pHashTable[iHash] = -1;

                // Remove old entry and add new entry to current bytes
                m_CurrentBytes -= m_HeaderSize + ((Entry.pEntry->GetSize() + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1));
                m_CurrentBytes += m_HeaderSize + RoundedSize;
                m_MaximumBytes = max( m_MaximumBytes, m_CurrentBytes );

                // Update flags for entries
                if( UpdateEntry )
                {
                    Entry.pEntry->SetFlags( 0, log_memory::flag_memory_active );
                    pEntry->SetFlags( log_memory::flag_memory_active, log_memory::flag_memory_active );
                    pEntry->SetCurrentBytes( m_CurrentBytes );
                }

                // Modify entry
                Entry.pEntry = pEntry;
                Entry.Hash   = Address;

                // Insert modified entry into hash table
                iHash = Entry.Hash % m_HashTableSize;
                while( m_pHashTable[iHash] != -1 )
                {
                    iHash = (iHash+1) % m_HashTableSize;
                }
                m_pHashTable[iHash] = iEntry;
            }
            else
            {
                // Error realloc of a non-existant prior allocation
                pEntry->SetFlags( log_entry::flag_system_error, log_entry::flag_system_error );
            }
        }
    }
}

//==============================================================================

void CHeapState::ApplyOperationFree( log_memory* pEntry, bool UpdateEntry )
{
    // Find the previous allocation in the hash
    u32 Address = pEntry->GetAddress();

    // Check for ZERO_SIZE address
    // TODO: Remove this when new memory manager installed
    if( Address == 0xffffffff )
        return;

    s32 Index   = Address % m_HashTableSize;
    s32 iHash   = Index;
    s32 iEntry  = -1;
    do
    {
        s32 iTestEntry = m_pHashTable[iHash];
        if( (iTestEntry != -1) && (m_Entries[iTestEntry].Hash == Address) )
        {
            iEntry = iTestEntry;
            break;
        }
        iHash = (iHash+1) % m_HashTableSize;
    } while( iHash != Index );

    if( UpdateEntry && (iEntry == -1) )
    {
        pEntry->SetFlags( log_memory::flag_system_error, log_memory::flag_system_error );
    }
    else
    {
        if( iEntry != -1 )
        {
            ASSERT( m_Entries[iEntry].Hash == Address );

            // Remove old entry from current bytes
            m_CurrentBytes -= m_HeaderSize + ((m_Entries[iEntry].pEntry->GetSize() + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1));

            // Update flags for entry
            if( UpdateEntry )
            {
                m_Entries[iEntry].pEntry->SetFlags( 0, log_memory::flag_memory_active );
                pEntry->SetSize( m_Entries[iEntry].pEntry->GetSize() );
                pEntry->SetCurrentBytes( m_CurrentBytes );
            }

            // Get old entry and remove from hash
            m_pHashTable[iHash] = -1;
            m_Entries[iEntry].pEntry = NULL;

            // Add Entry to free list
            m_FreeEntries.Append() = iEntry;
        }
    }
}

//==============================================================================

void CHeapState::ApplyOperation( log_memory* pEntry, bool UpdateEntry )
{
    switch( pEntry->GetOperation() )
    {
    case xtool::LOG_MEMORY_MALLOC:
        ApplyOperationMalloc( pEntry, UpdateEntry );
        break;
    case xtool::LOG_MEMORY_REALLOC:
        ApplyOperationRealloc( pEntry, UpdateEntry );
        break;
    case xtool::LOG_MEMORY_FREE:
        ApplyOperationFree( pEntry, UpdateEntry );
        break;
    case xtool::LOG_MEMORY_MARK:
        break;
    default:
        ASSERT( 0 );
        break;
    }
}

//==============================================================================

void CHeapState::ApplyOperations( log_memory** pEntries, s32 nEntries, bool UpdateEntry )
{
    while( nEntries-- > 0 )
    {
        log_memory* pEntry = *pEntries++;
        switch( pEntry->GetOperation() )
        {
        case xtool::LOG_MEMORY_MALLOC:
            ApplyOperationMalloc( pEntry, UpdateEntry );
            break;
        case xtool::LOG_MEMORY_REALLOC:
            ApplyOperationRealloc( pEntry, UpdateEntry );
            break;
        case xtool::LOG_MEMORY_FREE:
            ApplyOperationFree( pEntry, UpdateEntry );
            break;
        case xtool::LOG_MEMORY_MARK:
            break;
        default:
            ASSERT( 0 );
            break;
        }
    }
}

//==============================================================================

void CHeapState::SetMinAddress( u32 Address )
{
    m_MinAddress = Address;
}

//==============================================================================

void CHeapState::SetMaxAddress( u32 Address )
{
    m_MaxAddress = Address;
}

//==============================================================================

s32 CHeapState::GetNumActiveAllocations( void )
{
    s32 NumActiveAllocations = m_Entries.GetCount() - m_FreeEntries.GetCount();
    return NumActiveAllocations;
}

//==============================================================================

s32 CHeapState::GetActiveAllocations( xarray<log_memory*>& Array )
{
    s32 NumActiveAllocations = m_Entries.GetCount() - m_FreeEntries.GetCount();

    Array.Clear();
    Array.SetCapacity( NumActiveAllocations );
#if 0
    static s32 Toggle = 0;
    Toggle = !Toggle;
    if( Toggle )
    {
        for( s32 i=m_Entries.GetCount()-1 ; i>=0 ; i-- )
        {
            if( m_Entries[i].pEntry )
                Array.Append() = m_Entries[i].pEntry;
        }
    }
    else
    {
        for( s32 i=0 ; i<m_Entries.GetCount() ; i++ )
        {
            if( m_Entries[i].pEntry )
                Array.Append() = m_Entries[i].pEntry;
        }
    }
#else
    for( s32 i=0 ; i<m_HashTableSize ; i++ )
    {
        if( m_pHashTable[i] != -1 )
            Array.Append() = m_Entries[m_pHashTable[i]].pEntry;
    }
#endif

    return NumActiveAllocations;
}

//==============================================================================

void CHeapState::Clear( void )
{
    m_Entries.SetCount( 0 );
    m_FreeEntries.SetCount( 0 );

    memset( m_pHashTable, 0xff, m_HashTableSize * sizeof(s32) );

    m_HeaderSize    = MEMORY_HEADER_SIZE;
    m_MinAddress    = U32_MAX;
    m_MaxAddress    = U32_MIN;
    m_CurrentBytes  = 0;
    m_MaximumBytes  = 0;
}

//==============================================================================

log_memory* CHeapState::GetPrevEntry( log_memory* pEntry )
{
    log_memory* pTop    = NULL;
    u32         TopAddress = 0;
    log_memory* pNext   = NULL;
    u32         BestDelta = U32_MAX;

    u32 Address = pEntry->GetAddress();
    for( s32 i=0 ; i<m_Entries.GetCount() ; i++ )
    {
        log_memory* pTest = m_Entries[i].pEntry;
        if( pTest )
        {
            u32 TestAddress = pTest->GetAddress();
            if( TestAddress < Address )
            {
                u32 Delta = Address - TestAddress;
                if( Delta < BestDelta )
                {
                    BestDelta = Delta;
                    pNext = pTest;
                }
            }
            if( TestAddress > TopAddress )
            {
                TopAddress = TestAddress;
                pTop = pTest;
            }
        }
    }

    if( pNext )
        return pNext;
    return pTop;
}

//==============================================================================

log_memory* CHeapState::GetNextEntry( log_memory* pEntry )
{
    log_memory* pBot    = NULL;
    u32         BotAddress = U32_MAX;
    log_memory* pNext   = NULL;
    u32         BestDelta = U32_MAX;

    u32 Address = pEntry->GetAddress();
    for( s32 i=0 ; i<m_Entries.GetCount() ; i++ )
    {
        log_memory* pTest = m_Entries[i].pEntry;
        if( pTest )
        {
            u32 TestAddress = pTest->GetAddress();
            if( TestAddress > Address )
            {
                u32 Delta = TestAddress - Address;
                if( Delta < BestDelta )
                {
                    BestDelta = Delta;
                    pNext = pTest;
                }
            }
            if( TestAddress < BotAddress )
            {
                BotAddress = TestAddress;
                pBot = pTest;
            }
        }
    }

    if( pNext )
        return pNext;
    return pBot;
}

/////////////////////////////////////////////////////////////////////////////

log_memory* CHeapState::AddressToLogEntry( u32 Address )
{
    // Scan existing list of blocks
    for( s32 i=0 ; i<m_HashTableSize ; i++ )
    {
        s32 Index = m_pHashTable[i];
        if( Index != -1 )
        {
            log_memory* pEntry = m_Entries[Index].pEntry;
            if( pEntry )
            {
                u32 BlockMinAddress = pEntry->GetAddress() - MEMORY_HEADER_SIZE;
                u32 BlockMaxAddress = (pEntry->GetAddress() + (pEntry->GetSize() + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1));
                if( (BlockMinAddress <= Address) && (BlockMaxAddress >= Address) )
                    return pEntry;
            }
        }
    }

    // No block found
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

log_memory* CHeapState::IsRangeOccupied( u32 Address, u32 Length )
{
    u32 End = Address + Length;

    // Scan existing list of blocks
    for( s32 i=0 ; i<m_HashTableSize ; i++ )
    {
        s32 Index = m_pHashTable[i];
        if( Index != -1 )
        {
            log_memory* pEntry = m_Entries[Index].pEntry;
            if( pEntry )
            {
                u32 BlockMinAddress = pEntry->GetAddress() - MEMORY_HEADER_SIZE;
                u32 BlockMaxAddress = (pEntry->GetAddress() + (pEntry->GetSize() + (MEMORY_GRANULARITY-1)) & ~(MEMORY_GRANULARITY-1));
                if( (End > BlockMinAddress) && (Address < BlockMaxAddress) )
                    return pEntry;
            }
        }
    }

    // No block found
    return NULL;
}

//==============================================================================

xbool CHeapState::IsEntryActive( log_memory* pEntry )
{
    ASSERT( (pEntry->GetOperation() == xtool::LOG_MEMORY_MALLOC) ||
            (pEntry->GetOperation() == xtool::LOG_MEMORY_REALLOC) );

    u32     Address = pEntry->GetAddress();
    s32     Index   = Address % m_HashTableSize;
    s32     iHash   = Index;
    s32     iEntry  = -1;
    xbool   Found   = FALSE;
    do
    {
        iEntry = m_pHashTable[iHash];
        if( (iEntry != -1) && (m_Entries[iEntry].Hash == Address) )
        {
            Found = TRUE;
        }
        iHash = (iHash+1) % m_HashTableSize;
    } while( iHash != Index );

    return Found;
}

//==============================================================================

void CHeapState::Serialize(CArchive& ar)
{
    CObject::Serialize( ar );

	if( ar.IsStoring() )
	{
        ar << m_MinAddress;
        ar << m_MaxAddress;
	}
	else
	{
        ar >> m_MinAddress;
        ar >> m_MaxAddress;

        // Rebuild the hash table
        ResizeHashTable( m_Entries.GetCount()*2 );
	}
}

//==============================================================================
