// StringEntryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "StringEntryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CStringEntryDlg dialog


CStringEntryDlg::CStringEntryDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CStringEntryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CStringEntryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_nTextLimit = MAX_PATH;
}


void CStringEntryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CStringEntryDlg)
	DDX_Control(pDX, IDC_STATIC_TEXT_DISPLAY, m_stDisplay);
	DDX_Control(pDX, IDC_DATA_ENTRY, m_edEntry);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CStringEntryDlg, CDialog)
	//{{AFX_MSG_MAP(CStringEntryDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CStringEntryDlg message handlers

void CStringEntryDlg::OnOK() 
{
	// TODO: Add extra validation here
	m_stDisplay.GetWindowText(m_strDisplayText);
    m_edEntry.GetWindowText(m_strEntryText);
	
	CDialog::OnOK();
}

BOOL CStringEntryDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_stDisplay.SetWindowText(m_strDisplayText);
    m_edEntry.SetWindowText(m_strEntryText);

    m_edEntry.SetFocus();
    m_edEntry.SetLimitText(m_nTextLimit);

	return FALSE;  // return TRUE unless you set the focus to a control
	               // EXCEPTION: OCX Property Pages should return FALSE
}

void CStringEntryDlg::SetTextLimit(int nLimit)
{
    m_nTextLimit = nLimit;
}
