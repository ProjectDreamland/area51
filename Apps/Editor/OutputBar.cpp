// OutputBar.cpp : implementation file
//
/////////////////////////////////////////////////////////////////////////////

#include "BaseStdAfx.h"
#include "Editor.h"
#include "OutputBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputBar

COutputBar::COutputBar()
{

}

COutputBar::~COutputBar()
{
	// TODO: add destruction code here.
}

IMPLEMENT_DYNAMIC(COutputBar, CXTDockWindow)

BEGIN_MESSAGE_MAP(COutputBar, CXTDockWindow)
	//{{AFX_MSG_MAP(COutputBar)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COutputBar message handlers

int COutputBar::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    // Create a font that is friendly to read 
    m_Font.CreatePointFont( 10, "Courier");//"Fixedsys" );

	if (CXTDockWindow::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create  the flat tab control.
	if (!m_flatTabCtrl.Create(WS_CHILD|WS_VISIBLE|FTS_XT_DEFAULT|FTS_XT_HSCROLL,
		CRect(0,0,0,0), this, IDC_FLATTABCTRL))
	{
		TRACE0( "Failed to create flattab control\n" );
		return -1;
	}

	// Define the default style for the output list boxes.
	DWORD dwStyle = WS_CHILD | WS_VSCROLL | WS_HSCROLL | WS_TABSTOP | WS_CLIPCHILDREN| ES_MULTILINE | ES_READONLY | ES_AUTOHSCROLL ;

	// Create the sheet1 list box.
	if (!m_CompileOutput.Create( dwStyle, CRect(0,0,0,0), &m_flatTabCtrl, IDC_SHEET1 ))
	{
		TRACE0( "Failed to create sheet1.\n" );
		return -1;
	}

    // Set the output window for the messages
	if (!m_DebugMsgOutput.Create( dwStyle, CRect(0,0,0,0), &m_flatTabCtrl, IDC_SHEET1 ))
	{
		TRACE0( "Failed to create sheet1.\n" );
		return -1;
	}
    m_DebugMsgOutput.SetFont( &m_Font );


    // Set the output window for the messages
	if (!m_LogMsgOutput.Create( dwStyle, CRect(0,0,0,0), &m_flatTabCtrl, IDC_SHEET3 ))
	{
		TRACE0( "Failed to create sheet1.\n" );
		return -1;
	}
    m_LogMsgOutput.SetFont( &m_Font );
    m_LogMsgOutput.OnInitialUpdate();

 
	// Create the sheet2 list box.
	dwStyle = WS_CHILD | WS_VSCROLL | WS_TABSTOP | LBS_NOINTEGRALHEIGHT;
	if (!m_sheet2.Create( dwStyle, CRect(0,0,0,0), &m_flatTabCtrl, IDC_SHEET2 ))
	{
		TRACE0( "Failed to create sheet2.\n" );
		return -1;
	}
    m_sheet2.SetFont( &m_Font );

    dwStyle = WS_CHILD | WS_VSCROLL | WS_TABSTOP | LBS_NOINTEGRALHEIGHT;
    if (!m_SelectionsOutput.Create( dwStyle, CRect(0,0,0,0), &m_flatTabCtrl, IDC_SHEET4 ))
    {
	    TRACE0( "Failed to create Selectioned sheet.\n" );
	    return -1;
    }
    m_SelectionsOutput.SetFont( &m_Font );
    
    
    // Insert tabs into the flat tab control.
	m_flatTabCtrl.InsertItem(0, _T("Compile"),  &m_CompileOutput);
    m_flatTabCtrl.InsertItem(1, _T("DebugMsg"), &m_DebugMsgOutput);
	m_flatTabCtrl.InsertItem(2, _T("Sheet 2"),  &m_sheet2);
    m_flatTabCtrl.InsertItem(3, _T("Log"),      &m_LogMsgOutput);
    m_flatTabCtrl.InsertItem(3, _T("Selected Objects"),  &m_SelectionsOutput);

	// Insert text into the list box.
//	m_CompileOutput.AddString(_T("Compile Output..."));
    m_DebugMsgOutput.SetWindowText( _T("Ready 2 Output...\n") );

    //m_CompileOutput.LimitText(1 );

	m_sheet2.AddString(_T("Sheet 2 Output...This is a test"));

    m_LogMsgOutput.SetWindowText( _T("Ready 3 Log Output...\n") );

	// Set the current tab.
	m_flatTabCtrl.SetCurSel(0);

	// Draw an edge around the control.
	SetXTBarStyle(CBRS_XT_DEFAULT|CBRS_XT_CLIENT_STATIC);

	// Associate the flat tab control with the docking window.
	SetChild(&m_flatTabCtrl);



/*
    CRichEditCtrl& theEdit = m_CompileOutput;
    CString strTemp = "";
    s32 lines = theEdit.GetLineCount();
    
    char* lpszMessage = "hello";

    if( LockWindowUpdate() )
    {
        if(lines > 50)
        {
            s32 linendx = theEdit.LineIndex(1);
            theEdit.SetSel( 0, linendx );
            theEdit.ReplaceSel(strTemp);
        }
        // coming in, lpszMessage contains the
        // string to be displayed.
        strTemp = lpszMessage;

        // make the next line start on a new line
        strTemp += _T("\r\n");

        // Find out where the string should be
        // inserted
        UINT len = theEdit.GetBufferLength();
        theEdit.SetSel(len,len, FALSE);
        DWORD dwSel = theEdit.GetSel();
        theEdit.SetSel(HIWORD(dwSel), -1);

        // put the new string at the end of the
        // edit control's buffer
        theEdit.ReplaceSel(strTemp);

        // now can update the screen without
        // the scroll bar jumping
        UnlockWindowUpdate();
     };
*/

	return 0;
}
