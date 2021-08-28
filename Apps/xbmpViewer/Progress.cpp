// Progress.cpp : implementation file
//

#include "stdafx.h"
#include "xbmpviewer.h"
#include "Progress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgress dialog


CProgress::CProgress(CWnd* pParent /*=NULL*/)
	: CDialog(CProgress::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProgress)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProgress)
	DDX_Control(pDX, IDC_TEXT, m_CtrlText);
	DDX_Control(pDX, IDC_PROGRESS, m_CtrlProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProgress, CDialog)
	//{{AFX_MSG_MAP(CProgress)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProgress message handlers

void CProgress::SetText( const char* pText )
{
    m_CtrlText.SetWindowText( pText );
}

void CProgress::SetProgress( s32 Percentage )
{
    m_CtrlProgress.SetPos( Percentage );
}


BOOL CProgress::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
    m_CtrlProgress.SetRange( 0, 100 );
    m_CtrlProgress.SetPos( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
