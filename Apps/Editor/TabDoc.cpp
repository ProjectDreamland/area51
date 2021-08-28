// TabDoc.cpp : implementation of the CTabDoc class
//

#include "BaseStdAfx.h"
#include "Editor.h"
#include "TabDoc.h"
#include "TabView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabDoc

IMPLEMENT_DYNCREATE(CTabDoc, CDocument)

BEGIN_MESSAGE_MAP(CTabDoc, CDocument)
	//{{AFX_MSG_MAP(CTabDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabDoc construction/destruction

CTabDoc::CTabDoc()
{
	// TODO: add one-time construction code here

}

CTabDoc::~CTabDoc()
{
}

BOOL CTabDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CTabDoc serialization

void CTabDoc::Serialize(CArchive& ar)
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
// CTabDoc diagnostics

#ifdef _DEBUG
void CTabDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTabDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTabDoc commands
