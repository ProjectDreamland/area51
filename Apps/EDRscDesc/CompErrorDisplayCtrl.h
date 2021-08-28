#if !defined(AFX_COMPERRORDISPLAYCTRL_H__F1341D9C_51CD_4D1E_8FAC_2206B1515CBC__INCLUDED_)
#define AFX_COMPERRORDISPLAYCTRL_H__F1341D9C_51CD_4D1E_8FAC_2206B1515CBC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CompErrorDisplayCtrl.h : header file
//
#include "OutputCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CompErrorDisplayCtrl window

class CompErrorDisplayCtrl : public COutputCtrl
{
/////////////////////////////////////////////////////////////////////////////
public:

    static CompErrorDisplayCtrl& GetDisplay( void ){ ASSERT(s_pThis); return *s_pThis; }

/////////////////////////////////////////////////////////////////////////////
protected:
    CFont                           m_Font;
    static CompErrorDisplayCtrl*    s_pThis;

/////////////////////////////////////////////////////////////////////////////
// MFC STUFF
/////////////////////////////////////////////////////////////////////////////
public:
	CompErrorDisplayCtrl();
	virtual ~CompErrorDisplayCtrl();

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CompErrorDisplayCtrl)
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CompErrorDisplayCtrl)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_COMPERRORDISPLAYCTRL_H__F1341D9C_51CD_4D1E_8FAC_2206B1515CBC__INCLUDED_)
