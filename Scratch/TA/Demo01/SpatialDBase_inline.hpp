//==============================================================================
//
//  SpatialDBase_inline.hpp
//
//==============================================================================

//==============================================================================

inline
spatial_cell* spatial_dbase::GetCell( u16 ID ) const
{
    if( ID == SPATIAL_CELL_NULL )
        return NULL;

    ASSERT( ID >= 0 );
    ASSERT( ID < m_nCellsAllocated );
    return &m_pCell[ ID ];
}

//==============================================================================
inline
u16 spatial_dbase::GetNextInSearch( u16 ID ) 
{
    ASSERT( ID >= 0 );
    ASSERT( ID < m_nCellsAllocated );
    ASSERT( (m_pCell[ ID ].SearchNext == SPATIAL_CELL_NULL) || 
            ( m_pCell[ ID ].SearchNext >= 0 && ID < m_nCellsAllocated ) );

    return m_pCell[ ID ].SearchNext;
}

//==============================================================================

inline
spatial_cell* spatial_dbase::GetCell( s32 X, s32 Y, s32 Z, s32 Level, xbool Create )
{
    u16 ID = GetCellIndex( X, Y, Z, Level, Create );
    return GetCell( ID );
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

    bbox BBox;
    BBox.Min.X = X*W;
    BBox.Min.Y = Y*W;
    BBox.Min.Z = Z*W;
    BBox.Max.X = BBox.Min.X + W;
    BBox.Max.Y = BBox.Min.Y + W;
    BBox.Max.Z = BBox.Min.Z + W;

    return BBox;
}

//==============================================================================

inline
bbox spatial_dbase::GetCellBBox( spatial_cell& Cell ) const
{
    f32 W = m_CellWidth[ Cell.Level ];

    bbox BBox;
    BBox.Min.X = Cell.X*W;
    BBox.Min.Y = Cell.Y*W;
    BBox.Min.Z = Cell.Z*W;
    BBox.Max.X = BBox.Min.X + W;
    BBox.Max.Y = BBox.Min.Y + W;
    BBox.Max.Z = BBox.Min.Z + W;

    return BBox;
}

//==============================================================================

inline
s32 spatial_dbase::GetBBoxLevel( const bbox& BBox ) const
{
    // Get largest dimension...treat it as a cube
    vector3 Size = BBox.GetSize();
    f32     S    = fMax( fMax(Size.X, Size.Y), Size.Z);

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
    ASSERTS( FALSE, "Object too big or too few levels" );
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

    MinX = (s32)(( BBox.Min.X * W )+1024.0f)-1024;
    MinY = (s32)(( BBox.Min.Y * W )+1024.0f)-1024;
    MinZ = (s32)(( BBox.Min.Z * W )+1024.0f)-1024;
    MaxX = (s32)(( BBox.Max.X * W )+1024.0f)-1024;
    MaxY = (s32)(( BBox.Max.Y * W )+1024.0f)-1024;
    MaxZ = (s32)(( BBox.Max.Z * W )+1024.0f)-1024;

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
        const spatial_cell* pCell = GetCell( ID );
        ASSERT( pCell );

        if( (pCell->X     == X) && 
            (pCell->Y     == Y) && 
            (pCell->Z     == Z) && 
            (pCell->Level == Level) )
        {
            return ID;
        }

        ID = pCell->HashNext;
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
        s32 Index = (s32)(&m_pCell[m_nCellsAllocated] - &Cell);
        ASSERT( Index == ((((byte*)&Cell) - (byte*)m_pCell) / (s32)sizeof(spatial_cell)) );
        FreeCell( Index );
        //FreeCell( (((byte*)pCell) - m_pCell) / m_nBytesPerCell );
    }
}

//==============================================================================
