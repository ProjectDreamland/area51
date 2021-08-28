#if !defined(AFX_INTENSITYVIEW_H__2037FC0B_3B63_4571_BF51_6806045CE231__INCLUDED_)
#define AFX_INTENSITYVIEW_H__2037FC0B_3B63_4571_BF51_6806045CE231__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CIntensityView.h : header file
//


//==============================================================================
// EXTERNAL CLASSES
//==============================================================================
class CSoundDoc;

//==============================================================================
// CIntensityView
//==============================================================================

class CIntensityView : public CXTTreeView
{

protected:

    CImageList	        m_imageList;
    HTREEITEM           m_PrevItemHit;
    HTREEITEM           m_CurrentItemHit;

    HTREEITEM           m_hItemFirstSel;

    xarray<HTREEITEM>   m_hItemSelList;
    xarray<HTREEITEM>   m_hItemCopyList;
    
    xbool               m_RightMouseMenu;
    s32                 m_EditLock;
    xbool               m_IntensitySelected;
    xbool               m_PackageType;

public:    
//==============================================================================


    CSoundDoc*          GetDocument( void );

    void                Refresh                ( void );
    void                OnNewIntensity         ( void );
    void                OnDescToIntensity      ( void );
    void                OnOpen                 ( void );
    void                OnClose                ( void );
    void                OnDelDir               ( void );
    void                OnSortDescending       ( void );
    void                UpdateTreeItemLabel    (  const char* pLabel );
    virtual void        OnTabActivate          ( BOOL bActivate );
//==============================================================================
protected:

    xbool               AddItemsToList          ( HTREEITEM hItemFrom, HTREEITEM hItemTo, xarray<HTREEITEM>& ItemList );
    void                ClearSelection          ( void );
    HTREEITEM           GetPrevSelectedItem     ( HTREEITEM hItem );
    HTREEITEM           GetNextSelectedItem     ( HTREEITEM hItem );
    HTREEITEM           GetFirstSelectedItem    ( void );
    HTREEITEM           GetNextItem             ( HTREEITEM hItem );
    void                SelectControlItem       ( xarray<HTREEITEM>& ItemList );

//==============================================================================
// MFC
//==============================================================================
public:
	CIntensityView();           // protected constructor used by dynamic creation
	virtual ~CIntensityView();
	DECLARE_DYNCREATE(CIntensityView)

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIntensityView)


	//}}AFX_VIRTUAL


#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CIntensityView)
	afx_msg int  OnCreate           ( LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint            ( );
	afx_msg BOOL OnEraseBkgnd       ( CDC* pDC);
	afx_msg void OnRButtonDown      ( UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp        ( UINT nFlags, CPoint point );
	afx_msg void OnLButtonDown      ( UINT nFlags, CPoint point);
	afx_msg UINT OnNcHitTest        ( CPoint point);
    virtual void OnUpdate           ( CView* pSender, LPARAM lHint, CObject* pHint );
    virtual void OnActivateView     ( BOOL bActivate, CView* pActivateView, CView* pDeactiveView );
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_INTENSITYVIEW_H__2037FC0B_3B63_4571_BF51_6806045CE231__INCLUDED_)
