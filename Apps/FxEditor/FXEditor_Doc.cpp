// FxEditor_Doc.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "FxEditor_Doc.h"
#include "FxEditor_Frame.h"
#include "FxEditor_View.h"

#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\EDRscDesc\EDRscDesc_Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFxEditor_Doc

IMPLEMENT_DYNCREATE(CFxEditor_Doc, CBaseDocument)

BEGIN_MESSAGE_MAP(CFxEditor_Doc, CBaseDocument)
	//{{AFX_MSG_MAP(CFxEditor_Doc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
// DECLARE THE EDITOR
//=========================================================================
REG_EDITOR( s_RegLocEdit, "Locomotion Editor", "anim", IDR_RSC_FxEditor, CFxEditor_Doc, CFxEditor_Frame, CFxEditor_View );

//=========================================================================
// FUNCTIONS
//=========================================================================
void LinkFxEditor( void ){}

//=========================================================================

#ifdef _DEBUG
void CFxEditor_Doc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

//=========================================================================

void CFxEditor_Doc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

CFxEditor_Doc::CFxEditor_Doc()
{
}

//=========================================================================

BOOL CFxEditor_Doc::OnNewDocument()
{
	if (!CBaseDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

//=========================================================================

CFxEditor_Doc::~CFxEditor_Doc()
{
}

//=========================================================================

void CFxEditor_Doc::Serialize(CArchive& ar)
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
void CFxEditor_Doc::Save( void )
{
    x_try;

    m_LocoEd.Save();

    x_catch_display;
}

//=========================================================================
void CFxEditor_Doc::Open( void )
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

void CFxEditor_Doc::New( void )
{
    x_try;

    if( CloseApp() == FALSE )
        return;

    m_LocoEd.New();

    x_catch_display;

    m_pPropEditor->Refresh();
}

//=========================================================================

xbool CFxEditor_Doc::CloseApp( void )
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

BOOL CFxEditor_Doc::OnOpenDocument(LPCTSTR lpszPathName) 
{
    reg_editor::on_open& Open = *((reg_editor::on_open*)lpszPathName);

    animation_desc& Anim = animation_desc::GetSafeType( *((rsc_desc*)Open.pData) );
    
    m_LocoEd.BeginEdit( Anim );

	return TRUE;
}
