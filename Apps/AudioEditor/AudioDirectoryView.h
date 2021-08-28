#if !defined(AFX_AUDIODIRECTORYVIEW_H__9180C5C1_8E6C_4CBE_AF02_B2267274CE02__INCLUDED_)
#define AFX_AUDIODIRECTORYVIEW_H__9180C5C1_8E6C_4CBE_AF02_B2267274CE02__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioDirectoryView.h : header file

#include "AudioDefines.h"

//==============================================================================
// EXTERNAL CLASSES
//==============================================================================
class CSoundDoc;


//==============================================================================

//==============================================================================
// CAUDIODIRECTORYVIEW VIEW
//==============================================================================

class CAudioDirectoryView : public CXTTreeView
{

//==============================================================================
public:

	CSoundDoc*  GetDocument             ();

    void        FinishEditing           ( NMTVDISPINFO& Info );
    void        OnCreateDescriptor      ( void );
    void        OnAddElement            ( void );
    
    void        RecursePath             ( xhandle hData, CString strPath, CString strWildcard, HTREEITEM hRoot );
    CString     ItemToPath              ( HTREEITEM hItem );
    void        BuildTreeFromPath       ( CString strRootPath, CString strWildcard, CString strForcedExt );
    void        UsePreviousPathAsDisplay( BOOL bUsePrev ) { m_bUsePrev = bUsePrev; }
        
    void        Refresh                 ( void );
    void        ClearTree               ( void );
    BOOL        IsFolder                ( CString strPath);

    xbool       AddItemsToList          ( HTREEITEM hItemFrom, HTREEITEM hItemTo, xarray<HTREEITEM>& ItemList );
    void        ClearSelection          ( void );
    HTREEITEM   GetNextSelectedItem     ( HTREEITEM hItem );
    HTREEITEM   GetFirstSelectedItem    ( void );
    HTREEITEM   GetNextItem             ( HTREEITEM hItem );
    void        SelectControlItem       ( xarray<HTREEITEM>& ItemList );
    HTREEITEM   GetPrevSelectedItem     ( HTREEITEM hItem );


    xarray< fname >                 m_PathName;
    xharray<tree_structure_info>    m_xaTreeStruct;
    BOOL                            m_bInit;
    BOOL                            m_bUsePrev;
  	CImageList	                    m_imageList;
    HTREEITEM                       m_hItemFirstSel;
    xarray<HTREEITEM>               m_hItemSelList;

//==============================================================================
// MFC
//==============================================================================
public:

	CAudioDirectoryView();           // protected constructor used by dynamic creation
	virtual ~CAudioDirectoryView();
    DECLARE_DYNCREATE(CAudioDirectoryView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioDirectoryView)
	public:
	virtual void OnInitialUpdate();
	virtual void OnDraw         (CDC* pDC);      // overridden to draw this view
    virtual void OnUpdate       ( CView* pSender, LPARAM lHint, CObject* pHint );
	//}}AFX_VIRTUAL

//==============================================================================
protected:
	//{{AFX_MSG(CAudioDirectoryView)
	afx_msg BOOL OnEraseBkgnd   (CDC* pDC);
	afx_msg void OnPaint        ();
	afx_msg int OnCreate        (LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize         (UINT nType, int cx, int cy);
    afx_msg void OnRButtonDown  (UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown  (UINT nFlags, CPoint point);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//==============================================================================
// END
//==============================================================================

#endif // !defined(AFX_AUDIODIRECTORYVIEW_H__9180C5C1_8E6C_4CBE_AF02_B2267274CE02__INCLUDED_)
