// XTSortListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "XTSortListCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CXTSortListCtrl

IMPLEMENT_DYNCREATE(CXTSortListCtrl, CXTListCtrl)

CXTSortListCtrl::CXTSortListCtrl()
{
}

CXTSortListCtrl::~CXTSortListCtrl()
{
}

BEGIN_MESSAGE_MAP(CXTSortListCtrl, CXTListCtrl)
	//{{AFX_MSG_MAP(CXTSortListCtrl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXTSortListCtrl diagnostics

#ifdef _DEBUG
void CXTSortListCtrl::AssertValid() const
{
	CXTListCtrl::AssertValid();
}

void CXTSortListCtrl::Dump(CDumpContext& dc) const
{
	CXTListCtrl::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CXTSortListCtrl message handlers
//=========================================================================

bool CXTSortListCtrl::SortList(
	// passed in from control, index of column clicked.
	int nCol,
	// passed in from control, true if sort order should 
	// be ascending.
	bool bAscending )
{
	CXTSortClass csc (this, nCol);
	csc.Sort(bAscending, DT_STRING);
	return true;
}
