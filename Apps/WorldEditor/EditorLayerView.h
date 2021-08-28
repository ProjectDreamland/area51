#if !defined(AFX_EditorLayerView_H__B96A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorLayerView_H__B96A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorLayerView.h : header file
//
#include "..\Editor\PaletteView.h"

class CEditorPaletteDoc;
class CEditorFrame;

/////////////////////////////////////////////////////////////////////////////
// CEditorLayerView view

class CEditorLayerView : public CPaletteView
{
protected:
	CEditorLayerView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorLayerView)

// Attributes
public:
	CEditorPaletteDoc* GetDocument();

    void LoadLayers();
    BOOL CanModify(BOOL bLayerChange = FALSE);
    BOOL IsLayerSelected(BOOL bIncludeDefault = TRUE, BOOL bIncludeGlobal = TRUE);
    BOOL IsFolderSelected();
    BOOL IsResourceSelected();
    BOOL IsZoneSelected();
    BOOL IsPortalSelected();
    BOOL IsLayerLoaded();
    BOOL IsLayerEditable();

    HTREEITEM AddObjectToLayer(CString strLayer, CString strPath, guid ObjectGuid);
    BOOL RemoveObjectFromLayer(CString strLayer, CString strPath, guid ObjectGuid);
    HTREEITEM AddBlueprintToLayer(CString strLayer, CString strPath, guid BlueprintGuid);
    BOOL RemoveBlueprintFromLayer(CString strLayer, CString strPath, guid BlueprintGuid);
    HTREEITEM AddResourceToLayer(CString strLayer, CString strPath, CString strRes);
    BOOL RemoveResourceFromLayer(CString strLayer, CString strPath, CString strRes);

    BOOL SetLayerDirtyColor(CString strLayer, HTREEITEM hLayerItem = NULL);
    void SetAllLayersClean();

    void SelectChildren(HTREEITEM hParent);
    void SaveChildPaths(HTREEITEM hParent);
    BOOL GetItemPath(HTREEITEM hItem, CString &strPath, CString &strLayer, BOOL bIgnoreSelected = FALSE);
    BOOL DoesSiblingExist(HTREEITEM hItem, CString strName);
    BOOL DoesChildExist(HTREEITEM hParent, CString strName);

    HTREEITEM FindObject(HTREEITEM hParent, guid ItemGuid);
    HTREEITEM FindBlueprint(HTREEITEM hParent, guid ItemGuid);
    HTREEITEM FindResource(HTREEITEM hParent, CString strRes);
    HTREEITEM FindLayer(CString strLayer);

    void SelectObject(guid ItemGuid, BOOL bAndExpand = FALSE);
    void SelectBlueprint(guid ItemGuid, BOOL bAndExpand = FALSE);
    void SelectTreeItem(HTREEITEM hItem, BOOL bAndExpand = FALSE);
    BOOL IsItemChainExpanded(HTREEITEM hItem);
    BOOL GetChildBounds( HTREEITEM hParent, bbox& Bounds );

    BOOL DoesZoneExist(CString strLayer, CString strZone);
    
    virtual void    OnTabActivate( BOOL bActivate );

    void DeleteSelected(void);

// Operations
public:
    void TrackOpenTreeFolders( void );
    void UpdateTreeFolders( void );


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorLayerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorLayerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorLayerView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvtbAddLayer();
	afx_msg void OnLvtbAddFolder();
	afx_msg void OnLvtbAddZone();
	afx_msg void OnLvtbAddPortal();
	afx_msg void OnLvtbAddResource();
	afx_msg void OnLvtbDeleteLayer();
	afx_msg void OnLvtbRenameLayer();
	afx_msg void OnLvtbSetAsActiveLayer();
    afx_msg void OnLvtbLightLayer();
    afx_msg void OnLvtbSelectLayer();
    afx_msg void OnLvtbLoadLayer();
    afx_msg void OnLvtbLoadAllLayers();
    afx_msg void OnLvtbCheckOutLayer();
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);

    afx_msg void OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMDblClick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLvrmHideThis();
    afx_msg void OnLvrmShowThis();
    afx_msg void OnLvrmSelectAllThisObject();
    afx_msg void OnLvrmSelectAllThisObjectRigid();
    afx_msg void OnLvrmSelectAllThisObjectAnimation();
    afx_msg void OnLvrmHideAllThis();
    afx_msg void OnLvrmShowAllThis();
    afx_msg void OnLvrmThawThis();
    afx_msg void OnLvrmFreezeThis();
    afx_msg void OnLvrmThawAllThis();
    afx_msg void OnLvrmFreezeAllThis();
    afx_msg void OnLvrmHideAll();
    afx_msg void OnLvrmShowAll();
    afx_msg void OnLvrmThawAll();
    afx_msg void OnLvrmFreezeAll();
    afx_msg void OnLvrmSelectAll();
    afx_msg void OnLvrmHideThisObjectAnimation();
    afx_msg void OnLvrmHideThisObjectRigid();
    afx_msg void OnLvrmShowThisObjectAnimation();
    afx_msg void OnLvrmShowThisObjectRigid();
    afx_msg void OnLvrmThawThisObjectAnimation();
    afx_msg void OnLvrmThawThisObjectRigid();
    afx_msg void OnLvrmFreezeThisObjectAnimation();
    afx_msg void OnLvrmFreezeThisObjectRigid();

	afx_msg void OnUpdateLvtbAddLayer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLvtbAddFolder(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLvtbAddResource(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLvtbAddZone(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLvtbAddPortal(CCmdUI* pCmdUI);   
	afx_msg void OnUpdateLvtbDeleteLayer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLvtbRenameLayer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLvtbSetAsActiveLayer(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLvtbLightLayer(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLvtbSelectLayer(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLvtbLoadLayer(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLvtbLoadAllLayers(CCmdUI* pCmdUI);
    afx_msg void OnUpdateLvtbCheckOutLayer(CCmdUI* pCmdUI);
	//}}AFX_MSG
    afx_msg void OnLvtbLightLayerType(UINT nId);
	DECLARE_MESSAGE_MAP()

    xhandle FindHandleForObjectGuid( guid Guid );
    guid GetObjectGuidFromHandle( xhandle xh );
    xhandle AddObjectGuidToStorageArray( guid Guid );

    xhandle FindHandleForBlueprintGuid( guid Guid );
    guid GetBlueprintGuidFromHandle( xhandle xh );
    xhandle AddBlueprintGuidToStorageArray( guid Guid );

    HTREEITEM AddObjectToTree(HTREEITEM hParent, guid ObjGuid);
    HTREEITEM AddBlueprintToTree(HTREEITEM hParent, guid BPGuid);

    void        SetFolderImageForItem           ( HTREEITEM hItem );
    void        SetMenuSelectionType            ( int type );
    int         GetMenuSelectionType            ( void );
    void        SetMenuSelectionGuid            ( guid Guid );
    guid        GetMenuSelectionGuid            ( void );
    void        SetMenuSelectionItem            ( HTREEITEM hItem );
    HTREEITEM   GetMenuSelectionItem            ( void );
    void        CollectAllObjectsInFolder       ( xarray<guid>& ObjList );
    void        CollectObjectList               ( xarray<guid>& ObjList );





private:
    CXTTreeCtrl     m_tcLayer;
	CImageList	    m_imageList;
    xharray<guid>   m_lstObjectGuidStorage;
    xharray<guid>   m_lstBlueprintGuidStorage;
    BOOL            m_bInternalSelect;
    int             m_ScrollPos;
    int             m_SelectionType;
    HTREEITEM       m_SelectionItem;
    guid            m_Guid;


        

};

inline CEditorPaletteDoc* CEditorLayerView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorLayerView_H__B96A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
