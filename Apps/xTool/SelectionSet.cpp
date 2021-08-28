//==============================================================================
//
//  SelectionSet.cpp
//
//==============================================================================

#include "stdafx.h"

#include "SelectionSet.h"

//==============================================================================

IMPLEMENT_DYNCREATE( CSelectionSet, CObject );

//==============================================================================

CSelectionSet::CSelectionSet()
{
    m_IterateBegun = FALSE;
}

//==============================================================================

CSelectionSet::~CSelectionSet()
{
}

//==============================================================================

int fn_SortEntries( const void* p1, const void* p2 )
{
    CSelectionSet::entry* pEntry1 = (CSelectionSet::entry*)p1;
    CSelectionSet::entry* pEntry2 = (CSelectionSet::entry*)p2;

    return pEntry1->Start - pEntry2->Start;
}

void CSelectionSet::Sort( void )
{
    x_qsort( m_Entries.GetPtr(), m_Entries.GetCount(), sizeof(entry), fn_SortEntries );
}

//==============================================================================

void CSelectionSet::Coalesce( void )
{
    s32 i = 0;

    while( i < (m_Entries.GetCount()-1) )
    {
        entry& Entry1 = m_Entries[i  ];
        entry& Entry2 = m_Entries[i+1];

        // Merge 2 entries?
        if( Entry2.Start <= Entry1.End )
        {
            ASSERT( Entry1.Start <= Entry2.Start );
            Entry1.End = MAX( Entry1.End, Entry2.End );
            m_Entries.Delete( i+1 );
        }
        else
        {
            i++;
        }
    }
}

//==============================================================================

void CSelectionSet::Clear( void )
{
    m_IterateBegun = FALSE;

    m_Entries.SetCount( 0 );
}

//==============================================================================

void CSelectionSet::Select( s32 Start, s32 End )
{
    m_IterateBegun = FALSE;

    ASSERT( Start <= End );

    // Create a new entry
    entry& Entry = m_Entries.Append();
    Entry.Start = Start;
    Entry.End   = End;

    // Sort & Coalesce to resolve any overlaps introduced
    Sort();
    Coalesce();
}

//==============================================================================

void CSelectionSet::Deselect( s32 Start, s32 End )
{
    m_IterateBegun = FALSE;

    ASSERT( Start <= End );

    s32 i=0 ;

    // Loop through the entries deselecting the block from Start to End
    while( i < m_Entries.GetCount() )
    {
        entry& Entry = m_Entries[i];

        // Is the block totally deselected?
        if( (Start <= Entry.Start) && (End >= Entry.End) )
        {
            m_Entries.Delete( i );
            continue;
        }

        // Start of block deselected?
        if( (Start <= Entry.Start) && (End >= Entry.Start) )
        {
            Entry.Start = End+1;
            ASSERT( Entry.Start <= Entry.End );

            i++;
            continue;
        }

        // End of block deselected?
        if( (Start <= Entry.End) && (End >= Entry.End ) )
        {
            Entry.End = Start-1;
            ASSERT( Entry.Start <= Entry.End );

            i++;
            continue;
        }

        // Need to split the block
        if( (Start > Entry.Start) && (End < Entry.End) )
        {
            entry NewEntry;
            NewEntry.Start = End+1;
            NewEntry.End   = Entry.End;
            Entry.End      = Start-1;

            ASSERT(    Entry.Start <=    Entry.End );
            ASSERT( NewEntry.Start <= NewEntry.End );

            m_Entries.Insert( i+1, NewEntry );

            i += 2;
            continue;
        }

        // Next block
        i++;
    }
}

//==============================================================================

xbool CSelectionSet::IsSelected( s32 Item )
{
    // TODO: Add a binary search here for speed
    for( s32 i=0 ; i<m_Entries.GetCount() ; i++ )
    {
        entry& Entry = m_Entries[i];

        if( (Entry.Start <= Item) && (Entry.End >= Item) )
            return TRUE;
    }

    return FALSE;
}

//==============================================================================

xbool CSelectionSet::BeginIterate( s32& Item )
{
    if( m_Entries.GetCount() > 0 )
    {
        m_IterateEntry = 0;
        m_IterateItem  = m_Entries[0].Start;
        m_IterateBegun = TRUE;
    }
    else
    {
        m_IterateEntry = 0;
        m_IterateItem  = -1;
        m_IterateBegun = FALSE;
    }

    Item = m_IterateItem;

    return m_IterateBegun;
}

//==============================================================================

xbool CSelectionSet::Iterate( s32& Item )
{
    ASSERT( Item == m_IterateItem );

    if( m_IterateBegun )
    {
        m_IterateItem++;
        if( m_IterateItem > m_Entries[m_IterateEntry].End )
        {
            m_IterateEntry++;
            if( m_IterateEntry >= m_Entries.GetCount() )
            {
                m_IterateBegun = FALSE;
                m_IterateItem = -1;
            }
            else
            {
                m_IterateItem = m_Entries[m_IterateEntry].Start;
            }
        }
    }

    Item = m_IterateItem;

    return m_IterateBegun;
}

//==============================================================================

void CSelectionSet::Serialize(CArchive& ar)
{
    int i;

    CObject::Serialize( ar );

	if( ar.IsStoring() )
	{
        ar << (int)m_Entries.GetCount();

        // Write the entries
        for( i=0 ; i<m_Entries.GetCount() ; i++ )
        {
            ar << m_Entries[i].Start;
            ar << m_Entries[i].End;
        }
	}
	else
	{
        // Delete existing entries
        m_Entries.SetCount( 0 );

        // Read number of entries
        int NumEntries;
        ar >> NumEntries;

        // Read the entries
        for( i=0 ; i<NumEntries ; i++ )
        {
            int Start;
            int End;
            ar >> Start;
            ar >> End;
            entry& Entry = m_Entries.Append();
            Entry.Start = Start;
            Entry.End   = End;
        }
	}
}

//==============================================================================
