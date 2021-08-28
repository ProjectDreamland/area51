#ifndef __MANIPSCALE_H
#define __MANIPSCALE_H

#include "Manipulator.h"

//============================================================================
// Manipulator.h
//============================================================================

class CManipScale : public CManipulator
{
public:
                    CManipScale();
    virtual        ~CManipScale();

    virtual void    Render              ( const view& View );

    virtual s32     HitTest             ( const view& View, f32 ScreenX, f32 ScreenY, vector3& Intersection );
    virtual xbool   ClearHighlight      ( void );
    virtual xbool   Highlight           ( const view& View, f32 ScreenX, f32 ScreenY );
    virtual xbool   BeginDrag           ( const view& View, f32 ScreenX, f32 ScreenY );
    virtual void    UpdateDrag          ( const view& View, f32 ScreenX, f32 ScreenY );
    virtual void    EndDrag             ( const view& View );

    void            SetPosition         ( const vector3& Pos );
    const vector3&  GetPosition         ( void );
    void            SetScale            ( const vector3& Scale );
    vector3         GetScale            ( void );

private:
    void            ComputeSize         ( const view& View );
    void            RenderPlane         ( const vector3& Origin, const vector3& Axis1, const vector3& Axis2, f32 Scale1, f32 Scale2, f32 Size, xbool Highlight );
    void            RenderArrow         ( const vector3& Origin, const vector3& Direction, f32 Size, xcolor Color, xbool Highlight );
    xbool           BeginDragAxis       ( const view& View, f32 ScreenX, f32 ScreenY, const vector3& Axis );
    xbool           BeginDragPlane      ( const view& View, f32 ScreenX, f32 ScreenY, const plane& Plane );
    void            UpdateDragAll       ( const view& View, f32 ScreenX, f32 ScreenY );
    void            UpdateDragAxis      ( const view& View, f32 ScreenX, f32 ScreenY, const vector3& Axis, f32& Scale );
    void            UpdateDragPlane     ( const view& View, f32 ScreenX, f32 ScreenY, const plane& Plane );

private:
    vector3         m_Position;             // Current position
    vector3         m_Scale;                // Current scale
    vector3         m_RenderScale;          // Current render scale

    vector3         m_DragInitialScale;     // Initial manipulator scale when dragging started
    vector3         m_DragScale;            // Manipulator scale while dragging
    vector3         m_DragInitialOffset;    // Offset from manipulator position when dragging started
    f32             m_DragInitialSize;      // Size when dragging started
    f32             m_DragInitialScreenX;   // Initial ScreenX when dragging started
    f32             m_DragInitialScreenY;   // Initial ScreenY when dragging started
    plane           m_DragInitialPlaneXY;   // Initial XY plane when dragging started
    plane           m_DragInitialPlaneXZ;   // Initial XZ plane when dragging started
    plane           m_DragInitialPlaneYZ;   // Initial YZ plane when dragging started

    s32             m_Highlight;            // Bitmask of handles highlighted
};

#endif //MANIPTRANSLATE_H
