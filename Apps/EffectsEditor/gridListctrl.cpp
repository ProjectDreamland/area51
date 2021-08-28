// GridListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "GridListCtrl.h"
#include "GridEditCtrl.h"
#include "GridComboBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define OFFSET_FIRST			3
#define OFFSET_OTHER			3
#define IDC_EDITCTRL			0x5001
#define IDC_COMBOBOXINLISTVIEW	0x5002

#define COLOR_DATA_SELECTED RGB(70,70,125) //RGB(50,50,195)
#define COLOR_NAME_SELECTED RGB(70,70,125) //RGB(90,90,135)

/////////////////////////////////////////////////////////////////////////////
// CGridListCtrl
CGridListCtrl::CGridListCtrl():
m_cxImage(8),
m_cyImage(8),
m_bIsDragging(FALSE),
m_CurSubItem(0),
m_nDragTarget(-1),
m_nDragItem(-1),
m_bAllowDrag(FALSE),
m_pLastSelectedTreeItem(NULL)
{
	m_psTreeLine.CreatePen(PS_SOLID, 1, RGB(128,128,128));
	m_psRectangle.CreatePen(PS_SOLID, 1, RGB(198,198,198));
	m_psPlusMinus.CreatePen(PS_SOLID, 1, RGB(0,0,0));
	m_brushErase.CreateSolidBrush(RGB(255,255,255));
}

CGridListCtrl::~CGridListCtrl()
{
	m_psPlusMinus.DeleteObject();
	m_psRectangle.DeleteObject();
	m_psTreeLine.DeleteObject();
	m_brushErase.DeleteObject();

	while(m_RootItems.GetCount())
	{
		CGridTreeItem * root = (CGridTreeItem*)m_RootItems.RemoveHead();
		if(root!=NULL && GetData(root) != NULL)
		{
			delete GetData(root);
		}
		delete root;
	}
	m_RootItems.RemoveAll();
}

BEGIN_MESSAGE_MAP(CGridListCtrl, CListCtrl)
	//{{AFX_MSG_MAP(CGridListCtrl)
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT(NM_DBLCLK, OnDblclk)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, OnEndlabeledit)
	ON_WM_CREATE()
	ON_WM_HSCROLL()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(LVN_KEYDOWN, OnKeydown)
	ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnBegindrag)
	ON_WM_MEASUREITEM_REFLECT()
	ON_WM_DRAWITEM_REFLECT()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGridListCtrl message handlers

BOOL CGridListCtrl::PreCreateWindow(CREATESTRUCT& cs) 
{
	cs.style |= LVS_REPORT | LVS_SINGLESEL | LVS_SHAREIMAGELISTS | LVS_OWNERDRAWFIXED | LVS_SHOWSELALWAYS;	
	return CListCtrl::PreCreateWindow(cs);
}

int CGridListCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CGridListCtrl::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (lpDrawItemStruct->CtlType != ODT_LISTVIEW)
        return;

	if(lpDrawItemStruct->itemAction == ODA_DRAWENTIRE)
	{
		LV_ITEM lvi;
		static _TCHAR szBuff[MAX_PATH];
		LPCTSTR pszText;
	
		int nItem = lpDrawItemStruct->itemID;
		CRect rcItem(lpDrawItemStruct->rcItem);

		lvi.mask = LVIF_TEXT | LVIF_STATE | LVIF_INDENT | LVIF_PARAM; 
		lvi.iItem = nItem;
		lvi.iSubItem =0;
		lvi.pszText = szBuff;
		lvi.cchTextMax = sizeof(szBuff);
		lvi.stateMask = 0xFFFF;		
		GetItem(&lvi);

		CGridTreeItem *pSelItem = (CGridTreeItem*)lpDrawItemStruct->itemData;

		CRect rcLabel;
		GetItemRect(nItem, rcLabel, LVIR_LABEL);
		
		CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
		ASSERT(pDC);

		CRect rcClipBox;
		pDC->GetClipBox(rcClipBox);

		COLORREF crBackground, crText;
		
		BOOL bReadOnlyItem = FALSE;
        BOOL bHeader       = FALSE;
		CGridItemInfo* pInfo = NULL;
		if (pSelItem)
		{
			pInfo = GetData(pSelItem);
            if( pInfo )
            {
                bHeader       = pInfo->IsHeader();
                bReadOnlyItem = bHeader || pInfo->IsReadOnly(); 
            }
		}

		CRect rc;
		GetItemRect(nItem, rc, LVIR_BOUNDS);
		COLORREF crBorder = RGB(180,180,180);

		if (!pSelItem->m_pParent)
		{
			//root item
			crText = RGB(255,255,255);
			crBackground = crBorder;
		}
		else
		{
			if (lpDrawItemStruct->itemState & ODS_SELECTED)
			{
				// Set the text background and foreground colors
				crBackground = COLOR_NAME_SELECTED;
				if (bReadOnlyItem)
				{
					crText = RGB(192,192,192);
				}
				else
				{
					crText = RGB(255,255,255);
				}
			}
			else
			{
				// Set the text background and foreground colors to the standard window colors
                if( bHeader )
                {
                    //crBackground = RGB(210,210,210);
                    crBackground    = pSelItem->m_BackgroundColor;
                    crText          = RGB(64,64,64);
                }
                else
                {
                    crBackground    = pSelItem->m_BackgroundColor;

				    if (bReadOnlyItem)
				    {
					    crText = RGB(128,128,128);
				    }
				    else
				    {
					    crText = RGB(0,0,0);
				    }
                }
			}
		}
		pDC->FillSolidRect(&rc,crBackground);

		CRect rcBorder = rc;
		rcBorder.right = rcBorder.left + m_cxImage;
		pDC->FillSolidRect(&rcBorder,crBorder);

		DrawTreeItem(pDC, pSelItem, nItem, rc);

		//Draw selection bar (erase old selection too)
		pDC->SetBkColor(crBackground);

		CRect rcClipText = lpDrawItemStruct->rcItem;
		rcClipText.left += GetIndent(pSelItem) * m_cxImage + 2;
		if(rcClipText.left > GetColumnWidth(0))
		{
			rcClipText.left = GetColumnWidth(0);
		}
		//fill background color
		ExtTextOut(pDC->GetSafeHdc(), 0, 0, ETO_OPAQUE, rcClipText, NULL, 0, NULL);
		
		//draw color in first col if any
		rcClipText.right = rcLabel.right;
		CGridItemInfo *pItemInfo = GetData(pSelItem);		

		//draw 1. item	
		GetItemRect(nItem, rcItem, LVIR_LABEL);
		pszText = MakeShortString(pDC, szBuff, rcItem.right - rcItem.left, 2*OFFSET_FIRST);

		pDC->SetBkColor (crBackground);
		pDC->SetTextColor (crText);
		rcClipText.left += 3; //indent text
		pDC->DrawText(pszText,-1, rcClipText,DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_EXTERNALLEADING);

		//draw subitems..
		LV_COLUMN lvc;
		lvc.mask = LVCF_FMT | LVCF_WIDTH;
		for(int nColumn=1; GetColumn(nColumn, &lvc); nColumn++)
		{
			rcItem.left=rcItem.right;
			rcItem.right+=lvc.cx;
			if (rcItem.left < rcClipBox.right && rcItem.right > rcClipBox.left && rcItem.right > rcItem.left)
			{
				//support for colors in each cell
				int nRetLen = GetItemText(nItem, nColumn, szBuff, sizeof(szBuff));
				if(nRetLen==0)
				{
					pszText=NULL;
				}
				else
				{
					pszText=MakeShortString(pDC,szBuff,rcItem.right-rcItem.left,2*OFFSET_OTHER);
				}

				UINT nJustify=DT_LEFT;

				if(pszText==szBuff)
				{
					switch(lvc.fmt & LVCFMT_JUSTIFYMASK)
					{
					case LVCFMT_RIGHT:
						nJustify=DT_RIGHT;
						break;
					case LVCFMT_CENTER:
						nJustify=DT_CENTER;
						break;
					default:
						break;
					}
				}
				rcLabel=rcItem;
				rcLabel.left+=OFFSET_OTHER;
				rcLabel.right-=OFFSET_OTHER;

				if (lpDrawItemStruct->itemState & ODS_SELECTED && !m_bIsDragging)
				{
					//not a root item
					if (pSelItem->m_pParent)
					{
						DrawFocusCell(pDC, lpDrawItemStruct->itemID, nColumn, bReadOnlyItem);
					}
				}

				if(pszText!=NULL)
				{
					pDC->DrawText(pszText,-1,rcLabel, nJustify | DT_SINGLELINE | DT_NOPREFIX | DT_VCENTER | DT_EXTERNALLEADING);
				}
			}

			// draw selection for root items
			if (lpDrawItemStruct->itemState & ODS_SELECTED && !m_bIsDragging)
			{
                //Check for selection change
                OnSelectionChange();

				//for a root item
				if (!pSelItem->m_pParent)
				{
					CBrush brGrid1(RGB(192,192,192));
					CBrush brGrid2(RGB(222,222,222));

					CRect rectFrame = rc;
					rectFrame.left += m_cxImage;
					rectFrame.DeflateRect(0,1);
					pDC->FrameRect(rectFrame,&brGrid1);
					rectFrame.DeflateRect(0,0,0,1);
					pDC->FrameRect(rectFrame,&brGrid2);
				}
			}

/********************************************************************************
			//Draw GridLines
			CPen* pOldPen = pDC->SelectObject(&m_psTreeLine);
			pDC->MoveTo(rc.left+m_cxImage,rc.bottom-1);
			pDC->LineTo(rc.right,rc.bottom-1);

			//draw the separator for non-root items for first column
			if (pSelItem->m_pParent)
			{
				pDC->MoveTo(GetColumnWidth(0),rc.top);
				pDC->LineTo(GetColumnWidth(0),rc.bottom-1);
			}

			//reselect last pen
			pDC->SelectObject(pOldPen);		
********************************************************************************/

			CGridItemInfo::CONTROLTYPE ctrlType;
			pInfo->GetControlType(nColumn-1, ctrlType);

			//draw special controls regardless of rcClipBox rgn
			if ((lpDrawItemStruct->itemState & ODS_SELECTED) ||
				(ctrlType == pInfo->CONTROLTYPE::GCT_EXTERNAL) ||
				(ctrlType == pInfo->CONTROLTYPE::GCT_BUTTON) ||
				(ctrlType == pInfo->CONTROLTYPE::GCT_COLOR_BUTTON))
			{
				DrawControl(pDC, pSelItem, nItem, nColumn, crBorder);
			}

		}//for
	}//ODA_DRAWENTIRE
}


void CGridListCtrl::AutoSizeColumns(int col /*=-1*/) 
{
     // Call this after your list control is filled
     int mincol = col < 0 ? 0 : col;
     int maxcol = col < 0 ? GetColumnCount()-1 : col;
     for (col = mincol; col <= maxcol; col++) 
	 {
          SetColumnWidth(col,LVSCW_AUTOSIZE);
          int wc1 = GetColumnWidth(col);
          SetColumnWidth(col,LVSCW_AUTOSIZE_USEHEADER);
          int wc2 = GetColumnWidth(col);
          int wc = max(20,max(wc1,wc2));
          SetColumnWidth(col,wc);
     }
     Invalidate(); 
}


void CGridListCtrl::DrawControl(CDC* pDC, CGridTreeItem *pSelItem, int nItem, int nColumn, COLORREF crBorder)
{
	CGridItemInfo* pInfo = GetData(pSelItem);
	CGridItemInfo::CONTROLTYPE ctrlType;
	if(pInfo->GetControlType(nColumn-1, ctrlType))
	{
		switch(ctrlType)
		{
			case pInfo->CONTROLTYPE::GCT_COLOR_BUTTON:
			{
				CRect rect;
				GetSubItemRect(nItem, nColumn, LVIR_BOUNDS, rect);
				rect.left=rect.right - GetSystemMetrics(SM_CYVSCROLL);
				rect.DeflateRect(0,1);
				pDC->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONPUSH);
				rect.DeflateRect(2,0,2,4);
				COLORREF crOld = pDC->GetTextColor();
				pDC->SetTextColor(RGB(0,0,255));
				pDC->DrawText("c",rect,DT_CENTER);
				pDC->SetTextColor(crOld);
			}
			break;
			case pInfo->CONTROLTYPE::GCT_DIR_BUTTON:
			{
				CRect rect;
				GetSubItemRect(nItem, nColumn, LVIR_BOUNDS, rect);
				rect.left=rect.right - GetSystemMetrics(SM_CYVSCROLL);
				rect.DeflateRect(0,1);
				pDC->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONPUSH);
				rect.DeflateRect(2,2);
				COLORREF crOld = pDC->GetTextColor();
				pDC->SetTextColor(RGB(0,0,0));
				pDC->DrawText("...",rect,DT_CENTER);
				pDC->SetTextColor(crOld);
			}
			break;
			case pInfo->CONTROLTYPE::GCT_COMBOBOX:
			{
				CRect rect;
				GetSubItemRect(nItem, nColumn, LVIR_BOUNDS, rect);
				rect.left=rect.right - GetSystemMetrics(SM_CYVSCROLL);
				pDC->DrawFrameControl(rect, DFC_SCROLL, DFCS_SCROLLDOWN);
			}
			break;
			case pInfo->CONTROLTYPE::GCT_EXTERNAL:
			case pInfo->CONTROLTYPE::GCT_BUTTON:
			{
				CRect rect;
				GetSubItemRect(nItem, nColumn, LVIR_BOUNDS, rect);
				rect.DeflateRect(1,0);
				pDC->DrawFrameControl(rect, DFC_BUTTON, DFCS_BUTTONPUSH);
				rect.DeflateRect(2,2);
				COLORREF crOld = pDC->GetTextColor();
				pDC->SetTextColor(RGB(0,0,0));
				pDC->DrawText(pInfo->GetSubItem(nColumn-1),rect,DT_CENTER);
				pDC->SetTextColor(crOld);
			}
			break;
		}
	}
}

//this piece of code is borrowed from the wndproc.c file in the odlistvw.exe example from MSDN and was converted to mfc-style
void CGridListCtrl::MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct)
{
    if (lpMeasureItemStruct->CtlType != ODT_LISTVIEW)
	{
        return;
	}

	TEXTMETRIC tm;
	CClientDC dc(this);	
	CFont* pFont = GetFont();
	CFont* pOldFont = dc.SelectObject(pFont);	
	dc.GetTextMetrics(&tm);
	int nItemHeight = tm.tmHeight + tm.tmExternalLeading;
	lpMeasureItemStruct->itemHeight = nItemHeight + 4; //or should I go for max(nItemheight+4, m_cxImage+2);
	dc.SelectObject(pOldFont);
}

//the basic rutine making the ... thing snatched it from some tedious code example some where in MSDN call odlistvw.exe
LPCTSTR CGridListCtrl::MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset)
{
	static const _TCHAR szThreeDots[]=_T("~");

	int nStringLen=lstrlen(lpszLong);

	if(nStringLen==0 || pDC->GetTextExtent(lpszLong,nStringLen).cx + nOffset < nColumnLen)
	{
		return(lpszLong);
	}

	static _TCHAR szShort[MAX_PATH];

	lstrcpy(szShort,lpszLong);
	int nAddLen = pDC->GetTextExtent(szThreeDots,x_strlen(szThreeDots)).cx;

	for(int i=nStringLen-1; i > 0; i--)
	{
		szShort[i]=0;
		if(pDC->GetTextExtent(szShort,i).cx + nOffset + nAddLen < nColumnLen)
			break;
	}
	lstrcat(szShort,szThreeDots);
	return(szShort);
}

void CGridListCtrl::OnKeydown(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_KEYDOWN* pLVKeyDow = (LV_KEYDOWN*)pNMHDR;
	switch(pLVKeyDow->wVKey)
	{
/* don't want deleting right now
		case VK_DELETE: 
			{
				int nItem = GetSelectedItem();
				if(nItem!=-1)
				{
					CGridTreeItem* pSelItem = GetTreeItem(nItem);
					if(pSelItem != NULL)
					{
						if(OnDeleteItem(pSelItem, nItem))
						{
							DeleteItemEx(pSelItem, nItem);
						}
					}	
				}
			}	
			break;
*/
		case VK_MULTIPLY:
			{  
				int nIndex = GetSelectedItem();
				if(nIndex != -1)
				{
					CWaitCursor wait;
					SetRedraw(0);
					CGridTreeItem *pParent = GetTreeItem(nIndex);
					int nScroll=0;
					if(OnVKMultiply(pParent, nIndex))
					{	
						ExpandAll(pParent, nScroll);
					}		
					SetRedraw(1);
					RedrawItems(nIndex, nScroll);
					EnsureVisible(nScroll, TRUE);
				 }
			 }
			break;

		case VK_ADD:
			{
					int nIndex = GetSelectedItem();
					if(nIndex!=-1)
					{
						CWaitCursor wait;
						CGridTreeItem *pSelItem = GetTreeItem(nIndex);
						int nScrollIndex = 0;
						if(OnVKAdd(pSelItem, nIndex))
						{
							 nScrollIndex = Expand(pSelItem, nIndex);
						}
						CRect rc;
						GetItemRect(nIndex, rc, LVIR_BOUNDS);
						InvalidateRect(rc);
						UpdateWindow();
						EnsureVisible(nScrollIndex, 1);
					}
			}
			break;

		case VK_SUBTRACT:
			{
				int nIndex = GetSelectedItem();
				if(nIndex!=-1)
				{
					CWaitCursor wait;
					CGridTreeItem *pSelItem = GetTreeItem(nIndex);
					if(OnVkSubTract(pSelItem, nIndex))
					{
						Collapse(pSelItem);
					}
					CRect rc;
					GetItemRect(nIndex, rc, LVIR_BOUNDS);
					InvalidateRect(rc);
					UpdateWindow();
				}
			}
			break;
		default :
			break;
	}
	*pResult = 0;
}

BOOL CGridListCtrl::HitTestOnSign(CPoint point, LVHITTESTINFO& ht)
{
	ht.pt = point;
	// Test which subitem was clicked.
	SubItemHitTest(&ht);
	if(ht.iItem!=-1)
	{
		CGridTreeItem* pItem = GetTreeItem(ht.iItem);
		if(pItem!=NULL)
		{
			//if haschildren and clicked on + or - then expand/collapse
			if(ItemHasChildren(pItem))
			{
				//hittest on the plus/sign "button" 
				CRect rcBounds;
				GetItemRect(ht.iItem, rcBounds, LVIR_BOUNDS);
				CGridTreeToggle rect(this, NULL, GetIndent(pItem), rcBounds);
				BOOL bRedraw=0;//if OnItemExpanding or OnCollapsing returns false, dont redraw
				if(rect.HitTest(point))
				{
					SetRedraw(0);
					int nScrollIndex=0;
					if(IsCollapsed(pItem))
					{	
						if(OnItemExpanding(pItem, ht.iItem))
						{
							nScrollIndex = Expand(pItem, ht.iItem);
							OnItemExpanded(pItem, ht.iItem);
							bRedraw=1;
						}
					}	
					else 
                    {
					   if(OnCollapsing(pItem))
					   {
							Collapse(pItem);
							OnItemCollapsed(pItem);
                            nScrollIndex = GetCurIndex(pItem);
							bRedraw=1;
					   }
					}
					SetRedraw(1);
					if(bRedraw)
					{
						CRect rc;
						GetItemRect(ht.iItem, rc, LVIR_BOUNDS);
						InvalidateRect(rc);
						UpdateWindow();
						EnsureVisible(nScrollIndex, 1);
						return 0;
					}	
				}
			}//has children
		}//pItem!=NULL
	}

	return 1;
}

//browse button handler
void CGridListCtrl::OnButtonBrowse(int iItem, int iSubItem)
{
	CFileDialog dlgBrowse(TRUE);
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

void CGridListCtrl::OnButtonColorPick(int iItem, int iSubItem)
{
	CGridTreeItem* pItem = GetTreeItem(iItem);

	CColorDialog dlgColor;
	if (dlgColor.DoModal() == IDOK)
	{
		CString strColor;
		COLORREF color = dlgColor.GetColor();
		strColor.Format("% 3u, % 3u, % 3u",GetRValue(color), GetGValue(color), GetBValue(color));
		SetItemText(iItem, iSubItem, strColor);

		CGridItemInfo* lp = GetData(pItem);
		lp->SetSubItemText(iSubItem-1, strColor);

		pItem->NotifyOfChange(this);
		OnGridItemChange(pItem);
	}
}

BOOL CGridListCtrl::OnEraseBkgnd(CDC* pDC) 
{
	// Just return...otherwise it'll "erase" it to white which causes flicker
    CRect r;
    GetClientRect( &r );
    pDC->FillSolidRect( &r, RGB(192,192,192) );

//    return CListCtrl::OnEraseBkgnd(pDC);
	return FALSE;
}

void CGridListCtrl::OnDblclk(NMHDR* pNMHDR, LRESULT* pResult) 
{
	if( GetFocus() != this) 
	{
		SetFocus();
	}

	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	LVHITTESTINFO ht;
	ht.pt = pNMListView->ptAction;
	SubItemHitTest(&ht);
	if(OnItemLButtonDown(ht))
	{
		BOOL bSelect=1;
		bSelect = HitTestOnSign(pNMListView->ptAction, ht);
		//normal selection
		if(bSelect && ht.iItem !=-1)
		{		
			int nIndex = GetSelectedItem();
			if(nIndex!=-1)
			{
				CGridTreeItem *pSelItem = GetTreeItem(nIndex);
				if (pSelItem != NULL)
				{
					BOOL bRedraw=0;
					if(ItemHasChildren(pSelItem))
					{
						SetRedraw(0);
						int nScrollIndex=0;
						if(IsCollapsed(pSelItem))
						{		
							if(OnItemExpanding(pSelItem, nIndex))
							{
								nScrollIndex = Expand(pSelItem, nIndex);
								OnItemExpanded(pSelItem, nIndex);
								bRedraw=1;
							}
						}	
					
						else 
						{
						   if(OnCollapsing(pSelItem))
						   {
								Collapse(pSelItem);
								OnItemCollapsed(pSelItem);
								bRedraw=1;
						   }
						}
						SetRedraw(1);

						if(bRedraw)
						{
							CRect rc;
							GetItemRect(nIndex,rc,LVIR_BOUNDS);
							InvalidateRect(rc);
							UpdateWindow();
							EnsureVisible(nScrollIndex,1);
						}
					}//ItemHasChildren	
				}//!=NULL
			}
		}
	}

	*pResult = 0;
}

void CGridListCtrl::OnSelectionChange() 
{
    int nFirstSelected = GetSelectedItem();
    if (nFirstSelected!=-1)
    {
        CGridTreeItem* pFirstItemSelected = GetTreeItem(nFirstSelected);
        if (m_pLastSelectedTreeItem!=pFirstItemSelected)
        {
            //selection has changed
            OnTreeItemSelected(pFirstItemSelected);
            m_pLastSelectedTreeItem = pFirstItemSelected;
        }
    }
}


void CGridListCtrl::OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
	if(m_bAllowDrag && pNMListView->iItem != -1)
	{
		m_nDragItem = pNMListView->iItem;
		CImageList* pDragImage=NULL;
		pDragImage = CreateDragImageEx(m_nDragItem);
		if(pDragImage)
		{
			pDragImage->BeginDrag(0, CPoint(0,0));
			pDragImage->DragEnter(this, pNMListView->ptAction);
			SetCapture();
			m_bIsDragging = TRUE;
		}
		delete pDragImage;
	}
	*pResult = 0;
}

//Create dragimage : Icon + the itemtext
CImageList *CGridListCtrl::CreateDragImageEx(int nItem)
{
    CImageList *pList = new CImageList;          
	//get image index
	LV_ITEM lvItem;
	lvItem.mask =  LVIF_IMAGE;
	lvItem.iItem = nItem;
	lvItem.iSubItem = 0;
	GetItem(&lvItem);

	CRect rc;
	GetItemRect(nItem, &rc, LVIR_BOUNDS);         

	CString str;
	str = GetItemText(nItem, 0);
	CFont *pFont = GetFont();

	rc.OffsetRect(-rc.left, -rc.top);            
	rc.right = GetColumnWidth(0);                
	pList->Create(rc.Width(), rc.Height(),ILC_COLOR24| ILC_MASK , 1, 1);
	CDC *pDC = GetDC();                          
	if(pDC) 
	{
		CDC dc;	      
		dc.CreateCompatibleDC(pDC);      
		CBitmap bmpMap;
		bmpMap.CreateCompatibleBitmap(pDC, rc.Width(), rc.Height());

		CBitmap *pOldBmp = dc.SelectObject(&bmpMap);
		CFont *pOldFont = dc.SelectObject(pFont);
		dc.FillSolidRect(rc, GetSysColor(COLOR_WINDOW));
		CImageList *pImgList = GetImageList(LVSIL_SMALL);
		if(pImgList)
			pImgList->Draw(&dc, lvItem.iImage, CPoint(0,0), ILD_TRANSPARENT);
		dc.TextOut(m_cxImage + 4, 0, str);
		dc.SelectObject(pOldFont);
		dc.SelectObject(pOldBmp);                 
		//causes an error if the 1st column is hidden so must check the imagelist
		if(pList->m_hImageList != NULL)
		{
			pList->Add(&bmpMap, RGB(255,255,255));
		}
		else 
		{ 
			delete pList;
			pList=NULL;
		}
		ReleaseDC(pDC);   
	}   
	return pList;
}

void CGridListCtrl::OnMouseMove(UINT nFlags, CPoint point) 
{
    if(m_bIsDragging)
    {
		KillTimer(1);
		if ((CWnd::GetCapture() != this) ||
			(nFlags==MK_RBUTTON || nFlags==MK_MBUTTON) ||
			(GetKeyState(VK_ESCAPE) < 0) )
		{
			m_bIsDragging=0;
		}
		
		if(!m_bIsDragging)
		{
			SetItemState (m_nDragTarget, 0, LVIS_DROPHILITED);
			CImageList::DragLeave(this);
			CImageList::EndDrag();
			ReleaseCapture();
			InvalidateRect(NULL);
			UpdateWindow();
		}
		else
		{
			CPoint ptList(point);
			MapWindowPoints(this, &ptList, 1);
			CImageList::DragMove(ptList);
			UINT uHitTest = LVHT_ONITEM;
			m_nDragTarget = HitTest(ptList, &uHitTest);
			// try turn off hilight for previous DROPHILITED state
			int nPrev = GetNextItem(-1,LVNI_DROPHILITED);
			if(nPrev != m_nDragTarget)//prevents flicker 
			{	
				SetItemState(nPrev, 0, LVIS_DROPHILITED);
			}

			CRect rect;
			GetClientRect (rect);
			int cy = rect.Height();
			if(m_nDragTarget!=-1)
			{
				SetItemState(m_nDragTarget, LVIS_DROPHILITED, LVIS_DROPHILITED);
				CGridTreeItem* pTarget = GetTreeItem(m_nDragTarget);
				if ((point.y >= 0 && point.y <= m_cyImage) || (point.y >= cy - m_cyImage && point.y <= cy) || 	
					( pTarget!=NULL && ItemHasChildren(pTarget) && IsCollapsed(pTarget)))
				{
					SetTimer(1,300,NULL);
				}
			}
		}
    }

	CListCtrl::OnMouseMove(nFlags, point);
}

void CGridListCtrl::OnTimer(UINT nIDEvent) 
{
	CListCtrl::OnTimer(nIDEvent);
	if(nIDEvent==1)
	{
		if(CWnd::GetCapture()!=this)
		{
			SetItemState(m_nDragTarget, 0, LVIS_DROPHILITED);
			m_bIsDragging=0;
			CImageList::DragLeave(this);
			CImageList::EndDrag();
			ReleaseCapture();
			InvalidateRect(NULL);
			UpdateWindow();
			KillTimer(1);
			return;
		}

		SetTimer(1,300,NULL);//reset timer
		DWORD dwPos = ::GetMessagePos();
		CPoint ptList(LOWORD(dwPos),HIWORD(dwPos));
		ScreenToClient(&ptList);

		CRect rect;
		GetClientRect(rect);
		int cy = rect.Height();
		//
		// perform autoscroll if the cursor is near the top or bottom.
		//
		if (ptList.y >= 0 && ptList.y <= m_cyImage) 
		{
			int n = GetTopIndex(); 
			CImageList::DragShowNolock(0);
			SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEUP, 0), NULL);
			CImageList::DragShowNolock(1);
			if (GetTopIndex()== n)
			{
				KillTimer (1);
			}
			else 
			{
				CImageList::DragShowNolock(0);
				CImageList::DragMove(ptList);
				CImageList::DragShowNolock(1);
				return;
			}
		}
		else if (ptList.y >= cy - m_cyImage && ptList.y <= cy) 
		{
			int n = GetTopIndex(); 
			CImageList::DragShowNolock(0);
			SendMessage(WM_VSCROLL, MAKEWPARAM(SB_LINEDOWN, 0), NULL);
			CImageList::DragShowNolock(1);
			if (GetTopIndex()== n)
			{
				KillTimer (1);
			}
			else 
			{
				CImageList::DragShowNolock(0);
				CImageList::DragMove(ptList);
				CImageList::DragShowNolock(1);
				return;
			}
		}
		
		//Hover test 
		CImageList::DragMove(ptList);
		UINT uHitTest = LVHT_ONITEM;
		m_nDragTarget = HitTest(ptList, &uHitTest);
	
		if(m_nDragTarget!=-1)
		{
			//if the target has children
			//expand them
			CGridTreeItem* pTarget=GetTreeItem(m_nDragTarget);
			if(pTarget != NULL && ItemHasChildren(pTarget) && IsCollapsed(pTarget) && (m_nDragItem!=-1))
			{
				CImageList::DragShowNolock(0);
				CGridTreeItem* pSource = GetTreeItem(m_nDragItem);
	
				SetRedraw(0);
				int nScrollIndex=0;
				if(ItemHasChildren(pTarget) && IsCollapsed(pTarget))
				{	
					if(OnItemExpanding(pTarget, m_nDragTarget))
					{	
						nScrollIndex = Expand(pTarget, m_nDragTarget);
						OnItemExpanded(pTarget, m_nDragTarget);
					}
				}		
				m_nDragItem = NodeToIndex(pSource);
				SetRedraw(1);
				EnsureVisible(nScrollIndex, 1);
				InvalidateRect(NULL);
				UpdateWindow();
				CImageList::DragShowNolock(1);
				KillTimer(1);
				return;
			}	
		}
		KillTimer(1);
	}
}

void CGridListCtrl::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if(m_bIsDragging == TRUE)
    {
		KillTimer(1);
        CImageList::DragLeave(this);
        CImageList::EndDrag();
        ReleaseCapture();
        m_bIsDragging = FALSE;
		SetItemState(m_nDragTarget, 0, LVIS_DROPHILITED);
        if((m_nDragTarget != -1) && (m_nDragTarget != m_nDragItem) && (m_nDragItem!=-1))//no drop on me self
        {
		
			CGridTreeItem* pSource = GetTreeItem(m_nDragItem);
			CGridTreeItem* pTarget = GetTreeItem(m_nDragTarget);
			if(IsRoot(pSource))
			{
				return;
			}
			
			CGridTreeItem* pParent = GetParentItem(pSource);
			//can't drag child to parent
			if(pParent==pTarget) 
			{
				return;
			}

			//can't drag parent to child
			if(!IsChildOf(pSource, pTarget))
			{
				CWaitCursor wait;
				SetRedraw(0);
				if(DoDragDrop(pTarget, pSource))
				{
					UINT uflag = LVIS_SELECTED | LVIS_FOCUSED;
					SetItemState(m_nDragTarget, uflag, uflag);
					m_nDragItem=-1;
					//delete source
					int nIndex = NodeToIndex(pSource);			
					DeleteItem(nIndex);
					HideChildren(pSource, TRUE, nIndex);
					Delete(pSource);
					InternaleUpdateTree();
					SetRedraw(1);
					InvalidateRect(NULL);
					UpdateWindow();
				}
				else
				{
					SetRedraw(1);
				}
			}
	    }
    }
    else
	{
		CListCtrl::OnLButtonUp(nFlags, point);
	}
}

//used with the drag/drop operation
void CGridListCtrl::CopyChildren(CGridTreeItem* pDest, CGridTreeItem* pSrc)
{
	if (ItemHasChildren(pSrc))
	{
		POSITION pos = pSrc->m_listChild.GetHeadPosition();
		while (pos != NULL)
		{
			CGridTreeItem* pItem = (CGridTreeItem *)pSrc->m_listChild.GetNext(pos);
			CGridItemInfo* lp = CopyData(GetData(pItem));
			CGridTreeItem* pNewItem = InsertItem(pDest, lp);
			CopyChildren(pNewItem, pItem);
		}
	}
}

BOOL CGridListCtrl::DoDragDrop(CGridTreeItem* pTarget, CGridTreeItem* pSource)
{
	if(pTarget==NULL)
	{
		return 0;
	}

	BOOL bUpdate=FALSE;
	if(!IsCollapsed(pTarget))
	{
		//children are expanded, want to see update right away
		bUpdate=TRUE; 
	}
	
	//make a copy of the source data
	CGridItemInfo* lp = CopyData(GetData(pSource));
	//create new node with the source data and make pTarget the parent
	
	CGridTreeItem* pNewParent = InsertItem(pTarget, lp, bUpdate);
	//if the source has children copy all source data and make the newly create item the parent
	if(ItemHasChildren(pSource))
	{
		CopyChildren(pNewParent, pSource);
	}

	return 1;
}

void CGridListCtrl::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	//its not meself
	if( GetFocus() != this) 
	{
		SetFocus();
	}
	
	CListCtrl::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CGridListCtrl::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if( GetFocus() != this) 
	{
		SetFocus();
	}

	CListCtrl::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CGridListCtrl::PreTranslateMessage(MSG* pMsg) 
{
	if(pMsg->message == WM_KEYDOWN)
	{
		if(GetFocus()==this)
		{
			switch( pMsg->wParam )
			{
				case VK_LEFT:
					{
						// Decrement the order number.
						m_CurSubItem--;
						if(m_CurSubItem < 0) 
						{
							m_CurSubItem = 0;
						}
						else
						{			
							CHeaderCtrl* pHeader = GetHeaderCtrl();
							// Make the column visible.
							// We have to take into account that the header may be reordered.
							MakeColumnVisible( Header_OrderToIndex( pHeader->m_hWnd, m_CurSubItem));
							// Invalidate the item.
							int iItem = GetSelectedItem();
							if( iItem != -1 )
							{
								CRect rcBounds;
								GetItemRect(iItem, rcBounds, LVIR_BOUNDS);
								InvalidateRect(&rcBounds);
								UpdateWindow();
							}
						}
					}
					return TRUE;

				case VK_RIGHT:
					{
						// Increment the order number.
						m_CurSubItem++;
						CHeaderCtrl* pHeader = GetHeaderCtrl();
						int nColumnCount = pHeader->GetItemCount();
						// Don't go beyond the last column.
						if( m_CurSubItem > nColumnCount -1 ) 
						{
							m_CurSubItem = nColumnCount-1;
						}
						else
						{
							MakeColumnVisible(Header_OrderToIndex( pHeader->m_hWnd, m_CurSubItem));
							 
							int iItem = GetSelectedItem();
							// Invalidate the item.
							if( iItem != -1 )
							{
								CRect rcBounds;
								GetItemRect(iItem, rcBounds, LVIR_BOUNDS);
								InvalidateRect(&rcBounds);
								UpdateWindow();
							}
						}
					}
					return TRUE;

				case VK_RETURN://edit itemdata
					{
						BOOL bResult = OnVkReturn();
						if(!bResult)
						{
							return 1;
						}
					}
					break;
				default:
					break;
			}
		}
	}
	return CListCtrl::PreTranslateMessage(pMsg);
}

CEdit* CGridListCtrl::EditLabelEx(int nItem, int nCol, BOOL bIsNumeric, BOOL bReduceSize /*= FALSE*/)
{
	CRect rect;
	int offset = 0;
	if(!EnsureVisible(nItem, TRUE)) 
	{
		return NULL;
	}

	GetSubItemRect(nItem, nCol, LVIR_BOUNDS, rect);
	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect(rcClient);
	if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
	{
		CSize size(offset + rect.left,0);		
		Scroll(size);
		rect.left -= size.cx;
	}
	rect.left += offset;	
	rect.right = rect.left + GetColumnWidth(nCol);
	if(rect.right > rcClient.right) 
	   rect.right = rcClient.right;

	if (bReduceSize)
	{
		rect.right -= GetSystemMetrics(SM_CYVSCROLL);
	}

	// Get Column alignment	
	LV_COLUMN lvcol;
	lvcol.mask = LVCF_FMT;
	GetColumn(nCol, &lvcol);

	DWORD dwStyle;
	if((lvcol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_LEFT)
		dwStyle = ES_LEFT;
	else if((lvcol.fmt & LVCFMT_JUSTIFYMASK) == LVCFMT_RIGHT)
		dwStyle = ES_RIGHT;
	else 
		dwStyle = ES_CENTER;	

	
	dwStyle |=WS_BORDER|WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL;

	if (bIsNumeric)
	{
		dwStyle |= ES_NUMBER;
	}

	CEdit *pEdit = CreateEditCtrl(nItem, nCol, GetItemText(nItem, nCol));
	pEdit->Create(dwStyle, rect, this, IDC_EDITCTRL);	
//	pEdit->SetUseMask(FALSE);

	return pEdit;
}

CEdit* CGridListCtrl::CreateEditCtrl(int nItem, int nCol, CString strLabel)
{
	return new CGridEditCtrl(nItem, nCol, strLabel);
}

void CGridListCtrl::OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult) 
{
	LV_DISPINFO *plvDispInfo = (LV_DISPINFO*)pNMHDR;
 	LV_ITEM *plvItem = &plvDispInfo->item;
	if (plvItem->pszText != NULL)//valid text
	{
		if(plvItem->iItem != -1) //valid item
		{
			CGridTreeItem *pSelItem = GetTreeItem(plvItem->iItem);
			if(pSelItem != NULL)
			{
				BOOL bGridUpdate = OnUpdateListViewItem(pSelItem, plvItem);
				if (pSelItem->m_lpNodeInfo && pSelItem->m_lpNodeInfo->CheckNotificationStatus() == CGridItemInfo::NTFY_CHILD)
				{
					pSelItem->m_pParent->NotifyOfChildChange(this);
					if (pSelItem->m_pParent->m_lpNodeInfo && 
						pSelItem->m_pParent->m_lpNodeInfo->CheckNotificationStatus() == CGridItemInfo::NTFY_BOTH)
					{
						//for double compounded object
						pSelItem->m_pParent->m_pParent->NotifyOfChildChange(this);
					}
				}
				else if (pSelItem->m_lpNodeInfo && pSelItem->m_lpNodeInfo->CheckNotificationStatus() == CGridItemInfo::NTFY_PARENT)
				{
					pSelItem->NotifyOfChange(this);
				}
				else if (pSelItem->m_lpNodeInfo && pSelItem->m_lpNodeInfo->CheckNotificationStatus() == CGridItemInfo::NTFY_BOTH)
				{
					pSelItem->NotifyOfChange(this);
					pSelItem->m_pParent->NotifyOfChildChange(this);
				}

                if (bGridUpdate) OnGridItemChange(pSelItem);
			}
		}
	}
	*pResult = 0;
}

int CGridListCtrl::GetColumnCount()
{
	CHeaderCtrl* pHeader = GetHeaderCtrl();
	return pHeader ? pHeader->GetItemCount() : 0;
}

void CGridListCtrl::MakeColumnVisible(int nCol)
{
	if(nCol < 0)
		return;
	// Get the order array to total the column offset.
	CHeaderCtrl* pHeader = GetHeaderCtrl();

	int nColCount = pHeader->GetItemCount();
	ASSERT( nCol < nColCount);
	int *pOrderarray = new int[nColCount];
	Header_GetOrderArray(pHeader->m_hWnd, nColCount, pOrderarray);
	// Get the column offset
	int offset = 0;
	for(int i = 0; pOrderarray[i] != nCol; i++)
		offset += GetColumnWidth(pOrderarray[i]);

	int colwidth = GetColumnWidth(nCol);
	delete[] pOrderarray;

	CRect rect;
	GetItemRect(0, &rect, LVIR_BOUNDS);
	// Now scroll if we need to show the column
	CRect rcClient;
	GetClientRect(&rcClient);
	if(offset + rect.left < 0 || offset + colwidth + rect.left > rcClient.right)
	{
		CSize size(offset + rect.left,0);
		Scroll(size);
		InvalidateRect(NULL);
		UpdateWindow();
	}
}

int CGridListCtrl::IndexToOrder( int iIndex )
{
	// This translates a column index value to a column order value.
	CHeaderCtrl* pHeader = GetHeaderCtrl();
	int nColCount = pHeader->GetItemCount();
	int *pOrderarray = new int[nColCount];
	Header_GetOrderArray(pHeader->m_hWnd, nColCount, pOrderarray);
	for(int i=0; i<nColCount; i++)
	{
		if(pOrderarray[i] == iIndex )
		{
			delete[] pOrderarray;
			return i;
		}
	}
	delete[] pOrderarray;
	return -1;
}

void CGridListCtrl::DrawFocusCell(CDC *pDC, int nItem, int iSubItem, BOOL bReadOnly)
{
	if(iSubItem==m_CurSubItem)
	{
		CRect rect;
		GetSubItemRect(nItem, iSubItem, LVIR_BOUNDS, rect);
		CBrush br( COLOR_DATA_SELECTED );
		if(iSubItem==0)
			GetItemRect(iSubItem, rect, LVIR_LABEL);
		if (!bReadOnly)
		{
			pDC->FillRect(rect, &br);
			rect.DeflateRect(1,1);
			pDC->DrawFocusRect(rect);
		}
	}
}

//insert item and return new parent pointer.
CGridTreeItem* CGridListCtrl::InsertItem(CGridTreeItem *pParent, CGridItemInfo* lpInfo,  BOOL bUpdate)
{
	if(pParent==NULL)
		return NULL;

	CGridTreeItem *pItem = NULL;
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
	return pItem;
}	

void CGridListCtrl::InternaleUpdateTree()
{
	int nItems = GetItemCount();
	for(int nItem=0; nItem < nItems; nItem++)
	{
		CGridTreeItem* pItem = GetTreeItem(nItem);
		SetCurIndex(pItem, nItem);
	}
}

int CGridListCtrl::NodeToIndex(CGridTreeItem *pNode)
{
	int nStartIndex=0;
	POSITION pos = m_RootItems.GetHeadPosition();
	while(pos!=NULL)
	{
		CGridTreeItem * root = (CGridTreeItem*)m_RootItems.GetNext(pos);
		int ret = _NodeToIndex(root, pNode, nStartIndex);
		if(ret != -1)
			return ret;
	}
	return -1;
}

CGridTreeItem* CGridListCtrl::GetRootItem(int nIndex)
{
	POSITION pos = m_RootItems.FindIndex(nIndex);
	if(pos==NULL)
		return NULL;
	return (CGridTreeItem*)m_RootItems.GetAt(pos);
}

int CGridListCtrl::GetRootIndex(CGridTreeItem * root)
{
	int nIndex = 0;
	POSITION pos = m_RootItems.GetHeadPosition();
	while(pos != NULL)
	{
		CGridTreeItem * pItem = (CGridTreeItem*)m_RootItems.GetNext(pos);
		if(pItem== root)
			return nIndex;
		nIndex++;
	}
	return -1;
}

BOOL CGridListCtrl::IsRoot(CGridTreeItem * lpItem)
{
	return m_RootItems.Find(lpItem) != NULL;
}

void CGridListCtrl::DeleteRootItem(CGridTreeItem * root)
{
	POSITION pos = m_RootItems.Find(root);
	if(pos!=NULL)
	{
		CGridTreeItem* pRoot=(CGridTreeItem*)m_RootItems.GetAt(pos);
		if(pRoot->m_lpNodeInfo!=NULL)
				delete pRoot->m_lpNodeInfo;
		delete pRoot;
		m_RootItems.RemoveAt(pos);
	}
}

CGridTreeItem*  CGridListCtrl::InsertRootItem(CGridItemInfo * lpInfo)
{
	if(lpInfo==NULL)
		lpInfo = new CGridItemInfo;

	CGridTreeItem* pRoot = NULL;
	
	pRoot =  new CGridTreeItem();

	CleanUp(pRoot);
	UpdateData(pRoot, lpInfo);
	SetIndent(pRoot, 1);
	SetCurIndex(pRoot, GetItemCount());
	SetParentItem(pRoot, NULL);

	CGridItemInfo* lp = GetData(pRoot);
	LV_ITEM lvItem;		
	lvItem.mask = LVIF_TEXT | LVIF_INDENT | LVIF_PARAM;
	CString strItem = lp->GetItemText();
	lvItem.pszText = strItem.GetBuffer(1); 
	lvItem.iItem = GetItemCount();
	lvItem.lParam = (LPARAM)pRoot;
	lvItem.iIndent = 1;
	lvItem.iSubItem = 0;
	CListCtrl::InsertItem(&lvItem);
	int nSize = lp->GetItemCount();
	for(int i=0; i < nSize;i++)
	{
	   CString str = lp->GetSubItem(i);
	   lvItem.mask = LVIF_TEXT;
	   lvItem.iSubItem = i+1;
	   lvItem.pszText = str.GetBuffer(1);
	   SetItem(&lvItem);
	}
	m_RootItems.AddTail(pRoot);
	return pRoot;
}

void CGridListCtrl::DrawTreeItem(CDC* pDC, CGridTreeItem* pSelItem, int nListItem, const CRect& rcBounds)
{
	int nColWidth = GetColumnWidth(0);
	int yDown = rcBounds.top;
    CPen* pPenTreeLine = pDC->SelectObject(&m_psTreeLine);
	int iIndent = GetIndent(pSelItem);
	int nHalfImage = (m_cxImage >> 1);
	int nBottomDown = yDown + nHalfImage + ((rcBounds.Height() - m_cyImage) >> 1);
	//
	BOOL bChild = ItemHasChildren(pSelItem);
	BOOL bCollapsed = IsCollapsed(pSelItem);

	//draw outline	
	while(1)
	{
		CGridTreeItem* pParent = GetParentItem(pSelItem);
		if(pParent==NULL)//no more parents, stop
			break;

		POSITION pos = pParent->m_listChild.GetTailPosition();
		while(pos!=NULL)
		{
			CGridTreeItem *pLastChild = (CGridTreeItem*)pParent->m_listChild.GetPrev(pos);
			int nIndex = GetCurIndex(pLastChild);
			int nCurIndent = GetIndent(pLastChild);
			if(nListItem > nIndex && iIndent > nCurIndent)//no need to go further in this loop
				break;

			//no drawing outside the 1st columns right
			int xLine =  rcBounds.left + (nCurIndent-1) * m_cxImage - nHalfImage;
			if(nIndex == nListItem && nCurIndent==iIndent)
			{
				//draw '-
				int x;
				pDC->MoveTo(xLine, yDown);
				pDC->LineTo(xLine, nBottomDown);
				// -
				xLine + nHalfImage > nColWidth ? x = nColWidth: x = xLine + nHalfImage + m_cxImage;
				
				pDC->MoveTo(xLine, nBottomDown);
				pDC->LineTo(x, nBottomDown);
				break;
			}
			else
			if(nIndex > nListItem && nCurIndent==iIndent)
			{
				//draw |-
				int x;
				xLine + nHalfImage > nColWidth ? x = nColWidth : x = xLine + nHalfImage + m_cxImage;
				pDC->MoveTo(xLine, nBottomDown);
				pDC->LineTo(x, nBottomDown);
				//-
				pDC->MoveTo(xLine, yDown);
				pDC->LineTo(xLine, rcBounds.bottom);
				break;
			}
			else
			if(nIndex > nListItem && nCurIndent < iIndent)
			{
				//draw |
				pDC->MoveTo(xLine, yDown);
				pDC->LineTo(xLine, rcBounds.bottom);
				break;
			}
		}			
		pSelItem = pParent;//next
	}

	//draw plus/minus sign
	if(bChild)
	{
		CGridTreeToggle rect(this, pDC, iIndent, rcBounds);

		rect.DrawRectangle(this);

		CPen* pPenPlusMinus = pDC->SelectObject(&m_psPlusMinus);
		if(bCollapsed)
			rect.DrawPlus();
		else {
			rect.DrawMinus();
			//draw line up to parent folder
			CPen* pLine = pDC->SelectObject(&m_psTreeLine);
			int nOffset = (rcBounds.Height() - m_cyImage)/2;
			pDC->MoveTo(rect.GetLeft(), rcBounds.top + m_cyImage+nOffset);
			pDC->LineTo(rect.GetLeft(), rcBounds.bottom);
			pDC->SelectObject(pLine);		
		}
		pDC->SelectObject(pPenPlusMinus);		
	}
	pDC->SelectObject(pPenTreeLine);
}

//walk all over the place setting the hide/show flag of the nodes.
//it also deletes items from the listviewctrl.
void CGridListCtrl::HideChildren(CGridTreeItem *pItem, BOOL bHide,int nItem)
{
	if(!IsCollapsed(pItem))
	if(ItemHasChildren(pItem))
	{
		Hide(pItem, bHide);
		POSITION pos = pItem->m_listChild.GetHeadPosition();
		while (pos != NULL)
		{
			HideChildren((CGridTreeItem *)pItem->m_listChild.GetNext(pos),bHide,nItem+1);
			DeleteItem(nItem);

		}
	}
}

void CGridListCtrl::Collapse(CGridTreeItem *pItem)
{
	if(pItem != NULL && ItemHasChildren(pItem))
	{
		SetRedraw(0);
		int nIndex = NodeToIndex(pItem);			
		HideChildren(pItem, TRUE, nIndex+1);
		InternaleUpdateTree();
		SetRedraw(1);
	}
}

void CGridListCtrl::ExpandAll(CGridTreeItem *pItem, int& nScroll)
{
	const int nChildren = pItem->m_listChild.GetCount();
	if (nChildren > 0)
	{
		int nIndex = NodeToIndex(pItem);
		nScroll = Expand(pItem, nIndex);
	}

	POSITION pos = pItem->m_listChild.GetHeadPosition();
	while (pos)
	{
		CGridTreeItem *pChild = (CGridTreeItem*)pItem->m_listChild.GetNext(pos);
		ExpandAll(pChild, nScroll);
	}
}

int CGridListCtrl::Expand(CGridTreeItem* pSelItem, int nIndex)
{
	if(ItemHasChildren(pSelItem) && IsCollapsed(pSelItem))
	{
		LV_ITEM lvItem;
		lvItem.mask = LVIF_INDENT;
		lvItem.iItem = nIndex;
		lvItem.iSubItem = 0;
		lvItem.lParam=(LPARAM)pSelItem;
		lvItem.iIndent = GetIndent(pSelItem);
		SetItem(&lvItem);

		Hide(pSelItem, FALSE);
		//expand children
		POSITION pos = pSelItem->m_listChild.GetHeadPosition();
		while(pos != NULL)
		{
			CGridTreeItem* pNextNode = (CGridTreeItem*)pSelItem->m_listChild.GetNext(pos);
			CString str = GetData(pNextNode)->GetItemText();
			LV_ITEM lvItem;
			lvItem.mask = LVIF_TEXT | LVIF_INDENT | LVIF_PARAM;
			lvItem.pszText =str.GetBuffer(1); 
			lvItem.iItem = nIndex + 1;
			lvItem.iSubItem = 0;
			lvItem.lParam=(LPARAM)pNextNode;
			lvItem.iIndent = GetIndent(pSelItem)+1;
			CListCtrl::InsertItem(&lvItem);

			//get subitems
			int nSize = GetData(pNextNode)->GetItemCount();
			for(int i=0; i< nSize;i++)
			{
			   CString str=GetData(pNextNode)->GetSubItem(i);
			   lvItem.mask = LVIF_TEXT;
			   lvItem.iSubItem = i+1;
			   lvItem.pszText=str.GetBuffer(1);
			   SetItem(&lvItem);
			}
			nIndex++;
		}
	}
	InternaleUpdateTree();
	return nIndex;
}

int CGridListCtrl::SelectNode(CGridTreeItem *pLocateNode)
{
	if(IsRoot(pLocateNode))
	{
		UINT uflag = LVIS_SELECTED | LVIS_FOCUSED;
		SetItemState(0, uflag, uflag);
		return 0;
	}

	int nSelectedItem=-1;
	CGridTreeItem* pNode = pLocateNode;
	CGridTreeItem* pTopLevelParent=NULL;
	//Get top parent
	while(1)
	{
		CGridTreeItem *pParent = GetParentItem(pLocateNode);
		if(IsRoot(pParent))
			break;
		pLocateNode = pParent;
	}
	pTopLevelParent = pLocateNode;//on top of all

	//Expand the folder
	if(pTopLevelParent != NULL)
	{
		SetRedraw(0);
		CWaitCursor wait;
		CGridTreeItem *pRoot = GetParentItem(pTopLevelParent);

		if(IsCollapsed(pRoot))
			Expand(pRoot,0);

		ExpandUntil(pTopLevelParent, pNode);

		UINT uflag = LVIS_SELECTED | LVIS_FOCUSED;
		nSelectedItem = NodeToIndex(pNode);

		SetItemState(nSelectedItem, uflag, uflag);

		SetRedraw(1);
//		EnsureVisible(nSelectedItem, TRUE);
	}
	return nSelectedItem;
}

void CGridListCtrl::ExpandUntil(CGridTreeItem *pItem, CGridTreeItem* pStopAt)
{
	const int nChildren = pItem->m_listChild.GetCount();
	if (nChildren > 0)
	{
		POSITION pos = pItem->m_listChild.GetHeadPosition();
		while (pos)
		{
			CGridTreeItem *pChild = (CGridTreeItem*)pItem->m_listChild.GetNext(pos);
			if(pChild == pStopAt)	
			{
				int nSize = GetIndent(pChild);
				CGridTreeItem** ppParentArray = new CGridTreeItem*[nSize];
				int i=0;
				while(1)
				{
					CGridTreeItem *pParent = GetParentItem(pChild);
					
					if(IsRoot(pParent))
						break;
					pChild = pParent;
					ppParentArray[i] = pChild;
					i++;
				}

				for(int x=i; x > 0; x--)
				{
					CGridTreeItem *pParent = ppParentArray[x-1];
					Expand(pParent, NodeToIndex(pParent));
				}
				delete [] ppParentArray;
				return;
			}
		}
	}

	POSITION pos = pItem->m_listChild.GetHeadPosition();
	while (pos)
	{
		CGridTreeItem *pChild = (CGridTreeItem*)pItem->m_listChild.GetNext(pos);
		ExpandUntil(pChild, pStopAt);
	}
	
}

void CGridListCtrl::DeleteItemEx(CGridTreeItem *pSelItem, int nItem)
{
	SetRedraw(0);
	DeleteItem(nItem);//delete cur item in listview
	//delete/hide all children in pSelItem
	HideChildren(pSelItem, TRUE, nItem);
	//delete all internal nodes
	// If root, must delete from m_rootData
	if(GetParentItem(pSelItem) == NULL )
	{
		DeleteRootItem(pSelItem);
	}
	else
	{
		Delete(pSelItem);
	}

	InternaleUpdateTree();
	if(nItem-1<0)//no more items in list
	{
		SetRedraw(1); 
		InvalidateRect(NULL);
		UpdateWindow();
		return;
	}

	UINT uflag = LVIS_SELECTED | LVIS_FOCUSED;
	//Test If Item Is Valid To Select Otherwise Subtrack One From Item
	CRect rcTest;
	GetItemRect(nItem, rcTest ,LVIR_LABEL) ? SetItemState(nItem, uflag, uflag) : SetItemState(nItem-1, uflag, uflag);
	
	SetRedraw(1);
	InvalidateRect(NULL);
	UpdateWindow();
}

void CGridListCtrl::CleanUp(CGridTreeItem *pItem)
{
	// delete child nodes
	POSITION pos = pItem->m_listChild.GetHeadPosition();
	while (pos != NULL)
	{
		CGridTreeItem* pItemData = (CGridTreeItem*)pItem->m_listChild.GetNext(pos);
		if(pItemData!=NULL)
		{
			if(pItemData->m_lpNodeInfo!=NULL)
				delete pItemData->m_lpNodeInfo;

			pItemData->m_listChild.RemoveAll();
			delete pItemData;
		}
	}
	pItem->m_listChild.RemoveAll();
}

CGridTreeItem* CGridListCtrl::GetNext(CGridTreeItem* pStartAt, CGridTreeItem* pNode, BOOL bInit, BOOL bDontIncludeHidden)
{
	static BOOL bFound;
	if (bInit)
	{
		bFound = FALSE;
	}
		
	if (pNode == pStartAt)
	{
		bFound = TRUE;
	}

	if(bDontIncludeHidden)
	{
		if (!IsCollapsed(pStartAt))
		{
			POSITION pos = pStartAt->m_listChild.GetHeadPosition();
			while (pos != NULL)
			{
				CGridTreeItem* pChild = (CGridTreeItem*)pStartAt->m_listChild.GetNext(pos);
				if (bFound)
					return pChild;
				pChild = GetNext(pChild, pNode, FALSE, TRUE);
				if (pChild != NULL)
					return pChild;
			}
		}
	}
	else 
	{
		POSITION pos = pStartAt->m_listChild.GetHeadPosition();
		while (pos != NULL)
		{
			CGridTreeItem* pChild = (CGridTreeItem*)pStartAt->m_listChild.GetNext(pos);
			if (bFound)
				return pChild;
			pChild = GetNext(pChild, pNode, FALSE,FALSE);
			if (pChild != NULL)
				return pChild;
		}
	}

	// if reached top and last level return original
	if (bInit)
		return pNode;
	else
		return NULL;
}

CGridTreeItem* CGridListCtrl::GetPrev(CGridTreeItem* pStartAt, CGridTreeItem* pNode, BOOL bInit, BOOL bDontIncludeHidden)
{
	static CGridTreeItem* pPrev;
	if (bInit)
	{
		pPrev = pStartAt;
	}

	if (pNode == pStartAt)
	{
		return pPrev;
	}

	pPrev = pStartAt;

	if(bDontIncludeHidden)
	{
		if (!IsCollapsed(pStartAt))
		{
			POSITION pos = pStartAt->m_listChild.GetHeadPosition();
			while (pos != NULL)
			{
				CGridTreeItem* pCur = (CGridTreeItem*)pStartAt->m_listChild.GetNext(pos);
				CGridTreeItem* pChild = GetPrev(pCur,pNode, FALSE,TRUE);
				if (pChild != NULL)
				{
					return pChild;
				}
			}
		}
	}
	else 
	{
		POSITION pos = pStartAt->m_listChild.GetHeadPosition();
		while (pos != NULL)
		{
			CGridTreeItem* pCur = (CGridTreeItem*)pStartAt->m_listChild.GetNext(pos);
			CGridTreeItem* pChild = GetPrev(pCur,pNode, FALSE,FALSE);
			if (pChild != NULL)
			{
				return pChild;
			}
		}
	}

	if (bInit)
		return pPrev;
	else
		return NULL;
}

int CGridListCtrl::_NodeToIndex(CGridTreeItem *pStartpos, CGridTreeItem* pNode, int& nIndex, BOOL binit)
{
	static BOOL bFound;	
	// Account for other root nodes
	if(GetParentItem(pStartpos) == NULL && GetRootIndex(pStartpos) !=0)
		nIndex++;

	if(binit)
		bFound=FALSE;

	if(pStartpos==pNode)
		bFound=TRUE;

	if(!IsCollapsed(pStartpos))
	{
		POSITION pos = GetHeadPosition(pStartpos);
		while (pos)
		{
			CGridTreeItem *pChild = GetNextChild(pStartpos, pos);
			if(bFound)
				return nIndex;

			_NodeToIndex(pChild, pNode, nIndex, binit);
			nIndex++;
		}
	}
	if(binit && bFound)
		return nIndex;
	else
		return -1;
}


BOOL CGridListCtrl::Delete(CGridTreeItem* pNode, BOOL bClean)
{
	POSITION pos = m_RootItems.GetHeadPosition();
	while(pos!=NULL)
	{
		CGridTreeItem * pRoot = (CGridTreeItem*)m_RootItems.GetNext(pos);
		if(_Delete(pRoot, pNode, bClean))
			return TRUE;
	}
	return FALSE;
}

BOOL CGridListCtrl::_Delete(CGridTreeItem* pStartAt, CGridTreeItem* pNode, BOOL bClean)
{
	POSITION pos = pStartAt->m_listChild.GetHeadPosition();
	while (pos != NULL)
	{
		POSITION posPrev = pos;
		CGridTreeItem *pChild = (CGridTreeItem*)pStartAt->m_listChild.GetNext(pos);
		if (pChild == pNode)
		{
			pStartAt->m_listChild.RemoveAt(posPrev);
			if(bClean)
			{
				if(GetData(pNode)!=NULL)
					delete GetData(pNode);
				delete pNode;
			}
			return TRUE;
		}
		if (_Delete(pChild, pNode) == TRUE)
			return TRUE;
	}
	return FALSE;
}

BOOL CGridListCtrl::IsChildOf(const CGridTreeItem* pParent, const CGridTreeItem* pChild) const
{
	if (pChild == pParent)
		return TRUE;
	POSITION pos = pParent->m_listChild.GetHeadPosition();
	while (pos != NULL)
	{
		CGridTreeItem* pNode = (CGridTreeItem*)pParent->m_listChild.GetNext(pos);
		if (IsChildOf(pNode, pChild))
			return TRUE;
	}
	return FALSE;
}

void CGridListCtrl::OnLButtonDown(UINT nFlags, CPoint point) 
{
	if( GetFocus() != this) 
		SetFocus();

	LVHITTESTINFO ht;
	ht.pt = point;
	SubItemHitTest(&ht);
	if(OnItemLButtonDown(ht))
	{
		BOOL bSelect=1;
		bSelect = HitTestOnSign(point, ht);
		if(bSelect && ht.iItem!=-1)
		{
			m_CurSubItem = IndexToOrder(ht.iSubItem);
			CHeaderCtrl* pHeader = GetHeaderCtrl();
			// Make the column fully visible.
			MakeColumnVisible(Header_OrderToIndex(pHeader->m_hWnd, m_CurSubItem));
			CListCtrl::OnLButtonDown(nFlags, point);
			OnControlLButtonDown(nFlags, point, ht);
			//update row anyway for selection bar
			CRect rc;
			GetItemRect(ht.iItem, rc, LVIR_BOUNDS);
			InvalidateRect(rc);
			UpdateWindow();
		}
	}
}

BOOL CGridListCtrl::OnUpdateListViewItem(CGridTreeItem* lpItem, LV_ITEM *plvItem)
{
	//default implementation you would go for this 9 out of 10 times
	CGridItemInfo *lp = GetData(lpItem);
	CString str = (CString)plvItem->pszText;
	if(lp!=NULL)
	{
		if(plvItem->iSubItem==0)
			lp->SetItemText(str);
		else //subitem data 
			lp->SetSubItemText(plvItem->iSubItem-1, str);
	   UpdateData(lpItem, lp);
	}
	SetItemText(plvItem->iItem, plvItem->iSubItem, plvItem->pszText);

    return TRUE;
}

void CGridListCtrl::DeleteAll()
{
    if (GetCount()>0)
    {
        EnsureVisible(0,FALSE);
    }

	POSITION pos = m_RootItems.GetHeadPosition();
	while(pos!=NULL)
	{
		CGridTreeItem * pRoot = (CGridTreeItem*)m_RootItems.GetNext(pos);
		if(pRoot!=NULL)
			DeleteItemEx(pRoot, 0);
	}
}

POSITION CGridListCtrl::GetRootHeadPosition() const
{
	return m_RootItems.GetHeadPosition();
}

POSITION CGridListCtrl::GetRootTailPosition() const
{
	return m_RootItems.GetTailPosition();
}

CGridTreeItem* CGridListCtrl::GetNextRoot(POSITION& pos) const
{
	return (CGridTreeItem*)m_RootItems.GetNext(pos);
}

CGridTreeItem* CGridListCtrl::GetPrevRoot(POSITION& pos) const
{
	return (CGridTreeItem*)m_RootItems.GetNext(pos);
}

POSITION CGridListCtrl::GetHeadPosition(CGridTreeItem* pItem) const
{
	return pItem->m_listChild.GetHeadPosition();
}

CGridTreeItem* CGridListCtrl::GetNextChild(CGridTreeItem *pItem, POSITION& pos) const
{
	return (CGridTreeItem*)pItem->m_listChild.GetNext(pos);
}

CGridTreeItem* CGridListCtrl::GetPrevChild(CGridTreeItem *pItem, POSITION& pos) const
{
	return (CGridTreeItem*)pItem->m_listChild.GetPrev(pos);
}

POSITION CGridListCtrl::GetTailPosition(CGridTreeItem *pItem) const
{
	return pItem->m_listChild.GetTailPosition();
}

void CGridListCtrl::AddTail(CGridTreeItem *pParent, CGridTreeItem *pChild)
{
	pParent->m_listChild.AddTail(pChild);
}

int CGridListCtrl::CompareChildren(const void* p1, const void* p2)
{
	CGridTreeItem * pChild1 = *(CGridTreeItem**)p1;
	CGridTreeItem * pChild2 = *((CGridTreeItem**)p2);
	CGridItemInfo *pItem1=(*pChild1).m_lpNodeInfo;
	CGridItemInfo *pItem2=(*pChild2).m_lpNodeInfo;
	return StrComp(&(pItem1->GetItemText()), &(pItem2->GetItemText()));
}

void CGridListCtrl::Sort(CGridTreeItem* pParent, BOOL bSortChildren)
{
	const int nChildren = NumChildren(pParent);
	if (nChildren > 1)
	{
		CGridTreeItem** ppSortArray = new CGridTreeItem*[nChildren];
		// Fill in array with pointers to our children.
		POSITION pos = pParent->m_listChild.GetHeadPosition();
		for (int i=0; pos; i++)
		{
			ASSERT(i < nChildren);
			ppSortArray[i] = (CGridTreeItem*)pParent->m_listChild.GetAt(pos);
			pParent->m_listChild.GetNext(pos);
		}

		qsort(ppSortArray, nChildren, sizeof(CGridTreeItem*), CompareChildren);
		// reorg children with new sorted list
		pos = pParent->m_listChild.GetHeadPosition();
		for (i=0; pos; i++)
		{
			ASSERT(i < nChildren);
			pParent->m_listChild.SetAt(pos, ppSortArray[i]);
			pParent->m_listChild.GetNext(pos);
		}

		delete [] ppSortArray;
	}

	if(bSortChildren)
	{
		POSITION pos = pParent->m_listChild.GetHeadPosition();
		while (pos)
		{
			CGridTreeItem *pChild = (CGridTreeItem*)pParent->m_listChild.GetNext(pos);
			Sort(pChild, TRUE);
		}
	}
}

int CGridListCtrl::NumChildren(const CGridTreeItem *pItem) const
{
	return pItem->m_listChild.GetCount();
}

BOOL CGridListCtrl::ItemHasChildren(const CGridTreeItem* pItem) const
{ 
	BOOL bChildren = pItem->m_listChild.GetCount() != 0;
	//see if we have a flag
	int nFlag = pItem->m_bSetChildFlag;
	if(nFlag!=-1)
		return 1;
	else
		return bChildren;
}

void CGridListCtrl::SetChildrenFlag(CGridTreeItem *pItem, int nFlag) const
{
	pItem->m_bSetChildFlag = nFlag;
}

BOOL CGridListCtrl::IsCollapsed(const CGridTreeItem* pItem) const
{
	return pItem->m_bHideChildren;//e.g not visible
}

void CGridListCtrl::Hide(CGridTreeItem* pItem, BOOL bFlag)
{
	pItem->m_bHideChildren=bFlag;
}

int CGridListCtrl::GetIndent(const CGridTreeItem* pItem) const
{
	return pItem->m_nIndent;
}

void CGridListCtrl::SetIndent(CGridTreeItem *pItem, int iIndent)
{
	pItem->m_nIndent = iIndent;
}

int CGridListCtrl::GetCurIndex(const CGridTreeItem *pItem) const
{
	return pItem->m_nIndex;
}

void CGridListCtrl::SetCurIndex(CGridTreeItem* pItem, int nIndex) 
{
	pItem->m_nIndex = nIndex;
}

void CGridListCtrl::SetParentItem(CGridTreeItem*pItem, CGridTreeItem* pParent)
{
	pItem->m_pParent=pParent;

}

CGridTreeItem* CGridListCtrl::GetParentItem(const CGridTreeItem* pItem) 
{
	return pItem->m_pParent;
};

CGridItemInfo* CGridListCtrl::GetData(const CGridTreeItem* pItem) 
{
	return pItem->m_lpNodeInfo;
}

void CGridListCtrl::UpdateData(CGridTreeItem* pItem, CGridItemInfo* lpInfo)
{
	pItem->m_lpNodeInfo = lpInfo;
}

//overrides
CGridItemInfo* CGridListCtrl::CopyData(CGridItemInfo* lpSrc)
{
	ASSERT(FALSE);
	return NULL;  
}


void CGridListCtrl::OnControlLButtonDown(UINT nFlags, CPoint point, LVHITTESTINFO& ht)
{
	CGridTreeItem *pSelItem = GetTreeItem(ht.iItem);
	if(pSelItem!=NULL)
	{	
		CGridItemInfo* pInfo = GetData(pSelItem);
		CGridItemInfo::CONTROLTYPE ctrlType;
		if (pInfo && (pInfo->IsReadOnly() || pInfo->IsHeader()))
		{
			return; //readonly, do nothing
		}
		else if (pInfo)
		{	
			pInfo->GetControlType(ht.iSubItem-1, ctrlType);
			switch(ctrlType)
			{
				case pInfo->CONTROLTYPE::GCT_COLOR_BUTTON:
					{
						CRect rect;
						GetSubItemRect(ht.iItem, ht.iSubItem, LVIR_BOUNDS, rect);
						rect.left=rect.right - GetSystemMetrics(SM_CYVSCROLL);
						if (rect.PtInRect(point))
						{
							OnButtonColorPick(ht.iItem,ht.iSubItem);
						}
						else if(ht.iSubItem!=0)
						{
							EditLabelEx(ht.iItem, ht.iSubItem, FALSE, TRUE);	
						}
					}
					break;
				case pInfo->CONTROLTYPE::GCT_EXTERNAL:
				case pInfo->CONTROLTYPE::GCT_BUTTON:
					{
						CRect rect;
						GetSubItemRect(ht.iItem, ht.iSubItem, LVIR_BOUNDS, rect);
						if (rect.PtInRect(point))
						{
							OnGridItemChange(pSelItem);
						}
					}
					break;
				case pInfo->CONTROLTYPE::GCT_DIR_BUTTON:
					{
						CRect rect;
						GetSubItemRect(ht.iItem, ht.iSubItem, LVIR_BOUNDS, rect);
						rect.left=rect.right - GetSystemMetrics(SM_CYVSCROLL);
						if (rect.PtInRect(point))
						{
							OnButtonBrowse(ht.iItem,ht.iSubItem);
						}
						else if(ht.iSubItem!=0)
						{
							EditLabelEx(ht.iItem, ht.iSubItem, FALSE, TRUE);	
						}
					}
					break;
				case pInfo->CONTROLTYPE::GCT_COMBOBOX:
					{
						CStringList* list=NULL;
						pInfo->GetListData(ht.iSubItem-1, list);
						CComboBox * pList = ShowList(ht.iItem, ht.iSubItem, list);
					}
					break;
				case pInfo->CONTROLTYPE::GCT_FLOAT_EDIT:
				case pInfo->CONTROLTYPE::GCT_ROTATION_EDIT:
				case pInfo->CONTROLTYPE::GCT_3D_COORDINATE:
				case pInfo->CONTROLTYPE::GCT_STRING_EDIT:
				case pInfo->CONTROLTYPE::GCT_NUMERIC_EDIT:
				case pInfo->CONTROLTYPE::GCT_DEGREE_EDIT:
				case pInfo->CONTROLTYPE::GCT_GUID_EDIT:
					{
						// don't edit the first column
						if(ht.iSubItem!=0)
						{
							EditLabelEx(ht.iItem, ht.iSubItem, FALSE);	
						}
					}
					break;
                case pInfo->CONTROLTYPE::GCT_BOOL:
                    {
                        CString strValue = pInfo->GetSubItem(ht.iSubItem-1);
                        if (strValue.CompareNoCase("true")==0)
                            strValue = "false";
                        else
                            strValue = "true";
                        pInfo->SetSubItemText(ht.iSubItem-1,strValue);
                        SetItemText(ht.iItem, ht.iSubItem, strValue);
                        OnGridItemChange(pSelItem);
                    }
                    break;
                case pInfo->CONTROLTYPE::GCT_NULL_ENTRY:
				case pInfo->CONTROLTYPE::GCT_BOUNDING_BOX:
					//do nothing
					break;
			}
		}								
	}
}
	
CComboBox* CGridListCtrl::ShowList(int nItem, int nCol, CStringList *lstItems)
{
	CString strFind = GetItemText(nItem, nCol);

	//basic code start
	CRect rect;
	int offset = 0;
	// Make sure that the item is visible
	if( !EnsureVisible(nItem, TRUE)) return NULL;
	GetSubItemRect(nItem, nCol, LVIR_BOUNDS, rect);
	// Now scroll if we need to expose the column
	CRect rcClient;
	GetClientRect(rcClient);
	if( offset + rect.left < 0 || offset + rect.left > rcClient.right )
	{
		CSize size;
		size.cx = offset + rect.left;
		size.cy = 0;
		Scroll(size);
		rect.left -= size.cx;
	}
	
	rect.left += offset;	
	rect.right = rect.left + GetColumnWidth(nCol);
	if(rect.right > rcClient.right) 
	   rect.right = rcClient.right;
	//basic code end

	int iMult = lstItems->GetCount();
	if (iMult > 5) iMult = 5;
	rect.bottom += rect.Height() * (iMult+1);//dropdown area
	
	DWORD dwStyle =  WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST | CBS_DISABLENOSCROLL;
	CComboBox *pList = new CGridComboBox(nItem, nCol, lstItems);
	pList->Create(dwStyle, rect, this, IDC_COMBOBOXINLISTVIEW);
	pList->ModifyStyleEx(0,WS_EX_CLIENTEDGE);//can we tell at all
	pList->SetHorizontalExtent(CalcHorzExtent(pList, lstItems));
	pList->ShowDropDown();
	pList->SelectString(-1, strFind.GetBuffer(1));
	// The returned pointer should not be saved
	return pList;
}


int CGridListCtrl::CalcHorzExtent(CWnd* pWnd, CStringList *pList)
{
	int nExtent=0;
	if(pWnd!=NULL)
	{
		CDC* pDC = pWnd->GetDC();
		HFONT hFont = (HFONT)pWnd->SendMessage(WM_GETFONT); //why not pWnd->GetFont();..I like the send thing alot and
		CFont *pFont = CFont::FromHandle(hFont);			//this way I get to use this function..cool :)
		if(pFont!=NULL)										//ya what ever makes me happy,.right :}
		{
			CFont* pOldFont = pDC->SelectObject(pFont);
			CSize newExtent;
			POSITION pos = pList->GetHeadPosition();
			while(pos != NULL)
			{
				CString str(pList->GetNext(pos));
				newExtent = pDC->GetTextExtent(str);
				newExtent.cx += 6;
				if (newExtent.cx > nExtent)
				{
					nExtent = newExtent.cx;
				}
			}
			pDC->SelectObject(pOldFont);
		}
		pWnd->ReleaseDC(pDC);
	}
	return nExtent;
}
	
BOOL CGridListCtrl::OnVKMultiply(CGridTreeItem *pItem, int nIndex)
{
	return 1;
}

BOOL CGridListCtrl::OnVkSubTract(CGridTreeItem *pItem, int nIndex)
{
	return 1;
}

BOOL CGridListCtrl::OnVKAdd(CGridTreeItem *pItem, int nIndex)
{
	return 1;
}

BOOL CGridListCtrl::OnDeleteItem(CGridTreeItem* pItem, int nIndex)
{
	return 1;
}

BOOL CGridListCtrl::OnItemExpanding(CGridTreeItem *pItem, int iItem)
{
	return 1;
}

BOOL CGridListCtrl::OnItemExpanded(CGridTreeItem* pItem, int iItem)
{
	return 1;
}

BOOL CGridListCtrl::OnCollapsing(CGridTreeItem *pItem)
{
	return 1;
}

BOOL CGridListCtrl::OnItemCollapsed(CGridTreeItem *pItem)
{
	return 1;
}

BOOL CGridListCtrl::OnItemLButtonDown(LVHITTESTINFO& ht)
{
	return 1;
}

BOOL CGridListCtrl::OnVkReturn()
{
	BOOL bResult=FALSE;
	
	int iItem = GetNextItem( -1, LVNI_ALL | LVNI_SELECTED);
	if( GetCurSubItem() != -1 && iItem != -1)
	{
		CGridTreeItem *pSelItem = GetTreeItem(iItem);
		if(pSelItem!=NULL)
		{	
			CHeaderCtrl* pHeader = (CHeaderCtrl*)GetDlgItem(0);
			int iSubItem = Header_OrderToIndex(pHeader->m_hWnd, GetCurSubItem());
			CGridItemInfo* pInfo = GetData(pSelItem);
			CGridItemInfo::CONTROLTYPE ctrlType;
			
			if (pInfo && (pInfo->IsReadOnly() || pInfo->IsHeader()))
			{
				bResult=TRUE; //readonly
			}
			else if(pInfo)
			{	
				pInfo->GetControlType(iSubItem-1, ctrlType);
				switch(ctrlType)
				{
					case pInfo->CONTROLTYPE::GCT_COLOR_BUTTON:
						OnButtonColorPick(iItem,iSubItem);
						bResult=TRUE; 
						break;
					case pInfo->CONTROLTYPE::GCT_EXTERNAL:
					case pInfo->CONTROLTYPE::GCT_BUTTON:
						{
							OnGridItemChange(pSelItem);
							break;
						}
					case pInfo->CONTROLTYPE::GCT_DIR_BUTTON:
						{
							OnButtonBrowse(iItem,iSubItem);
							bResult=TRUE; 
							break;
						}
					case pInfo->CONTROLTYPE::GCT_COMBOBOX: 
						{
							CStringList* list=NULL;
							pInfo->GetListData(iSubItem-1, list);
							CComboBox * pList = ShowList(iItem, iSubItem, list);
							bResult=TRUE; 
						}
						break;
					case pInfo->CONTROLTYPE::GCT_ROTATION_EDIT:
					case pInfo->CONTROLTYPE::GCT_NUMERIC_EDIT: 
					case pInfo->CONTROLTYPE::GCT_3D_COORDINATE:
					case pInfo->CONTROLTYPE::GCT_STRING_EDIT: 
					case pInfo->CONTROLTYPE::GCT_FLOAT_EDIT:
					case pInfo->CONTROLTYPE::GCT_DEGREE_EDIT:
	    			case pInfo->CONTROLTYPE::GCT_GUID_EDIT:
						{
							int iItem = GetSelectedItem();
							if( m_CurSubItem != -1 && iItem != -1)
							{
								CHeaderCtrl* pHeader = GetHeaderCtrl();
								int iSubItem = Header_OrderToIndex(pHeader->m_hWnd, m_CurSubItem);
								// don't edit tree column
								if(iSubItem!=0)
								{
									bResult=TRUE; 
									EditLabelEx(iItem, iSubItem, FALSE);	
								}
							}
						}
						break;
                    case pInfo->CONTROLTYPE::GCT_BOOL:
                        {
                            CString strValue = pInfo->GetSubItem(iSubItem-1);
                            if (strValue.CompareNoCase("true")==0)
                                strValue = "false";
                            else
                                strValue = "true";
                            pInfo->SetSubItemText(iSubItem-1,strValue);
                            SetItemText(iItem, iSubItem, strValue);
                            OnGridItemChange(pSelItem);
                        }
                    break;
                    case pInfo->CONTROLTYPE::GCT_NULL_ENTRY:
					case pInfo->CONTROLTYPE::GCT_BOUNDING_BOX:
						//do nothing
						break;
					default:
						break;
				}
			}
		}
	}
	return bResult;
}

void CGridListCtrl::OnSysColorChange() 
{
	CListCtrl::OnSysColorChange();
}

UINT CGridListCtrl::_GetCount(CGridTreeItem* pItem, UINT& nCount)
{
	POSITION pos = pItem->m_listChild.GetHeadPosition();
	while (pos)
	{
		CGridTreeItem *pChild = (CGridTreeItem*)pItem->m_listChild.GetNext(pos);
		nCount = _GetCount(pChild, nCount);
		nCount++;				
	}
	return nCount;
}

UINT CGridListCtrl::GetCount(void) 
{
	UINT nCount=0;
	UINT _nCount=0;
	POSITION pos = m_RootItems.GetHeadPosition();
	while(pos!=NULL)
	{
		CGridTreeItem * pRoot = (CGridTreeItem*)m_RootItems.GetNext(pos);
		nCount += _GetCount(pRoot, _nCount) + 1;
	}
	return nCount;
}

CGridTreeItem* CGridListCtrl::GetTreeItem(int nIndex  ) 
{
	return reinterpret_cast<CGridTreeItem*>(GetItemData(nIndex));
}

int CGridListCtrl::GetSelectedItem(void) const
{
	return GetNextItem(-1, LVNI_ALL | LVNI_SELECTED); 
}

//////////////////////////////////////////////////////////////////////////
//
// Simple class CGridTreeToggle for the + - sign, 
//
//////////////////////////////////////////////////////////////////////////

CGridTreeToggle::CGridTreeToggle(CGridListCtrl* pCtrl, CDC* pDC, int iIndent, const CRect& rcBounds)
{
	m_pDC=pDC;
	int nHalfImage = (pCtrl->m_cxImage >> 1);
	int nBottomDown = rcBounds.top + nHalfImage + ((rcBounds.Height() - pCtrl->m_cyImage) >> 1);
	m_right_bottom.cx = (pCtrl->m_cxImage>>1)+2+1;
	m_right_bottom.cy = (pCtrl->m_cyImage>>1)+2+1;
	m_left = rcBounds.left  + iIndent * pCtrl->m_cxImage - nHalfImage;
	m_top = nBottomDown - (m_right_bottom.cy >> 1);
	m_left_top.x = m_left -  (m_right_bottom.cx >> 1);
	m_left_top.y = m_top;
	m_topdown = nBottomDown;
}
	
void CGridTreeToggle::DrawRectangle(CGridListCtrl* pCtrl)
{
	//erase bkgrnd
	CRect rc(m_left_top, m_right_bottom);
	m_pDC->FillRect(rc, &pCtrl->m_brushErase);
	//draw rectangle	
	CPen* pPenRectangle = m_pDC->SelectObject(&pCtrl->m_psRectangle);
	m_pDC->Rectangle(rc);
	m_pDC->SelectObject(pPenRectangle);		
}

CGridTreeToggle::~CGridTreeToggle()
{
}

BOOL CGridTreeToggle::HitTest(CPoint pt)
{
	CRect rc = GetHitTestRect();
	return rc.PtInRect(pt);
}

void CGridTreeToggle::DrawPlus(void)
{
	m_pDC->MoveTo(m_left, m_topdown-2);
	m_pDC->LineTo(m_left, m_topdown+3);

	m_pDC->MoveTo(m_left-2, m_topdown);
	m_pDC->LineTo(m_left+3, m_topdown);
}

void CGridTreeToggle::DrawMinus(void)
{
	m_pDC->MoveTo(m_left-2, m_topdown);
	m_pDC->LineTo(m_left+3, m_topdown);
}

