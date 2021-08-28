// EventEditorDoc.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "EventEditorDoc.h"
#include "EventEditorFrame.h"
#include "EventEditorView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CEventEditorDoc, CBaseDocument)


BEGIN_MESSAGE_MAP(CEventEditorDoc, CBaseDocument)
	//{{AFX_MSG_MAP(CEventEditorDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// REGISTRATION
/////////////////////////////////////////////////////////////////////////////

REG_EDITOR( s_RegEventEdit, "Event Editor", "evt", IDS_RSC_EVENTEDITOR, CEventEditorDoc, CEventEditorFrame, CEventEditorView );
void LinkEventEditor(void){}

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//=========================================================================

CEventEditorDoc::CEventEditorDoc()
{
    m_pA = "hello world";
}

//=========================================================================

BOOL CEventEditorDoc::OnNewDocument()
{
	if (!CBaseDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

//=========================================================================

CEventEditorDoc::~CEventEditorDoc()
{
}

//=========================================================================

#ifdef _DEBUG
void CEventEditorDoc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

//=========================================================================

void CEventEditorDoc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

void CEventEditorDoc::Serialize(CArchive& ar)
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

//=========================================================================

BOOL CEventEditorDoc::OnOpenDocument(LPCTSTR lpszPathName) 
{
    reg_editor::on_open& Open = *((reg_editor::on_open*)lpszPathName);

    event_description& Anim = event_description::GetSafeType( *((rsc_desc*)Open.pData) );
    
    m_EventEditor.Edit( Anim );

	
	// TODO: Add your specialized creation code here
	
	return TRUE;
}
