// DlgFindLog.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "DlgFindLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgFindLog dialog

CDlgFindLog::CDlgFindLog(LPCTSTR pTitle, CWnd* pParent /*=NULL*/)
	: CDialog(CDlgFindLog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CDlgFindLog)
	//}}AFX_DATA_INIT

    m_Title       = pTitle;
    m_pRecentList = NULL;
}

void CDlgFindLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgFindLog)
	DDX_Control(pDX, IDC_STRING, m_CtrlString);
	//}}AFX_DATA_MAP
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CDlgFindLog, CDialog)
	//{{AFX_MSG_MAP(CDlgFindLog)
	ON_BN_CLICKED(IDC_FIND_NEXT, OnFindNext)
	ON_BN_CLICKED(IDC_MARK_ALL, OnMarkAll)
	//}}AFX_MSG_MAP
	ON_CONTROL( CBN_ENTER, IDC_STRING, OnEnterString)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgFindLog message handlers

void CDlgFindLog::OnEnterString()
{
    // Select the new text	
    CString String;
    m_CtrlString.GetWindowText( String );

    // Tell the parent to find the string
    CWnd* pParent = m_pParentWnd;
    if( pParent )
    {
        pParent->SendMessage( AM_FIND, (WPARAM)&String, 0 );
    }

    // Add to recent list
    m_pRecentList->Add( String );

    // End the dialog
    EndDialog( 0 );
}

/////////////////////////////////////////////////////////////////////////////

BOOL CDlgFindLog::OnInitDialog() 
{
	CDialog::OnInitDialog();

    SetWindowText( m_Title );

    // Update the combo most recent list
    m_CtrlString.SetExtendedUI();
    UpdateMostRecent();
    m_CtrlString.SetCurSel( 0 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////

void CDlgFindLog::UpdateMostRecent( void )
{
    // Select the new text
    CString String;
    m_CtrlString.GetWindowText( String );

    // Add to recent list & recreate combo list
    m_CtrlString.SetRedraw( FALSE );
    if( m_pRecentList )
    {
        if( String.GetLength() > 0 )
            m_pRecentList->Add( String );

        m_CtrlString.ResetContent();
        for( int i=0 ; i<m_pRecentList->GetCount() ; i++ )
        {
            CString s = m_pRecentList->GetString( i );

            if( s.GetLength() > 0 )
                m_CtrlString.InsertString( i, s );
        }
    }
    m_CtrlString.SetWindowText( String );
    m_CtrlString.SetRedraw( TRUE );
}

/////////////////////////////////////////////////////////////////////////////

void CDlgFindLog::OnFindNext() 
{
    // Select the new text
    CString String;
    m_CtrlString.GetWindowText( String );

    // Update the combo most recent list
    UpdateMostRecent();

    // Tell the parent to find the string
    CWnd* pParent = m_pParentWnd;
    if( pParent )
    {
        pParent->SendMessage( AM_FIND, (WPARAM)&String, 0 );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CDlgFindLog::OnMarkAll()
{
    // Select the new text
    CString String;
    m_CtrlString.GetWindowText( String );

    // Update the combo most recent list
    UpdateMostRecent();

    // Tell the parent to mark all the strings
    CWnd* pParent = m_pParentWnd;
    if( pParent )
    {
        pParent->SendMessage( AM_MARK_ALL, (WPARAM)&String, 0 );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CDlgFindLog::SetRecentList( CRecentList& RecentList )
{
    m_pRecentList = &RecentList;
}

/////////////////////////////////////////////////////////////////////////////
