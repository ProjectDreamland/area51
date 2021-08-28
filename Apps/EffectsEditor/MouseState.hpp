#ifndef __MOUSESTATE_H
#define __MOUSESTATE_H


//============================================================================
// Maintain control of the current mouse state
class mousestate
{
public:
    enum state
    {
        SELECT,
        SELECT2,            // while mouse is clicked, drag rect
        CREATE,             // next click creates something
        CREATE_DRAG,        // next click creates then hold and drag size
        BEING_DRAGGED       // mouse is currently being dragged
    };

    
protected:

    state           m_State;
    plane           m_ColPlane;
    view*           m_pView;
    const CView*    m_pViewContext;
    vector3         m_LastPt;
    plane           m_SelVol[6];

    POINT           m_PickPt[2];

public:
    
    void                SetContext      ( const CView& View )           { m_pViewContext = &View; }
    const CView*        GetContext      ( void )                        { return m_pViewContext; }

    void                SetView         ( view* pView )                 { m_pView = pView; }
    void                SetupPlane      ( const plane& Plane )          { m_ColPlane = Plane; }
    void                GetLastPt       ( vector3& Pt )         const   { Pt = m_LastPt; }

    void                SetState        ( mousestate::state State )     { m_State = State; }    
    mousestate::state   GetState        ( void )                const   { return m_State;  }

    void                SetPickPoint    ( s32 Index, s32 X, s32 Y );
    void                GetPickPoint    ( s32 Index, POINT& Point );

    vector3             GetPoint        ( s32 X, s32 Y );
    vector3             GetSelPoint     ( s32 X, s32 Y );
    void                SetupSelVol     ( s32 X1, s32 Y1, s32 X2, s32 Y2 );
    void                SetupSelVol     ( void );
    
    bool                BBoxInSelVol    ( const bbox& BBox );
};



#endif