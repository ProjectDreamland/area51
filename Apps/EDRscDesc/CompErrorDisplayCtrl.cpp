// CompErrorDisplayCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "CompErrorDisplayCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BEGIN_MESSAGE_MAP(CompErrorDisplayCtrl, COutputCtrl)
	//{{AFX_MSG_MAP(CompErrorDisplayCtrl)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// SOME VARIABLES
/////////////////////////////////////////////////////////////////////////////

CompErrorDisplayCtrl* CompErrorDisplayCtrl::s_pThis = NULL;

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//=========================================================================
CompErrorDisplayCtrl::CompErrorDisplayCtrl()
{
    ASSERT( s_pThis == NULL );

    // For now we just allow only one type of such control to be created
    s_pThis = this;
}

//=========================================================================
CompErrorDisplayCtrl::~CompErrorDisplayCtrl()
{
    ASSERT( s_pThis == this );
    s_pThis = NULL;
}

//=========================================================================

int CompErrorDisplayCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    lpCreateStruct->dwExStyle |= WS_HSCROLL | WS_VSCROLL;

	if (COutputCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;
	
    // Create the font fix-size font
    m_Font.CreatePointFont( 10, "Courier");//"Fixedsys" );

	// TODO: Add your specialized creation code here
	SetFont( &m_Font );

    // Tell out selves that we are ready to do work
    SetWindowText( _T("Compile Output...\n") );

	return 0;
}
