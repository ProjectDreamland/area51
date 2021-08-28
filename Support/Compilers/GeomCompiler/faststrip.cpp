//=========================================================================
//
//  FASTSTRIP.CPP
//
//=========================================================================
                                                        
#include <windows.h>
#include "x_files.hpp"
#include "faststrip.hpp"

//=========================================================================
//=========================================================================
//=========================================================================

static s32 s_StartingVert[6]   = {0,1,2,0,1,2};
static s32 s_StartingCW[6]     = {0,0,0,1,1,1};
static s32 s_ExitVert[6]       = {1,2,0,2,0,1};
xtimer ScoreStripTime;
xtimer SortStripTime;

//#define LOG x_DebugMsg
#define LOG

//=========================================================================
//=========================================================================
//=========================================================================

faststrip::faststrip( void )
{
    m_pTri = NULL;
    m_pStrip = NULL;
    m_pHash = NULL;
    Clear();
}

//=========================================================================

faststrip::~faststrip( void )
{
    Clear();
}

//=========================================================================

void faststrip::Clear( void )
{
    x_free( m_pTri );
    m_pTri = NULL;
    m_nTris = 0;
    m_nTrisAllocated = 0;

    x_free( m_pStrip );
    m_pStrip = NULL;
    m_nStrips = 0;
    m_nKeepStrips = 0;

    x_free( m_pHash );
    m_nHashEntries = 0;

    m_MinStripLen = 0;
    m_DoBackfaceCulling = TRUE;
    m_TriSequence = 0;
    m_StripSequence = 0;

    SetWeights(0.212642f, 0.539046f, 0.162557f, 0.017980f, 0.067775f );

    m_RescorePercentage = 0.1f;
}

//=========================================================================

void faststrip::SetWeights( f32 W0, f32 W1, f32 W2, f32 W3, f32 W4 )
{
    m_ScoreWeight[0] = W0;
    m_ScoreWeight[1] = W1;
    m_ScoreWeight[2] = W2;
    m_ScoreWeight[3] = W3;
    m_ScoreWeight[4] = W4;

    f32 T=0;
    s32 i;
    for( i=0; i<5; i++ )
        T += m_ScoreWeight[i];
    for( i=0; i<5; i++ )
        m_ScoreWeight[i] /= T;
}

//=========================================================================

void faststrip::Open( s32 MinStripLen, xbool DoBackfaceCulling )
{
    Clear();
    m_MinStripLen = MinStripLen;
    m_DoBackfaceCulling = DoBackfaceCulling;
}

//=========================================================================

void faststrip::AddTri( s32 I1, s32 I2, s32 I3 )
{
    // Extend array of tris if we need to
    if( m_nTris == m_nTrisAllocated )
    {
        m_nTrisAllocated += 2048;
        m_pTri = (tri*)x_realloc( m_pTri, sizeof(tri)*m_nTrisAllocated );
        ASSERT( m_pTri );
    }

    // Build new tri
    tri& T = m_pTri[m_nTris++];

    T.IsActive = TRUE;
    T.Sequence = 0;
    T.VID[0] = I1;
    T.VID[1] = I2;
    T.VID[2] = I3;
    T.NID[0] = -1;
    T.NID[1] = -1;
    T.NID[2] = -1;
    T.NVID[0] = -1;
    T.NVID[1] = -1;
    T.NVID[2] = -1;
    T.nActiveNeighbors = 0;
    for( s32 i=0; i<6; i++ ) T.FirstInStripID[i] = -1;
}

//=========================================================================

void faststrip::Close( void )
{
    xtimer TotalTime;
    xtimer SetupTime;
    xtimer ScoreTime;
    xtimer InitialScoreTime;
    xtimer FindBestTime;
    xtimer CleanupTime;
    TotalTime.Start();

    SetupTime.Start();

        LOG("--------------------------------------------\n");
        LOG("Stripping: %d triangles\n",m_nTris);

        LOG("Initializing hashtable...\n");
        InitHashTable();

        LOG("Solving triangle neighbors...\n");
        SolveTriangleNeighbors();

        LOG("Enumerating all strips...\n");
        EnumerateAllStrips();

    SetupTime.Stop();

    InitialScoreTime.Start();
        LOG("Computing initial scoring...\n");
        ScoreAndSortAllStrips();
    InitialScoreTime.Stop();

    //DumpScores("c:/temp/dump.txt");
    //DumpHashTable("c:/temp/hashdump.txt");

    LOG("Finding optimal strips...\n");
    s32 C=0;
    while( m_nActiveStrips )
    {
        ScoreTime.Start();
            UpdateStripScores();
        ScoreTime.Stop();

        FindBestTime.Start();
            FindBestStrip();
        FindBestTime.Stop();

        C++;
        if( (m_nTris>5000) && (C%50) == 0 )
        {
            LOG("found...%d remaining...%d\n",m_nKeepStrips,m_nActiveStrips);
            //LOG("ScoreTime: %1.4f\n",ScoreTime.ReadMs());
            //LOG("FindBest:  %1.4f\n",FindBestTime.ReadMs());
            //SanityCheck();
        }

    }

    LOG("Cleaning...\n");

    // Move all kept strips to top of strip list
    CleanupTime.Start();
    {
        s32 i,j=0;
        for( i=0; i<m_nStrips; i++ )
        {
            if( m_pStrip[i].Keep )
            {
                m_pStrip[j] = m_pStrip[i];
                j++;
            }
        }
    }
    CleanupTime.Stop();

    TotalTime.Stop();
    LOG("**** STRIPPING ****\n");
    LOG("SetupTime:        %1.4f\n",SetupTime.ReadMs());
    LOG("InitialScoreTime: %1.4f\n",InitialScoreTime.ReadMs());
    LOG("Score&SortTime:   %1.4f\n",ScoreTime.ReadMs());
    LOG("  ScoreTime:      %1.4f %d\n",ScoreStripTime.ReadMs(),ScoreStripTime.GetNSamples());
    LOG("  SortTime:       %1.4f %d\n",SortStripTime.ReadMs(),SortStripTime.GetNSamples());
    LOG("FindBest:         %1.4f\n",FindBestTime.ReadMs());
    LOG("CleanupTime:      %1.4f\n",CleanupTime.ReadMs());
    LOG("------------------------\n");
    LOG("TotalTime:        %1.4f\n",TotalTime.ReadMs());

    //DumpFinalStats("c:/temp/finalstrips.txt");

}

//=========================================================================
//=========================================================================
//=========================================================================
// SETUP FUNCTIONS
//=========================================================================
//=========================================================================
//=========================================================================

void faststrip::SolveTriangleNeighbors( void )
{
    s32     i,j,k;
    s32*    pVertTriID;
    s32*    pTriNext;

    // Determine number of verts used by triangles
    s32 MaxVI = 0;
    s32 MinVI = S32_MAX;
    s32 nVerts;
    for( i=0; i<m_nTris; i++ )
    {
        MaxVI = MAX( MaxVI, m_pTri[i].VID[0] );
        MaxVI = MAX( MaxVI, m_pTri[i].VID[1] );
        MaxVI = MAX( MaxVI, m_pTri[i].VID[2] );
        MinVI = MIN( MinVI, m_pTri[i].VID[0] );
        MinVI = MIN( MinVI, m_pTri[i].VID[1] );
        MinVI = MIN( MinVI, m_pTri[i].VID[2] );
    }
    nVerts = MaxVI+1;

    // Allocate room for triangle next array
    pTriNext = (s32*)x_malloc(sizeof(s32)*m_nTris*3);
    ASSERT(pTriNext);
    x_memset( pTriNext, 0xFFFFFFFF, sizeof(s32)*m_nTris*3 );

    // Allocate room for starting index for each vert's triangle list
    pVertTriID = (s32*)x_malloc(sizeof(s32)*nVerts);
    ASSERT(pVertTriID);
    x_memset( pVertTriID, 0xFFFFFFFF, sizeof(s32)*nVerts );

    // Setup tris
    for( i=0; i<m_nTris; i++ )
    {
        // Add tri to vert list
        for( j=0; j<3; j++ )
        {
            s32 VID = m_pTri[i].VID[j];
            pTriNext[i*3+j] = pVertTriID[VID];
            pVertTriID[VID] = i;
        }
    }

    {
        s32 i,j;

        // Loop through used verts
        for( s32 V=MinVI; V<=MaxVI; V++ )
        {
            // Loop through tris touching vert
            s32 TA = pVertTriID[V];
            while( TA != -1 )
            {
                // Find next in list for TA
                s32 TANext;
                if( m_pTri[TA].VID[0] == V ) TANext = pTriNext[TA*3+0];
                else
                if( m_pTri[TA].VID[1] == V ) TANext = pTriNext[TA*3+1];
                else
                if( m_pTri[TA].VID[2] == V ) TANext = pTriNext[TA*3+2];

                // Loop through next and rest of tris for edge match
                s32 TB = TANext;
                while( TB != -1 )
                {
                    // Compare edges of TA and TB
                    ASSERT( TA != TB );

                    // If TB has all its neighbors don't bother looking for edges
                    if( m_pTri[TB].nActiveNeighbors < 3 )
                    {
                        xbool FoundEdge = FALSE;
                        for( i=0; i<3; i++ )
                        if( m_pTri[TA].NID[i] == -1 )
                        {
                            s32 TA0,TA1;
                            TA0 = m_pTri[TA].VID[(i+0)%3];
                            TA1 = m_pTri[TA].VID[(i+1)%3];

                            for( j=0; j<3; j++ )
                            if( m_pTri[TB].NID[j] == -1 )
                            {
                                s32 TB0,TB1;
                                TB0 = m_pTri[TB].VID[(j+0)%3];
                                TB1 = m_pTri[TB].VID[(j+1)%3];

                                // Found shared edge
                                if( (TA0==TB1) && (TA1==TB0) )
                                {
                                    ASSERT( m_pTri[TA].nActiveNeighbors<3 );
                                    ASSERT( m_pTri[TB].nActiveNeighbors<3 );

                                    m_pTri[TA].NID[i]  = TB;     
                                    m_pTri[TA].NVID[i] = j;
                                    m_pTri[TA].nActiveNeighbors++;

                                    m_pTri[TB].NID[j]  = TA;     
                                    m_pTri[TB].NVID[j] = i;
                                    m_pTri[TB].nActiveNeighbors++;

                                    FoundEdge = TRUE;
                                }

                                if( FoundEdge )
                                    break;
                            }

                            if( FoundEdge )
                                break;
                        }

                        // If we've found all the neighbors for TA stop looking
                        if( m_pTri[TA].nActiveNeighbors==3 )
                            break;
                    }

                    // Find next in list for TB
                    if( m_pTri[TB].VID[0] == V ) TB = pTriNext[TB*3+0];
                    else
                    if( m_pTri[TB].VID[1] == V ) TB = pTriNext[TB*3+1];
                    else
                    if( m_pTri[TB].VID[2] == V ) TB = pTriNext[TB*3+2];
                }

                TA = TANext;
            }
        }
    }

    if( 0 )
    {
        // Be sure all neighbors are orthogonal
        for( i=0; i<m_nTris; i++ )
        {
            for( j=0; j<3; j++ )
            if( m_pTri[i].NID[j] != -1 )
            {
                for( k=0; k<3; k++ )
                if( m_pTri[m_pTri[i].NID[j]].NID[k] == i )
                    break;

                ASSERT( k!=3 );
            }
        }
    }

    // Deallocate verts and triangle next list
    x_free(pVertTriID);
    x_free(pTriNext);
}

//=========================================================================

void faststrip::EnumerateAllStrips( void )
{
    s32 i,j;

    // Allocate room for all the strips
    m_nStripsPerTri = (m_DoBackfaceCulling) ? (3):(6);
    m_nStrips = m_nStripsPerTri*m_nTris;
    m_pStrip = (strip*)x_malloc(sizeof(strip)*m_nStrips);
    ASSERT(m_pStrip);
    x_memset( m_pStrip, 0, sizeof(strip)*m_nStrips );

    // Build description of initial scripts
    for( i=0; i<m_nTris; i++ )
    for( j=0; j<m_nStripsPerTri; j++ )
    {
        strip& S = m_pStrip[ i*m_nStripsPerTri + j ];
        S.iStartTri = i;
        S.iStartCW   = s_StartingCW[j];
        S.iStartVert = s_StartingVert[j];
        S.IsActive  = TRUE;
        S.Keep      = FALSE;
        S.iEndTri   = -1;
    }

    m_nActiveStrips = m_nStrips;
}

//=========================================================================
//=========================================================================
//=========================================================================
// STRIPPING FUNCTIONS
//=========================================================================
//=========================================================================
//=========================================================================

void faststrip::DeactivateStrip( s32 iStrip )
{
    ASSERT( (iStrip>=0) && (iStrip<m_nStrips) );
    ASSERT( m_pStrip[iStrip].IsActive == TRUE );

    strip& S = m_pStrip[iStrip];
    S.IsActive = FALSE;
    m_nActiveStrips--;

    // Remove from hash
    RemoveStripFromHash( iStrip );
}

//=========================================================================

void faststrip::ScoreAndSortAllStrips( void )
{
    // Score all strips and hook into hash table
    for( s32 i=0; i<m_nStrips; i++ )
    {
        ScoreStrip(i);
        AddStripToHash(i);
    }
}

//=========================================================================

void faststrip::UpdateStripScores( void )
{
    s32 nToRescore = (s32)(m_nActiveStrips * m_RescorePercentage);
    nToRescore = MAX(1,nToRescore);
    nToRescore = MIN(1000,nToRescore);
    //nToRescore = 50;

    StepStripSequence();

    s32 nRescored=0;
    s32 MinI = m_MinHashIndex;
    s32 MaxI = m_MaxHashIndex;
    for( s32 i=MaxI; i>=MinI; i-- )
    if( m_pHash[i] != -1 )
    {
        s32 SID = m_pHash[i];
        while( (SID != -1) && (nRescored<nToRescore) )
        {
            s32 NextSID = m_pStrip[SID].HashNext;

            if( m_pStrip[SID].Sequence != m_StripSequence)
            {
                nRescored++;
                m_pStrip[SID].Sequence = m_StripSequence;

                //ScoreStripTime.Start();
                    ScoreStrip(SID);
                //ScoreStripTime.Stop();

                //SortStripTime.Start();
                    ResortStripInHash(SID);
                //SortStripTime.Stop();
            }

            SID = NextSID;
        }

        if( nRescored >= nToRescore )
            break;
    }

    // Keep scoring top strip until it wins
    while( 1 )
    {
        s32 SID = m_pHash[m_MaxHashIndex];
        ASSERT( SID != -1 );
        if( m_pStrip[SID].Sequence == m_StripSequence )
            break;
        m_pStrip[SID].Sequence = m_StripSequence;
        ScoreStrip(SID);
        ResortStripInHash(SID);
    }
    
    //LOG("UpdateScores: %d %d %5.1f\n",m_nActiveStrips,nRescored,100.0f*(f32)nRescored/(f32)m_nActiveStrips);
}

//=========================================================================

void faststrip::StepTriSequence( void )
{
    s32 i;

    if( m_TriSequence == 0x7FFFFFFF )
    {
        m_TriSequence = 0;

        for( i=0; i<m_nStrips; i++ )
            m_pStrip[i].Sequence = 0;
    }

    m_TriSequence++;
}

//=========================================================================

void faststrip::StepStripSequence( void )
{
    s32 i;

    if( m_StripSequence == 0x7FFFFFFF )
    {
        m_StripSequence = 0;

        for( i=0; i<m_nStrips; i++ )
            m_pStrip[i].Sequence = 0;
    }

    m_StripSequence++;
}

//=========================================================================

void faststrip::ScoreStrip( s32 iStrip )
{
    ASSERT( m_pStrip[iStrip].IsActive );

    StepTriSequence();

    strip& S        = m_pStrip[iStrip];
    s32 iTri        = S.iStartTri;
    s32 CWOffset    = (S.iStartCW) ? (3) : (0);
    s32 iEntryVert  = S.iStartVert;
    s32 iExitVert   = s_ExitVert[ iEntryVert + CWOffset ];
    s32 nTris       = 0;
    s32 Stats[4]={0,0,0,0};

    while( 1 )
    {
        tri& T = m_pTri[ iTri ];

        // Mark tri as visited
        T.Sequence = m_TriSequence;
        nTris++;

        // How many active neighbors does this tri have?
        Stats[T.nActiveNeighbors]++;

        // Find next tri
        s32 iNextTri = T.NID[ iExitVert ];

        // Check if we have terminated strip
        if( (iNextTri==-1) || 
            (m_pTri[iNextTri].Sequence == m_TriSequence) ||
            (m_pTri[iNextTri].IsActive != TRUE))
            break;

        // Switch direction
        CWOffset = (CWOffset) ? (0) : (3);

        // Decide entry vert of new tri
        iEntryVert = T.NVID[ iExitVert ];
        iExitVert  = s_ExitVert[ iEntryVert + CWOffset ];

        // Switch tris
        iTri = iNextTri;
    }

    S.Len = nTris;
    S.iEndTri = iTri;
    S.Score = (s32)(1000.0f*( Stats[0]*m_ScoreWeight[0] +
                              Stats[1]*m_ScoreWeight[1] +
                              Stats[2]*m_ScoreWeight[2] +
                              Stats[3]*m_ScoreWeight[3] +
                              S.Len*m_ScoreWeight[4] ));

    if( S.Len == 1 )
        S.Score = 0;
    //S.Score = S.Len;
}

//=========================================================================

void faststrip::FindBestStrip( void )
{
    ASSERT( m_pHash[ m_MaxHashIndex ] != -1 );
    s32 SID = m_pHash[m_MaxHashIndex];

    //if( m_pStrip[SID].Len == 1 )
        //DumpScores("scores.txt");

    m_pStrip[SID].Keep = TRUE;    
    m_nKeepStrips++;

    // Mark strip triangles as kept
    StepTriSequence();

    strip& S        = m_pStrip[SID];
    s32 iTri        = S.iStartTri;
    s32 CWOffset    = (S.iStartCW) ? (3) : (0);
    s32 iEntryVert  = S.iStartVert;
    s32 iExitVert   = s_ExitVert[ iEntryVert + CWOffset ];

    while( 1 )
    {
        tri& T = m_pTri[iTri];

        ASSERT( iTri >= 0 );

        // Mark triangle that it belongs to a strip
        T.IsActive = FALSE;

        // Deactivate all strips that begin with this tri
        for( s32 i=0; i<m_nStripsPerTri; i++ )
            DeactivateStrip( (iTri*m_nStripsPerTri)+i );

        // Tell all neighbor triangles it is no longer active
        for( i=0; i<3; i++ )
        if( T.NID[i] != -1 )
            m_pTri[T.NID[i]].nActiveNeighbors--;

        // Mark tri as visited
        T.Sequence = m_TriSequence;

        // Find next tri
        s32 iNextTri = T.NID[ iExitVert ];

        // Check if we have terminated strip
        if( (iNextTri==-1) || 
            (m_pTri[iNextTri].Sequence == m_TriSequence) ||
            (m_pTri[iNextTri].IsActive != TRUE))
            break;

        // Switch direction
        CWOffset = (CWOffset) ? (0) : (3);

        // Decide entry vert of new tri
        iEntryVert = T.NVID[ iExitVert ];
        iExitVert  = s_ExitVert[ iEntryVert + CWOffset ];

        // Switch tris
        iTri = iNextTri;
    }
}

//=========================================================================
//=========================================================================
//=========================================================================
// POST-STRIPPING INTERFACE
//=========================================================================
//=========================================================================
//=========================================================================

s32 faststrip::GetNStrips( void ) const
{
    return m_nKeepStrips;
}

//=========================================================================

s32 faststrip::GetStripNTris( s32 iStrip ) const
{
    return m_pStrip[iStrip].Len;
}

//=========================================================================

void faststrip::GetStripTris( s32 iStrip, s32* pTriIndices ) 
{
    StepTriSequence();

    strip& S        = m_pStrip[iStrip];
    s32 iTri        = S.iStartTri;
    s32 CWOffset    = (S.iStartCW) ? (3) : (0);
    s32 iEntryVert  = S.iStartVert;
    s32 iExitVert   = s_ExitVert[ iEntryVert + CWOffset ];
    s32 nTris       = 0;

    while( 1 )
    {
        ASSERT( iTri >= 0 );
        ASSERT( m_pTri[iTri].IsActive == FALSE );

        // Mark tri as visited
        m_pTri[ iTri ].Sequence = m_TriSequence;
        pTriIndices[ nTris ] = iTri;
        nTris++;

        if( iTri == S.iEndTri )
            break;

        // Find next tri
        s32 iNextTri = m_pTri[iTri].NID[ iExitVert ];

        // Check if we have terminated strip
        if( (iNextTri==-1) || 
            (m_pTri[iNextTri].Sequence == m_TriSequence))
            break;

        // Switch direction
        CWOffset = (CWOffset) ? (0) : (3);

        // Decide entry vert of new tri
        iEntryVert = m_pTri[iTri].NVID[ iExitVert ];
        iExitVert  = s_ExitVert[ iEntryVert + CWOffset ];

        // Switch tris
        iTri = iNextTri;
    }
}

//=========================================================================

s32* faststrip::GetIndices( s32& nIndices )
{
    s32 i,j;
    nIndices = (m_nKeepStrips*2 + m_nTris);
    nIndices += (m_nKeepStrips-1)*3;

    // Allocate room for indices
    s32* pI = (s32*)x_malloc(sizeof(s32)*nIndices);
    ASSERT( pI );
    x_memset(pI,0xFFFFFFFF,sizeof(s32)*nIndices);

    // Allocate buffer for tri list
    s32* pTI = (s32*)x_malloc(sizeof(s32)*4096);
    ASSERT(pTI);

    // Start ptr in index array
    s32 I=0;
    xbool CCW = TRUE;

    // Loop through strips and glue together
    for( i=0; i<m_nKeepStrips; i++ )
    {   
        strip& S = m_pStrip[i];
        GetStripTris( i, pTI );

        // Sew beginning of new strip with end of old
        if( i!=0 )
        {
            ASSERT( I < nIndices-2 );
            pI[I+0] = pI[I-1];
            CCW = !CCW;
            I++;

            pI[I] = m_pTri[ S.iStartTri ].VID[S.iStartVert];
            CCW = !CCW;
            I++;

            if( !CCW )
            {
                pI[I] = m_pTri[ S.iStartTri ].VID[S.iStartVert];
                CCW = !CCW;
                I++;
            }
        }

        // Solve new strip indices
        s32 CWOffset    = (S.iStartCW) ? (3) : (0);
        s32 iEntryVert  = S.iStartVert;
        s32 iExitVert   = s_ExitVert[ iEntryVert + CWOffset ];

        // Store first two verts
        ASSERT(CCW);
        ASSERT( I < nIndices-2 );
        pI[I+0] = m_pTri[ S.iStartTri ].VID[S.iStartVert];
        pI[I+1] = m_pTri[ S.iStartTri ].VID[(S.iStartVert+1)%3];
        I += 2;
        ASSERT( I < nIndices );
        for( j=0; j<S.Len; j++ )
        {
            s32 iTri = pTI[j];

            ASSERT( I < nIndices );

            // Store exit vert
            if( CWOffset==0 )
                pI[I] = m_pTri[iTri].VID[ (iExitVert+1)%3 ];
            else
                pI[I] = m_pTri[iTri].VID[ iExitVert ];
            I++;
            CCW = !CCW;

            // Switch direction
            CWOffset = (CWOffset) ? (0) : (3);

            // Decide entry vert of new tri
            iEntryVert = m_pTri[iTri].NVID[ iExitVert ];
            iExitVert  = s_ExitVert[ iEntryVert + CWOffset ];
        }

    }

    ASSERT( I <= nIndices );
    nIndices = I;

    x_free(pTI);
    return pI;
}

//=========================================================================

s32 faststrip::GetMaxNumIndices( s32 MaxStripLength )
{
    s32 nIndices = (m_nKeepStrips*2 + m_nTris);
    nIndices += (m_nKeepStrips-1)*3;

    // Worst case of having to pad 2 dummy verts to end of buffer
    // e.g. Attempting to start a new strip with less than 3 verts in the buffer
    nIndices += (nIndices * 2) / MaxStripLength;

    return( nIndices );
}

//=========================================================================

s32 faststrip::GetIndicesPS2( s32* pIndices, s32 MaxStripLength )
{
    s32 i,j;
    s32 nIndices = GetMaxNumIndices( MaxStripLength );

    // Allocate room for indices
    s32* pI = (s32*)x_malloc(sizeof(s32)*nIndices);
    ASSERT( pI );
    x_memset( pI, 0xFF, sizeof(s32) * nIndices );

    // Allocate buffer for tri list
    s32* pTI = (s32*)x_malloc(sizeof(s32)*4096);
    ASSERT(pTI);

    // Start ptr in index array
    s32 I=0;
    xbool bCCW;

    // Loop through strips and glue together
    for( i=0; i<m_nKeepStrips; i++ )
    {   
        strip& S = m_pStrip[i];
        GetStripTris( i, pTI );

        // Solve new strip indices
        s32 CWOffset    = (S.iStartCW) ? (3) : (0);
        s32 iEntryVert  = S.iStartVert;
        s32 iExitVert   = s_ExitVert[ iEntryVert + CWOffset ];

        bCCW = !S.iStartCW;

        // Store first two indexes
        ASSERT( I < nIndices-2 );
        pI[I+0] = m_pTri[ S.iStartTri ].VID[ S.iStartVert     ] | ADC;
        pI[I+1] = m_pTri[ S.iStartTri ].VID[(S.iStartVert+1)%3] | ADC;
        I += 2;
        ASSERT( I < nIndices );

        // Process rest of strip
        for( j=0; j<S.Len; j++ )
        {
            s32 iTri = pTI[j];

            ASSERT( I < nIndices );

            // Store exit vert
            if( CWOffset==0 )
                pI[I] = m_pTri[iTri].VID[ (iExitVert+1)%3 ];
            else
                pI[I] = m_pTri[iTri].VID[ iExitVert ];

            // Store CCW bit.
            if( bCCW )
                pI[I] |= CCW;

            I++;
            bCCW = !bCCW;

            // Switch direction
            CWOffset = (CWOffset) ? (0) : (3);

            // Decide entry vert of new tri
            iEntryVert = m_pTri[iTri].NVID[ iExitVert ];
            iExitVert  = s_ExitVert[ iEntryVert + CWOffset ];
        }
    }

    ASSERT( I <= nIndices );

    x_free(pTI);
    
    //
    // Now go through list and split it into chunks of size MaxStripLength
    //

    if( 1 )
    {
        s32 Count   = I;
        s32 Length  = 0;
        s32 nADC    = 0;
        I = 0;
        
        for( i=0; i<Count; i++ )
        {
            s32 Current = pI[i];
        
            if( Length == MaxStripLength )
                Length = 0;
            
            if( Current & ADC )
            {
                // Ensure the source strip never ends on an ADC bit
                ASSERT( i < (Count-2) );
                ASSERT( pI[ i+1 ] & ADC );
            
                s32 Free = MaxStripLength - Length;
                    
                // Check if there is enough room for at least 1 tri
                if( Free < 3 )
                {
                    // Not enough room in buffer so pad it out with dummy verts and start a new one
                    if( Free == 2 )
                    {
                        // Fill out current buffer with 2 dummy verts
                        pIndices[ I++ ] = Current;
                        pIndices[ I++ ] = Current;
                        Length += 2;
                    }
                    else
                    {
                        // Fill out current buffer with 1 dummy vert
                        pIndices[ I++ ] = Current;
                        Length += 1;
                    }
                    
                    // Start a new buffer since this one is full
                    ASSERT( Length == MaxStripLength );
                    Length = 0;
                }
            
                // Copy the 2 ADC verts
                pIndices[ I++ ] = Current;
                pIndices[ I++ ] = pI[ i+1 ];
                
                // Skip over 2nd ADC vert since we have copied it already
                i++;
                Length += 2;
            }
            else
            {
                // Check if we are starting a new buffer
                if( Length == 0 )
                {
                    // Strip was broken in the middle so we must insert the 2 previous verts
                    ASSERT( i >= 2 );
                    
                    //ASSERT( (pI[ i-1 ] & ADC) == 0 );
                    
                    pIndices[ I++ ] = pI[ i-2 ] | ADC;
                    pIndices[ I++ ] = pI[ i-1 ] | ADC;
                    Length += 2;
                }
                
                pIndices[ I++ ] = Current;
                Length += 1;
                    
                ASSERT( Length <= MaxStripLength );
            }
        }
        
        // Check ADC bits are correct
        for( i=0; i<I; i++ )
        {
            if( ((i % MaxStripLength) == 0) || ( pIndices[i] & ADC) )
            {
                ASSERT( i < (I-2) );
                ASSERT( pIndices[ i+0 ] & ADC );
                
                if( (i+1) % MaxStripLength )
                {
                    ASSERT( pIndices[ i+1 ] & ADC );
                    i++;
                }
            }
        }
    }
    else
    {
        for( i=0; i<I; i++ )
        {
            pIndices[i] = pI[i];
        }
    }
    
    x_free( pI );

    return( I );
}

//=========================================================================

s32 faststrip::GetIndex( s32 Index )
{
    return( Index & ~(ADC | CCW) );
}

//=========================================================================

xbool faststrip::IsIndexNewStrip( s32 Index )
{
    return( (Index & ADC) ? TRUE : FALSE );
}

//=========================================================================

xbool faststrip::IsIndexCCWTri( s32 Index )
{
    return( (Index & CCW) ? TRUE : FALSE );
}

//=========================================================================
//=========================================================================
//=========================================================================
// HASHING FUNCTIONS
//=========================================================================
//=========================================================================
//=========================================================================

void faststrip::InitHashTable( void )
{
    m_nHashEntries = 16384;
    m_pHash = (s32*)x_malloc(sizeof(s32)*m_nHashEntries);
    ASSERT(m_pHash);
    x_memset( m_pHash, 0xFFFFFFFF, sizeof(s32)*m_nHashEntries );
    m_MinHashIndex = m_nHashEntries;
    m_MaxHashIndex = -1;
}

//=========================================================================

void faststrip::AddStripToHash( s32 iStrip )
{
    strip& S = m_pStrip[iStrip];

    // Compute hash index
    S.HID = S.Score;
    ASSERT( S.HID >= 0 );
    if( S.HID > m_nHashEntries-1 ) S.HID = m_nHashEntries-1;

    // Add strip to hash entry
    S.HashNext = m_pHash[S.HID];
    S.HashPrev = -1;
    if( m_pHash[S.HID] != -1 )
        m_pStrip[ m_pHash[S.HID] ].HashPrev = iStrip;
    m_pHash[S.HID] = iStrip;

    // Modify min & max
    m_MaxHashIndex = MAX( m_MaxHashIndex, S.HID );
    m_MinHashIndex = MIN( m_MinHashIndex, S.HID );
}

//=========================================================================

void faststrip::RemoveStripFromHash( s32 iStrip )
{
    strip& S = m_pStrip[iStrip];

    // Unlink S from hash
    if( S.HashNext != -1 )
        m_pStrip[S.HashNext].HashPrev = S.HashPrev;
    if( S.HashPrev != -1 )
        m_pStrip[S.HashPrev].HashNext = S.HashNext;
    if( m_pHash[S.HID] == iStrip )
        m_pHash[S.HID] = S.HashNext;

    // Update Min
    while( (m_MinHashIndex<m_nHashEntries) && (m_pHash[m_MinHashIndex] == -1) )
        m_MinHashIndex++;

    // Update Max
    while( (m_MaxHashIndex>-1) && (m_pHash[m_MaxHashIndex]==-1) )
        m_MaxHashIndex--;

    S.HashNext = -1;
    S.HashPrev = -1;
    S.HID      = -1;
}

//=========================================================================

void faststrip::ResortStripInHash( s32 iStrip )
{
    strip& S = m_pStrip[iStrip];

    // Compute new hash index
    s32 NewHID = S.Score;
    if( NewHID > m_nHashEntries-1 ) NewHID = m_nHashEntries-1;

    // If new hash is same as old hash then bail
    if( NewHID == S.HID )
        return;

    // Remove strip from this hash entry

    if( S.HashNext != -1 )
        m_pStrip[S.HashNext].HashPrev = S.HashPrev;
    if( S.HashPrev != -1 )
        m_pStrip[S.HashPrev].HashNext = S.HashNext;
    if( m_pHash[S.HID] == iStrip )
        m_pHash[S.HID] = S.HashNext;

    // Add strip to new hash entry
    S.HID = NewHID;

    // Add strip to hash entry
    S.HashNext = m_pHash[S.HID];
    S.HashPrev = -1;
    if( m_pHash[S.HID] != -1 )
        m_pStrip[ m_pHash[S.HID] ].HashPrev = iStrip;
    m_pHash[S.HID] = iStrip;

    // Modify min & max
    m_MaxHashIndex = MAX( m_MaxHashIndex, S.HID );
    m_MinHashIndex = MIN( m_MinHashIndex, S.HID );

    // Update Min
    while( (m_MinHashIndex<m_nHashEntries) && (m_pHash[m_MinHashIndex] == -1) )
        m_MinHashIndex++;

    // Update Max
    while( (m_MaxHashIndex>-1) && (m_pHash[m_MaxHashIndex]==-1) )
        m_MaxHashIndex--;
}

//=========================================================================
//=========================================================================
//=========================================================================
// DEBUGGING FUNCTIONS
//=========================================================================
//=========================================================================
//=========================================================================

void faststrip::DumpHashTable( const char* pFilename )
{
    X_FILE* fp;
    fp = x_fopen(pFilename,"wt");
    ASSERT(fp);

    x_fprintf(fp,"nHashEntries: %d\n",m_nHashEntries);
    x_fprintf(fp,"nItems:       %d\n",m_nActiveStrips);
    x_fprintf(fp,"MIN:          %d\n",m_MinHashIndex);
    x_fprintf(fp,"MAX:          %d\n",m_MaxHashIndex);

    for( s32 i=0; i<m_nHashEntries; i++ )
    if( m_pHash[i] != -1 )
    {
        s32 C=0;
        s32 SID = m_pHash[i];
        s32 MinScore=S32_MAX;
        s32 MaxScore=0;

        while( SID != -1 )
        {
            C++;
            MinScore = MIN(MinScore,m_pStrip[SID].Score);
            MaxScore = MAX(MaxScore,m_pStrip[SID].Score);
            SID = m_pStrip[SID].HashNext;
        }

        x_fprintf(fp,"%6d] (%6d,%6d) %d\n",i,MinScore,MaxScore,C);
    }

    x_fclose(fp);
}

//=========================================================================

void faststrip::DumpScores( const char* pFilename )
{
    X_FILE* fp;
    fp = x_fopen(pFilename,"wt");
    ASSERT(fp);

    x_fprintf(fp,"nStrips active: %d\n",m_nActiveStrips);
    s32 C=0;
    for( s32 SID=0; SID<m_nStrips; SID++ )
    if( m_pStrip[SID].IsActive )
    {
        x_fprintf(fp,"%6d       I %6d       L %6d        S %d\n",C,SID,m_pStrip[SID].Len,m_pStrip[SID].Score);
        C++;
    }
    x_fclose(fp);
}

//=========================================================================

void faststrip::DumpFinalStats( const char* pFilename )
{
    X_FILE* fp;
    fp = x_fopen(pFilename,"wt");
    ASSERT(fp);
    s32 LenStats[1000]={0};
    s32 TotalNStrips=0;
    s32 TotalTris=0;
    s32 i;

    s32 C=0;
    for( s32 SID=0; SID<m_nKeepStrips; SID++ )
    {
        s32 L = m_pStrip[SID].Len;
        if( L>999 ) L = 999;
        LenStats[L]++;
        TotalTris += L;
        TotalNStrips++;
    }

    s32 TotalVerts=0;
    for( i=0; i<1000; i++ )
    {
        TotalVerts += LenStats[i]*2 + LenStats[i]*i;
    }

    s32 RunningTotalStrips=0;
    s32 RunningTotalVerts=0;
    for( i=0; i<1000; i++ )
    if( LenStats[i] > 0 )
    {
        s32 nVerts = LenStats[i]*2 + LenStats[i]*i;
        RunningTotalStrips += LenStats[i];
        RunningTotalVerts  += nVerts;
        x_fprintf(fp,"%3d] %4d V(%5d,%6.2f,%6.2f) S(%6.2f,%6.2f)\n",
            i,LenStats[i],
            nVerts,
            100.0f*nVerts/TotalVerts,
            100.0f*RunningTotalVerts/TotalVerts,
            100.0f*LenStats[i]/TotalNStrips,
            100.0f*RunningTotalStrips/TotalNStrips);
    }
    x_fprintf(fp,"Total tris   %d\n",TotalTris);
    x_fprintf(fp,"Total strips %d\n",RunningTotalStrips);
    x_fprintf(fp,"Total verts  %d\n",RunningTotalVerts);
    x_fprintf(fp,"VPT          %1.3f",(f32)RunningTotalVerts/(f32)TotalTris);

    x_fclose(fp);
}

//=========================================================================

void faststrip::SanityCheck(void)
{
    // Confirm number of strips in hash table matches nActive
    {
        s32 C=0;
        for( s32 i=m_MinHashIndex; i<=m_MaxHashIndex; i++ )
        if( m_pHash[i] != -1 )
        {
            s32 SID = m_pHash[i];
            while( SID != -1 )
            {
                C++;
                SID = m_pStrip[SID].HashNext;
            }
        }

        ASSERT( C == m_nActiveStrips );
    }
}

//=========================================================================
