//==============================================================================
//
//  RecentList.cpp
//
//==============================================================================

#include "stdafx.h"

#include "RecentList.h"

//==============================================================================

IMPLEMENT_DYNCREATE( CRecentList, CObject );

//==============================================================================

#define RECENT_DEFAULT_MAX  10

//==============================================================================

CRecentList::CRecentList()
{
    // Set default max items
    m_MaxItems = RECENT_DEFAULT_MAX;
}

//==============================================================================

CRecentList::~CRecentList()
{
}

//==============================================================================

void CRecentList::SetMaxItems( s32 Max )
{
    // Set the max
    m_MaxItems = max(1,Max);

    // Delete any items above the max
    while( m_Array.GetSize() > m_MaxItems )
        m_Array.RemoveAt( m_Array.GetSize()-1 );
}

//==============================================================================

s32 CRecentList::Find( const char* pString ) const
{
    // Loop over array
    for( int i=0 ; i<m_Array.GetSize() ; i++ )
    {
        // Check for match
        if( m_Array[i] == pString )
            return i;
    }

    // Return not found
    return -1;
}

//==============================================================================

void CRecentList::Add( const char* pString )
{
    int iItem = Find( pString );

    // Remove existing item from list
    if( iItem != -1 )
    {
        m_Array.RemoveAt( iItem );
    }

    // Insert at head of list
    m_Array.InsertAt( 0, pString );

    // If count > max items then delete last item
    if( m_Array.GetSize() > m_MaxItems )
        m_Array.RemoveAt( m_Array.GetSize()-1 );
}

//==============================================================================

CString CRecentList::GetString( s32 iEntry ) const
{
    ASSERT( (iEntry >= 0) && (iEntry < m_Array.GetSize()) );
    return m_Array[iEntry];
}

//==============================================================================

s32 CRecentList::GetCount( void ) const
{
    return m_Array.GetSize();
}

//==============================================================================

void CRecentList::Serialize(CArchive& ar)
{
    CObject::Serialize( ar );

	if( ar.IsStoring() )
	{
        m_Array.Serialize( ar );
	}
	else
	{
        m_Array.Serialize( ar );
	}
}

//==============================================================================

void CRecentList::LoadRegistry( LPCTSTR pSection )
{
    CXTRegistryManager regManager;
	m_MaxItems = regManager.GetProfileInt( pSection, _T("Max Items"), RECENT_DEFAULT_MAX);
	int Count  = regManager.GetProfileInt( pSection, _T("Count"), 0);
    for( int i=0 ; i<Count ; i++ )
    {
        CString Entry;
        Entry.Format( "%d", i );
        CString String = regManager.GetProfileString( pSection, Entry, _T("") );
        m_Array.InsertAt( i, String );
    }
}

//==============================================================================

void CRecentList::SaveRegistry( LPCTSTR pSection )
{
    CXTRegistryManager regManager;
	regManager.WriteProfileInt( pSection, _T("Max Items"), m_MaxItems );
	regManager.WriteProfileInt( pSection, _T("Count"), m_Array.GetSize() );
    for( int i=0 ; i<m_Array.GetSize() ; i++ )
    {
        CString Entry;
        Entry.Format( "%d", i );
        regManager.WriteProfileString( pSection, Entry, m_Array[i] );
    }
}

//==============================================================================
