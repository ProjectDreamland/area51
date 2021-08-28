#if !defined(AFX_VIEWMEMORYGRAPH_H__FDBC6B7F_082D_458F_94F7_5FF03E0F18E7__INCLUDED_)
#define AFX_VIEWMEMORYGRAPH_H__FDBC6B7F_082D_458F_94F7_5FF03E0F18E7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewMemoryGraph.h : header file
//

#include "ViewBase.h"
#include "Cxbmp.h"

/////////////////////////////////////////////////////////////////////////////
// CViewMemoryGraph view

class log_memory;

class CViewMemoryGraph : public CViewBase
{
    enum state
    {
        STATE_NULL,
        STATE_SELECT_FULL,
        STATE_SELECT_ZOOM,
        STATE_DRAG_ZOOM,
    };

protected:
	CViewMemoryGraph();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CViewMemoryGraph)

// Attributes
public:
    UINT        m_UpdateTimerID;

    CRect       m_rClient;
    CRect       m_rFull;
    CRect       m_rZoom;
    CRect       m_rSummary;
    CRect       m_rDetails;

    u32         m_FullMinAddress;
    u32         m_FullMaxAddress;

    double      m_ZoomCenterAddress;
    double      m_ZoomMinAddress;
    double      m_ZoomMaxAddress;
    bool        m_ZoomAddressSet;
    double      m_OldZoomMinAddress;
    double      m_OldZoomMaxAddress;

    state       m_State;
    CPoint      m_PointButtonDown;
    CPoint      m_PointButtonUp;
    u32         m_AddressLButtonDown;
    u32         m_AddressLButtonUp;

    xarray<log_memory*> m_MemBlocks;

    log_memory* m_pSelectedEntry;

    CFont       m_Font;

    Cxbmp       m_BitmapFull;
    Cxbmp       m_BitmapZoom;

	//CXTTipWindow m_TipWindow;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CViewMemoryGraph)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnUpdate(CView* pSender, LPARAM lHint, CObject* pHint);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CViewMemoryGraph();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

    double      PixelToAddress              ( CRect& r, double Min, double Max, s32 x );
    double      AddressToPixel              ( CRect& r, double Min, double Max, u32 Address, bool Clip = true );
    log_memory* AddressToLogEntry           ( u32 Address );
    void        MousePointsToZoomAddresses  ( void );

    void        ZoomIn                      ( void );
    void        ZoomOut                     ( void );

	// Generated message map functions
protected:
	//{{AFX_MSG(CViewMemoryGraph)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnMouseHover(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWMEMORYGRAPH_H__FDBC6B7F_082D_458F_94F7_5FF03E0F18E7__INCLUDED_)
