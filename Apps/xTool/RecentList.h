//==============================================================================
//
//  RecentList.h
//
//==============================================================================

#ifndef RECENTLIST_H
#define RECENTLIST_H

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
//  CLASS CRecentList
//==============================================================================

class CRecentList : public CObject
{
protected:
    CStringArray    m_Array;
    s32             m_MaxItems;

public:
                    CRecentList      ( );
                   ~CRecentList      ( );

    DECLARE_DYNCREATE(CRecentList)

    void            SetMaxItems     ( s32 Max );                            // Set max items to keep in list
    s32             Find            ( const char* pString ) const;          // Find entry in list, -1 = failed, otherwise index
    void            Add             ( const char* pString );                // Add entry to list, -1 = failed, otherwise index
    CString         GetString       ( s32 iEntry ) const;                   // Get string given index
    s32             GetCount        ( void ) const;                         // Get count of strings

    void            Serialize       ( CArchive& ar );                       // Serialize to file
    void            LoadRegistry    ( LPCTSTR pSection );                   // Load from registry
    void            SaveRegistry    ( LPCTSTR pSection );                   // Save to registry

};

//==============================================================================
#endif // RECENTLIST_H
//==============================================================================
