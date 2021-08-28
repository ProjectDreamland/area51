#if !defined(AFX_PropertyGrid_H__09B964C4_953D_11D2_9B04_002018026B76__INCLUDED_)
#define AFX_PropertyGrid_H__09B964C4_953D_11D2_9B04_002018026B76__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyGrid.h : header file
//
#include "GridListCtrl.h"
#include "GridTreeItem.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertyGrid window

class CPropertyGrid : public CGridListCtrl
{
// Construction
public:
	CPropertyGrid();
	virtual ~CPropertyGrid();

    BOOL OnUpdateItem(CGridTreeItem* lpItem);

	void InitializeGrid(void);
	void SetFirstColWidth(int iWidth);
	int GetFirstColWidth();

    void SaveStructure();
    BOOL ShouldItemExpand(CString strIdentifier);
    void RestoreSavedSelection();
    CString GetSavedSelectedItemID() { return m_strSaveSelectedID; }

protected:
    CString         m_strSaveSelectedID;
    xarray<CString> m_lstVisibleItems;
    CPoint          m_ptSavedPosition;

public:

	//search item and subitems 
	//usage:
	//CGridTreeItem *pResult = Search("Item 1","subitem x","Subitem y","subitem 0", NULL); //must end with NULL
	//if(pResult!=NULL) { ... }
	//returns the pointer to node that matches the search criteria or NULL if not found
	CGridTreeItem* Search(CString strItem, ...);
	CGridTreeItem* SearchEx(CGridTreeItem *pStartPosition, CString strItem);

	BOOL OnUpdateListViewItem(CGridTreeItem* lpItem, LV_ITEM *plvItem);
	CGridItemInfo* CopyData(CGridItemInfo* lpSrc);
	BOOL OnItemLButtonDown(LVHITTESTINFO& ht);

    CGridTreeItem* GetRootItemWithIdentifier(CString strIdentifier);
    CGridTreeItem* FindTreeItemWithIdentifier(CString strIdentifier);
    CGridTreeItem* FindChildWithIdentifier(CString strIdentifier, CGridTreeItem* pParent);

    // SB - New and improved - uses coherency and a much faster search than the other function
    CGridTreeItem* FastFindTreeItemWithIdentifier(const CString& strIdentifier, CGridTreeItem* pPreviousResult) ;

    bool            IsChildOf       ( CGridTreeItem* pRoot, CGridTreeItem* pSearchItem );
    bool            IsValidItem     ( CGridTreeItem* pItemToValidate );

	//override this to activate any control when lButtonDown in a cell, called from within OnLButtonDown
	virtual void OnControlLButtonDown(UINT nFlags, CPoint point, LVHITTESTINFO& ht);
	//called from PreTranslateMessage, override this to handle other controls than editctrl's
	virtual BOOL OnVkReturn(void);
	//recalc the window size
	virtual void RecalcSize();
	//handle newly selected item
    virtual void OnTreeItemSelected(CGridTreeItem* pItem);

	//Insert item and return new parent node.
	//the bUpdate is here for performance reasons, when you insert say 100 node each having 10 children(1000 listview items)
	//the bUpdate should be set to FALSE(default) but when you want to insert an item, and you want to user to see it right away
	//set bUpdate to TRUE.(see the use of bUpdate in the HowToInsertItemsAfterTheGridHasBeenInitialized function in the CMyGridListCtrl)
	virtual CGridTreeItem* InsertItem(CGridTreeItem *pParent, CGridItemInfo* lpInfo, BOOL bUpdate=FALSE);
	//color button handler
	virtual void OnButtonColorPick(int iItem, int iSubItem);
	//browse button handler
	virtual void OnButtonBrowse(int iItem, int iSubItem);

    virtual BOOL OnItemExpanded(CGridTreeItem* pItem, int iItem);
	virtual BOOL OnItemCollapsed(CGridTreeItem *pItem);
	//handle calling out
	virtual void OnGridItemChange(CGridTreeItem *pSelItem);
	//handle guid message
	virtual void OnGuidSelect(CGridTreeItem *pSelItem);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyGrid)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	//}}AFX_VIRTUAL

protected:
	//positions an edit-control and creates it
//	virtual CEdit* EditLabelEx(int nItem, int nCol, BOOL bIsNumeric, BOOL bReduceSize = FALSE);
	//creates the actual edit control
	virtual CEdit* CreateEditCtrl(int nItem, int nCol, CString strLabel);
	virtual void DrawControl(CDC *pDC, CGridTreeItem *pSelItem, int nItem, int nColumn, COLORREF crBorder);

	// Generated message map functions
	//{{AFX_MSG(CPropertyGrid)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnMouseHover(WPARAM w, LPARAM l);
    afx_msg LRESULT OnMouseLeave(WPARAM w, LPARAM l);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CImageList      m_image;//Must provide an imagelist
	CToolTipCtrl*   m_pToolTip;
    CPoint          m_ptHover;
};

class CPropertyTreeColor : public CGridTreeItem
{
public:
	CPropertyTreeColor();

	COLORREF		m_cr;
	int				m_alpha;
	CString FormatColor();
	void ParseColor(CString strColor);

	// handle compound objects
	virtual void NotifyOfChildChange(CGridListCtrl *pGrid);
	// handle compound objects
	virtual void NotifyOfChange(CGridListCtrl *pGrid);
};

class CPropertyTree2DPos : public CGridTreeItem
{
public:
	CPropertyTree2DPos();

	float		m_x;
	float		m_y;
	CString FormatPos();
	void ParsePos(CString strPos);

	// handle compound objects
	virtual void NotifyOfChildChange(CGridListCtrl *pGrid);
	// handle compound objects
	virtual void NotifyOfChange(CGridListCtrl *pGrid);
};

class CPropertyTree3DPos : public CGridTreeItem
{
public:
	CPropertyTree3DPos();

	float		m_x;
	float		m_y;
	float		m_z;
	CString FormatPos();
	void ParsePos(CString strPos);

	// handle compound objects
	virtual void NotifyOfChildChange(CGridListCtrl *pGrid);
	// handle compound objects
	virtual void NotifyOfChange(CGridListCtrl *pGrid);
};

class CPropertyTreeRotation : public CGridTreeItem
{
public:
	CPropertyTreeRotation();

	float		m_Roll;
	float		m_Pitch;
	float		m_Yaw;
	CString FormatRot();
	void ParseRot(CString strRot);

	// handle compound objects
	virtual void NotifyOfChildChange(CGridListCtrl *pGrid);
	// handle compound objects
	virtual void NotifyOfChange(CGridListCtrl *pGrid);
};

class CPropertyTreeBoundingBox : public CGridTreeItem
{
public:
	CPropertyTreeBoundingBox();
	CString m_strBound1;
	CString m_strBound2;

    CPropertyTree3DPos *m_pVectorMin;
    CPropertyTree3DPos *m_pVectorMax;

	void CalcExtents(CGridListCtrl *pGrid);

	// handle compound objects
	virtual void NotifyOfChildChange(CGridListCtrl *pGrid);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PropertyGrid_H__09B964C4_953D_11D2_9B04_002018026B76__INCLUDED_)
