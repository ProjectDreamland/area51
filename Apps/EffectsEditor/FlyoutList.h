#if !defined(AFX_FLYOUTLIST_H__9A363957_3251_4AEF_A667_54BF19053DB7__INCLUDED_)
#define AFX_FLYOUTLIST_H__9A363957_3251_4AEF_A667_54BF19053DB7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FlyoutList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CFlyoutList window

class CFlyoutList : public CWnd
{
    // Construction / Destruction
    public:
	             CFlyoutList();
	    virtual ~CFlyoutList();


    // Properties
    protected:
        // FlyoutList Properties
        int         m_Selection;
        int         m_nItems;

        CString*    m_pItemLabels;

        // Drawing Properties
        int         m_Width;
        int         m_Height;

        COLORREF    m_ColorBackground;

        CFont       m_FontSelected;
        CFont       m_FontList;

        




    //-----------------------------------------------------------------------
    // MFC Stuff below
    //-----------------------------------------------------------------------

    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CFlyoutList)
	public:
	virtual BOOL Create(CWnd* pParentWnd, int NumItems, CString* pItemLabels, int posX, int posY, int nWidth, int nHeight, UINT nID);
	//}}AFX_VIRTUAL

	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CFlyoutList)
	afx_msg void OnPaint();
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FLYOUTLIST_H__9A363957_3251_4AEF_A667_54BF19053DB7__INCLUDED_)
