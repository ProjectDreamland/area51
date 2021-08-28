#if !defined(AFX_LOCOEDITOR_DOC_H__96EEB46E_2EDA_4351_8834_969B0324F983__INCLUDED_)
#define AFX_LOCOEDITOR_DOC_H__96EEB46E_2EDA_4351_8834_969B0324F983__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LocoEditor_Doc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CLocoEditor_Doc document
#include "BaseDocument.h"
#include "LocoEditor.hpp"

class CPropertyEditorDoc;
class CLocoEditor_Frame;

class CLocoEditor_Doc : public CBaseDocument
{
/////////////////////////////////////////////////////////////////////////////
public:

    void                Save    ( void );
    void                Open    ( void );
    void                New     ( void );
    xbool               CloseApp( void );

/////////////////////////////////////////////////////////////////////////////
public:
    CPropertyEditorDoc* m_pPropEditor;
    locomotion_ed       m_LocoEd;
    CLocoEditor_Frame*  m_pFrame;



/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
public:
	DECLARE_DYNCREATE(CLocoEditor_Doc)
             CLocoEditor_Doc();           // protected constructor used by dynamic creation
	virtual ~CLocoEditor_Doc();

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocoEditor_Doc)
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
	//{{AFX_MSG(CLocoEditor_Doc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LOCOEDITOR_DOC_H__96EEB46E_2EDA_4351_8834_969B0324F983__INCLUDED_)
