#include "PlaySurfaceMgr.hpp"

//=========================================================================
// typedefs and structures
//=========================================================================

struct dbase_file_hdr
{
    s32 CellSize;
    s32 nCells;
    s32 nSurfaces;
};

//=========================================================================
// Implementation
//=========================================================================

playsurface_mgr::dbase::dbase( void ) :
    m_QueryNumber(0),
    m_BuildingNewDBase(FALSE),
    m_CellSize( 0 ),
    m_Cells(),
    m_Surfaces()
{
}

//=========================================================================

playsurface_mgr::dbase::~dbase( void )
{
}

//=========================================================================

void playsurface_mgr::dbase::Reset( void )
{
    m_QueryNumber = 0;
    m_BuildingNewDBase = FALSE;
    m_CellSize = 0;
    m_Cells.Clear();
    m_Surfaces.Clear();
}

//=========================================================================

#ifdef X_DEBUG
void playsurface_mgr::dbase::SanityCheck( void )
{
// Took it out because it takes WAY too long to load a level
return;
    // verify the cells are correct
    s32 i, j;
    for( i = 0; i < m_Cells.GetCount(); i++ )
    {
        playsurface_mgr::dbase::cell& Cell = m_Cells[i];

        // After the dbase has been resolved the number of surfaces
        // should be equivalent to the max surfaces--no wasted memory!
        #ifndef X_EDITOR
        ASSERT( Cell.nSurfaces == Cell.MaxSurfaces );
        #endif

        // verify that every surface pointer is valid
        for( j = Cell.iSurfaces; j < (Cell.iSurfaces + Cell.nSurfaces); j++ )
        {
            playsurface_mgr::surface* pSurface = m_Surfaces[j];
            ASSERT( pSurface );

            // verify that this cell is indeed in the surface's bbox
            s32 XMin, XMax, YMin, YMax, ZMin, ZMax;
            GetCellRange( pSurface->WorldBBox, XMin, XMax, YMin, YMax, ZMin, ZMax );
            ASSERT( (Cell.X>=XMin) && (Cell.X <= XMax) );
            ASSERT( (Cell.Y>=YMin) && (Cell.Y <= YMax) );
            ASSERT( (Cell.Z>=ZMin) && (Cell.Z <= ZMax) );
        }
    }

    // now verify that the surfaces are in all the cells that they should be,
    // and that our array is full of valid surfaces
    for( i = 0; i < m_Surfaces.GetCount(); i++ )
    {
        playsurface_mgr::surface* pSurface = m_Surfaces[i];
        ASSERT( pSurface );

        s32 XMin, XMax, YMin, YMax, ZMin, ZMax;
        GetCellRange( pSurface->WorldBBox, XMin, XMax, YMin, YMax, ZMin, ZMax );

        for ( s32 XCell = XMin; XCell <= XMax; XCell++ )
        for ( s32 YCell = YMin; YCell <= YMax; YCell++ )
        for ( s32 ZCell = ZMin; ZCell <= ZMax; ZCell++ )
        {
            // make sure a cell exists in this range
            cell* pCell = GetCell( XCell, YCell, ZCell );
            ASSERT( pCell );

            // now make sure the cell contains this surface
            xbool Found = FALSE;
            for( j = pCell->iSurfaces; j < (pCell->iSurfaces + pCell->nSurfaces); j++ )
            {
                if( m_Surfaces[j] == pSurface )
                {
                    Found = TRUE;
                    break;
                }
            }
            ASSERT( Found );
        }
    }
}
#endif

//=========================================================================

s32 playsurface_mgr::dbase::Load( X_FILE* fh )
{
    ASSERT( fh );
    m_Cells.Clear();
    m_Surfaces.Clear();

    // read in the header
    dbase_file_hdr Hdr;
    x_fread( &Hdr, sizeof(dbase_file_hdr), 1, fh );
    m_CellSize = Hdr.CellSize;

    // allocate space for the cells and surfaces
    m_Cells.SetCapacity( Hdr.nCells );
    m_Cells.SetCount( Hdr.nCells );
    m_Surfaces.SetCapacity( Hdr.nSurfaces );
    m_Surfaces.SetCount( Hdr.nSurfaces );

    // read in the hash table
    x_fread( m_HashTable, sizeof(hash_entry), HASH_SIZE, fh );

    // read in the cells
    x_fread( m_Cells.GetPtr(), sizeof(cell), Hdr.nCells, fh );
    
    // done!
    return (sizeof(dbase_file_hdr) +
            sizeof(hash_entry) * HASH_SIZE +
            sizeof(cell)       * m_Cells.GetCount());
}

//=========================================================================

s32 playsurface_mgr::dbase::Save( X_FILE* fh )
{
    ASSERT( fh );

    // build and save out the header
    dbase_file_hdr Hdr;
    Hdr.CellSize  = m_CellSize;
    Hdr.nCells    = m_Cells.GetCount();
    Hdr.nSurfaces = m_Surfaces.GetCount();
    x_fwrite( &Hdr, sizeof(dbase_file_hdr), 1, fh );

    // save out the hash table
    x_fwrite( m_HashTable, sizeof(hash_entry), HASH_SIZE, fh );

    // save out the cells, only clear out the nSurfaces value first...this
    // way we are already set when loaded
    for ( s32 iCell = 0; iCell < m_Cells.GetCount(); iCell++ )
    {
        cell CellToSave      = m_Cells[iCell];
        CellToSave.nSurfaces = 0;
        x_fwrite( &CellToSave, sizeof(cell), 1, fh );
    }

    // done!
    return (sizeof(dbase_file_hdr) +
            sizeof(hash_entry) * HASH_SIZE +
            sizeof(cell)       * m_Cells.GetCount());
}

//=========================================================================

void playsurface_mgr::dbase::StartNewDBase( s32 CellSize )
{
    m_BuildingNewDBase = TRUE;
    m_CellSize         = CellSize;
    m_Cells.Clear();
    m_Surfaces.Clear();
    for ( s32 i = 0; i < HASH_SIZE; i++ )
    {
        m_HashTable[i].iCells = -1;
        m_HashTable[i].nCells = 0;
    }
}

//=========================================================================

void playsurface_mgr::dbase::AddToNewDBase( surface& Surface )
{
    ASSERT( m_BuildingNewDBase );
    m_Surfaces.Append( &Surface );
}

//=========================================================================

bbox playsurface_mgr::dbase::GetCellBounds( s32 XCell, s32 YCell, s32 ZCell )
{
    return bbox( vector3( (f32)((XCell+0)*m_CellSize), (f32)((YCell+0)*m_CellSize), (f32)((ZCell+0)*m_CellSize) ),
                 vector3( (f32)((XCell+1)*m_CellSize), (f32)((YCell+1)*m_CellSize), (f32)((ZCell+1)*m_CellSize) ) );
}

//=========================================================================

void playsurface_mgr::dbase::GetCellRange( const bbox& BBox, s32& XMin, s32& XMax, s32& YMin, s32& YMax, s32& ZMin, s32& ZMax )
{
    GetCell( BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ(), XMin, YMin, ZMin );
    GetCell( BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ(), XMax, YMax, ZMax );
}

//=========================================================================

playsurface_mgr::dbase::cell* playsurface_mgr::dbase::GetCell( s32 XCell, s32 YCell, s32 ZCell )
{
    s32 HashIndex = DBaseHashFn(XCell, YCell, ZCell);
    
    for ( s32 iCell = m_HashTable[HashIndex].iCells;
          iCell < m_HashTable[HashIndex].iCells+m_HashTable[HashIndex].nCells;
          iCell++ )
    {
        cell& Cell = m_Cells[iCell];
        if ( (Cell.X == XCell) && (Cell.Y == YCell) && (Cell.Z == ZCell) )
            return &Cell;
    }
    
    return NULL;
}

//=========================================================================

void playsurface_mgr::dbase::GetCell( f32 XPos, f32 YPos, f32 ZPos, s32& XCell, s32& YCell, s32& ZCell )
{
    // The playstation and pc like to handle floats differently, and rounding
    // errors will eat you alive, so convert the positions to integers for
    // these operations. (Trust me...or test x_floor((f32)XPos/1600.0f)) The
    // integer method introduces an error of 1 centimeter, which should be
    // close enough.
    s32 XPosInt = (s32)XPos;
    s32 YPosInt = (s32)YPos;
    s32 ZPosInt = (s32)ZPos;

    if ( XPosInt >= 0 ) XCell = XPosInt / m_CellSize;
    else                XCell = XPosInt / m_CellSize - ((XPosInt%m_CellSize)?1:0);
    if ( YPosInt >= 0 ) YCell = YPosInt / m_CellSize;
    else                YCell = YPosInt / m_CellSize - ((YPosInt%m_CellSize)?1:0);
    if ( ZPosInt >= 0 ) ZCell = ZPosInt / m_CellSize;
    else                ZCell = ZPosInt / m_CellSize - ((ZPosInt%m_CellSize)?1:0);
}

//=========================================================================

u32 playsurface_mgr::dbase::DBaseHashFn( s32 XCell, s32 YCell, s32 ZCell )
{
    return (u32)((((XCell<<10)+YCell)<<10)+ZCell)%HASH_SIZE;
}

//=========================================================================

void playsurface_mgr::dbase::AddCell( xarray<s32>& NextLinks, s32 XCell, s32 YCell, s32 ZCell )
{
    // which hash index?
    u32 HashIndex = DBaseHashFn( XCell, YCell, ZCell );

    // does this cell already exist?
    s32 Next = m_HashTable[HashIndex].iCells;
    while ( Next != -1 )
    {
        cell& Cell = m_Cells[Next];
        if ( (Cell.X == XCell) && (Cell.Y == YCell) && (Cell.Z == ZCell) )
        {
            // cell already exists, so nothing to add
            return;
        }
        Next = NextLinks[Next];
    }

    // add the cell
    cell& NewCell       = m_Cells.Append();
    NewCell.X           = XCell;
    NewCell.Y           = YCell;
    NewCell.Z           = ZCell;
    NewCell.iSurfaces   = -1;
    NewCell.nSurfaces   = 0;
    NewCell.MaxSurfaces = 0;

    // link it into the hash table
    NextLinks.Append(m_HashTable[HashIndex].iCells);
    m_HashTable[HashIndex].iCells = m_Cells.GetCount()-1;
}

//=========================================================================

void playsurface_mgr::dbase::ReorderCells( xarray<s32>& NextLinks )
{
    s32 iHash, iCell;

    // create a re-ordered set of cells, and count up how many cells each
    // hash entry has
    xarray<cell> NewCells;
    for ( iHash = 0; iHash < HASH_SIZE; iHash++ )
    {
        m_HashTable[iHash].nCells = 0;
        s32 Next                  = m_HashTable[iHash].iCells;
        while ( Next != -1 )
        {
            cell& NewCell = NewCells.Append();
            NewCell = m_Cells[Next];
            m_HashTable[iHash].nCells++;
            Next = NextLinks[Next];
        }
    }

    // copy the new cells into the static structure
    m_Cells.Clear();
    for ( iCell = 0; iCell < NewCells.GetCount(); iCell++ )
    {
        m_Cells.Append( NewCells[iCell] );
    }
    NewCells.Clear();

    // now fix up the iCells members of the hash table
    s32 CellStart = 0;
    for ( iHash = 0; iHash < HASH_SIZE; iHash++ )
    {
        m_HashTable[iHash].iCells  = CellStart;
        CellStart                 += m_HashTable[iHash].nCells;
    }
    ASSERT( CellStart == m_Cells.GetCount() );

    // At this point, the linked list is useless, and all of the cells
    // are stored linearly in an array. The hash table just contains the
    // start point in the array, and the number of cells for that hash
    // entry.
}

//=========================================================================

void playsurface_mgr::dbase::EndNewDBase( void )
{
    ASSERT( m_BuildingNewDBase );

    // Create the cells...since at build time, the cells won't be ordered,
    // we'll use linked lists instead.
    xarray<s32> NextLinks;
    s32 iSurface;
    for ( iSurface = 0; iSurface < m_Surfaces.GetCount(); iSurface++ )
    {
        s32 XMin, XMax, YMin, YMax, ZMin, ZMax;
        GetCellRange( m_Surfaces[iSurface]->WorldBBox, XMin, XMax, YMin, YMax, ZMin, ZMax );
        for ( s32 XCell = XMin; XCell <= XMax; XCell++ )
        for ( s32 YCell = YMin; YCell <= YMax; YCell++ )
        for ( s32 ZCell = ZMin; ZCell <= ZMax; ZCell++ )
            AddCell( NextLinks, XCell, YCell, ZCell );
    }

    // Reorder the cells so that we can ditch the linked list thing (we should
    // be static, after all!)
    ReorderCells( NextLinks );
    NextLinks.Clear();  // no longer need this

    // now count the number of surfaces for each cell
    s32 NewSurfaceCount = 0;
    for ( iSurface = 0; iSurface < m_Surfaces.GetCount(); iSurface++ )
    {
        s32 XMin, XMax, YMin, YMax, ZMin, ZMax;
        GetCellRange( m_Surfaces[iSurface]->WorldBBox, XMin, XMax, YMin, YMax, ZMin, ZMax );
        for ( s32 XCell = XMin; XCell <= XMax; XCell++ )
        for ( s32 YCell = YMin; YCell <= YMax; YCell++ )
        for ( s32 ZCell = ZMin; ZCell <= ZMax; ZCell++ )
        {
            cell* pCell = GetCell( XCell, YCell, ZCell );
            ASSERT( pCell );
            pCell->MaxSurfaces++;
            NewSurfaceCount++;
        }
    }

    // Create what the new indices will be (these index into the new linear
    // surface array, which will be created shortly). Also, since we're looping
    // through the cells anyway, reset the surface counts back to zero. The idea
    // is that the number will get incremented and decremented as zones are
    // loaded and unloaded, but we always have enough space allocated for them.
    s32 SurfaceStart = 0;
    for ( s32 iCell = 0; iCell < m_Cells.GetCount(); iCell++ )
    {
        m_Cells[iCell].iSurfaces  = SurfaceStart;
        SurfaceStart             += m_Cells[iCell].MaxSurfaces;
    }   
    ASSERT( SurfaceStart == NewSurfaceCount ); 

    // now we know how many surface pointers will really need, make room
    // for them to be added
    m_Surfaces.Clear();
    m_Surfaces.SetCapacity(NewSurfaceCount);
    m_Surfaces.SetCount(NewSurfaceCount);

    // done!
    m_BuildingNewDBase = FALSE;
}

//=========================================================================

void playsurface_mgr::dbase::AddSurface( surface& Surface )
{
    ASSERT( m_QueryNumber == 0 );

    // which cells would this surface be in?
    s32 XMin, XMax, YMin, YMax, ZMin, ZMax;
    GetCellRange( Surface.WorldBBox, XMin, XMax, YMin, YMax, ZMin, ZMax );

    // don't add the surface if it's already there somewhere
    s32 XCell, YCell, ZCell;
    for ( XCell = XMin; XCell <= XMax; XCell++ )
    for ( YCell = YMin; YCell <= YMax; YCell++ )
    for ( ZCell = ZMin; ZCell <= ZMax; ZCell++ )
    {
        cell* pCell = GetCell( XCell, YCell, ZCell );
        ASSERT( pCell );
        for ( s32 iSurface = pCell->iSurfaces;
              iSurface < pCell->iSurfaces + pCell->nSurfaces;
              iSurface++ )
        {
            if ( m_Surfaces[iSurface] == &Surface )
            {
                return;
            }
        }
    }

    // add the surface to each of the cells that it touches
    for ( XCell = XMin; XCell <= XMax; XCell++ )
    for ( YCell = YMin; YCell <= YMax; YCell++ )
    for ( ZCell = ZMin; ZCell <= ZMax; ZCell++ )
    {
        cell* pCell = GetCell( XCell, YCell, ZCell );

        ASSERT( pCell );
        ASSERT( pCell->nSurfaces < pCell->MaxSurfaces );

        m_Surfaces[pCell->iSurfaces+pCell->nSurfaces] = &Surface;
        pCell->nSurfaces++;
    }
}

//=========================================================================

void playsurface_mgr::dbase::RemoveSurface( surface& Surface )
{
    ASSERT( m_QueryNumber == 0 );

    // which cells would this surface be in?
    s32 XMin, XMax, YMin, YMax, ZMin, ZMax;
    GetCellRange( Surface.WorldBBox, XMin, XMax, YMin, YMax, ZMin, ZMax );

    // remove the surface from each of the cells that it touches
    for ( s32 XCell = XMin; XCell <= XMax; XCell++ )
    for ( s32 YCell = YMin; YCell <= YMax; YCell++ )
    for ( s32 ZCell = ZMin; ZCell <= ZMax; ZCell++ )
    {
        xbool found = FALSE;
        cell* pCell = GetCell( XCell, YCell, ZCell );
        ASSERT( pCell );
        s32 iSurface;
        for ( iSurface = pCell->iSurfaces;
              iSurface < pCell->iSurfaces + pCell->nSurfaces;
              iSurface++ )
        {
            if ( m_Surfaces[iSurface] == &Surface )
            {
                found = TRUE;
                break;
            }
        }

        ASSERT( found );
        if ( found )
        {
            // shift the other surface pointers down to keep the linear
            // thing going
            surface** pDst  = m_Surfaces.GetPtr() + iSurface;
            surface** pSrc  = m_Surfaces.GetPtr() + iSurface + 1;
            s32       Count = pCell->nSurfaces -
                              (iSurface-pCell->iSurfaces) - 1;
            if ( Count )
            {
                x_memmove( pDst, pSrc, Count*sizeof(surface*) );
            }
            pCell->nSurfaces--;
        }
    }
}

//=========================================================================

void playsurface_mgr::dbase::CollectSurfaces( const vector3& WorldSpaceRayStart,
                                              const vector3& WorldSpaceRayEnd,
                                              u32            QueryNumber,
                                              u32            Attributes,
                                              u32            NotTheseAttributes )
{
    // Get ray coordinates in spatial-dbase cells
    vector3     RayStart        = WorldSpaceRayStart / (f32)m_CellSize;
    vector3     RayEnd          = WorldSpaceRayEnd / (f32)m_CellSize;
    vector3     RayDir          = RayEnd - RayStart;
    f32         RayLen          = RayDir.Length();
    
    if( RayLen == 0.0f )
        return;
    else
        RayDir /= RayLen;

    // Initialize which cell we are starting in                
    s32 Cell[3]={0,0,0};
    Cell[0] = (((s32)(RayStart[0] + 16384))-16384);
    Cell[1] = (((s32)(RayStart[1] + 16384))-16384);
    Cell[2] = (((s32)(RayStart[2] + 16384))-16384);

    // Clear search info
    m_QueryNumber    = QueryNumber;
    m_CurrentSurface = -1;
    surface* pPrevSurface = NULL;

    // Clear the current distance and begin loop through ray length
    f32 CurrentDist=0;
    while( CurrentDist < RayLen )
    {
        // Process current Cell
        {
            cell* pCell = GetCell( Cell[0], Cell[1], Cell[2] );
            if ( pCell && pCell->nSurfaces )
            {
                // check each of the cell surfaces
                for ( s32 iSurface = pCell->iSurfaces;
                    iSurface < (pCell->iSurfaces+pCell->nSurfaces);
                    iSurface++ )
                {
                    surface* pSurface = m_Surfaces[iSurface];
                    if( pSurface->DBaseQuery == QueryNumber )
                        continue;
                    if( (pSurface->AttrBits & Attributes) == 0 )
                        continue;
                    if( (pSurface->AttrBits & NotTheseAttributes) != 0 )
                        continue;

                    f32 T;
                    if( pSurface->WorldBBox.Intersect(T,WorldSpaceRayStart,WorldSpaceRayEnd) )
                    {
                        pSurface->DBaseQuery  = QueryNumber;
                        pSurface->NextSurface = -1;
                        if ( m_CurrentSurface == -1 )
                        {
                            m_CurrentSurface = iSurface;
                        }
                        else
                        {
                            pPrevSurface->NextSurface = iSurface;
                        }

                        pPrevSurface = pSurface;
                    }
                }
            }
        }

        // Step to next cell.  Note that the computations are calculating the
        // distance to the exit point for the cell from scratch.  There is no
        // delta tracking except the integral cell[3] index.  This is to avoid
        // numerical precision issues due to the accumulation of small deltas.
        {
            // Init shortest distance to a new cell
            f32 NewDist = F32_MAX;
            s32 NewCell[3];

            // Loop through the three dimensions and figure out what the next
            // cell will be.
            for( s32 i=0; i<3; i++ )
            {
                if( RayDir[i] > 0.000001f )
                {
                    f32 Dist = ((Cell[i]+1.0f) - RayStart[i]) / RayDir[i];
                    if( Dist < NewDist )
                    {
                        NewDist = Dist;
                        NewCell[0] = Cell[0];
                        NewCell[1] = Cell[1];
                        NewCell[2] = Cell[2];
                        NewCell[i] = Cell[i] + 1;
                    }
                }
                else
                if( RayDir[i] < -0.000001f )
                {
                    f32 Dist = ((Cell[i]) - RayStart[i]) / RayDir[i];
                    if( Dist < NewDist )
                    {
                        NewDist = Dist;
                        NewCell[0] = Cell[0];
                        NewCell[1] = Cell[1];
                        NewCell[2] = Cell[2];
                        NewCell[i] = Cell[i] - 1;
                    }
                }
            }

            ASSERT( NewDist != F32_MAX );
            CurrentDist = NewDist;
            Cell[0] = NewCell[0];
            Cell[1] = NewCell[1];
            Cell[2] = NewCell[2];
        }
    }

    // disable the query if no surfaces were found
    if ( m_CurrentSurface == -1 )
        m_QueryNumber  = 0;
    else
        m_NextSurface = m_CurrentSurface;
}

//=========================================================================

void playsurface_mgr::dbase::CollectSurfaces( const bbox& BBox,
                                              u32         QueryNumber,
                                              u32         Attributes,
                                              u32         NotTheseAttributes )
{
    m_QueryNumber    = QueryNumber;
    m_CurrentSurface = -1;

    // what is the range of cells to collect?
    s32 XMin, XMax, YMin, YMax, ZMin, ZMax;
    GetCellRange( BBox, XMin, XMax, YMin, YMax, ZMin, ZMax );

    // walk the cells, and build a linked list
    surface* pPrevSurface = NULL;
    for ( s32 XCell = XMin; XCell <= XMax; XCell++ )
    for ( s32 YCell = YMin; YCell <= YMax; YCell++ )
    for ( s32 ZCell = ZMin; ZCell <= ZMax; ZCell++ )
    {
        cell* pCell = GetCell( XCell, YCell, ZCell );
        if ( pCell && pCell->nSurfaces )
        {
            // check each of the cell surfaces
            for ( s32 iSurface = pCell->iSurfaces;
                  iSurface < (pCell->iSurfaces+pCell->nSurfaces);
                  iSurface++ )
            {
                surface* pSurface = m_Surfaces[iSurface];

                if( pSurface->DBaseQuery == QueryNumber )
                    continue;
                if( (pSurface->AttrBits & Attributes) == 0 )
                    continue;
                if( (pSurface->AttrBits & NotTheseAttributes) != 0 )
                    continue;
                
                if ( pSurface->WorldBBox.Intersect(BBox) )
                {
                    pSurface->DBaseQuery  = QueryNumber;
                    pSurface->NextSurface = -1;
                    if ( m_CurrentSurface == -1 )
                    {
                        m_CurrentSurface = iSurface;
                    }
                    else
                    {
                        pPrevSurface->NextSurface = iSurface;
                    }

                    pPrevSurface = pSurface;
                }
            }
        }
    }

    // disable the query if no surfaces were found
    if ( m_CurrentSurface == -1 )
        m_QueryNumber  = 0;
    else
        m_NextSurface = m_CurrentSurface;
}

//=========================================================================

playsurface_mgr::surface* playsurface_mgr::dbase::GetNextSurface( void )
{
    // empty search or we've reached the end?
    if ( m_QueryNumber == 0 )
    {
        m_CurrentSurface = -1;
        return NULL;
    }

    // get the value to return
    ASSERT( m_CurrentSurface >= 0 );
    m_CurrentSurface  = m_NextSurface;
    surface* pSurface = m_Surfaces[m_CurrentSurface];

    // move to the next cell/surface
    m_NextSurface = pSurface->NextSurface;
    if ( m_NextSurface == -1 )
        m_QueryNumber = 0;  // we've reached the end

    return pSurface;
}

//=========================================================================

playsurface_mgr::surface* playsurface_mgr::dbase::GetCurrentSurface( void )
{
    if ( m_CurrentSurface == -1 )
        return NULL;
    else
        return m_Surfaces[m_CurrentSurface];
}

//=========================================================================

guid playsurface_mgr::dbase::GetCurrentGuid( void ) const
{
    if ( m_CurrentSurface == -1 )
        return guid(0);
    else
        return guid(((u64)m_CurrentSurface<<32) | 0xffffffff);
}

//=========================================================================

playsurface_mgr::surface* playsurface_mgr::dbase::GetSurfaceByGuid( guid Guid )
{
    ASSERT( Guid.GetLow() == 0xffffffff );
    return m_Surfaces[(s32)Guid.GetHigh()];
}

//=========================================================================
