// FileList.cpp : implementation file
//

#include "stdafx.h"
#include "xbmpViewer.h"
#include "FileList.h"
#include "x_threads.hpp"
#include "aux_bitmap.hpp"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CString g_CmdLinePath;
extern CString g_CmdLineFile;

/////////////////////////////////////////////////////////////////////////////

CFileList*                  s_pFileList         = NULL;
xthread*                    s_pFileListThread   = NULL;
xmesgq                      s_MessageQueue(8);
xarray<file_rec*>           s_Files;
CRITICAL_SECTION            s_CriticalSection;

/////////////////////////////////////////////////////////////////////////////

void FileListThread( void )
{
    s_pFileList->Thread();
}

/////////////////////////////////////////////////////////////////////////////
// CFileList

CFileList::CFileList()
{
    // Initialize the critical section one time only.
    InitializeCriticalSection( &s_CriticalSection );
}

CFileList::~CFileList()
{
    EnterCriticalSection( &s_CriticalSection );

    // Unlock all the items and clear the list
    for( s32 i=0 ; i<s_Files.GetCount() ; i++ )
    {
        s_Files[i]->Unlock();
    }
    s_Files.Clear();

    LeaveCriticalSection( &s_CriticalSection );
    DeleteCriticalSection( &s_CriticalSection );
}

void CFileList::Thread( void )
{
    // Loop collecting files in directory when told by UI thread
    while( 1 )
    {
        // Wait until we are told to load the directory
        s_MessageQueue.Recv( MQ_BLOCK );

        EnterCriticalSection( &s_CriticalSection );

        // Unlock all the items and clear the list
        for( s32 i=0 ; i<s_Files.GetCount() ; i++ )
        {
            s_Files[i]->Unlock();
        }
        s_Files.Clear();

        LeaveCriticalSection( &s_CriticalSection );

        // Setup the path for the file find
        CString Path = m_Path;
        Path += "*";

        // File find loop
        CFileFind Finder;
        BOOL bFound = Finder.FindFile( Path );
        while( bFound )
        {
            bFound = Finder.FindNextFile();

            // Get the name of the file
            CString Name = Finder.GetFileName();
            CString NameUpper = Name;
            NameUpper.MakeUpper();

            // Match our criteria
            if( !Finder.IsDirectory() && 
                ( (NameUpper.Right(5) == ".XBMP") ||
                  (NameUpper.Right(4) == ".XBM" ) ||
                  (NameUpper.Right(4) == ".BMP" ) ||
                  (NameUpper.Right(4) == ".PSD" ) ||
                  (NameUpper.Right(4) == ".TGA" ) ) )
            {
                // Add to list
                file_rec* pFile = new file_rec;
                ASSERT( pFile );
                pFile->Lock();
                pFile->Name = Name;
                pFile->Size = (s32)Finder.GetLength();
                pFile->GotInfo = FALSE;
                pFile->InfoDisplayed = FALSE;

                EnterCriticalSection( &s_CriticalSection );
                s_Files.Append( pFile );
                LeaveCriticalSection( &s_CriticalSection );
            }

            // Break out of this loop if we have a new task
            if( !s_MessageQueue.IsEmpty() )
                break;
        }
        Finder.Close();

        // Post message to UI thread if loop was not aborted and start to scan file info
        if( s_MessageQueue.IsEmpty() )
        {
            // Tell UI to add files to list control
            PostMessage( NM_POPULATELIST );

            xtimer    RefreshTimer;
            RefreshTimer.Start();

            // Loop through the files
            for( s32 i=0 ; i<s_Files.GetCount() ; i++ )
            {
                // Exit loop if we have received a new command
                if( !s_MessageQueue.IsEmpty() )
                    break;

                // Form file path
                file_rec* pFile = s_Files[i];
                CString Path = m_Path;
                Path += pFile->Name;

                // Fill in all the info about the bitmap
                {
/*
                    xbitmap b;
                    auxbmp_Load( b, Path );
                    pFile->FormatID = b.GetFormat();
                    pFile->Format   = b.GetFormatInfo().String;
                    pFile->Width    = b.GetWidth();
                    pFile->Height   = b.GetHeight();
                    pFile->BitDepth = b.GetBPP();
                    pFile->nMips    = b.GetNMips();
                    pFile->GotInfo  = TRUE;
*/
                    xbitmap::info Info;
                    auxbmp_Info( Path, Info );
                    const xbitmap::format_info& FormatInfo = xbitmap::GetFormatInfo( Info.Format );

                    pFile->FormatID = Info.Format;
                    pFile->Format   = FormatInfo.pString;
                    pFile->Width    = Info.W;
                    pFile->Height   = Info.H;
                    pFile->BitDepth = FormatInfo.BPP;
                    pFile->nMips    = Info.nMips;
                    pFile->GotInfo  = TRUE;
                }

                if( RefreshTimer.ReadMs() > 200.0f )
                {
                    RefreshTimer.Reset();
//                    PostMessage( NM_REFRESHLIST );
                }
            }
            PostMessage( NM_REFRESHLIST );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////

xarray<file_rec*> CFileList::GetSelected( void )
{
    xarray<file_rec*> Array;

    s32 Count = 0;

    for( s32 i=0 ; i<m_List.GetItemCount() ; i++ )
    {
        file_rec* pFile = (file_rec*)m_List.GetItemData( i );
        ASSERT( pFile );

        BOOL Selected = m_List.GetItemState( i, LVIS_SELECTED );
        if( Selected )
        {
            Array.Append( pFile );
        }
    }

    return Array;
}

/////////////////////////////////////////////////////////////////////////////

xstring CFileList::GetPath( void )
{
    return (const char*)m_Path;
}

/////////////////////////////////////////////////////////////////////////////

static int CALLBACK fnCompare( LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort )
{
    s32             SortCode = 0;
    CListCtrlEx*    pList   = (CListCtrlEx*)lParamSort;
    file_rec*       pFile1  = (file_rec*)lParam1;
    file_rec*       pFile2  = (file_rec*)lParam2;
    ASSERT( pFile1 );
    ASSERT( pFile2 );

    s32             iColumn = pList->GetSortColumn();
    if( iColumn == 0 )
        iColumn = 1;
    xbool           Descending = iColumn < 0;

    // Get sortcode based on column
    switch( x_abs(iColumn) )
    {
    case 1:
        SortCode = x_strcmp( pFile1->Name, pFile2->Name );
        break;
    case 2:
        SortCode = pFile1->Size - pFile2->Size;
        break;
    case 3:
        SortCode = pFile1->Size - pFile2->Size;
        break;
    case 4:
        SortCode = pFile1->nMips - pFile2->nMips;
        break;
    case 5:
        {
            char Drive[X_MAX_DRIVE];
            char Dir[X_MAX_DIR];
            char FName[X_MAX_FNAME];
            char Ext1[X_MAX_EXT];
            char Ext2[X_MAX_EXT];
            x_splitpath( pFile1->Name, Drive, Dir, FName, Ext1 );
            x_splitpath( pFile2->Name, Drive, Dir, FName, Ext2 );
            strlwr( Ext1 );
            strlwr( Ext2 );
            SortCode = x_strcmp( &Ext1[1], &Ext2[1] );
        }
        break;
    case 6:
//        SortCode = pFile1->FormatID - pFile2->FormatID;
        SortCode = x_strcmp( pFile1->Format, pFile2->Format );
        break;
    }

    if( Descending )
        SortCode = -SortCode;

    return SortCode;
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CFileList, CWnd)
    //{{AFX_MSG_MAP(CFileList)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_WM_DESTROY()
    ON_WM_RBUTTONDOWN()
    ON_WM_CONTEXTMENU()
    ON_MESSAGE( NM_DIRCHANGED, OnDirChanged )
    ON_MESSAGE( NM_POPULATELIST, OnPopulateList )
    ON_MESSAGE( NM_REFRESHLIST, OnRefreshList )
    ON_NOTIFY( LVN_ITEMCHANGED, AFX_IDW_PANE_FIRST, OnItemChanged )
    ON_NOTIFY( LVN_COLUMNCLICK, AFX_IDW_PANE_FIRST, OnColumnClick )
    ON_NOTIFY( LVN_DELETEALLITEMS, AFX_IDW_PANE_FIRST, OnDeleteAllItems )
    ON_COMMAND(ID_CONTEXT_CONVERT_TGA, &CFileList::OnContextConvertTga)
    ON_COMMAND(ID_CONTEXT_CONVERT_XBMP, &CFileList::OnContextConvertXbmp)
    ON_WM_ACTIVATE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFileList message handlers

BOOL CFileList::PreCreateWindow(CREATESTRUCT& cs) 
{
    // TODO: Add your specialized code here and/or call the base class

    if( !CWnd::PreCreateWindow(cs) )
        return FALSE;

    cs.dwExStyle |= WS_EX_CLIENTEDGE;
    cs.style &= ~WS_BORDER;
    cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
        ::LoadCursor(NULL, IDC_ARROW), HBRUSH(COLOR_WINDOW+1), NULL);

    return TRUE;
}

int CFileList::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    if (CWnd::OnCreate(lpCreateStruct) == -1)
        return -1;

    // Create the list
    m_List.Create( WS_VISIBLE|WS_CHILD|LVS_REPORT|LVS_SHOWSELALWAYS/*|LVS_SINGLESEL*/, CRect(0,0,0,0), this, AFX_IDW_PANE_FIRST );
    m_List.SetExtendedStyle( LVS_EX_FULLROWSELECT|/*LVS_EX_GRIDLINES|*/LVS_EX_HEADERDRAGDROP|LVS_EX_LABELTIP|LVS_EX_ONECLICKACTIVATE|LVS_EX_UNDERLINEHOT );
    m_List.EnableSortIcon();

    CHeaderCtrl* pHeader = m_List.GetHeaderCtrl();
    pHeader->ModifyStyle( 0, HDS_HOTTRACK );

    // Create list columns
    m_List.InsertColumn( 0, "Name"      , LVCFMT_LEFT , 256, -1 );
    m_List.InsertColumn( 1, "Size"      , LVCFMT_RIGHT,  96, 1 );
    m_List.InsertColumn( 2, "Dimensions", LVCFMT_RIGHT, 128, 2 );
    m_List.InsertColumn( 3, "Mips"      , LVCFMT_RIGHT,  64, 3 );
    m_List.InsertColumn( 4, "Type"      , LVCFMT_RIGHT,  64, 4 );
    m_List.InsertColumn( 5, "Format"    , LVCFMT_RIGHT, 128, 5 );

    s_pFileList = this;
    s_pFileListThread = new xthread( FileListThread, "FileList", 65536, THREAD_BASE_PRIORITY );

    return 0;
}

void CFileList::OnSize(UINT nType, int cx, int cy) 
{
    CWnd::OnSize(nType, cx, cy);
    
    // TODO: Add your message handler code here
    if( m_List.GetSafeHwnd() )
    {
        CRect rc;
        GetClientRect( &rc );
        m_List.MoveWindow( rc );
    }
}

LRESULT CFileList::OnDirChanged( WPARAM wParam, LPARAM lParam )
{
    const char* pPath = (const char*)wParam;
    m_Path = pPath;
    m_Path += "\\";

    // Wake the thread that reads the directory
    s_MessageQueue.Send( (void*)1, MQ_BLOCK );

    return 0;
}

LRESULT CFileList::OnPopulateList( WPARAM wParam, LPARAM lParam )
{
    s32     i;
    s32     iSelect = -1;

    m_List.SetRedraw( FALSE );

    EnterCriticalSection( &s_CriticalSection );

    // Clear the list
    m_List.DeleteAllItems();

    // Add all new files to the list
    for( i=0 ; i<s_Files.GetCount() ; i++ )
    {
        file_rec* pFile = s_Files[i];
        pFile->Lock();
        pFile->InfoDisplayed = FALSE;
        m_List.InsertItem( LVIF_TEXT|LVIF_PARAM, i, pFile->Name, 0, 0, 0, (DWORD)pFile );
        xstring SizeString = xfs("%d", pFile->Size/1024);
        xstring SizeString2;
        s32 c = SizeString.GetLength();
        for( s32 j=0 ; j<SizeString.GetLength() ; j++ )
        {
            SizeString2 += SizeString[j];
            c--;
            if( ((c % 3) == 0) && (c>0) )
                SizeString2 += ',';
        }
        SizeString2 += " KB";
        m_List.SetItemText( i, 1, SizeString2 );

        if( ( 0 == g_CmdLineFile.CompareNoCase( CString(pFile->Name) ) ) && ( 0 == g_CmdLinePath.CompareNoCase( m_Path ) ) )
        {
            iSelect = i;
        }
    }

    LeaveCriticalSection( &s_CriticalSection );

    // If an item should be selected select it
    if( iSelect != -1 )
    {
        m_List.SetItemState( iSelect, LVIS_SELECTED, LVIS_SELECTED );

        file_rec* pFile = (file_rec*)m_List.GetItemData( iSelect );
        ASSERT( pFile );

        CWnd* pFrame = AfxGetMainWnd();
        CString Path = m_Path;
        Path += pFile->Name;
        pFrame->SendMessage( NM_NEWBITMAP, (WPARAM)(const char*)Path, (LPARAM)pFile );
    }

    m_List.SortItems( fnCompare, (LPARAM)&m_List );

    // If an item was selected ensure it is visible
    if( iSelect )
    {
        POSITION Pos = m_List.GetFirstSelectedItemPosition();
        if( Pos )
        {
            m_List.EnsureVisible( m_List.GetNextSelectedItem(Pos), FALSE );
        }
    }

    m_List.SetRedraw( TRUE );

    return 0;
}

LRESULT CFileList::OnRefreshList( WPARAM wParam, LPARAM lParam )
{
    int     nFiles = 0;
    double  nBytes  = 0.0;
    int     nFilesSelected = 0;
    double  nBytesSelected = 0.0;

    for( s32 i=0 ; i<m_List.GetItemCount() ; i++ )
    {
        file_rec* pFile = (file_rec*)m_List.GetItemData( i );
        ASSERT( pFile );
        if( pFile->GotInfo && !pFile->InfoDisplayed )
        {
            // TODO: Add all the info here
            char Drive[X_MAX_DRIVE];
            char Dir[X_MAX_DIR];
            char FName[X_MAX_FNAME];
            char Ext[X_MAX_EXT];
            x_splitpath( pFile->Name, Drive, Dir, FName, Ext );
            strlwr( Ext );
            m_List.SetItemText( i, 2, xfs("%dx%dx%db", pFile->Width, pFile->Height, pFile->BitDepth) );
            m_List.SetItemText( i, 3, xfs("%d", pFile->nMips) );
            m_List.SetItemText( i, 4, &Ext[1] );
            m_List.SetItemText( i, 5, (const char*)pFile->Format );
            pFile->InfoDisplayed = TRUE;

            BOOL Selected = m_List.GetItemState( i, LVIS_SELECTED );
            if( Selected )
            {
                nFilesSelected++;
                nBytesSelected += pFile->Size;
            }

            nBytes += pFile->Size;
        }

        nFiles++;
    }

    // Generate total string
    CString Size;
    if( nBytes > (1024*1024) )
        Size.Format( _T("(%.1f MB)"), nBytes/(1024*1024) );
    else
        Size.Format( _T("(%.1f KB)"), nBytes/1024 );
    CString s;
    s.Format( _T("   Total %d %s %s   "), nFiles, (nFiles==1)?_T("file"):_T("files"), Size );
    g_pMainFrame->SetStatusPane( INDICATOR_TOTAL, s );

    // Generate selected string
    if( nBytesSelected > (1024*1024) )
        Size.Format( _T("(%.1f MB)"), nBytesSelected/(1024*1024) );
    else
        Size.Format( _T("(%.1f KB)"), nBytesSelected/1024 );
    s.Format( _T("   Selected %d %s %s   "), nFilesSelected, (nFilesSelected==1)?_T("file"):_T("files"), Size );
    g_pMainFrame->SetStatusPane( INDICATOR_SELECTED, s );

    return 0;
}

void CFileList::OnDestroy() 
{
    CWnd::OnDestroy();

    // Kill the thread
    delete s_pFileListThread;
}

void CFileList::OnItemChanged( NMHDR* pHeader, LRESULT* pResult )
{
    NMLISTVIEW* pNM = (NMLISTVIEW*)pHeader;
    if( (pNM->iItem != -1) && (pNM->uNewState & LVIS_FOCUSED) )
    {
        file_rec* pFile = (file_rec*)m_List.GetItemData( pNM->iItem );
        ASSERT( pFile );

        CWnd* pFrame = AfxGetMainWnd();
        CString Path = m_Path;
        Path += pFile->Name;
        g_CmdLinePath = m_Path;
        g_CmdLineFile = pFile->Name;
        pFrame->SendMessage( NM_NEWBITMAP, (WPARAM)(const char*)Path, (LPARAM)pFile );
    }

    int     nFilesSelected = 0;
    double  nBytesSelected = 0.0;

    for( int i=0 ; i<m_List.GetItemCount() ; i++ )
    {
        file_rec* pFile = (file_rec*)m_List.GetItemData( i );
        ASSERT( pFile );
        if( pFile->GotInfo )
        {
            BOOL Selected = m_List.GetItemState( i, LVIS_SELECTED );
            if( Selected )
            {
                nFilesSelected++;
                nBytesSelected += pFile->Size;
            }
        }
    }

    // Generate selected string
    CString Size;
    CString s;
    if( nBytesSelected > (1024*1024) )
        Size.Format( _T("(%.1f MB)"), nBytesSelected/(1024*1024) );
    else
        Size.Format( _T("(%.1f KB)"), nBytesSelected/1024 );
    s.Format( _T("   Selected %d %s %s   "), nFilesSelected, (nFilesSelected==1)?_T("file"):_T("files"), Size );
    g_pMainFrame->SetStatusPane( INDICATOR_SELECTED, s );
}

void CFileList::OnColumnClick( NMHDR* pHeader, LRESULT* pResult )
{
    NMLISTVIEW* pNM = (NMLISTVIEW*)pHeader;

    if( m_List.GetSortColumn() != 0 )
    {
        m_List.SortItems( fnCompare, (LPARAM)&m_List );
        m_List.ColorSortColumn( TRUE, m_List.GetSortColumn() );
    }
}

void CFileList::OnDeleteAllItems( NMHDR* pHeader, LRESULT* pResult )
{
    NMLISTVIEW* pNM = (NMLISTVIEW*)pHeader;

    // Unlock all the items
    for( s32 i=0 ; i<m_List.GetItemCount() ; i++ )
    {
        file_rec* pFile = (file_rec*)m_List.GetItemData( i );
        ASSERT( pFile );
        pFile->Unlock();
    }
}

void CFileList::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized) 
{
    CWnd::OnActivate(nState, pWndOther, bMinimized);
    
    // TODO: Add your message handler code here
    m_List.SendMessage( WM_ACTIVATE, WA_ACTIVE );
}

void CFileList::OnRButtonDown(UINT nFlags, CPoint point)
{
    UINT uFlags;
    int nHitItem = m_List.HitTest(point, &uFlags);

    if (nHitItem != -1 && (uFlags & LVHT_ONITEM))
    {
        if (!(m_List.GetItemState(nHitItem, LVIS_SELECTED) & LVIS_SELECTED))
        {
            for (int i = 0; i < m_List.GetItemCount(); i++)
            {
                if (m_List.GetItemState(i, LVIS_SELECTED) & LVIS_SELECTED)
                {
                    m_List.SetItemState(i, 0, LVIS_SELECTED);
                }
            }

            m_List.SetItemState(nHitItem, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

            file_rec* pFile = (file_rec*)m_List.GetItemData(nHitItem);
            ASSERT(pFile);
            
            CWnd* pFrame = AfxGetMainWnd();
            CString Path = m_Path;
            Path += pFile->Name;
            g_CmdLinePath = m_Path;
            g_CmdLineFile = pFile->Name;
            pFrame->SendMessage(NM_NEWBITMAP, (WPARAM)(const char*)Path, (LPARAM)pFile);
        }
    }
    ClientToScreen(&point);
    OnContextMenu(this, point);
}

void CFileList::OnContextMenu(CWnd* pWnd, CPoint pos)
{
    if (pWnd != this && pWnd != &m_List)
        return;

    if (pos.x == -1 && pos.y == -1)
    {
        CRect rect;
        GetClientRect(rect);
        ClientToScreen(rect);

        pos = rect.TopLeft();
        pos.Offset(rect.Width() / 2, rect.Height() / 2);
    }
    else
    {
        CPoint clientPos = pos;
        m_List.ScreenToClient(&clientPos);
        
        UINT uFlags;
        int nHitItem = m_List.HitTest(clientPos, &uFlags);
        
        if (nHitItem == -1 || !(uFlags & LVHT_ONITEM))
            return;
    }

    CMenu menu;
    if (menu.LoadMenu(IDR_CONTEXT_MENU))
    {
        CMenu* pSubMenu = menu.GetSubMenu(0);
        if (pSubMenu)
        {
            UINT enableFlag = (m_List.GetSelectedCount() > 0) ? MF_ENABLED : MF_GRAYED;
            pSubMenu->EnableMenuItem(ID_CONTEXT_CONVERT_TGA, enableFlag);
            pSubMenu->EnableMenuItem(ID_CONTEXT_CONVERT_XBMP, enableFlag);
            
            UINT nCmd = pSubMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_RETURNCMD, pos.x, pos.y, this);

            if (nCmd != 0)
            {
                SendMessage(WM_COMMAND, nCmd);
            }
        }
    }
}

void CFileList::OnContextConvertTga()
{
    CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
    if (pMainFrame)
    {
        pMainFrame->OnConvertTga();
    }
}

void CFileList::OnContextConvertXbmp()
{
    CMainFrame* pMainFrame = (CMainFrame*)AfxGetMainWnd();
    if (pMainFrame)
    {
        pMainFrame->OnConvertXbmp();
    }
}