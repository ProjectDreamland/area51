// MeshWorkspaceView.cpp : implementation file
//

#include "StdAfx.h"
#include "MeshWorkspaceView.h"
#include "MeshWorkspaceDoc.h"
#include "resource.h"
#include "MeshViewer_Frame.h"
#include "MeshViewer_Doc.h"
#include "MeshViewer_View.h"

#include "..\WinControls\FileSearch.h"
#include "..\EDRscDesc\RSCDesc.hpp"
#include "..\editor\project.hpp"

#include "RigidDesc.hpp"
#include "SkinDesc.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceView

IMPLEMENT_DYNCREATE(CMeshWorkspaceView, CView)

CMeshWorkspaceView::CMeshWorkspaceView()
{
}

CMeshWorkspaceView::~CMeshWorkspaceView()
{
}


BEGIN_MESSAGE_MAP(CMeshWorkspaceView, CView)
	//{{AFX_MSG_MAP(CMeshWorkspaceView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_LBN_DBLCLK(IDR_MV_THEME_LIST, OnDblclkListBox)
	ON_NOTIFY(TVN_SELCHANGED, IDR_MV_THEME_FOLDERS, OnSelchangedThemeFolder)
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceView drawing

void CMeshWorkspaceView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceView diagnostics

#ifdef _DEBUG
void CMeshWorkspaceView::AssertValid() const
{
	CView::AssertValid();
}

void CMeshWorkspaceView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceView message handlers

int CMeshWorkspaceView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

    // Create the ToolBar
/*
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | TBBS_WRAPPED) ||
		!m_wndToolBar.LoadToolBar(IDR_MESHVIEW_WRKSPC_FILTER))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
*/
    
    if (!m_stTitle.Create("No Theme Loaded",WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                         CRect(0,0,0,0),this,IDR_MV_THEME_TITLE))
    {
		TRACE0("Failed to create static\n");
		return -1;      // fail to create
    }

    if (!m_lstBox.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | LBS_SORT | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
                         CRect(0,0,0,0),this,IDR_MV_THEME_LIST))
    {
		TRACE0("Failed to create listbox\n");
		return -1;      // fail to create
    }

    if (!m_fbcDirs.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                           TVS_EDITLABELS | TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDR_MV_THEME_FOLDERS))
    {
		TRACE0("Failed to create folders\n");
        return -1;	      
    }
    m_fbcDirs.UsePreviousPathAsDisplay(TRUE);

    RefreshThemeRsc();

	return 0;
}

//==============================================================================

void CMeshWorkspaceView::OnInitialUpdate() 
{
	CView::OnInitialUpdate();
}	

//==============================================================================

void CMeshWorkspaceView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

//	CSize size = m_wndToolBar.CalcLayout(LM_HORZ| LM_COMMIT,cx);
//	m_wndToolBar.MoveWindow(0,0,cx,size.cy);

    int nSizeDirs = (cy-16)/3;
    int nSizeBox = (cy-16) - nSizeDirs;
    m_fbcDirs.MoveWindow(0,0,cx,nSizeDirs);
    m_stTitle.MoveWindow(0,nSizeDirs,cx,16);
    m_lstBox.MoveWindow(0,16+nSizeDirs,cx,nSizeBox);
}

//==============================================================================

void CMeshWorkspaceView::RefreshThemeRsc()
{
    m_fbcDirs.ClearTree();

    char cThemePath[MAX_PATH];
    for( int j = g_Project.GetFirstResourceDir( cThemePath ); j != -1; j = g_Project.GetNextResourceDir( j, cThemePath ) )
    {
        CString strThemePath(cThemePath);
/*
        int nThemeLen = strThemePath.GetLength();
        CFileSearch::FormatPath(strThemePath);

        CString strThemeName;
        strThemeName = strThemePath.Left(strThemePath.ReverseFind('\\'));
        strThemeName = strThemeName.Right(strThemeName.GetLength() - strThemeName.ReverseFind('\\') - 1);

        strThemePath = strThemeName + strThemePath.Right( strThemePath.GetLength() - nThemeLen);
*/
        m_fbcDirs.BuildTreeFromPath(strThemePath, "", "");
    }

    RefreshGeomList();
}

//==============================================================================

void CMeshWorkspaceView::RefreshGeomList()
{
    m_lstBox.ResetContent();

    CString strRscPath = GetRscPath();
    if (!strRscPath.IsEmpty())
    {
        CFileSearch::FormatPath(strRscPath);

        CString strThemeName = strRscPath.Left(strRscPath.ReverseFind('\\'));
        strThemeName = strThemeName.Right(strThemeName.GetLength() - strThemeName.ReverseFind('\\') - 1);

        m_stTitle.SetWindowText(strThemeName);

        //add rigidgeom items
        CFileSearch fSearch;
        fSearch.AddDirPath(strRscPath);
        fSearch.GetFiles("*.rigidgeom");
        CString strNextFile = fSearch.GetNextFile(TRUE);
        while (!strNextFile.IsEmpty())
        {   
            m_lstBox.AddString(strNextFile);  
            strNextFile = fSearch.GetNextFile(TRUE);
        }

        //add skingeom items
        fSearch.ClearFiles();
        fSearch.GetFiles("*.skingeom");
        strNextFile = fSearch.GetNextFile(TRUE);
        while (!strNextFile.IsEmpty())
        {   
            m_lstBox.AddString(strNextFile);  
            strNextFile = fSearch.GetNextFile(TRUE);
        }
    }
}

//==============================================================================

BOOL CMeshWorkspaceView::CanAddToTheme()
{
    return (g_Project.IsProjectOpen() && (!GetRscPath().IsEmpty()));
}

//==============================================================================

CString CMeshWorkspaceView::GetRscPath()
{
    return m_fbcDirs.GetSelectedPath();
}

//==============================================================================

void CMeshWorkspaceView::OnDblclkListBox() 
{
	int nIndex = m_lstBox.GetCurSel();
    if (nIndex!=LB_ERR)
    {
        CString strString;
        m_lstBox.GetText( nIndex, strString );
        if (!strString.IsEmpty())
        {
            CString strPath = GetRscPath() + "\\" + strString;

            //set up query
            prop_query propQuery;
            char cMatxString[MAX_PATH];
            propQuery.RQueryFileName( "ResDesc\\FileName", &cMatxString[0]);

            text_in GeomFile;
            GeomFile.OpenFile(strPath);

            //get extension
            strString = strString.Right(strString.GetLength() - strString.ReverseFind('.') -1);

            BOOL bFound = FALSE;
            if (strString.CompareNoCase("rigidgeom") == 0)
            {
                rigidgeom_rsc_desc RscDesc;
                RscDesc.OnLoad(GeomFile);
                bFound = RscDesc.OnProperty(propQuery);
            }
            else if (strString.CompareNoCase("skingeom") == 0)
            {
                skingeom_rsc_desc RscDesc;
                RscDesc.OnLoad(GeomFile);
                bFound = RscDesc.OnProperty(propQuery);
            }

            if (bFound)
            {
                GetDocument()->GetFramePointer()->m_pDoc->LoadMeshFromFile(cMatxString);
                POSITION pos = GetDocument()->GetFramePointer()->m_pDoc->GetFirstViewPosition();
                while (pos != NULL)
                {
                    CMeshViewer_View* pView = (CMeshViewer_View*)GetDocument()->GetFramePointer()->m_pDoc->GetNextView(pos);
                    pView->RedrawWindow();
                }  
            }

            GeomFile.CloseFile();
        }
    }
}

//==============================================================================

void CMeshWorkspaceView::OnSelchangedThemeFolder(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

    RefreshGeomList();
    m_fbcDirs.RedrawWindow();

	*pResult = 0;
}

//==============================================================================

BOOL CMeshWorkspaceView::OnEraseBkgnd(CDC* pDC) 
{
	// TODO: Add your message handler code here and/or call default
	
	return CView::OnEraseBkgnd(pDC);
}
