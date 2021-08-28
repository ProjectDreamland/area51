// GridItemInfo.cpp : implementation file
//

#include "stdafx.h"
#include "GridItemInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CGridItemInfo

CGridItemInfo::CGridItemInfo() :
m_lParam(NULL),
m_bReadOnly(FALSE),
m_eNotify(NTFY_NONE),
m_bHeader(FALSE),
m_iDataIndex(0)
{	

}

CGridItemInfo::~CGridItemInfo()
{
	POSITION pos = m_listdata.GetStartPosition();
	while(pos != NULL)
	{
		int nKey;
		CStringList* b; 
		m_listdata.GetNextAssoc(pos, nKey, b);
		if(b!=NULL)
		{
			b->RemoveAll();
			delete b;
		}
	}
	m_listdata.RemoveAll();
}

///////////////////////////////////////////////////////////////
// text accessors
///////////////////////////////////////////////////////////////


void CGridItemInfo::SetItemText(const CString& strItem) 
{ 
	m_strItemName = strItem; 
}


void CGridItemInfo::AddSubItemText(const CString& strSubItem)
{ 
	int nIndex = m_SubItems.Add(strSubItem); 
}


void CGridItemInfo::SetSubItemText(int iSubItem, const CString& strSubItem)
{
	m_SubItems.SetAtGrow(iSubItem, strSubItem);
}

const CString& CGridItemInfo::GetItemText() 
{ 
	return m_strItemName; 
}

CString CGridItemInfo::GetSubItem(int iSubItem) 
{ 
	return m_SubItems.GetAt(iSubItem); 
}

int CGridItemInfo::GetItemCount()  
{ 
	return m_SubItems.GetSize(); 
}

////////////////////////////////////////////////////////////////
// Control handlers
////////////////////////////////////////////////////////////////

void CGridItemInfo::SetControlType(CONTROLTYPE enumCtrlType, int nCol)
{ 
	m_controlType.SetAt(nCol, enumCtrlType); 
}

BOOL CGridItemInfo::GetControlType(int nCol, CONTROLTYPE& controlType) 
{
	if(!m_controlType.Lookup(nCol,controlType))
	{
		controlType = GCT_STRING_EDIT;//default;
		return 0;
	}
	return 1;
}

///////////////////////////////////////////////////////////////
// data handlers
///////////////////////////////////////////////////////////////

void CGridItemInfo::SetListData(int iSubItem, CStringList *strInitArr)
{
	CStringList *list;

    // forcible empty the listdata
	POSITION pos = m_listdata.GetStartPosition();
	while(pos != NULL)
	{
		int nKey;
		CStringList* b; 
		m_listdata.GetNextAssoc(pos, nKey, b);
		if(b!=NULL)
		{
			b->RemoveAll();
			delete b;
		}
	}
	m_listdata.RemoveAll();

    // allocate new listdata
	list = new CStringList;		//list will be deleted in destructor
	list->AddTail(strInitArr);
	m_listdata.SetAt(iSubItem, list);
}	

BOOL CGridItemInfo::GetListData(int iSubItem, CStringList*& pList) 
{	
	return m_listdata.Lookup(iSubItem, pList);
}

///////////////////////////////////////////////////////////////
// Copy handlers 
///////////////////////////////////////////////////////////////

void CGridItemInfo::CopyObjects(CGridItemInfo* pItemInfo)
{
	SetItemText(pItemInfo->GetItemText());
	m_SubItems.Copy(pItemInfo->m_SubItems);
	CopyControls(pItemInfo);
	m_lParam = pItemInfo->m_lParam;
	m_bReadOnly = pItemInfo->m_bReadOnly;
	m_eNotify = pItemInfo->m_eNotify;
}

void CGridItemInfo::CopyControls(CGridItemInfo* pItemInfo)
{
	for(int nCol=0; nCol < pItemInfo->GetItemCount(); nCol++)
	{
		CGridItemInfo::CONTROLTYPE ctrlType;
		if(pItemInfo->GetControlType(nCol, ctrlType))//true if other than edit-control
		{
			SetControlType(ctrlType, nCol);
			//should test if this is listdata material
			CStringList *list = NULL;
			pItemInfo->GetListData(nCol, list);
			if(list!=NULL)
			{
				SetListData(nCol, list);	
			}
		}
	}
}



