#if !defined(AFX_AUDIOEDITORVIEW_H__2037FC0B_3B63_4571_BF51_6806045CE231__INCLUDED_)
#define AFX_AUDIOEDITORVIEW_H__2037FC0B_3B63_4571_BF51_6806045CE231__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CAudioEditorView.h : header file
//

#include "AudioDefines.h"

//==============================================================================
// EXTERNAL CLASSES
//==============================================================================
class CSoundDoc;


//==============================================================================
// CAUDIOEDITORVIEW VIEW
//==============================================================================

class CAudioEditorView : public CXTTreeView
{

protected:

    CImageList	        m_imageList;
    HTREEITEM           m_PrevItemHit;
    HTREEITEM           m_CurrentItemHit;

    HTREEITEM           m_NewestAddedPackage;
    HTREEITEM           m_NewestAddedDesc;
	HTREEITEM           m_NewestAddedElement;

    HTREEITEM           m_hItemFirstSel;

    HTREEITEM	        m_hItemDrop;
    HTREEITEM	        m_hItemDrag;
    CImageList*	        m_pDragImage;
    xbool   		    m_bLDragging;

    xarray<HTREEITEM>   m_hItemSelList;
    xarray<HTREEITEM>   m_hItemCopyList;
    xarray<HTREEITEM>   m_hItemMoveList;
    
    xbool               m_RightMouseMenu;
    s32                 m_EditLock;

	HCURSOR             m_CursorHand;
	HCURSOR             m_CursorArr;
	HCURSOR             m_CursorNo;
	xbool	            m_IsCursorArrow;
    xbool               m_CheckMultiSelectRelease;


public:    
//==============================================================================


    CSoundDoc*          GetDocument             ( void );

    void                OnNewDir                ( void );
    
    void                OnDelDir                ( void );

    void                OnNewAudioPackageEvent  ( void );
    s32                 OnNewAudioPackage       ( void );
    void                OnNewAudioDescriptorEvent   ( void );
    s32                 OnNewAudioDescriptor    ( void );
    s32                 OnNewAudioElement       ( CString PathName );
    void                RefrenceDescriptor      ( void );
    void                ChangeElementSelection  ( void );
    void                OnNewRootDir            ( void );
    void                OnCreateDescFromSample  ( CString PathName );

    void                OnBuildPackage          ( void );
    void                OnReBuildPackage        ( void );
    
    void                OnLoadProject           ( void );
    void                OnCopyEvent             ( void );
    xbool               OnCopy                  ( void );
    void                OnPaste                 ( void );
    void                OnOpen                  ( void );
    void                OnClose                 ( void );

    void                OnMoveTo                ( void );
    void                OnMovePaste             ( void );
    
    void                UpdateTreeItemLabel     ( const char* pLabel );
    void                OnSortDescending        ( void );

    virtual void        OnTabActivate           ( BOOL bActivate );
    void                SetDefaultCursor        ( void );
//==============================================================================
protected:

    xbool               AddItemsToList          ( HTREEITEM hItemFrom, HTREEITEM hItemTo, xarray<HTREEITEM>& ItemList );
    void                GetSubFolders           ( HTREEITEM hItem, xarray< tree_item >& TreeList );
    void                OnLoadVirtualDir        ( CString VirtualDir, xhandle PackageIndex );
    void                OnLoadAudioPackage      ( xhandle PackageHandle, HTREEITEM ParentItem );
    void                ClearSelection          ( void );
    HTREEITEM           GetPrevSelectedItem     ( HTREEITEM hItem );
    HTREEITEM           GetNextSelectedItem     ( HTREEITEM hItem );
    HTREEITEM           GetFirstSelectedItem    ( void );
    HTREEITEM           GetNextItem             ( HTREEITEM hItem );
    void                SelectControlItem       ( xarray<HTREEITEM>& ItemList );
    void                SetSelectedItemData     ( void );
    s32                 CreateElement           ( HTREEITEM Parent, HTREEITEM& rAddedItem );

//==============================================================================
// MFC
//==============================================================================
public:
	CAudioEditorView();           // protected constructor used by dynamic creation
	virtual ~CAudioEditorView();
	DECLARE_DYNCREATE(CAudioEditorView)

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioEditorView)
	public:
	virtual void OnInitialUpdate();
	//}}AFX_VIRTUAL


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CAudioEditorView)
	afx_msg int  OnCreate           ( LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint            ( );
	afx_msg void OnFileRename       ( );
	afx_msg BOOL OnEraseBkgnd       ( CDC* pDC);
	afx_msg void OnRButtonDown      ( UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp        ( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDown      ( UINT nFlags, CPoint point);
	afx_msg UINT OnNcHitTest        ( CPoint point);
	afx_msg void OnBeginlabeledit   ( NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeledit     ( NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnKillFocus        ( CWnd* pNewWnd );
    afx_msg void OnBeginDrag        ( NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMouseMove        ( UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp        ( UINT nFlags, CPoint point);

    virtual void OnUpdate           ( CView* pSender, LPARAM lHint, CObject* pHint );
    virtual void OnActivateView     ( BOOL bActivate, CView* pActivateView, CView* pDeactiveView );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_AUDIOEDITORVIEW_H__2037FC0B_3B63_4571_BF51_6806045CE231__INCLUDED_)
