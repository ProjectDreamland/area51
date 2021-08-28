// CMyMDIWndTab : implementation file
/////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "Resource.h"
#include "MyMDIWndTab.h"

/////////////////////////////////////////////////////////////////////////////
// CMyMDIWndTab

CMyMDIWndTab::CMyMDIWndTab()
{
	// TODO: add construction code here.

}

CMyMDIWndTab::~CMyMDIWndTab()
{
	// TODO: add destruction code here.

}

void CMyMDIWndTab::OnAddPadding(CXTString& strLabelText)
{
    int Index = strLabelText.ReverseFind( ':' );
    if( Index != -1 )
        strLabelText = strLabelText.Left( Index );
    CXTTabCtrlBase::OnAddPadding( strLabelText );
}

/////////////////////////////////////////////////////////////////////////////
