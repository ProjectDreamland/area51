// ManipTranslate.cpp : implementation file
//

#include "stdafx.h"
#include "ManipTranslate.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//============================================================================

#define ARROW_NSEGMENTS     16
#define ARROW_LENGTH        1.0f
#define ARROW_HEAD_RADIUS   0.05f
#define ARROW_HEAD_LENGTH   0.2f

#define HANDLE_SIZE         0.03f
#define HANDLE_SIZE_MULTI   0.3f

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
// CManipulator

CManipTranslate::CManipTranslate()
{
    m_Size          = 100.0f;
    m_RenderSize    = 1.0f;
    m_Highlight     = 0;
}

CManipTranslate::~CManipTranslate()
{
}

//============================================================================

void CManipTranslate::ComputeSize( const view& View )
{
    // Compute size to stay constant regardless of distance from view
    vector3 Origin = View.GetPosition();
    f32 Distance = (m_Position - Origin).Length();
    if( Distance < 1.0f )
        Distance = 1.0f;
    m_RenderSize = m_Size * Distance / 500.0f;

    // If dragging then use the size it was when dragging started
    if( m_IsDragging && (&View == m_pDragView) )
    {
        m_RenderSize = m_DragInitialSize;
    }
}

//============================================================================

void CManipTranslate::Render( const view& View )
{
    // Compute size
    ComputeSize( View );

    // Draw Axis Handles
    RenderArrow( m_Position, vector3(1,0,0), m_RenderSize, XCOLOR_RED  , (m_Highlight & HANDLE_X) );
    RenderArrow( m_Position, vector3(0,1,0), m_RenderSize, XCOLOR_GREEN, (m_Highlight & HANDLE_Y) );
    RenderArrow( m_Position, vector3(0,0,1), m_RenderSize, XCOLOR_BLUE , (m_Highlight & HANDLE_Z) );

    // Draw Plane Handles
    RenderPlane( m_Position, vector3(1,0,0), vector3(0,1,0), m_RenderSize, (m_Highlight == HANDLE_XY) );
    RenderPlane( m_Position, vector3(0,0,1), vector3(1,0,0), m_RenderSize, (m_Highlight == HANDLE_XZ) );
    RenderPlane( m_Position, vector3(0,1,0), vector3(0,0,1), m_RenderSize, (m_Highlight == HANDLE_YZ) );
}

//============================================================================

void CManipTranslate::RenderPlane( const vector3& Origin, const vector3& Axis1, const vector3& Axis2, f32 Size, xbool Highlight )
{
    xcolor  cOutline;
    xcolor  cFill;

    // Get vertices along the 2 axes
    vector3 v1 = Axis1*Size*HANDLE_SIZE_MULTI;
    vector3 v2 = Axis2*Size*HANDLE_SIZE_MULTI;

    // Determine colors based on highlighted or not
    if( Highlight )
    {
        cOutline    = xcolor(192,192,192);
        cFill       = xcolor(255,255,255,96);
    }
    else
    {
        cOutline    = xcolor(128,128,128);
        cFill       = xcolor(255,255,255,32);
    }

    // Reset transform
    draw_ClearL2W();

    // Draw alpha blended plane
    draw_Begin( DRAW_TRIANGLES, DRAW_USE_ALPHA|DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE );

    draw_Color( cFill );
    
    draw_Vertex( Origin );
    draw_Vertex( Origin+v1 );
    draw_Vertex( Origin+v1+v2 );

    draw_Vertex( Origin );
    draw_Vertex( Origin+v1+v2 );
    draw_Vertex( Origin+v1 );

    draw_Vertex( Origin+v1+v2 );
    draw_Vertex( Origin+v2 );
    draw_Vertex( Origin );

    draw_Vertex( Origin+v2 );
    draw_Vertex( Origin+v1+v2 );
    draw_Vertex( Origin );

    draw_End();

    // Draw lines for outline
    draw_Begin( DRAW_LINES, DRAW_NO_ZBUFFER|DRAW_NO_ZWRITE );
    draw_Color( cOutline );

    draw_Vertex( Origin+v1 );
    draw_Vertex( Origin+v1+v2 );

    draw_Vertex( Origin+v1+v2 );
    draw_Vertex( Origin+v2 );

    draw_End();
}

//============================================================================

void CManipTranslate::RenderArrow( const vector3& Origin, const vector3& Direction, f32 Size, xcolor Color, xbool Highlight )
{
    static vector3  s_Verts  [6*ARROW_NSEGMENTS];
    static xcolor   s_Colors [6*ARROW_NSEGMENTS];
    static s16      s_Indices[6*ARROW_NSEGMENTS];

    f32 Length      = Size * ARROW_LENGTH;
    f32 HeadRadius  = Size * ARROW_HEAD_RADIUS;
    f32 HeadLength  = Size * ARROW_HEAD_LENGTH;

    // Build matrix to translate arrow to Origin
    matrix4 m;
    m.Identity();
    m.SetTranslation( Origin );

    // Build rotation into matrix to align arrow with Direction
    vector3     ArrowDir(0,0,1);
    quaternion  q1;
    q1.Setup( ArrowDir, Direction );
    m.SetRotation( q1 );

    // Setup colors for arrow
    xcolor Color1 = Color;
    xcolor Color2((u8)(Color.R*.5f), (u8)(Color.G*.5f), (u8)(Color.B*.5f) );

    // Build vertices
    for( s32 i=0 ; i<ARROW_NSEGMENTS ; i++ )
    {
        f32 s1 = x_sin( (f32) i   /(ARROW_NSEGMENTS) * R_360 );
        f32 c1 = x_cos( (f32) i   /(ARROW_NSEGMENTS) * R_360 );
        f32 s2 = x_sin( (f32)(i+1)/(ARROW_NSEGMENTS) * R_360 );
        f32 c2 = x_cos( (f32)(i+1)/(ARROW_NSEGMENTS) * R_360 );

        s_Verts[0+i*6] = vector3( 0,0,Length );
        s_Verts[1+i*6] = vector3( s2*HeadRadius,c2*HeadRadius,Length-HeadLength );
        s_Verts[2+i*6] = vector3( s1*HeadRadius,c1*HeadRadius,Length-HeadLength );
        s_Verts[3+i*6] = vector3( 0,0,Length-HeadLength );
        s_Verts[4+i*6] = vector3( s1*HeadRadius,c1*HeadRadius,Length-HeadLength );
        s_Verts[5+i*6] = vector3( s2*HeadRadius,c2*HeadRadius,Length-HeadLength );

        s_Colors[0+i*6] = Color1;
        s_Colors[1+i*6] = Color1;
        s_Colors[2+i*6] = Color1;
        s_Colors[3+i*6] = Color2;
        s_Colors[4+i*6] = Color2;
        s_Colors[5+i*6] = Color2;
    }

    // Build indicies
    for( i=0 ; i<(ARROW_NSEGMENTS*6) ; i++ )
    {
        s_Indices[i] = i;
    }

    // Render it
    draw_Begin( DRAW_LINES, DRAW_NO_ZBUFFER );
    draw_SetL2W( m );
    draw_Color( Highlight ? XCOLOR_YELLOW : Color );
    draw_Vertex( vector3(0,0,0) );
    draw_Vertex( vector3(0,0,1) * (ARROW_LENGTH - ARROW_HEAD_LENGTH/2) * Size );
    draw_End();
    draw_Begin( DRAW_TRIANGLES, DRAW_NO_ZBUFFER );
    draw_SetL2W( m );
    draw_Colors ( &s_Colors [0], 6*ARROW_NSEGMENTS, sizeof(xcolor) );
    draw_Verts  ( &s_Verts  [0], 6*ARROW_NSEGMENTS, sizeof(vector3) );
    draw_Execute( &s_Indices[0], 6*ARROW_NSEGMENTS );
    draw_End();
}

//============================================================================

s32 CManipTranslate::HitTest( const view& View, f32 ScreenX, f32 ScreenY, vector3& Intersection )
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

                if( ((i!=2) && ((p.GetX() <= 0.0f) || (p.GetX() > HANDLE_SIZE_MULTI*m_RenderSize))) ||
                    ((i!=1) && ((p.GetY() <= 0.0f) || (p.GetY() > HANDLE_SIZE_MULTI*m_RenderSize))) ||
                    ((i!=0) && ((p.GetZ() <= 0.0f) || (p.GetZ() > HANDLE_SIZE_MULTI*m_RenderSize))) ||
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
                return Test[0].Handle;
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

xbool CManipTranslate::ClearHighlight( void )
{
    // Save old highlight value for dirty test, then reset highlight
    s32 OldHighlight = m_Highlight;
    m_Highlight = 0;

    // Dirty?
    return( m_Highlight != OldHighlight );
}

//============================================================================

xbool CManipTranslate::Highlight( const view& View, f32 ScreenX, f32 ScreenY )
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

xbool CManipTranslate::BeginDrag( const view& View, f32 ScreenX, f32 ScreenY )
{
    // Are any handles highlighted?
    if( m_Highlight & HANDLE_XYZ )
    {
        // Save off some initial drag parameters
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

xbool CManipTranslate::BeginDragAxis( const view& View, f32 ScreenX, f32 ScreenY, const vector3& Axis )
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
    m_DragInitialPosition   = m_Position;

    return TRUE;
}

//============================================================================

xbool CManipTranslate::BeginDragPlane( const view& View, f32 ScreenX, f32 ScreenY, const plane& Plane )
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
    m_DragInitialPosition   = m_Position;

    return TRUE;
}

//============================================================================

void CManipTranslate::UpdateDrag( const view& View, f32 ScreenX, f32 ScreenY )
{
    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // What are the drag constraints
    switch( m_Highlight )
    {
    case HANDLE_X:
        UpdateDragAxis( View, ScreenX, ScreenY, vector3(1,0,0) );
        break;
    case HANDLE_Y:
        UpdateDragAxis( View, ScreenX, ScreenY, vector3(0,1,0) );
        break;
    case HANDLE_Z:
        UpdateDragAxis( View, ScreenX, ScreenY, vector3(0,0,1) );
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
    }
}

//============================================================================

void CManipTranslate::UpdateDragAxis( const view& View, f32 ScreenX, f32 ScreenY, const vector3& Axis )
{
    // Generate ray from screen
    vector3 Start = View.GetPosition();
    vector3 Direction;
    Direction = View.RayFromScreen( ScreenX, ScreenY );

    // Find closest point on axis from ray
    vector3 p1;
    vector3 p2;
    ClosestPointsOnLines( Start, Direction, m_Position, Axis, p1, p2 );

    // Update position on selected axis
    if( m_Highlight & HANDLE_X )
        m_Position.GetX() = p2.GetX() - m_DragInitialOffset.GetX();
    if( m_Highlight & HANDLE_Y )
        m_Position.GetY() = p2.GetY() - m_DragInitialOffset.GetY();
    if( m_Highlight & HANDLE_Z )
        m_Position.GetZ() = p2.GetZ() - m_DragInitialOffset.GetZ();
}

//============================================================================

void CManipTranslate::UpdateDragPlane( const view& View, f32 ScreenX, f32 ScreenY, const plane& Plane )
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

    if( m_Highlight & HANDLE_X )
        m_Position.GetX() = p.GetX() - m_DragInitialOffset.GetX();
    if( m_Highlight & HANDLE_Y )
        m_Position.GetY() = p.GetY() - m_DragInitialOffset.GetY();
    if( m_Highlight & HANDLE_Z )
        m_Position.GetZ() = p.GetZ() - m_DragInitialOffset.GetZ();
}

//============================================================================

void CManipTranslate::EndDrag( const view& View )
{
    // Done dragging
    m_IsDragging = FALSE;

    // Compute size
    ComputeSize( View );
}

//============================================================================

void CManipTranslate::SetPosition( const vector3& Pos )
{
    m_Position = Pos;
}

//============================================================================

const vector3& CManipTranslate::GetPosition( void )
{
    return m_Position;
}

//============================================================================
