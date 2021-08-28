//==============================================================================
//
//  SpatialDBase.cpp
//
//==============================================================================

//==============================================================================
//  INCLUDES
//==============================================================================

#include "spatialdbase.hpp"

#include "x_stdio.hpp"
#include "Entropy.hpp"
#include "x_color.hpp"

//TOMAS #include "occludermgr\occludermgr.hpp"
//==============================================================================
// GLOBAL VARIABLES
//==============================================================================

spatial_dbase g_SpatialDBase;

//==============================================================================
// FUNCTIONS
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

void spatial_dbase::SanityCheck( void )
{
    s32 i;

    //
    // Check the hash
    //
    for( i=0; i<HASH_SIZE; i++ )
    {
        s32 Count = 0;
        for( u16 hCell = m_Hash[i].FirstCell; hCell != SPATIAL_CELL_NULL; hCell = m_pCell[hCell].HashNext )
        {
            spatial_cell& Cell = m_pCell[hCell];

            // Check that all the cell in the hash have a parent
            if( Cell.Level < (MAX_LEVELS-1) ) 
            {
                ASSERT( Cell.Parent != SPATIAL_CELL_NULL );
                ASSERT( m_pCell[Cell.Parent].Level == (Cell.Level+1) );
            }

            // Check the next node if it is okay
            if( Cell.HashNext != SPATIAL_CELL_NULL )
            {
                ASSERT( m_pCell[Cell.HashNext].HashPrev == hCell );
            }

            // Check the prev node if it is okay
            if( Cell.HashPrev != SPATIAL_CELL_NULL )
            {
                ASSERT( m_pCell[Cell.HashPrev].HashNext == hCell );
            }

            // Make sure that we are dealing with the right cell
            ASSERT( GetCell( Cell.X, Cell.Y, Cell.Z, Cell.Level, FALSE ) == &Cell );

            // Check that the count is correct
            Count++;
            ASSERT( Count <= m_Hash[i].nCells );
        }
        ASSERT( m_Hash[i].nCells == Count );
    }
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
            const spatial_cell& Cell = GetCell( ID );

            if( (Cell.Level >= MinLevel) && (Cell.Level <= MaxLevel) )
            {
                // Decide color
                if( Cell.Level > 5 )
                    draw_Color( XCOLOR_WHITE );
                else
                    draw_Color( Color[Cell.Level] );

                // Get bbox bounds
                bbox BBox;
                BBox = GetCellBBox( Cell.X, Cell.Y, Cell.Z, Cell.Level );

                P[0].GetX() = BBox.Min.GetX();  P[0].GetY() = BBox.Min.GetY();  P[0].GetZ() = BBox.Min.GetZ();
                P[1].GetX() = BBox.Min.GetX();  P[1].GetY() = BBox.Min.GetY();  P[1].GetZ() = BBox.Max.GetZ();
                P[2].GetX() = BBox.Min.GetX();  P[2].GetY() = BBox.Max.GetY();  P[2].GetZ() = BBox.Min.GetZ();
                P[3].GetX() = BBox.Min.GetX();  P[3].GetY() = BBox.Max.GetY();  P[3].GetZ() = BBox.Max.GetZ();
                P[4].GetX() = BBox.Max.GetX();  P[4].GetY() = BBox.Min.GetY();  P[4].GetZ() = BBox.Min.GetZ();
                P[5].GetX() = BBox.Max.GetX();  P[5].GetY() = BBox.Min.GetY();  P[5].GetZ() = BBox.Max.GetZ();
                P[6].GetX() = BBox.Max.GetX();  P[6].GetY() = BBox.Max.GetY();  P[6].GetZ() = BBox.Min.GetZ();
                P[7].GetX() = BBox.Max.GetX();  P[7].GetY() = BBox.Max.GetY();  P[7].GetZ() = BBox.Max.GetZ();

                //
                // Render linees
                //
                {
                    s32 i;

                    for( i=0; i<((s32)(sizeof(Index)/sizeof(s16))); i++ )
                        draw_Vertex( P[Index[i]] );

                    //draw_Verts( P, 8 );
                    //draw_Execute( Index, sizeof(Index)/sizeof(s16) );
                }

                nCells++;
            }

            ID = Cell.HashNext;
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
    MEMORY_OWNER( "SPATIAL DBASE - spatial_dbase::AllocCell()" );

    // Check if we need to grow cell list
    if( m_nCells == m_nCellsAllocated )
    {
        // WARNING:
        // Todo this is very unsave. Since some of the functions have references to the 
        // memory previously allocated. So when it reallocs and changes the memory it crashes
        // the computer.
        //
        if( m_nCellsAllocated > 0 )
        {
            ASSERT( 0 );
            x_throw( "Must Make the spacial data base bigger" );
            #if defined( CONFIG_VIEWER )
            extern void x_DebugSetCause( const char* pCause );
            x_DebugSetCause( "Spatial database needs more Cells!" );
            *(u32*)1 = 0;
            #endif
        }

        // WARNING:
        // Make sure to grow here all that we will ever need.
        //
        #ifndef TARGET_PC
        m_nCellsAllocated += 2048 + 512; // TODO: CJ: This was bumped up by 512 for Alien SL4_1
        #else
        m_nCellsAllocated += (2048*8);
        #endif

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
            spatial_cell& Cell = GetCell( i );
            
            // Lunch the construtors
            xConstruct( &Cell );

            // Add cells into the empty list
            Cell.HashNext = m_FirstFree;
            Cell.HashPrev = SPATIAL_CELL_NULL;

            if( m_FirstFree != SPATIAL_CELL_NULL )
            {
                spatial_cell& First = GetCell( m_FirstFree );
                First.HashPrev = i;
            }

            m_FirstFree = i;
        }
    }

    //
    // Increase number of Cells
    //
    m_nCells++;
    m_nCellsInLevel[ Level ]++;

    //
    // Pull Cell from free list
    //
    ASSERT( m_FirstFree != SPATIAL_CELL_NULL );
    u32             ID   = m_FirstFree;
    spatial_cell&   Cell = GetCell( ID );

    m_FirstFree = Cell.HashNext;
    if( m_FirstFree != SPATIAL_CELL_NULL )
    {
        spatial_cell& First = GetCell( m_FirstFree );
        First.HashPrev      = SPATIAL_CELL_NULL;
    }

    //
    // Fill out cell
    //
    {
        Cell.X            = X;
        Cell.Y            = Y;
        Cell.Z            = Z;
        Cell.Level        = Level;
        Cell.OccFlags     = 0;        // TODO: Add flags?
    }

    //
    // Compute hash entry
    //
    s32 H = ComputeHash( X, Y, Z, Level );
    
    // Add to hash list
    if( m_Hash[H].FirstCell != SPATIAL_CELL_NULL )
    {
        spatial_cell& First = GetCell( m_Hash[H].FirstCell );
        First.HashPrev = ID;
    }

    Cell.HashNext = m_Hash[H].FirstCell;
    Cell.HashPrev = SPATIAL_CELL_NULL;
    m_Hash[H].FirstCell = ID;
    m_Hash[H].nCells++;

    // Hook up to parents and siblings
    if( Level < MAX_LEVELS-1 )
    {
        // Do recursive call to create parent
        s32 ParentID = GetCellIndex( X>>1, Y>>1, Z>>1, Level+1, TRUE );
       
        // Get ptrs to parent and child
        spatial_cell& Parent = GetCell( ParentID );

        // Remember who the parent is
        Cell.Parent = ParentID;
        Cell.Child  = SPATIAL_CELL_NULL;

        // Attach child to parent
        if( Parent.Child != SPATIAL_CELL_NULL )
        {
            spatial_cell& FirstChild = GetCell( Parent.Child );
            FirstChild.Prev = ID;
        }

        Cell.Next     = Parent.Child;
        Cell.Prev     = SPATIAL_CELL_NULL;
        Parent.Child  = ID;
    }
    else
    {
        Cell.Parent       = SPATIAL_CELL_NULL;
        Cell.Child        = SPATIAL_CELL_NULL;
        Cell.Next         = SPATIAL_CELL_NULL;
        Cell.Prev         = SPATIAL_CELL_NULL;
    }

    return ID;
}

//==============================================================================


void spatial_dbase::FreeCell( u16 ID )
{
    //x_DebugMsg("Calling FreeCell on %3d\n", I);
    spatial_cell& Cell = GetCell( ID );

    // TODO: Confirm there are no children
    ASSERT( Cell.Child    == SPATIAL_CELL_NULL );
    ASSERT( Cell.OccFlags == 0 );

    //
    // Remove from parent
    //
    if( Cell.Level < MAX_LEVELS-1 )
    {
        spatial_cell& Parent = GetCell( Cell.Parent );

        if( Parent.Child == ID )
            Parent.Child = Cell.Next;

        if( Cell.Next != SPATIAL_CELL_NULL )
        {
            spatial_cell& Prev = GetCell( Cell.Next );
            Prev.Prev          = Cell.Prev;        
        }

        if( Cell.Prev != SPATIAL_CELL_NULL )
        {   
            spatial_cell& Prev = GetCell( Cell.Prev );
            Prev.Next          = Cell.Next;
        }

        // Check if we should recursively delete parent
        if( (Parent.Child == SPATIAL_CELL_NULL) && (Parent.OccFlags==0) )
            FreeCell( Cell.Parent );

        Cell.Parent = SPATIAL_CELL_NULL;
    }

    //
    // Decrease number of Cells
    //
    m_nCells--;
    m_nCellsInLevel[ Cell.Level ]--;

    //
    // Remove node from the hash table
    //

    // Compute hash
    s32 H = ComputeHash( Cell.X, Cell.Y, Cell.Z, Cell.Level );
    /*if(H == 0)
    {
        x_DebugMsg("H is 0\n");
    }*/
    //x_DebugMsg("     Hash is '%d', nCells = '%d'\n", H, m_Hash[H].nCells);

    // Remove from hash
    if( Cell.HashNext != SPATIAL_CELL_NULL )
    {
        spatial_cell& HashNext = GetCell( Cell.HashNext );
        HashNext.HashPrev      = Cell.HashPrev;
    }

    if( Cell.HashPrev != SPATIAL_CELL_NULL )
    {
        spatial_cell& HashPrev = GetCell( Cell.HashPrev );
        HashPrev.HashNext      = Cell.HashNext;
    }

    if( m_Hash[H].FirstCell == ID )
        m_Hash[H].FirstCell = Cell.HashNext;

    m_Hash[H].nCells--;
    ASSERT( m_Hash[H].nCells >= 0 );

    //
    // Add node to free list
    //
    Cell.HashNext = m_FirstFree;
    Cell.HashPrev = SPATIAL_CELL_NULL;

    if( m_FirstFree != SPATIAL_CELL_NULL )
    {
        spatial_cell& Frist = GetCell( m_FirstFree );
        Frist.HashPrev = ID;
    }

    m_FirstFree = ID;
}

//==============================================================================
//TOMAS extern xbool g_DoSpatialDBaseOcclusion;

u16 spatial_dbase::BuildVisList( const view& View, xbool DoCulling )
{
    CONTEXT( "spatial_dbase::BuildVisList" );

    //
    // Initialize
    //
    s32 nChecked    = 0;
    m_FirstSearch   = SPATIAL_CELL_NULL;
    m_nSearchNodes  = 0;

    //
    // For debugging
    //
    if( 0 )
    {
        // Loop through all cells and check if inside view
        for( s32 H = 0; H<HASH_SIZE; H++ )
        {
            u16 ID = m_Hash[H].FirstCell;
            while( ID != SPATIAL_CELL_NULL )
            {
                spatial_cell& Cell = GetCell( ID );

                if( Cell.OccFlags )
                {
                    nChecked++;

                    if( View.BBoxInView(GetCellBBox( Cell )) )
                    {
                        // Add to vis list
                        Cell.SearchNext = m_FirstSearch;
                        m_FirstSearch   = ID;
                        m_nSearchNodes++;
                    }
                }

                ID = Cell.HashNext;
            } 
        }

        return m_FirstSearch;
    }

    //
    // Full implementation
    //

    // Scan through huge cells
    u16     Cursor[MAX_LEVELS];
    s32     InViewLevel;

    for( s32 CX=-4; CX<=4; CX++ )
    for( s32 CY=-4; CY<=4; CY++ )
    for( s32 CZ=-4; CZ<=4; CZ++ )
    {
        // Check at highest level then start traversing children
        InViewLevel                 = -1;
        Cursor[MAX_LEVELS-1]        = GetCellIndex( CX, CY, CZ, MAX_LEVELS-1);

        if( Cursor[MAX_LEVELS-1] != SPATIAL_CELL_NULL )
        {
            s32 Level = MAX_LEVELS-1;

            while( Level<MAX_LEVELS )
            {
                ASSERT(Level < MAX_LEVELS) ;
                if( Cursor[Level] == SPATIAL_CELL_NULL )
                {
                    Level++;
                    if( Level == InViewLevel )
                        InViewLevel = -1;
                    continue;
                }

                // Get cell
                spatial_cell&   Cell    = GetCell( Cursor[Level] );
                s32             InView  = 1;
                u32             CheckPlaneMask = 0;

                // Clear teh Partially visible flag
                Cell.Flags &= ~spatial_cell::FLAG_SKIN_PLANES;

                nChecked++;
                if( Level >= InViewLevel )
                {
                    // Check if current cell is visible
                    InView = View.BBoxInView( GetCellBBox( Cell ), CheckPlaneMask );

                    if( !DoCulling && InView==0 )
                        InView = 2;
/*
                    // Apply occlusion
 TOMAS
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

                    // is ismpletly in view then set the new inviewlevel
                    if( InView == 1 )
                        InViewLevel = Level;
                }

                if( InView )
                {
                    // Is there anything in this node?
                    if( Cell.OccFlags )
                    {
                        Cell.SearchNext     = m_FirstSearch;
                        m_FirstSearch       = Cursor[Level];
                        ASSERT(Level < MAX_LEVELS) ;
                        Cursor[Level]       = Cell.Next;
                        m_nSearchNodes++;

                        Cell.Flags |= (u8)CheckPlaneMask;

                        if( InView == 2 )
                        {
                        //    Cell.Flags |= spatial_cell::FLAG_PARTIALLY_VISIBLE;
                        }

                    }

                    // Push children
                    if( Level > 0 )
                    {
                        ASSERT(Level < MAX_LEVELS) ;
                        Cursor[Level] = Cell.Next;
                        Level--;
                        ASSERT(Level < MAX_LEVELS) ;
                        Cursor[Level] = Cell.Child;
                    }
                }
                else
                {
                    // Move to next in list
                    ASSERT(Level < MAX_LEVELS) ;
                    Cursor[Level] = Cell.Next;
                }
            }
        }
    }

    return m_FirstSearch;
}


//==============================================================================

u16 spatial_dbase::TraverseCells( 
    const bbox&     RegionBBox, 
    u32             OccFlags )
{
    CONTEXT( "spatial_dbase::TraverseCells-BBox" );

    s32 i,X,Y,Z;
    s32 MinX,MinY,MinZ;
    s32 MaxX,MaxY,MaxZ;
    s32 RegionLevel;
    s32 nChecked=0;
    u16 Cursor[MAX_LEVELS];

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
        if(CellID != SPATIAL_CELL_NULL )
        {
        
            spatial_cell* pCell = &GetCell( CellID );

            if( pCell->SearchSeq != m_SearchSeq )
            {
//                pCell->SearchSeq = m_SearchSeq;

                // Run up parents
//                s32 C=0;
                while( pCell->Parent != SPATIAL_CELL_NULL )
                {
                    //ASSERT( (C++) < 1000 );
                    u16  ID = pCell->Parent;

                    pCell = &GetCell( ID );

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
                    ASSERT(CLevel < MAX_LEVELS) ;
                    Cursor[CLevel] = GetCellIndex( CX, CY, CZ, CLevel);
                    s32 Level = CLevel;

//                    s32 C=0;
                    while( Level<=CLevel )
                    {
                        //ASSERT( (C++) < 1000 ); 

                        // Pop up
                        if( Cursor[Level] == SPATIAL_CELL_NULL )
                        {
                            Level++;
                            continue;
                        }

                        // Get cell
                        nChecked++;
                        u16           ID    = Cursor[Level];
                        spatial_cell* pCell = &GetCell( ID );
                        ASSERT(pCell);

                        if( pCell->SearchSeq == m_SearchSeq )
                        {
//                            Level++;
                            break;
                        }

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
                            ASSERT(Level < MAX_LEVELS) ;
                            Cursor[Level] = pCell->Next;

                            // Push children
                            if( Level > 0 )
                            {
                                Level--;
                                ASSERT(Level < MAX_LEVELS) ;
                                Cursor[Level] = pCell->Child;
                            }
                        }
                        else
                        {
                            // Move to next in list
                            ASSERT(Level < MAX_LEVELS) ;
                            Cursor[Level] = pCell->Next;
                        }
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

    return m_FirstSearch;
}

//==============================================================================

u16 spatial_dbase::TraverseCells( const vector3& WorldSpaceRayStart,
                                  const vector3& WorldSpaceRayEnd,
                                        u32      OccFlags )
{
    CONTEXT( "spatial_dbase::TraverseCells-Ray" );

    // Get ray coordinates in spatial-dbase cells
    f32         BaseCellSize    = m_CellWidth[0];
    vector3     RayStart        = WorldSpaceRayStart / BaseCellSize;
    vector3     RayEnd          = WorldSpaceRayEnd / BaseCellSize;
    vector3     RayDir          = RayEnd - RayStart;
    f32         RayLen          = RayDir.Length();
    
    if( RayLen == 0.0f )
        return SPATIAL_CELL_NULL;
    else
        RayDir /= RayLen;

    // Initialize which cell we are starting in                
    s32 Cell[3]={0,0,0};
    Cell[0] = (((s32)(RayStart[0] + 16384))-16384);
    Cell[1] = (((s32)(RayStart[1] + 16384))-16384);
    Cell[2] = (((s32)(RayStart[2] + 16384))-16384);

    // Clear search info
    m_SearchSeq++;
    m_FirstSearch  = SPATIAL_CELL_NULL;
    m_nSearchNodes = 0;

    // Clear the current distance and begin loop through ray length
    f32 CurrentDist=0;
    while( CurrentDist < RayLen )
    {
        // Process current Cell
        {
            s32 CX = Cell[0];
            s32 CY = Cell[1];
            s32 CZ = Cell[2];

            // Find the first cell that actually contains something
            s32 CellID = -1;
            s32 CellLevel = 0;
            while( CellLevel < MAX_LEVELS )
            {
                CellID = GetCellIndex( CX, CY, CZ, CellLevel );
                if( CellID != SPATIAL_CELL_NULL )
                    break;

                CX >>= 1;
                CY >>= 1;
                CZ >>= 1;
                CellLevel++;
            }
            
            // Process this cell and parents
            while( CellID != SPATIAL_CELL_NULL )
            {
                // Get access to this cell
                spatial_cell* pCell = &GetCell( CellID );

                // If we have seen this cell during this search already
                // then we know we have seen all the parents so bail.
                if( pCell->SearchSeq == m_SearchSeq )
                    break;

                // Remember we have visited this cell
                pCell->SearchSeq = m_SearchSeq;

                // Add this cell to the search results
                if( pCell->OccFlags & OccFlags )
                {
                    m_nSearchNodes++;
                    
                    if( m_FirstSearch == SPATIAL_CELL_NULL )
                    {
                        pCell->SearchNext = SPATIAL_CELL_NULL;
                        m_FirstSearch     = CellID;
                    }
                    else
                    {
                        pCell->SearchNext = m_FirstSearch;
                        m_FirstSearch     = CellID;
                    }
                }

                // Go to parent
                CellID = pCell->Parent;
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

    //extern xbool COLL_DISPLAY_OBJECTS;
    //if( COLL_DISPLAY_OBJECTS )
    //    x_DebugMsg("TraverseCells: %d hit\n",m_nSearchNodes);

    return m_FirstSearch;
}

//==============================================================================
