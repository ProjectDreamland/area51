// GridComboBox.cpp : implementation file

#include "stdafx.h"
#include "GridComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridComboBox
CGridComboBox::CGridComboBox(int iItem, int iSubItem, CStringList *plstItems) :
m_iItem(iItem),
m_iSubItem(iSubItem),
m_bVK_ESCAPE(0)
{	
	m_lstItems.AddTail(plstItems);
}

CGridComboBox::~CGridComboBox()
{
}

BEGIN_MESSAGE_MAP(CGridComboBox, CComboBox)	
	//{{AFX_MSG_MAP(CGridComboBox)
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_SIZE()
	ON_WM_NCDESTROY()
	ON_WM_KILLFOCUS()
	ON_CONTROL_REFLECT(CBN_CLOSEUP, OnCloseup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

int CGridComboBox::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CComboBox::OnCreate(lpCreateStruct) == -1)	
	{
		return -1;	
	}

	CFont* font = GetParent()->GetFont();	
	SetFont(font);

	//add the items from CStringlist
	POSITION pos = m_lstItems.GetHeadPosition();
	while(pos != NULL)
	{
		AddString((LPCTSTR)(m_lstItems.GetNext(pos)));	
	}
	SetFocus();	

	return 0;
}

// handle the keyboard input
BOOL CGridComboBox::PreTranslateMessage(MSG* pMsg) 
{
	if( pMsg->message == WM_KEYDOWN )	
	{		
		if(pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)	
		{
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);			
			return 1;
		}	
	}	
	return CComboBox::PreTranslateMessage(pMsg);
}

//when focus is lost we need to dismiss this control
void CGridComboBox::OnKillFocus(CWnd* pNewWnd) 
{	
	int nIndex = GetCurSel();

	CComboBox::OnKillFocus(pNewWnd);

	CString str;	
	GetWindowText(str);

	// Send Notification to parent of ListCtrl	
	LV_DISPINFO lvDispinfo;
	lvDispinfo.hdr.hwndFrom = GetParent()->m_hWnd;
	lvDispinfo.hdr.idFrom = GetDlgCtrlID();
	lvDispinfo.hdr.code = LVN_ENDLABELEDIT;
	lvDispinfo.item.mask = LVIF_TEXT | LVIF_PARAM;	
	lvDispinfo.item.iItem = m_iItem;
	lvDispinfo.item.iSubItem = m_iSubItem;
	lvDispinfo.item.pszText = m_bVK_ESCAPE ? NULL : LPTSTR((LPCTSTR)str);
	lvDispinfo.item.cchTextMax = str.GetLength();
	lvDispinfo.item.lParam = GetItemData(GetCurSel());
	if(nIndex!=CB_ERR)
	{
		GetParent()->GetParent()->SendMessage(WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&lvDispinfo);
	}
	PostMessage(WM_CLOSE);
}

//need to catch the VK_ESCAPE for the notification msg
void CGridComboBox::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	if(nChar == VK_ESCAPE || nChar == VK_RETURN)	
	{		
		if( nChar == VK_ESCAPE)
		{
			m_bVK_ESCAPE = 1;
		}

		GetParent()->SetFocus();		
		return;	
	}	
	CComboBox::OnChar(nChar, nRepCnt, nFlags);
}

//we need to clean up now
void CGridComboBox::OnNcDestroy() 
{
	CComboBox::OnNcDestroy();		
	delete this;
}

void CGridComboBox::OnCloseup() 
{
	GetParent()->SetFocus();
}

void CGridComboBox::OnSize(UINT nType, int cx, int cy) 
{
	CComboBox::OnSize(nType, cx, cy);
}
