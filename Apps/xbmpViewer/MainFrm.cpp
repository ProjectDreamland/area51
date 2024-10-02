// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "xbmpViewer.h"
#include "aux_bitmap.hpp"
#include "MainFrm.h"
#include "Resource.h"
#include "Progress.h"
#include "ConvertSettingsDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//#define HappyDebuger

extern CString g_CmdLinePath;
extern CString g_CmdLineFile;

CMainFrame* g_pMainFrame = NULL;

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CXTFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CXTFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_SETFOCUS()
    ON_WM_ACTIVATE()
    ON_MESSAGE( NM_DIRCHANGED, OnDirChanged )
    ON_MESSAGE( NM_NEWBITMAP, OnNewBitmap )
    ON_COMMAND(ID_CONVERT_TGA, OnConvertTga)
    ON_UPDATE_COMMAND_UI(ID_CONVERT_TGA, OnUpdateConvertTga)
    ON_COMMAND(ID_CONVERT_XBMP, OnConvertXbmp)
    ON_UPDATE_COMMAND_UI(ID_CONVERT_XBMP, OnUpdateConvertXbmp)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
    ID_SEPARATOR,           // status line indicator
    0,
    0, //ID_INDICATOR_CAPS,
    0, //ID_INDICATOR_NUM,
    0, //ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
    // TODO: add member initialization code here
//    x_Init();

    g_pMainFrame = this;
}

CMainFrame::~CMainFrame()
{
//    x_Kill();
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CXTFrameWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Enable/Disable XP GUI Mode
    xtAfxData.bXPMode = TRUE;

    // Enable/Disable Menu Shadows
    xtAfxData.bMenuShadows = TRUE;

    // create the window to occupy the client area of the frame
    if(!m_wndFileList.Create( NULL, NULL, AFX_WS_DEFAULT_VIEW|WS_CLIPCHILDREN|WS_CHILD, CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST ) )
    {
        TRACE0("Failed to create view window\n");
        return -1;
    }

    if (!m_wndMenuBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndMenuBar.LoadMenuBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create menubar\n");
        return -1;      // fail to create
    }
    
    if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
        | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
        !m_wndToolBar.LoadToolBar(IDR_MAINFRAME))
    {
        TRACE0("Failed to create toolbar\n");
        return -1;      // fail to create
    }

    if( !m_wndExplorerBar.Create( this, IDW_EXPLORERBAR, _T("Explorer"), CSize(400,400), CBRS_LEFT) )
    {
        TRACE0( "Failed to create explorer bar\n" );
        return -1;
    }

    if( !m_wndPreviewBar.Create( this, IDW_PREVIEWBAR, _T("Preview"), CSize(400,400), CBRS_BOTTOM) )
    {
        TRACE0( "Failed to create preview bar\n" );
        return -1;
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
          sizeof(indicators)/sizeof(UINT)))
    {
        TRACE0("Failed to create status bar\n");
        return -1;      // fail to create
    }

    
    XT_STATUSPANE   PaneInfo;
    PaneInfo.cxText = 100;
    PaneInfo.nID = 0;
    PaneInfo.nStyle = 0;
    PaneInfo.nFlags = 0;
    PaneInfo.strText = _T("");
    m_wndStatusBar.AddIndicator( 10000+INDICATOR_FOCUS, INDICATOR_FOCUS );
    m_wndStatusBar.SetPaneInfoEx( INDICATOR_FOCUS, &PaneInfo );
    m_wndStatusBar.AddIndicator( 10000+INDICATOR_TOTAL, INDICATOR_TOTAL );
    m_wndStatusBar.SetPaneInfoEx( INDICATOR_TOTAL, &PaneInfo );
    m_wndStatusBar.AddIndicator( 10000+INDICATOR_SELECTED, INDICATOR_SELECTED );
    m_wndStatusBar.SetPaneInfoEx( INDICATOR_SELECTED, &PaneInfo );
 

    m_wndStatusBar.SetPaneWidth( 1, 100 );
    m_wndStatusBar.SetPaneWidth( 2, 100 );
    m_wndStatusBar.SetPaneWidth( 3, 100 );
    m_wndStatusBar.SetPaneWidth( 4, 100 );

    // Setup docking
    m_wndMenuBar.    EnableDockingEx( CBRS_ALIGN_ANY, CBRS_XT_SEMIFLAT );
    m_wndToolBar.    EnableDockingEx( CBRS_ALIGN_ANY, CBRS_XT_SEMIFLAT );
    m_wndExplorerBar.EnableDockingEx( CBRS_ALIGN_ANY, CBRS_XT_SEMIFLAT|CBRS_XT_GRIPPER_GRAD );
    m_wndPreviewBar. EnableDockingEx( CBRS_ALIGN_ANY, CBRS_XT_SEMIFLAT|CBRS_XT_GRIPPER_GRAD );
    EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_SEMIFLAT);
    DockControlBar(&m_wndMenuBar);
    DockControlBar(&m_wndToolBar);
    DockControlBar(&m_wndExplorerBar );
    DockControlBar(&m_wndPreviewBar);

    // Cool menus
    InstallCoolMenus(IDR_MAINFRAME);

    m_wndFileList.SetActiveWindow();
    m_wndFileList.SendMessage( WM_ACTIVATE, WA_ACTIVE );

    return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
    if( !CXTFrameWnd::PreCreateWindow(cs) )
        return FALSE;
    // TODO: Modify the Window class or styles here by modifying
    //  the CREATESTRUCT cs

    cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
    cs.lpszClass = AfxRegisterWndClass( 0 );
    // Helps to reduce screen flicker.
    cs.lpszClass = AfxRegisterWndClass( 0, NULL, NULL, AfxGetApp()->LoadIcon(IDR_MAINFRAME) );

    return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CXTFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
    CXTFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
    // forward focus to the view window
    m_wndFileList.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
    // let the view have first crack at the command
    if( m_wndFileList.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo) )
        return TRUE;

    // otherwise, do default handling
    return CXTFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnClose() 
{
    // Save control bar postion.
    SaveBarState(_T("Control Bar State"));
/* FUCK THIS SHIT
    CXTRegistryManager regManager;
    regManager.WriteProfileString( _T("Settings"), _T("Current Folder"), m_wndExplorerBar.GetPath() );
    regManager.WriteProfileString( _T("Settings"), _T("Current File"  ), g_CmdLineFile              );
*/
    // Save frame window size and position.
    m_wndPosition.SaveWindowPos(this);

    CXTFrameWnd::OnClose();
}

BOOL CMainFrame::ShowWindowEx(int nCmdShow)
{
    ASSERT_VALID(this);

    // Restore control bar postion.
    LoadBarState(_T("Control Bar State"));

    // Restore frame window size and position.
    m_wndPosition.LoadWindowPos(this);
    nCmdShow = m_wndPosition.showCmd;

    // Load Settings from registry
/* FUCK THIS SHIT
    CXTRegistryManager regManager;
    CString Folder = regManager.GetProfileString( _T("Settings"), _T("Current Folder"), _T("") );
    CString File   = regManager.GetProfileString( _T("Settings"), _T("Current File"  ), _T("") );


    // If not found then set to c:
    if( Folder.IsEmpty() )
        Folder = "C:\\";

    // Check if a file was specified on the command line
    if( g_CmdLinePath.GetLength() > 0 )
        Folder = g_CmdLinePath;
    else
        g_CmdLinePath = Folder+"\\";

    // Check if a file was specified on the command line
    if( g_CmdLineFile.GetLength() == 0 )
        g_CmdLineFile = File;

    m_wndExplorerBar.SetPath( Folder );
    */

    return ShowWindow(nCmdShow);
}

LRESULT CMainFrame::OnDirChanged( WPARAM wParam, LPARAM lParam )
{
    m_wndFileList.SendMessage( NM_DIRCHANGED, wParam, lParam );
    return 0;
}

LRESULT CMainFrame::OnNewBitmap( WPARAM wParam, LPARAM lParam )
{
    m_wndPreviewBar.SendMessage( NM_NEWBITMAP, wParam, lParam );
    return 0;
}


void CMainFrame::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
    CXTFrameWnd::OnActivate(nState, pWndOther, bMinimized);

    // TODO: Add your message handler code here
    m_wndFileList.SendMessage( WM_ACTIVATE, WA_ACTIVE );
}

void CMainFrame::SetStatusPane( int Index, const CString& String )
{
//    SetFont( &xtAfxData.font );
    CDC* pDC = m_wndStatusBar.GetDC();
    ASSERT( pDC );
    pDC->SelectObject( xtAfxData.font );
    CSize sz = pDC->GetOutputTextExtent( String );
    m_wndStatusBar.ReleaseDC( pDC );

    XT_STATUSPANE PaneInfo;
    PaneInfo.cxText  = sz.cx;
    PaneInfo.nID     = 0;
    PaneInfo.nStyle  = 0;
    PaneInfo.nFlags  = 0;
    PaneInfo.strText = String;

    m_wndStatusBar.SetRedraw( FALSE );
    m_wndStatusBar.SetPaneInfoEx( Index, &PaneInfo );
    m_wndStatusBar.SetRedraw( TRUE );
    m_wndStatusBar.RedrawWindow();
}

/////////////////////////////////////////////////////////////////////////////
//// XBMP TO TGA
/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateConvertTga(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( TRUE ); //m_wndFileList.GetSelectedCount() > 0 );
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnConvertTga() 
{
    // Get the export folder
    IShellFolder* pShellFolder;
    if( SHGetDesktopFolder( &pShellFolder ) == NOERROR )
    {
        LPITEMIDLIST    pIDList;

        if( pShellFolder->ParseDisplayName( NULL, NULL, L"", NULL, &pIDList, NULL ) == NOERROR )
        {
            if( pIDList )
            {
                BROWSEINFO BrowseInfo;
                BrowseInfo.hwndOwner = GetSafeHwnd();
                BrowseInfo.pidlRoot = pIDList;
                BrowseInfo.pszDisplayName = NULL;
                BrowseInfo.ulFlags = BIF_NEWDIALOGSTYLE;
                BrowseInfo.lpfn = NULL;
                BrowseInfo.lParam = NULL;
                BrowseInfo.iImage = 0;
                BrowseInfo.lpszTitle = NULL;

                LPITEMIDLIST pIDList = SHBrowseForFolder( &BrowseInfo );

                if( pIDList )
                {
                    char Buffer[32768] = {0};

                    if( SHGetPathFromIDList( pIDList, Buffer ) )
                    {
                        xstring OutPath = Buffer;
                        if( (OutPath.GetLength() > 0) &&
                            (OutPath[OutPath.GetLength()-1] != '/') &&
                            (OutPath[OutPath.GetLength()-1] != '\\') )
                        {
                            OutPath += "\\";

                            // Get the root path and list of files selected
                            xstring             Path  = m_wndFileList.GetPath();
                            xarray<file_rec*>   Array = m_wndFileList.GetSelected();

                            CProgress Progress;

                            Progress.Create( IDD_PROGRESS, this );
                            Progress.SetWindowText( "Converting..." );
                            Progress.ShowWindow( SW_SHOW );

                            // Iterate over the files
                            for( s32 i=0 ; i<Array.GetCount() ; i++ )
                            {
                                file_rec* pFile = Array[i];
                                if( pFile )
                                {
                                    char Drive[_MAX_DRIVE];
                                    char Dir[_MAX_DIR];
                                    char FName[_MAX_FNAME];
                                    char Ext[_MAX_EXT];
                                    _splitpath( pFile->Name, Drive, Dir, FName, Ext );
                                    xstring InFile  = Path + pFile->Name;
                                    xstring OutFile = OutPath + FName + ".tga";

                                    Progress.SetText( xfs( "%d of %d - %s", i+1, Array.GetCount(), pFile->Name ) );

                                    xbitmap b;
                                    auxbmp_Load( b, InFile );

                                    if( b.GetFlags() & xbitmap::FLAG_GCN_DATA_SWIZZLED )
                                    {
                                        b.GCNUnswizzleData();
                                    }

                                    switch( b.GetFormat() )
                                    {
                                        case xbitmap::FMT_DXT1:
                                        case xbitmap::FMT_DXT3:
                                        case xbitmap::FMT_DXT5:
                                            auxbmp_Decompress( b );
                                            break;
                                    }

                                    b.SaveTGA( OutFile );

                                    Progress.SetProgress( (i+1)*100 / Array.GetCount() );
                                }
                            }

                            Progress.DestroyWindow();
                        }
                    }
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
//// TGA TO XBMP
/////////////////////////////////////////////////////////////////////////////

bool IsPowerOfTwo(int n)
{
    return (n > 0) && ((n & (n - 1)) == 0);
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnUpdateConvertXbmp(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable( TRUE );
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::OnConvertXbmp() 
{
    //Get settings window
    CConvertSettingsDialog dlg;

    //OK = Cheezy man
    if (dlg.DoModal() == IDOK)
    {
        //Get settings
        CString platform = dlg.m_SelectedPlatform;
        CString format = dlg.m_SelectedFormat;
        int mipLevels = dlg.m_MipLevels;

        //Get the export folder
        IShellFolder* pShellFolder;
        if (SHGetDesktopFolder(&pShellFolder) == NOERROR)
        {
            LPITEMIDLIST pIDList;

            if (pShellFolder->ParseDisplayName(NULL, NULL, L"", NULL, &pIDList, NULL) == NOERROR)
            {
                if (pIDList)
                {
                    BROWSEINFO BrowseInfo;
                    BrowseInfo.hwndOwner = GetSafeHwnd();
                    BrowseInfo.pidlRoot = pIDList;
                    BrowseInfo.pszDisplayName = NULL;
                    BrowseInfo.ulFlags = BIF_NEWDIALOGSTYLE;
                    BrowseInfo.lpfn = NULL;
                    BrowseInfo.lParam = NULL;
                    BrowseInfo.iImage = 0;
                    BrowseInfo.lpszTitle = NULL;

                    LPITEMIDLIST pIDList = SHBrowseForFolder(&BrowseInfo);

                    if (pIDList)
                    {
                        char Buffer[32768] = {0};

                        if (SHGetPathFromIDList(pIDList, Buffer))
                        {
                            xstring OutPath = Buffer;
                            if ((OutPath.GetLength() > 0) &&
                                (OutPath[OutPath.GetLength() - 1] != '/') &&
                                (OutPath[OutPath.GetLength() - 1] != '\\'))
                            {
                                OutPath += "\\";

                                //Get the root path and list of files selected
                                xstring Path = m_wndFileList.GetPath();
                                xarray<file_rec*> Array = m_wndFileList.GetSelected();

                                CProgress Progress;
                                Progress.Create(IDD_PROGRESS, this);
                                Progress.SetWindowText("Converting TGA to XBMP...");
                                Progress.ShowWindow(SW_SHOW);

                                //Go through each file
                                for (s32 i = 0; i < Array.GetCount(); i++)
                                {
                                    file_rec* pFile = Array[i];
                                    if (pFile)
                                    {
                                        char Drive[_MAX_DRIVE];
                                        char Dir[_MAX_DIR];
                                        char FName[_MAX_FNAME];
                                        char Ext[_MAX_EXT];
                                        _splitpath(pFile->Name, Drive, Dir, FName, Ext);
                                        xstring InFile = Path + pFile->Name;
                                        xstring OutFile = OutPath + FName + ".xbmp";

                                        Progress.SetText(xfs("%d of %d - %s", i + 1, Array.GetCount(), pFile->Name));

                                        //Load the TGA file
                                        xbitmap b;
                                        if (!auxbmp_Load(b, InFile))                
                                    
                                        //IT IS VERY IMPORTANT TO FOLLOW CONSISTENCY!!!!
                                        //COMPRESSION TYPE
                                        //PLATFORM DEFINICATIONS
                                        //MIPS
                                        
/////////////////////////////////////////////////////////////////////////////
//// YANDERE DEV CODE
/////////////////////////////////////////////////////////////////////////////                                        

                                        if (format == _T("32_RGBA_8888"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_32_RGBA_8888);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 32_RGBA_8888 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("32_RGBU_8888"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_32_RGBU_8888);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 32_RGBU_8888 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("32_ARGB_8888"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_32_ARGB_8888);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 32_ARGB_8888 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("32_URGB_8888"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_32_URGB_8888);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 32_URGB_8888 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("24_RGB_888"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_24_RGB_888);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 24_RGB_888 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("16_RGBA_4444"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_16_RGBA_4444);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 16_RGBA_4444 !!!"));
                                            #endif
                                        }
                                        else if (format == __T("16_ARGB_4444"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_16_ARGB_4444);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 16_ARGB_4444 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("16_RGBA_5551"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_16_RGBA_5551);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 16_RGBA_5551 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("16_RGBU_5551"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_16_RGBU_5551);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 16_RGBU_5551 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("16_ARGB_1555"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_16_ARGB_1555);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 16_ARGB_1555 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("16_URGB_1555"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_16_URGB_1555);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 16_URGB_1555 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("16_RGB_565"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_16_RGB_565);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression 16_RGB_565 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("P4_RGB_565"))
                                        {
                                            b.ConvertFormat(xbitmap::FMT_P4_RGB_565);
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected compression P4_RGB_565 !!!"));
                                            #endif
                                        }
                                        else if (format == _T("NoComp"))
                                        {
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected NoComp !!!"));
                                            #endif
                                        }
                                        
/////////////////////////////////////////////////////////////////////////////
//// YANDERE DEV CODE - END
/////////////////////////////////////////////////////////////////////////////

                                        if (platform == _T("PC") || platform == _T("Xbox"))
                                        {
                                            auxbmp_ConvertToD3D(b); // PC oder Xducks conversion.
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected PC or XBOX VERSION !!!"));
                                            #endif
                                        }
                                        else if (platform == _T("PS2"))
                                        {
                                            auxbmp_ConvertToPS2(b);  // PS2 conversion.
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected PS2 VERSION !!!"));
                                            #endif
                                        }
                                        else if (platform == _T("GameCube"))
                                        {
                                            auxbmp_ConvertToGCN(b);  // GameCube conversion.
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected GameCum VERSION !!!"));
                                            #endif
                                        }
                                        else if (platform == _T("Native"))
                                        {
                                            auxbmp_ConvertToNative(b);  // Native conversion.
                                            #ifdef HappyDebuger
                                            AfxMessageBox(_T("Selected Native VERSION !!!"));
                                            #endif
                                        }                                                                                  

                                        //MIPS only support images with a power of two.
                                        int width = b.GetWidth();
                                        int height = b.GetHeight();

                                        if (mipLevels > 0 && mipLevels <= 16)
                                        {
                                            if (IsPowerOfTwo(width) && IsPowerOfTwo(height))
                                            {
                                                b.BuildMips(mipLevels);
                                            }
                                            else
                                            {
                                                AfxMessageBox(_T("Warning: Generation of MIP maps possible only for images with a power of two!"));
                                            }
                                        }

                                        //Save the file as XBMP
                                        if (b.Save(OutFile))
                                        {
                                            Progress.SetProgress((i + 1) * 100 / Array.GetCount());
                                        }
                                    }
                                }

                                Progress.DestroyWindow();
                            }
                        }
                    }
                }
            }
        }
    }
}

