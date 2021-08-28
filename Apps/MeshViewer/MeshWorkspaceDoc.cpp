// MeshWorkspaceDoc.cpp : implementation file
//

#include "stdafx.h"
#include "MeshWorkspaceDoc.h"
#include "MeshWorkspaceView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceDoc

IMPLEMENT_DYNCREATE(CMeshWorkspaceDoc, CDocument)

CMeshWorkspaceDoc::CMeshWorkspaceDoc() :
m_pTabParent(NULL),
m_pFrameMV(NULL)
{
}

BOOL CMeshWorkspaceDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CMeshWorkspaceDoc::~CMeshWorkspaceDoc()
{
}


BEGIN_MESSAGE_MAP(CMeshWorkspaceDoc, CDocument)
	//{{AFX_MSG_MAP(CMeshWorkspaceDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceDoc diagnostics

#ifdef _DEBUG
void CMeshWorkspaceDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CMeshWorkspaceDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceDoc serialization

void CMeshWorkspaceDoc::Serialize(CArchive& ar)
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
// CMeshWorkspaceDoc commands

