#if !defined(AFX_STATUSBOX_H__931CB1B2_26AF_47C8_AEF8_B2307ECC11E2__INCLUDED_)
#define AFX_STATUSBOX_H__931CB1B2_26AF_47C8_AEF8_B2307ECC11E2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StatusBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStatusBox window

class CStatusBox : public CWnd
{
    // Construction
    public:
	    CStatusBox();

    // Attributes
    public:

    // Operations
    public:

    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CStatusBox)
	public:
	virtual BOOL Create(CWnd* pParentWnd, const char* Text, int posX, int posY, int nWidth, int nHeight, UINT nID, COLORREF bkColor = RGB(96,96,96), COLORREF txtColor = RGB(192,192,192) );
	virtual void MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE );
	//}}AFX_VIRTUAL

    // Implementation
    public:
	    virtual ~CStatusBox();

        void        SetStatusText       ( const char* Text );
        void        SetBackgroundColor  ( COLORREF bkColor  = RGB( 96, 96, 96) );
        void        SetTextColor        ( COLORREF txtColor = RGB(192,192,192) );

    protected:
        CString     m_StatusText;

        int         m_Width;
        int         m_Height;

        COLORREF    m_clrText;
        COLORREF    m_clrBackground;
        CFont       m_Font;


	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CStatusBox)
	afx_msg void OnPaint();
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STATUSBOX_H__931CB1B2_26AF_47C8_AEF8_B2307ECC11E2__INCLUDED_)
