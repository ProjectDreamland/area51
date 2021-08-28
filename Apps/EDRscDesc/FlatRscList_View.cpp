// FlatRscList_View.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "FlatRscList_View.h"
#include "EDRscDesc_Doc.h"

#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFlatRscList_View

IMPLEMENT_DYNCREATE(CFlatRscList_View, CXTListView)

CFlatRscList_View::CFlatRscList_View()
{
}

CFlatRscList_View::~CFlatRscList_View()
{
}


BEGIN_MESSAGE_MAP(CFlatRscList_View, CXTListView)
	//{{AFX_MSG_MAP(CFlatRscList_View)
	ON_WM_LBUTTONUP()
	ON_WM_CAPTURECHANGED()
    ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
    ON_NOTIFY_REFLECT(NM_DBLCLK, OnDoubleClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//     ON_NOTIFY(LVN_ITEMCHANGED, 0, OnItemChanged)

/////////////////////////////////////////////////////////////////////////////
// CFlatRscList_View drawing

void CFlatRscList_View::OnDraw(CDC* pDC)
{
	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CFlatRscList_View diagnostics

#ifdef _DEBUG
void CFlatRscList_View::AssertValid() const
{
	CXTListView::AssertValid();
}

void CFlatRscList_View::Dump(CDumpContext& dc) const
{
	CXTListView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CFlatRscList_View message handlers


static col_data columns[] =
{
	{ _T("Name"),              200, LVCFMT_LEFT,  DT_STRING },
    { _T("Type"),              100, LVCFMT_LEFT,  DT_STRING },
	{ _T("Attr"),               60, LVCFMT_RIGHT, DT_STRING },
	{ _T("Modified"),          120, LVCFMT_LEFT,  DT_STRING },
    { _T("Theme"),             200, LVCFMT_LEFT,  DT_STRING },

/*
	{ _T("Size"),               80, LVCFMT_RIGHT, DT_INT      },
	{ _T("Compile(T/F)"),       70, LVCFMT_RIGHT, DT_STRING   },
	{ _T("Internal Dep."),     100, LVCFMT_RIGHT, DT_STRING   },
	{ _T("External Dep."),      90, LVCFMT_LEFT,  DT_STRING   },
	{ _T("Compiler commands"), 200, LVCFMT_LEFT,  DT_STRING   }
*/
};

//=========================================================================

void CFlatRscList_View::UpdateAll( void )
{
    CListCtrl& listCtrl = GetListCtrl();

    listCtrl.SetRedraw( 0 );

    listCtrl.DeleteAllItems();

    for( s32 i=0; i<GetMgr().GetRscDescCount(); i++ )
    {
        char Name[256];
        char Ext [256];

        rsc_desc_mgr::node& Node = GetMgr().GetRscDescIndex( i );
        x_splitpath( Node.pDesc->GetName(), NULL, NULL, Name, Ext );

        listCtrl.InsertItem( i, Name);
        listCtrl.SetItem   ( i, 1, LVIF_TEXT, &Ext[1], 0, 0, 0, 0);

        char FullName[256];
        Node.pDesc->GetFullName(FullName);

        SetDetails(i, FullName, Node.Theme);
    }

    SortList( m_nSortedCol, m_bAscending );

    listCtrl.SetRedraw( 1 );
    listCtrl.RedrawWindow();
}

//=========================================================================

void CFlatRscList_View::UpdateSelected( void )
{
    CListCtrl& listCtrl = GetListCtrl();

    POSITION pos = listCtrl.GetFirstSelectedItemPosition();
    while (pos)
    {
        int i = listCtrl.GetNextSelectedItem(pos);
        xstring Name = xfs("%s.%s",(const char*)listCtrl.GetItemText( i, 0 ),(const char*)listCtrl.GetItemText( i, 1 ) );
        xstring Theme = listCtrl.GetItemText( i, 4 );
        rsc_desc& Desc = GetMgr().GetRscDescByString( Name );

        char FullName[256];
        Desc.GetFullName(FullName);

        SetDetails(i, FullName, Theme);
    }
}

//=========================================================================

void CFlatRscList_View::SetDetails( s32 i, const char* pPath, const char* pTheme )
{
    CListCtrl& listCtrl = GetListCtrl();

    xstring Attrs(" ");
    CFileStatus status;
    if( CFile::GetStatus( pPath, status ) )   // static function
    {
        if (status.m_attribute & CFile::readOnly)
        {
            Attrs = xfs("R%s", (const char*)Attrs);
        }
        listCtrl.SetItem   ( i, 2, LVIF_TEXT, Attrs, 0, 0, 0, 0);
        listCtrl.SetItem   ( i, 3, LVIF_TEXT, status.m_mtime.Format("%Y, %m/%d,%H:%M"), 0, 0, 0, 0);
        listCtrl.SetItem   ( i, 4, LVIF_TEXT, pTheme, 0, 0, 0, 0);
    }
    else
    {
        listCtrl.SetItem   ( i, 2, LVIF_TEXT, " ", 0, 0, 0, 0);
        listCtrl.SetItem   ( i, 3, LVIF_TEXT, " ", 0, 0, 0, 0);
        listCtrl.SetItem   ( i, 4, LVIF_TEXT, pTheme, 0, 0, 0, 0);
    }
}

//=========================================================================

void CFlatRscList_View::OnInitialUpdate() 
{
	CXTListView::OnInitialUpdate();
	
    InsertColumns( columns, 5 );

	SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	SubclassHeader();
	GetFlatHeaderCtrl()->ShowSortArrow(TRUE);
    SetSortImage(0, false);	

    //
    // Add all the items in the grid
    //
    UpdateAll();
}

//=========================================================================

BOOL CFlatRscList_View::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Add your specialized code here and/or call the base class
	cs.style |= LVS_EX_GRIDLINES | LVS_REPORT;
	return CXTListView::PreCreateWindow(cs);
}

//=========================================================================

bool CFlatRscList_View::SortList(
	// passed in from control, index of column clicked.
	int nCol,
	// passed in from control, true if sort order should 
	// be ascending.
	bool bAscending )
{
	CXTSortClass csc (&GetListCtrl(), nCol);
	csc.Sort(bAscending, columns[nCol].type);
	return true;
}

//=========================================================================

void CFlatRscList_View::OnUpdate( CView* pSender, LPARAM lHint, CObject* pHint )
{
    if( lHint )
    {
        UpdateAll();
    }
    else
    {
        UpdateSelected();
    }
}

//=========================================================================

void CFlatRscList_View::DeleteAllColumn( void )
{
	// TODO: Add your specialized code here and/or call the base class
    CListCtrl& listCtrl = GetListCtrl();
    
    s32 ColumnCount = listCtrl.GetHeaderCtrl()->GetItemCount();

    s32 i = 0;

    x_try;
    listCtrl.DeleteAllItems();
    for( ; i < ColumnCount; i++ )
    {
        if( !listCtrl.DeleteColumn( 0 ) )
            x_throw( xfs("Error is trying to delete Column %d", i ) );
    }
    x_catch_display;
        


    UpdateAll();   
}

//=========================================================================

void CFlatRscList_View::DeleteColumn( s32 ColNum )
{
	// TODO: Add your specialized code here and/or call the base class
    CListCtrl& listCtrl = GetListCtrl();
    
    s32 ColCount = listCtrl.GetHeaderCtrl()->GetItemCount();

    if( ColNum > ColCount || ColNum < 0 )
    {
        x_throw( "Trying to delete a Column that is out of range." );
        return;
    }

    if( listCtrl.DeleteColumn( ColNum ) )
    {
        UpdateAll();
    }
}

//=========================================================================

void CFlatRscList_View::InsertColumns( col_data* pColumns, s32 ColCount )
{
	// TODO: Add your specialized code here and/or call the base class
    CListCtrl& listCtrl = GetListCtrl();

    //
    // Add the column names
    //
	s32 i;
	for (i = 0; i < ColCount; ++i)
	{
		listCtrl.InsertColumn(i, pColumns[i].name, pColumns[i].fmt, 
			pColumns[i].width);
	}
}

//=========================================================================

void CFlatRscList_View::OnDoubleClick( NMHDR* pNMHDR, LRESULT* pResult )
{
	NMLISTVIEW* lphdi = (NMLISTVIEW*)pNMHDR;

    if( lphdi && lphdi->iItem >= 0 )
    {
        x_try;

	    CListCtrl* pList = &GetListCtrl();

        char Name[256];
        pList->GetItemText( lphdi->iItem, 0, Name, 256 );
        
        char Ext[256];
        pList->GetItemText( lphdi->iItem, 1, Ext, 256 );

        rsc_desc& Desc = g_RescDescMGR.GetRscDescByString(xfs("%s.%s",Name,Ext) );
        if( Desc.IsBeingEdited() )
            x_throw( "This resource is already been edited" );

        reg_editor* pEditor = reg_editor::FindEditorByType( Ext );
        if( pEditor )
        {
            pEditor->CreateInstance( Desc.GetName(), &Desc );
        }

        x_catch_display;
    }
}

//=========================================================================

void CFlatRscList_View::OnItemChanged( NMHDR* pNMHDR, LRESULT* pResult )
{
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
}


