// MeshViewer_View.cpp : implementation file
//

#include "stdafx.h"
#include "MeshViewer_View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMeshViewer_View

IMPLEMENT_DYNCREATE(CMeshViewer_View, CView3D)

CMeshViewer_View::CMeshViewer_View() :
m_pFrameEdit(NULL)
{
}

CMeshViewer_View::~CMeshViewer_View()
{
}


BEGIN_MESSAGE_MAP(CMeshViewer_View, CView3D)
	//{{AFX_MSG_MAP(CMeshViewer_View)
	ON_WM_PAINT()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMeshViewer_View drawing

void CMeshViewer_View::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CMeshViewer_View diagnostics

#ifdef _DEBUG
void CMeshViewer_View::AssertValid() const
{
	CView3D::AssertValid();
}

void CMeshViewer_View::Dump(CDumpContext& dc) const
{
	CView3D::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMeshViewer_View message handlers

void CMeshViewer_View::OnPaint() 
{
    CView3D::OnPaint();

    if( ((CMeshViewer_Doc*)GetDocument())->IsOrbitMode() )
    {
        CameraOrbitMode( ((CMeshViewer_Doc*)GetDocument())->m_Viewer.GetObjectFocus() );      
    }
    else
    {
        CameraFreeFlyMode();
    }

/*
    s32 y=0;
    s32 x=0;
    x_printfxy( x,y++, "Left Button: %d ", GetMouseLeftButton() );
    x_printfxy( x,y++, "Right Button: %d ", GetMouseRightButton() );
    x_printfxy( x,y++, "DeltaX: %d ", GetMouseDeltaX() );
    x_printfxy( x,y++, "DeltaY: %d ", GetMouseDeltaY() );
    x_printfxy( x,y++, "MouseX: %d ", GetMouseX() );
    x_printfxy( x,y++, "MouseY: %d ", GetMouseY() );
    x_printfxy( x,y++, "Action View: %d ", IsActionMode() );
*/
    eng_SetBackColor( xcolor(0x1f, 0x1f, 0x1f, 0xff) );

    if( eng_Begin( "Mesh Viewer.hpp" ) )
    {
        CMeshViewer_Doc* pDoc = (CMeshViewer_Doc*)GetDocument();

        m_Grid.Render();
        m_Axis.Render();

        pDoc->m_Viewer.SetRenderToBind      ( pDoc->m_bTakeToBindPose   );
        pDoc->m_Viewer.SetRenderSkel        ( pDoc->m_bRenderSkeleton   );
        pDoc->m_Viewer.SetRenderSkelLabels  ( pDoc->m_bRenderBoneLavels );
        pDoc->m_Viewer.Render();

        eng_End();
    }

    eng_PageFlip();
    
}

void RenderSolid( void ) {}

void CMeshViewer_View::OnTimer(UINT nIDEvent) 
{
    if( (((CMeshViewer_Doc*)GetDocument())->IsDocumentActive()) && 
        (((CMeshViewer_Doc*)GetDocument())->IsPause() == FALSE ))
    {
        // Do something if we are asked to do this in real time
	    SetViewDirty();
    }

	CView3D::OnTimer(nIDEvent);
}


void CMeshViewer_View::OnActivateFrame( UINT nState, CFrameWnd* pFrameWnd )
{
	if (!m_pFrameEdit)
    {
	    m_pFrameEdit = (CMeshViewer_Frame*) pFrameWnd;
    }
	CView3D::OnActivateFrame( nState, pFrameWnd );
}