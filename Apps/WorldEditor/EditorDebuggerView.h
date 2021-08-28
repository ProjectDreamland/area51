#if !defined(AFX_EditorDebuggerView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorDebuggerView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorDebuggerView.h : header file
//

class CEditorPaletteDoc;

#include "..\Editor\PaletteView.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorDebuggerView view

class CEditorDebuggerView : public CPaletteView
{
public:

	CEditorPaletteDoc* GetDocument();
    void    RefreshView     ( void );

    virtual void    OnTabActivate( BOOL bActivate );

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorDebuggerView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL


protected:

	CEditorDebuggerView();           // protected constructor used by dynamic creation
	virtual ~CEditorDebuggerView();
	DECLARE_DYNCREATE(CEditorDebuggerView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(CEditorDebuggerView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGdtbGuidLookup();
	afx_msg void OnUpdateGdtbGuidLookup(CCmdUI* pCmdUI);
	afx_msg void OnGdtbGlobalGuidLookup();
	afx_msg void OnUpdateGdtbGlobalGuidLookup(CCmdUI* pCmdUI);
	afx_msg void OnGdtbClearInfo();
	afx_msg void OnUpdateGdtbClearInfo(CCmdUI* pCmdUI);
	afx_msg void OnSelchangedTree(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    CXTTreeCtrl     m_tcDebugger;
	CImageList	    m_imageList;
    CString         m_strDebugItem;
    xharray<guid>   m_lstObjectGuidStorage;
    int             m_QueryType;

    enum debug_query_type
    {
        DQT_NULL,
        DQT_GUID,
        DQT_GLOBAL
    };

    xhandle FindHandleForObjectGuid( guid Guid );
    guid GetObjectGuidFromHandle( xhandle xh );
    xhandle AddObjectGuidToStorageArray( guid Guid );

};

inline CEditorPaletteDoc* CEditorDebuggerView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorDebuggerView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
