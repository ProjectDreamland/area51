#if !defined(AFX_ELEMBROWSER_H__A31522E6_2D58_4F3A_9423_B791CB77FFC7__INCLUDED_)
#define AFX_ELEMBROWSER_H__A31522E6_2D58_4F3A_9423_B791CB77FFC7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ElemBrowser.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CElemBrowser dialog

class CElemBrowser : public CDialog
{
// Construction
public:
	CElemBrowser(CWnd* pParent = NULL);   // standard constructor

public:
    ~CElemBrowser();

    void    SetEffectDocument   ( CPartEdDoc* pDoc )    { m_pDoc = pDoc;      }
    s32     GetSelCount         ( void )                { return m_SelCount;  }
    DWORD   GetSelItem          ( s32 Idx )             { return m_pData[Idx]; }

// Dialog Data
	//{{AFX_DATA(CElemBrowser)
	enum { IDD = IDD_ELEM_LIST };
	CXTButton	m_Down;
	CXTButton	m_Up;
	CListBox	m_ElemList;
	BOOL	m_ShouldHide;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CElemBrowser)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

    CPartEdDoc*         m_pDoc;
    DWORD*              m_pData;
    s32                 m_SelCount;

    void    PopulateList        ( void );

	// Generated message map functions
	//{{AFX_MSG(CElemBrowser)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeElemList();
	afx_msg void OnChangeRenameSelected();
	afx_msg void OnDestroy();
	afx_msg void OnDblclkElemList();
	afx_msg void OnUp();
	afx_msg void OnDown();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ELEMBROWSER_H__A31522E6_2D58_4F3A_9423_B791CB77FFC7__INCLUDED_)
