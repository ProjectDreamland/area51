// ManipScale.cpp : implementation file
//

#include "stdafx.h"
#include "ManipScale.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//============================================================================

#define BOX_SIZE            0.05f
#define ARROW_LENGTH        1.0f

#define HANDLE_SIZE         0.03f
#define HANDLE_SIZE_MULTI1  0.4f
#define HANDLE_SIZE_MULTI2  0.6f

#define DRAG_SCALE_F        1.0f            // Factor for how scale manipulator reacts to mouse moves 3D
#define DRAG_SCALE_F2       80.0f           // Factor for how scale manipulator reacts to mouse moves 2D

enum handles
{
    HANDLE_NONE = 0,
    HANDLE_X    = (1<<0),
    HANDLE_Y    = (1<<1),
    HANDLE_Z    = (1<<2),
    HANDLE_XY   = HANDLE_X|HANDLE_Y,
    HANDLE_XZ   = HANDLE_X|         HANDLE_Z,
    HANDLE_YZ   =          HANDLE_Y|HANDLE_Z,
    HANDLE_XYZ  = HANDLE_X|HANDLE_Y|HANDLE_Z,
};

//============================================================================
// CManipulatorScale

CManipScale::CManipScale()
{
    m_Size          = 1.0f;
    m_RenderSize    = 1.0f;
    m_Highlight     = 0;
    m_Scale.Set( 1.0f, 1.0f, 1.0f );
}

CManipScale::~CManipScale()
{
}

//============================================================================

void CManipScale::ComputeSize( const view& View )
{
    // If not dragging then compute size to stay constant regardless of distance from view
    if( !m_IsDragging )
    {
        vector3 Origin = View.GetPosition();
        f32 Distance = (m_Position - Origin).Length();
        if( Distance < 1.0f )
            Distance = 1.0f;
        m_RenderSize = m_Size * Distance / 500.0f;
        m_RenderScale.Set( 1.0f, 1.0f, 1.0f );
    }
    else
    {
        m_RenderScale = m_DragScale;
    }
}

//============================================================================

void CManipScale::Render( const view& View )
{
    // Compute size
    ComputeSize( View );

    // Draw Axis Handles
    RenderArrow( m_Position, vector3(1,0,0), m_RenderScale.GetX() * m_RenderSize, XCOLOR_RED  , (m_Highlight & HANDLE_X) );
    RenderArrow( m_Position, vector3(0,1,0), m_RenderScale.GetY() * m_RenderSize, XCOLOR_GREEN, (m_Highlight & HANDLE_Y) );
    RenderArrow( m_Position, vector3(0,0,1), m_RenderScale.GetZ() * m_RenderSize, XCOLOR_BLUE , (m_Highlight & HANDLE_Z) );

    // Draw Plane Handles
    RenderPlane( m_Position, vector3(1,0,0), vector3(0,1,0), m_RenderScale.GetX(), m_RenderScale.GetY(), m_RenderSize, HANDLE_XY );
    RenderPlane( m_Position, vector3(0,0,1), vector3(1,0,0), m_RenderScale.GetZ(), m_RenderScale.GetX(), m_RenderSize, HANDLE_XZ );
    RenderPlane( m_Position, vector3(0,1,0), vector3(0,0,1), m_RenderScale.GetY(), m_RenderScale.GetZ(), m_RenderSize, HANDLE_YZ );
}

//============================================================================

void CManipScale::RenderPlane( const vector3& Origin, const vector3& Axis1, const vector3& Axis2, f32 Scale1, f32 Scale2, f32 Size, s32 Handles )
{
    xcolor  cOutline1   = xcolor(128,128,128);
    xcolor  cOutline2   = xcolor(128,128,128);   
    xcolor  cFill1      = xcolor(255,255,255,32);
    xcolor  cFill2      = xcolor(255,255,255,32);

    // Get vertices along the 2 axes
    vector3 v1 = Axis1*Scale1*Size*HANDLE_SIZE_MULTI1;
    vector3 v2 = Axis2*Scale2*Size*HANDLE_SIZE_MULTI1;
    vector3 v3 = Axis1*Scale1*Size*HANDLE_SIZE_MULTI2;
    vector3 v4 = Axis2*Scale2*Size*HANDLE_SIZE_MULTI2;

    // Determine colors based on highlighted or not
    if( m_Highlight == HANDLE_XYZ )
    {
        cOutline1   = xcolor(192,192,192);
        cFill1      = xcolor(255,255,255,96);
    }
    else if( m_Highlight == Handles )
    {
        cOutline1   = xcolor(192,192,192);
        cOutline2   = xcolor(192,192,192);
        cFill2      = xcolor(255,255,255,96);
    }

    // Reset transform
    draw_ClearL2W();

    // Draw alpha blended plane
    draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE );

    draw_Color( cFill1 );
    draw_Vertex( Origin );
    draw_Vertex( Origin+v1 );
    draw_Vertex( Origin+v2 );
    draw_Vertex( Origin );
    draw_Vertex( Origin+v2 );
    draw_Vertex( Origin+v1 );

    draw_Color( cFill2 );
    draw_Vertex( Origin+v1 );
    draw_Vertex( Origin+v3 );
    draw_Vertex( Origin+v2 );
    draw_Vertex( Origin+v1 );
    draw_Vertex( Origin+v2 );
    draw_Vertex( Origin+v3 );

    draw_Vertex( Origin+v2 );
    draw_Vertex( Origin+v4 );
    draw_Vertex( Origin+v3 );
    draw_Vertex( Origin+v2 );
    draw_Vertex( Origin+v3 );
    draw_Vertex( Origin+v4 );

    draw_End();

    // Draw lines for outline
    draw_Begin( DRAW_LINES, DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE );
    draw_Color( cOutline1 );
    draw_Vertex( Origin+v1 );
    draw_Vertex( Origin+v2 );
    draw_Color( cOutline2 );
    draw_Vertex( Origin+v3 );
    draw_Vertex( Origin+v4 );
    draw_End();
}

//============================================================================

void CManipScale::RenderArrow( const vector3& Origin, const vector3& Direction, f32 Size, xcolor Color, xbool Highlight )
{
    f32 Length      = Size * ARROW_LENGTH;

    // Build matrix to translate arrow to Origin
    matrix4 m;
    m.Identity();
    m.SetTranslation( Origin );

    // Build rotation into matrix to align arrow with Direction
    vector3     ArrowDir(0,0,1);
    quaternion  q1;
    q1.Setup( ArrowDir, Direction );
    m.SetRotation( q1 );

    // Render it
    draw_Begin(DRAW_LINES );
    draw_SetL2W( m );
    draw_Color( Highlight ? XCOLOR_YELLOW : Color );
    draw_Vertex( vector3(0,0,0) );
    draw_Vertex( vector3(0,0,1) * ARROW_LENGTH * Size );
    draw_End();

    draw_Begin( DRAW_QUADS, DRAW_NO_ZBUFFER );
    f32 s = m_RenderSize*BOX_SIZE/2.0f;
    f32 o = ARROW_LENGTH * Size - s;
    draw_Color( Color );

    draw_Vertex( vector3(-s, s,o+s) );
    draw_Vertex( vector3( s, s,o+s) );
    draw_Vertex( vector3( s, s,o-s) );
    draw_Vertex( vector3(-s, s,o-s) );

    draw_Vertex( vector3( s,-s,o+s) );
    draw_Vertex( vector3(-s,-s,o+s) );
    draw_Vertex( vector3(-s,-s,o-s) );
    draw_Vertex( vector3( s,-s,o-s) );

    draw_Vertex( vector3( s, s,o+s) );
    draw_Vertex( vector3(-s, s,o+s) );
    draw_Vertex( vector3(-s,-s,o+s) );
    draw_Vertex( vector3( s,-s,o+s) );

    draw_Vertex( vector3( s,-s,o-s) );
    draw_Vertex( vector3(-s,-s,o-s) );
    draw_Vertex( vector3(-s, s,o-s) );
    draw_Vertex( vector3( s, s,o-s) );

    draw_Vertex( vector3( s,-s,o+s) );
    draw_Vertex( vector3( s,-s,o-s) );
    draw_Vertex( vector3( s, s,o-s) );
    draw_Vertex( vector3( s, s,o+s) );

    draw_Vertex( vector3(-s, s,o+s) );
    draw_Vertex( vector3(-s, s,o-s) );
    draw_Vertex( vector3(-s,-s,o-s) );
    draw_Vertex( vector3(-s,-s,o+s) );

    draw_End();
}

//============================================================================

s32 CManipScale::HitTest( const view& View, f32 ScreenX, f32 ScreenY, vector3& Intersection )
{
    // Compute size
    ComputeSize( View );

    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // Bounding sphere test for trivial rejection
    if( RaySphereIntersect( Start, Direction, m_Position, m_RenderSize, Intersection ) )
    {
        // Test for plane intersections
        {
            struct temp
            {
                s32    Handle;
                plane  Plane;
                f32    tRay;
            } Test[3];

            // Setup the planes for testing
            Test[0].Handle = HANDLE_X|HANDLE_Y;
            Test[1].Handle = HANDLE_X|HANDLE_Z;
            Test[2].Handle = HANDLE_Y|HANDLE_Z;
            Test[0].Plane.Setup( m_Position, vector3(0,0,1) );
            Test[1].Plane.Setup( m_Position, vector3(0,1,0) );
            Test[2].Plane.Setup( m_Position, vector3(1,0,0) );

            // Run tests
            for( s32 i=0 ; i<3 ; i++ )
            {
                Test[i].Plane.Intersect( Test[i].tRay, Start, Start + Direction );
                vector3 p = Start + Direction * Test[i].tRay - m_Position;

                // Check if intersection is within the triangular region
                f32 l = m_RenderSize * HANDLE_SIZE_MULTI2;
                plane Plane;
                Plane.Setup( vector3(1,0,0)*l, vector3(0,0,1)*l, vector3(0,1,0)*l );

                if( ((i!=2) && (p.GetX() < -0.1f)) ||
                    ((i!=1) && (p.GetY() < -0.1f)) ||
                    ((i!=0) && (p.GetZ() < -0.1f)) ||
//                    Plane.InBack( p ) ||
                    Test[i].tRay < 0.0f )
                {
                    Test[i].tRay = F32_MAX;
                }
            }

            // Sort plane intersections
            if( Test[2].tRay < Test[1].tRay )
                Test[1] = Test[2];
            if( Test[1].tRay < Test[0].tRay )
                Test[0] = Test[1];

            // Return closest intersection if we hit anything
            if( Test[0].tRay != F32_MAX )
            {
                Intersection = Start + Direction*Test[0].tRay;

                // Check if intersection is in the inner sector to drag all planes, else drag 1 plane
                f32 l = m_RenderSize * HANDLE_SIZE_MULTI1;
                plane Plane;
                Plane.Setup( m_Position + vector3(1,0,0)*l, m_Position + vector3(0,0,1)*l, m_Position + vector3(0,1,0)*l );
                if( Plane.InFront( Intersection ) )
                    return HANDLE_XYZ;
                else
                {
                    f32 l = m_RenderSize * HANDLE_SIZE_MULTI2;
                    Plane.Setup( m_Position + vector3(1,0,0)*l, m_Position + vector3(0,0,1)*l, m_Position + vector3(0,1,0)*l );
                    if( Plane.InFront( Intersection ) )
                        return Test[0].Handle;
                }
            }
        }

        // Test for axis intersections
        {
            struct temp
            {
                s32     Handle;
                vector3 Axis;
                f32     tRay;
            } Test[3];

            // Setup tests
            Test[0].Handle = HANDLE_X;
            Test[1].Handle = HANDLE_Y;
            Test[2].Handle = HANDLE_Z;
            Test[0].Axis.Set(1,0,0);
            Test[1].Axis.Set(0,1,0);
            Test[2].Axis.Set(0,0,1);

            // Run tests
            for( s32 i=0 ; i<3 ; i++ )
            {
                vector3 p1;
                vector3 p2;
                ClosestPointsOnLineAndSegment( Start, Direction, m_Position, Test[i].Axis, p1, p2 );
                f32 tAxis = Test[i].Axis.Dot( p2-m_Position );
                if( (tAxis >= 0.0f) && (tAxis < m_RenderSize) && ((p2-p1).Length() < (m_RenderSize * HANDLE_SIZE)) )
                    Test[i].tRay = (p2-Start).Length();
                else
                    Test[i].tRay = F32_MAX;
            }
            
            // Sort intersections
            if( Test[2].tRay < Test[1].tRay )
                Test[1] = Test[2];
            if( Test[1].tRay < Test[0].tRay )
                Test[0] = Test[1];

            // Return closest intersection if we hit anything
            if( Test[0].tRay != F32_MAX )
            {
                Intersection = Start + Direction * Test[0].tRay;
                return Test[0].Handle;
            }
        }
    }

    // No handle hit
    return HANDLE_NONE;
}

//============================================================================

xbool CManipScale::ClearHighlight( void )
{
    // Save old highlight value for dirty test, then reset highlight
    s32 OldHighlight = m_Highlight;
    m_Highlight = 0;

    // Dirty?
    return( m_Highlight != OldHighlight );
}

//============================================================================

xbool CManipScale::Highlight( const view& View, f32 ScreenX, f32 ScreenY )
{
    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // Save old highlight value for dirty test
    s32 OldHighlight = m_Highlight;

    // Determine highlight from hit test
    vector3 Intersection;
    m_Highlight = HitTest( View, ScreenX, ScreenY, Intersection );

    // Dirty?
    return( m_Highlight != OldHighlight );
}

//============================================================================

xbool CManipScale::BeginDrag( const view& View, f32 ScreenX, f32 ScreenY )
{
    // Are any handles highlighted?
    if( m_Highlight & HANDLE_XYZ )
    {
        // Save off some initial drag parameters
        m_DragInitialScale   = m_Scale;
        m_DragScale.Set( 1.0f, 1.0f, 1.0f );
        m_DragInitialScreenX = ScreenX;
        m_DragInitialScreenY = ScreenY;
        m_DragInitialPlaneXY.Setup( m_Position, vector3(0,0,1) );
        m_DragInitialPlaneXZ.Setup( m_Position, vector3(0,1,0) );
        m_DragInitialPlaneYZ.Setup( m_Position, vector3(1,0,0) );

        // What are the drag constraints
        switch( m_Highlight )
        {
        case HANDLE_X:
            m_IsDragging = BeginDragAxis( View, ScreenX, ScreenY, vector3(1,0,0) );
            break;
        case HANDLE_Y:
            m_IsDragging = BeginDragAxis( View, ScreenX, ScreenY, vector3(0,1,0) );
            break;
        case HANDLE_Z:
            m_IsDragging = BeginDragAxis( View, ScreenX, ScreenY, vector3(0,0,1) );
            break;
        case HANDLE_XY:
            m_IsDragging = BeginDragPlane( View, ScreenX, ScreenY, m_DragInitialPlaneXY );
            break;
        case HANDLE_XZ:
            m_IsDragging = BeginDragPlane( View, ScreenX, ScreenY, m_DragInitialPlaneXZ );
            break;
        case HANDLE_YZ:
            m_IsDragging = BeginDragPlane( View, ScreenX, ScreenY, m_DragInitialPlaneYZ );
            break;
        case HANDLE_XYZ:
            m_IsDragging = TRUE;
            break;
        }

        // Set dragging view?
        if( m_IsDragging )
        {
            m_DragInitialSize = m_RenderSize;
            m_pDragView = &View;
        }
    }

    // Return dragging state
    return m_IsDragging;
}

//============================================================================

xbool CManipScale::BeginDragAxis( const view& View, f32 ScreenX, f32 ScreenY, const vector3& Axis )
{
    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // Find closest point on axis from ray
    vector3 p1;
    vector3 p2;
    ClosestPointsOnLines( Start, Direction, m_Position, Axis, p1, p2 );
    m_DragInitialOffset     = p2 - m_Position;

    return TRUE;
}

//============================================================================

xbool CManipScale::BeginDragPlane( const view& View, f32 ScreenX, f32 ScreenY, const plane& Plane )
{
    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // Find plane / ray intersection point
    f32     t;
    vector3 p;
    Plane.Intersect( t, Start, Start+Direction );
    p = Start + Direction*t;
    m_DragInitialOffset     = p - m_Position;

    return TRUE;
}

//============================================================================

void CManipScale::UpdateDrag( const view& View, f32 ScreenX, f32 ScreenY )
{
    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // What are the drag constraints
    switch( m_Highlight )
    {
    case HANDLE_X:
        UpdateDragAxis( View, ScreenX, ScreenY, vector3(1,0,0), m_DragScale.GetX() );
        break;
    case HANDLE_Y:
        UpdateDragAxis( View, ScreenX, ScreenY, vector3(0,1,0), m_DragScale.GetY() );
        break;
    case HANDLE_Z:
        UpdateDragAxis( View, ScreenX, ScreenY, vector3(0,0,1), m_DragScale.GetZ() );
        break;
    case HANDLE_XY:
        UpdateDragPlane( View, ScreenX, ScreenY, m_DragInitialPlaneXY );
        break;
    case HANDLE_XZ:
        UpdateDragPlane( View, ScreenX, ScreenY, m_DragInitialPlaneXZ );
        break;
    case HANDLE_YZ:
        UpdateDragPlane( View, ScreenX, ScreenY, m_DragInitialPlaneYZ );
        break;
    case HANDLE_XYZ:
        UpdateDragAll( View, ScreenX, ScreenY );
        break;
    }
}

//============================================================================

void CManipScale::UpdateDragAll( const view& View, f32 ScreenX, f32 ScreenY )
{
    f32 dy = m_DragInitialScreenY - ScreenY;

    if( dy >= 0 )
    {
        f32 Scale = (dy+DRAG_SCALE_F2) / DRAG_SCALE_F2;
        m_DragScale.Set( Scale, Scale, Scale );
    }
    else
    {
        f32 Scale = 1.0f / ((DRAG_SCALE_F2-dy) / DRAG_SCALE_F2 );
        m_DragScale.Set( Scale, Scale, Scale );
    }
}

//============================================================================

void CManipScale::UpdateDragAxis( const view& View, f32 ScreenX, f32 ScreenY, const vector3& Axis, f32& Scale )
{
    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // Find closest point on axis from ray
    vector3 p1;
    vector3 p2;
    ClosestPointsOnLines( Start, Direction, m_Position, Axis, p1, p2 );

    // Compute the scale
    p2 -= m_Position;
    f32 t1 = Axis.Dot( m_DragInitialOffset );
    f32 t2 = Axis.Dot( p2 );
    f32 d  = t2 - t1;
    f32 f = DRAG_SCALE_F * m_RenderSize;
    if( d >= 0 )
    {
        Scale = (d+f) / f;
    }
    else
    {
        Scale = 1.0f / ((f-d) / f );
    }
}

//============================================================================

void CManipScale::UpdateDragPlane( const view& View, f32 ScreenX, f32 ScreenY, const plane& Plane )
{
    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // Find plane / ray intersection point
    f32     t;
    if( !Plane.Intersect( t, Start, Start+Direction ) || (t < 0.0f) )
    {
        m_DragScale = m_DragInitialScale;
        return;
    }

    vector3 p = Start + Direction*t - m_Position;

    f32 t1;
    f32 t2;
    f32 f = DRAG_SCALE_F * m_RenderSize;

    // Update scale on selected axes
    if( m_Highlight & HANDLE_X )
    {
        t1 = vector3(1,0,0).Dot( m_DragInitialOffset );
        t2 = vector3(1,0,0).Dot( p );
        f32 d  = t2 - t1;
        if( d >= 0 )
        {
            m_DragScale.GetX() = (d+f) / f;
        }
        else
        {
            m_DragScale.GetX() = 1.0f / ((f-d) / f );
        }
    }

    if( m_Highlight & HANDLE_Y )
    {
        t1 = vector3(0,1,0).Dot( m_DragInitialOffset );
        t2 = vector3(0,1,0).Dot( p );
        f32 d  = t2 - t1;
        if( d >= 0 )
        {
            m_DragScale.GetY() = (d+f) / f;
        }
        else
        {
            m_DragScale.GetY() = 1.0f / ((f-d) / f );
        }
    }
    if( m_Highlight & HANDLE_Z )
    {
        t1 = vector3(0,0,1).Dot( m_DragInitialOffset );
        t2 = vector3(0,0,1).Dot( p );
        f32 d  = t2 - t1;
        if( d >= 0 )
        {
            m_DragScale.GetZ() = (d+f) / f;
        }
        else
        {
            m_DragScale.GetZ() = 1.0f / ((f-d) / f );
        }
    }
}

//============================================================================

void CManipScale::EndDrag( const view& View )
{
    // Done dragging
    m_IsDragging = FALSE;

    // Compute size
    ComputeSize( View );
}

//============================================================================

void CManipScale::SetPosition( const vector3& Pos )
{
    m_Position = Pos;
}

//============================================================================

const vector3& CManipScale::GetPosition( void )
{
    return m_Position;
}

//============================================================================

void CManipScale::SetScale( const vector3& Scale )
{
    m_Scale = Scale;
}

//============================================================================

vector3 CManipScale::GetScale( void )
{
    return m_Scale;
}

//============================================================================
