#if !defined(AFX_PROPERTYEDITORDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_)
#define AFX_PROPERTYEDITORDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyEditorDoc.h : header file
//
#include "Auxiliary\MiscUtils\property.hpp"
#include "..\Editor\BaseDocument.h"

class CGridTreeItem;
class CPropertyEditorView;

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorDoc document

class CPropertyEditorDoc : public CBaseDocument
{
public:
	CPropertyEditorDoc();           // protected constructor used by dynamic creation
	CPropertyEditorView* GetView();
    void SetCommandHandler(CWnd *pWnd) { m_pCommandHandler = pWnd; }

    // SetInterface -   Call this to set the interface, if the data is not correctly formatted, this will
    //                  return false and clear the grid;
    // ClearGrid    -   call this to clear the grid
    // SaveProperty -   this is called whenever properties are changed

    void                ClearInterface  ( void );
	void                SetInterface    ( prop_interface& pi, xbool bReadOnly = FALSE );
    prop_interface*     GetInterface    ( void );
    void                ClearGrid       ( void );
    void                SaveProperty    ( CGridTreeItem* lpItem, BOOL bReloadObject );        
    void                Refresh         ( void );
    void                GuidSelect      ( CGridTreeItem* lpItem );   
    
    void                SetGridBackgroundColor(COLORREF cr);

public:
    CWnd* m_pCommandHandler;

/////////////////////////////////////////////////////////////////////////////
// MFC specific code below
/////////////////////////////////////////////////////////////////////////////

protected: // create from serialization only
	DECLARE_DYNCREATE(CPropertyEditorDoc)

    void UpdateIndividualItem(CGridTreeItem* lpItem);
    void GetDataFromXarray(int iXaIndex, 
                           CGridTreeItem* lpItem, 
                           BOOL bCreateObject,
                           CGridTreeItem* &pParentTreeItem) ;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyEditorDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropertyEditorDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyEditorDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    prop_interface *    m_pProperties;
    xbool               m_bReadOnly;
    prop_enum           m_xaList;
};

inline CPropertyEditorView* CPropertyEditorDoc::GetView()
{ 
	POSITION pos = GetFirstViewPosition();
	return (CPropertyEditorView*)GetNextView (pos); 
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYEDITORDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_)
