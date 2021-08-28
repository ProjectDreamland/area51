//==============================================================================
//  
//  guid.hpp
//
//==============================================================================

#ifndef GUID_HPP
#define GUID_HPP

//==============================================================================

#include "x_types.hpp"
#include "x_string.hpp"
#include "x_bitstream.hpp"

//==============================================================================

void    guid_Init               ( void );
void    guid_Kill               ( void );

void    guid_StoreSequence      ( bitstream& BitStream );
void    guid_RestoreSequence    ( bitstream& BitStream );
void    guid_DumpSequence       ( bitstream& BitStream );

u64     guid_GetSequence        ( void );

guid    guid_New                ( void );

//==============================================================================

xstring guid_ToString   ( guid GUID );
guid    guid_FromString ( const char* pGUID );

//==============================================================================

class guid_lookup
{
public:

             guid_lookup    ( void );
            ~guid_lookup    ( void );

    void    SetCapacity     ( s32 NGuids, xbool CanGrow );

    s32     GetNGUIDS       ( void ) { return m_nNodes; }
    s32     GetNBytes       ( void );
    s32     GetCapacity     ( void ) { return m_nNodesAllocated; }

    void    Add             ( guid GUID, u32    Data );
    void    Add             ( guid GUID, s32    Data );
    void    Add             ( guid GUID, void*  Data );
    
    xbool   Find            ( guid GUID );
    xbool   Find            ( guid GUID, u32&   Data );
    xbool   Find            ( guid GUID, s32&   Data );
    xbool   Find            ( guid GUID, void*& Data );


    s32     GetIndex        ( guid GUID  );
    u32&    GetU32          ( s32 Index );
    s32&    GetS32          ( s32 Index );
    void*&  GetP32          ( s32 Index );


    void    Del             ( guid GUID );
    void    Clear           ( void );

    void    Dump            ( const char* pFileName );

    void    SanityCheck     ( void );

protected:

    struct node
    {
        guid    GUID;
        u32     Data;
        s32     Next;
        s32     Prev;
        u32     PAD;
    };

    s32     m_nNodes;
    s32     m_nNodesAllocated;
    node*   m_pNode;
    s32     m_FirstFreeNode;
    s32     m_nHashEntries;
    s32*    m_pHashEntry;
    xbool   m_CanGrow;


    void    Resize          ( s32 NGuids );
    s32     AllocNode       ( void );
    void    DeallocNode     ( s32 Node );
    s32     GetHash         ( guid GUID );
};

//==============================================================================

inline s32 guid_lookup::GetHash( guid GUID )
{
    // this requires a 64-bit divide which is expensive on the PS2
    //s32 H = (s32)(GUID % m_nHashEntries);
    // instead, we'll use folding so that we only require a 32-bit divide
    s32 H = (s32)((u32)(GUID.GetLow() + GUID.GetHigh()) % m_nHashEntries);
    return H;
}

//==============================================================================

inline s32 guid_lookup::GetIndex( guid GUID  )
{
    if( m_nHashEntries == 0 ) return -1;

    // Compute first node in hash table
    s32 I = m_pHashEntry[GetHash(GUID)];

    // Check linked list for match
    while( I != -1 )
    {
        if( m_pNode[I].GUID == GUID )
            return I;
        I = m_pNode[I].Next;
    }

    // Not in list
    return -1;
}

//==============================================================================
#endif