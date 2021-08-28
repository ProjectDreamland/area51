#if !defined(AFX_EDITINT_H__A199593D_20B2_4819_8E8F_2C98C96094A2__INCLUDED_)
#define AFX_EDITINT_H__A199593D_20B2_4819_8E8F_2C98C96094A2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditInt.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditInt window

// User defined messages

#define WM_USER_MSG_EDIT_ENTERED    WM_USER + 100

class CEditInt : public CEdit
{
    // Construction
    public:
	    CEditInt();

    // Attributes
    public:

    // Operations
    public:

    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CEditInt)
	    public:
	    virtual BOOL PreTranslateMessage(MSG* pMsg);
	    //}}AFX_VIRTUAL

    // Implementation
    public:
	    virtual ~CEditInt();

        int     GetValue    ( void ) const;
        int     SetValue    ( int Value );

    protected:
        int         m_Value;

        COLORREF    m_clrText;
        COLORREF    m_clrBackground;
        CBrush      m_brBackground;


	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CEditInt)
	    afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	    afx_msg void OnPaint();
	    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	    afx_msg void OnSetFocus(CWnd* pOldWnd);
	    afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnUpdate();
	//}}AFX_MSG

	    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITINT_H__A199593D_20B2_4819_8E8F_2C98C96094A2__INCLUDED_)
