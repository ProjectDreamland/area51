#if !defined(AFX_PARAMETRICSPLITTER_H__0505CAAB_5516_42E4_B4CD_FB5EC4E2644E__INCLUDED_)
#define AFX_PARAMETRICSPLITTER_H__0505CAAB_5516_42E4_B4CD_FB5EC4E2644E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParametricSplitter.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParametricSplitter frame with splitter

#ifndef __AFXEXT_H__
#include <afxext.h>
#endif

class CParametricSplitter : public CSplitterWnd
{
//	DECLARE_DYNCREATE(CParametricSplitter)

public:
	CParametricSplitter();

// Attributes
protected:
    f32         m_SplitX;
    f32         m_SplitY;

    xbool       m_IsMaximized;
    s32         m_MaximizedViewportID;
    s32         m_ActiveViewportID;

public:

// Operations
public:
    void        SetActiveViewport   ( s32 ID );
    xbool       IsViewportMaximized ( void );
    void        MaximizeViewport    ( void );
    void        UnmaximizeViewport  ( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CParametricSplitter)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

	virtual void RecalcLayout();    // call after changing sizes

// Implementation
public:
	virtual ~CParametricSplitter();

	// Generated message map functions
	//{{AFX_MSG(CParametricSplitter)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARAMETRICSPLITTER_H__0505CAAB_5516_42E4_B4CD_FB5EC4E2644E__INCLUDED_)
