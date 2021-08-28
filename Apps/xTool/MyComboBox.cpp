// MyComboBox.cpp : implementation file
//

#include "stdafx.h"
#include "xTool.h"
#include "MyComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyComboBox

CMyComboBox::CMyComboBox()
{
}

CMyComboBox::~CMyComboBox()
{
}


BEGIN_MESSAGE_MAP(CMyComboBox, CComboBox)
	//{{AFX_MSG_MAP(CMyComboBox)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyComboBox message handlers

BOOL CMyComboBox::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if( pMsg->message == WM_KEYDOWN )
    {
        UINT nChar = pMsg->wParam;

        if( nChar == VK_RETURN )
        {
            // Generate notification
            GetParent()->SendNotifyMessage( WM_COMMAND, (CBN_ENTER<<16) | GetDlgCtrlID(), (LPARAM)GetSafeHwnd() );
            return 1;
        }
    }

	return CComboBox::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////
