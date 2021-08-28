//===========================================================================
//
//  ViewportCamera.cpp
//
//  A camera class to be used for the viewports in 3D editing tools
//
//===========================================================================

//------------------------------------------------------------------------------+
//  INCLUDES
//------------------------------------------------------------------------------+

#include <stdafx.h>

#include "ViewportCamera.hpp"

//------------------------------------------------------------------------------+
//	Constructor / Destructor
//------------------------------------------------------------------------------+

ViewportCamera::ViewportCamera( void )
 :  m_bIsOrtho       ( false ),

    m_Pos       ( 0.0f, 100.0f, -100.0f ),
    m_Focus     ( 141.0f ),
    m_FOV       ( R_60 ),

    m_LastPos   ( 0.0f, 100.0f, -100.0f ),
    m_LastFocus ( 141.0f ),

    m_HistoryIndex  ( 0 ),
    m_UndoCount     ( 0 ),
    m_RedoCount     ( 0 )
{
    // Make sure CAM_HISTORY SIZE is a power of 2
    ASSERT( (CAM_HISTORY_SIZE & ( CAM_HISTORY_SIZE - 1 )) == 0x00 );

    m_Rot.Identity();
    m_LastRot.Identity();

    m_Rot.RotateX       ( DEG_TO_RAD(45) );
    m_LastRot.RotateX   ( DEG_TO_RAD(45) );
}

ViewportCamera::~ViewportCamera( void )
{

}

//------------------------------------------------------------------------------+
//	Functions - Public
//------------------------------------------------------------------------------+

//--------------------------------------------------------------+
//	GetPos
//--------------------------------------------------------------+

const vector3&      ViewportCamera::GetPosition( void ) const
{
    return m_Pos;
}

//--------------------------------------------------------------+
//	GetRotation
//--------------------------------------------------------------+

const quaternion&   ViewportCamera::GetRotation( void ) const
{
    return m_Rot;
}

//--------------------------------------------------------------+
//	GetTargetPos
//--------------------------------------------------------------+

const vector3   ViewportCamera::GetTargetPos( void ) const
{
    vector3 TargetPos   = m_Pos + ( m_Rot * vector3( 0.0f, 0.0f, m_Focus ) );
    return TargetPos;
}

//--------------------------------------------------------------+
//	GetUndoCount
//--------------------------------------------------------------+

const s32   ViewportCamera::GetUndoCount( void ) const
{
    return m_UndoCount;
}

//--------------------------------------------------------------+
//	GetRedoCount
//--------------------------------------------------------------+

const s32   ViewportCamera::GetRedoCount( void ) const
{
    return m_RedoCount;
}

//--------------------------------------------------------------+
//	Navigate_Begin
//--------------------------------------------------------------+

void    ViewportCamera::Navigate_Begin( void )
{
    m_LastPos   = m_Pos;
    m_LastRot   = m_Rot;
    m_LastFocus = m_Focus;
}

//--------------------------------------------------------------+
//	Navigate_End
//--------------------------------------------------------------+

void    ViewportCamera::Navigate_End( void )
{
    // Save the last settings in Undo Buffer
    // myVar & ( CAM_HISTORY_SIZE - 1 ) == myVar % CAM_HISTORY_SIZE
    s32     NewEntryIndex   = ( m_HistoryIndex + m_UndoCount ) & ( CAM_HISTORY_SIZE - 1 );

    m_CamHistory[ NewEntryIndex ].m_Pos     = m_LastPos;
    m_CamHistory[ NewEntryIndex ].m_Rot     = m_LastRot;
    m_CamHistory[ NewEntryIndex ].m_Focus   = m_LastFocus;

    // Update the counters
    m_UndoCount++;
    m_RedoCount = 0;


    // Update HistoryIndex & Clamp UndoCount...if we've wrapped all the way around the circular array
    if( m_UndoCount > 128 )
    {
        // myVar & ( CAM_HISTORY_SIZE - 1 ) == myVar % CAM_HISTORY_SIZE
        m_HistoryIndex  = ( m_HistoryIndex + 1 ) & ( CAM_HISTORY_SIZE - 1 );
        m_UndoCount     = 128;
    }

    // Sync the current settings
    m_LastPos   = m_Pos;
    m_LastRot   = m_Rot;
    m_LastFocus = m_Focus;
}

//--------------------------------------------------------------+
//	Navigate_Cancel
//--------------------------------------------------------------+

void    ViewportCamera::Navigate_Cancel( void )
{
    m_Pos   = m_LastPos;
    m_Rot   = m_LastRot;
    m_Focus = m_LastFocus;
}

//--------------------------------------------------------------+
//	Navigate_Undo
//--------------------------------------------------------------+

void    ViewportCamera::Navigate_Undo( void )
{
    if( m_UndoCount > 0 )
    {
        // myVar & ( CAM_HISTORY_SIZE - 1 ) == myVar % CAM_HISTORY_SIZE
        s32     UndoIndex   = ( m_HistoryIndex + m_UndoCount ) & ( CAM_HISTORY_SIZE - 1 );

        m_Pos   = m_CamHistory[ UndoIndex ].m_Pos;
        m_Rot   = m_CamHistory[ UndoIndex ].m_Rot;
        m_Focus = m_CamHistory[ UndoIndex ].m_Focus;

        // Set "Last" values equal to current values...just to be safe
        m_LastPos   = m_Pos;
        m_LastRot   = m_Rot;
        m_LastFocus = m_Focus;

        // Convert the Undo we just did into a Redo
        m_UndoCount--;
        m_RedoCount++;
    }
}

//--------------------------------------------------------------+
//	Navigate_Redo
//--------------------------------------------------------------+

void    ViewportCamera::Navigate_Redo( void )
{
    if( m_RedoCount > 0 )
    {
        // myVar & ( CAM_HISTORY_SIZE - 1 ) == myVar % CAM_HISTORY_SIZE
        s32     RedoIndex   = ( m_HistoryIndex + m_UndoCount ) & ( CAM_HISTORY_SIZE - 1 );

        m_Pos   = m_CamHistory[ RedoIndex ].m_Pos;
        m_Rot   = m_CamHistory[ RedoIndex ].m_Rot;
        m_Focus = m_CamHistory[ RedoIndex ].m_Focus;

        // Set "Last" values equal to current values...just to be safe
        m_LastPos   = m_Pos;
        m_LastRot   = m_Rot;
        m_LastFocus = m_Focus;

        // Convert the Redo we just did back into an Undo
        m_UndoCount++;
        m_RedoCount--;
    }
}

//--------------------------------------------------------------+
//	Pan - Move in Camera XY
//--------------------------------------------------------------+

void    ViewportCamera::Pan( f32 DeltaX, f32 DeltaY )
{
    // Calculate position relative to Navigate_Begin
    m_Pos   = m_LastPos + ( m_Rot * vector3( DeltaX, DeltaY, 0.0f ) * m_Focus );
}

//--------------------------------------------------------------+
//	Fly - Move in Camera XZ
//--------------------------------------------------------------+

void    ViewportCamera::Fly( f32 DeltaX, f32 DeltaY )
{
    // Calculate position relative to Navigate_Begin
    m_Pos   = m_LastPos + ( m_Rot * vector3( DeltaX, 0.0f, -DeltaY ) * m_Focus );
}

//--------------------------------------------------------------+
//	Look    - DeltaX rotates about World  Y
//          - DeltaY rotates about Camera X
//--------------------------------------------------------------+

void    ViewportCamera::Look( f32 DeltaX, f32 DeltaY )
{
    // Adjust speed/direction of normalized mouse DeltaX/DeltaY
    DeltaX  *= ( -2 * PI );
    DeltaY  *= (  2 * PI );

    // Set up our rotations
    quaternion  WorldRotY   ( vector3(0,1,0), DeltaX );
    quaternion  CamRotX     ( vector3(1,0,0), DeltaY );

    // Calculate rotation relative to Navigate_Begin
    m_Rot  = WorldRotY * m_LastRot * CamRotX;
}

//--------------------------------------------------------------+
//	Orbit   - DeltaX rotates about focus point in World  Y
//          - DeltaY rotates about focus point in Camera X
//--------------------------------------------------------------+

void    ViewportCamera::Orbit( f32 DeltaX, f32 DeltaY )
{
    // Adjust speed/direction of normalized mouse DeltaX/DeltaY
    DeltaX  *= ( -2 * PI );
    DeltaY  *= (  2 * PI );

    // Set up our rotations
    quaternion  WorldRotY   ( vector3(0,1,0), DeltaX );
    quaternion  CamRotX     ( vector3(1,0,0), DeltaY );

    // Calculate the final rotation
    quaternion  FinalRot    = WorldRotY * m_LastRot * CamRotX;

    // Calculate the position of the camera's "focus point"
    vector3     FocusPos    = m_LastPos + ( m_LastRot * vector3( 0.0f, 0.0f, m_Focus ) );

    // Calculate the camera's offset from the "focus point" after orbiting about it
    vector3     CamOffset   = FinalRot * vector3( 0.0f, 0.0f, -m_Focus );

    // Update the final Camera position & rotation
    m_Pos  = FocusPos + CamOffset;
    m_Rot  = FinalRot;
}

//--------------------------------------------------------------+
//	OrbitPoint  - DeltaX rotates about pivot point in World  Y
//              - DeltaY rotates about pivot point in Camera X
//--------------------------------------------------------------+

void    ViewportCamera::OrbitPoint( f32 DeltaX, f32 DeltaY, const vector3& PivotPoint )
{
    // Adjust speed/direction of normalized mouse DeltaX/DeltaY
    DeltaX  *= ( -2 * PI );
    DeltaY  *= (  2 * PI );

    // Set up our rotations
    quaternion  WorldRotY   ( vector3(0,1,0), DeltaX );
    quaternion  CamRotX     ( vector3(1,0,0), DeltaY );

    // Calculate the final rotation
    quaternion  FinalRot    = WorldRotY * m_LastRot * CamRotX;

    // Calculate the camera's offset from the PivotPoint
    vector3     Offset      = m_LastPos - PivotPoint;

    // Calculate the camera's offset from the PivotPoint after orbiting about it
    vector3     CamAxisX        = m_LastRot * vector3(1,0,0);
    quaternion  WorldCamRotX    ( CamAxisX, DeltaY );
    quaternion  FinalOrbit      = WorldRotY * WorldCamRotX;

    vector3     CamOffset       = FinalOrbit * Offset;

    // Update the final Camera position & rotation
    m_Pos  = PivotPoint + CamOffset;
    m_Rot  = FinalRot;
}

//--------------------------------------------------------------+
//	Zoom
//--------------------------------------------------------------+

void    ViewportCamera::Zoom( f32 DeltaY )
{
    // Calculate the ZoomAmount relative to the FocusLength at Navigate_Begin
    vector3 ZoomAmount  = m_Rot * vector3( 0.0f, 0.0f, -DeltaY ) * m_LastFocus;
    
    // Calculate Position & Focus relative to Navigate_Begin
    m_Pos  = m_LastPos + ZoomAmount;
    m_Focus   = m_LastFocus * ( 1.0f + DeltaY );
}

//--------------------------------------------------------------+
//	ZoomRegion
//--------------------------------------------------------------+

void    ViewportCamera::ZoomRegion( f32 NormLeft, f32 NormTop, f32 NormRight, f32 NormBottom )
{
    // Make sure they didn't give us values that are backwards(ie NormLeft > NormRight or NormTop > NormBottom )
    f32     Left, Right;
    f32     Top, Bottom;

    if( NormRight > NormLeft )
    {
        Left    = NormLeft;
        Right   = NormRight;
    }
    else
    {
        Left    = NormRight;
        Right   = NormLeft;
    }

    if( NormBottom > NormTop )
    {
        Top     = NormTop;
        Bottom  = NormBottom;
    }
    else
    {
        Top     = NormBottom;
        Bottom  = NormTop;
    }

    // Calculate the size of the Zoom Region
    f32     ZoomWidth   = (Right  - Left);
    f32     ZoomHeight  = (Bottom - Top );

    // Get the center of the Zoom Region
    f32     CenterX     = Left + ( ZoomWidth  * 0.5f );
    f32     CenterY     = Top  + ( ZoomHeight * 0.5f );

    // Calculate the dimensions of the Focus plane(Plane that's perpendicular to camera & FocusLength away)
    radian  HalfFOV     = m_FOV * 0.5f;
    f32     FocusWidth  = 2 * ( m_LastFocus * x_tan( HalfFOV ) );
    f32     FocusHeight = 2 * ( m_LastFocus * x_tan( HalfFOV ) );
    //f32     FocusWidth  = 2 * ( m_LastPos.Length() * x_tan( HalfFOV ) );
    //f32     FocusHeight = 2 * ( m_LastPos.Length() * x_tan( HalfFOV ) );

    // Calculate XY offset relative to the center point on the Focus plane in local(Camera) space
    // In doing so, we negate our calculations, because entropy's XY space is backwards from Windows' XY space
    vector3         LocalOffset(0,0,0);
    LocalOffset.GetX() = -( FocusWidth  * ( CenterX - 0.5f ) );
    LocalOffset.GetY() = -( FocusHeight * ( CenterY - 0.5f ) );

    // Calculate the change in FocusLength as an inverse ratio of the average zoom factor in XY
    // In other words, if you draw out a small zoom window it means we move in further on Z
    LocalOffset.GetZ() = m_LastFocus * ( 1.0f - (0.5f * (ZoomWidth + ZoomHeight)) );
    //LocalOffset.Z   = m_LastPos.Length() * ( 1.0f - ( 0.5f * (ZoomWidth + ZoomHeight)) );

    // Convert the camera offset to world space
    vector3         WorldOffset     = m_LastRot * LocalOffset;

    // Update Position & FocusLength relative to Navigate_Begin
    m_Pos      = m_LastPos + WorldOffset;
    //m_Focus   = m_LastFocus - LocalOffset.Z;
}

//--------------------------------------------------------------+
//	ZoomExtents
//--------------------------------------------------------------+

void    ViewportCamera::ZoomExtents( const bbox& World_Aligned_Bounds )
{
    // Make a BBox that is oriented relative to our camera
    matrix4     World2Cam   ( m_LastRot );
    bbox        Bounds      ( World_Aligned_Bounds );
    Bounds.Transform        ( World2Cam );

    //---------------------------------------------------------------------------------------------------------
    // Update the FocusLength...to do this, we:
    //---------------------------------------------------------------------------------------------------------
    // 1. Think of the relation between the 1/2 FOV angle, 1/2 BBox Width, and FocusLength as a right triangle
    // 2. Half of the BBox Width divided by tan(1/2 FOV) will give us the length we need to fit the BBox
    // 3. We then add half of the depth of the BBox since our Focus Target will be placed at the BBox center
    // 4. Finally, we multiply the whole thing by a fudge factor so we get a border of space around our objects
    //---------------------------------------------------------------------------------------------------------
    f32     HalfWidth       = Bounds.GetSize().GetX() * 0.5f;
    f32     HalfDepth       = Bounds.GetSize().GetZ() * 0.5f;
    radian  HalfFOV         = m_FOV * 0.5f;

    m_Focus   = 1.05f * ( ( HalfWidth / x_tan(HalfFOV) ) + HalfDepth );

    // Update the final Camera position
    m_Pos      = World_Aligned_Bounds.GetCenter() + ( m_LastRot * vector3( 0.0f, 0.0f, -m_Focus ) );
}

