#if !defined(AFX_VALUEGRID_H__082CF43F_AE2B_456F_91F6_4D31ECB2028E__INCLUDED_)
#define AFX_VALUEGRID_H__082CF43F_AE2B_456F_91F6_4D31ECB2028E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ValueGrid.h : header file
//
#include "..\PropertyEditor\GridListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CValueGrid window

class CValueGrid : public CGridListCtrl
{
// Construction
public:
	CValueGrid();

// Attributes
public:

// Operations
public:
    
	void InitializeGrid(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CValueGrid)
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CValueGrid();

	//handle calling out
	virtual void OnGridItemChange(CGridTreeItem *pSelItem);
	//handle guid message
	virtual void OnGuidSelect(CGridTreeItem *pSelItem);

	// Generated message map functions
protected:
	//{{AFX_MSG(CValueGrid)
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VALUEGRID_H__082CF43F_AE2B_456F_91F6_4D31ECB2028E__INCLUDED_)
