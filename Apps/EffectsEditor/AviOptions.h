#if !defined(AFX_AVIOPTIONS_H__ED20B205_B60B_4C28_91D9_FD64928A984A__INCLUDED_)
#define AFX_AVIOPTIONS_H__ED20B205_B60B_4C28_91D9_FD64928A984A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AviOptions.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CAviOptions dialog

class CAviOptions : public CDialog
{
// Construction
public:
	CAviOptions(CWnd* pParent = NULL);   // standard constructor

    enum res
    {
        RES_160,
        RES_320,
        RES_640
    };

    enum vport
    {
        VIEW_UL,
        VIEW_UR,
        VIEW_LL,
        VIEW_LR,
        VIEW_MAX
    };

    void        SetRange        ( int Start, int End   )    { m_Start = Start; m_End = End; }
    res         GetSelectedRes  ( void )                    { return (res)m_OutputRes;      }
    vport       GetSelectedView ( void )                    { return (vport)m_ViewSel;      }
    CString&    GetFileName     ( void )                    { return m_FileName;            }
    void        GetRange        ( int& Start, int& End )    { Start = m_Start; End = m_End; }
    
// Dialog Data
	//{{AFX_DATA(CAviOptions)
	enum { IDD = IDD_AVI_EXPORT };
	CString	m_FileName;
	int		m_OutputRes;
	int		m_ViewSel;
	int		m_Start;
	int		m_End;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAviOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CAviOptions)
	afx_msg void OnBrowseAvi();
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AVIOPTIONS_H__ED20B205_B60B_4C28_91D9_FD64928A984A__INCLUDED_)
