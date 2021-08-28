#if !defined(AFX_TEXTEDITORVIEW_H__9303C29E_F473_42C9_9F54_5B51D9477835__INCLUDED_)
#define AFX_TEXTEDITORVIEW_H__9303C29E_F473_42C9_9F54_5B51D9477835__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TextEditorView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// TextEditorView view

class TextEditorView : public CView
{
protected:
	TextEditorView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(TextEditorView)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(TextEditorView)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~TextEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(TextEditorView)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTEDITORVIEW_H__9303C29E_F473_42C9_9F54_5B51D9477835__INCLUDED_)
