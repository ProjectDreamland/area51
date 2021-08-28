// ProjectDoc.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "editor.h"
#include "Project.hpp"
#include "ProjectDoc.h"
#include "x_files.hpp"
#include "Parsing\TextIn.hpp"
#include "Parsing\TextOut.hpp"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\WorldEditor\WorldEditor.hpp"
#include "..\WinControls\FileSearch.h"
#include "..\WinControls\ListBoxDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern xbool     g_bAutoBuild;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CProjectDoc, CBaseDocument)

BEGIN_MESSAGE_MAP(CProjectDoc, CBaseDocument)
	//{{AFX_MSG_MAP(CProjectDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//=========================================================================

CProjectDoc::CProjectDoc()
{
    m_pProjectProp = NULL;
}

//=========================================================================

BOOL CProjectDoc::OnNewDocument()
{
	if (!CBaseDocument::OnNewDocument())
		return FALSE;
	return TRUE;
}

//=========================================================================

CProjectDoc::~CProjectDoc()
{
}

//=========================================================================

#ifdef _DEBUG
void CProjectDoc::AssertValid() const
{
	CBaseDocument::AssertValid();
}

//=========================================================================

void CProjectDoc::Dump(CDumpContext& dc) const
{
	CBaseDocument::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

void CProjectDoc::Serialize(CArchive& ar)
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

BOOL CProjectDoc::SaveModified()
{
    //this is to get around an MFC forced check on dirty docs
    return TRUE;
}

//=========================================================================

xbool CProjectDoc::HandleChangeSave( void )
{
    s32 nDirtyLayers = g_WorldEditor.GetDirtyLayerCount();

    if( g_Project.IsProjectOpen() && (nDirtyLayers > 0) && !g_bAutoBuild )
    {
        CString strText;
        strText.Format( "If you continue you will lose your current changes!\n"
                        "WorldEditor reports %d layer(s) are dirty.\n"
                        "Do you wish to continue?\n", nDirtyLayers );

        long ID = ::AfxMessageBox( strText, MB_YESNO | MB_ICONWARNING );

        if( ID ==IDNO )
            return FALSE;
    }

    return TRUE;
}

//=========================================================================

void CProjectDoc::InitFormFrame( CPropertyEditorDoc* pSettingsProp, CPropertyEditorDoc* pPropEditor )
{
    ASSERT( pPropEditor );
    ASSERT( pSettingsProp );
    m_pProjectProp = pPropEditor;
    m_pSettingsProp = pSettingsProp;
    m_pSettingsProp->SetInterface( g_Settings );
}

//=========================================================================

xbool CProjectDoc::FileNew()
{
    x_try;

    if( HandleChangeSave() == FALSE )
        return FALSE;

    CFileDialog dlgFileOpen(FALSE, 
                    _T("Project"), 
                    _T("Level.Project"), 
                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
                    "Project Files (*.project)|*.project||");

    CString strOpenPath(g_Settings.GetSourcePath());
    strOpenPath += "\\Levels";
    dlgFileOpen.m_ofn.lpstrInitialDir = strOpenPath;

    if( dlgFileOpen.DoModal() == IDOK )
    {
        CString strPathName = dlgFileOpen.GetPathName();
        g_Project.OnNew( strPathName );
        m_pProjectProp->SetInterface( g_Project, FALSE );

        CProjectFrame* pFrame = (CProjectFrame*)GetView()->GetParentFrame();
        if (pFrame)
        {
            pFrame->SetProject(g_Project.GetName());
        }

        return TRUE;
    }

    x_catch_display;

    return FALSE;
}

//=========================================================================

xbool CProjectDoc::FileClose()
{   
    if( HandleChangeSave() == FALSE )
        return FALSE;

    CProjectFrame* pFrame = (CProjectFrame*)GetView()->GetParentFrame();
    if (pFrame)
        pFrame->SetProject("");

    g_Project.Close();
    m_pProjectProp->ClearInterface();

    return TRUE;
}

//=========================================================================

xbool CProjectDoc::FileOpen()
{
    x_try;

    CFileDialog dlgFileOpen(TRUE, 
                    _T("Project"), 
                    _T(""), 
                    OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST, 
                    "Project Files (*.project)|*.project||");

    CString strOpenPath(g_Settings.GetSourcePath());
    strOpenPath += "\\Levels";
    dlgFileOpen.m_ofn.lpstrInitialDir = strOpenPath;

    if( dlgFileOpen.DoModal() == IDOK )
    {
        CString strPathName = dlgFileOpen.GetPathName();
        return LoadProject(strPathName);
    }

    x_catch_display;

    return FALSE;
}

//=========================================================================

xbool CProjectDoc::FileSave()
{
    x_try;

    if (g_Project.IsProjectOpen() )
    {
        //save the readme
        char pName[256];
        g_Project.GetFileName(pName);
        CString strReadme(pName);
        int iSlash = strReadme.ReverseFind('\\');

	    POSITION pos = GetFirstViewPosition();
	    CProjectView* pView = (CProjectView*)GetNextView (pos); 

        if (iSlash != -1 && pView)
        {
            strReadme = strReadme.Left(iSlash);
            strReadme += "\\readme.rtf";

            CTextEditor& TextEdit = pView->GetTextEditor();

            TextEdit.FileSave( strReadme );
        }

        g_Project.Save();
        return TRUE;
    }

    x_catch_display;

    return FALSE;
}

//=========================================================================

void CProjectDoc::OnProjectRefresh()
{
    if (m_pProjectProp)
        m_pProjectProp->Refresh();
}

//=========================================================================

const char* CProjectDoc::GetProjectName( void )
{
    x_try;
    
    if( g_Project.IsProjectOpen() )
        return g_Project.GetName();

    x_catch_display;

    return NULL;
}

//=========================================================================

void CProjectDoc::CreateNewTheme( void )
{
    x_try;

    if( g_Project.IsProjectOpen() == FALSE )
        x_throw( "You must have a working project to create a theme" );

    CFileDialog dlgFileOpen(FALSE, 
                    _T("Project"), 
                    _T("Theme.theme"), 
                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
                    "Theme Files (*.theme)|*.theme||");

    CString strOpenPath(g_Settings.GetSourcePath());
    strOpenPath += "\\Themes";
    dlgFileOpen.m_ofn.lpstrInitialDir = strOpenPath;

    if( dlgFileOpen.DoModal() == IDOK )
    {
        CString strPathName = dlgFileOpen.GetPathName();
        g_Project.CreateTheme( strPathName );
        m_pProjectProp->Refresh();
    }

    x_catch_display;
}

//=========================================================================

void CProjectDoc::InsertTheme( void )
{
    x_try;

    if( g_Project.IsProjectOpen() == FALSE )
        x_throw( "You must have a working project to insert a theme" );

    CFileDialog dlgFileOpen(TRUE, 
                    _T("Project"), 
                    _T("Theme.theme"), 
                    0, 
                    "Theme Files (*.theme)|*.theme||");

    CString strOpenPath(g_Settings.GetSourcePath());
    strOpenPath += "\\Themes";
    dlgFileOpen.m_ofn.lpstrInitialDir = strOpenPath;

    if( dlgFileOpen.DoModal() == IDOK )
    {
        CString strPathName = dlgFileOpen.GetPathName();
        g_Project.InsertTheme( strPathName );
        m_pProjectProp->Refresh();
    }

    x_catch_display;
}

//=========================================================================

void CProjectDoc::RemoveTheme( void )
{
    x_try;

    CListBoxDlg dlg;
    dlg.SetDisplayText("Which of the following themes do you wish to remove from the project?");
    
    for (int i = 0; i < g_Project.GetThemeCount(); i++)
    {
        dlg.AddString(g_Project.GetThemeName(i));
    }

    if (dlg.DoModal() == IDOK)
    {
        CString strThemeName = dlg.GetSelection();
        if (!strThemeName.IsEmpty())
        {
            g_Project.RemoveTheme(strThemeName);
            m_pProjectProp->Refresh();
        }
    }

    x_catch_display;
}

//=========================================================================

xbool CProjectDoc::LoadProject( const char* fullLevelName )
{      
    CONTEXT( "CProjectDoc::LoadProject" );

    if (CFileSearch::DoesFileExist(fullLevelName))
    {
        g_Project.Load( fullLevelName );
        m_pProjectProp->SetInterface( g_Project, FALSE );
        
        CProjectFrame* pFrame = (CProjectFrame*)GetView()->GetParentFrame();
        if (pFrame)
        {
            pFrame->SetProject(g_Project.GetName());

	        POSITION pos = GetFirstViewPosition();
	        CProjectView* pView = (CProjectView*)GetNextView (pos); 

            //now load the readme file if it exists
            CString strReadme(fullLevelName);
            int iSlash = strReadme.ReverseFind('\\');
            if (iSlash != -1 && pView)
            {
                strReadme = strReadme.Left(iSlash);
                strReadme += "\\readme.rtf";

                CTextEditor& TextEdit = pView->GetTextEditor();

                if( CFileSearch::DoesFileExist(strReadme) )
                {
                    TextEdit.FileOpen( strReadme );
                }
                else
                {
                    // Also if there is already a text file read it in
                    strReadme = strReadme.Left(iSlash);
                    strReadme += "\\readme.txt";

                    if( CFileSearch::DoesFileExist(strReadme) )
                    {
                        TextEdit.ImportText( strReadme );
                        ::MessageBox( NULL, "The Readme.txt file should be remove from perforce\nand the project to save the new readme.rtf", "WARNING", MB_OK );

                        // Create the file
                        strReadme = strReadme.Left(iSlash);
                        strReadme += "\\readme.rtf";
                        TextEdit.FileSave( strReadme );
                    }
                }                
            }
            return true;
        }
    }

    return false;
}