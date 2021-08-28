// HAISDoc.cpp : implementation of the CHAISDoc class
//

#include "stdafx.h"
#include "HAIS.h"

#include "HAISDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHAISDoc

IMPLEMENT_DYNCREATE(CHAISDoc, CDocument)

BEGIN_MESSAGE_MAP(CHAISDoc, CDocument)
END_MESSAGE_MAP()


// CHAISDoc construction/destruction

CHAISDoc::CHAISDoc()
{
	// TODO: add one-time construction code here

}

CHAISDoc::~CHAISDoc()
{
}

BOOL CHAISDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}




// CHAISDoc serialization

void CHAISDoc::Serialize(CArchive& ar)
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


// CHAISDoc diagnostics

#ifdef _DEBUG
void CHAISDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CHAISDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CHAISDoc commands
