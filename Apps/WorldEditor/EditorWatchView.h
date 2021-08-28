#if !defined(AFX_EditorWatchView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorWatchView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorWatchView.h : header file
//
#include "EditorPaletteDoc.h"
#include "..\Editor\PaletteView.h"
#include "..\Support\Globals\Global_Variables_Manager.hpp"
#include "..\WinControls\XTSortListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorWatchView view

class CEditorWatchView : public CPaletteView
{
protected:
	CEditorWatchView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorWatchView)

// Attributes
public:
	CEditorPaletteDoc* GetDocument();

    CXTSortListCtrl    m_Watch;

    static CEditorWatchView* GetEditorWatchView(void) { return s_WatchView; }
    virtual void    OnTabActivate( BOOL bActivate );
    void            RefreshView     ( void );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorWatchView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorWatchView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorWatchView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnWvtbPropWatch();
	afx_msg void OnUpdateWvtbPropWatch(CCmdUI* pCmdUI);
	afx_msg void OnWvtbGlobalWatch();
	afx_msg void OnUpdateWvtbGlobalWatch(CCmdUI* pCmdUI);
	afx_msg void OnWvtbClearInfo();
	afx_msg void OnUpdateWvtbClearInfo(CCmdUI* pCmdUI);
	afx_msg void OnWvtbDeleteWatch();
	afx_msg void OnUpdateWvtbDeleteWatch(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    static CEditorWatchView* s_WatchView;
    
private:

    struct watch_obj_props 
    {
        guid        ObjGuid;
        xstring     PropName;
        prop_type   PropType;
    };

    struct watch_globals 
    {
        xstring                 GlobalName;
        var_mngr::global_types  GlobalType;
    };

    xarray<watch_obj_props> m_propsList;
    xarray<watch_globals>   m_globalsList;
};

inline CEditorPaletteDoc* CEditorWatchView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorWatchView_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
