//==============================================================================
//
//  SpatialDBase.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_stdio.hpp"
#include "Entropy.hpp"
#include "x_color.hpp"

#include "spatialdbase.hpp"
//TOMAS #include "occludermgr\occludermgr.hpp"

//==============================================================================
//==============================================================================
//==============================================================================
// SPATIAL_DBASE
//==============================================================================
//==============================================================================
//==============================================================================

//==============================================================================

spatial_dbase::spatial_dbase( void )
{
    m_pCell = NULL;
    Clear();
}

//==============================================================================


spatial_dbase::~spatial_dbase( void )
{
    Clear();
}

//==============================================================================


void spatial_dbase::Clear( void )
{
    if( m_pCell ) 
        x_free( m_pCell );

    m_pCell             =  NULL;
    m_nCellsAllocated   =  0;
    m_nCells            =  0;
    m_FirstFree         =  SPATIAL_CELL_NULL;
    m_SearchSeq         = -1;

    // Clear number of cells in each level
    for( s32 i=0; i<MAX_LEVELS; i++ )
        m_nCellsInLevel[i] = 0;
}

//==============================================================================


void spatial_dbase::Kill( void )
{
    Clear();
}

//==============================================================================

void spatial_dbase::Init( f32 MinCellWidth )
{
    s32 i;

    // Reset the class
    Clear();

    // Set the smallest cell
    m_CellWidth[0]  = MinCellWidth;

    // Pre-Compute cell sizes
    for( i=1; i<MAX_LEVELS; i++ )
        m_CellWidth[i] = 0;

    for( i=1; i<MAX_LEVELS; i++ )
        m_CellWidth[i] = m_CellWidth[i-1]*2.0f;

    // Clear hash table
    for( i=0; i<HASH_SIZE; i++ )
    {
        m_Hash[i].FirstCell = SPATIAL_CELL_NULL;
        m_Hash[i].nCells    = 0;
    }
}

//==============================================================================

void spatial_dbase::SanityCheck( void ) const
{
}

//==============================================================================


void spatial_dbase::DumpStats( void ) const
{
    s32 i;
    static s32 FCount=0;

    X_FILE* fp = x_fopen(xfs("hash%03d.txt",FCount++),"wt");
    ASSERT( fp );

    // Show basic info
    for( i=0; i<MAX_LEVELS; i++ )
        x_fprintf(fp,"%1d] Cell Size %f\n",i,m_CellWidth[i]);
    x_fprintf(fp,"\n");

    // Show number of cells
    x_fprintf(fp,"TotalCells %1d\n",m_nCells);
    for( i=0; i<MAX_LEVELS; i++ )
        x_fprintf(fp,"%1d] nCells %1d\n",i,m_nCellsInLevel[i]);
    x_fprintf(fp,"\n");

    // Hash situation
    s32 MaxHits=0;
    for( i=0; i<HASH_SIZE; i++ )
    {
        x_fprintf(fp,"%4d] %1d\n",i,m_Hash[i].nCells);
        MaxHits = iMax( m_Hash[i].nCells, MaxHits );
    }

    x_fprintf( fp, "MaxHits %1d\n", MaxHits );
    x_fclose( fp );
}

//==============================================================================


void spatial_dbase::Render( s32 MinLevel, s32 MaxLevel ) const
{
    s32                   nCells  = 0;
    static const s16      Index[] = {1,5,5,7,7,3,3,1,0,4,4,6,6,2,2,0,3,2,7,6,5,4,1,0};
    static const xcolor   Color[] = { XCOLOR_RED, XCOLOR_GREEN, XCOLOR_BLUE, XCOLOR_YELLOW, XCOLOR_AQUA, XCOLOR_PURPLE };
    vector3               P[8];

    draw_ClearL2W();
    draw_Begin( DRAW_LINES );
    draw_Color( Color[0] );

    // Render all cells of a particular level
    for( s32 H = 0; H<HASH_SIZE; H++ )
    {
        u16 ID = m_Hash[H].FirstCell;

        while( ID != SPATIAL_CELL_NULL )
        {
            spatial_cell* pCell = GetCell( ID );
            ASSERT( pCell );

            if( (pCell->Level >= MinLevel) && (pCell->Level <= MaxLevel) )
            {
                // Decide color
                if( pCell->Level > 5 )
                    draw_Color( XCOLOR_WHITE );
                else
                    draw_Color( Color[pCell->Level] );

                // Get bbox bounds
                bbox BBox;
                BBox = GetCellBBox( pCell->X, pCell->Y, pCell->Z, pCell->Level );

                P[0].X = BBox.Min.X;    P[0].Y = BBox.Min.Y;    P[0].Z = BBox.Min.Z;
                P[1].X = BBox.Min.X;    P[1].Y = BBox.Min.Y;    P[1].Z = BBox.Max.Z;
                P[2].X = BBox.Min.X;    P[2].Y = BBox.Max.Y;    P[2].Z = BBox.Min.Z;
                P[3].X = BBox.Min.X;    P[3].Y = BBox.Max.Y;    P[3].Z = BBox.Max.Z;
                P[4].X = BBox.Max.X;    P[4].Y = BBox.Min.Y;    P[4].Z = BBox.Min.Z;
                P[5].X = BBox.Max.X;    P[5].Y = BBox.Min.Y;    P[5].Z = BBox.Max.Z;
                P[6].X = BBox.Max.X;    P[6].Y = BBox.Max.Y;    P[6].Z = BBox.Min.Z;
                P[7].X = BBox.Max.X;    P[7].Y = BBox.Max.Y;    P[7].Z = BBox.Max.Z;

                // Render linees
                s32 i;

                for( i=0; i<((s32)(sizeof(Index)/sizeof(s16))); i++ )
                    draw_Vertex( P[Index[i]] );

                //draw_Verts( P, 8 );
                //draw_Execute( Index, sizeof(Index)/sizeof(s16) );
                nCells++;
            }

            ID = pCell->HashNext;
        } 
    }

    draw_End();
/*
    x_printf("NCELLS(%4d): ",m_nCells);
    for( s32 i=0; i<MAX_LEVELS; i++ )
        x_printf("%3d ",m_nCellsInLevel[i]);
    x_printf("\n");
*/
}

//==============================================================================


u16 spatial_dbase::AllocCell( s32 X, s32 Y, s32 Z, s32 Level )
{
    spatial_cell* pCell = NULL;

    // Check if we need to grow cell list
    if( m_nCells == m_nCellsAllocated )
    {
        m_nCellsAllocated += 2048;

        x_DebugMsg("WARNING: AllocCell1: %08X ",(u32)m_pCell);
        m_pCell = (spatial_cell*)x_realloc( m_pCell, sizeof(spatial_cell)*m_nCellsAllocated );
        if( m_pCell == NULL )
        {
            BREAK;
        }
        ASSERT( m_pCell );
        x_DebugMsg("AllocCell2: %08X\n",(u32)m_pCell);

        // Add new Cells to free list
        for( s32 i=m_nCellsAllocated-1; i>=m_nCells; i-- )
        {
            spatial_cell* pCell = GetCell( i );
            ASSERT( pCell );
            
            // Lunch the construtors
            xConstruct( pCell );

            pCell->HashNext = m_FirstFree;
            pCell->HashPrev = SPATIAL_CELL_NULL;

            if( m_FirstFree != SPATIAL_CELL_NULL )
            {
                spatial_cell* pFirst = GetCell( m_FirstFree );
                ASSERT( pFirst );
                pFirst->HashPrev = i;
            }

            m_FirstFree = i;
        }
    }

    // Increase number of Cells
    m_nCells++;
    m_nCellsInLevel[ Level ]++;

    //
    // Pull Cell from free list
    //
    ASSERT( m_FirstFree != SPATIAL_CELL_NULL );
    u32 ID = m_FirstFree;
    pCell  = GetCell( ID );
    ASSERT( pCell );

    m_FirstFree = pCell->HashNext;
    if( m_FirstFree != SPATIAL_CELL_NULL )
    {
        spatial_cell* pFirst = GetCell( m_FirstFree );
        ASSERT( pFirst );
        pFirst->HashPrev = SPATIAL_CELL_NULL;
    }

    // Fill out cell
    {
        pCell->X            = X;
        pCell->Y            = Y;
        pCell->Z            = Z;
        pCell->Level        = Level;
        pCell->OccFlags     = 0;
    }

    //
    // Compute hash entry
    //
    s32 H = ComputeHash( X, Y, Z, Level );
    
    // Add to hash list
    if( m_Hash[H].FirstCell != SPATIAL_CELL_NULL )
    {
        spatial_cell* pFirst = GetCell( m_Hash[H].FirstCell );
        pFirst->HashPrev = ID;
    }

    pCell->HashNext = m_Hash[H].FirstCell;
    pCell->HashPrev = SPATIAL_CELL_NULL;
    m_Hash[H].FirstCell = ID;
    m_Hash[H].nCells++;

    // Hook up to parents and siblings
    if( Level < MAX_LEVELS-1 )
    {
        // Do recursive call to create parent
        s32 P = GetCellIndex( X>>1, Y>>1, Z>>1, Level+1, TRUE );
        pCell = GetCell( ID );
        ASSERT( pCell );
       
        // Get ptrs to parent and child
        spatial_cell* pParent = GetCell( P );
        spatial_cell* pChild  = pCell;
        ASSERT( pParent );

        // Remember who the parent is
        pChild->Parent = P;
        pChild->Child  = SPATIAL_CELL_NULL;

        // Attach child to parent
        if( pParent->Child != SPATIAL_CELL_NULL )
        {
            spatial_cell* pChild = GetCell(pParent->Child);
            pChild->Prev = ID;
            ASSERT( pChild );
        }
        pChild->Next    = pParent->Child;
        pChild->Prev    = SPATIAL_CELL_NULL;
        pParent->Child  = ID;
    }
    else
    {
        pCell->Parent       = SPATIAL_CELL_NULL;
        pCell->Child        = SPATIAL_CELL_NULL;
        pCell->Next         = SPATIAL_CELL_NULL;
        pCell->Prev         = SPATIAL_CELL_NULL;
    }

    return ID;
}

//==============================================================================


void spatial_dbase::FreeCell( u16 ID )
{
    //x_DebugMsg("Calling FreeCell on %3d\n", I);
    spatial_cell* pCell = GetCell( ID );
    ASSERT( pCell );

    // TODO: Confirm there are no children
    ASSERT( pCell->Child    == SPATIAL_CELL_NULL );
    ASSERT( pCell->OccFlags == 0 );

    // Remove from parent
    if( pCell->Level < MAX_LEVELS-1 )
    {
        spatial_cell* pChild  = pCell;
        spatial_cell* pParent = GetCell( pChild->Parent );
        ASSERT( pParent );

        if( pParent->Child == ID )
            pParent->Child = pChild->Next;

        if( pChild->Next != SPATIAL_CELL_NULL )
        {
            spatial_cell* pPrev = GetCell( pChild->Next );
            ASSERT( pPrev );
            pPrev->Prev = pChild->Prev;        
        }

        if( pChild->Prev != SPATIAL_CELL_NULL )
        {   
            spatial_cell* pPrev = GetCell( pChild->Prev );
            ASSERT( pPrev );
            pPrev->Next = pChild->Next;
        }

        // Check if we should recursively delete parent
        if( (pParent->Child == SPATIAL_CELL_NULL) && (pParent->OccFlags==0) )
            FreeCell( pChild->Parent );

        pChild->Parent = SPATIAL_CELL_NULL;
    }

    // Decrease number of Cells
    m_nCells--;
    m_nCellsInLevel[ pCell->Level ]--;

    //
    // Remove node from the hash table
    //

    // Compute hash
    s32 H = ComputeHash( pCell->X, pCell->Y, pCell->Z, pCell->Level );
    /*if(H == 0)
    {
        x_DebugMsg("H is 0\n");
    }*/
    //x_DebugMsg("     Hash is '%d', nCells = '%d'\n", H, m_Hash[H].nCells);

    // Remove from hash
    if( pCell->HashNext != SPATIAL_CELL_NULL )
    {
        spatial_cell* pHashNext = GetCell( pCell->HashNext );
        ASSERT( pHashNext );
        pHashNext->HashPrev = pCell->HashPrev;
    }

    if( pCell->HashPrev != SPATIAL_CELL_NULL )
    {
        spatial_cell* pHashPrev = GetCell( pCell->HashPrev );
        ASSERT( pHashPrev );
        pHashPrev->HashNext = pCell->HashNext;
    }

    if( m_Hash[H].FirstCell == ID )
        m_Hash[H].FirstCell = pCell->HashNext;

    m_Hash[H].nCells--;
    ASSERT( m_Hash[H].nCells >= 0 );

    //
    // Add node to free list
    //
    pCell->HashNext = m_FirstFree;
    pCell->HashPrev = SPATIAL_CELL_NULL;

    if( m_FirstFree != SPATIAL_CELL_NULL )
    {
        spatial_cell* pFrist = GetCell( m_FirstFree );
        ASSERT( pFrist );
        pFrist->HashPrev = ID;
    }

    m_FirstFree = ID;
}

//==============================================================================
//TOMAS extern xbool g_DoSpatialDBaseOcclusion;

void spatial_dbase::BuildVisList( const view* pView, xbool DoCulling )
{
    xtimer Time;
    Time.Start();

    s32 nChecked    = 0;
    m_FirstSearch   = SPATIAL_CELL_NULL;
    m_nSearchNodes  = 0;

    // Loop through all cells and check if inside view
    for( s32 H = 0; H<HASH_SIZE; H++ )
    {
        u16 ID = m_Hash[H].FirstCell;
        while( ID != SPATIAL_CELL_NULL )
        {
            spatial_cell* pCell = GetCell( ID );
            ASSERT( pCell );

            if( pCell->OccFlags )
            {
                nChecked++;

                bbox BBox = GetCellBBox( pCell->X, pCell->Y, pCell->Z, pCell->Level );
                
                if( pView->BBoxInView( BBox ) )
                {
                    // Add to vis list
                    pCell->SearchNext = m_FirstSearch;
                    m_FirstSearch = ID;
                    m_nSearchNodes++;
                }
            }

            ID = pCell->HashNext;
        } 
    }

    // Scan through huge cells
    u16     Cursor[MAX_LEVELS];
    s32     InViewLevel;

    for( s32 CX=-4; CX<=4; CX++ )
    for( s32 CY=-4; CY<=4; CY++ )
    for( s32 CZ=-4; CZ<=4; CZ++ )
    {
        // Check at highest level then start traversing children
        InViewLevel = -1;
        Cursor[MAX_LEVELS-1] = GetCellIndex( CX, CY, CZ, MAX_LEVELS-1);
        if( Cursor[MAX_LEVELS-1] != SPATIAL_CELL_NULL )
        {
            s32 Level = MAX_LEVELS-1;

            while( Level<MAX_LEVELS )
            {
                if( Cursor[Level] == SPATIAL_CELL_NULL )
                {
                    Level++;
                    if( Level == InViewLevel )
                        InViewLevel = -1;
                    continue;
                }

                // Get cell
                spatial_cell* pCell = GetCell( Cursor[Level] );
                ASSERT(pCell);

                nChecked++;
                s32 InView = 1;
                if( Level >= InViewLevel )
                {
                    // Check if current cell is visible
                    bbox BBox = GetCellBBox( *pCell );
                    InView = pView->BBoxInView( BBox );
                    if( !DoCulling )
                        InView = 1;

                    // Apply occlusion
/* TOMAS
                    if( g_DoSpatialDBaseOcclusion )
                    {
                        if( InView )
                        {
                            xbool UseFusion = (Level>2);
//TOMAS                            if( g_OccluderMgr.IsBBoxOccluded( BBox, UseFusion ) )
//                                InView = 0;
                        }
                    }
*/

                    if( InView==1 )
                        InViewLevel = Level;
                }

                if( InView )
                {
                    // Is there anything in this node?
                    if( pCell->OccFlags )
                    {
                        pCell->SearchNext = m_FirstSearch;
                        m_FirstSearch     = Cursor[Level];
                        m_nSearchNodes++;
                        Cursor[Level] = pCell->Next;
                    }

                    // Push children
                    if( Level > 0 )
                    {
                        Cursor[Level] = pCell->Next;
                        Level--;
                        Cursor[Level] = pCell->Child;
                    }
                }
                else
                {
                    // Move to next in list
                    Cursor[Level] = pCell->Next;
                }
            }
        }
    }


    Time.Stop();

    //x_printf("nCells(%4d) nChecked(%4d) nVis(%4d)  %1.4f\n",m_nCells,nChecked,m_nVis,Time.ReadMs());
}


//==============================================================================

u16 spatial_dbase::TraverseCells( 
    const bbox&     RegionBBox, 
    u32             OccFlags )
{
    s32 i,X,Y,Z;
    s32 MinX,MinY,MinZ;
    s32 MaxX,MaxY,MaxZ;
    s32 RegionLevel;
    s32 nChecked=0;
    u16 Cursor[MAX_LEVELS];

    xtimer Time;
    Time.Start();

    m_SearchSeq++;

    // Get information about what level and span this region covers
    RegionLevel = GetBBoxLevel( RegionBBox );
    GetCellRegion( RegionBBox, RegionLevel, MinX, MinY, MinZ, MaxX, MaxY, MaxZ );

    // Loop through cells at original level
    m_FirstSearch  = SPATIAL_CELL_NULL;
    m_nSearchNodes = 0;
    i=0;
    for( X=MinX; X<=MaxX; X++ )
    for( Y=MinY; Y<=MaxY; Y++ )
    for( Z=MinZ; Z<=MaxZ; Z++ )
    {
        // For each cell, find lowest level that a cell exists
        s32 CX=X;
        s32 CY=Y;
        s32 CZ=Z;
        s32 CellID = -1;
        s32 CLevel = RegionLevel;
        while( CLevel < MAX_LEVELS )
        {
            CellID = GetCellIndex( CX, CY, CZ, CLevel );
            if( CellID != SPATIAL_CELL_NULL )
                break;

            CX >>= 1;
            CY >>= 1;
            CZ >>= 1;
            CLevel++;
        }

        ASSERT(CellID != -1);

        // Get access to cell
        spatial_cell* pCell = (spatial_cell*)GetCell( CellID );

        if( pCell && (pCell->SearchSeq != m_SearchSeq) )
        {
            pCell->SearchSeq = m_SearchSeq;

            // Run up parents
            s32 C=0;
            while( pCell->Parent != SPATIAL_CELL_NULL )
            {
                ASSERT( (C++) < 1000 );
                u16  ID = pCell->Parent;

                pCell = (spatial_cell*)GetCell( ID );
                ASSERT( pCell );

                if( pCell->SearchSeq == m_SearchSeq ) 
                    break;

                pCell->SearchSeq = m_SearchSeq;

                nChecked++;

                if( pCell->OccFlags & OccFlags )
                {
                    m_nSearchNodes++;
                        
                    if( m_FirstSearch == SPATIAL_CELL_NULL )
                    {
                        pCell->SearchNext = SPATIAL_CELL_NULL;
                        m_FirstSearch     = ID;
                    }
                    else
                    {
                        pCell->SearchNext = m_FirstSearch;
                        m_FirstSearch     = ID;
                    }
                }
            }

            // Process children
            {
                Cursor[CLevel] = GetCellIndex( CX, CY, CZ, CLevel);
                s32 Level = CLevel;

                s32 C=0;
                while( Level<=CLevel )
                {
                    ASSERT( (C++) < 1000 ); 

                    // Pop up
                    if( Cursor[Level] == SPATIAL_CELL_NULL )
                    {
                        Level++;
                        continue;
                    }

                    // Get cell
                    nChecked++;
                    u16           ID    = Cursor[Level];
                    spatial_cell* pCell = GetCell( ID );
                    ASSERT(pCell);

                    // Check if current cell is visible
                    xbool InRegion = GetCellBBox( *pCell ).Intersect( RegionBBox );

                    if( InRegion )
                    {
                        pCell->SearchSeq = m_SearchSeq;

                        // Call user function
                        if( pCell->OccFlags & OccFlags )
                        {
                            m_nSearchNodes++;
                            if( m_FirstSearch == SPATIAL_CELL_NULL )
                            {
                                pCell->SearchNext = SPATIAL_CELL_NULL;
                                m_FirstSearch     = ID;
                            }
                            else
                            {
                                pCell->SearchNext = m_FirstSearch;
                                m_FirstSearch     = ID;
                            }
                        }

                        // Move this level to sibling
                        Cursor[Level] = pCell->Next;

                        // Push children
                        if( Level > 0 )
                        {
                            Level--;
                            Cursor[Level] = pCell->Child;
                        }
                    }
                    else
                    {
                        // Move to next in list
                        Cursor[Level] = pCell->Next;
                    }
                }
            }
        }
    }

#if 0
    // recheck that all cells that should have been touched, were touched
    for ( s32 H = 0; H < HASH_SIZE; ++H )
    {
        s32 I = m_Hash[H].FirstCell;
        while( I != SPATIAL_CELL_NULL )
        {
            spatial_cell* pCell = CELL(I);
            ASSERT( pCell );
            bbox BBox = GetCellBBox( pCell );

            if ( BBox.Intersect( RegionBBox ) )
            {
                ASSERT( pCell->SearchSeq == m_SearchSeq );
            }
            
            I = pCell->HashNext;
        }
    }
#endif

    Time.Stop();

    return m_FirstSearch;
}

//==============================================================================

