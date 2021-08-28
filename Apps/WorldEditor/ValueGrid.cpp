// ValueGrid.cpp : implementation file
//

#include "stdafx.h"
#include "ValueGrid.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define EDITABLE_COL		1

/////////////////////////////////////////////////////////////////////////////
// CValueGrid

CValueGrid::CValueGrid()
{
}

CValueGrid::~CValueGrid()
{
}


BEGIN_MESSAGE_MAP(CValueGrid, CGridListCtrl)
	//{{AFX_MSG_MAP(CValueGrid)
    ON_WM_MOUSEMOVE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CValueGrid message handlers

void CValueGrid::InitializeGrid()
{
    m_BackgroundColor = RGB( 200, 210, 200);
    
	ModifyStyle(0,LVS_NOCOLUMNHEADER);
	ModifyStyleEx(0,WS_EX_CLIENTEDGE);

	LV_COLUMN   lvColumn;
	lvColumn.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvColumn.fmt = LVCFMT_LEFT;
	lvColumn.cx = 100;
	lvColumn.pszText = _T("Name");
	InsertColumn(0,&lvColumn);
	lvColumn.cx = 40;
	lvColumn.pszText = _T("Type");
	InsertColumn(1,&lvColumn);
	lvColumn.cx = 60;
	lvColumn.pszText = _T("Value");
	InsertColumn(2,&lvColumn);

	ModifyStyle(0,LVS_NOCOLUMNHEADER);
}

/////////////////////////////////////////////////////////////////////////////

//handle calling out
void CValueGrid::OnGridItemChange(CGridTreeItem *pSelItem) 
{
    if (pSelItem)
    {
        if (pSelItem->m_bGridCreated)
        {
            //machine created, hmmmm, send this message to parent
            OnGridItemChange (pSelItem->m_pParent);
        }
        else if (!GetParent()->SendMessage(WM_USER_MSG_GRID_ITEM_CHANGE,(WPARAM)pSelItem,0))
	    {
		    TRACE("CValueGrid::OnGridItemChange::ERROR SAVING VALUE\n");
	    }
    }
    else
    {
	    TRACE("CValueGrid::OnGridItemChange::INVALID POINTER\n");
    }
}

/////////////////////////////////////////////////////////////////////////////

//handle calling out
void CValueGrid::OnGuidSelect(CGridTreeItem *pSelItem) 
{
    if (pSelItem)
    {
        if (pSelItem->m_bGridCreated)
        {
            //machine created, hmmmm, send this message to parent
            OnGuidSelect (pSelItem->m_pParent);
        }
        else if (!GetParent()->SendMessage(WM_USER_MSG_GUID_SELECT_REQUEST,(WPARAM)pSelItem,0))
	    {
		    TRACE("CValueGrid::OnGuidSelect::ERROR SAVING GUID\n");
	    }
    }
    else
    {
	    TRACE("CValueGrid::OnGuidSelect::INVALID POINTER\n");
    }
}

/////////////////////////////////////////////////////////////////////////////

BOOL CValueGrid::PreTranslateMessage(MSG* pMsg) 
{
	BOOL bReturn = CGridListCtrl::PreTranslateMessage(pMsg);

	if(pMsg->message == WM_KEYDOWN)
	{
		if(GetFocus()==this)
		{
			switch( pMsg->wParam )
			{
				case VK_UP:
				case VK_DOWN:
					{
						m_CurSubItem = EDITABLE_COL+1; //always have selection on 2nd column
						int iItem = GetSelectedItem();
						if( iItem != -1 )
						{
							CRect rcBounds;
							GetItemRect(iItem, rcBounds, LVIR_BOUNDS);
							InvalidateRect(&rcBounds);
							UpdateWindow();
						}
					}
					break;
				case VK_LEFT:
					{
						m_CurSubItem = EDITABLE_COL+1; //always have selection on 2nd column
						int iItem = GetSelectedItem();
						if( iItem != -1 )
						{
							CWaitCursor wait;
							CGridTreeItem *pSelItem = GetTreeItem(iItem);
							if(OnVkSubTract(pSelItem, iItem))
							{
								Collapse(pSelItem);
							}
							CRect rc;
							GetItemRect(iItem, rc, LVIR_BOUNDS);
							InvalidateRect(rc);
							UpdateWindow();
						}
						bReturn = TRUE;
					}
					break;
				case VK_RIGHT:
					{
						m_CurSubItem = EDITABLE_COL+1; //always have selection on 2nd column
						int iItem = GetSelectedItem();
						if( iItem != -1 )
						{
							CWaitCursor wait;
							CGridTreeItem *pSelItem = GetTreeItem(iItem);
							int nScrollIndex = 0;
							if(OnVKAdd(pSelItem, iItem))
							{
								 nScrollIndex = Expand(pSelItem, iItem);
							}
							CRect rc;
							GetItemRect(iItem, rc, LVIR_BOUNDS);
							InvalidateRect(rc);
							UpdateWindow();
							EnsureVisible(nScrollIndex, 1);
						}
						bReturn = TRUE;
					}
					break;
				default:
					break;
			}
		}
	}

	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////

void CValueGrid::OnMouseMove(UINT nFlags, CPoint point) 
{
    if( m_bSplitterDrag )
    {
        CRect r;
        GetClientRect( &r );
        int Delta = point.x - m_SplitterDragStartPoint.x;
        int Width0 = m_SplitterDragStartWidth + Delta;

        if( Width0 < 6 ) Width0 = m_SplitterDragStartWidth;
        if( Width0 > ( r.Width() - 60 ) ) Width0 = m_SplitterDragStartWidth;
        int Width1 = 40;
        int Width2 = r.Width() - Width0 - Width1;

        // Set column widths so the combined total never exceeds the window width,
        // that stops the scroll bar popping onto the screen
        if( Width0 < GetColumnWidth(0) )
        {
            SetColumnWidth( 0, Width0 );
            SetColumnWidth( 1, Width1 );
            SetColumnWidth( 2, Width2 );
        }
        else
        {
            SetColumnWidth( 2, Width2 );
            SetColumnWidth( 1, Width1 );
            SetColumnWidth( 0, Width0 );
        }

        return;
    }
    else
        CGridListCtrl::OnMouseMove( nFlags, point );
}