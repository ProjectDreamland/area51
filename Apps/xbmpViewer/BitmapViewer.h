#if !defined(AFX_BITMAPVIEWER_H__F7E4294A_EA1D_4272_BC4C_683FAD2CCC43__INCLUDED_)
#define AFX_BITMAPVIEWER_H__F7E4294A_EA1D_4272_BC4C_683FAD2CCC43__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BitmapViewer.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBitmapViewer window

class CBitmapViewer : public CWnd
{
// Construction
public:
    CBitmapViewer();

// Attributes
public:
    xbitmap*    m_pBitmap;
    s32         m_Mip;
    HBITMAP     m_hBitmap;
    xbool       m_Alpha;
    xbool        m_TrackingMouse;

// Operations
public:
    void    SetBitmap       ( xbitmap* pBitmap, s32 Mip=0 );
    void    SetAlpha        ( void );
    s32     GetBitmapWidth  ( void );
    s32     GetBitmapHeight ( void );

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CBitmapViewer)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CBitmapViewer();

    // Generated message map functions
protected:
    //{{AFX_MSG(CBitmapViewer)
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnMouseLeave(WPARAM w, LPARAM l);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BITMAPVIEWER_H__F7E4294A_EA1D_4272_BC4C_683FAD2CCC43__INCLUDED_)
