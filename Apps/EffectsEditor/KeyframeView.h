#if !defined(AFX_KEYFRAMEVIEW_H__5C28FC98_B8AF_4D80_A4C4_6D4E10E77CA1__INCLUDED_)
#define AFX_KEYFRAMEVIEW_H__5C28FC98_B8AF_4D80_A4C4_6D4E10E77CA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// KeyframeView.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CKeyframeView window

class CKeyframeView : public CControlBar
{
// Construction
public:
	CKeyframeView();

// Attributes
public:

// Operations
public:
    void  OnUpdateCmdUI( CFrameWnd* pTarget, BOOL bDisableIfNoHndler );
    CSize CalcFixedLayout( BOOL bStretch, BOOL bHorz );
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CKeyframeView)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CKeyframeView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CKeyframeView)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_KEYFRAMEVIEW_H__5C28FC98_B8AF_4D80_A4C4_6D4E10E77CA1__INCLUDED_)
