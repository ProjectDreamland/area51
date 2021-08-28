#if !defined(AFX_PUSHBUTTONBMP_H__6FB53992_0336_4AC9_8B5C_F8E615150CA5__INCLUDED_)
#define AFX_PUSHBUTTONBMP_H__6FB53992_0336_4AC9_8B5C_F8E615150CA5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CPushButtonBmp.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPushButtonBmp window

#define WM_USER_MSG_PUSHBTN_CLICKED       WM_USER + 300

class CPushButtonBmp : public CWnd
{
    // Construction
    public:
	    CPushButtonBmp();

    // Attributes
    public:

    // Operations
    public:

    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CPushButtonBmp)
	public:
	virtual BOOL Create(CWnd* pParentWnd, const char* Label, int posX, int posY, int nWidth, int nHeight, UINT nID);
	virtual void MoveWindow(int x, int y, int nWidth, int nHeight, BOOL bRepaint = TRUE );
	//}}AFX_VIRTUAL

    // Implementation
    public:
	    virtual     ~CPushButtonBmp();

        enum ButtonType
        {
            BUTTON_TYPE_PUSHBUTTON,
            BUTTON_TYPE_CHECKBUTTON,
            BUTTON_TYPE_TOGGLEBUTTON,

            BUTTON_TYPE_PAD = 0xffffffff
        };

        void        SetButtonType       ( ButtonType BtnType = BUTTON_TYPE_PUSHBUTTON );

        void        SetIsChecked        ( bool IsChecked );
        void        SetCheckColor       ( COLORREF CheckColor );

        void        SetPushBitmapUp     ( UINT nIDResourceColor, UINT nIDResourceAlpha = NULL );
        void        SetToggleBitmapUp   ( UINT nIDResourceColor, UINT nIDResourceAlpha = NULL );

    protected:
        // Button Properties
        ButtonType  m_ButtonType;

        bool        m_IsChecked;
        bool        m_IsHot;
        bool        m_IsPressed;

        int         m_Width;
        int         m_Height;

        // Drawing Properties
        COLORREF    m_ColorChecked;

        CFont       m_Font;

        CString     m_Label;

        CBitmap     m_BmpPushColor_Up;
        CBitmap     m_BmpPushAlpha_Up;

        CBitmap     m_BmpToggleColor_Up;
        CBitmap     m_BmpToggleAlpha_Up;


/****** NOT USED YET ******************
        CBitmap     m_BmpPushColor_Down;
        CBitmap     m_BmpPushAlpha_Down;

        CBitmap     m_BmpToggleColor_Down;
        CBitmap     m_BmpToggleAlpha_Down;

        CBitmap     m_BmpPushColor_Hot;
        CBitmap     m_BmpPushAlpha_Hot;

        CBitmap     m_BmpToggleColor_Hot;
        CBitmap     m_BmpToggleAlpha_Hot;
***************************************/


	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CPushButtonBmp)
	    afx_msg void OnPaint();
	    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PUSHBUTTONBMP_H__6FB53992_0336_4AC9_8B5C_F8E615150CA5__INCLUDED_)
