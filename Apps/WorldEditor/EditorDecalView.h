#if !defined(AFX_EDITORDECALVIEW_H__023D45AE_FAAC_4F55_AAA3_BE68A779115B__INCLUDED_)
#define AFX_EDITORDECALVIEW_H__023D45AE_FAAC_4F55_AAA3_BE68A779115B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorDecalView.h : header file
//

#include "EditorPaletteDoc.h"
#include "..\Editor\PaletteView.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorDecalView view

class CEditorDecalView : public CPaletteView
{
public:
    virtual void    OnTabActivate   ( BOOL bActivate );
            void    LoadList        ( void );
            BOOL    CanAdd          ( void ) const { return m_bCanAdd; }

protected:
	CEditorDecalView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorDecalView)

	CTreeCtrl   m_rscTree;

// Attributes
public:
	CEditorPaletteDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorDecalView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorDecalView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorDecalView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSelchangeList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDvtbPaintMode();
	afx_msg void OnUpdateDvtbPaintMode(CCmdUI* pCmdUI);
	afx_msg void OnDvtbSelectMode();
	afx_msg void OnUpdateDvtbSelectMode(CCmdUI* pCmdUI);
	afx_msg void OnDvtbRefresh();
	afx_msg void OnUpdateDvtbRefresh(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
    void      AddPathToTree     (CString& strPath,    CString&  strName);
    HTREEITEM DoesChildExist    (const CString& strCurrent, HTREEITEM hParent);
   	
    struct tree_data
    {
        enum { TYPE_FOLDER = 0,
               TYPE_PACKAGE,
               TYPE_GROUP,
               TYPE_DECALDEF };

        s32     Type;
        s32     Data;
    };

    CImageList	        m_imageList;
    xharray<CString>    m_xaPaths;
    xharray<tree_data>  m_TreeItems;
    BOOL                m_bCanAdd;
};

inline CEditorPaletteDoc* CEditorDecalView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORDECALVIEW_H__023D45AE_FAAC_4F55_AAA3_BE68A779115B__INCLUDED_)
