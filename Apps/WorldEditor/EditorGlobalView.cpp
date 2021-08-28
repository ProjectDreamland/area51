// EditorGlobalView.cpp

#include "StdAfx.h"
#include "EditorGlobalView.h"
#include "EditorPaletteDoc.h"
#include "EditorFrame.h"
#include "EditorView.h"
#include "Resource.h"
#include "Auxiliary\MiscUtils\Guid.hpp"
#include "NewGlobalDlg.h"
#include "..\WinControls\StringEntryDlg.h"
#include "worldeditor.hpp"
#include "..\Editor\Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CEditorGlobalView

IMPLEMENT_DYNCREATE(CEditorGlobalView, CPaletteView)

BEGIN_MESSAGE_MAP(CEditorGlobalView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorGlobalView)
	ON_WM_CREATE()
    ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(ID_GVTB_DELETE_GLOBAL, OnGvtbDeleteGlobal)
	ON_UPDATE_COMMAND_UI(ID_GVTB_DELETE_GLOBAL, OnUpdateGvtbDeleteGlobal)
	ON_COMMAND(ID_GVTB_NEW_FOLDER, OnGvtbNewFolder)
	ON_UPDATE_COMMAND_UI(ID_GVTB_NEW_FOLDER, OnUpdateGvtbNewFolder)
	ON_COMMAND(ID_GVTB_NEW_GLOBAL, OnGvtbNewGlobal)
	ON_UPDATE_COMMAND_UI(ID_GVTB_NEW_GLOBAL, OnUpdateGvtbNewGlobal)
	ON_COMMAND(ID_GVTB_REFRESH, OnGvtbRefresh)
	ON_UPDATE_COMMAND_UI(ID_GVTB_REFRESH, OnUpdateGvtbRefresh)
	ON_MESSAGE( WM_USER_MSG_GRID_ITEM_CHANGE, OnGridItemChange )
	ON_MESSAGE( WM_USER_MSG_GUID_SELECT_REQUEST, OnGuidSelect )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditorGlobalView diagnostics

#ifdef _DEBUG
void CEditorGlobalView::AssertValid() const
{
	CPaletteView::AssertValid();
}

void CEditorGlobalView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================

CEditorGlobalView::CEditorGlobalView()
{
}

//=========================================================================

CEditorGlobalView::~CEditorGlobalView()
{
}

//=========================================================================

void CEditorGlobalView::OnDraw(CDC* pDC)
{
}

//=========================================================================

int CEditorGlobalView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_GLOBAL_FILTER;
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_tcGlobals.Create( WS_CHILD|WS_VISIBLE|WS_TABSTOP, CRect(0,0,0,0), this, IDR_GLOBALS_LIST))
    {
		TRACE0("Failed to create globabls tree control\n");
        return -1;
    }

    m_tcGlobals.InitializeGrid();
    RefreshView();

    m_tcGlobals.LoadColumnState( "BarState - Globals" );

	return 0;
}

//=========================================================================

void CEditorGlobalView::OnDestroy()
{
    m_tcGlobals.SaveColumnState( "BarState - Globals" );
}

//=========================================================================

void CEditorGlobalView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorGlobalView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

    CSize size = SizeToolBar(cx, cy);
    m_tcGlobals.MoveWindow(0,size.cy,cx,cy-size.cy);

	CRect rect;
	m_tcGlobals.GetClientRect(&rect);
	int nWidth = rect.Width() - m_tcGlobals.GetColumnWidth(0) - m_tcGlobals.GetColumnWidth(1);
	m_tcGlobals.SetColumnWidth(2,nWidth);
	m_tcGlobals.ShowScrollBar( SB_HORZ, FALSE );
}

//=========================================================================

void CEditorGlobalView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::Globals");
    }
}

//=========================================================================

void CEditorGlobalView::RefreshView( void )
{
    m_tcGlobals.DeleteAll();

    if (!g_Project.IsProjectOpen())
        return;

	CGridItemInfo* lp = new CGridItemInfo();
	lp->SetItemText("Globals");
	lp->AddSubItemText("");// 1
	lp->SetReadOnly(TRUE);
    lp->SetControlType(CGridItemInfo::GCT_STRING_EDIT,0); //first sub column
    lp->SetIsHeader(TRUE);
    lp->SetDataIndex(0);
    m_tcGlobals.InsertRootItem(lp);

    g_VarMgr.GetGlobalsList(m_hGlobalArray);
    //we should now have all the current globals...
    for ( int i = 0; i < m_hGlobalArray.GetCount(); i++ )
    {
        var_mngr::global_def& Def = m_hGlobalArray[i];
        CString strFullPath(Def.Name.Get());
        AddGlobalToView( strFullPath, Def.Type);
    }

    //expand all roots
	POSITION pos = m_tcGlobals.GetRootHeadPosition();
	while(pos != NULL)
	{
		CGridTreeItem *pItem = (CGridTreeItem*)m_tcGlobals.GetNextRoot(pos); 
        if (pItem && m_tcGlobals.IsCollapsed(pItem))
        {
            m_tcGlobals.Expand(pItem,pItem->m_nIndex);
        }
    }
}

//=========================================================================

void CEditorGlobalView::AddGlobalToView( CString& strVar, var_mngr::global_types Type )
{
    int iSlash = strVar.Find("\\");
    if (iSlash == -1)
        return;

    CString strRoot   = strVar.Left(iSlash);
    CString strGlobal = strVar.Right(strVar.GetLength()-1-iSlash);

	POSITION pos = m_tcGlobals.GetRootHeadPosition();
    CGridTreeItem *pRootItem = NULL;
    BOOL bPoolFound = FALSE;
	while(pos != NULL)
	{
		pRootItem = (CGridTreeItem*)m_tcGlobals.GetNextRoot(pos); 
        if (pRootItem && (strRoot.Compare(pRootItem->m_lpNodeInfo->GetItemText()) == 0 ))
        {
            bPoolFound = TRUE;
            break;
        }
    }

    if (!bPoolFound)
    {
	    CGridItemInfo* pRootInfo = new CGridItemInfo();
	    pRootInfo->SetItemText(strRoot);
	    pRootInfo->SetReadOnly(TRUE);
        pRootInfo->SetIsHeader(TRUE);
        pRootItem = m_tcGlobals.InsertRootItem(pRootInfo);
    }

	CGridItemInfo* lp = new CGridItemInfo();
	lp->SetItemText(strGlobal);
    lp->SetControlType(CGridItemInfo::GCT_NULL_ENTRY,0);

    switch (Type)
    {
    case var_mngr::GLOBAL_FLOAT: 
        {
            lp->SetControlType(CGridItemInfo::GCT_FLOAT_EDIT,1); 
            lp->AddSubItemText("float");
            xhandle rHandle;
            if (g_VarMgr.GetVarHandle(strVar, &rHandle))
            {
                lp->AddSubItemText(CString(xfs("%g", g_VarMgr.GetFloat(rHandle))));
            }
        }
        break;
    case var_mngr::GLOBAL_INT:     
        {
            lp->SetControlType(CGridItemInfo::GCT_NUMERIC_EDIT,1); 
            lp->AddSubItemText("int");
            xhandle rHandle;
            if (g_VarMgr.GetVarHandle(strVar, &rHandle))
            {
                lp->AddSubItemText(CString(xfs("%d", g_VarMgr.GetInt(rHandle))));
            }
        }
        break;
    case var_mngr::GLOBAL_BOOL:   
        {
            lp->SetControlType(CGridItemInfo::GCT_BOOL,1); 
            lp->AddSubItemText("bool");
            xhandle rHandle;
            if (g_VarMgr.GetVarHandle(strVar, &rHandle))
            {
                if (g_VarMgr.GetBool(rHandle))
                    lp->AddSubItemText("true");
                else
                    lp->AddSubItemText("false");
            }
        }
        break;
    case var_mngr::GLOBAL_TIMER:
        {
            lp->SetControlType(CGridItemInfo::GCT_NULL_ENTRY,1); 
            lp->AddSubItemText("timer");
        }
        break;
    case var_mngr::GLOBAL_GUID:     
        {
            lp->SetControlType(CGridItemInfo::GCT_GUID_EDIT,1); 
            lp->AddSubItemText("guid");
            xhandle rHandle;
            if (g_VarMgr.GetGuidHandle(strVar, &rHandle))
            {
                lp->AddSubItemText(CString(guid_ToString(g_VarMgr.GetGuid(rHandle))));
            }
        }
        break;
    default: ASSERT(FALSE); break;
    }
    
    //add the child
	CGridTreeItem *pVarItem = m_tcGlobals.InsertItem(pRootItem, lp, FALSE /*expand*/);
    pVarItem->m_strIdentifier = strVar;
}


//=========================================================================

void CEditorGlobalView::OnGvtbDeleteGlobal() 
{
    int iItem = m_tcGlobals.GetSelectedItem();
    if (iItem != -1)
    {
	    CGridTreeItem* pTreeItem = m_tcGlobals.GetTreeItem(iItem);
        if (pTreeItem->m_pParent != NULL)
        {
            if ( g_WorldEditor.IsGlobalInUse(pTreeItem->m_strIdentifier) )
            {
                GetDocument()->GetFramePointer()->GetEditorView()->SetFocusPos(g_WorldEditor.GetMinPositionForSelected());
                GetDocument()->GetFramePointer()->GetEditorView()->FocusCamera();
                ::AfxMessageBox("You can not delete this global because there are objects loaded that still refer to this global. The first such object has been selected for you.");
                return;
            }
            else if (::AfxMessageBox(xfs("Are you sure you want to delete the global \"%s\". Any scripted objects using this global may fail to work after this action. This can not be undone!", pTreeItem->m_strIdentifier),
                MB_YESNO) != IDYES)
            {
                //don't do anything
                return;
            }
            
            //this is a global, delete it
            CGridItemInfo::CONTROLTYPE Type;
            pTreeItem->m_lpNodeInfo->GetControlType(1, Type);
            switch(Type)
            {
            case CGridItemInfo::GCT_FLOAT_EDIT:
            case CGridItemInfo::GCT_BOOL:
            case CGridItemInfo::GCT_NUMERIC_EDIT:
                {
                    xhandle rHandle;
                    if (g_VarMgr.GetVarHandle(pTreeItem->m_strIdentifier, &rHandle) )
                    {
                        g_VarMgr.DestroyVariable(rHandle);
                    }
                }
                break;
            case CGridItemInfo::GCT_NULL_ENTRY:
                {
                    xhandle rHandle;
                    if (g_VarMgr.GetTimerHandle(pTreeItem->m_strIdentifier, &rHandle) )
                    {
                        g_VarMgr.DestroyTimer(rHandle);
                    }
                }
                break;
            case CGridItemInfo::GCT_GUID_EDIT:
                {
                    xhandle rHandle;
                    if (g_VarMgr.GetGuidHandle(pTreeItem->m_strIdentifier, &rHandle) )
                    {
                        g_VarMgr.DestroyGuid(rHandle);
                    }
                }
                break;
            default:
                ASSERT(FALSE);
                break;
            }

            RefreshView();
        }
    }
}

//=========================================================================

void CEditorGlobalView::OnUpdateGvtbDeleteGlobal(CCmdUI* pCmdUI) 
{
    BOOL bEnable = FALSE;
    int iItem = m_tcGlobals.GetSelectedItem();
    if (iItem != -1 && g_Project.IsProjectOpen())
    {
	    CGridTreeItem* pTreeItem = m_tcGlobals.GetTreeItem(iItem);
        bEnable = (pTreeItem->m_pParent != NULL);
    }

    pCmdUI->Enable(bEnable);
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorGlobalView::OnGvtbNewFolder() 
{
    CStringEntryDlg dlg;
    dlg.SetDisplayText("Enter a new global folder name. (This is only temporary and a global var must immediately be added to this folder for this folder to remain)");
    dlg.SetEntryText("<GLOBAL FOLDER>");
    dlg.SetTextLimit(30);
    if (dlg.DoModal() == IDOK)
    {
        CString strFolder = dlg.GetEntryText();
	    POSITION pos = m_tcGlobals.GetRootHeadPosition();
	    while(pos != NULL)
	    {
		    CGridTreeItem* pRootItem = (CGridTreeItem*)m_tcGlobals.GetNextRoot(pos); 
            if (pRootItem && (strFolder.Compare(pRootItem->m_lpNodeInfo->GetItemText()) == 0 ))
            {
                ::AfxMessageBox("A Folder with that name already exists");
                return;
            }
        }

	    CGridItemInfo* pRootInfo = new CGridItemInfo();
	    pRootInfo->SetItemText(strFolder);
	    pRootInfo->SetReadOnly(TRUE);
        pRootInfo->SetIsHeader(TRUE);
        m_tcGlobals.InsertRootItem(pRootInfo);
    }
}

//=========================================================================

void CEditorGlobalView::OnUpdateGvtbNewFolder(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorGlobalView::OnGvtbNewGlobal() 
{
    int iItem = m_tcGlobals.GetSelectedItem();
    if (iItem == -1)
        return;

	CGridTreeItem* pTreeItem = m_tcGlobals.GetTreeItem(iItem);
    if (pTreeItem->m_pParent == NULL)
    {
        //this is a root item
        CNewGlobalDlg dlg;
        if (dlg.DoModal() == IDOK)
        {
            CString strGlobal(pTreeItem->m_lpNodeInfo->GetItemText());
            strGlobal += "\\";
            strGlobal += dlg.GetGlobalName();
            if (g_VarMgr.DoesGlobalExist(strGlobal))
            {
                ::AfxMessageBox("A Global of that name exists already!");
                return;
            }
            
            switch (dlg.GetGlobalType())
            {
            case var_mngr::GLOBAL_FLOAT: 
                g_VarMgr.RegisterFloat(strGlobal);
                break;
            case var_mngr::GLOBAL_INT:     
                g_VarMgr.RegisterInt(strGlobal);
                break;
            case var_mngr::GLOBAL_BOOL:   
                g_VarMgr.RegisterBool(strGlobal);
                break;
            case var_mngr::GLOBAL_TIMER:
                g_VarMgr.RegisterTimer(strGlobal);
                break;
            case var_mngr::GLOBAL_GUID:     
                g_VarMgr.RegisterGuid(strGlobal);
                break;
            default: ASSERT(FALSE); break;
            }

            //AddGlobalToView( strGlobal, dlg.GetGlobalType());
            RefreshView();
        }
    }
}

//=========================================================================

void CEditorGlobalView::OnUpdateGvtbNewGlobal(CCmdUI* pCmdUI) 
{
    BOOL bEnable = FALSE;
    int iItem = m_tcGlobals.GetSelectedItem();
    if (iItem != -1 && g_Project.IsProjectOpen())
    {
	    CGridTreeItem* pTreeItem = m_tcGlobals.GetTreeItem(iItem);
        bEnable = (pTreeItem->m_pParent == NULL);
    }

    pCmdUI->Enable(bEnable);
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorGlobalView::OnGvtbRefresh() 
{
	RefreshView();
}

//=========================================================================

void CEditorGlobalView::OnUpdateGvtbRefresh(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

LRESULT CEditorGlobalView::OnGridItemChange(WPARAM wParam, LPARAM lParam)
{
	CGridTreeItem* lpItem = (CGridTreeItem*)wParam;
    if (lpItem)
    {
	    TRACE(xfs("CEditorGlobalView::OnGridItemChange::SAVE PROPERTY %s\n",
            (const char*)lpItem->m_strIdentifier));

        CGridItemInfo::CONTROLTYPE Type;
        lpItem->m_lpNodeInfo->GetControlType(1, Type);
        CString strValue = lpItem->m_lpNodeInfo->GetSubItem(1);
        switch(Type)
        {
        case CGridItemInfo::GCT_FLOAT_EDIT:
            {
                xhandle rHandle;
                if (g_VarMgr.GetVarHandle(lpItem->m_strIdentifier, &rHandle) )
                {
                    g_VarMgr.SetFloat(rHandle, (f32)atof(strValue));
                }
            }
            break;
        case CGridItemInfo::GCT_BOOL:
            {
                xhandle rHandle;
                if (g_VarMgr.GetVarHandle(lpItem->m_strIdentifier, &rHandle) )
                {
                    if (strValue.CompareNoCase("true")==0)
                        g_VarMgr.SetBool(rHandle, TRUE);
                    else
                        g_VarMgr.SetBool(rHandle, FALSE);
                }
            }
            break;
        case CGridItemInfo::GCT_NUMERIC_EDIT:
            {
                xhandle rHandle;
                if (g_VarMgr.GetVarHandle(lpItem->m_strIdentifier, &rHandle) )
                {
                    g_VarMgr.SetInt(rHandle, atoi(strValue));
                }
            }
            break;
        case CGridItemInfo::GCT_NULL_ENTRY:
            //this shouldn't change
            ASSERT(FALSE);
            break;
        case CGridItemInfo::GCT_GUID_EDIT:
            {
                xhandle rHandle;
                if (g_VarMgr.GetGuidHandle(lpItem->m_strIdentifier, &rHandle) )
                {
                    g_VarMgr.SetGuid(rHandle, guid_FromString(strValue));
                }
            }
            break;
        default:
            ASSERT(FALSE);
            break;
        }
    }
    else 
    {
        ASSERT(FALSE);
        return 0;
    }

	return 1;
}

//=========================================================================

LRESULT CEditorGlobalView::OnGuidSelect(WPARAM wParam, LPARAM lParam)
{
	CGridTreeItem* lpItem = (CGridTreeItem*)wParam;
    if (lpItem)
    {
	    TRACE(xfs("CEditorGlobalView::OnGuidSelect::PROPERTY %s\n",
            (const char*)lpItem->m_strIdentifier));

        char* pszIdentifier = lpItem->m_strIdentifier.GetBuffer(lpItem->m_strIdentifier.GetLength());
        g_WorldEditor.SetGuidSelectMode((const char*)pszIdentifier, TRUE, FALSE);
    }
    else 
    {
        ASSERT(FALSE);
        return 0;
    }

	return 1;
}

//=========================================================================

BOOL CEditorGlobalView::PreCreateWindow(CREATESTRUCT& cs)
{
    // TODO: Add your specialized code here and/or call the base class
    cs.style |= WS_CLIPCHILDREN;

    return CPaletteView::PreCreateWindow(cs);
}
