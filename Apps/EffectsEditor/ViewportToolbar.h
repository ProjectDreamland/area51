#if !defined(AFX_VIEWPORTTOOLBAR_H__4D2868C5_B54A_4164_A561_A3C300A7B08B__INCLUDED_)
#define AFX_VIEWPORTTOOLBAR_H__4D2868C5_B54A_4164_A561_A3C300A7B08B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ViewportToolbar.h : header file
//

#include "FlyoutList.h"
#include "PushButtonBmp.h"

/////////////////////////////////////////////////////////////////////////////
// CViewportToolbar window

#define WM_USER_MSG_VIEWPORT_TOOLBAR_MINMAX_CHANGED     WM_USER + 2000

class CViewportToolbar : public CControlBar
{
    // Construction
    public:
	             CViewportToolbar();
	    virtual ~CViewportToolbar();


    // Controls
    protected:
        CFlyoutList         m_CameraList;

        bool                m_IsMaximized;
        CPushButtonBmp      m_Btn_Maximize;





    //-----------------------------------------------------------------------
    // MFC Stuff below
    //-----------------------------------------------------------------------

    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CViewportToolbar)
	public:
	virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID);
	//}}AFX_VIRTUAL

	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CViewportToolbar)
	afx_msg void OnPaint();
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()

        afx_msg LRESULT OnPushButton_Clicked    ( WPARAM wParam, LPARAM lParam );

        afx_msg void    OnUpdateCmdUI           ( CFrameWnd* pTarget, BOOL bDisableIfNoHndler );
        afx_msg CSize   CalcFixedLayout         ( BOOL bStretch, BOOL bHorz );
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEWPORTTOOLBAR_H__4D2868C5_B54A_4164_A561_A3C300A7B08B__INCLUDED_)
