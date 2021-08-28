// GridTreeItem.cpp : implementation file
//

#include "stdafx.h"
#include "GridTreeItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CGridTreeItem::CGridTreeItem(): 
m_pParent(NULL), 
m_bHideChildren(0), 
m_nIndex(-1), 
m_nIndent(-1),
m_bSetChildFlag(-1),
m_bGridCreated(FALSE),
m_bMustReloadData(FALSE)
{
    m_BackgroundColor   = RGB(176,176,176);
}


CGridTreeItem::~CGridTreeItem()
{
	// delete child nodes
	POSITION pos = m_listChild.GetHeadPosition();
	while (pos != NULL)
	{
		CGridTreeItem* pItem = (CGridTreeItem*)m_listChild.GetNext(pos);
		if(pItem!=NULL)
		{
			if(pItem->m_lpNodeInfo!=NULL)
				delete pItem->m_lpNodeInfo;
			delete pItem;
		}
	}
	m_listChild.RemoveAll();

    m_strIdentifier.Empty();
}
