//==============================================================================
//
//  SpatialDBase.hpp
//
//==============================================================================

#ifndef SPATIAL_DBASE_HPP
#define SPATIAL_DBASE_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_stdio.hpp"
#include "x_math.hpp"
#include "spatial_user.hpp"     // User must provided this file

//==============================================================================
// DEFINES
//==============================================================================
#define SPATIAL_CELL_NULL        ((u16)0xFFFF)

//==============================================================================
// SPATIAL_DBASE
//==============================================================================
class spatial_cell : public spacial_user
{
public:
    spatial_cell( void )
    {
        X = Y = Z    =  0;
        Level        =  0;
        SearchSeq    = -1;
        OccFlags     =  0;
        Flags        =  0;
        HashNext     = SPATIAL_CELL_NULL;
        HashPrev     = SPATIAL_CELL_NULL;
        Parent       = SPATIAL_CELL_NULL;
        Child        = SPATIAL_CELL_NULL;
        Next         = SPATIAL_CELL_NULL;
        Prev         = SPATIAL_CELL_NULL;
        SearchNext   = SPATIAL_CELL_NULL;
    }

    u32     OccFlags;           // Is there something in it

    enum flags
    {
        FLAG_SKIN_PLANES = XBIN(111111)
    };
    inline u32 GetFlags( void ) {return Flags;}
protected: 

    s16     X,Y,Z;              // Actual position of the cell in the world
    u8      Level;              // Level in the Hirarchy tree
    u8      Flags;              // Flags
    u16     HashNext;           // Next node in the hash entry
    u16     HashPrev;           // Previous node in the hash entry
    u16     Parent;             // Link to the Parent in the tree 
    u16     Child;              // Link to its first Child  in the tree 
    u16     Next;               // Link to the next node in the tree
    u16     Prev;               // Link to the Previous node in the tree

    u16     SearchNext;         // Links all visible nodes
    s32     SearchSeq;          // Sequence for the search

private:
    // Not copy allow
    const spatial_cell& operator = ( const spatial_cell& Cell ) 
    {(void)Cell;return *this;}



    friend class spatial_dbase;
};

//==============================================================================
// SPATIAL_DBASE
//==============================================================================
class view;

class spatial_dbase
{
//------------------------------------------------------------------------------
//  Public Functions
//------------------------------------------------------------------------------
public:

                        spatial_dbase   ( void );
                       ~spatial_dbase   ( void );

        void            Init            ( f32 MinCellWidth );
        void            Kill            ( void );
        void            SanityCheck     ( void );
        void            DumpStats       ( void ) const;
        void            Render          ( s32 MinLevel, s32 MaxLevel ) const;



        spatial_cell*   GetCell         ( s32 X, s32 Y, s32 Z, s32 Level, xbool Create=FALSE );
        spatial_cell&   GetCell         ( u16 ID ) const;
        u16             GetCellIndex    ( s32 X, s32 Y, s32 Z, s32 Level, xbool Create=FALSE );

        bbox            GetCellBBox     ( s32 X, s32 Y, s32 Z, s32 Level ) const;
        bbox            GetCellBBox     ( const spatial_cell& Cell ) const;
        void            UpdateCell      ( spatial_cell& pCell );

        void            GetCellRegion   ( const bbox& BBox, s32 Level,
                                                s32&  MinX, s32 &MinY, s32& MinZ,
                                                s32&  MaxX, s32 &MaxY, s32& MaxZ ) const;

        s32             GetBBoxLevel    ( const bbox& BBox ) const;
        s32             GetCellCount    ( void ) { return m_nCells; }

        u16             BuildVisList    ( const view& View, xbool DoCulling );
        u16             TraverseCells   ( const bbox& BBox, u32 OccFlags = 0xFFFFFFFF );
        u16             TraverseCells   ( const vector3& RayStart,
                                          const vector3& RayEnd,
                                                u32      OccFlags = 0xFFFFFFFF );

        u16             GetFirstInSearch( void   );
        u16             GetNextInSearch ( u16 ID );


//------------------------------------------------------------------------------
// TYPES
//------------------------------------------------------------------------------
protected:

        enum 
        {
            MAX_LEVELS  =   12,
            HASH_SIZE   =   16001,
        };

        struct hash
        {
            u16             FirstCell;
            s16             nCells;
        };

//------------------------------------------------------------------------------
//  Private Functions
//------------------------------------------------------------------------------

public:
        void            Clear       ( void );

protected:
        s32             ComputeHash ( s32 X, s32 Y, s32 Z, s32 Level ) const;
        u16             AllocCell   ( s32 X, s32 Y, s32 Z, s32 Level );
        void            FreeCell    ( u16 ID );

//------------------------------------------------------------------------------
//  Private Data
//------------------------------------------------------------------------------
protected:

        spatial_cell*   m_pCell;                    // Pointer of nodes
        s32             m_nCells;                   // Number of nodes in the tree
        s32             m_nCellsAllocated;          // How many total nodes we have allocated

        s32             m_SearchSeq;                // Sequence use to skip nodes in searches
        u16             m_FirstFree;                // Link list of free nodes
        u16             m_FirstSearch;              // Link list of nodes for the active search
        s32             m_nSearchNodes;             // Number of 

        f32             m_CellWidth    [ MAX_LEVELS ];  // Precomputer cell sizes
        s32             m_nCellsInLevel[ MAX_LEVELS ];  // Counts of cells in levels
        hash            m_Hash         [ HASH_SIZE  ];  // Hash of active cells

};

//==============================================================================

#include "SpatialDBase_inline.hpp"


extern spatial_dbase g_SpatialDBase;
//==============================================================================
#endif // SPATIALDBASE_HPP
//==============================================================================

