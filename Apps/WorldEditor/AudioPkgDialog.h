#if !defined(AFX_AUDIOPKGDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_)
#define AFX_AUDIOPKGDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioPkgDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// EXTERNALS
/////////////////////////////////////////////////////////////////////////////
extern CString g_String;

/////////////////////////////////////////////////////////////////////////////
// CAudioPkgDialog dialog

class CAudioPkgDialog : public CDialog
{
// Construction
public:
	CAudioPkgDialog(CWnd* pParent = NULL);   // standard constructor

    void OnLoadPackage ( void );

	enum { IDD = IDD_RESOURCE_BROWSER_DLG };
    CListBox	        m_DescListBox;
    CString             m_DescName;
    xbool               m_DescLoaded;
    xarray<xstring>     m_PackageLoaded;
    CTreeCtrl	        m_rscTree;
  	CImageList	        m_imageList;
    xharray<CString>    m_xaPaths;

// Implementation
protected:

    void        AddPathToTree   (CString strPath, CString strName);
    HTREEITEM   DoesChildExist  (CString strCurrent, HTREEITEM hParent);

	// Generated message map functions
	//{{AFX_MSG(CAudioPkgDialog)
	virtual BOOL OnInitDialog();
    virtual void OnOK( );
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

public:

    virtual BOOL DestroyWindow();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedClear();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOPKGDIALOG_H__0371E38C_43DF_4830_A25B_C92EA6D2093D__INCLUDED_)
