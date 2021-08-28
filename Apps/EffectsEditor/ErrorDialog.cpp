// ErrorDialog.cpp : implementation file
//

#include "stdafx.h"
#include "parted.h"
#include "ErrorDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CErrorDialog dialog


CErrorDialog::CErrorDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CErrorDialog::IDD, pParent)
{
    m_pErrorLog = NULL;
	//{{AFX_DATA_INIT(CErrorDialog)
	m_ValEdit = _T("");
	//}}AFX_DATA_INIT
}


void CErrorDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CErrorDialog)
	DDX_Control(pDX, IDC_EDIT, m_CtrlEdit);
	DDX_Text(pDX, IDC_EDIT, m_ValEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CErrorDialog, CDialog)
	//{{AFX_MSG_MAP(CErrorDialog)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CErrorDialog message handlers

int CErrorDialog::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    xstring s;
    for( s32 i=0 ; i<m_pErrorLog->GetCount() ; i++ )
    {
	    s += m_pErrorLog->GetError(i);
        s += "\r\n";
    }

    m_ValEdit = s;

	return 0;
}
