#include <stdafx.h>

#include "mousestate.hpp"


//==============================================================================
// Get a 3D point given a 2D screen position
vector3 mousestate::GetPoint( s32 X, s32 Y )
{
    vector3 ViewPos = m_pView->GetPosition();
    vector3 Ray;
    
    Ray = m_pView->RayFromScreen( (f32)X, (f32)Y );

    f32 T;
    m_ColPlane.Intersect( T, ViewPos, ViewPos + Ray );
    m_LastPt = ( ViewPos + ( Ray * T ) );

    return m_LastPt;
}


//==============================================================================
// Get a 3D point given a 2D screen position
vector3 mousestate::GetSelPoint( s32 X, s32 Y )
{
    vector3 ViewPos = m_pView->GetPosition();
    vector3 Ray;
    
    Ray = m_pView->RayFromScreen( (f32)X, (f32)Y );

    return ViewPos + Ray ;
}


//==============================================================================
// Setup a selection volume from 4 on-screen points
void mousestate::SetupSelVol( s32 X1, s32 Y1, s32 X2, s32 Y2 )
{
    vector3 Endpoint[4];

    // get the 4 ends
    Endpoint[0] = GetSelPoint( MIN(X1,X2), MIN(Y1,Y2) );
    Endpoint[1] = GetSelPoint( MAX(X1,X2), MIN(Y1,Y2) );
    Endpoint[2] = GetSelPoint( MAX(X1,X2), MAX(Y1,Y2) );
    Endpoint[3] = GetSelPoint( MIN(X1,X2), MAX(Y1,Y2) );

    // get the camera position
    vector3 Viewpoint = m_pView->GetPosition();

    // now setup the frustum
    m_SelVol[0].Setup( Endpoint[3], Endpoint[0], Viewpoint   );    // Left
    m_SelVol[1].Setup( Endpoint[0], Endpoint[1], Viewpoint   );    // Top   
    m_SelVol[2].Setup( Endpoint[2], Viewpoint,   Endpoint[1] );    // Right
    m_SelVol[3].Setup( Endpoint[3], Viewpoint,   Endpoint[2] );    // Bottom
    
    m_SelVol[4].Setup( Viewpoint, m_pView->GetViewZ() ) ;        // Near
}

//==============================================================================
// Setup a selection volume from the PickPts
void mousestate::SetupSelVol( void )
{
    if ( (x_abs(m_PickPt[1].x - m_PickPt[0].x) < 3) &&
         (x_abs(m_PickPt[1].y - m_PickPt[0].y) < 3) )
    {
        m_PickPt[0].x -= 5;
        m_PickPt[0].y -= 5;
        m_PickPt[1].x += 5;
        m_PickPt[1].y += 5;
    }

    SetupSelVol( m_PickPt[0].x, m_PickPt[0].y,
                 m_PickPt[1].x, m_PickPt[1].y );
}

//==============================================================================
// Determine if a BBox is in the selection volume
bool mousestate::BBoxInSelVol( const bbox& BBox )
{

    // Loop through planes looking for a trivial reject.
    s32 InFrontOf = 0;

    for( s32 i=0; i<5; i++ )
    {
        // Compute max dist along normal
        if ( m_SelVol[i].InFront( vector3(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Min.GetZ()) ) ||
             m_SelVol[i].InFront( vector3(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Min.GetZ()) ) ||
             m_SelVol[i].InFront( vector3(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Min.GetZ()) ) ||
             m_SelVol[i].InFront( vector3(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Min.GetZ()) ) ||
             m_SelVol[i].InFront( vector3(BBox.Min.GetX(), BBox.Min.GetY(), BBox.Max.GetZ()) ) ||
             m_SelVol[i].InFront( vector3(BBox.Max.GetX(), BBox.Min.GetY(), BBox.Max.GetZ()) ) ||
             m_SelVol[i].InFront( vector3(BBox.Max.GetX(), BBox.Max.GetY(), BBox.Max.GetZ()) ) ||
             m_SelVol[i].InFront( vector3(BBox.Min.GetX(), BBox.Max.GetY(), BBox.Max.GetZ()) ) )
        {
            InFrontOf++;
        }
    }

    if ( InFrontOf == 5 )
        return true;
    else
        return false;
} 

//==============================================================================
// Record a pick point
void mousestate::SetPickPoint( s32 Index, s32 X, s32 Y )
{
    ASSERT( Index < 2 );
    ASSERT( Index >= 0 );
    m_PickPt[Index].x = X;
    m_PickPt[Index].y = Y;

}

//==============================================================================
// Retrieve a pick point
void mousestate::GetPickPoint( s32 Index, POINT& Point )
{
    ASSERT( Index < 2 );
    ASSERT( Index >= 0 );
    Point.x = m_PickPt[Index].x;
    Point.y = m_PickPt[Index].y;
}