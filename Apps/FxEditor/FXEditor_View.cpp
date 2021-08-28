// CFxEditor_View.cpp : implementation file
//

#include "stdafx.h"
#include "FxEditor_View.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFxEditor_View

IMPLEMENT_DYNCREATE(CFxEditor_View, CView3D)

CFxEditor_View::CFxEditor_View()
{
}

CFxEditor_View::~CFxEditor_View()
{
}


BEGIN_MESSAGE_MAP(CFxEditor_View, CView3D)
	//{{AFX_MSG_MAP(CFxEditor_View)
		// NOTE - the ClassWizard will add and remove mapping macros here.
        ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFxEditor_View drawing

void CFxEditor_View::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CFxEditor_View diagnostics

#ifdef _DEBUG
void CFxEditor_View::AssertValid() const
{
	CView3D::AssertValid();
}

void CFxEditor_View::Dump(CDumpContext& dc) const
{
	CView3D::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFxEditor_View message handlers
void CFxEditor_View::OnPaint() 
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
