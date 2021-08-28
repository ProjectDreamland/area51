// EditorPaletteDoc.cpp : implementation file
//

#include "stdafx.h"
#include "EditorPaletteDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorPaletteDoc

IMPLEMENT_DYNCREATE(CEditorPaletteDoc, CDocument)

CEditorPaletteDoc::CEditorPaletteDoc() :
m_pTabParent(NULL),
m_pFrameEdit(NULL)
{
}

BOOL CEditorPaletteDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CEditorPaletteDoc::~CEditorPaletteDoc()
{
}


BEGIN_MESSAGE_MAP(CEditorPaletteDoc, CDocument)
	//{{AFX_MSG_MAP(CEditorPaletteDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorPaletteDoc diagnostics

#ifdef _DEBUG
void CEditorPaletteDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CEditorPaletteDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CEditorPaletteDoc serialization

void CEditorPaletteDoc::Serialize(CArchive& ar)
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
// CEditorPaletteDoc commands

