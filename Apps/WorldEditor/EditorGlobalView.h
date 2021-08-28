#if !defined(AFX_EditorGlobalView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorGlobalView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorGlobalView.h : header file
//

class CEditorPaletteDoc;

#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "ValueGrid.h"
#include "..\Editor\PaletteView.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorGlobalView view

class CEditorGlobalView : public CPaletteView
{
public:

	CEditorPaletteDoc* GetDocument();
    void    RefreshView     ( void );

    virtual void    OnTabActivate( BOOL bActivate );

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorGlobalView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL


protected:

	CEditorGlobalView();           // protected constructor used by dynamic creation
	virtual ~CEditorGlobalView();
	DECLARE_DYNCREATE(CEditorGlobalView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(CEditorGlobalView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGvtbDeleteGlobal();
	afx_msg void OnUpdateGvtbDeleteGlobal(CCmdUI* pCmdUI);
	afx_msg void OnGvtbNewFolder();
	afx_msg void OnUpdateGvtbNewFolder(CCmdUI* pCmdUI);
	afx_msg void OnGvtbNewGlobal();
	afx_msg void OnUpdateGvtbNewGlobal(CCmdUI* pCmdUI);
	afx_msg void OnGvtbRefresh();
	afx_msg void OnUpdateGvtbRefresh(CCmdUI* pCmdUI);
	afx_msg LRESULT OnGridItemChange(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGuidSelect(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    void        AddGlobalToView     ( CString& strVar, var_mngr::global_types Type );        

private:

    CValueGrid                      m_tcGlobals;
	CImageList	                    m_imageList;
    xharray<var_mngr::global_def>   m_hGlobalArray;

protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};

inline CEditorPaletteDoc* CEditorGlobalView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorGlobalView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
