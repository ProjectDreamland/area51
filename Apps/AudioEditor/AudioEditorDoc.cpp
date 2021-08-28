// CAudioEditorDoc.cpp : implementation file
//

#include "stdafx.h"
#include "..\Editor\Resource.h"
#include "AudioEditorDoc.h"
#include "AudioEditorFrame.h"
#include "AudioEditorView.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAudioEditorDoc



IMPLEMENT_DYNCREATE(CAudioEditorDoc, CBaseDocument)

CAudioEditorDoc::CAudioEditorDoc() :
m_pCommandHandler(NULL)
{
}

CAudioEditorDoc::~CAudioEditorDoc()
{
}


BEGIN_MESSAGE_MAP(CAudioEditorDoc, CBaseDocument)
	//{{AFX_MSG_MAP(CAudioEditorDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAudioEditorDoc diagnostics

#ifdef _DEBUG

//=========================================================================

BOOL CAudioEditorDoc::OnNewDocument()
{
	if (!CBaseDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

//=========================================================================

void CAudioEditorDoc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

//=========================================================================

void CAudioEditorDoc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CAudioEditorDoc serialization

//=========================================================================

void CAudioEditorDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

/////////////////////////////////////////////////////////////////////////////
// CAudioEditorDoc commands
