// WorkspaceTabCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "WorkspaceTabCtrl.h"
#include "..\Editor\PaletteView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================
// CWorkspaceTabCtrl
//=========================================================================

CWorkspaceTabCtrl::CWorkspaceTabCtrl()
{
    m_nLastActiveView = 0;
}

//=========================================================================

CWorkspaceTabCtrl::~CWorkspaceTabCtrl()
{
}

//=========================================================================

BEGIN_MESSAGE_MAP(CWorkspaceTabCtrl, CXTTabCtrlBar)
	//{{AFX_MSG_MAP(CWorkspaceTabCtrl)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
// CWorkspaceTabCtrl message handlers
//=========================================================================

void CWorkspaceTabCtrl::OnTabSelChange(int nIDCtrl, CXTTabCtrl* pTabCtrl)
{
    UpdateView(m_nLastActiveView, FALSE);
    m_nLastActiveView = GetTabCtrl().GetCurSel();
    UpdateView(m_nLastActiveView, TRUE);
    
    CXTTabCtrlBar::OnTabSelChange(nIDCtrl, pTabCtrl);
}

//=========================================================================

void CWorkspaceTabCtrl::UpdateView(int Index, BOOL bActivate)
{
    CWnd* pWnd = GetView(Index);
    if( pWnd->IsKindOf( RUNTIME_CLASS(CFrameWnd) ) )
    {
        CFrameWnd* pFrame = (CFrameWnd*)pWnd;
        CView*     pView = pFrame->GetActiveView();

        if( pView->IsKindOf( RUNTIME_CLASS(CPaletteView) ) )
        {
            CPaletteView* pPaletteView = (CPaletteView*)pView;
            pPaletteView->OnTabActivate(bActivate);
        }
    }
}

//=========================================================================

CWnd* CWorkspaceTabCtrl::GetWorkspaceView( CRuntimeClass *pViewClass )
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