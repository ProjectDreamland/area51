
#if !defined(AFX_TREESTUFF_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_)
#define AFX_TREESTUFF_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "GridItemInfo.h"

class CGridListCtrl;

//the nested class 
class CGridTreeItem : public CObject
{
public:
    CGridTreeItem();
	~CGridTreeItem();

	// handle compound objects
	virtual void NotifyOfChildChange(CGridListCtrl *pGrid) {};
	// handle compound objects
	virtual void NotifyOfChange(CGridListCtrl *pGrid) {};

	CObList			m_listChild;
	CGridTreeItem*	m_pParent;
	CGridItemInfo*	m_lpNodeInfo;
	BOOL			m_bHideChildren;  
	int				m_nIndex; //CListCtrl index
	int				m_nIndent; 
	int				m_bSetChildFlag;

    CString         m_strIdentifier;
    CString         m_strComment;
    BOOL            m_bGridCreated;
    BOOL            m_bMustReloadData;
};

#endif // !defined(AFX_TREESTUFF_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_)
