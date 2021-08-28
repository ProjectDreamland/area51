//==============================================================================
//
//  SpatialDBase_inline.hpp
//
//==============================================================================

//==============================================================================

inline
spatial_cell& spatial_dbase::GetCell( u16 ID ) const
{
    ASSERT( ID != SPATIAL_CELL_NULL );
    ASSERT( ID < m_nCellsAllocated );
    return m_pCell[ ID ];
}

//==============================================================================
inline
u16 spatial_dbase::GetFirstInSearch( void ) 
{
    return m_FirstSearch;
}


//==============================================================================
inline
u16 spatial_dbase::GetNextInSearch( u16 ID ) 
{
    ASSERT( ID < m_nCellsAllocated );
    ASSERT( (m_pCell[ ID ].SearchNext == SPATIAL_CELL_NULL) || 
            ( ID < m_nCellsAllocated ) );

    return m_pCell[ ID ].SearchNext;
}

//==============================================================================

inline
spatial_cell* spatial_dbase::GetCell( s32 X, s32 Y, s32 Z, s32 Level, xbool Create )
{
    u16 ID = GetCellIndex( X, Y, Z, Level, Create );
    if( ID == SPATIAL_CELL_NULL )
        return NULL;
    return &GetCell( ID );
}

//==============================================================================

inline
s32 spatial_dbase::ComputeHash( s32 X, s32 Y, s32 Z, s32 Level ) const
{
    // Compute hash index
    s32 H = ((u32)((((((X<<10)+Y)<<10)+Z)<<10)+Level)) % HASH_SIZE;
    ASSERT( (H>=0) && (H<HASH_SIZE) );
    return H;
}

//==============================================================================

inline
bbox spatial_dbase::GetCellBBox( s32 X, s32 Y, s32 Z, s32 Level ) const
{
    const f32 W = m_CellWidth[ Level ];

    return bbox( vector3(X*W,   Y*W,   Z*W),
                 vector3(X*W+W, Y*W+W, Z*W+W) );
}

//==============================================================================

inline
bbox spatial_dbase::GetCellBBox( const spatial_cell& Cell ) const
{
    return GetCellBBox( Cell.X, Cell.Y, Cell.Z, Cell.Level );
}

//==============================================================================

inline
s32 spatial_dbase::GetBBoxLevel( const bbox& BBox ) const
{
    // Get largest dimension...treat it as a cube
    vector3 Size = BBox.GetSize();
    f32     S    = fMax( fMax(Size.GetX(), Size.GetY()), Size.GetZ());

    // Find level such that S <= CellWidth
    for( s32 i=0; i<MAX_LEVELS; i++ )
    if( S <= m_CellWidth[i] )
    {
        // Push objects that are close to the size of the cell
        // to the next level
        if( x_abs(S-m_CellWidth[i]) < 2.0f )
        {
            i++;
            ASSERT( i<MAX_LEVELS );
        }

        return i;
    }

    // Object too big or too few levels
    
    x_throw( "Object too big or too few levels" );
    return 0;
}

//==============================================================================

inline
void spatial_dbase::GetCellRegion( 
    const bbox& BBox, 
    s32         Level, s32& MinX, s32& MinY, s32& MinZ,
                       s32& MaxX, s32& MaxY, s32& MaxZ ) const
{
    const f32 W = 1.0f / m_CellWidth[Level];

    MinX = (s32)(( BBox.Min.GetX() * W )+1024.0f)-1024;
    MinY = (s32)(( BBox.Min.GetY() * W )+1024.0f)-1024;
    MinZ = (s32)(( BBox.Min.GetZ() * W )+1024.0f)-1024;
    MaxX = (s32)(( BBox.Max.GetX() * W )+1024.0f)-1024;
    MaxY = (s32)(( BBox.Max.GetY() * W )+1024.0f)-1024;
    MaxZ = (s32)(( BBox.Max.GetZ() * W )+1024.0f)-1024;

/*
    ASSERT( (MaxX-MinX) <= 1 );
    ASSERT( (MaxY-MinY) <= 1 );
    ASSERT( (MaxZ-MinZ) <= 1 );

    ASSERT( BBox.Max.X <= (MaxX+1)*m_CellWidth[Level] );
    ASSERT( BBox.Min.X >= MinX*m_CellWidth[Level] );
    ASSERT( BBox.Max.Y <= (MaxY+1)*m_CellWidth[Level] );
    ASSERT( BBox.Min.Y >= MinY*m_CellWidth[Level] );
    ASSERT( BBox.Max.Z <= (MaxZ+1)*m_CellWidth[Level] );
    ASSERT( BBox.Min.Z >= MinZ*m_CellWidth[Level] );
*/

}

//==============================================================================

inline
u16 spatial_dbase::GetCellIndex( s32 X, s32 Y, s32 Z, s32 Level, xbool Create )
{
    // Compute hash entry
    s32 H = ComputeHash( X, Y, Z, Level );

    // Loop through hash entries and look for a match
    u16 ID = m_Hash[ H ].FirstCell;
    while( ID != SPATIAL_CELL_NULL )
    {
        const spatial_cell& Cell = GetCell( ID );

        if( (Cell.X     == X) && 
            (Cell.Y     == Y) && 
            (Cell.Z     == Z) && 
            (Cell.Level == Level) )
        {
            return ID;
        }

        ID = Cell.HashNext;
    }   

    // Could not find cell and caller requested to create one
    if( Create )
    {
        ID = AllocCell( X, Y, Z, Level );
        return ID;
    }

    // Could not find cell and caller did not request to create one
    // so just return
    return SPATIAL_CELL_NULL;
}

//==============================================================================

inline
void spatial_dbase::UpdateCell( spatial_cell& Cell )
{
    ASSERT( (&Cell >= m_pCell) && (&Cell < (&m_pCell[m_nCellsAllocated])) );

    if( (Cell.OccFlags == 0) && (Cell.Child==SPATIAL_CELL_NULL) )
    {
        s32 Index = (((byte*)&Cell) - (byte*)m_pCell) / ((s32)sizeof(spatial_cell));
        ASSERT( Index >= 0 && Index < m_nCellsAllocated );
        FreeCell( Index );
    }
}

//==============================================================================
