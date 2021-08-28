#if !defined(AFX_EditorSettingsView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorSettingsView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorSettingsView.h : header file
//
#include "..\Editor\PaletteView.h"

class CEditorPaletteDoc;

/////////////////////////////////////////////////////////////////////////////
// CEditorSettingsView view

class CEditorSettingsView : public CPaletteView
{
protected:
	CEditorSettingsView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorSettingsView)

	CTreeCtrl	m_mgrTree;

// Operations
public:
	CEditorPaletteDoc* GetDocument();
    virtual void    OnTabActivate( BOOL bActivate );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorSettingsView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorSettingsView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorSettingsView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeList(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
    void SortAllChildren( void );
    void AddItemToTree( CString strItem );
    HTREEITEM DoesChildExist(CString strCurrent, HTREEITEM hParent);
    CString ItemToPath(HTREEITEM hItem);
};

inline CEditorPaletteDoc* CEditorSettingsView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorSettingsView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
