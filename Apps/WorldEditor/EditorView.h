// EditorView.h : interface of the CEditorView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITORVIEW_H__88CF635A_7C9D_4BE1_BA51_B0188CA88DC0__INCLUDED_)
#define AFX_EDITORVIEW_H__88CF635A_7C9D_4BE1_BA51_B0188CA88DC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//=========================================================================

#include "..\Editor\View3d.h"
#include "..\Editor\Grid3d.hpp"
#include "..\Editor\Axis3d.hpp"

//=========================================================================

class CEditorDoc;
class CEditorFrame;
class view;

//=========================================================================
//  view_history
//=========================================================================

class view_history
{
public:
    // The structure of a history entry, currently saves view & Focus Point
    struct entry
    {
        view        m_View;
        vector3     m_FocusPos;

        entry( ) { };
        entry( const view& View, const vector3& FocusPos ) : m_View( View ), m_FocusPos( FocusPos ) { };
    };

protected:
    // Circular history buffer, xarray used for convenience it does not grow
    xarray<entry>   m_Buffer;
    s32             m_BufferSize;       // Number of elements in the buffer
    s32             m_BufferBase;       // Index of the base of the circular buffer
    s32             m_BufferLast;       // Index of the last element
    s32             m_BufferCursor;     // Index of cursor
    xbool           m_Empty;            // Empty history

protected:
    s32             BasedIndex          ( s32 Index );

public:
                    view_history        ( );
                   ~view_history        ( );

    void            SetSize             ( s32 Size );

    void            AppendView          ( const view& View, const vector3& FocusPos );

    xbool           CanUndo             ( void );
    xbool           CanRedo             ( void );
    entry           Undo                ( const view& CurrentView, const vector3& FocusPos );
    entry           Redo                ( const view& CurrentView, const vector3& FocusPos );
};

//=========================================================================

class view_favorite
{
public:
    xstring     m_Name;
    view        m_View;
    vector3     m_FocusPos;
};

//=========================================================================
//  CEditorView
//=========================================================================

class CEditorView : public CView3D
{
public:
    virtual void   SetViewDirty     ( void );  


protected: // create from serialization only
	CEditorView();
	DECLARE_DYNCREATE(CEditorView)
    void Initialize();
    xbool SetupView( const view* pPlayerView, view& PortalView, xbool FPV );
    void GetViewRatio( xbool bFirstPerson, s32& XRes, s32& YRes, f32& PixelScale );
    void RenderFPV();
    void RenderNormal();
    void Render( );
    CEditorFrame* GetFrame() { return m_pFrameEdit; }
	CEditorDoc* GetDocument();
    view& GetCameraView() { return m_View; }

    void CleanView();
    
    void CancelPlacementMode();
    void EnterPlacementMode();
    void CancelMovementMode();
    void EnterMovementMode();
    void CancelBlueprintMode();
    void EnterBlueprintMode();
    void CancelDecalPlacementMode();
    void EnterDecalPlacementMode();

    BOOL IsPlacementMode() { return m_enMode == WEV_PLACEMENT_MODE; }
    BOOL IsStandardMode() { return m_enMode == WEV_STANDARD_MODE; }
    BOOL IsMovementMode() { return m_enMode == WEV_MOVEMENT_MODE; }
    BOOL IsBlueprintMode() { return m_enMode == WEV_BLUEPRINT_MODE; }
    BOOL IsDecalPlacementMode() { return m_enMode == WEV_DECAL_PLACEMENT_MODE; }

    void MoveGridToSelectedObjects();

    void ForceMovementUndoRecording() { m_bObjectMoved = TRUE; }

    void SetFocusPos(const vector3& vPos) { m_PrevFocusPos = m_FocusPos; m_FocusPos = vPos; }
    vector3& GetFocusPos() { return m_FocusPos; }
    void FocusCamera();
    void FocusCameraWithUndo( const vector3& FocusPoint, f32 Distance = 2000.0f );
    void FocusCameraWithUndoAndNoRotation( const vector3& FocusPoint );
    void SetCameraWithUndo( const view& View, const vector3& FocusPoint );
    void SetHighlightVolume(const bbox& Volume, xcolor Color) { m_HighlightVolume = Volume; m_HighlightColor = Color; }
    void DoPortalWalk(BOOL bWalkPortals);

    void SetMessage(CString strMessage) { m_strLastMsg = strMessage; m_MsgTimer.Start(); }

    void SetGuidSelHightLight(guid GuidToHighlight) { m_GuidToHighLight = GuidToHighlight; }

    void UpdateWatchTimer(BOOL bActive);
    xbool CheckForCameraDrop ( void );

    // Camera History & Favorite functions
    void                    CameraUndo          ( void );
    void                    CameraRedo          ( void );
    xbool                   CameraCanUndo       ( void );
    xbool                   CameraCanRedo       ( void );
    xarray<view_favorite>&  GetViewFavorites    ( void ) { return m_ViewFavorites; }
    void                    CameraGotoPlayer    ( void );

// Attributes
public:

    enum EDIT_MODE {
        WEV_STANDARD_MODE,
        WEV_PLACEMENT_MODE,
        WEV_MOVEMENT_MODE,
        WEV_BLUEPRINT_MODE,
        WEV_DECAL_PLACEMENT_MODE,
    };

    BOOL            m_bShowSelectionBounds;
    BOOL            m_bShowGrid;
    BOOL            m_bShowSpacD;
    BOOL            m_bShowAxis;
    BOOL            m_bGridSnap;
    BOOL            m_bAddWithRaySnap;
    BOOL            m_bRenderPreview;
    BOOL            m_bDoPortalWalk;
    BOOL            m_bRenderIcons;

/////////////////////////////////////////////////////////////////////////////
protected:
    
    view_history            m_ViewHistory;
    xarray<view_favorite>   m_ViewFavorites;

    grid3d          m_Grid;
    grid3d          m_GridBaseline;
    axis3d          m_Axis;
    vector3         m_PrevFocusPos;
    vector3         m_FocusPos;
    CEditorFrame*   m_pFrameEdit;
    EDIT_MODE       m_enMode;
    BOOL            m_bDraggingObject;
    BOOL            m_bObjectMoved; //for undo purposes
    bbox            m_HighlightVolume;
    xcolor          m_HighlightColor;
    
    guid            m_GuidToHighLight;

    xtimer          m_WatchUpdateTimer;

    //warning message
    CString         m_strLastMsg;
    xtimer          m_MsgTimer;

    //banded selections
    BOOL            m_bDragSelect;
    BOOL            m_bDragUnSelect;
    BOOL            m_bAllLayers;
    CPoint          m_ptDragSelectBegin;
    CPoint          m_ptDragSelectEnd;
    plane           m_plDragSelect[5];
    bool            BBoxInDragBand( const bbox& BBox );
    void            CastRayFrom2D( s32 X, s32 Y, vector3& Start, vector3& End );
    vector3         Get3DPointFrom2D( s32 X, s32 Y );

    void    OnMoveObjects(CPoint point);
    void    OnRotateObjects(s32 rX, s32 rY, s32 rZ);


    // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnActivateFrame( UINT nState, CFrameWnd* pFrameWnd );
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif
    
// Generated message map functions
protected:
	//{{AFX_MSG(CEditorView)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnUndo();
	afx_msg void OnRedo();
	afx_msg void OnCutObjects();
	afx_msg void OnCopyObjects();
	afx_msg void OnPasteObjects();
	afx_msg void OnShowReport();
	afx_msg void OnUpdateUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCopyObjects(CCmdUI* pCmdUI);
	afx_msg void OnUpdateCutObjects(CCmdUI* pCmdUI);
	afx_msg void OnUpdatePasteObjects(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in EditorView.cpp
inline CEditorDoc* CEditorView::GetDocument()
   { return (CEditorDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORVIEW_H__88CF635A_7C9D_4BE1_BA51_B0188CA88DC0__INCLUDED_)
