// PropertyGrid.cpp : implementation file
//

#include "stdafx.h"
#include "PropertyGrid.h"
#include "resource.h"
#include "PropertyEditCtrl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPropertyGrid

#define EDITABLE_COL		0


CPropertyGrid::CPropertyGrid() :
m_pToolTip(NULL)
{
    m_ptHover = CPoint(0,0);
}

CPropertyGrid::~CPropertyGrid()
{
    if (m_pToolTip)
 	    delete m_pToolTip;
}


BEGIN_MESSAGE_MAP(CPropertyGrid, CGridListCtrl)
	//{{AFX_MSG_MAP(CPropertyGrid)
	ON_WM_LBUTTONDOWN()
	ON_WM_SIZE()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
    ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL CPropertyGrid::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) 
{
    Scroll(CSize(0,zDelta/10));

	return CGridListCtrl::OnMouseWheel(nFlags, zDelta, pt);
}

void CPropertyGrid::SaveStructure()
{
    m_strSaveSelectedID = "";
	int nIndexSel = GetNextItem(-1, LVNI_ALL | LVNI_SELECTED); 
	if(nIndexSel!=-1)
	{
        //get first selected item
        CGridTreeItem *pItem = GetTreeItem(nIndexSel);
        if (pItem)
        {
            while (pItem->m_strIdentifier.IsEmpty())
            {
                //hmmm must get parent identifier
                pItem = pItem->m_pParent;
            }

            m_strSaveSelectedID = pItem->m_strIdentifier;
        }
    }

    //save visible RANGE
    m_ptSavedPosition = CPoint(0,0);
    if (GetCount()>0)
    {
        GetItemPosition(0,&m_ptSavedPosition);
    }

    m_lstVisibleItems.Clear();

	int nIndex = GetNextItem(-1, LVNI_ALL); 
	if(nIndex!=-1)
	{
		//GetHeadPosition
		POSITION pos = GetRootHeadPosition();
		while(pos != NULL)
		{
			CGridTreeItem *pParent =(CGridTreeItem*)GetNextRoot(pos); 
			CGridTreeItem *pItem = pParent;

            //don't add roots, since roots autoexpand
            //if (ItemHasChildren(pItem) && !IsCollapsed(pItem))
            //{
            //    m_lstVisibleItems.Append(pItem->m_strIdentifier);
            //}

			//GetNext ....loop through all children 
			for(;;)
			{
				CGridTreeItem *pCur = GetNext(pParent,pItem, TRUE, FALSE/*regardless of the item are hidden or not*/);	  

				if(!IsChildOf(pParent, pCur))
					break;
				else
				if(pCur==pItem)
					break;
				
				pItem=pCur;

                //add roots?
                if (ItemHasChildren(pItem) && !IsCollapsed(pItem))
                {
                    if (!pItem->m_strIdentifier.IsEmpty())
                    {
                        m_lstVisibleItems.Append(pItem->m_strIdentifier);
                    }
                }
			}
        }
    }
}

void CPropertyGrid::RestoreSavedSelection()
{
//    SetRedraw(FALSE);
    for (int i=0; i<m_lstVisibleItems.GetCount(); i++)
    {
        CGridTreeItem *pTreeItem = FindTreeItemWithIdentifier(m_lstVisibleItems.GetAt(i));
        if (pTreeItem)
        {
			Expand(pTreeItem, pTreeItem->m_nIndex);
        }
    }
//    SetRedraw(TRUE);

    //recall visible RANGE
    if (GetCount()>0)
    {
        Scroll(CSize(0,-m_ptSavedPosition.y));
    }

    //select item
    CGridTreeItem *pTreeItemSelected = FindTreeItemWithIdentifier(m_strSaveSelectedID);
    if (pTreeItemSelected) SelectNode(pTreeItemSelected);

//    Invalidate();
}

BOOL CPropertyGrid::ShouldItemExpand(CString strIdentifier)
{
    for (int i=0; i<m_lstVisibleItems.GetCount(); i++)
    {
        CString strItem = m_lstVisibleItems.GetAt(i);
        if (strItem.CompareNoCase(strIdentifier) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyGrid message handlers

void CPropertyGrid::OnMouseMove(UINT nFlags, CPoint point) 
{
    if ((abs(m_ptHover.x-point.x) > 2) &&
        (abs(m_ptHover.y-point.y) > 2))
    {
        TRACKMOUSEEVENT tme;
        tme.cbSize = sizeof (tme);
        tme.dwFlags = TME_HOVER | TME_LEAVE;
        tme.hwndTrack = m_hWnd;
        //tme.dwHoverTime = HOVER_DEFAULT;
	    tme.dwHoverTime = 0;
        _TrackMouseEvent (&tme);
    }

	CGridListCtrl::OnMouseMove(nFlags, point);
}

LRESULT CPropertyGrid::OnMouseHover(WPARAM w, LPARAM l)
{
	/* This line corrects a problem with the tooltips not displaying when 
	the mouse passes over them, if the parent window has not been clicked yet.
	Normally this isn't an issue, but when developing multi-windowed apps, this 
	bug would appear. Setting the ActiveWindow to the parent is a solution to that.
	*/
	::SetActiveWindow(GetParent()->GetSafeHwnd());

	if (m_pToolTip != NULL)
	{
		delete m_pToolTip;
		m_pToolTip = NULL;
	}

	LVHITTESTINFO ht;
    GetCursorPos(&(ht.pt));
    ScreenToClient(&(ht.pt));
	SubItemHitTest(&ht);

	if(ht.iItem!=-1)
	{
        int iCount = GetCount();
        if (iCount > ht.iItem)
        {
		    CGridTreeItem* pItem = GetTreeItem(ht.iItem);
		    if(pItem)
		    {
    		    CGridItemInfo* lp = GetData(pItem);
                if (lp)
                {
                    //set hover point
                    GetCursorPos(&m_ptHover);
                    ScreenToClient(&m_ptHover);

                    CString strText = lp->GetSubItem(EDITABLE_COL);
		        
                    // Create a new Tooltip 
	                if (m_pToolTip == NULL)
	                {
		                m_pToolTip = new CToolTipCtrl;
		                // Create ToolTip control
		                m_pToolTip->Create(this);
	                }

	                // If there is no tooltip defined then add it
	                if (m_pToolTip->GetToolCount() == 0)
	                {
		                CRect rect; 
		                GetClientRect(rect);
		                m_pToolTip->AddTool(this, strText, rect, 1);
	                }

	                // Set text for tooltip
	                m_pToolTip->Activate(TRUE);
                    m_pToolTip->SetDelayTime( TTDT_AUTOPOP, 5000 );

	                if (m_pToolTip != NULL)
                    {
			            //Display ToolTip
			            m_pToolTip->Update();
                    }
                }
            }
        }
    }

	Invalidate();

    return 0;
}

LRESULT CPropertyGrid::OnMouseLeave(WPARAM w, LPARAM l)
{
	if (m_pToolTip != NULL)
	{
		delete m_pToolTip;
		m_pToolTip = NULL;
	}

    return 0;
}


void CPropertyGrid::InitializeGrid()
{
	ModifyStyle(0,LVS_NOCOLUMNHEADER);

	LPTSTR lpszCols[] = {_T("Property"),_T("Value"),0};
	LV_COLUMN   lvColumn;
	//initialize the columns
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 100;
	for(int x = 0; lpszCols[x]!=NULL; x++)
    {
		lvColumn.pszText = lpszCols[x];
		InsertColumn(x,&lvColumn);
    }
}


BOOL CPropertyGrid::OnUpdateItem(CGridTreeItem* lpItem)
{
	CGridItemInfo *lp = GetData(lpItem);
	if(lp!=NULL)
	{
	    SetItemText(lpItem->m_nIndex, 0, lp->GetItemText()); //name
	    SetItemText(lpItem->m_nIndex, 1, lp->GetSubItem(0)); //value
      	//now carry back the proper values
    	lpItem->NotifyOfChange(this);

        Invalidate();
	}

    return TRUE;
}

//browse button handler
void CPropertyGrid::OnButtonBrowse(int iItem, int iSubItem)
{
	CGridTreeItem* pItem = GetTreeItem(iItem);
	CGridItemInfo* lpInfo = GetData(pItem);
    CStringList *pstrList;
    lpInfo->GetListData(EDITABLE_COL,pstrList);
    CString strFilter;
    if (pstrList && pstrList->GetCount()>0) 
    {
	     strFilter = pstrList->GetAt(pstrList->FindIndex(0));
    }

    CString strFileName = lpInfo->GetSubItem(EDITABLE_COL);
	CFileDialog dlgBrowse(TRUE,NULL,strFileName,OFN_HIDEREADONLY,strFilter);

	if (dlgBrowse.DoModal() == IDOK)
	{
		CString strPath = dlgBrowse.GetPathName( );
		SetItemText(iItem, iSubItem, strPath);

		CGridTreeItem* pItem = GetTreeItem(iItem);
		CGridItemInfo* lp = GetData(pItem);
		lp->SetSubItemText(iSubItem-1, strPath);

		OnGridItemChange(pItem);
	}
}

void CPropertyGrid::OnButtonColorPick(int iItem, int iSubItem)
{
	CPropertyTreeColor *pColor = (CPropertyTreeColor*) GetTreeItem(iItem);

	CXTColorDialog dlgColor(pColor->m_cr, pColor->m_cr,0,this);
	if (dlgColor.DoModal() == IDOK)
	{
		CString strColor;
		COLORREF color = dlgColor.GetColor();
		strColor.Format("%u, %u, %u, %u",GetRValue(color), GetGValue(color), GetBValue(color), pColor->m_alpha);
		SetItemText(iItem, iSubItem, strColor);

		CGridItemInfo* lp = GetData(pColor);
		lp->SetSubItemText(iSubItem-1, strColor);

		pColor->NotifyOfChange(this);
		OnGridItemChange(pColor);
	}
}

void CPropertyGrid::DrawControl(CDC* pDC, CGridTreeItem *pSelItem, int nItem, int nColumn, COLORREF crBorder)
{
	CGridItemInfo* pInfo = GetData(pSelItem);
	CGridItemInfo::CONTROLTYPE ctrlType;
	if(pInfo->GetControlType(nColumn-1, ctrlType))
	{
		if (ctrlType == pInfo->CONTROLTYPE::GCT_COLOR_BUTTON)
		{
			CRect rect;
			GetSubItemRect(nItem, nColumn, LVIR_BOUNDS, rect);
			rect.left=rect.right - GetSystemMetrics(SM_CYVSCROLL);
			CPropertyTreeColor *pColor = (CPropertyTreeColor*) pSelItem;
			pDC->FillSolidRect(rect,pColor->m_cr);
			CRect rcNew = rect;
			rcNew.left = rcNew.right - rcNew.Width()/3;
			pDC->FillSolidRect(rcNew,RGB(pColor->m_alpha,pColor->m_alpha,pColor->m_alpha));
			CBrush br(crBorder);
			pDC->FrameRect(rect,&br);
			return;
		}
	}
	CGridListCtrl::DrawControl(pDC, pSelItem, nItem, nColumn, crBorder);
}

void CPropertyGrid::SetFirstColWidth(int iWidth)
{
	SetColumnWidth(0,iWidth);
	RecalcSize();
}

int CPropertyGrid::GetFirstColWidth()
{
	return GetColumnWidth(0);
}

void CPropertyGrid::OnSize(UINT nType, int cx, int cy) 
{
    CGridListCtrl::OnSize(nType, cx, cy);
/*  this code below was the cause of the scroll problem...
    we now do this from the parent

	if (nType!= SIZE_MINIMIZED)
	{
        SetRedraw(FALSE);
        RecalcSize();
        SetRedraw(TRUE);
    }
*/
    // for vertical scroll on
    ShowScrollBar( SB_VERT, TRUE );

    //turn off horizontal scroll
    ShowScrollBar( SB_HORZ, FALSE );
}

void CPropertyGrid::RecalcSize()
{
	if (GetSafeHwnd())
	{
		CRect rect;
		GetClientRect(&rect);

		int nWidth = rect.Width() - GetColumnWidth(0);
		SetColumnWidth(1,nWidth);

        //turn off horizontal scroll
		ShowScrollBar( SB_HORZ, FALSE );
	}
}

//insert item and return new parent pointer.
CGridTreeItem* CPropertyGrid::InsertItem(CGridTreeItem *pParent, CGridItemInfo* lpInfo,  BOOL bUpdate)
{
	if(pParent==NULL)
		return NULL;

	CGridTreeItem *pItem = NULL;
	CGridItemInfo::CONTROLTYPE ctrlType;
	if (lpInfo && lpInfo->GetControlType(EDITABLE_COL, ctrlType))
	{
		switch(ctrlType)
		{
			case CGridItemInfo::GCT_COLOR_BUTTON:
				{
					lpInfo->SetNotification(CGridItemInfo::NTFY_PARENT);
					CPropertyTreeColor *pItemColor = new CPropertyTreeColor();
					pItemColor->ParseColor(lpInfo->GetSubItem(EDITABLE_COL));
					pItem = (CGridTreeItem*) pItemColor;
				}
				break;
            case CGridItemInfo::GCT_2D_COORDINATE:
                {
					lpInfo->SetNotification(CGridItemInfo::NTFY_PARENT);
					CPropertyTree2DPos *pItemPos = new CPropertyTree2DPos();
					pItemPos->ParsePos(lpInfo->GetSubItem(EDITABLE_COL));
					pItem = (CGridTreeItem*) pItemPos;
                }
                break;
			case CGridItemInfo::GCT_3D_COORDINATE:
				{
					lpInfo->SetNotification(CGridItemInfo::NTFY_PARENT);
					CPropertyTree3DPos *pItemPos = new CPropertyTree3DPos();
					pItemPos->ParsePos(lpInfo->GetSubItem(EDITABLE_COL));
					pItem = (CGridTreeItem*) pItemPos;
				}
				break;
			case CGridItemInfo::GCT_ROTATION_EDIT:
				{
					lpInfo->SetNotification(CGridItemInfo::NTFY_PARENT);
					CPropertyTreeRotation *pItemRot = new CPropertyTreeRotation();
					pItemRot->ParseRot(lpInfo->GetSubItem(EDITABLE_COL));
					pItem = (CGridTreeItem*) pItemRot;
				}
				break;
			case CGridItemInfo::GCT_BOUNDING_BOX:
				{
					lpInfo->SetNotification(CGridItemInfo::NTFY_PARENT);
					CPropertyTreeBoundingBox *pItemBox = new CPropertyTreeBoundingBox();
					pItem = (CGridTreeItem*) pItemBox;
				}
				break;
		}
	}

	if (!pItem)
		pItem =  new CGridTreeItem();


	if(lpInfo==NULL)
		lpInfo = new CGridItemInfo;

	UpdateData(pItem, lpInfo);
	SetIndent(pItem, GetIndent(pParent)+1);
	SetParentItem(pItem, pParent);
	//add as the last child 
	pParent->m_listChild.AddTail(pItem);

	if(!bUpdate)
		 Hide(pParent, TRUE);	
	else
	{
		//calc listview index for the new node
		int nIndex = NodeToIndex(pItem);			
		CString str = GetData(pItem)->GetItemText();
		LV_ITEM     lvItem;
		lvItem.mask = LVIF_TEXT | LVIF_INDENT | LVIF_PARAM;
		lvItem.pszText = str.GetBuffer(1); 
		//insert item
		lvItem.iItem = nIndex;
		lvItem.iSubItem = 0;
		lvItem.lParam = (LPARAM)pItem;
		lvItem.iIndent = GetIndent(pItem);
		CListCtrl::InsertItem(&lvItem);

		//Get subitems
		int nSize = GetData(pItem)->GetItemCount();
		for(int i=0; i < nSize;i++)
		{
		   CString str = GetData(pItem)->GetSubItem(i);
		   lvItem.mask = LVIF_TEXT;
		   lvItem.iSubItem = i+1;
		   lvItem.pszText = str.GetBuffer(1);
		   SetItem(&lvItem);
		}
		InternaleUpdateTree();//better do this
	}

	//now add children for compound objects
	if (lpInfo && lpInfo->GetControlType(EDITABLE_COL, ctrlType))
	{
        CGridTreeItem *pNewItem = NULL;
		switch(ctrlType)
		{
			case CGridItemInfo::GCT_COLOR_BUTTON:
				{
					CPropertyTreeColor *pItemColor = (CPropertyTreeColor*) pItem;

					CString strVal;
					CGridItemInfo* lp = new CGridItemInfo();
					lp->SetItemText(_T("Red"));
					strVal.Format("%d",GetRValue(pItemColor->m_cr));
					lp->AddSubItemText(strVal);// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_NUMERIC_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					lp = new CGridItemInfo();
					lp->SetItemText(_T("Green"));
					strVal.Format("%d",GetGValue(pItemColor->m_cr));
					lp->AddSubItemText(strVal);// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_NUMERIC_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					lp = new CGridItemInfo();
					lp->SetItemText(_T("Blue"));
					strVal.Format("%d",GetBValue(pItemColor->m_cr));
					lp->AddSubItemText(strVal);// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_NUMERIC_EDIT, EDITABLE_COL);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					lp = new CGridItemInfo();
					lp->SetItemText(_T("Alpha"));
					strVal.Format("%d",pItemColor->m_alpha);
					lp->AddSubItemText(strVal);// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_NUMERIC_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					pItemColor->NotifyOfChange(this);
                    Hide(pItemColor,TRUE);
				}
				break;
			case CGridItemInfo::GCT_BOUNDING_BOX:
				{
					CPropertyTreeBoundingBox *pItemBox = (CPropertyTreeBoundingBox*) pItem;

					CStringList *pstrList;
					lpInfo->GetListData(EDITABLE_COL,pstrList);
					if (pstrList && pstrList->GetCount()==2) //two items
					{
						CGridItemInfo* lp = new CGridItemInfo();
						lp->SetItemText(_T("Min"));
						lp->AddSubItemText(pstrList->GetAt(pstrList->FindIndex(0)));// 1
						lp->SetControlType(lp->CONTROLTYPE::GCT_3D_COORDINATE, EDITABLE_COL);
						lp->SetReadOnly(lpInfo->IsReadOnly());

			    		pNewItem = InsertItem(pItem, lp);
                        pNewItem->m_bGridCreated = TRUE;
						pItemBox->m_pVectorMin = (CPropertyTree3DPos*)pNewItem;

                        //override notification type
						lp->SetNotification(CGridItemInfo::NTFY_BOTH);

						lp = new CGridItemInfo();
						lp->SetItemText(_T("Max"));
						lp->AddSubItemText(pstrList->GetAt(pstrList->FindIndex(1)));// 1
						lp->SetControlType(lp->CONTROLTYPE::GCT_3D_COORDINATE, EDITABLE_COL);
						lp->SetReadOnly(lpInfo->IsReadOnly());

			    		pNewItem = InsertItem(pItem, lp);
                        pNewItem->m_bGridCreated = TRUE;
						pItemBox->m_pVectorMax = (CPropertyTree3DPos*)pNewItem;

                        //override notification type
						lp->SetNotification(CGridItemInfo::NTFY_BOTH);
					}
					else ASSERT(FALSE);

					pItemBox->NotifyOfChildChange(this);
					pItemBox->CalcExtents(this);

                    Hide(pItemBox,TRUE);
				}
				break;
			case CGridItemInfo::GCT_2D_COORDINATE:
				{
					CPropertyTree2DPos *pItemPos = (CPropertyTree2DPos*) pItem;

					CGridItemInfo* lp = new CGridItemInfo();
					lp->SetItemText(_T("X"));
					lp->AddSubItemText("0.000000");// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_FLOAT_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					lp = new CGridItemInfo();
					lp->SetItemText(_T("Y"));
					lp->AddSubItemText("0.000000");// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_FLOAT_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					pItemPos->NotifyOfChange(this);
                    Hide(pItemPos,TRUE);
				}
				break;
			case CGridItemInfo::GCT_3D_COORDINATE:
				{
					CPropertyTree3DPos *pItemPos = (CPropertyTree3DPos*) pItem;

					CGridItemInfo* lp = new CGridItemInfo();
					lp->SetItemText(_T("X"));
					lp->AddSubItemText("0.000000");// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_FLOAT_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					lp = new CGridItemInfo();
					lp->SetItemText(_T("Y"));
					lp->AddSubItemText("0.000000");// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_FLOAT_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					lp = new CGridItemInfo();
					lp->SetItemText(_T("Z"));
					lp->AddSubItemText("0.000000");// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_FLOAT_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					pItemPos->NotifyOfChange(this);
                    Hide(pItemPos,TRUE);
				}
				break;
			case CGridItemInfo::GCT_ROTATION_EDIT:
				{
					CPropertyTreeRotation *pItemRot = (CPropertyTreeRotation*) pItem;

					CGridItemInfo* lp = new CGridItemInfo();
					lp->SetItemText(_T("Roll"));
					lp->AddSubItemText("0.00");// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_DEGREE_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					lp = new CGridItemInfo();
					lp->SetItemText(_T("Pitch"));
					lp->AddSubItemText("0.00");// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_DEGREE_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					lp = new CGridItemInfo();
					lp->SetItemText(_T("Yaw"));
					lp->AddSubItemText("0.00");// 1
					lp->SetControlType(lp->CONTROLTYPE::GCT_DEGREE_EDIT, EDITABLE_COL);
					lp->SetNotification(CGridItemInfo::NTFY_CHILD);
					lp->SetReadOnly(lpInfo->IsReadOnly());
					pNewItem = InsertItem(pItem, lp);
                    pNewItem->m_bGridCreated = TRUE;

					pItemRot->NotifyOfChange(this);
                    Hide(pItemRot,TRUE);
				}
				break;
		}
	}

	return pItem;
}	

//helper function to copy CGridItemInfo used when drag/drop you must override this this function to suit your own CGridItemInfo class
CGridItemInfo* CPropertyGrid::CopyData(CGridItemInfo* lpSrc)
{
	ASSERT(lpSrc!=NULL);
	CGridItemInfo* lpDest = new CGridItemInfo;
	//well okay I put all the copy thing in one function, located in CGridItemInfo class, 
	//so you should check out this function, remember to modify this function each time you add new data to CGridItemInfo class.
	lpDest->CopyObjects(lpSrc);
	return lpDest;
}

CEdit* CPropertyGrid::CreateEditCtrl(int nItem, int nCol, CString strLabel)
{
	CGridItemInfo* pInfo = NULL;
	CGridTreeItem*pSelItem = GetTreeItem(nItem);
	CGridItemInfo::CONTROLTYPE ctrlType = CGridItemInfo::GCT_STRING_EDIT;

	CPropertyEditCtrl* pEdit = new CPropertyEditCtrl(nItem, nCol, strLabel);
	if(pSelItem!=NULL)
	{	
		pInfo = GetData(pSelItem);
		if(pInfo && pEdit)
		{	
			pInfo->GetControlType(EDITABLE_COL, ctrlType);
			switch(ctrlType)
			{
				case pInfo->CONTROLTYPE::GCT_FLOAT_EDIT:
					pEdit->SetType(CPropertyEditCtrl::PET_FLOAT);
					break;
				case pInfo->CONTROLTYPE::GCT_COLOR_BUTTON:
					pEdit->SetType(CPropertyEditCtrl::PET_INT_LIST, 4);
					pEdit->SetMinMax(0,255);
					break;
				case pInfo->CONTROLTYPE::GCT_ROTATION_EDIT:
					pEdit->SetType(CPropertyEditCtrl::PET_DEGREE_LIST, 3);
					pEdit->SetMinMax(0,360);
					break;
				case pInfo->CONTROLTYPE::GCT_2D_COORDINATE:
					pEdit->SetType(CPropertyEditCtrl::PET_FLOAT_LIST, 2);
					break;
				case pInfo->CONTROLTYPE::GCT_3D_COORDINATE:
					pEdit->SetType(CPropertyEditCtrl::PET_FLOAT_LIST, 3);
					break;
				case pInfo->CONTROLTYPE::GCT_NUMERIC_EDIT:
					pEdit->SetType(CPropertyEditCtrl::PET_INT);
					break;
				case pInfo->CONTROLTYPE::GCT_DEGREE_EDIT:
					pEdit->SetType(CPropertyEditCtrl::PET_DEGREE);
					break;
                case pInfo->CONTROLTYPE::GCT_BOOL:
                case pInfo->CONTROLTYPE::GCT_NULL_ENTRY:
				case pInfo->CONTROLTYPE::GCT_BOUNDING_BOX:
					ASSERT(FALSE);
					delete pEdit;
					break;
				default:
					break;
			}
		}
	}	
	
	return pEdit;
}

//override
BOOL CPropertyGrid::OnUpdateListViewItem(CGridTreeItem* lpItem, LV_ITEM *plvItem)
{
	//put some extra validation here 
	CGridItemInfo *lp = GetData(lpItem);
    CString strOldText = lp->GetSubItem(EDITABLE_COL);
    CString strNewText = (CString)plvItem->pszText;
	if(strNewText.Compare(strOldText)!=0)
	{
        //text has changed
    	return CGridListCtrl::OnUpdateListViewItem(lpItem, plvItem);
    }
    return FALSE;
}

// we want to also not edit the second column for this example
void CPropertyGrid::OnControlLButtonDown(UINT nFlags, CPoint point, LVHITTESTINFO& ht)
{
//	if(ht.iSubItem>1)
//	{
		CGridListCtrl::OnControlLButtonDown(nFlags, point, ht);
//	}
}

// we want to also not edit the second column for the headers
BOOL CPropertyGrid::OnVkReturn()
{
	BOOL bResult=FALSE;

	int iItem = GetNextItem( -1, LVNI_ALL | LVNI_SELECTED);
	CGridTreeItem*pSelItem = GetTreeItem(iItem);
	if( GetCurSubItem() == 1 && (pSelItem->m_pParent == NULL))
	{
		bResult = FALSE;
	}
	else
	{
		bResult=CGridListCtrl::OnVkReturn();
	}
	return bResult;
}

BOOL CPropertyGrid::OnItemLButtonDown(LVHITTESTINFO& ht)
{
	return 1;
}

BOOL CPropertyGrid::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bReturn = CGridListCtrl::PreTranslateMessage(pMsg);

	if(pMsg->message == WM_KEYDOWN)
	{
		if(GetFocus()==this)
		{
			switch( pMsg->wParam )
			{
				case VK_UP:
				case VK_DOWN:
					{
						m_CurSubItem = EDITABLE_COL+1; //always have selection on 2nd column
						int iItem = GetSelectedItem();
						if( iItem != -1 )
						{
							CRect rcBounds;
							GetItemRect(iItem, rcBounds, LVIR_BOUNDS);
							InvalidateRect(&rcBounds);
							UpdateWindow();
						}
					}
					break;
				case VK_LEFT:
					{
						m_CurSubItem = EDITABLE_COL+1; //always have selection on 2nd column
						int iItem = GetSelectedItem();
						if( iItem != -1 )
						{
							CWaitCursor wait;
							CGridTreeItem *pSelItem = GetTreeItem(iItem);
							if(OnVkSubTract(pSelItem, iItem))
							{
								Collapse(pSelItem);
							}
							CRect rc;
							GetItemRect(iItem, rc, LVIR_BOUNDS);
							InvalidateRect(rc);
							UpdateWindow();
						}
						bReturn = TRUE;
					}
					break;
				case VK_RIGHT:
					{
						m_CurSubItem = EDITABLE_COL+1; //always have selection on 2nd column
						int iItem = GetSelectedItem();
						if( iItem != -1 )
						{
							CWaitCursor wait;
							CGridTreeItem *pSelItem = GetTreeItem(iItem);
							int nScrollIndex = 0;
							if(OnVKAdd(pSelItem, iItem))
							{
								 nScrollIndex = Expand(pSelItem, iItem);
							}
							CRect rc;
							GetItemRect(iItem, rc, LVIR_BOUNDS);
							InvalidateRect(rc);
							UpdateWindow();
							EnsureVisible(nScrollIndex, 1);
						}
						bReturn = TRUE;
					}
					break;
				default:
					break;
			}
		}
	}

	if (m_pToolTip != NULL)
    {
		m_pToolTip->RelayEvent(pMsg);		
    }

	return bReturn;
}

BOOL CPropertyGrid::OnItemExpanded(CGridTreeItem* pItem, int iItem)
{
	RecalcSize();
	return 1;
}

BOOL CPropertyGrid::OnItemCollapsed(CGridTreeItem *pItem)
{
	RecalcSize();
	return 1;
}

void CPropertyGrid::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if( GetFocus() != this) 
		SetFocus();

    if( HitTestOnSplitter( point ) )
    {
        m_SplitterDragStartWidth = GetColumnWidth( 0 );
        m_SplitterDragStartPoint = point;
        m_bSplitterDrag = true;
        SetCapture();
        return;
    }

	LVHITTESTINFO ht;
	ht.pt = point;
	SubItemHitTest(&ht);
	if(OnItemLButtonDown(ht))
	{
		BOOL bSelect=1;
		bSelect = HitTestOnSign(point, ht);
		if(bSelect && ht.iItem!=-1)
		{
			m_CurSubItem = EDITABLE_COL+1; //always have selection on 2nd column
			CHeaderCtrl* pHeader = GetHeaderCtrl();
			// Make the column fully visible.
			MakeColumnVisible(Header_OrderToIndex(pHeader->m_hWnd, m_CurSubItem));
			CListCtrl::OnLButtonDown(nFlags, point);

			if(ht.iItem!=-1)
			{
				CGridTreeItem*pSelItem = GetTreeItem(ht.iItem);
				if(pSelItem!=NULL)
				{	
					CGridItemInfo* pInfo = GetData(pSelItem);
					if ((ht.iSubItem==1) && (pSelItem->m_pParent == NULL))
					{
						//no editing heading tree items
					}
					else
					{
						OnControlLButtonDown(nFlags, point, ht);
					}
				}
			}
			//update row anyway for selection bar
			CRect rc;
			GetItemRect(ht.iItem, rc, LVIR_BOUNDS);
			InvalidateRect(rc);
			UpdateWindow();
		}
	}
}

//this is just one way to search items...strItem must match and then all subitems must be
//a match before returning the node
//the search function here search all nodes regardless if collapsed or expanded
CGridTreeItem* CPropertyGrid::Search(CString strItem,...)
{
	if(!GetItemCount())
		return NULL;

	va_list args;
	va_start(args, strItem);
	int nCount=0;
	
	for(;;)
	{
		LPCTSTR lpsz = va_arg(args, LPCTSTR);
		if(lpsz==NULL)
			break;
	   nCount++;
	}


	POSITION pos = GetRootHeadPosition();
	while(pos != NULL)
	{
		CGridTreeItem *pParent = (CGridTreeItem*)GetNextRoot(pos); 
		CGridTreeItem *pItem = pParent;
		CGridItemInfo* lp = GetData(pParent);
		CString strData = lp->GetItemText();
		if(strData==strItem)//must be a match before going further...suit you self
		{
			va_start(args, strItem);
			int nResult=0;
			for(int i=0; i<nCount;i++)
			{
				LPCTSTR lpsz = va_arg(args, LPCTSTR);   
				for(int nCol=0; nCol < lp->GetItemCount(); nCol++)
				{
					CString str = lp->GetSubItem(nCol);
					if(!str.CompareNoCase(lpsz))
					{
						nResult++;
						break;
					}
				}	
			}
			va_end(args);
			if(nCount==nResult)//all args was found return node
				return pParent;
		}

		//GetNext ....loop through all children 
		for(;;)
		{
			CGridTreeItem *pCur = GetNext(pParent, pItem, TRUE, FALSE/*regardless of the item are hidden or not, set to TRUE if only visible items must be included in the search*/);	  
			if(!IsChildOf(pParent, pCur))
				break;
			else
			if(pCur==pItem)
				break;
			CGridItemInfo* lp = GetData(pCur);
			CString strData = lp->GetItemText();
			if(strData==strItem)//must be a match before going further
			{
				va_start(args, strItem);
				int nResult=0;
				for(int i=0; i<nCount;i++)
				{
					LPCTSTR lpsz = va_arg(args, LPCTSTR);   
					for(int nCol=0; nCol < lp->GetItemCount(); nCol++)
					{
						CString str = lp->GetSubItem(nCol);
						if(!str.CompareNoCase(lpsz))
						{
							nResult++;
							break;
						}
					}	
				}
				va_end(args);
				if(nCount==nResult)//all args was found return node
					return pCur;
			}
			pItem=pCur;//next;
		}
	}	
	return NULL;
}

//another search thing
CGridTreeItem* CPropertyGrid::SearchEx(CGridTreeItem *pStartPosition, CString strItem)
{
	CGridItemInfo* lp = GetData(pStartPosition);
	//if(lp->GetCheck()) another condition here maybe
	CString strData = lp->GetItemText();
	if(strData==strItem)
	{
		return pStartPosition;
	}

	const int nChildren = NumChildren(pStartPosition);
	if (nChildren > 0)
	{
		POSITION pos = GetHeadPosition(pStartPosition);
		while (pos)
		{
			CGridTreeItem *pChild = GetNextChild(pStartPosition, pos);
			CGridItemInfo* lp = GetData(pChild);
			CString strData = lp->GetItemText();
			if(strData==strItem)
			{
				return pChild;
			}
		}
	}

	POSITION pos = GetHeadPosition(pStartPosition);
	while (pos)
	{
		CGridTreeItem *pChild = GetNextChild(pStartPosition, pos);
		CGridItemInfo* lp = GetData(pChild);
		CString strData = lp->GetItemText();
		if(strData==strItem)
		{
			return pChild;
		}

		pChild = SearchEx(pChild, strItem);
		if(pChild!=NULL)
			return pChild;
	}
	return NULL;
}

CGridTreeItem* CPropertyGrid::GetRootItemWithIdentifier(CString strIdentifier)
{
	POSITION pos = GetRootHeadPosition();
	while(pos != NULL)
	{
		CGridTreeItem *pItem = (CGridTreeItem*)GetNextRoot(pos); 
        if (pItem && strIdentifier.Compare(pItem->m_strIdentifier)==0)
        {
            return pItem;
        }
    }

    return NULL;
}

CGridTreeItem* CPropertyGrid::FindTreeItemWithIdentifier(CString strIdentifier)
{
    //strIdentifier is a hierarchtical string ie "root\subitem\subitem"
    int iIndex = strIdentifier.Find('\\');
    CString strRoot;
    if (iIndex == -1) 
    {
        //nothing inserted
        return GetRootItemWithIdentifier(strIdentifier);
    }
    else
    {
        strRoot = strIdentifier.Left(iIndex);
    }

    CGridTreeItem* pRoot = GetRootItemWithIdentifier(strRoot);
    if (pRoot)
    {
        return FindChildWithIdentifier(strIdentifier, pRoot);
    }

    return NULL;
}

CGridTreeItem* CPropertyGrid::FindChildWithIdentifier(CString strIdentifier, CGridTreeItem* pParent)
{
    if (pParent)
    {
        //get new string to search for
        int iLenParent = pParent->m_strIdentifier.GetLength() + 1; //plus 1 for '\'
        int iLenChild = strIdentifier.GetLength();

        if (iLenChild>iLenParent)
        {
            int iNewIndex = strIdentifier.Find('\\',iLenParent);
            CString strNewSearchId;
            BOOL bFinal = FALSE;
            if (iNewIndex==-1)
            {
                bFinal = TRUE;
                strNewSearchId = strIdentifier;
            }
            else
            {
                strNewSearchId = strIdentifier.Left(iNewIndex);
            }

	        //GetNext ....loop through all children 
            CGridTreeItem* pItem = pParent;
	        for(;;)
	        {
		        CGridTreeItem *pCur = GetNext(pParent, pItem, TRUE, FALSE);	  
		        if(!IsChildOf(pParent, pCur))
			        break;
		        else
		        if(pCur==pItem)
			        break;

                //
                if (pCur && strNewSearchId.Compare(pCur->m_strIdentifier)==0)
                {
                    if (bFinal)
                        return pCur;
                    else
                        return FindChildWithIdentifier(strIdentifier,pCur);
                }

		        pItem=pCur;//next;
	        }
        }
        else ASSERT(FALSE);
    }

    return NULL;
}

// SB - A fast function that returns TRUE if the strings are the same
static xbool FastAreStringsEqual( const CString& A, const CString& B )
{
    // Quick reject - if lengths are different, no need to go further
    // (strcmp would still continue to figure out the < or > )
    if (A.GetLength() != B.GetLength())
        return FALSE ;

    // Perform slow compare...
    return (A.Compare(B) == 0) ;
}

// SB - A simple recursive search for the requested item
static CGridTreeItem* FastFindTreeItem(CGridTreeItem* pItem, const CString& strIdentifier)
{
    ASSERT(pItem) ;

    // Found?
    if (FastAreStringsEqual(strIdentifier, pItem->m_strIdentifier))
        return pItem ;

    // Recurse on children
    POSITION pos = pItem->m_listChild.GetHeadPosition();
    while (pos)
    {
	    CGridTreeItem* pChild  = (CGridTreeItem*)pItem->m_listChild.GetNext(pos);
	    CGridTreeItem* pSearch = FastFindTreeItem(pChild, strIdentifier) ;
        if (pSearch)
            return pSearch ;

    }

    // Not found
    return NULL ;
}

// SB - New and improved - uses coherency from last search and a much faster search than the other function
CGridTreeItem* CPropertyGrid::FastFindTreeItemWithIdentifier(const CString& strIdentifier, CGridTreeItem* pPreviousResult)
{
    // Quick test - was last item a match for this item also
    if ( (pPreviousResult) && (FastAreStringsEqual(strIdentifier, pPreviousResult->m_strIdentifier)) )
        return pPreviousResult ;
    
    // Check all root items
    s32 Count = GetRootCount() ;
    for (s32 i = 0 ; i < Count ; i++)
    {
        CGridTreeItem* pItem   = GetRootItem(i) ;
        CGridTreeItem* pSearch = FastFindTreeItem(pItem, strIdentifier) ;
        if (pSearch)
            return pSearch ;
    }

    return NULL ;
}

bool CPropertyGrid::IsChildOf( CGridTreeItem* pRoot, CGridTreeItem* pSearchItem )
{
    ASSERT( pRoot );
    ASSERT( pSearchItem );

    // Recurse on children
    POSITION pos = pRoot->m_listChild.GetHeadPosition();
    while( pos )
    {
        CGridTreeItem* pChild  = (CGridTreeItem*)pRoot->m_listChild.GetNext( pos );

        if( pChild == pSearchItem )
            return true;

        if( IsChildOf( pChild, pSearchItem ) )
            return true;
    }

    return false;
}

bool CPropertyGrid::IsValidItem( CGridTreeItem* pItemToValidate )
{
    s32 RootCount = GetRootCount();
    for( s32 i=0; i<RootCount; i++ )
    {
        CGridTreeItem* pRootItem = GetRootItem(i);
        ASSERT( pRootItem );

        if( pItemToValidate == pRootItem )
            return true;

        if( IsChildOf( pRootItem, pItemToValidate ) )
            return true;
    }

    return false;
}


void CPropertyGrid::OnTreeItemSelected(CGridTreeItem* pItem)
{
    if (pItem)
    {
        if (pItem->m_bGridCreated || pItem->m_strIdentifier.IsEmpty())
        {
            //no id or machine created, maybe parent has a comment
            if (pItem->m_pParent)
            {
                //has parent so get parent properties
                OnTreeItemSelected(pItem->m_pParent);
                return;
            }
        }
        
        //we get here so we need to send the message
        if (!GetParent()->SendMessage(WM_USER_MSG_GRID_SELECTION_CHANGE,(WPARAM)pItem,0))
	    {
		    TRACE("CPropertyGrid::OnTreeItemSelected::ERROR SELECTING PROPERTY\n");
	    }
    }
    else
    {
	    TRACE("CPropertyGrid::OnTreeItemSelected::INVALID POINTER\n");
    }
}

//handle calling out
void CPropertyGrid::OnGridItemChange(CGridTreeItem *pSelItem) 
{
    if (pSelItem)
    {
        if (pSelItem->m_bGridCreated)
        {
            //machine created, hmmmm, send this message to parent
            OnGridItemChange (pSelItem->m_pParent);
        }
        else if (!GetParent()->SendMessage(WM_USER_MSG_GRID_ITEM_CHANGE,(WPARAM)pSelItem,0))
	    {
		    TRACE("CPropertyGrid::OnGridItemChange::ERROR SAVING PROPERTY\n");
	    }
    }
    else
    {
	    TRACE("CPropertyGrid::OnGridItemChange::INVALID POINTER\n");
    }
}

//handle calling out
void CPropertyGrid::OnGuidSelect(CGridTreeItem *pSelItem) 
{
    if (pSelItem)
    {
        if (pSelItem->m_bGridCreated)
        {
            //machine created, hmmmm, send this message to parent
            OnGuidSelect (pSelItem->m_pParent);
        }
        else if (!GetParent()->SendMessage(WM_USER_MSG_GUID_SELECT_REQUEST,(WPARAM)pSelItem,0))
	    {
		    TRACE("CPropertyGrid::OnGuidSelect::ERROR SAVING PROPERTY\n");
	    }
    }
    else
    {
	    TRACE("CPropertyGrid::OnGuidSelect::INVALID POINTER\n");
    }
}

////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////


CPropertyTreeColor::CPropertyTreeColor() :
m_cr(RGB(0,0,0)),
m_alpha(0)
{
}

CString CPropertyTreeColor::FormatColor()
{
	CString strColor;
	strColor.Format("%u, %u, %u, %u",GetRValue(m_cr), GetGValue(m_cr), GetBValue(m_cr), m_alpha);
	return strColor;
}

void CPropertyTreeColor::ParseColor(CString strColor)
{
	int r=0,g=0,b = 0;
	sscanf( strColor, "%u, %u, %u, %u",&r,&g,&b,&m_alpha );
	m_cr = RGB(r,g,b);
	if (m_alpha>255) m_alpha=255;
}

// handle compound objects
void CPropertyTreeColor::NotifyOfChildChange(CGridListCtrl *pGrid)
{
	//Color Change, get data from children
	//GetNext ....loop through all children 
	CString strData;
	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		if (!strData.IsEmpty()) strData += ", ";
		strData += lp->GetSubItem(EDITABLE_COL);
		pItem=pCur;
	}
	ParseColor(strData);
	m_lpNodeInfo->SetSubItemText(EDITABLE_COL,FormatColor());
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, FormatColor());

	//now carry back the proper values
	NotifyOfChange(pGrid);
}

// handle compound objects
void CPropertyTreeColor::NotifyOfChange(CGridListCtrl *pGrid)
{
	//first parse in the new color
	CGridItemInfo* lpParent = pGrid->GetData(this);
	ParseColor(lpParent->GetSubItem(EDITABLE_COL));

	lpParent->SetSubItemText(EDITABLE_COL,FormatColor());
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, FormatColor());

	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		CString strVal = lp->GetItemText();
		CString strClr;
		if (strVal.CompareNoCase("Red")==0)
		{
			strClr.Format("%u",GetRValue(m_cr));
		}
		else if (strVal.CompareNoCase("Green")==0)
		{
			strClr.Format("%u",GetGValue(m_cr));
		}
		else if (strVal.CompareNoCase("Blue")==0)
		{
			strClr.Format("%u",GetBValue(m_cr));
		}
		else if (strVal.CompareNoCase("Alpha")==0)
		{
			strClr.Format("%u",m_alpha);
		}
		lp->SetSubItemText(EDITABLE_COL,strClr);
		pGrid->SetItemText(pCur->m_nIndex, EDITABLE_COL+1, strClr);
		pItem=pCur;
	}
}

////////////////////////////////////////////////////////////////////////////////////////

CPropertyTree2DPos::CPropertyTree2DPos() :
m_x(0.0),
m_y(0.0)
{
}

CString CPropertyTree2DPos::FormatPos()
{
	CString strPos;
	strPos.Format("%g, %g",m_x, m_y);
	return strPos;
}

void CPropertyTree2DPos::ParsePos(CString strPos)
{
	sscanf( strPos, "%f, %f",&m_x,&m_y );
}

// handle compound objects
void CPropertyTree2DPos::NotifyOfChildChange(CGridListCtrl *pGrid)
{
	//GetNext ....loop through all children 
	CString strData;
	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		if (!strData.IsEmpty()) strData += ", ";
		strData += lp->GetSubItem(EDITABLE_COL);
		pItem=pCur;
	}
	ParsePos(strData);
	m_lpNodeInfo->SetSubItemText(EDITABLE_COL,FormatPos());
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, FormatPos());
}

// handle compound objects
void CPropertyTree2DPos::NotifyOfChange(CGridListCtrl *pGrid)
{
	//first parse in the new color
	CGridItemInfo* lpParent = pGrid->GetData(this);
	ParsePos(lpParent->GetSubItem(EDITABLE_COL));

	lpParent->SetSubItemText(EDITABLE_COL,FormatPos());
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, FormatPos());

	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		CString strVal = lp->GetItemText();
		CString strClr;
		if (strVal.CompareNoCase("X")==0)
		{
			strClr.Format("%g",m_x);
		}
		else if (strVal.CompareNoCase("Y")==0)
		{
			strClr.Format("%g",m_y);
		}
		lp->SetSubItemText(EDITABLE_COL,strClr);
		pGrid->SetItemText(pCur->m_nIndex, EDITABLE_COL+1, strClr);
		pItem=pCur;
	}
}

////////////////////////////////////////////////////////////////////////////////////////

CPropertyTree3DPos::CPropertyTree3DPos() :
m_x(0.0),
m_y(0.0),
m_z(0.0)
{
}

CString CPropertyTree3DPos::FormatPos()
{
	CString strPos;
	strPos.Format("%g, %g, %g",m_x, m_y, m_z);
	return strPos;
}

void CPropertyTree3DPos::ParsePos(CString strPos)
{
	sscanf( strPos, "%f, %f, %f",&m_x,&m_y,&m_z  );
}

// handle compound objects
void CPropertyTree3DPos::NotifyOfChildChange(CGridListCtrl *pGrid)
{
	//GetNext ....loop through all children 
	CString strData;
	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		if (!strData.IsEmpty()) strData += ", ";
		strData += lp->GetSubItem(EDITABLE_COL);
		pItem=pCur;
	}
	ParsePos(strData);
	m_lpNodeInfo->SetSubItemText(EDITABLE_COL,FormatPos());
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, FormatPos());
}

// handle compound objects
void CPropertyTree3DPos::NotifyOfChange(CGridListCtrl *pGrid)
{
	//first parse in the new color
	CGridItemInfo* lpParent = pGrid->GetData(this);
	ParsePos(lpParent->GetSubItem(EDITABLE_COL));

	lpParent->SetSubItemText(EDITABLE_COL,FormatPos());
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, FormatPos());

	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		CString strVal = lp->GetItemText();
		CString strClr;
		if (strVal.CompareNoCase("X")==0)
		{
			strClr.Format("%g",m_x);
		}
		else if (strVal.CompareNoCase("Y")==0)
		{
			strClr.Format("%g",m_y);
		}
		else if (strVal.CompareNoCase("Z")==0)
		{
			strClr.Format("%g",m_z);
		}
		lp->SetSubItemText(EDITABLE_COL,strClr);
		pGrid->SetItemText(pCur->m_nIndex, EDITABLE_COL+1, strClr);
		pItem=pCur;
	}
}

////////////////////////////////////////////////////////////////////////////////////////

CPropertyTreeRotation::CPropertyTreeRotation() :
m_Roll(0.0),
m_Pitch(0.0),
m_Yaw(0.0)
{
}

CString CPropertyTreeRotation::FormatRot()
{
	CString strRot;
	strRot.Format("%g, %g, %g", m_Roll, m_Pitch, m_Yaw );
	return strRot;
}

void CPropertyTreeRotation::ParseRot(CString strRot)
{
	sscanf( strRot, "%f, %f, %f", &m_Roll, &m_Pitch, &m_Yaw );
}

// handle compound objects
void CPropertyTreeRotation::NotifyOfChildChange(CGridListCtrl *pGrid)
{
	//GetNext ....loop through all children 
	CString strData;
	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		if (!strData.IsEmpty()) strData += ", ";
        
        CString strEditText = lp->GetSubItem(EDITABLE_COL);
		strData += strEditText;
		pItem=pCur;
	}
	ParseRot(strData);
	m_lpNodeInfo->SetSubItemText(EDITABLE_COL,FormatRot());
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, FormatRot());
}

// handle compound objects
void CPropertyTreeRotation::NotifyOfChange(CGridListCtrl *pGrid)
{
	//first parse in the new color
	CGridItemInfo* lpParent = pGrid->GetData(this);
	ParseRot(lpParent->GetSubItem(EDITABLE_COL));

	lpParent->SetSubItemText(EDITABLE_COL,FormatRot());
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, FormatRot());

	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		CString strVal = lp->GetItemText();
		CString strClr;
		if (strVal.CompareNoCase("Roll")==0)
		{
			strClr.Format("%g",m_Roll);
		}
		else if (strVal.CompareNoCase("Pitch")==0)
		{
			strClr.Format("%g",m_Pitch);
		}
		else if (strVal.CompareNoCase("Yaw")==0)
		{
			strClr.Format("%g",m_Yaw);
		}
		lp->SetSubItemText(EDITABLE_COL,strClr);
		pGrid->SetItemText(pCur->m_nIndex, EDITABLE_COL+1, strClr);
		pItem=pCur;
	}
}

////////////////////////////////////////////////////////////////////////////////////////

CPropertyTreeBoundingBox::CPropertyTreeBoundingBox() :
m_pVectorMin(NULL),
m_pVectorMax(NULL)
{
}

void CPropertyTreeBoundingBox::CalcExtents(CGridListCtrl *pGrid)
{
	//get children and display
	CString strDisplay;
	
	float x1 = 0.0, y1 = 0.0, z1 = 0.0;
	float x2 = 0.0, y2 = 0.0, z2 = 0.0;
	sscanf( m_strBound1, "%f, %f, %f",&x1,&y1,&z1  );
	sscanf( m_strBound2, "%f, %f, %f",&x2,&y2,&z2  );

	bbox box(vector3(x1,y1,z1),vector3(x2,y2,z2));
	vector3 vExtent = box.GetSize();

	strDisplay.Format("Extent {%g, %g, %g}",vExtent[0], vExtent[1], vExtent[2]);

	m_lpNodeInfo->SetSubItemText(EDITABLE_COL,strDisplay);
	pGrid->SetItemText(m_nIndex, EDITABLE_COL+1, strDisplay);
}

// handle compound objects
void CPropertyTreeBoundingBox::NotifyOfChildChange(CGridListCtrl *pGrid)
{
	//GetNext ....loop through all children 
	CString strData;
	CGridTreeItem* pItem = this;
	for(;;)
	{
		CGridTreeItem *pCur = pGrid->GetNext(this,pItem, TRUE, FALSE);	  

		if(!pGrid->IsChildOf(this, pCur))
			break;
		else if(pCur==pItem)
			break;
		CGridItemInfo* lp = pGrid->GetData(pCur);

		CString strVal = lp->GetItemText();
		if (strVal.CompareNoCase("Min")==0)
		{
			m_strBound1 = lp->GetSubItem(EDITABLE_COL);
		}
		else if (strVal.CompareNoCase("Max")==0)
		{
			m_strBound2 = lp->GetSubItem(EDITABLE_COL);
		}

		pItem=pCur;
	}
	CalcExtents(pGrid);
}
