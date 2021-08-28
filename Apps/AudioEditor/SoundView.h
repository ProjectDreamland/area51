// SoundView.h : interface of the CSoundView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_SOUNDVIEW_H__AEF2D34E_1125_4631_A802_82FA088B05F4__INCLUDED_)
#define AFX_SOUNDVIEW_H__AEF2D34E_1125_4631_A802_82FA088B05F4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//==============================================================================
// EXTERNAL CLASSES
//==============================================================================
class CSoundDoc;

//==============================================================================
// SOUND VIEW
//==============================================================================
class CSoundView : public CView
{

//==============================================================================

protected: 
	DECLARE_DYNCREATE(CSoundView)
    s32             m_SampleOffsetMouseUp;
    CPoint          m_StartOffset;
    s32             m_SoundUnits;    

    CFont           m_Font;
    s32             m_ZoomFactor;
    s32             m_SelectedSample;
    xbool           m_MoveSample;
    CRect           m_ClientRect;
    s32             m_PreferredZoom;

//==============================================================================
public:
	CSoundDoc*          GetDocument             ( void );
    int                 GetClientWidth          ( void );
    int                 GetClientHeight         ( void );

    void                DrawRuler               ( CDC* pDC, CRect& r );
    void                DrawMarker              ( CDC* pDC, CRect& r );
    void                RefreshSampleView       ( void );
    xbool               UpdateElementOffset     (s32 SampleOffset, s32 SampleIndex);
    xbool               CheckElementOffset      (s32 SampleOffset, s32 SampleIndex);

	                    CSoundView();
	virtual             ~CSoundView();
    void                CalibrateScrollbar      ( void );

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


//==============================================================================
// MFC
//==============================================================================
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSoundView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void OnInitialUpdate();
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
    virtual void OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint );

	//}}AFX_VIRTUAL

// Generated message map functions
protected:
	//{{AFX_MSG(CSoundView)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
    afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSoundZoomIn();
	afx_msg void OnSoundZoomInFull();
	afx_msg void OnSoundZoomOut();
	afx_msg void OnSoundZoomOutFull();
	afx_msg void OnSoundZoomSelection();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in SoundView.cpp
inline CSoundDoc* CSoundView::GetDocument()
   { return (CSoundDoc*)m_pDocument; }
#endif

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_SOUNDVIEW_H__AEF2D34E_1125_4631_A802_82FA088B05F4__INCLUDED_)
