#if !defined(AFX_AUDIOEDITORDOC_H__7AD34A34_7013_49E7_923B_E909A43B9BB3__INCLUDED_)
#define AFX_AUDIOEDITORDOC_H__7AD34A34_7013_49E7_923B_E909A43B9BB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioEditorDoc.h : header file
//
#include "..\Editor\BaseDocument.h"
#include "AudioEditor.hpp"

/////////////////////////////////////////////////////////////////////////////
// AudioEditorDoc document





class CAudioEditorDoc : public CBaseDocument
{
public:

    void SetCommandHandler(CWnd *pWnd) { m_pCommandHandler = pWnd; }

public:


	CWnd* 				m_pCommandHandler;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
protected:
	CAudioEditorDoc();           // protected constructor used by dynamic creation
	virtual ~CAudioEditorDoc();
    DECLARE_DYNCREATE(CAudioEditorDoc)


/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAudioEditorDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL


/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CAudioEditorDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOEDITORDOC_H__7AD34A34_7013_49E7_923B_E909A43B9BB3__INCLUDED_)
