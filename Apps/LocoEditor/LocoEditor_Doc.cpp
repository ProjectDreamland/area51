// LocoEditor_Doc.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "LocoEditor_Doc.h"
#include "LocoEditor_Frame.h"
#include "LocoEditor_View.h"

#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\EDRscDesc\EDRscDesc_Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLocoEditor_Doc

IMPLEMENT_DYNCREATE(CLocoEditor_Doc, CBaseDocument)

BEGIN_MESSAGE_MAP(CLocoEditor_Doc, CBaseDocument)
	//{{AFX_MSG_MAP(CLocoEditor_Doc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
// DECLARE THE EDITOR
//=========================================================================
REG_EDITOR( s_RegLocEdit, "Locomotion Editor", "anim", IDR_RSC_LOCOEDITOR, CLocoEditor_Doc, CLocoEditor_Frame, CLocoEditor_View );

//=========================================================================
// FUNCTIONS
//=========================================================================
void LinkLocoEditor( void ){}

//=========================================================================

#ifdef _DEBUG
void CLocoEditor_Doc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

//=========================================================================

void CLocoEditor_Doc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

CLocoEditor_Doc::CLocoEditor_Doc()
{
}

//=========================================================================

BOOL CLocoEditor_Doc::OnNewDocument()
{
	if (!CBaseDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

//=========================================================================

CLocoEditor_Doc::~CLocoEditor_Doc()
{
}

//=========================================================================

void CLocoEditor_Doc::Serialize(CArchive& ar)
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
void CLocoEditor_Doc::Save( void )
{
    x_try;

    m_LocoEd.Save();

    x_catch_display;
}

//=========================================================================
void CLocoEditor_Doc::Open( void )
{
    x_try;

    if( CloseApp() == FALSE )
        return;

    CFileDialog		FileOpen(	TRUE, 
						    _T("anim"), 
						    _T(""), 
						    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
						    (_T("Anim Files (*.anim)|*.anim|All Files (*.*)|*.*||")));


    if( FileOpen.DoModal() != IDOK )
        return;

    CString FileName = FileOpen.GetFileName();

    m_LocoEd.Load( FileName );

    x_catch_display;

    m_pPropEditor->Refresh();
}

//=========================================================================

void CLocoEditor_Doc::New( void )
{
    x_try;

    if( CloseApp() == FALSE )
        return;

    m_LocoEd.New();

    x_catch_display;

    m_pPropEditor->Refresh();
}

//=========================================================================

xbool CLocoEditor_Doc::CloseApp( void )
{
    if( m_LocoEd.NeedSave() )
    {
        long ID = MessageBox( NULL, "Do you want to save the current anim desc?", "Warning",MB_ICONWARNING|MB_YESNOCANCEL );
        if( IDYES == ID )
        {
            m_LocoEd.Save();
        }
        
        if( IDCANCEL == ID )
        {
            return FALSE;
        }
    }

    return TRUE;
}

//=========================================================================

BOOL CLocoEditor_Doc::OnOpenDocument(LPCTSTR lpszPathName) 
{
    reg_editor::on_open& Open = *((reg_editor::on_open*)lpszPathName);

    animation_desc& Anim = animation_desc::GetSafeType( *((rsc_desc*)Open.pData) );
    
    m_LocoEd.BeginEdit( Anim );

	return TRUE;
}
