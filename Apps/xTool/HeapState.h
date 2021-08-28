//==============================================================================
//
//  HeapState.h
//
//==============================================================================

#ifndef HEAP_STATE_H
#define HEAP_STATE_H

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
//  CLASS CHeapState
//==============================================================================

class log_memory;

class CHeapState : public CObject
{
protected:
    struct entry
    {
        log_memory* pEntry;                     // Pointer to log entry
        u32         Hash;                       // Hash key for this entry
    };

    xarray<entry>       m_Entries;              // Array of entries
    xarray<s32>         m_FreeEntries;          // Array of free entries

    s32*                m_pHashTable;           // Pointer to hash table
    s32                 m_HashTableSize;        // Number of entries in hash table (each entry is an s32)

    u32                 m_HeaderSize;           // Size of memory header in bytes
    u32                 m_MinAddress;           // Min address in heap
    u32                 m_MaxAddress;           // Max address in heap
    u32                 m_CurrentBytes;         // Current bytes allocated
    u32                 m_MaximumBytes;         // Maximum bytes allocated

    void                ResizeHashTable         ( s32 NewSize );

    void                ApplyOperationMalloc    ( log_memory* pEntry, bool UpdateEntry );
    void                ApplyOperationRealloc   ( log_memory* pEntry, bool UpdateEntry );
    void                ApplyOperationFree      ( log_memory* pEntry, bool UpdateEntry );

public:
                        CHeapState       ( );
                       ~CHeapState       ( );

    DECLARE_DYNCREATE(CHeapState)

    void                Clear                   ( void );

    void                ApplyOperation          ( log_memory* pEntry, bool UpdateEntry = false );
    void                ApplyOperations         ( log_memory** pEntries, s32 nEntries, bool UpdateEntry = false );
    void                SetMinAddress           ( u32 Address );
    void                SetMaxAddress           ( u32 Address );
    u32                 GetMinAddress           ( void ) { return m_MinAddress; }
    u32                 GetMaxAddress           ( void ) { return m_MaxAddress; }
    s32                 GetNumActiveAllocations ( void );
    s32                 GetActiveAllocations    ( xarray<log_memory*>& Array );
    u32                 GetCurrentBytes         ( void ) { return m_CurrentBytes; }
    u32                 GetMaximumBytes         ( void ) { return m_MaximumBytes; }

    log_memory*         GetPrevEntry            ( log_memory* pEntry );
    log_memory*         GetNextEntry            ( log_memory* pEntry );

    log_memory*         AddressToLogEntry       ( u32 Address );
    log_memory*         IsRangeOccupied         ( u32 Address, u32 Length );
    xbool               IsEntryActive           ( log_memory* pEntry );

    void                Serialize               ( CArchive& ar );
};

//==============================================================================
#endif // HEAP_STATE_H
//==============================================================================
