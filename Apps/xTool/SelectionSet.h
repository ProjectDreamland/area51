//==============================================================================
//
//  SelectionSet.h
//
//==============================================================================

#ifndef SELECTIONSET_H
#define SELECTIONSET_H

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
//  CLASS CSelectionSet
//==============================================================================

class CSelectionSet : public CObject
{
public:
    struct entry
    {
        s32         Start;                  // Start of range
        s32         End;                    // End of range
    };

protected:
    xarray<entry>   m_Entries;              // Array of entries
    xbool           m_IterateBegun;         // Iteration has begun
    s32             m_IterateEntry;         // Entry currently iterating on
    s32             m_IterateItem;          // Item currently iterating on

    void            Sort            ( void );
    void            Coalesce        ( void );

public:
                    CSelectionSet   ( );
                   ~CSelectionSet   ( );

    DECLARE_DYNCREATE(CSelectionSet)

    void            Clear           ( void );                       // Clear the selection set

    void            Select          ( s32 Start, s32 End );         // Select items
    void            Deselect        ( s32 Start, s32 End );         // Deselect items

    xbool           IsSelected      ( s32 Item );                   // Is an item selected?

    xbool           BeginIterate    ( s32& Item );                  // Begin an iteration over the items
    xbool           Iterate         ( s32& Item );                  // Iterate over the items

    void            Serialize       ( CArchive& ar );
};

//==============================================================================
#endif // SELECTIONSET_H
//==============================================================================
