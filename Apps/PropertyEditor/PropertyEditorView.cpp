// PropertyEditorView.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "PropertyEditorView.h"
#include "GridTreeItem.h"
#include "Auxiliary\MiscUtils\property.hpp"
#include "PropertyEditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define		IDC_PROP_GRID_ITEM		0x2130
#define		IDC_STATIC_TITLE_ITEM	0x2131
#define		IDC_STATIC_HELP_ITEM	0x2132
#define		IDC_COL_SLIDER_ITEM		0x2133
#define		IDC_DEBUG_REFRESH_BTN	0x2134

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorView

IMPLEMENT_DYNCREATE(CPropertyEditorView, CView)

CPropertyEditorView::CPropertyEditorView()
{
	VERIFY(m_fntTitle.CreateFont(
	   15,                        // nHeight
	   0,                         // nWidth
	   0,                         // nEscapement
	   0,                         // nOrientation
	   FW_BOLD,					  // nWeight
	   FALSE,                     // bItalic
	   FALSE,                     // bUnderline
	   0,                         // cStrikeOut
	   ANSI_CHARSET,              // nCharSet
	   OUT_DEFAULT_PRECIS,        // nOutPrecision
	   CLIP_DEFAULT_PRECIS,       // nClipPrecision
	   DEFAULT_QUALITY,           // nQuality
	   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
	   "Arial"));                 // lpszFacename

	VERIFY(m_fntText.CreateFont(
	   12,                        // nHeight
	   0,                         // nWidth
	   0,                         // nEscapement
	   0,                         // nOrientation
	   FW_NORMAL,                 // nWeight
	   FALSE,                     // bItalic
	   FALSE,                     // bUnderline
	   0,                         // cStrikeOut
	   ANSI_CHARSET,              // nCharSet
	   OUT_DEFAULT_PRECIS,        // nOutPrecision
	   CLIP_DEFAULT_PRECIS,       // nClipPrecision
	   DEFAULT_QUALITY,           // nQuality
	   DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
	   "Arial"));                 // lpszFacename

    m_RedrawDisableCount = 0;
}

CPropertyEditorView::~CPropertyEditorView()
{
	m_fntTitle.DeleteObject();
	m_fntText.DeleteObject();
}


BEGIN_MESSAGE_MAP(CPropertyEditorView, CView)
	//{{AFX_MSG_MAP(CPropertyEditorView)
	ON_WM_CREATE()
	ON_WM_SIZE()
    ON_BN_CLICKED(IDC_DEBUG_REFRESH_BTN, OnRefreshClick )
	ON_MESSAGE( WM_USER_MSG_SLIDER_MOVED, OnSliderMove )
	ON_MESSAGE( WM_USER_MSG_GRID_ITEM_CHANGE, OnGridItemChange )
	ON_MESSAGE( WM_USER_MSG_GUID_SELECT_REQUEST, OnGuidSelect )
    ON_MESSAGE( WM_USER_MSG_GRID_SELECTION_CHANGE, OnSelectionChange )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPropertyEditorView::OnRefreshClick()
{
    GetDocument()->Refresh();
}

LRESULT CPropertyEditorView::OnSliderMove(WPARAM wParam, LPARAM lParam)
{
	m_lcPropertyGrid.SetFirstColWidth(wParam);
	return 1;
}

LRESULT CPropertyEditorView::OnGridItemChange(WPARAM wParam, LPARAM lParam)
{
	CGridTreeItem* lpItem = (CGridTreeItem*)wParam;
    if (lpItem)
    {
	    TRACE(xfs("CPropertyEditorView::OnGridItemChange::SAVE PROPERTY %s\n",
            (const char*)lpItem->m_strIdentifier));
        GetDocument()->SaveProperty(lpItem, lpItem->m_bMustReloadData);
    }
    else 
    {
        ASSERT(FALSE);
        return 0;
    }

	return 1;
}

LRESULT CPropertyEditorView::OnGuidSelect(WPARAM wParam, LPARAM lParam)
{
	CGridTreeItem* lpItem = (CGridTreeItem*)wParam;
    if (lpItem)
    {
	    TRACE(xfs("CPropertyEditorView::OnGuidSelect::PROPERTY %s\n",
            (const char*)lpItem->m_strIdentifier));
        GetDocument()->GuidSelect(lpItem);
    }
    else 
    {
        ASSERT(FALSE);
        return 0;
    }

	return 1;
}

LRESULT CPropertyEditorView::OnSelectionChange(WPARAM wParam, LPARAM lParam)
{
	CGridTreeItem* lpItem = (CGridTreeItem*)wParam;
    if (lpItem)
    {
        CString strComment = lpItem->m_strComment;
        CString strCommentTitle = lpItem->m_strIdentifier;

        if (strCommentTitle.IsEmpty()) 
        {
            strComment = "Death. Taxes. Games.";
            strCommentTitle = "INEVITABLE";
        }

        AddComment(strCommentTitle,strComment);

        //send selection change message
        if (GetDocument()->m_pCommandHandler)
        {
            char* pszId = strCommentTitle.GetBuffer(strCommentTitle.GetLength());
            GetDocument()->m_pCommandHandler->SendMessage(WM_USER_MSG_SELECTION_CHANGE,(long)pszId);
            strCommentTitle.ReleaseBuffer();
        }

        CGridItemInfo::CONTROLTYPE ctrlType;
        BOOL bGuidSent = FALSE;
        if ((GetDocument()->m_pCommandHandler) && 
            (lpItem->m_lpNodeInfo->GetControlType(0,ctrlType)))
        {
            if ( ctrlType == CGridItemInfo::GCT_GUID_EDIT)
            {
                CString strValue = lpItem->m_lpNodeInfo->GetSubItem(0);
                char* pszIdentifier = strValue.GetBuffer(strValue.GetLength());
                GetDocument()->m_pCommandHandler->SendMessage(WM_USER_MSG_GUID_HIGHLIGHT_REQUEST, (WPARAM)pszIdentifier);
                bGuidSent = TRUE;
            }
        }

        if (!bGuidSent)
        {
            if (GetDocument()->m_pCommandHandler)
            {
                GetDocument()->m_pCommandHandler->SendMessage(WM_USER_MSG_GUID_HIGHLIGHT_REQUEST, 0);
            }
        }
    }
    else 
    {
        ASSERT(FALSE);
        return 0;
    }

    return 1;
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorView drawing

void CPropertyEditorView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorView diagnostics

#ifdef _DEBUG
void CPropertyEditorView::AssertValid() const
{
	CView::AssertValid();
}

void CPropertyEditorView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditorView message handlers

int CPropertyEditorView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

    m_lcPropertyGrid.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP, CRect(0,0,0,0),this,IDC_PROP_GRID_ITEM);	
    m_lcPropertyGrid.InitializeGrid();
    m_lcPropertyGrid.ModifyStyleEx(0,WS_EX_CLIENTEDGE);
    m_lcPropertyGrid.RecalcSize();

    m_stHelpTitle.Create("INEVITABLE",WS_CHILD | WS_VISIBLE, CRect(0,0,0,0),this,IDC_STATIC_TITLE_ITEM);
    m_stHelpTitle.SetFont(&m_fntTitle);
    m_stHelp.Create("Death. Taxes. Games.",WS_CHILD | WS_VISIBLE, CRect(0,0,0,0),this,IDC_STATIC_HELP_ITEM);
    m_stHelp.SetFont(&m_fntText);

    m_btnRefresh.Create("Force Refresh", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON, 
        CRect(0,0,0,0),this,IDC_DEBUG_REFRESH_BTN);
    m_btnRefresh.SetFont(&m_fntText);

	return 0;
}

void CPropertyEditorView::OnInitialUpdate() 
{
    CView::OnInitialUpdate();
}	

void CPropertyEditorView::OnSize(UINT nType, int cx, int cy) 
{
	CView::OnSize(nType, cx, cy);

    int nRefreshSize = 16;

	if (m_lcPropertyGrid.GetSafeHwnd())	
    {
        m_lcPropertyGrid.SetRedraw(FALSE);
		m_lcPropertyGrid.MoveWindow(0,0,cx,cy-64-nRefreshSize);
        m_lcPropertyGrid.SetRedraw(TRUE);
		m_lcPropertyGrid.RecalcSize();
	}
	
	if (m_btnRefresh.GetSafeHwnd())	    m_btnRefresh.MoveWindow(0,cy - 64-nRefreshSize,cx,nRefreshSize);
	if (m_stHelpTitle.GetSafeHwnd())	m_stHelpTitle.MoveWindow(0,cy - 64,cx,16);
	if (m_stHelp.GetSafeHwnd())			m_stHelp.MoveWindow(0,cy - 48,cx,48);
}

void CPropertyEditorView::EnableRedraw( void )
{
    if( m_RedrawDisableCount > 0 )
    {
        if( --m_RedrawDisableCount == 0 )
            m_lcPropertyGrid.SetRedraw( TRUE );
    }
}

void CPropertyEditorView::DisableRedraw( void )
{
    if( m_RedrawDisableCount++ == 0 )
        m_lcPropertyGrid.SetRedraw( FALSE );
}

void CPropertyEditorView::ClearGrid()
{
//    if( IsWindow( m_stHelpTitle.GetSafeHwnd() ) )
	    m_stHelpTitle.SetWindowText("INEVITABLE");
//    if( IsWindow( m_stHelp.GetSafeHwnd() ) )
	    m_stHelp.SetWindowText("Death. Taxes. Games.");

//    if( IsWindow( m_lcPropertyGrid.GetSafeHwnd() ) )
    {
        DisableRedraw();
	    m_lcPropertyGrid.DeleteAll();
        EnableRedraw();
        m_lcPropertyGrid.Invalidate();
    }
}

void CPropertyEditorView::AddComment(CString strTitle, CString strText)
{
	m_stHelpTitle.SetWindowText(strTitle);
	m_stHelp.SetWindowText(strText);
}

void CPropertyEditorView::ExpandRoots()
{
	POSITION pos = m_lcPropertyGrid.GetRootHeadPosition();
	while(pos != NULL)
	{
		CGridTreeItem *pItem = (CGridTreeItem*)m_lcPropertyGrid.GetNextRoot(pos); 
        if (pItem && m_lcPropertyGrid.IsCollapsed(pItem))
        {
            m_lcPropertyGrid.Expand(pItem,pItem->m_nIndex);
        }
    }
}

BOOL CPropertyEditorView::AddGridDataElement(CString strName, CString strValue, CString strComment,
											 CGridItemInfo::CONTROLTYPE type, 
                                             CStringList &list, int iXaIndex,
											 BOOL bReadOnly, BOOL bMustEnum, BOOL bHeader,
                                             CGridTreeItem* &pParentTreeItem)
{
    BOOL bReturn = TRUE;

	CGridItemInfo* lp = new CGridItemInfo();
	lp->SetItemText(strName);
	lp->AddSubItemText(strValue);// 1
	lp->SetReadOnly(bReadOnly);
	lp->SetControlType(type,0); //first sub column
	lp->SetListData(0, &list);
    lp->SetIsHeader(bHeader);
    lp->SetDataIndex(iXaIndex);

    CGridTreeItem *pTreeItemInserted = NULL;

    if (strName.Find('\\') == -1)
    {
        //Must be a header
		pTreeItemInserted = m_lcPropertyGrid.InsertRootItem(lp);

        if (type == PROP_TYPE_VECTOR3 ||
            type == PROP_TYPE_ROTATION ||
            type == PROP_TYPE_BBOX ||
            type == PROP_TYPE_COLOR)
        {
            //illegal header type
            return FALSE;
        }
    }
    else
    {
        int iIndex = strName.ReverseFind('\\');
        if (iIndex == -1) 
        {
            //nothing inserted
            delete lp;
            bReturn = FALSE;
        }
        else
        {
            CString strShortName = strName.Right(strName.GetLength()-(iIndex+1));
            CString strParentName = strName.Left(iIndex);

            pParentTreeItem = m_lcPropertyGrid.FastFindTreeItemWithIdentifier(strParentName, pParentTreeItem);
            
            if (pParentTreeItem)
            {
		        if (pParentTreeItem->m_lpNodeInfo && 
                    pParentTreeItem->m_lpNodeInfo->IsReadOnly())
		        {
			        //roll down the read only flag
			        lp->SetReadOnly(TRUE);
		        }
       	        lp->SetItemText(strShortName);

		        pTreeItemInserted = m_lcPropertyGrid.InsertItem(pParentTreeItem, lp, FALSE /*expand*/);
            }
            else
            {
                //nothing inserted
                delete lp;
                bReturn = FALSE;
                x_throw( xfs("Error: Unable to find parent for property [%s]", (const char*)strName) );
            }
        }
    }

    if (!bReturn || pTreeItemInserted==NULL)
    {
        //invalid format in object, can not edit!
        m_lcPropertyGrid.DeleteAll();
    }
    else
    {
        pTreeItemInserted->m_strIdentifier = strName;
        pTreeItemInserted->m_strComment = strComment;
        pTreeItemInserted->m_bMustReloadData = bMustEnum;
    }

	return bReturn;
}

bool CPropertyEditorView::IsValidItem( CGridTreeItem* pItem )
{
    return m_lcPropertyGrid.IsValidItem( pItem );
}

void CPropertyEditorView::UpdateItem(CGridTreeItem* lpItem)
{
    m_lcPropertyGrid.OnUpdateItem(lpItem);
}

void CPropertyEditorView::SaveStructure()
{
    m_lcPropertyGrid.RecalcSize();
    m_lcPropertyGrid.SaveStructure();
}

void CPropertyEditorView::RestoreSavedSelection()
{
    m_lcPropertyGrid.RestoreSavedSelection();
}

void CPropertyEditorView::SetGridBackgroundColor(COLORREF cr)
{ 
    m_lcPropertyGrid.SetBackgroundColor(cr); 
}


BOOL CPropertyEditorView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Add your specialized code here and/or call the base class
    cs.style |= WS_CLIPCHILDREN;

    return CView::PreCreateWindow(cs);
}

void CPropertyEditorView::SaveColumnState( LPCTSTR lpszProfileName )
{
    m_lcPropertyGrid.SaveColumnState( lpszProfileName );
}

void CPropertyEditorView::LoadColumnState( LPCTSTR lpszProfileName )
{
    m_lcPropertyGrid.LoadColumnState( lpszProfileName );
}
