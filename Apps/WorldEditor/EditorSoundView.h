#if !defined(AFX_EditorSoundVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
#define AFX_EditorSoundVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorSoundView.h : header file
//
#include "..\Editor\PaletteView.h"

class CEditorPaletteDoc;

/////////////////////////////////////////////////////////////////////////////
// CEditorSoundView view

class CEditorSoundView : public CPaletteView
{
public:

	CEditorPaletteDoc* GetDocument();

    xbool            m_CreateSoundConnectionMode;
    CTreeCtrl        m_SoundTree;

    virtual void    OnTabActivate( BOOL bActivate );

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorSoundView)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

protected:

	CEditorSoundView();           // protected constructor used by dynamic creation
	virtual ~CEditorSoundView();
	DECLARE_DYNCREATE(CEditorSoundView)

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	//{{AFX_MSG(CEditorSoundView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnButtonCreateSoundNode();
	afx_msg void OnButtonCreateSoundConnection();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
};

inline CEditorPaletteDoc* CEditorSoundView::GetDocument()
   { return (CEditorPaletteDoc*)m_pDocument; }

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorSoundVIEW_H__B46A38B0_E4FA_4AEC_A622_1A4851479423__INCLUDED_)
