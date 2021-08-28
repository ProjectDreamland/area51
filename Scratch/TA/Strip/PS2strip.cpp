
#include "PS2Strip.hpp"

//==============================================================================
// VARIABLES
//==============================================================================
struct dir
{
    s32 D1,D2;
};
static const s32 s_nDir = 6;
static const dir s_Dir[6]=
{
    {0, 1},
    {1, 0},
    {0, 2},
    {2, 0},
    {1, 2},
    {2, 1}
};

//==============================================================================
// FUNCTIONS
//==============================================================================

//==============================================================================

ps2_strip::ps2_strip( void )
{
    m_RunNum        = 0;
    m_nTriangles    = 0;
    m_nVertices     = 0;
    m_pVertex       = NULL;
}

//==============================================================================

ps2_strip::~ps2_strip( void )
{
    if( m_pVertex )
        delete []m_pVertex;
}

//==============================================================================

void ps2_strip::Begin( s32 nVertices, s32 CacheSize  )
{
    ASSERT( nVertices >= 0 );

    m_nVertices = nVertices;
    m_pVertex   = new vertex[ nVertices ];
    if( m_pVertex == NULL ) 
        e_throw( "Out of memory" );

    x_memset( m_pVertex, 0, nVertices*sizeof(vertex) );

    m_RunNum    = 0;
    m_CacheSize = CacheSize;
    m_Index.SetCapacity( nVertices*3 );
}

//==============================================================================

void ps2_strip::AddTri( s32 I1, s32 I2, s32 I3 )
{
    ASSERT( I1 >= 0 );
    ASSERT( I1 < m_nVertices );
    ASSERT( I2 >= 0 );
    ASSERT( I2 < m_nVertices );
    ASSERT( I3 >= 0 );
    ASSERT( I3 < m_nVertices );

    xhandle   hTri;
    triangle& Tri = m_lTriangle.Add( hTri );

    m_pVertex[I1].hTriangle[ m_pVertex[I1].nRef ] = hTri;
    m_pVertex[I1].nRef++;
    ASSERT( m_pVertex[I1].nRef < 32 );

    m_pVertex[I2].hTriangle[ m_pVertex[I2].nRef ] = hTri;
    m_pVertex[I2].nRef++;
    ASSERT( m_pVertex[I2].nRef < 32 );

    m_pVertex[I3].hTriangle[ m_pVertex[I3].nRef ] = hTri;
    m_pVertex[I3].nRef++;
    ASSERT( m_pVertex[I3].nRef < 32 );

    Tri.iV[0] = I1;
    Tri.iV[1] = I2;
    Tri.iV[2] = I3;

    Tri.RunNum = 0;

}

//==============================================================================

xhandle ps2_strip::GetNextTri( xhandle hTri, s32 iD1, s32 iD2 )
{
    triangle& Tri = m_lTriangle( hTri );

    // Must find a triangle that share same 2 verts    
    vertex& V1 = m_pVertex[ Tri.iV[iD1] ];
    vertex& V2 = m_pVertex[ Tri.iV[iD2] ];
    
    s32 i;
    for( i=0; i<V1.nRef; i++ )
    {
        if( V1.hTriangle[i] == hTri )
            continue;

        for( s32 j=0; j<V2.nRef; j++ )
        {
            if( V2.hTriangle[j] == hTri )
                continue;

            if( V1.hTriangle[i] == V2.hTriangle[j] )
                break;
        }

        // We did find it so must be done
        if( j != V2.nRef )
            break;
    }

    if( i == V1.nRef )
    {
        xhandle hNull;
        hNull.Handle = HNULL;
        return hNull;
    }
        
    return V1.hTriangle[i];
}

//==============================================================================

void ps2_strip::GetNextDirection( xhandle hNext, xhandle hTri, s32& iD1, s32& iD2 )
{
    triangle& Tri       = m_lTriangle( hTri );
    triangle& NextTri   = m_lTriangle( hNext );
    s32       iD1x=-1;
    s32       iD2x=-1;
    s32       iD3x=-1;

    for( s32 i=0; i<3; i++ )
    {
        if( NextTri.iV[i] == Tri.iV[iD1] )
        {
            iD1x = i;
            continue;
        }

        if( NextTri.iV[i] == Tri.iV[iD2] )
        {
            iD2x = i;
            continue;
        }

        iD3x = i;
    }

    ASSERT( iD1x != -1 ) ;
    ASSERT( iD2x != -1 ) ;
    ASSERT( iD3x != -1 ) ;

    // Update the indices
    iD1 = iD3x;
    iD2 = iD1x;

    ASSERT( iD1 <= 2 );
    ASSERT( iD1 >= 0 );
    ASSERT( iD2 <= 2 );
    ASSERT( iD2 >= 0 );
}

//==============================================================================

void ps2_strip::UpdateRun( void )
{
    m_RunNum++;
    if( m_RunNum == 0 )
    {
        for( s32 i=0; i<m_lTriangle.GetCount(); i++ )
        {
            m_lTriangle[i].RunNum = 0;
        }
        m_RunNum++;
    }
}

//==============================================================================

s32 ps2_strip::ComputeRunScore( xhandle hTri, s32 iD1, s32 iD2 )
{
    s32 nTris = 0;
    UpdateRun();

    // Upadte the run
    m_lTriangle(hTri).RunNum = m_RunNum;

    xhandle hNext = GetNextTri( hTri, iD1, iD2 );
    while( hNext != HNULL )
    {
        nTris++;

        // Stop if we run into the same triantle twice
        if( m_lTriangle(hNext).RunNum == m_RunNum )
            break;

        // update the run
        m_lTriangle(hNext).RunNum = m_RunNum;

        // Get next dir
        GetNextDirection( hNext, hTri, iD1, iD2 );

        // Get next tri
        hTri  = hNext;
        hNext = GetNextTri( hNext, iD1, iD2 );
    }

    return nTris;
}

//==============================================================================

xhandle ps2_strip::GetBestRun( s32& iD1, s32& iD2, s32 Size )
{
    s32         BestScore=-1;
    xhandle     hBestTri;
    s32         BestDir[2];

    for( s32 i=0; i<m_lTriangle.GetCount(); i++ )
    {        
        s32     ExtraScore = 0;
        xhandle hTri       = m_lTriangle.GetHandleByIndex( i );     

        // TODO: Add bonus points from using one of the previous vertex in the index ( one of the last two )
        // Give extra score if there are missing conecting edge
        if( GetNextTri( hTri, 0, 1 ) == HNULL ) ExtraScore += 1000;
        if( GetNextTri( hTri, 1, 2 ) == HNULL ) ExtraScore += 1000;
        if( GetNextTri( hTri, 0, 2 ) == HNULL ) ExtraScore += 1000;
        
        for( s32 j=0; j<s_nDir; j++ )
        {
            s32 Score    = 0;
            s32 RunScore = ComputeRunScore( hTri, s_Dir[j].D1, s_Dir[j].D2 );

            //
            // Give bonus points for matching the expected size
            //
            if( Size == RunScore )
                ExtraScore += 10000;

            // We want to be bias towards shorter strips and keep the longer ones for later
            if( RunScore > (Size-2)  )
                ExtraScore -= 10000;

            // Tally up all the score.
            Score = ExtraScore + RunScore;


            if( BestScore < Score )
            {
                BestScore  = Score;
                hBestTri   = hTri;
                BestDir[0] = s_Dir[j].D1;
                BestDir[1] = s_Dir[j].D2;
            }
        }
    }

    // Return the best tri
    iD1 = BestDir[0];
    iD2 = BestDir[1];
    return hBestTri;
}

//==============================================================================

void ps2_strip::RemoveVert( s32 iVert, xhandle hTri )
{
    vertex& V = m_pVertex[ iVert ];

    s32 i,j;
    for( i=j=0; i<V.nRef; i++ )
    {
        V.hTriangle[j] = V.hTriangle[i];
        if( V.hTriangle[i] != hTri ) 
            j++;
    }
    V.nRef--;
}

//==============================================================================

void ps2_strip::RemoveTri( xhandle hTri )
{
    triangle& Tri = m_lTriangle( hTri );

    for( s32 i=0; i<3; i++)
        RemoveVert( Tri.iV[i], hTri );

    m_lTriangle.DeleteByHandle( hTri );
}

//==============================================================================

s32 ps2_strip::GetLastIndex( xhandle hTri, s32 iD1, s32 iD2 )
{
    triangle& Tri       = m_lTriangle( hTri );

    for( s32 i=0; i<3; i++ )
    {
        if( i != iD1 && i != iD2 )
            break;
    }

    ASSERT( i != 3 );
    return Tri.iV[i];
}

//==============================================================================
// TODO: Check the previous 2 verts from the index list to see if any thing can
//       be reuse to avoid setting two initial verts
void ps2_strip::BuildStrip( xhandle hTri, s32 iD1, s32 iD2, s32 Count )
{
    UpdateRun();

    // update the run
    m_lTriangle(hTri).RunNum = m_RunNum;

    s32 iLast     = GetLastIndex( hTri, iD1, iD2 );

    Count -= 3;
    m_Index.Append() = iLast | (1<<30);
    m_Index.Append() = m_lTriangle(hTri).iV[iD2]   | (1<<30);
    m_Index.Append() = m_lTriangle(hTri).iV[iD1]   ;

    xhandle hNext = GetNextTri  ( hTri, iD1, iD2 );   
    RemoveTri( hTri );

    while( hNext != HNULL && Count > 0 )
    {
        // Stop if we run into the same triantle twice
        if( m_lTriangle(hNext).RunNum == m_RunNum )
            break;

        // update the run
        m_lTriangle(hNext).RunNum = m_RunNum;

        // Get next dir
        GetNextDirection( hNext, hTri, iD1, iD2 );
        m_Index.Append() = m_lTriangle(hNext).iV[iD1];
        Count --;

        // Get next tri
        hTri  = hNext;
        hNext = GetNextTri( hNext, iD1, iD2 );

        // Make sure to collect the last just in case this is the last try
        RemoveTri( hTri );
    }
}

//==============================================================================

void ps2_strip::End( void )
{
    // Get the total number of triangles
    m_nTriangles = m_lTriangle.GetCount();

    //
    // Compute the strips
    //
    while( m_lTriangle.GetCount() )
    {
        s32 D1, D2;

        s32 Count = m_CacheSize - (m_Index.GetCount()%m_CacheSize);
        ASSERT( Count >= 0 );

        xhandle hTri = GetBestRun( D1, D2, Count );
        BuildStrip( hTri, D1, D2, Count );
    }
}

//==============================================================================

s32 ps2_strip::GetIndexCount( void  ) const
{
    return m_Index.GetCount();
}

//==============================================================================

s32 ps2_strip::IsIndexNewStrip( s32 I ) const
{
    return (m_Index[I]&(1<<30)) != 0;
}

//==============================================================================

s32 ps2_strip::GetIndex( s32 I ) const
{
    return m_Index[I] & (~(1<<30));
}

//==============================================================================

f32 ps2_strip::GetFinalScore( void ) const
{
    return GetIndexCount() / (f32)m_nTriangles;
}

//==============================================================================

s32 ps2_strip::GetCacheSize( void ) const
{
    return m_CacheSize;
}