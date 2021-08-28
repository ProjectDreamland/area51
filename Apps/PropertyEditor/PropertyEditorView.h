#if !defined(AFX_PROPERTYEDITORVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_PROPERTYEDITORVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyEditorView.h : header file
//
#include "PropertyGrid.h"
#include "GridItemInfo.h"

class CPropertyEditorDoc;
/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorView view

class CPropertyEditorView : public CView
{
protected:
	CPropertyEditorView();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CPropertyEditorView)

// Attributes
public:
	CPropertyEditorDoc* GetDocument();

    void EnableRedraw( void );
    void DisableRedraw( void );
	void ClearGrid();
	void AddComment(CString strTitle, CString strText);
	BOOL AddGridDataElement(CString strName, CString strValue, CString strComment,
		CGridItemInfo::CONTROLTYPE type, CStringList &list, int iXaIndex,
        BOOL bReadOnly, BOOL bMustEnum, BOOL bHeader,
        CGridTreeItem* &pParentTreeItem) ;
    void ExpandRoots();
    void RestoreSavedSelection();
    void SaveStructure();

    bool IsValidItem( CGridTreeItem* pItem );
    void UpdateItem(CGridTreeItem* lpItem);
    
    void SetGridBackgroundColor(COLORREF cr); 

    void SaveColumnState( LPCTSTR lpszProfileName );
    void LoadColumnState( LPCTSTR lpszProfileName );

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyEditorView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CPropertyEditorView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyEditorView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnSliderMove(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnGridItemChange(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnSelectionChange(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnGuidSelect(WPARAM wParam, LPARAM lParam);
    afx_msg void OnRefreshClick();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
	CPropertyGrid			m_lcPropertyGrid;
	CStatic					m_stHelpTitle;
	CStatic					m_stHelp;
	CFont					m_fntTitle;
	CFont					m_fntText;
    CXTButton               m_btnRefresh;

    int                     m_RedrawDisableCount;

    friend class CPropertyEditorDoc;
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
};

inline CPropertyEditorDoc* CPropertyEditorView::GetDocument()
   { return (CPropertyEditorDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYEDITORVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
