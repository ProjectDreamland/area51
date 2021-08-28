// AviOptions.cpp : implementation file
//

#include "stdafx.h"
#include "parted.h"
#include "AviOptions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAviOptions dialog


CAviOptions::CAviOptions(CWnd* pParent /*=NULL*/)
	: CDialog(CAviOptions::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAviOptions)
	m_FileName = _T("");
	m_OutputRes = -1;
	m_ViewSel = -1;
	m_Start = 0;
	m_End = 0;
	//}}AFX_DATA_INIT
}


void CAviOptions::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAviOptions)
	DDX_Text(pDX, IDC_FILENAME, m_FileName);
	DDX_CBIndex(pDX, IDC_OUTPUT_RES, m_OutputRes);
	DDX_CBIndex(pDX, IDC_VIEW_SEL, m_ViewSel);
	DDX_Text(pDX, IDC_START_FRAME, m_Start);
	DDX_Text(pDX, IDC_END_FRAME, m_End);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAviOptions, CDialog)
	//{{AFX_MSG_MAP(CAviOptions)
	ON_BN_CLICKED(IDC_BROWSE_AVI, OnBrowseAvi)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAviOptions message handlers

void CAviOptions::OnBrowseAvi() 
{
	// TODO: Add your control notification handler code here
    char Filter[] = "AVI Files (*.avi)|*.avi||";

    UpdateData();

    CFileDialog Dlg( FALSE, ".AVI", NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Filter );

    if ( Dlg.DoModal() == IDOK )
    {
        m_FileName = Dlg.GetPathName();
        UpdateData( FALSE );
    }
}


void CAviOptions::OnOK() 
{
	// TODO: Add extra validation here
    UpdateData();

    if ( m_Start > m_End )
    {
        MessageBox( "Start frame must be less than end frame!", "Error!", MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL );
        return;
    }

    if ( !m_FileName.GetLength() )
    {
        MessageBox( "You must choose a file name!", "Error!", MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL );
        return;
    }
    
	CDialog::OnOK();
}

BOOL CAviOptions::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	
    m_ViewSel = VIEW_MAX;
    m_OutputRes = RES_320;
    UpdateData( FALSE );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
