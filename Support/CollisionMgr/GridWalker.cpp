//==============================================================================
//
//  GridWalker.cpp
//
//==============================================================================

#include "GridWalker.hpp"

//==============================================================================
//==============================================================================
//==============================================================================
// GRID_WALKER
//==============================================================================
//==============================================================================
//==============================================================================

grid_walker::grid_walker( void )
{
    m_Cell[0] = 0;
    m_Cell[1] = 0;
    m_Cell[2] = 0;
    m_GridCellSize = 0;
    m_GridSpaceCurrentLen = 0;
}

//=========================================================================

void grid_walker::Setup( const vector3&    StartPos, 
                         const vector3&    EndPos, 
                         const vector3&    GridBasePos, 
                               f32         GridCellSize )
{
    ASSERT( GridCellSize > 0.0f );

    m_GridBase              = GridBasePos;
    m_GridCellSize          = GridCellSize;
    m_GridSpaceRayStart     = (StartPos-m_GridBase) / m_GridCellSize;
    m_GridSpaceRayEnd       = (EndPos-m_GridBase) / m_GridCellSize;
    m_GridSpaceRayDir       = m_GridSpaceRayEnd - m_GridSpaceRayStart;
    m_GridSpaceRayLen       = m_GridSpaceRayDir.Length();
    m_GridSpaceCurrentLen   = 0;
    m_GridSpaceSegmentStart = m_GridSpaceRayStart;
    m_GridSpaceSegmentEnd   = m_GridSpaceRayStart;

    // Normalize ray direction
    if( m_GridSpaceRayLen == 0.0f )
        m_GridSpaceRayDir = vector3(1,0,0);
    else
        m_GridSpaceRayDir /= m_GridSpaceRayLen;

    for( s32 i=0; i<3; i++ )
    {
        // Initialize which cell we are starting in
        m_Cell[i] = (((s32)(m_GridSpaceRayStart[i] + 16384))-16384);

        // Setup information for fast stepping
        step_info& S = m_StepInfo[i];

        if( m_GridSpaceRayDir[i] > 0.000001f )
        {
            S.CellIndexDelta                = 1;
            S.OneOverRayDirDelta            = 1.0f / m_GridSpaceRayDir[i];
            S.CellExitDeltaMinusRayStart    = 1.0f - m_GridSpaceRayStart[i];
            S.bDoStep                       = TRUE;
        }
        else
        if( m_GridSpaceRayDir[i] < -0.000001f )
        {
            S.CellIndexDelta                = -1;
            S.OneOverRayDirDelta            = 1.0f / m_GridSpaceRayDir[i];
            S.CellExitDeltaMinusRayStart    = 0.0f - m_GridSpaceRayStart[i];
            S.bDoStep                       = TRUE;
        }
        else
        {
            S.bDoStep = FALSE;
        }
    }
}

//=========================================================================

xbool grid_walker::Step( void )
{
    // Step to next cell.  Note that the computations are calculating the
    // distance to the exit point for the cell from scratch.  There is no
    // delta tracking except the integral cell[3] index.  This is to avoid
    // numerical precision issues due to the accumulation of deltas.
    // If computers could just do numbers well...

    // Init shortest distance to a new cell
    f32 NewCurrentLen = F32_MAX;
    s32 NewCell[3]={0,0,0};

    // Loop through the three dimensions and step into next cell
    for( s32 i=0; i<3; i++ )
    {
        if( m_StepInfo[i].bDoStep )
        {
            step_info& S = m_StepInfo[i];
            f32 ExitDist = (m_Cell[i]+S.CellExitDeltaMinusRayStart) * S.OneOverRayDirDelta;
            if( ExitDist < NewCurrentLen )
            {
                NewCurrentLen = ExitDist;
                NewCell[0] = m_Cell[0];
                NewCell[1] = m_Cell[1];
                NewCell[2] = m_Cell[2];
                NewCell[i] = m_Cell[i] + S.CellIndexDelta;
            }
        }
    }

    ASSERT( NewCurrentLen != F32_MAX );

    // Update new cell indices
    m_Cell[0] = NewCell[0];
    m_Cell[1] = NewCell[1];
    m_Cell[2] = NewCell[2];

    // Update new len along ray
    m_GridSpaceCurrentLen = NewCurrentLen;

    // Compute new segment
    m_GridSpaceSegmentStart = m_GridSpaceSegmentEnd;
    m_GridSpaceSegmentEnd   = m_GridSpaceRayStart + (m_GridSpaceRayDir * m_GridSpaceCurrentLen);

    // Return TRUE if we didn't step past the end of the ray
    return( m_GridSpaceCurrentLen <= m_GridSpaceRayLen );
}

//==============================================================================

