#if !defined(AFX_RESOURCEBROWSERDLG_H__E9F5FFB9_24C9_4E6D_9AE3_DEE144DB346B__INCLUDED_)
#define AFX_RESOURCEBROWSERDLG_H__E9F5FFB9_24C9_4E6D_9AE3_DEE144DB346B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourceBrowserDlg.h : header file
//
#include "resource.h"
#include "ResourcePreview.h"

/////////////////////////////////////////////////////////////////////////////
// CResourceBrowserDlg dialog

class CResourceBrowserDlg : public CDialog
{
// Construction
public:
	CResourceBrowserDlg(CWnd* pParent = NULL);   // standard constructor

    void SetType(CString strType) { m_strType = strType; }

    CString GetPath() { return m_strPath; }
    CString GetName() { return m_strName; }

// Dialog Data
	//{{AFX_DATA(CResourceBrowserDlg)
	enum { IDD = IDD_RESOURCE_BROWSER_DLG };
	CButton	m_btnOk;
	CTreeCtrl	m_rscTree;
    CResourcePreview    m_wndPreview;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourceBrowserDlg)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

    void AddPathToTree(CString strPath, CString strName);
    HTREEITEM DoesChildExist(CString strCurrent, HTREEITEM hParent);

	// Generated message map functions
	//{{AFX_MSG(CResourceBrowserDlg)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangedTreeResourceSelector(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedClear();

private:
    CString             m_strPath;
    CString             m_strName;
    CString             m_strType;
  	CImageList	        m_imageList;
    xharray<CString>    m_xaPaths;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCEBROWSERDLG_H__E9F5FFB9_24C9_4E6D_9AE3_DEE144DB346B__INCLUDED_)
