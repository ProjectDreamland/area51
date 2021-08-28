// LogView.cpp : implementation file
//

#include "BaseStdAfx.h"
#include "resource.h"
#include "LogView.h"
#include "MiscUtils\Guid.hpp"
#include "..\WorldEditor\WorldEditor.hpp"
#include "..\WorldEditor\EditorPaletteDoc.h"
#include "..\WorldEditor\EditorFrame.h"
#include "..\WorldEditor\EditorView.h"
#include "..\WorldEditor\EditorDoc.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLogView

IMPLEMENT_DYNCREATE(CLogView, CXTListCtrl)

CLogView::CLogView()
{
}

CLogView::~CLogView()
{
}


BEGIN_MESSAGE_MAP(CLogView, CXTListCtrl)
	//{{AFX_MSG_MAP(CLogView)
    ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//     ON_NOTIFY(LVN_ITEMCHANGED, 0, OnItemChanged)

/////////////////////////////////////////////////////////////////////////////
// CLogView drawing

void CLogView::OnDraw(CDC* pDC)
{
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CLogView diagnostics

#ifdef _DEBUG
void CLogView::AssertValid() const
{
	CXTListCtrl::AssertValid();
}

void CLogView::Dump(CDumpContext& dc) const
{
	CXTListCtrl::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CLogView message handlers


static col_data columns[] =
{
	{ _T("Squence"),           50,  LVCFMT_RIGHT, DT_INT      },
	{ _T("Channel"),           100, LVCFMT_LEFT,  DT_STRING   },
    { _T("Type"),              100, LVCFMT_LEFT,  DT_STRING   },
	{ _T("Message"),           800, LVCFMT_LEFT,  DT_STRING   },
	{ _T("File"),              100, LVCFMT_LEFT,  DT_STRING   },
	{ _T("Line"),               50, LVCFMT_RIGHT, DT_INT      },
};

//=========================================================================
static CLogView* s_pListCtrl = NULL;
static s32       s_Index     = 0;
extern xbool     g_bAutoBuild;

void Log_Display( const char* pChannel, log_type Type, const char* pMsg, 
                  const char* pFileName, s32 LineNumber )
{
    // Build string for logtype   
    const char* pType;
    switch( Type )
    {
        case LOG_TYPE_MESSAGE:  pType = "TYPE_MSG";      break;
        case LOG_TYPE_WARNING:  pType = "TYPE_WARNING";  break;
        case LOG_TYPE_ERROR:    pType = "TYPE_ERROR";    break;
        case LOG_TYPE_ASSERT:   pType = "TYPE_ASSERT";   break;
        default:                pType = "TYPE_UNKNOWN";  break;
    }
    
    if( g_bAutoBuild )
    {
        CString s;
        s.Format( "@%s@%s@%s", pType, pChannel, pMsg );
        fprintf( stdout, (const char*)s );
        fflush( stdout );
    }
    else
    if( s_pListCtrl )
    {    
        s32 Index;
        Index = s_pListCtrl->InsertItem( 0, xfs("%d",s_Index++) );
        s_pListCtrl->SetItem   ( Index, 1, LVIF_TEXT, pChannel, 0, 0, 0, 0);
        s_pListCtrl->SetItem   ( Index, 2, LVIF_TEXT, pType, 0, 0, 0, 0);
        s_pListCtrl->SetItem   ( Index, 3, LVIF_TEXT, pMsg, 0, 0, 0, 0);
        s_pListCtrl->SetItem   ( Index, 4, LVIF_TEXT, pFileName, 0, 0, 0, 0);
        s_pListCtrl->SetItem   ( Index, 5, LVIF_TEXT, xfs("%d",LineNumber), 0, 0, 0, 0);
    }
}

//=========================================================================

void CLogView::OnInitialUpdate() 
{
//	CXTListCtrl::OnInitialUpdate();
	
    InsertColumns( columns, 6 );

	SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT | LVS_NOSCROLL );
	SubclassHeader();
	GetFlatHeaderCtrl()->ShowSortArrow(TRUE);
    SetSortImage(0, false);	

    //
    // Register the call back
    //
    s_pListCtrl = this;
#ifndef X_RETAIL
    log_RegisterCallBack( Log_Display );
#endif
}

//=========================================================================

BOOL CLogView::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= LVS_EX_GRIDLINES | LVS_REPORT;
	return CXTListCtrl::PreCreateWindow(cs);
}

//=========================================================================

bool CLogView::SortList(
	// passed in from control, index of column clicked.
	int nCol,
	// passed in from control, true if sort order should 
	// be ascending.
	bool bAscending )
{
	CXTSortClass csc (this, nCol);
	csc.Sort(bAscending, columns[nCol].type);
	return true;
}

//=========================================================================

void CLogView::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
    if( lHint )
    {
        //UpdateAll();
    }
}

//=========================================================================

void CLogView::InsertColumns( col_data* pColumns, s32 ColCount )
{
	// TODO: Add your specialized code here and/or call the base class

    //
    // Add the column names
    //
	s32 i;
	for (i = 0; i < ColCount; ++i)
	{
		InsertColumn(i, pColumns[i].name, pColumns[i].fmt, 
			pColumns[i].width);
	}
}

//=========================================================================

void CLogView::OnDoubleClick( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLISTVIEW* lphdi = (NMLISTVIEW*)pNMHDR;

    if( lphdi && lphdi->iItem >= 0 )
    {
        x_try;

        CString Text = GetItemText( lphdi->iItem, 3 );
        
        // Search for an object GUID in the string
        bool Match = false;
        s32 i=0;
        s32 j=0;
       
        for( i=0 ; i<=(Text.GetLength()-17); i++ )
        {
            Match = true;

            for( j=0 ; j<17 ; j++ )
            {
                if( !((j == 8) && (Text[i+j] == ':')) &&
                    !(x_ishex(Text[i+j])) )
                {
                    Match = false;
                    break;
                }
            }

           
            if( Match )
                break;
            
        }

        // Select the object matching the guid
        if( Match )
        {
            guid GuidToFind = guid_FromString( Text.Mid( i, 17 ) );

            if( g_WorldEditor.SelectObject(GuidToFind) )
            {
                CEditorDoc* pEditorDoc = NULL;
                POSITION posEditor = CMainFrame::s_pMainFrame->m_pWorldEditDocTemplate->GetFirstDocPosition( );
                if (posEditor)
                {
                    pEditorDoc = (CEditorDoc*)CMainFrame::s_pMainFrame->m_pWorldEditDocTemplate->GetNextDoc( posEditor );
                    if( pEditorDoc && pEditorDoc->IsKindOf( RUNTIME_CLASS( CEditorDoc ) ) )
                    {
                        pEditorDoc->GetView()->FocusCameraWithUndoAndNoRotation( g_WorldEditor.GetMinPositionForSelected() );
                    }
                }
            }
            else
            {
                ::AfxMessageBox("Guid Not Found.");
            }
        }

        x_catch_display;
    }
}

//=========================================================================

void CLogView::OnItemChanged( NMHDR* pNMHDR, LRESULT* pResult )
{
    /*
	NMLISTVIEW* lphdi = (NMLISTVIEW*)pNMHDR;

    if( lphdi->uNewState == 3 )
    {
        x_try;

        CListCtrl* pList = &GetListCtrl();

        char Name[256];
        pList->GetItemText( lphdi->iItem, 0, Name, 256 );
        
        char Ext[256];
        pList->GetItemText( lphdi->iItem, 1, Ext, 256 );

        GetDocument()->SetActiveDesc( xfs("%s.%s",Name,Ext) );

        x_catch_display;
    }
    */
}
