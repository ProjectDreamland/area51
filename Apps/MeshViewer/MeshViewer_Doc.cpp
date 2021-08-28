// MeshViewer_Doc.cpp : implementation file
//

#include "stdafx.h"
#include "MeshViewer_Doc.h"
#include "meshViewer_View.h"
#include "meshViewer_Frame.h"
#include "resource.h"
#include "..\Editor\project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CMeshViewer_Doc, CBaseDocument)


BEGIN_MESSAGE_MAP(CMeshViewer_Doc, CBaseDocument)
	//{{AFX_MSG_MAP(CMeshViewer_Doc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// VARS
/////////////////////////////////////////////////////////////////////////////

REG_EDITOR( s_RegMeshViewer, "MATX Importer", "", IDR_RSC_MESH_VIEWER, CMeshViewer_Doc, CMeshViewer_Frame, CMeshViewer_View );
void LinkMeshViewer(void){}


/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//=========================================================================

CMeshViewer_Doc::CMeshViewer_Doc()
{
    m_bOrbitMode        = TRUE;
    m_bPlayInPlace      = FALSE;
    m_bRenderBoneLavels = FALSE;
    m_bRenderBoneLavels = FALSE;
    m_bTakeToBindPose   = FALSE;
    m_bRenderSkeleton   = FALSE;
}

//=========================================================================

BOOL CMeshViewer_Doc::OnNewDocument()
{//
	if (!CBaseDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}

//=========================================================================

CMeshViewer_Doc::~CMeshViewer_Doc()
{
}

//=========================================================================

#ifdef _DEBUG
void CMeshViewer_Doc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

//=========================================================================

void CMeshViewer_Doc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

void CMeshViewer_Doc::Serialize(CArchive& ar)
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

void CMeshViewer_Doc::LoadMesh( void )
{
	CFileDialog		FileOpen(	TRUE, 
								_T("matx"), 
								_T(""), 
								OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
								(_T("Text Files (*.matx)|*.matx|All Files (*.*)|*.*||")));

    CString strOpenPath(g_Settings.GetSourcePath());
    strOpenPath += "\\Art";
    FileOpen.m_ofn.lpstrInitialDir = strOpenPath;

    if( FileOpen.DoModal() == IDOK )
    {
        CString FileName = FileOpen.GetFileName();
        LoadMeshFromFile(FileName);
    }
}

//=========================================================================

void CMeshViewer_Doc::LoadMeshFromFile ( CString strFile )
{
    x_try;
    if (!strFile.IsEmpty())
    {
        m_Viewer.Load( strFile );
    
        POSITION pos = GetFirstViewPosition();
        while (pos != NULL)
        {
            CMeshViewer_View* pView = (CMeshViewer_View*)GetNextView(pos);
            pView->CameraOrbitMode( m_Viewer.GetObjectFocus() );
        }   
    }
    x_catch_display;
}

//=========================================================================

xbool CMeshViewer_Doc::IsPause ( void )
{
    return m_Viewer.IsPause();
}

//=========================================================================

void CMeshViewer_Doc::PlayAnimation( void )
{
    m_Viewer.PlayAnimation( m_bPlayInPlace );
}

//=========================================================================

void CMeshViewer_Doc::PauseAnimation( void )
{
    m_Viewer.PauseAnimation();
}

//=========================================================================

void CMeshViewer_Doc::CameraFreeFlyMode( void )
{
    m_bOrbitMode = FALSE;
}

//=========================================================================

void CMeshViewer_Doc::CameraOrbitMode( void )
{
    m_bOrbitMode = TRUE;
}

//=========================================================================

xbool CMeshViewer_Doc::IsOrbitMode( void )
{
    return m_bOrbitMode;
}

//=========================================================================

void CMeshViewer_Doc::OnProjectOpen()
{
    CONTEXT( "CMeshViewer_Doc::OnProjectOpen" );

    POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
        CMeshViewer_View* pView = (CMeshViewer_View*)GetNextView(pos);
        pView->GetFrame()->RefreshThemeRsc();
    }   
}

//=========================================================================

void CMeshViewer_Doc::OnProjectClose()
{
    POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
        CMeshViewer_View* pView = (CMeshViewer_View*)GetNextView(pos);
        pView->GetFrame()->RefreshThemeRsc();
    }
}

//=========================================================================

void CMeshViewer_Doc::OnProjectSave()
{
}

//=========================================================================

void CMeshViewer_Doc::OnProjectNew()
{
    POSITION pos = GetFirstViewPosition();
    while (pos != NULL)
    {
        CMeshViewer_View* pView = (CMeshViewer_View*)GetNextView(pos);
        pView->GetFrame()->RefreshThemeRsc();
    }
}
