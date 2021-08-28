// HAISView.cpp : implementation of the CHAISView class
//

#include "stdafx.h"
#include "HAIS.h"

#include "HAISDoc.h"
#include "HAISView.h"
#include ".\haisview.h"

#include <dbghelp.h>

#pragma comment( lib, "dbghelp.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHAISView

IMPLEMENT_DYNCREATE(CHAISView, CView)

BEGIN_MESSAGE_MAP(CHAISView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
    ON_COMMAND(ID_OBJECT_SELECT, OnObjectSelect)
    ON_UPDATE_COMMAND_UI(ID_OBJECT_SELECT, OnUpdateObjectSelect)
    ON_COMMAND(ID_OBJECT_COVER, OnObjectCover)
    ON_UPDATE_COMMAND_UI(ID_OBJECT_COVER, OnUpdateObjectCover)
    ON_COMMAND(ID_OBJECT_RED, OnObjectRed)
    ON_UPDATE_COMMAND_UI(ID_OBJECT_RED, OnUpdateObjectRed)
    ON_COMMAND(ID_OBJECT_BLUE, OnObjectBlue)
    ON_UPDATE_COMMAND_UI(ID_OBJECT_BLUE, OnUpdateObjectBlue)
    ON_WM_SETCURSOR()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

// CHAISView construction/destruction

PVOID g_pHandler = NULL;

LONG WINAPI Handler( PEXCEPTION_POINTERS pExceptionInfo )
{
    if( pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT )
    {
        char Buffer[1024];
        sprintf( Buffer, "0x%08x - 0x%08x", pExceptionInfo->ExceptionRecord->ExceptionCode, (DWORD)pExceptionInfo->ExceptionRecord->ExceptionAddress );
        ::MessageBox( NULL, Buffer, "Breakpoint", MB_OK );

        {
            // Create the dump file name
            CString sFile;
            sFile.Format( _T("C:\\HAIS.dmp") );

            // Create the file and write the minidump to the file
            HANDLE hFile = CreateFile( sFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
            if( hFile )
            {
                MINIDUMP_EXCEPTION_INFORMATION eInfo;
                eInfo.ThreadId = GetCurrentThreadId();
                eInfo.ExceptionPointers = pExceptionInfo;
                eInfo.ClientPointers = FALSE;

                //MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
                //cbMiniDump.CallbackRoutine = CExceptionReport::miniDumpCallback;
                //cbMiniDump.CallbackParam = 0;

                MiniDumpWriteDump(
                    GetCurrentProcess(),
                    GetCurrentProcessId(),
                    hFile,
                    MiniDumpWithDataSegs,
                    &eInfo,
                    NULL,
                    NULL ); //&cbMiniDump);
            }

            // Close file
            CloseHandle(hFile);
        }

        pExceptionInfo->ContextRecord->Eip += 1;
        return EXCEPTION_CONTINUE_EXECUTION;
    }
    else if( (pExceptionInfo->ExceptionRecord->ExceptionCode & 0xffff0000) != 0x40010000 )
    {
        char Buffer[1024];
        sprintf( Buffer, "0x%08x - 0x%08x", pExceptionInfo->ExceptionRecord->ExceptionCode, (DWORD)pExceptionInfo->ExceptionRecord->ExceptionAddress );
        ::MessageBox( NULL, Buffer, "Exception", MB_OK );
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

CHAISView::CHAISView()
{
	// TODO: add construction code here
    m_Mode = MODE_SELECT;

//    if( !IsDebuggerPresent() )
        g_pHandler = AddVectoredExceptionHandler( 1, Handler );
}

CHAISView::~CHAISView()
{
    RemoveVectoredExceptionHandler( g_pHandler );
}

BOOL CHAISView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CHAISView drawing

void CHAISView::OnDraw(CDC* /*pDC*/)
{
	CHAISDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: add draw code for native data here
}


// CHAISView printing

BOOL CHAISView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CHAISView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing
}

void CHAISView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CHAISView diagnostics

#ifdef _DEBUG
void CHAISView::AssertValid() const
{
	CView::AssertValid();
}

void CHAISView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CHAISDoc* CHAISView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CHAISDoc)));
	return (CHAISDoc*)m_pDocument;
}
#endif //_DEBUG


// CHAISView message handlers

void CHAISView::OnObjectSelect()
{
    m_Mode = MODE_SELECT;
}

void CHAISView::OnUpdateObjectSelect(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck( (m_Mode == MODE_SELECT) ? 1 : 0 );
}

void CHAISView::OnObjectCover()
{
    m_Mode = MODE_ADD_COVER;

    *(int*)0 = 0;
}

void CHAISView::OnUpdateObjectCover(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck( (m_Mode == MODE_ADD_COVER) ? 1 : 0 );
}

void CHAISView::OnObjectRed()
{
    m_Mode = MODE_ADD_RED;

    DebugBreak();
//    RaiseException( EXCEPTION_BREAKPOINT, 0, 0, NULL );

    ::MessageBox( NULL, "Text", "Post Exception", MB_OK );

    OutputDebugString( "Hello!\n" );
}

void CHAISView::OnUpdateObjectRed(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck( (m_Mode == MODE_ADD_RED) ? 1 : 0 );
}

void CHAISView::OnObjectBlue()
{
    m_Mode = MODE_ADD_BLUE;
}

void CHAISView::OnUpdateObjectBlue(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck( (m_Mode == MODE_ADD_BLUE) ? 1 : 0 );
}

BOOL CHAISView::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message)
{
    char*   pCursor = NULL;

    // TODO: Add your message handler code here and/or call default
    if( nHitTest == HTCLIENT )
    {
        switch( m_Mode )
        {
        case MODE_ADD_COVER:
            pCursor = MAKEINTRESOURCE( IDC_ADD_COVER );
            break;
        case MODE_ADD_RED:
            pCursor = MAKEINTRESOURCE( IDC_ADD_RED );
            break;
        case MODE_ADD_BLUE:
            pCursor = MAKEINTRESOURCE( IDC_ADD_BLUE );
            break;
        default:
            break;
        }
    }

    if( pCursor )
    {
        SetCursor( LoadCursor( AfxGetInstanceHandle(), pCursor ) );
        return 1;
    }
    else
        return CView::OnSetCursor(pWnd, nHitTest, message);
}

void CHAISView::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CView::OnLButtonDown(nFlags, point);
}

void CHAISView::OnLButtonUp(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CView::OnLButtonUp(nFlags, point);
}

void CHAISView::OnMouseMove(UINT nFlags, CPoint point)
{
    // TODO: Add your message handler code here and/or call default

    CView::OnMouseMove(nFlags, point);
}
