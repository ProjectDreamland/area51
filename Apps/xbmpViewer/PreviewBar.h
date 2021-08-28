#if !defined(AFX_PREVIEWBAR_H__80A71474_9533_4F96_A99D_295077064A1C__INCLUDED_)
#define AFX_PREVIEWBAR_H__80A71474_9533_4F96_A99D_295077064A1C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExplorerBar.h : header file
//

#include "BitmapViewer.h"

/////////////////////////////////////////////////////////////////////////////
// CExplorerBar window

class CPreviewBar : public CXTDockWindow
{
// Construction
public:
	CPreviewBar();

// Attributes
public:
    CString         m_Path;
    xbitmap*        m_pBitmap;
    CBitmapViewer   m_wndBitmap1;
    CBitmapViewer   m_wndBitmap2;
    CSliderCtrl     m_wndMip;

// Operations
public:
    void    RepositionWindows( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPreviewBar)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPreviewBar();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPreviewBar)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
    afx_msg LRESULT OnNewBitmap(WPARAM, LPARAM);
    afx_msg LRESULT OnNewMipLevel(WPARAM, LPARAM);

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PREVIEWBAR_H__80A71474_9533_4F96_A99D_295077064A1C__INCLUDED_)
