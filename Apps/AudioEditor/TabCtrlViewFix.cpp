// TabCtrlViewFix.cpp : implementation file
//

#include "stdafx.h"
#include "TabCtrlViewFix.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================
// CTabCtrlViewFix
//=========================================================================

CTabCtrlViewFix::CTabCtrlViewFix()
{
    m_nLastActiveView = 0;
}

//=========================================================================

CTabCtrlViewFix::~CTabCtrlViewFix()
{
}

//=========================================================================

BEGIN_MESSAGE_MAP(CTabCtrlViewFix, CXTTabCtrlBar)
	//{{AFX_MSG_MAP(CTabCtrlViewFix)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
// CTabCtrlViewFix message handlers
//=========================================================================

void CTabCtrlViewFix::OnTabSelChange( int nIDCtrl, CXTTabCtrl* pTabCtrl )
{
    s32 iLastActive = m_nLastActiveView;
    s32 iNewActive  = GetTabCtrl().GetCurSel();

    CWnd* pLastActive = GetView(iLastActive);
    CWnd* pNewActive  = GetView(iNewActive);
    ASSERT( pLastActive->IsKindOf( RUNTIME_CLASS(CFrameWnd) ) );
    ASSERT( pNewActive->IsKindOf( RUNTIME_CLASS(CFrameWnd) ) );


    CView* pLastView = ((CFrameWnd*)pLastActive)->GetActiveView();
    CView* pNewView  = ((CFrameWnd*)pNewActive)->GetActiveView();
    ASSERT( pLastView->IsKindOf( RUNTIME_CLASS(CView) ) );
    ASSERT( pNewView->IsKindOf( RUNTIME_CLASS(CView) ) );


    // Hack class
    struct spy : public CView
    {
        inline void Fix( BOOL Q, CView* A, CView* B ) { OnActivateView ( Q, A, B ); }
    };

    // Notify changes
    ((spy*)pNewView)->Fix( TRUE,  pNewView, pLastView );
        
    // Make sure to remeber the last active view
    m_nLastActiveView = iNewActive;

    // Update the control
    CXTTabCtrlBar::OnTabSelChange(nIDCtrl, pTabCtrl);
}


//=========================================================================

CWnd* CTabCtrlViewFix::GetWorkspaceView( CRuntimeClass *pViewClass )
{
    CWnd* pWnd=NULL;
    for( s32 i=0; pWnd=GetView(i); i++ )
    {
        if( pWnd->IsKindOf( RUNTIME_CLASS(CFrameWnd) ) )
        {
            CFrameWnd* pFrame = (CFrameWnd*)pWnd;
            CView*     pView = pFrame->GetActiveView();

            if( pView->IsKindOf( pViewClass ) )
            {
                pWnd = pView;
                break;
            }
        }
    }

    return pWnd;
}