//
// ExplorerBar.cpp : implementation file
//

#include "stdafx.h"
#include "xbmpViewer.h"
#include "ExplorerBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CExplorerBar

CExplorerBar::CExplorerBar()
{
}

CExplorerBar::~CExplorerBar()
{
}

void CExplorerBar::SetPath( CString Path )
{
    m_Path = Path;
    m_Tree.PopulateTree( Path );
    m_Tree.TunnelTree( Path );
}

CString CExplorerBar::GetPath( void )
{
    return m_Path;
}

BEGIN_MESSAGE_MAP(CExplorerBar, CXTDockWindow)
    //{{AFX_MSG_MAP(CExplorerBar)
    ON_WM_CREATE()
    ON_WM_WINDOWPOSCHANGED()
    ON_MESSAGE( XTWM_SHELL_NOTIFY, OnUpdateShell )
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CExplorerBar message handlers

int CExplorerBar::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CXTDockWindow::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_Tree.EnableAutoInit( TRUE );
    m_Tree.SetEnumFlags( SHCONTF_FOLDERS );
    m_Tree.AssociateList( this );

    // Create the shell control
    VERIFY( m_Tree.Create( WS_VISIBLE, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST ) );

    return 0;
}

void CExplorerBar::OnWindowPosChanged(WINDOWPOS FAR* lpwndpos)
{
    CXTDockWindow::OnWindowPosChanged(lpwndpos);

    CRect rc;
    GetInsideRect( rc );
    rc.DeflateRect( 1, 1 );

    // Resize controls
    if( m_Tree.GetSafeHwnd() )
    {
        m_Tree.MoveWindow( &rc ); // rc.left, rc.top, rc.Width(), rc.Height() );
        m_Tree.RedrawWindow();
    }

}

LRESULT CExplorerBar::OnUpdateShell( WPARAM wParam, LPARAM lParam )
{
    switch( wParam )
    {
    case SHN_XT_TREESELCHANGE:
        {
            XT_TVITEMDATA* pItemData = (XT_TVITEMDATA*)lParam;
            ASSERT( pItemData );

            m_Tree.GetSelectedFolderPath( m_Path );
            const char* pPath = m_Path;
            CWnd* pMainWnd = AfxGetMainWnd();
            pMainWnd->SendMessage( NM_DIRCHANGED, (WPARAM)pPath, 0 );
        }
        break;
    }

    return 0;
}
