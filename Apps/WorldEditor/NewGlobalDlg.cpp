// NewGlobalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewGlobalDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewGlobalDlg dialog
/////////////////////////////////////////////////////////////////////////////


CNewGlobalDlg::CNewGlobalDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewGlobalDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewGlobalDlg)
	m_RadioButtonData = -1;
	//}}AFX_DATA_INIT
}

/////////////////////////////////////////////////////////////////////////////

void CNewGlobalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewGlobalDlg)
	DDX_Control(pDX, IDC_GLOBAL_NAME, m_EditGlobalName);
	DDX_Radio(pDX, IDC_GLOBAL_INT, m_RadioButtonData);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CNewGlobalDlg, CDialog)
	//{{AFX_MSG_MAP(CNewGlobalDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewGlobalDlg message handlers

BOOL CNewGlobalDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    m_EditGlobalName.SetWindowText("<TYPE GLOBAL NAME>");
    m_EditGlobalName.SetSel( 0, -1 );
    m_EditGlobalName.SetFocus();
    m_EditGlobalName.SetLimitText(30);

    m_RadioButtonData = 0;
    UpdateData(FALSE);
	
	return FALSE;  // return TRUE unless you set the focus to a control
	               // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////

void CNewGlobalDlg::OnOK() 
{
    m_EditGlobalName.GetWindowText(m_strName);
    if (m_strName.IsEmpty())
    {
        ::AfxMessageBox("You must enter a name for the new global!");
        return;
    }
    UpdateData(TRUE);

	CDialog::OnOK();
}

/////////////////////////////////////////////////////////////////////////////

void CNewGlobalDlg::OnCancel() 
{
	CDialog::OnCancel();
}

/////////////////////////////////////////////////////////////////////////////

var_mngr::global_types CNewGlobalDlg::GetGlobalType()
{
    switch (m_RadioButtonData)
    {
    case 0: 
        return var_mngr::GLOBAL_INT;
        break;
    case 1: 
        return var_mngr::GLOBAL_FLOAT;
        break;
    case 2: 
        return var_mngr::GLOBAL_BOOL;
        break;
    case 3: 
        return var_mngr::GLOBAL_GUID;
        break;
    case 4: 
        return var_mngr::GLOBAL_TIMER;
        break;
    }

    return var_mngr::GLOBAL_INT;
}