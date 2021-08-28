#ifndef __MANIPULATOR_H
#define __MANIPULATOR_H

//============================================================================
// Manipulator.h
//============================================================================

class CManipulator
{
public:
                    CManipulator();
    virtual        ~CManipulator();

    virtual void    Render              ( const view& View ) = 0;

    virtual s32     HitTest             ( const view& View, f32 ScreenX, f32 ScreenY, vector3& Intersection ) = 0;
    virtual xbool   ClearHighlight      ( void ) = 0;
    virtual xbool   Highlight           ( const view& View, f32 ScreenX, f32 ScreenY ) = 0;
    virtual xbool   BeginDrag           ( const view& View, f32 ScreenX, f32 ScreenY ) = 0;
    virtual void    UpdateDrag          ( const view& View, f32 ScreenX, f32 ScreenY ) = 0;
    virtual void    EndDrag             ( const view& View ) = 0;

    void            SetSize             ( f32 Size );

protected:
    bool RaySphereIntersect             ( const vector3& Start, const vector3& Direction,
                                          const vector3& Center, f32 Radius,
                                          vector3& Intersection );

    void ClosestPointsOnLines           ( const vector3& Start1, const vector3& Direction1,
                                          const vector3& Start2, const vector3& Direction2,
                                          vector3& Result1, vector3& Result2 );

    void ClosestPointsOnLineAndSegment  ( const vector3& Start1, const vector3& Direction1,
                                          const vector3& Start2, const vector3& Direction2,
                                          vector3& Result1, vector3& Result2 );

protected:
    f32             m_Size;                 // Size of the manipulator
    f32             m_RenderSize;           // Size of the manipulator for rendering
    xbool           m_IsDragging;           // Is dragging
    const view*     m_pDragView;            // View that is dragging
};

#endif //MANIPULATOR_H
