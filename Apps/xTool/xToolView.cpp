// xToolView.cpp : implementation of the CxToolView class
//

#include "stdafx.h"
#include "xTool.h"

#include "xToolDoc.h"
#include "xToolView.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CxToolView

IMPLEMENT_DYNCREATE(CxToolView, CXTTabView)

BEGIN_MESSAGE_MAP(CxToolView, CXTTabView)
	//{{AFX_MSG_MAP(CxToolView)
	ON_WM_CREATE()
	ON_COMMAND(ID_LOG_EXPORT_CSV, OnLogExportCsv)
	ON_COMMAND(ID_FILE_IMPORT_MAPFILE, OnFileImportMapfile)
	ON_COMMAND(ID_MEMLOG_EXPORT_CSV, OnMemlogExportCsv)
	ON_COMMAND(ID_MEMLOG_EXPORT_ACTIVE_CSV, OnMemlogExportActiveCsv)
	//}}AFX_MSG_MAP
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CXTTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CXTTabView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CXTTabView::OnFilePrintPreview)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CxToolView construction/destruction

CxToolView::CxToolView()
{
}

CxToolView::~CxToolView()
{
}

BOOL CxToolView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CXTTabView::PreCreateWindow(cs))
		return FALSE;

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CxToolView printing

BOOL CxToolView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CxToolView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CxToolView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}

/////////////////////////////////////////////////////////////////////////////
// CxToolView diagnostics

#ifdef _DEBUG
void CxToolView::AssertValid() const
{
	// TODO: Why doesn't this work?
    CXTTabView::AssertValid();
}

void CxToolView::Dump(CDumpContext& dc) const
{
	CXTTabView::Dump(dc);
}

CxToolDoc* CxToolView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CxToolDoc)));
	return (CxToolDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CxToolView message handlers

int CxToolView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CXTTabView::OnCreate(lpCreateStruct) == -1)
		return -1;

    CCreateContext* pContext = (CCreateContext*)lpCreateStruct->lpCreateParams;
    ASSERT( pContext );
    CCreateContext cc = *pContext;
    ZeroMemory( &cc, sizeof(cc) );
    cc.m_pCurrentDoc = pContext->m_pCurrentDoc;
    ASSERT( cc.m_pCurrentDoc );

	// Create pane
//	if( !m_wndLog.Create( NULL, _T(""), WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,0,0), this, MAKEINTRESOURCE(IDR_MENU_LOG), 0, &cc ) )
	if( !m_wndLog.Create( NULL, _T(""), WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,0,0), this, NULL, 0, &cc ) )
	{
		TRACE0( "Failed to create Log window.\n" );
		return -1;
	}

	// Create pane
//	if( !m_wndMemory.Create( NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,0,0), this, MAKEINTRESOURCE(IDR_MENU_MEMORY), 0, &cc ) )
	if( !m_wndMemory.Create( NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,0,0), this, NULL, 0, &cc ) )
	{
		TRACE0( "Failed to create Memory window.\n" );
		return -1;
	}

    // Create pane
    if( !m_wnd3D.Create( NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,100,100), this, NULL, 0, &cc ) )
    {
        TRACE0( "Failed to create 3D window.\n" );
        return -1;
    }

	// Create pane
	if( !m_wndIO.Create( NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,0,0), this, NULL, 0, &cc ) )
	{
		TRACE0( "Failed to create IO window.\n" );
		return -1;
	}

	// Create pane
    if( !m_wndStats.Create( NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,0,0), this, NULL, 0, &cc ) )
	{
		TRACE0( "Failed to create Stats window.\n" );
		return -1;
	}

	// Create pane
	if( !m_wndTweaks.Create( NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,0,0), this, NULL, 0, &cc ) )
	{
		TRACE0( "Failed to create Tweaks window.\n" );
		return -1;
	}

	// Create pane
	if( !m_wndProfile.Create( NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,0,0), this, NULL, 0, &cc ) )
	{
		TRACE0( "Failed to create Profile window.\n" );
		return -1;
	}

	// Create pane
	if( !m_wndScreenshot.Create( NULL, NULL, WS_CHILD|WS_VISIBLE|WS_CLIPCHILDREN, CRect(0,0,100,100), this, NULL, 0, &cc ) )
	{
		TRACE0( "Failed to create Screenshot window.\n" );
		return -1;
	}

	// Insert the controls into the tabbed view.
    AddControl( CResString(IDS_TAB_LOG       ), &m_wndLog        );
    AddControl( CResString(IDS_TAB_MEMORY    ), &m_wndMemory     );
    AddControl( CResString(IDS_TAB_3D        ), &m_wnd3D         );
    AddControl( CResString(IDS_TAB_IO        ), &m_wndIO         );
    AddControl( CResString(IDS_TAB_STATS     ), &m_wndStats      );
    AddControl( CResString(IDS_TAB_TWEAKS    ), &m_wndTweaks     );
    AddControl( CResString(IDS_TAB_PROFILE   ), &m_wndProfile    );
    AddControl( CResString(IDS_TAB_SCREENSHOT), &m_wndScreenshot );

	// Set the active view to the first tab.
	SetActiveView( 0 );

    CMainFrame* pMainFrame = (CMainFrame*)GetTopLevelFrame();
//    pMainFrame->ReplaceMenu( IDR_MENU_LOG );

	return 0;
}

/////////////////////////////////////////////////////////////////////////////

void CxToolView::ActivateView( CWnd* pTabView )
{
    CXTTabView::ActivateView( pTabView );

/*
    ASSERT( pTabView->IsKindOf( RUNTIME_CLASS( CFrameBase ) ) );

    CFrameBase* pFrame = (CFrameBase*)pTabView;
    CXTTabView::ActivateView( pTabView );

    CMenu* pMenu = pFrame->GetMenu();
    if( pMenu )
    {
        ((CXTFrameWnd*)GetTopLevelFrame())->GetMenuBar()->LoadMenu( pMenu->GetSafeHmenu(), NULL );
    }
*/
}

/////////////////////////////////////////////////////////////////////////////

void CxToolView::OnLogExportCsv() 
{
    CxToolDoc* pDoc = GetDocument();
	ASSERT( pDoc );

    CString     Path    = pDoc->GetTitle();

    // Strip extension from Path
    char Drive[_MAX_DRIVE];
    char Dir  [_MAX_DIR  ];
    char File [_MAX_FNAME];
    _splitpath( Path, Drive, Dir, File, NULL );
    Path = Drive;
    Path += Dir;
    Path += File;

    CFileDialog FileDialog( FALSE, _T(".csv"), Path, OFN_ENABLESIZING|OFN_OVERWRITEPROMPT, _T("Excel CSV File (*.csv)|*.csv||"), this );
    FileDialog.m_ofn.lpstrTitle = _T("Export log to CSV");
    if( FileDialog.DoModal() == IDOK )
    {
        CString Path = FileDialog.GetPathName();

        pDoc->ExportFilteredLogToCSV( Path );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolView::OnFileImportMapfile() 
{
    CxToolDoc* pDoc = GetDocument();
	ASSERT( pDoc );

    CFileDialog FileDialog( TRUE, _T(".map.txt"), NULL, OFN_ENABLESIZING|OFN_OVERWRITEPROMPT, _T("SN Map File (*.map.txt)|*.map.txt||"), this );
    FileDialog.m_ofn.lpstrTitle = _T("Import SN map file");
    if( FileDialog.DoModal() == IDOK )
    {
        CString Path = FileDialog.GetPathName();
        pDoc->ImportMapFile( Path );
        pDoc->UpdateAllViews( NULL, CxToolDoc::HINT_SYMBOLS_LOADED );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolView::OnMemlogExportCsv() 
{
    CxToolDoc* pDoc = GetDocument();
	ASSERT( pDoc );

    CString     Path    = pDoc->GetTitle();

    // Strip extension from Path
    char Drive[_MAX_DRIVE];
    char Dir  [_MAX_DIR  ];
    char File [_MAX_FNAME];
    _splitpath( Path, Drive, Dir, File, NULL );
    Path = Drive;
    Path += Dir;
    Path += File;

    CFileDialog FileDialog( FALSE, _T(".csv"), Path, OFN_ENABLESIZING|OFN_OVERWRITEPROMPT, _T("Excel CSV File (*.csv)|*.csv||"), this );
    FileDialog.m_ofn.lpstrTitle = _T("Export memory log to CSV");
    if( FileDialog.DoModal() == IDOK )
    {
        CString Path = FileDialog.GetPathName();

        pDoc->ExportMemLogToCSV( Path, FALSE );
    }
}

/////////////////////////////////////////////////////////////////////////////

void CxToolView::OnMemlogExportActiveCsv() 
{
    CxToolDoc* pDoc = GetDocument();
	ASSERT( pDoc );

    CString     Path    = pDoc->GetTitle();

    // Strip extension from Path
    char Drive[_MAX_DRIVE];
    char Dir  [_MAX_DIR  ];
    char File [_MAX_FNAME];
    _splitpath( Path, Drive, Dir, File, NULL );
    Path = Drive;
    Path += Dir;
    Path += File;

    CFileDialog FileDialog( FALSE, _T(".csv"), Path, OFN_ENABLESIZING|OFN_OVERWRITEPROMPT, _T("Excel CSV File (*.csv)|*.csv||"), this );
    FileDialog.m_ofn.lpstrTitle = _T("Export memory log to CSV");
    if( FileDialog.DoModal() == IDOK )
    {
        CString Path = FileDialog.GetPathName();

        pDoc->ExportMemLogToCSV( Path, TRUE );
    }
}

/////////////////////////////////////////////////////////////////////////////

BOOL CxToolView::PreTranslateMessage(MSG* pMsg)
{
	return CCtrlView::PreTranslateMessage( pMsg );
}

