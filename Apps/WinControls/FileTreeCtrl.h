#if !defined(AFX_FileTreeCtrl_H__A46AC15C_96B1_4741_AF5E_3C3999DD6802__INCLUDED_)
#define AFX_FileTreeCtrl_H__A46AC15C_96B1_4741_AF5E_3C3999DD6802__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileTreeCtrl.h : header file
//
struct tree_structure_info
{
    char     cPath[MAX_PATH];
    char     cWildcard[MAX_PATH];
    char     cForcedExt[MAX_PATH];
};

/////////////////////////////////////////////////////////////////////////////
// CFileTreeCtrl window

class CFileTreeCtrl : public CTreeCtrl
{
// Construction
public:
	CFileTreeCtrl();

// Attributes
public:

// Operations
public:
    void    BuildTreeFromPath(CString strRootPath, CString strWildcard, CString strForcedExt);
    void    Refresh();
    void    ClearTree();
    void    UsePreviousPathAsDisplay(BOOL bUsePrev) { m_bUsePrev = bUsePrev; }

    CString GetSelectedPath();
    BOOL    IsFolder(CString strPath);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFileTreeCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFileTreeCtrl();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFileTreeCtrl)
    afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnFileRename();
	afx_msg void OnFileDelete();
	afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

protected:
    void        RecursePath(xhandle hData, CString strPath, CString strWildcard, HTREEITEM hRoot);
    CString     ItemToPath(HTREEITEM hItem);
    void        Delete(HTREEITEM hItem);

private:
    xharray<tree_structure_info>     m_xaTreeStruct;

    BOOL        m_bInit;
    BOOL        m_bUsePrev;
  	CImageList	m_imageList;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FileTreeCtrl_H__A46AC15C_96B1_4741_AF5E_3C3999DD6802__INCLUDED_)
