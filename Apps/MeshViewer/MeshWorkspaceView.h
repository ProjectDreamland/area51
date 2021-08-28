#if !defined(AFX_MeshWorkspaceVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_MeshWorkspaceVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MeshWorkspaceView.h : header file
//
#include "..\Editor\Project.hpp"
#include "..\WinControls\FileTreeCtrl.h"

class CMeshWorkspaceDoc;


/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceView view

class CMeshWorkspaceView : public CView
{
protected:
	CMeshWorkspaceView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMeshWorkspaceView)

// Attributes
public:
	CMeshWorkspaceDoc* GetDocument();

//    CXTToolBar		 m_wndToolBar;
    CListBox         m_lstBox;
    CFileTreeCtrl    m_fbcDirs;
    CStatic          m_stTitle;

// Operations
public:

    BOOL    CanAddToTheme();
    void    RefreshThemeRsc();
    void    RefreshGeomList();
    CString GetRscPath();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshWorkspaceView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CMeshWorkspaceView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMeshWorkspaceView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDblclkListBox();
	afx_msg void OnSelchangedThemeFolder(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

inline CMeshWorkspaceDoc* CMeshWorkspaceView::GetDocument()
   { return (CMeshWorkspaceDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MeshWorkspaceVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
