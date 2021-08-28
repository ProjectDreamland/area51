#if !defined(AFX_EditorPaletteDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_)
#define AFX_EditorPaletteDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorPaletteDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CEditorPaletteDoc document
class CEditorFrame;

class CEditorPaletteDoc : public CDocument
{
public:
	CEditorPaletteDoc();           // protected constructor used by dynamic creation
    void SetFramePointer(CEditorFrame* pFrame) { m_pFrameEdit = pFrame; }
    CEditorFrame* GetFramePointer() { return m_pFrameEdit; }
    void SetTabParent(CXTTabCtrlBar *pParent) { m_pTabParent = pParent; }
    CXTTabCtrlBar* GetTabParent() { return m_pTabParent; }

private:
    CEditorFrame*   m_pFrameEdit;
    CXTTabCtrlBar*  m_pTabParent;

protected: // create from serialization only
	DECLARE_DYNCREATE(CEditorPaletteDoc)

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorPaletteDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CEditorPaletteDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CEditorPaletteDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EditorPaletteDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_)
