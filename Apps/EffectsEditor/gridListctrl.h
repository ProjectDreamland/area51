#if !defined(AFX_GridListCtrl_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_)
#define AFX_GridListCtrl_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// GridListCtrl.h : header file
//
#include "GridItemInfo.h"
#include "GridTreeItem.h"

/////////////////////////////////////////////////////////////////////////////
//
// CGridListCtrl 
//
/////////////////////////////////////////////////////////////////////////////
class CGridListCtrl : public CListCtrl
{
// Construction
public:
	CGridListCtrl();
	virtual ~CGridListCtrl();

	//used in drag/drop operations
	virtual CGridItemInfo* CopyData(CGridItemInfo* lpSrc);
	//override this to provide your dragimage
	virtual CImageList *CreateDragImageEx(int nItem);

	//override this to update listview items, called from within OnEndlabeledit.
	virtual BOOL OnUpdateListViewItem(CGridTreeItem* lpItem, LV_ITEM *plvItem);
	//override this to activate any control when lButtonDown in a cell, called from within OnLButtonDown
	virtual void OnControlLButtonDown(UINT nFlags, CPoint point, LVHITTESTINFO& ht);

    //called before item is about to explode, return TRUE to continue, FALSE to prevent expanding
	virtual BOOL OnItemExpanding(CGridTreeItem *pItem, int iItem);
	//called after item has expanded
	virtual BOOL OnItemExpanded(CGridTreeItem* pItem, int iItem);
	//called before item are collapsed,return TRUE to continue, FALSE to prevent collapse
	virtual BOOL OnCollapsing(CGridTreeItem *pItem);
	//called after item has collapsed
	virtual BOOL OnItemCollapsed(CGridTreeItem *pItem);
	//called before item is about to be deleted,return TRUE to continue, FALSE to prevent delete item
	virtual BOOL OnDeleteItem(CGridTreeItem* pItem, int nIndex);
	//called before VK_MULTIPLY keydown, return TRUE to continue, FALSE to prevent expandall
	virtual BOOL OnVKMultiply(CGridTreeItem *pItem, int nIndex);
	//called before VK_SUBTRACT keydown, return TRUE to continue, FALSE to prevent collapse item
	virtual BOOL OnVkSubTract(CGridTreeItem *pItem, int nIndex);
	//called before VK_ADD keydown, return TRUE to continue, FALSE to prevent expanding item
	virtual BOOL OnVKAdd(CGridTreeItem *pItem, int nIndex);
	//called from PreTranslateMessage, override this to handle other controls than editctrl's
	virtual BOOL OnVkReturn(void);
	//called before from within OnlButtonDown and OnDblclk, but before anything happens, return TRUE to continue, FALSE to say not selecting the item
	virtual BOOL OnItemLButtonDown(LVHITTESTINFO& ht);
	//browse button handler
	virtual void OnButtonBrowse(int iItem, int iSubItem);
	//color button handler
	virtual void OnButtonColorPick(int iItem, int iSubItem);
	//handle calling out
	virtual void OnGridItemChange(CGridTreeItem *pSelItem) {}

	//root
	BOOL IsRoot(CGridTreeItem * lpItem);
	int GetRootCount() { return m_RootItems.GetCount();}
	//creates a root
	CGridTreeItem*  InsertRootItem(CGridItemInfo * lpRoot);
	//deletes a rootitem
	void DeleteRootItem(CGridTreeItem * lpRoot);
	//given the rootindex it returns the rootptr
	CGridTreeItem* GetRootItem(int nIndex);

	//call this to delete all items in grid
	void DeleteAll();
	//return previous node from pItem, given a RootItem
	CGridTreeItem* GetPrev(CGridTreeItem* pRoot, CGridTreeItem *pItem, BOOL bInit = TRUE, BOOL bDontIncludeHidden=TRUE);
	//return next node from pItem, given a RootItem
	CGridTreeItem* GetNext(CGridTreeItem* pRoot, CGridTreeItem* pNode, BOOL bInit = TRUE, BOOL bDontIncludeHidden=TRUE);
	//returns the selected item :)
	int GetSelectedItem(void) const;
	//returns the itemdata associated with the grid
	CGridTreeItem* GetTreeItem(int nIndex);
	// Retrieves the number of items associated with the Grid control.
	UINT GetCount(void);
	//returns number of children given the pItem node ptr.
	int NumChildren(const CGridTreeItem *pItem) const;
	//Determines if this tree item is a child of the specified parent
	BOOL IsChildOf(const CGridTreeItem* pParent, const CGridTreeItem* pChild) const;
	//got children?
	BOOL ItemHasChildren(const CGridTreeItem* pItem) const;
	// Use this to indicate that pItem has children, but has not been inserted yet.
	void SetChildrenFlag(CGridTreeItem *pItem,int nFlag) const;
	//are children collapsed
	BOOL IsCollapsed(const CGridTreeItem* pItem) const;
	//return the Indent Level of pItem
	int GetIndent(const CGridTreeItem* pItem) const;
	//Sets the Indentlevel of pItem
	void SetIndent(CGridTreeItem *pItem, int iIndent);
	//get the pItems current listview index, 
	int GetCurIndex(const CGridTreeItem *pItem) const;
	//set pItems current listview index
	void SetCurIndex(CGridTreeItem* pItem, int nIndex);
	//sets the pItem' parent
	void SetParentItem(CGridTreeItem*pItem, CGridTreeItem* pParent);
	//gets pItems parent
	CGridTreeItem* GetParentItem(const CGridTreeItem* pItem); 
	//return ptr to CGridItemInfo daaa
	CGridItemInfo* GetData(const CGridTreeItem* pItem); 
	//sets the CGridItemInfo ptr of pItem
	void UpdateData(CGridTreeItem* pItem, CGridItemInfo* lpInfo);
	//Insert item and return new parent node.
	//the bUpdate is here for performance reasons, when you insert say 100 node each having 10 children(1000 listview items)
	//the bUpdate should be set to FALSE(default) but when you want to insert an item, and you want to user to see it right away
	//set bUpdate to TRUE.
	virtual CGridTreeItem* InsertItem(CGridTreeItem *pParent, CGridItemInfo* lpInfo, BOOL bUpdate=FALSE);
	//collapse all children from pItem
	void Collapse(CGridTreeItem *pItem);
	//expand one folder and return the last index of the expanded folder
	int Expand(CGridTreeItem* pSelItem, int nIndex);
	//expand all items from pItem and return last index of the expanded folder
	void ExpandAll(CGridTreeItem *pItem, int& nScroll);
	//expand all node in pItem and stop at pStopAt, used in SelectNode function
	void ExpandUntil(CGridTreeItem *pItem, CGridTreeItem* pStopAt);
	//use this if you want to select a node 
	//if the node is collapsed all items with in the node are expanded and the node is selected
	//it returns the listview index for the selected node
	int SelectNode(CGridTreeItem *pLocateNode);
	//Delete an item in the listview 
	//takes the node to be delete and its listview item index as arg.
	//note the item you delete must be visible, hence the nItem as arg. 
	void DeleteItemEx(CGridTreeItem *pSelItem, int nItem);
	//does a Quicksort of the pParents children and if bSortChildren set, 
	//all items from pParent are sorted. 
	void Sort(CGridTreeItem* pParent, BOOL bSortChildren);
	// simple wrapper for the CObList in CGridTreeItem, same usage as in the COblist
	POSITION GetHeadPosition(CGridTreeItem* pItem) const;
	POSITION GetTailPosition(CGridTreeItem *pItem) const;
	CGridTreeItem* GetNextChild(CGridTreeItem *pItem, POSITION& pos) const;
	CGridTreeItem* GetPrevChild(CGridTreeItem *pItem, POSITION& pos) const;
	void AddTail(CGridTreeItem *pParent, CGridTreeItem *pChild);
	//simple wrapper for the CPtrList collection of rootitems
	POSITION GetRootHeadPosition(void) const;
	POSITION GetRootTailPosition(void) const;
	CGridTreeItem* GetNextRoot(POSITION& pos) const;
	CGridTreeItem* GetPrevRoot(POSITION& pos) const;
	//Get ListView Index from pNode
	int NodeToIndex(CGridTreeItem *pNode);

	int GetColumnCount();
	void AutoSizeColumns(int col = -1) ;

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridListCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

protected:

	//given the rootptr it returns the rootindex.
	int GetRootIndex(CGridTreeItem * lpRoot);
	//delete pItem and all of its children
	BOOL Delete(CGridTreeItem *pItem, BOOL bClean=TRUE/*delete itemdata*/);
	//used in drag/drop operation..
	void CopyChildren(CGridTreeItem* pDest, CGridTreeItem* pSrc);
	//drag/drop thing....clipboard not supported!
	BOOL DoDragDrop(CGridTreeItem* pTarget, CGridTreeItem* pSelItem);
	//updates internal nodes, called when ever insert,delete on listview
	void InternaleUpdateTree(void);
	//see if user clicked the [+] [-] sign.
	BOOL HitTestOnSign(CPoint point, LVHITTESTINFO& ht);
	//positions an edit-control and creates it
	virtual CEdit* EditLabelEx(int nItem, int nCol, BOOL bIsNumeric, BOOL bReduceSize = FALSE);
	//creates the actual edit control
	virtual CEdit* CreateEditCtrl(int nItem, int nCol, CString strLabel);
	//translates column index value to a column order value.
	int IndexToOrder(int iIndex);
	//set hideflag for pItem
	void Hide(CGridTreeItem* pItem, BOOL bFlag);
	//draw focus rect
	virtual void DrawFocusCell(CDC *pDC, int nItem, int iSubItem, BOOL bReadOnly);
	//draw the special control (down arrow for CB, or button)
	virtual void DrawControl(CDC *pDC, CGridTreeItem *pSelItem, int nItem, int nColumn, COLORREF crBorder);
	//handle newly selected item
    virtual void OnTreeItemSelected(CGridTreeItem* pItem) {}

	int GetCurSubItem(void){return m_CurSubItem;}
	int GetDragItem(void) const {return m_nDragItem; }
	int GetDropTargetItem(void) const {return m_nDragTarget; }

	//positions and creates/initalize a combobox control
	CComboBox* ShowList(int nItem, int nCol, CStringList *lstItems);
	//helper function called from ShowList...calcs the lists max horz extent
	int CalcHorzExtent(CWnd* pWnd, CStringList *pList);
	//checks whether a column is visible, if not scrolls to it
	void MakeColumnVisible(int nCol);
    void OnSelectionChange();

	//{{AFX_MSG(CGridListCtrl)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeydown(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnBegindrag(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);	
	afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnSysColorChange();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	int				m_CurSubItem;	//keyboard handling..
	int				m_cxImage;
	int				m_cyImage;		//+/- size
    CGridTreeItem*  m_pLastSelectedTreeItem;

private:
	//list of rootItems
	CPtrList m_RootItems;
	//internal helpers
	BOOL _Delete(CGridTreeItem* pStartAt, CGridTreeItem *pItem, BOOL bClean=TRUE/*delete itemdata*/);
	UINT _GetCount(CGridTreeItem* pItem, UINT& nCount);
	//overall draw
	void DrawTreeItem(CDC* pDC, CGridTreeItem* pSelItem, int nListItem, const CRect& rcBounds);
	//makes the ... 
	LPCTSTR MakeShortString(CDC* pDC, LPCTSTR lpszLong, int nColumnLen, int nOffset);
	//set the hideflag from pItem and all its children
	void HideChildren(CGridTreeItem *pItem, BOOL bHide, int iIndent);

	//helper compare fn used with Quicksort
	static int CompareChildren(const void* p1, const void* p2);
	void CleanUp(CGridTreeItem *pItem);
	int _NodeToIndex(CGridTreeItem *pStartpos, CGridTreeItem* pNode, int& nIndex, BOOL binit = TRUE);

    int				m_nDragItem;
	int				m_nDragTarget;
    BOOL			m_bIsDragging;
	BOOL			m_bAllowDrag;

	CPen			m_psTreeLine;
	CPen			m_psRectangle;
	CPen			m_psPlusMinus;
	CBrush			m_brushErase;

	friend class	CGridTreeToggle;
};


class CGridTreeToggle
{
public:
	CGridTreeToggle(CGridListCtrl* pCtrl, CDC* pDC, int iIndent, const CRect& rcBounds);
	~CGridTreeToggle();
	void DrawRectangle(CGridListCtrl *pCtrl);
	BOOL HitTest(CPoint pt);
	void DrawPlus(void);
	void DrawMinus(void);
	int GetLeft(void){return m_left;}
	int GetTop(void){return m_top;}
	CRect GetHitTestRect(void) { return CRect(m_left_top, m_right_bottom); }

private:
	CDC*	m_pDC;
	SIZE	m_right_bottom;
	int		m_left;
	int		m_top;
	POINT	m_left_top;
	int		m_topdown;
};



inline int StrComp(const CString* pElement1, const CString* pElement2)
{
	return pElement1->Compare(*pElement2);
}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GridListCtrl_H__C6DF1701_806D_11D2_9A94_002018026B76__INCLUDED_)
