// DialogTextEntry.cpp : implementation file
//

#include "stdafx.h"
#include "WorldEditor.hpp"
#include "DialogTextEntry.h"
#include ".\DialogTextEntry.h"


// CDialogTextEntry dialog

IMPLEMENT_DYNAMIC(CDialogTextEntry, CDialog)
CDialogTextEntry::CDialogTextEntry(CWnd* pParent /*=NULL*/)
	: CDialog(CDialogTextEntry::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDialogTextEntry)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CDialogTextEntry::~CDialogTextEntry()
{
}

void CDialogTextEntry::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDialogTextEntry)
    DDX_Text(pDX, IDC_NAME, m_Name);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDialogTextEntry, CDialog)
END_MESSAGE_MAP()


// CDialogTextEntry message handlers

BOOL CDialogTextEntry::OnInitDialog( )
{
    // Set the caption
    SetWindowText( m_Caption );

    // Success
    return TRUE;
}
