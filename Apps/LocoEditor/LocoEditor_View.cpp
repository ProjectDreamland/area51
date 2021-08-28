// CLocoEditor_View.cpp : implementation file
//

#include "stdafx.h"
#include "LocoEditor_View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocoEditor_View

IMPLEMENT_DYNCREATE(CLocoEditor_View, CView3D)

CLocoEditor_View::CLocoEditor_View()
{
}

CLocoEditor_View::~CLocoEditor_View()
{
}


BEGIN_MESSAGE_MAP(CLocoEditor_View, CView3D)
	//{{AFX_MSG_MAP(CLocoEditor_View)
		// NOTE - the ClassWizard will add and remove mapping macros here.
        ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocoEditor_View drawing

void CLocoEditor_View::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CLocoEditor_View diagnostics

#ifdef _DEBUG
void CLocoEditor_View::AssertValid() const
{
	CView3D::AssertValid();
}

void CLocoEditor_View::Dump(CDumpContext& dc) const
{
	CView3D::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLocoEditor_View message handlers
void CLocoEditor_View::OnPaint() 
{
    CView3D::OnPaint();

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

    if( eng_Begin( "Viewer.hpp" ) )
    {
        m_Grid.Render();
        m_Axis.Render();

        eng_End();
    }

    eng_PageFlip();
    
}
