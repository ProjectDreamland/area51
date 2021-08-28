//==============================================================================
//
//  GridWalker.hpp
//
//==============================================================================

#ifndef GRIDWALKER_HPP
#define GRIDWALKER_HPP

//==============================================================================
//  INCLUDES
//==============================================================================

#include "x_math.hpp"

//==============================================================================
//  TYPES
//==============================================================================

class grid_walker
{
public:

    grid_walker::grid_walker( void );

    void        Setup       ( const vector3&    StartPos, 
                              const vector3&    EndPos, 
                              const vector3&    GridBasePos, 
                                    f32         GridCellSize );

    xbool       Step        ( void );

    inline void GetCell     ( s32& X, s32& Y, s32& Z );
    inline void GetSegment  ( vector3& SegmentStart, vector3& SegmentEnd );

private:

    struct step_info
    {
        f32     CellExitDeltaMinusRayStart;
        s32     CellIndexDelta;
        f32     OneOverRayDirDelta;
        xbool   bDoStep;
    };

    s32         m_Cell[3];
    f32         m_GridCellSize;
    vector3     m_GridBase;
    vector3     m_GridSpaceRayStart;
    vector3     m_GridSpaceRayEnd;
    vector3     m_GridSpaceRayDir;
    vector3     m_GridSpaceSegmentStart;
    vector3     m_GridSpaceSegmentEnd;
    f32         m_GridSpaceRayLen;
    f32         m_GridSpaceCurrentLen;
    step_info   m_StepInfo[3];
};

//==============================================================================

void grid_walker::GetCell( s32& X, s32& Y, s32& Z )
{
    X = m_Cell[0];
    Y = m_Cell[1];
    Z = m_Cell[2];
}

//==============================================================================

void grid_walker::GetSegment( vector3& SegmentStart, vector3& SegmentEnd )
{
    SegmentStart = m_GridBase + (m_GridSpaceSegmentStart * m_GridCellSize);
    SegmentEnd   = m_GridBase + (m_GridSpaceSegmentEnd   * m_GridCellSize);
}

//==============================================================================
#endif // POLYCACHE_HPP
//==============================================================================

