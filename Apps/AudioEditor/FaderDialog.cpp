//=========================================================================
// FADERDIALOG.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdafx.h"
#include "Resource.h"
#include "FaderDialog.h"
#include "AudioEditor.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString g_String =  "NONE\0";

//=========================================================================

CFaderDialog::CFaderDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CFaderDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFaderDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//=========================================================================

void CFaderDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	//{{AFX_DATA_MAP(CFaderDialog)
	DDX_Control(pDX, IDC_FADER_NAME, m_FaderName);
	DDX_Control(pDX, IDC_FADER_VIEW, m_ListBox);
	//}}AFX_DATA_MAP
}

//=========================================================================

BEGIN_MESSAGE_MAP(CFaderDialog, CDialog)
	//{{AFX_MSG_MAP(CFaderDialog)
        ON_COMMAND( IDC_FADERDLG_ADD,   OnNewFader  )

	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================

BOOL CFaderDialog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    CString Faders = g_String;
    CString FirstFader;
    s32 Index = -1;
    Index = Faders.Find( '\0' );

    // Go through the string and add all the fader names the list box.
    do
    {        
        FirstFader = Faders.Left( Index );
        m_ListBox.AddString( FirstFader );
    
        Index++;

        if( Index >= Faders.GetLength() )
            return TRUE;

        Faders = Faders.Right( Faders.GetLength() - Index );
        Index = Faders.Find( '\0' );
    }
    while( (Index != -1) && (Faders.GetLength()) );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//=========================================================================

void CFaderDialog::OnNewFader ( void )
{
    CString WindowText;
    m_FaderName.GetWindowText( WindowText );
    WindowText.MakeUpper();

    if( WindowText.GetLength() > 0 )
    {        
        if( g_String[g_String.GetLength() - 1] != '\0' )
            g_String.Insert( g_String.GetLength(), '\0' );

        g_String.Insert( g_String.GetLength(), (LPCTSTR)WindowText );
        g_String.Insert( g_String.GetLength(), '\0' );
        g_pFaderList = g_String.GetBuffer( g_String.GetLength() );
        m_ListBox.AddString( WindowText );

        m_FaderName.SetWindowText( "" );
        UpdateData( );
    }
}

//=========================================================================
