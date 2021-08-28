#if !defined(AFX_EVENTEDITORDOC_H__0CE8E004_899F_4E00_AD56_A9CA5A3CC553__INCLUDED_)
#define AFX_EVENTEDITORDOC_H__0CE8E004_899F_4E00_AD56_A9CA5A3CC553__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EventEditorDoc.h : header file
//
#include "BaseDocument.h"
#include "EventEditor.hpp"

/////////////////////////////////////////////////////////////////////////////
// CEventEditorDoc document

class CPropertyEditorDoc;

class CEventEditorDoc : public CBaseDocument
{
public:
    char*               m_pA;
	event_editor	    m_EventEditor;
    CPropertyEditorDoc* m_pPropEditor;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
protected:
	CEventEditorDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEventEditorDoc)
	virtual ~CEventEditorDoc();

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventEditorDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CEventEditorDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTEDITORDOC_H__0CE8E004_899F_4E00_AD56_A9CA5A3CC553__INCLUDED_)
