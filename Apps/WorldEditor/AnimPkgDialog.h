#if !defined(AFX_ANIMPKGDIALOG_H__B7DBA95C_3A1F_48EA_A883_233E7B3F3245__INCLUDED_)
#define AFX_ANIMPKGDIALOG_H__B7DBA95C_3A1F_48EA_A883_233E7B3F3245__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AnimPkgDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAnimPkgDialog dialog
struct animation_desc;

class CAnimPkgDialog : public CDialog
{
// Construction
public:
	CAnimPkgDialog(CWnd* pParent = NULL);   // standard constructor


    // Handles loading the animation packages.
    void OnLoadAnimPackage      ( animation_desc* pAnimDesc );


//    CListBox	    m_DescListBox;
    CString         m_DescName;
    xbool           m_DescLoaded;
//    xarray<xstring> m_AnimPackageList;



// Dialog Data
	//{{AFX_DATA(CAnimPkgDialog)
	enum { IDD = IDD_ANIMPACKAGE_DIALOG };
    CTreeCtrl	m_rscTree;
	//}}AFX_DATA

    


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnimPkgDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAnimPkgDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()


  	CImageList	        m_imageList;

public:
    afx_msg void OnBnClickedBtnClear();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANIMPKGDIALOG_H__B7DBA95C_3A1F_48EA_A883_233E7B3F3245__INCLUDED_)
