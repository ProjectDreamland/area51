// EditorSettingsView.cpp : implementation file
//

#include "StdAfx.h"
#include "EditorSettingsView.h"
#include "EditorPaletteDoc.h"
#include "EditorFrame.h"
#include "resource.h"
#include "ManagerRegistration.hpp"
#include "..\PropertyEditor\PropertyEditorDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//=========================================================================
// CEditorSettingsView
//=========================================================================

IMPLEMENT_DYNCREATE(CEditorSettingsView, CPaletteView)

CEditorSettingsView::CEditorSettingsView() 
{
}

//=========================================================================

CEditorSettingsView::~CEditorSettingsView()
{
}

//=========================================================================


BEGIN_MESSAGE_MAP(CEditorSettingsView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorSettingsView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_LB_REGISTERED_MGRS, OnSelchangeList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=========================================================================
// CEditorSettingsView drawing
//=========================================================================

void CEditorSettingsView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//=========================================================================
// CEditorSettingsView diagnostics
//=========================================================================

#ifdef _DEBUG
void CEditorSettingsView::AssertValid() const
{
	CPaletteView::AssertValid();
}

//=========================================================================

void CEditorSettingsView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================
// CEditorSettingsView message handlers
//=========================================================================

int CEditorSettingsView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_mgrTree.Create(WS_VISIBLE | WS_CHILD | TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | 
                         TVS_SHOWSELALWAYS, CRect(0,0,0,0), this, IDC_LB_REGISTERED_MGRS))
    {
        ASSERT(FALSE);
        return -1;
    }

    for (int i=0; i < g_RegGameMgrs.GetCount(); i++)
    {
        AddItemToTree(g_RegGameMgrs.GetName(i));
    }
    
    SortAllChildren();

    return 0;
}

//=========================================================================

void CEditorSettingsView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorSettingsView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);
    m_mgrTree.MoveWindow(0,0,cx,cy);
}

//=========================================================================

void CEditorSettingsView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);
   
    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::Managers");
    }
}

//=========================================================================

void CEditorSettingsView::OnSelchangeList(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;
	
    if (pNMTreeView->itemNew.hItem)
    {
        CString strItem = ItemToPath(pNMTreeView->itemNew.hItem);

        if (!strItem.IsEmpty())
        {
            prop_interface* pProps = g_RegGameMgrs.GetManagerInterface(strItem);
            if (pProps)
            {
                GetDocument()->GetFramePointer()->GetSettingsEditorDoc()->SetInterface(*pProps);
                GetDocument()->GetFramePointer()->ShowSettings();
            }
            else
            {
                GetDocument()->GetFramePointer()->GetSettingsEditorDoc()->ClearGrid();
            }
        }
    }
}

//=========================================================================

void CEditorSettingsView::AddItemToTree( CString strItem )
{
    CString strPath = strItem;
    int nIndex = strPath.Find('\\');
    HTREEITEM hParent = TVI_ROOT;
    while (nIndex!=-1)
    {
        CString strCurrent = strPath.Left(nIndex);
        strPath = strPath.Right ( strPath.GetLength() - nIndex - 1);

        //does strCurrent exist at this level?
        HTREEITEM hCurrent = DoesChildExist(strCurrent, hParent);
        if ( hCurrent )
        {
            hParent = hCurrent;
        }
        else
        {
            //need to insert this item
            hParent = m_mgrTree.InsertItem(strCurrent, hParent);
        }

        nIndex = strPath.Find('\\');
    }

    HTREEITEM hItem = m_mgrTree.InsertItem(strPath, hParent);
}

//=========================================================================

HTREEITEM CEditorSettingsView::DoesChildExist(CString strCurrent, HTREEITEM hParent)
{
    HTREEITEM hNextItem;
    HTREEITEM hChildItem;
    
    if (hParent == TVI_ROOT)
    {
        hChildItem = m_mgrTree.GetRootItem();
        if (!hChildItem)
        {
            return NULL;
        }
    }
    else
    {
        if (m_mgrTree.ItemHasChildren(hParent))
        {   
            hChildItem = m_mgrTree.GetChildItem(hParent);
        }
        else
        {
            return NULL;
        }
    }

    while (hChildItem != NULL)
    {
        hNextItem = m_mgrTree.GetNextSiblingItem(hChildItem);
        if (strCurrent.CompareNoCase(m_mgrTree.GetItemText(hChildItem)) == 0)
        {
            //Found it!
            return hChildItem;
        }
        hChildItem = hNextItem;
    }

    return NULL;
}

//=========================================================================

void CEditorSettingsView::SortAllChildren( void )
{
    HTREEITEM hNextItem;
    HTREEITEM hChildItem;
    
    hChildItem = m_mgrTree.GetRootItem();
    
    if (!hChildItem)
    {
        return;
    }
    
    while (hChildItem != NULL)
    {
        hNextItem = m_mgrTree.GetNextSiblingItem(hChildItem);
        
        m_mgrTree.SortChildren(hChildItem);
        
        hChildItem = hNextItem;
    }
}

//=========================================================================

CString CEditorSettingsView::ItemToPath(HTREEITEM hItem)
{
    CString strPath;

    if (hItem)
    {
        //Create the full string of the tree item
        HTREEITEM hParent = hItem;
        while (hParent)
        {
            CString strItem = m_mgrTree.GetItemText(hParent);
            int nLength = strItem.GetLength();
            ASSERT(nLength);

            if (strItem.GetAt(nLength-1) == _T('\\'))
                strPath = strItem + strPath;
            else
            {
                if (strPath.GetLength())
                    strPath = strItem + _T('\\') + strPath;
                else
                    strPath = strItem;
            }
            hParent = m_mgrTree.GetParentItem(hParent);
        }
    }

    return strPath;
}