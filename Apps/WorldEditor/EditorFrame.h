// ChildFrm.h : interface of the CEditorFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_EDITORFRM_H__1528B0E0_8AA2_4031_9FFF_E1F775E22E4F__INCLUDED_)
#define AFX_EDITORFRM_H__1528B0E0_8AA2_4031_9FFF_E1F775E22E4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Objects\Object.hpp"
#include "..\Editor\BaseFrame.h"
#include "WorkspaceTabCtrl.h"

class CPropertyEditorDoc;
class CEditorView;
class CEditorDoc;
class CEditorPaletteDoc;

class CEditorFrame : public CBaseFrame
{
	DECLARE_DYNCREATE(CEditorFrame)
public:
	CEditorFrame();

    //CXTStatusBar    m_wndStatusBar;

    CXTToolBar		        m_wndToolBar;
    CWorkspaceTabCtrl       m_wndWrkspBar;
	CXTTabCtrlBar           m_wndProperty;
	CImageList              m_imageList;
    CEditorView*            m_pWorldEditView;
    CEditorPaletteDoc*      m_pPaletteDoc;
	CXTFlatComboBox         m_wndComboBox;
    CString                 m_strCurrentLevel;

// Attributes
public:
    CWnd*                  FindViewFromTab( CXTTabCtrlBar& Bar, CRuntimeClass *pViewClass );
    
    CPropertyEditorDoc*    GetPropertyEditorDoc();
    CPropertyEditorDoc*    GetSettingsEditorDoc();
    CEditorDoc*            GetEditorDoc();
    CEditorView*           GetEditorView() { return m_pWorldEditView; }
    void                   SetProject(CString strProject);

    void                   RefreshAll();
    void                   RefreshWatchWindowIfActive();
    void                   ShowSettings();
    void                   ShowProperties();

    guid                   CreateObject( const char* pObjectType, vector3& pos, BOOL bSelect);
    void                   CreateTemporaryObject( const char* pObjectType );
    void                   OnCreateObject();
    void                   OnLoadLayers();
    void                   SetActiveLayer(CString strLayer);
    void                   ReloadCurrentLevel();

    BOOL                   AddObjectToLayerView(CString strLayer, CString strLayerPath, guid ObjectGuid);
    BOOL                   AddObjectToActiveLayerView(guid ObjectGuid);
    BOOL                   RemoveObjectFromLayerView(CString strLayer, CString strLayerPath, guid ObjectGuid);
    BOOL                   AddBlueprintToLayerView(CString strLayer, CString strLayerPath, guid BlueprintGuid);
    BOOL                   AddBlueprintToActiveLayerView(guid BlueprintGuid);
    BOOL                   RemoveBlueprintFromLayerView(CString strLayer, CString strLayerPath, guid BlueprintGuid);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void ActivateFrame(int nCmdShow);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditorFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif


// Generated message map functions
	//{{AFX_MSG(CEditorFrame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnSelchangeActiveLayerCombo();
    afx_msg LRESULT OnUpdateRequired(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGuidSelectorRequest(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGuidHighlightRequest(WPARAM wParam, LPARAM lParam);
	afx_msg void OnCreateFromSelected();
	afx_msg void OnCreateGenericObject(UINT nID);
    afx_msg void OnUpdateCreateGenericObject(CCmdUI* pCmdUI);
    afx_msg void OnShowSelectionBounds();
	afx_msg void OnWetbUndo();
	afx_msg void OnWetbRedo();
	afx_msg void OnWetbTransStack();
	afx_msg void OnWetbShowaxis();
	afx_msg void OnWetbShowgrid();
	afx_msg void OnWetbShowspacd();
	afx_msg void OnWetbCreateObject();
	afx_msg void OnWetbDeleteSelected();
	afx_msg void OnWetbSnaptogrid();
    afx_msg void OnWetbSnapAddWithRay();
	afx_msg void OnWetbGridToObject();
	afx_msg void OnWetbMoveMode();
	afx_msg void OnWetbCopyObjects();
    afx_msg void OnWetbMoveObjectsToActiveLayer();
	afx_msg void OnWetbRunGame();
	afx_msg void OnWetbPauseGame();
	afx_msg void OnWetbStepGame();
	afx_msg void OnWetbToggleFPV();
    afx_msg void OnWetbSaveGame();
    afx_msg void OnWetbLoadGame();   
    afx_msg void OnWetbRenderIcons();
	afx_msg void OnWetbFocusCam();
	afx_msg void OnWetbLightLevel();
	afx_msg void OnWetbRenderPreview();
	afx_msg void OnWetbPortalWalk();
    afx_msg void OnWetbSoundDebugStats();
    afx_msg void OnWetbSoundPrintPackage();
    afx_msg void OnWetbSoundPropagateLevel();
    afx_msg void OnWetbRefreshRsc();
    afx_msg void OnWetbCameraHistoryNext();
    afx_msg void OnWetbCameraHistoryPrev();
    afx_msg void OnWetbCameraFavorites();
    afx_msg void OnWetbGotoPlayer();
    afx_msg void OnUpdateWetbRenderIcons(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbPortalWalk(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWetbSoundDebugStats(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbLightLevel(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWetbSoundPropagateLevel(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbFocusCam(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbUndo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbRedo(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbTransStack(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbShowaxis(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbShowgrid(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbShowspacd(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbCreateObject(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbDeleteSelected(CCmdUI* pCmdUI);
	afx_msg void OnUpdateShowSelectionBounds(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbSnaptogrid(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWetbSnapAddWithRay(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbGridToObject(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbMoveMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbCopyObjects(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWetbMoveObjectsToActiveLayer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbRunGame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbPauseGame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbStepGame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbSaveGame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbLoadGame(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbToggleFPV(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbRenderPreview(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbRefreshRsc(CCmdUI* pCmdUI);
	afx_msg void OnUpdateWetbCameraHistoryNext(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWetbCameraHistoryPrev(CCmdUI* pCmdUI);
    afx_msg void OnUpdateWetbCameraFavorites(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbCreateBlueprint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBptbCreateAnchor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBptbAddBlueprintAsObjects(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBptbAddBlueprint(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbShatterBlueprint(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbShatterBlueprintForEdit(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbRefresh(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbAddPlaysurface(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbAddPropsurface(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbAddAnimsurface(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbRefresh(CCmdUI* pCmdUI); 
    afx_msg void OnUpdateOvtbUpdateGeomsFromSel(CCmdUI* pCmdUI);
    afx_msg void OnUpdateOvtbUpdateRigidInst(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDvtbRefresh(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDvtbPaintMode(CCmdUI* pCmdUI);
	afx_msg void OnClose();
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnUpdateAIButtonCreatePlayer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAIButtonCreateNavNode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonConnectVisible(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonConnectSelected(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonConnectMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonCheckAllNodes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonChainNodes(CCmdUI* pCmdUI);
    afx_msg LRESULT OnToolBarDropDown(WPARAM wParam, LPARAM lParam);
    afx_msg void OnCameraFavoriteAdd();
    afx_msg void OnCameraFavorite( UINT nID );
    afx_msg void OnCameraFavoriteMore();

    afx_msg void OnUpdateSelectionSelectAll(CCmdUI* pCmdUI);
    afx_msg void OnSelectionSelectAll();

    afx_msg void OnUpdateDebugInvulnerable(CCmdUI* pCmdUI);
    afx_msg void OnUpdateDebugInfiniteAmmo(CCmdUI* pCmdUI);
    afx_msg void OnDebugInvulnerable();
    afx_msg void OnDebugInfiniteAmmo();
	//}}AFX_MSG
    afx_msg void OnWetbLightLevelType(UINT nId);
	DECLARE_MESSAGE_MAP()

protected:
    CStringList             m_lstGenericCreateMenuItems;
    CPtrList                m_lstGenericCreateNodeData;
        
private:
    CPropertyEditorDoc*     m_pPropertyEditorDoc;
    CPropertyEditorDoc*     m_pSettingsEditorDoc;

    HMENU                   m_hPreviousMenu;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORFRM_H__1528B0E0_8AA2_4031_9FFF_E1F775E22E4F__INCLUDED_)
