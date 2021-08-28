// Manipulator.cpp : implementation file
//

#include "stdafx.h"
#include "Manipulator.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//============================================================================
//  Helpers
//============================================================================

bool CManipulator::RaySphereIntersect( const vector3& Start, const vector3& Direction,
                                       const vector3& Center, f32 Radius,
                                       vector3& Intersection )
{
    f32   r2 = Radius*Radius;

    // Calculate line from start of ray to center of sphere
    vector3 StartToSphere = Center - Start;

    // Calculate length of projection of direction onto that line
    f32 d  = StartToSphere.Dot( Direction );
    f32 l2 = StartToSphere.Dot( StartToSphere );

    // Ray moving away from sphere & Start outside sphere then no intersection
    if( (d < 0) && (l2 > r2) )
    {
        return FALSE;
    }

    // Check for how far we are off the center
    f32 m2 = l2 - d*d;
    if( m2 > r2 )
        return FALSE;

    // Calculate intersection point
    f32 t;
    f32 q = x_sqrt( r2-m2 );
    if( l2 > r2 )
        t = d-q;
    else
        t = d+q;
    Intersection = Start + Direction*t;

    return TRUE;
}

//============================================================================

void CManipulator::ClosestPointsOnLines( const vector3& Start1, const vector3& Direction1,
                                         const vector3& Start2, const vector3& Direction2,
                                         vector3& Result1, vector3& Result2 )
{
    vector3 w = Start2 - Start1;
    f32     a = Direction1.Dot( Direction1 );
    f32     b = Direction1.Dot( Direction2 );
    f32     c = Direction2.Dot( Direction2 );
    f32     d = Direction1.Dot( w );
    f32     e = Direction2.Dot( w );
    f32 denom = a*c - b*b;
    f32     sc;
    f32     tc;

    // Parallel or failure?
    if( denom < 0.0000001f )
    {
        sc = 0.0f;
        tc = (b>c ? d/b : e/c);
    }
    else
    {
        sc = -(b*e - c*d) / denom;
        tc = -(a*e - b*d) / denom;
    }

    Result1 = Start1 + Direction1*sc;
    Result2 = Start2 + Direction2*tc;
}

//============================================================================

void CManipulator::ClosestPointsOnLineAndSegment( const vector3& Start1, const vector3& Direction1,
                                                  const vector3& Start2, const vector3& Direction2,
                                                  vector3& Result1, vector3& Result2 )
{
    vector3 w = Start2 - Start1;
    f32     a = Direction1.Dot( Direction1 );
    f32     b = Direction1.Dot( Direction2 );
    f32     c = Direction2.Dot( Direction2 );
    f32     d = Direction1.Dot( w );
    f32     e = Direction2.Dot( w );
    f32 denom = a*c - b*b;
    f32     sc;
    f32     tc;

    // Parallel or failure?
    if( denom < 0.0000001f )
    {
        sc = 0.0f;
        tc = (b>c ? d/b : e/c);
    }
    else
    {
        sc = -(b*e - c*d) / denom;
        tc = -(a*e - b*d) / denom;
    }

    if( tc < 0.0f )
        tc = 0.0f;

    Result1 = Start1 + Direction1*sc;
    Result2 = Start2 + Direction2*tc;
}

//============================================================================
// CManipulator

CManipulator::CManipulator()
{
    m_Size          = 1.0f;
    m_IsDragging    = FALSE;
    m_pDragView     = NULL;
}

CManipulator::~CManipulator()
{
}

//============================================================================

void CManipulator::SetSize( f32 Size )
{
    m_Size = Size;
}

//============================================================================
