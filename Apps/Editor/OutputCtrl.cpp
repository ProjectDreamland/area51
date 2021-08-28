// OutputCtrl.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "OutputCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COutputCtrl

COutputCtrl::COutputCtrl()
{
}

COutputCtrl::~COutputCtrl()
{
}


BEGIN_MESSAGE_MAP(COutputCtrl, CRichEditCtrl)
	//{{AFX_MSG_MAP(COutputCtrl)
	ON_WM_RBUTTONDOWN()
    ON_COMMAND(ID_EDIT_COPY, OnCopy)  
    ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateCopy)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COutputCtrl message handlers

void COutputCtrl::OnRButtonDown(UINT nFlags, CPoint point) 
{
	CRichEditCtrl::OnRButtonDown(nFlags, point);

	CXTMenu menu;
    menu.CreatePopupMenu();

    menu.AppendMenu(MF_STRING|MF_ENABLED, ID_EDIT_COPY, "Copy");

	CPoint pt;
	GetCursorPos( &pt );
	menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this);
}

//=========================================================================

void COutputCtrl::OnCopy() 
{
	CString source = GetSelText(); 

	//put your text in source
	if(OpenClipboard())
	{
		HGLOBAL clipbuffer;
		char * buffer;
		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, source.GetLength()+1);
		buffer = (char*)GlobalLock(clipbuffer);
		strcpy(buffer, LPCSTR(source));
		GlobalUnlock(clipbuffer);
		SetClipboardData(CF_TEXT,clipbuffer);
		CloseClipboard();
	}

}

//=========================================================================

void COutputCtrl::OnUpdateCopy(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(TRUE);
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

int COutputCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CRichEditCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}
