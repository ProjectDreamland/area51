// View1.cpp : implementation file
//

#include "BaseStdAfx.h"
//#include "TabCtrl.h"
#include "HelpView.h"
#include "resource.h"
#include "Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHelpView

IMPLEMENT_DYNCREATE(CHelpView, CXTHtmlView)

BEGIN_MESSAGE_MAP(CHelpView, CXTHtmlView)
	//{{AFX_MSG_MAP(CHelpView)
	ON_COMMAND(ID_BACK,    OnGoBack)
    ON_COMMAND(ID_FOWARD,  OnGoFoward)
    ON_COMMAND(ID_STOP,    OnStop)
    ON_COMMAND(ID_REFRESH, OnRefresh)
    ON_COMMAND(ID_GO_HOME, OnGoHome)
	ON_UPDATE_COMMAND_UI(ID_BACK,    OnEnable )
	ON_WM_PAINT()
	ON_UPDATE_COMMAND_UI(ID_FOWARD,  OnEnable )
	ON_UPDATE_COMMAND_UI(ID_STOP,    OnEnable )
	ON_UPDATE_COMMAND_UI(ID_REFRESH, OnEnable )
	ON_UPDATE_COMMAND_UI(ID_GO_HOME, OnEnable )
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CHelpView::OnGoBack()  { GoBack();     }
void CHelpView::OnGoFoward(){ GoForward();  }
void CHelpView::OnStop()    { Stop();       }
void CHelpView::OnRefresh() { Refresh();    }
void CHelpView::OnEnable(CCmdUI* pCmdUI){}

void CHelpView::OnGoHome()  
{ 
    /*
    char Path[256];

    x_strcpy( Path, g_Settings.GetCompilerPath() );

    s32 iLastDir=0;
    for( s32 i=0; Path[i]; i++ )
    {
        if( Path[i] == '/' ) iLastDir = i;
        if( Path[i] == '\\' ) iLastDir = i;
    }

    if( iLastDir != 0 ) Path[iLastDir] = 0;

    Navigate2( _T(xfs("%s\\Help\\Index.htm", Path )),NULL,NULL);    
    */
    Navigate2( _T("bugbase-a51//Help//Index.htm"),NULL,NULL);    
    

}


CHelpView::CHelpView()
{
	//{{AFX_DATA_INIT(CHelpView)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

    m_bFirstTimeVisible = TRUE;
    m_bInitialize       = FALSE;
}

CHelpView::~CHelpView()
{
}

void CHelpView::DoDataExchange(CDataExchange* pDX)
{
	CXTHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHelpView)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


/////////////////////////////////////////////////////////////////////////////
// CHelpView diagnostics

#ifdef _DEBUG
void CHelpView::AssertValid() const
{
	CXTHtmlView::AssertValid();
}

void CHelpView::Dump(CDumpContext& dc) const
{
	CXTHtmlView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CHelpView message handlers

void CHelpView::OnInitialUpdate() 
{
	// TODO: This code navigates to a popular spot on the web.
	//  change the code to go where you'd like.    

    if( !m_bInitialize )
    {
	    if (!m_wndToolBar.CreateEx( GetParent(), TBSTYLE_FLAT | TBSTYLE_LIST) ||
		    !m_wndToolBar.LoadToolBar(IDR_HELP_BROSWER))
	    {
		    TRACE0("Failed to create toolbar\n");
	    }

	    if (!m_wndAnimateBar.Create(WS_CHILD|WS_VISIBLE|ACS_CENTER,
		    CRect(500+0,0,500+38,22), &m_wndToolBar, 0) ||
		    !m_wndAnimateBar.Open(IDR_WEB_THINKING))
	    {
		    TRACE0("Failed to create animation control.\n");
	    }

        m_bInitialize = TRUE;
    }
}

void CHelpView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
    if( m_bFirstTimeVisible )
    {
        OnGoHome();
        m_bFirstTimeVisible = FALSE;
    }

	// TODO: Add your message handler code here
	
	// Do not call CXTHtmlView::OnPaint() for painting messages
}


void CHelpView::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel)
{
    m_wndAnimateBar.Play( 0, -1, -1);
    CXTHtmlView::OnBeforeNavigate2(lpszURL, nFlags, lpszTargetFrameName, baPostedData, lpszHeaders,pbCancel);
}

void CHelpView::OnNavigateComplete2(LPCTSTR strURL)
{
    m_wndAnimateBar.Stop();
    CXTHtmlView::OnNavigateComplete2(strURL);
}

void CHelpView::OnSize(UINT nType, int cx, int cy) 
{
	CXTHtmlView::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
    if( m_bInitialize )
    {
        CRect rect;

	    m_wndToolBar.GetWindowRect( rect );
        s32 p = rect.right - rect.left-38;
        m_wndAnimateBar.MoveWindow( CRect(p,0,p+38,22));
    }
	
}
