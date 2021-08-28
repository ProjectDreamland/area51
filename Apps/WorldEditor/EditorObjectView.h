#if !defined(AFX_EditorObjectView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorObjectView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorObjectView.h : header file
//
#include "ResourcePreview.h"
#include "..\Editor\PaletteView.h"

class CEditorPaletteDoc;

/////////////////////////////////////////////////////////////////////////////
// CEditorObjectView view

class CEditorObjectView : public CPaletteView
{
protected:
	CEditorObjectView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorObjectView)

    void LoadList();
    BOOL CanAdd();

    CResourcePreview    m_wndPreview;
	CTreeCtrl	        m_rscTree;
    CStatic             m_stPlaceholder;

// Operations
public:
	CEditorPaletteDoc* GetDocument();
    virtual void    OnTabActivate( BOOL bActivate );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorObjectView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorObjectView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorObjectView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnOvtbAddPlaysurface();
    afx_msg void OnOvtbAddPropsurface();
    afx_msg void OnOvtbAddAnimsurface();
    afx_msg void OnOvtbRefresh(); 
    afx_msg void OnOvtbUpdateGeomsFromSel();
    afx_msg void OnOvtbUpdateRigidInst();
    afx_msg void OnOvtbSelectBasedOnRigidInst(); 
    afx_msg void OnUpdateOvtbAddPlaysurface(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbAddPropsurface(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbAddAnimsurface(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbRefresh(CCmdUI* pCmdUI); 
    afx_msg void OnUpdateOvtbUpdateGeomsFromSel(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbUpdateRigidInst(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbSelectBasedOnRigidInst(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
    void AddPathToTree(CString strPath, CString strName);
    HTREEITEM DoesChildExist(CString strCurrent, HTREEITEM hParent);

  	CImageList	        m_imageList;
    xharray<CString>    m_xaPaths;
    CString             m_strType;
    BOOL                m_bCanAdd;
};

inline CEditorPaletteDoc* CEditorObjectView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorObjectView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
