#if !defined(AFX_EditorTriggerView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorTriggerView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorTriggerView.h : header file
//
#include "EditorPaletteDoc.h"
#include "..\Editor\PaletteView.h"
#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "..\WinControls\XTSortListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorTriggerView view

class CEditorTriggerView : public CPaletteView
{
protected:
	CEditorTriggerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorTriggerView)

// Attributes
public:
	CEditorPaletteDoc* GetDocument();

    CXTSortListCtrl m_TriggerLst;

    virtual void    OnTabActivate( BOOL bActivate );
    void            RefreshView     ( void );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorTriggerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	afx_msg void OnClickTriggerItem(NMHDR* pNMHDR, LRESULT* pResult);
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorTriggerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorTriggerView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnTrtbRefresh(); 
    afx_msg void OnUpdateTrtbRefresh(CCmdUI* pCmdUI); 
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    
};

inline CEditorPaletteDoc* CEditorTriggerView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorTriggerView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
