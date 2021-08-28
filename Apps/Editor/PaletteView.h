#if !defined(AFX_PALETTEVIEW_H__F07DE6E9_5368_4114_BA20_1FF4BFB0AD16__INCLUDED_)
#define AFX_PALETTEVIEW_H__F07DE6E9_5368_4114_BA20_1FF4BFB0AD16__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PaletteView.h : header file
//

#include "..\WinControls\XTDialogToolBar.h"

/////////////////////////////////////////////////////////////////////////////
// CPaletteView view

class CPaletteView : public CView
{
protected:
	CPaletteView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPaletteView)

// Operations
public:
    
    virtual void    OnTabActivate( BOOL bActivate );
    CSize           SizeToolBar(int cx, int cy);
    BOOL            IsActive() { return m_bActive; } 

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPaletteView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPaletteView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CPaletteView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:

    int m_ToolbarResourceId;    //0 for none
    BOOL m_bActive;
    CXTDialogToolBar m_wndToolBar;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PALETTEVIEW_H__F07DE6E9_5368_4114_BA20_1FF4BFB0AD16__INCLUDED_)
