// BitmapEditor_Doc.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "BitmapEditor_Doc.h"
#include "BitmapEditor_Frame.h"
#include "BitmapEditor_View.h"

#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\EDRscDesc\EDRscDesc_Doc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CBitmapEditor_Doc

IMPLEMENT_DYNCREATE(CBitmapEditor_Doc, CBaseDocument)

BEGIN_MESSAGE_MAP(CBitmapEditor_Doc, CBaseDocument)
	//{{AFX_MSG_MAP(CBitmapEditor_Doc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================
// DECLARE THE EDITOR
//=========================================================================
REG_EDITOR( s_RegBitmapEdit, "Bitmap Editor",    "xbmp",   IDR_RSC_BITMAPEDITOR, CBitmapEditor_Doc, CBitmapEditor_Frame, CBitmapEditor_View );
REG_EDITOR( s_RegEnvMapEdit, "EnvBitmap Editor", "envmap", IDR_RSC_ENVMAPEDITOR, CBitmapEditor_Doc, CBitmapEditor_Frame, CBitmapEditor_View );

//=========================================================================
// FUNCTIONS
//=========================================================================
void LinkBitmapEditor( void ){}

//=========================================================================

#ifdef _DEBUG
void CBitmapEditor_Doc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

//=========================================================================

void CBitmapEditor_Doc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

CBitmapEditor_Doc::CBitmapEditor_Doc()
{
}

//=========================================================================

BOOL CBitmapEditor_Doc::OnNewDocument()
{
	if (!CBaseDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

//=========================================================================

CBitmapEditor_Doc::~CBitmapEditor_Doc()
{
}

//=========================================================================

void CBitmapEditor_Doc::Serialize(CArchive& ar)
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
void CBitmapEditor_Doc::Save( void )
{
    x_try;

    m_BitmapEd.Save();

    x_catch_display;
}

//=========================================================================
void CBitmapEditor_Doc::Open( void )
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

    m_BitmapEd.Load( FileName );

    x_catch_display;

    m_pPropEditor->Refresh();
}

//=========================================================================

void CBitmapEditor_Doc::New( void )
{
    x_try;

    if( CloseApp() == FALSE )
        return;

    //m_BitmapEd.New();

    x_catch_display;

    m_pPropEditor->Refresh();
}

//=========================================================================

xbool CBitmapEditor_Doc::CloseApp( void )
{
    if( m_BitmapEd.NeedSave() )
    {
        long ID = MessageBox( NULL, "Do you want to save the current anim desc?", "Warning",MB_ICONWARNING|MB_YESNOCANCEL );
        if( IDYES == ID )
        {
            m_BitmapEd.Save();
        }
        
        if( IDCANCEL == ID )
        {
            return FALSE;
        }
    }

    return TRUE;
}

//=========================================================================

BOOL CBitmapEditor_Doc::OnOpenDocument(LPCTSTR lpszPathName) 
{
    reg_editor::on_open& Open = *((reg_editor::on_open*)lpszPathName);

    if( ((rsc_desc*)Open.pData)->IsKindOf( bitmap_desc::GetRTTI() ) )
    {
        bitmap_desc& Bitmap = bitmap_desc::GetSafeType( *((rsc_desc*)Open.pData) );
        m_BitmapEd.BeginEdit( Bitmap );
    }
    else if( ((rsc_desc*)Open.pData)->IsKindOf( envmap_desc::GetRTTI() ) )
    {
        envmap_desc& Envmap = envmap_desc::GetSafeType( *((rsc_desc*)Open.pData) );
        m_BitmapEd.BeginEdit( Envmap );
    }
    else
    {
        return FALSE;
    }

	return TRUE;
}
