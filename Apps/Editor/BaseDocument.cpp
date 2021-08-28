// BaseDocument.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "BaseDocument.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBaseDocument

IMPLEMENT_DYNCREATE(CBaseDocument, CDocument)

CBaseDocument::CBaseDocument()
{
    m_bDocActive = FALSE;
}

BOOL CBaseDocument::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

CBaseDocument::~CBaseDocument()
{
}


BEGIN_MESSAGE_MAP(CBaseDocument, CDocument)
	//{{AFX_MSG_MAP(CBaseDocument)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseDocument diagnostics

#ifdef _DEBUG
void CBaseDocument::AssertValid() const
{
	CDocument::AssertValid();
}

void CBaseDocument::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CBaseDocument serialization

void CBaseDocument::Serialize(CArchive& ar)
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
// CBaseDocument commands
