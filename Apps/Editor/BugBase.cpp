// View1.cpp : implementation file
//

#include "BaseStdAfx.h"
//#include "TabCtrl.h"
#include "BugBase.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBugBase

IMPLEMENT_DYNCREATE(CBugBase, CXTHtmlView)

CBugBase::CBugBase()
{
	//{{AFX_DATA_INIT(CBugBase)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
    m_bFirstTimeVisible = TRUE;
    m_bInitialize       = FALSE;
}

CBugBase::~CBugBase()
{
}

void CBugBase::DoDataExchange(CDataExchange* pDX)
{
	CXTHtmlView::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBugBase)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBugBase, CXTHtmlView)
	//{{AFX_MSG_MAP(CBugBase)
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

void CBugBase::OnGoBack()  { GoBack();     }
void CBugBase::OnGoFoward(){ GoForward();  }
void CBugBase::OnStop()    { Stop();       }
void CBugBase::OnRefresh() { Refresh();    }
void CBugBase::OnGoHome()  { Navigate2(_T("http://BugBase"),NULL,NULL);    }
void CBugBase::OnEnable(CCmdUI* pCmdUI){}


/////////////////////////////////////////////////////////////////////////////
// CBugBase diagnostics

#ifdef _DEBUG
void CBugBase::AssertValid() const
{
	CXTHtmlView::AssertValid();
}

void CBugBase::Dump(CDumpContext& dc) const
{
	CXTHtmlView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBugBase message handlers
static int a=0;
void CBugBase::OnInitialUpdate() 
{
	// TODO: This code navigates to a popular spot on the web.
	//  change the code to go where you'd like.
    //

    if( !m_bInitialize )
    {
	    if (!m_wndToolBar.CreateEx( GetParent(), TBSTYLE_FLAT | TBSTYLE_LIST ) ||
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


void CBugBase::OnPaint() 
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

void CBugBase::OnSize(UINT nType, int cx, int cy) 
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

void CBugBase::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel)
{
    m_wndAnimateBar.Play( 0, -1, -1);
    CXTHtmlView::OnBeforeNavigate2(lpszURL, nFlags, lpszTargetFrameName, baPostedData, lpszHeaders,pbCancel);
}

void CBugBase::OnNavigateComplete2(LPCTSTR strURL)
{
    m_wndAnimateBar.Stop();
    CXTHtmlView::OnNavigateComplete2(strURL);
}
