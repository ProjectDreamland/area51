#if !defined(AFX_EVENTEDITORVIEW_H__11E39955_DCD3_4142_AF39_CAB462B13965__INCLUDED_)
#define AFX_EVENTEDITORVIEW_H__11E39955_DCD3_4142_AF39_CAB462B13965__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EventEditorView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEventEditorView view

class CEventEditorView : public CView
{
protected:
	CEventEditorView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEventEditorView)

// Attributes
public:
    
    CListCtrl           m_Resource;


// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventEditorView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEventEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEventEditorView)
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    void OnMenuItem();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTEDITORVIEW_H__11E39955_DCD3_4142_AF39_CAB462B13965__INCLUDED_)
