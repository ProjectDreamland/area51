#if !defined(AFX_EditorAIVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorAIVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorAIView.h : header file
//
#include "EditorPaletteDoc.h"
#include "..\Editor\PaletteView.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorAIView view

class CEditorAIView : public CPaletteView
{
protected:
	CEditorAIView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorAIView)

// Attributes
public:
	CEditorPaletteDoc* GetDocument();

    xbool            m_CreateNavConnectionMode; 
    CTreeCtrl        m_aiTree;

    static CEditorAIView* GetEditorAIView(void) { return s_AIView; }
    virtual void    OnTabActivate( BOOL bActivate );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorAIView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorAIView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorAIView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonCreateNavNode();
	afx_msg void OnButtonCreateNavConnection();
	afx_msg void OnButtonCreateForcedNavZone();
	afx_msg void OnButtonGo();
	afx_msg void OnButtonSet1();
	afx_msg void OnButtonSet2();
	afx_msg void OnButtonCreatePlayer();
	afx_msg void OnButtonChainNodes();
	afx_msg void OnButtonCheckAllNodes();
	afx_msg void OnButtonConnectVisible();
	afx_msg void OnButtonConnectMode();
	afx_msg void OnButtonConnectSelected();
	afx_msg void OnButtonBatchFlag();
	afx_msg void OnUpdateAIButtonCreatePlayer(CCmdUI* pCmdUI);
	afx_msg void OnUpdateAIButtonCreateNavNode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonConnectVisible(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonConnectSelected(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonConnectMode(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonCheckAllNodes(CCmdUI* pCmdUI);
	afx_msg void OnUpdateButtonChainNodes(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    static CEditorAIView* s_AIView;
    
private:
};

inline CEditorPaletteDoc* CEditorAIView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorAIVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
