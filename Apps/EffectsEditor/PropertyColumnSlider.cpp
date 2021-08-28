// MySliderCtl.cpp : implementation file
//

#include "stdafx.h"
#include "PropertyColumnSlider.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "resource.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertyColumnSlider

CPropertyColumnSlider::CPropertyColumnSlider()
{
	m_bDragging = FALSE;
}

CPropertyColumnSlider::~CPropertyColumnSlider()
{
}


BEGIN_MESSAGE_MAP(CPropertyColumnSlider, CSliderCtrl)
	//{{AFX_MSG_MAP(CPropertyColumnSlider)
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_KEYUP()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropertyColumnSlider message handlers

void CPropertyColumnSlider::OnMouseMove(UINT nFlags, CPoint point) 
{
	if( m_bDragging )
	{
		Invalidate(FALSE);
		CPoint ptRef = point;
		if (ptRef.x < GetRangeMin( ) ) ptRef.x = GetRangeMin( );
		if (ptRef.x > GetRangeMax( ) ) ptRef.x = GetRangeMax( ) ;
		SetPos(ptRef.x);
	}
	else
	{
		CSliderCtrl::OnMouseMove(nFlags, point);
	}
}


void CPropertyColumnSlider::OnPaint() 
{
//	CSliderCtrl::OnPaint();
	CPaintDC dc(this); // device context for painting
	CRect rect;
	GetClientRect(&rect);

	dc.FillSolidRect(rect,RGB(192,192,192));

	CRect rectThumb;
	rectThumb.left = GetPos()+2;
	rectThumb.right = GetPos() +4;
//	GetThumbRect(&rectThumb);
	rectThumb.top = rect.top;
	rectThumb.bottom = rect.bottom;

	dc.FillSolidRect(rectThumb,RGB(128,128,128));
	CBrush br(RGB(255,255,255));
	dc.FrameRect(rectThumb,&br);
}

void CPropertyColumnSlider::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if (m_bDragging)
	{
		int i = GetPos();
		GetParent()->SendMessage(WM_USER_MSG_SLIDER_MOVED,i,0);
	}
	m_bDragging = FALSE;
	Invalidate(FALSE);
	CSliderCtrl::OnLButtonUp(nFlags, point);
}

void CPropertyColumnSlider::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bDragging =TRUE;
	Invalidate(FALSE);

	CPoint ptRef = point;
	if (ptRef.x < GetRangeMin( ) ) ptRef.x = GetRangeMin( );
	if (ptRef.x > GetRangeMax( ) ) ptRef.x = GetRangeMax( ) ;
	SetPos(ptRef.x);

	CSliderCtrl::OnLButtonDown(nFlags, point);
}

void CPropertyColumnSlider::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	int i = GetPos();

	Invalidate(FALSE);

	GetParent()->SendMessage(WM_USER_MSG_SLIDER_MOVED,i,0);
	
	CSliderCtrl::OnKeyUp(nChar, nRepCnt, nFlags);
}

void CPropertyColumnSlider::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	Invalidate(FALSE);
	
	CSliderCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
