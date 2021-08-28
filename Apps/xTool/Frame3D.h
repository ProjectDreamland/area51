#if !defined(AFX_FRAME3D_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_)
#define AFX_FRAME3D_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Frame3D.h : header file
//

#include "FrameBase.h"

class CViewBase;
class CViewGL;

/////////////////////////////////////////////////////////////////////////////
// CFrame3D window

class CFrame3D : public CFrameBase
{
// Construction
public:
	CFrame3D();

// Attributes
public:
    CXTSplitterWnd  m_Splitter;
    CViewBase*      m_pViewChannels;
    CViewGL*        m_pView3D;
    CXTToolBar      m_wndToolBar;
    CBitmap         m_bToolBarCold;
    CBitmap         m_bToolBarHot;
    CImageList      m_ilToolBarCold;
    CImageList      m_ilToolBarHot;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFrame3D)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFrame3D();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFrame3D)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRAME3D_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_)
