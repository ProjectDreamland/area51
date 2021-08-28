// HAISView.h : interface of the CHAISView class
//


#pragma once


class CHAISView : public CView
{
    enum mode
    {
        MODE_SELECT,
        MODE_ADD_COVER,
        MODE_ADD_RED,
        MODE_ADD_BLUE
    };

protected: // create from serialization only
	CHAISView();
	DECLARE_DYNCREATE(CHAISView)

// Attributes
public:
	CHAISDoc* GetDocument() const;

// Operations
public:

// Overrides
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CHAISView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    mode    m_Mode;             // Mode of this view - SELECT, ADD_COVER, ADD_RED, ADD_BLUE

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnObjectSelect();
    afx_msg void OnUpdateObjectSelect(CCmdUI *pCmdUI);
    afx_msg void OnObjectCover();
    afx_msg void OnUpdateObjectCover(CCmdUI *pCmdUI);
    afx_msg void OnObjectRed();
    afx_msg void OnUpdateObjectRed(CCmdUI *pCmdUI);
    afx_msg void OnObjectBlue();
    afx_msg void OnUpdateObjectBlue(CCmdUI *pCmdUI);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

#ifndef _DEBUG  // debug version in HAISView.cpp
inline CHAISDoc* CHAISView::GetDocument() const
   { return reinterpret_cast<CHAISDoc*>(m_pDocument); }
#endif

