#if !defined(AFX_EditorBlueprintVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorBlueprintVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorBlueprintView.h : header file
//

#include "..\WinControls\FileTreeCtrl.h"
#include "ResourcePreview.h"
#include "Parsing\TextIn.hpp"
#include "..\Editor\PaletteView.h"

class CEditorPaletteDoc;
class CEditorFrame;


/////////////////////////////////////////////////////////////////////////////
// CEditorBlueprintView view

class CEditorBlueprintView : public CPaletteView
{
protected:
	CEditorBlueprintView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEditorBlueprintView)

// Attributes
public:
	CEditorPaletteDoc* GetDocument();
    BOOL IsBlueprintAddable(CEditorFrame *pFrame);

    CResourcePreview    m_wndPreview;
    CStatic             m_stPlaceholder;

    void BuildTreeFromProject(); 
    virtual void    OnTabActivate( BOOL bActivate );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorBlueprintView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CEditorBlueprintView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorBlueprintView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBptbCreateAnchor();
	afx_msg void OnBptbCreateBlueprint();
    afx_msg void OnBptbAddBlueprintAsObjects();
    afx_msg void OnBptbAddBlueprint();
    afx_msg void OnBptbShatterBlueprint();
    afx_msg void OnBptbShatterBlueprintForEdit();
    afx_msg void OnBptbRefresh();
    afx_msg void OnBptbResync();
    afx_msg void OnBptbReplaceSelectedWithBlueprint();
    afx_msg LRESULT OnFileItemChange(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFileItemPreChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelchangeList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateBptbCreateBlueprint(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBptbCreateAnchor(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBptbAddBlueprintAsObjects(CCmdUI* pCmdUI);
	afx_msg void OnUpdateBptbAddBlueprint(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbShatterBlueprint(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbShatterBlueprintForEdit(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbRefresh(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbResync(CCmdUI* pCmdUI);
    afx_msg void OnUpdateBptbReplaceSelectedWithBlueprint(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
    BOOL RenderBlueprint(CString strPath);
    void AddObjectToRender( text_in& TextIn, CString strPath, const vector3& Offset, BOOL bIsRigid );

private:
    CFileTreeCtrl m_fbcBlueprints;
};

inline CEditorPaletteDoc* CEditorBlueprintView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorBlueprintVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
