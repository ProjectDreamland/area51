// CRulerBar : header file
/////////////////////////////////////////////////////////////////////////////

#ifndef __RULERBAR_H
#define __RULERBAR_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "Ruler.h"

/////////////////////////////////////////////////////////////////////////////
// CRulerBar class

class CRulerBar : public CXTControlBar
{
// Construction / destruction
public:

	// Constructs a CRulerBar object.
	CRulerBar();

	// Destroys a CRulerBar object, handles cleanup and de-allocation.
	virtual ~CRulerBar();

// Member variables
protected:
    CRuler              m_Ruler;
    CScrollBar          m_ScrollBar;

// Member operations
protected:
    int GetParentClientWidth( void );

// Member functions
public:
    BOOL    Create      (CWnd* pParentWnd, DWORD dwStyle, UINT nID);
    BOOL    CreateEx    (CWnd* pParentWnd, DWORD dwCtrlStyle, DWORD dwStyle, CRect rcBorders, UINT nID);

    void            SetUnits    ( CRuler::units Units ) { m_Ruler.SetUnits( Units ); };
    CRuler::units   GetUnits    ( void ) { return m_Ruler.GetUnits(); };

	//{{AFX_VIRTUAL(CRulerBar)
    protected:
    virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
    virtual CSize CalcDynamicLayout(int nLength, DWORD dwMode);
    virtual void DoPaint(CDC* pDC);
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CRulerBar)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // __RULERBAR_H

