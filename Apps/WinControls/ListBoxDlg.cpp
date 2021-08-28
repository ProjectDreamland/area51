// ListBoxDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ListBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CListBoxDlg dialog


CListBoxDlg::CListBoxDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CListBoxDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CListBoxDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CListBoxDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CListBoxDlg)
	DDX_Control(pDX, IDC_STATIC_TEXT_DISPLAY, m_stDisplayText);
	DDX_Control(pDX, IDC_LIST_STRINGS, m_lbStrings);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CListBoxDlg, CDialog)
	//{{AFX_MSG_MAP(CListBoxDlg)
	ON_LBN_DBLCLK(IDC_LIST_STRINGS, OnDblclkListStrings)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CListBoxDlg message handlers

void CListBoxDlg::OnDblclkListStrings() 
{
	OnOK();
}

void CListBoxDlg::OnOK() 
{
	m_lbStrings.GetText(m_lbStrings.GetCurSel(), m_strSelectedString);
	
	CDialog::OnOK();
}

BOOL CListBoxDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_stDisplayText.SetWindowText(m_strDisplayText);
    for (int i=0; i<m_lstStrings.GetCount(); i++)
    {
        CString strData = m_lstStrings.GetAt(m_lstStrings.FindIndex(i));
        m_lbStrings.AddString(strData);
    }
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
