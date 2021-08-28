// EditorWatchView.cpp : implementation file
//

#include "StdAfx.h"
#include "EditorWatchView.h"
#include "EditorPaletteDoc.h"
#include "EditorFrame.h"
#include "EditorView.h"
#include "Resource.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "WorldEditor.hpp"
#include "..\Editor\Project.hpp"
#include "Auxiliary\MiscUtils\guid.hpp"
#include "..\WinControls\ListBoxDlg.h"
#include "Obj_Mgr\Obj_Mgr.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CEditorWatchView* CEditorWatchView::s_WatchView = NULL;

//=========================================================================
// CEditorWatchView
//=========================================================================

IMPLEMENT_DYNCREATE(CEditorWatchView, CPaletteView)

CEditorWatchView::CEditorWatchView()
{
    s_WatchView = this;
}

//=========================================================================

CEditorWatchView::~CEditorWatchView()
{ 
}

//=========================================================================

BEGIN_MESSAGE_MAP(CEditorWatchView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorWatchView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_COMMAND(ID_WVTB_WATCH_PROP, OnWvtbPropWatch)
	ON_UPDATE_COMMAND_UI(ID_WVTB_WATCH_PROP, OnUpdateWvtbPropWatch)
	ON_COMMAND(ID_WVTB_WATCH_GLOBAL, OnWvtbGlobalWatch)
	ON_UPDATE_COMMAND_UI(ID_WVTB_WATCH_GLOBAL, OnUpdateWvtbGlobalWatch)
	ON_COMMAND(ID_WVTB_CLEAR_INFO, OnWvtbClearInfo)
	ON_UPDATE_COMMAND_UI(ID_WVTB_CLEAR_INFO, OnUpdateWvtbClearInfo)
	ON_COMMAND(ID_WVTB_DELETE_WATCH, OnWvtbDeleteWatch)
	ON_UPDATE_COMMAND_UI(ID_WVTB_DELETE_WATCH, OnUpdateWvtbDeleteWatch)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=========================================================================
// CEditorWatchView drawing
//=========================================================================

void CEditorWatchView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//=========================================================================
// CEditorWatchView diagnostics
//=========================================================================

#ifdef _DEBUG
void CEditorWatchView::AssertValid() const
{
	CPaletteView::AssertValid();
}

void CEditorWatchView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================
// CEditorWatchView message handlers
//=========================================================================


int CEditorWatchView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_WATCHVIEW_FILTER;   
	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_Watch.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT, CRect(0,0,0,0), this, IDR_WATCH_LIST))
    {
        ASSERT(FALSE);
        return -1;
    }

	m_Watch.InsertColumn(0, "*",     LVCFMT_LEFT, 12);
	m_Watch.InsertColumn(1, "Watch", LVCFMT_LEFT, 150);
	m_Watch.InsertColumn(2, "Type",  LVCFMT_LEFT, 60);
	m_Watch.InsertColumn(3, "Value", LVCFMT_LEFT, 130);
	m_Watch.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_Watch.SubclassHeader();
	m_Watch.GetFlatHeaderCtrl()->ShowSortArrow(TRUE);
    m_Watch.SetSortImage(0, false);	

	return 0;
}

//=========================================================================

void CEditorWatchView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorWatchView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

    CSize size = SizeToolBar(cx, cy);
    m_Watch.MoveWindow(0,size.cy,cx,cy - size.cy);
}

//=========================================================================

void CEditorWatchView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::WatchWindow");
        RefreshView();
    }
}

//=========================================================================

void CEditorWatchView::RefreshView( void )
{
    m_Watch.DeleteAllItems();

    s32 nItems = 0;
    
    //refresh globals
    for (s32 i = 0; i < m_globalsList.GetCount(); i++)
    {
        watch_globals &GlobalItem = m_globalsList.GetAt(i);

        m_Watch.InsertItem( nItems, "G");
        m_Watch.SetItem( nItems, 1, LVIF_TEXT, GlobalItem.GlobalName, 0, 0, 0, 0);

        switch (GlobalItem.GlobalType)
        {
        case var_mngr::GLOBAL_FLOAT: 
            {
                m_Watch.SetItem( nItems, 2, LVIF_TEXT, "float", 0, 0, 0, 0);
                xhandle rHandle;
                if (g_VarMgr.GetVarHandle(GlobalItem.GlobalName, &rHandle))
                {
                    m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("%g", g_VarMgr.GetFloat(rHandle)), 0, 0, 0, 0);
                }
                else
                {
                    m_Watch.SetItem( nItems, 3, LVIF_TEXT, "???", 0, 0, 0, 0);
                }
            }
            break;
        case var_mngr::GLOBAL_INT:     
            {
                m_Watch.SetItem( nItems, 2, LVIF_TEXT, "int", 0, 0, 0, 0);
                xhandle rHandle;
                if (g_VarMgr.GetVarHandle(GlobalItem.GlobalName, &rHandle))
                {
                    m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("%s (int)", (const char*)GlobalItem.GlobalName), 0, 0, 0, 0);
                }
                else
                {
                    m_Watch.SetItem( nItems, 3, LVIF_TEXT, "???", 0, 0, 0, 0);
                }
            }
            break;
        case var_mngr::GLOBAL_BOOL:   
            {
                m_Watch.SetItem( nItems, 2, LVIF_TEXT, "bool", 0, 0, 0, 0);
                xhandle rHandle;
                if (g_VarMgr.GetVarHandle(GlobalItem.GlobalName, &rHandle))
                {
                    if (g_VarMgr.GetBool(rHandle))
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, "true", 0, 0, 0, 0);
                    else
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, "false", 0, 0, 0, 0);
                }
                else
                {
                    m_Watch.SetItem( nItems, 3, LVIF_TEXT, "???", 0, 0, 0, 0);
                }
            }
            break;
        case var_mngr::GLOBAL_TIMER:
            {
                m_Watch.SetItem( nItems, 2, LVIF_TEXT, "timer", 0, 0, 0, 0);
                m_Watch.SetItem( nItems, 3, LVIF_TEXT, "???", 0, 0, 0, 0);
            }
            break;
        case var_mngr::GLOBAL_GUID:     
            {
                m_Watch.SetItem( nItems, 2, LVIF_TEXT, "guid", 0, 0, 0, 0);
                xhandle rHandle;
                if (g_VarMgr.GetGuidHandle(GlobalItem.GlobalName, &rHandle))
                {
                    m_Watch.SetItem( nItems, 3, LVIF_TEXT, guid_ToString(g_VarMgr.GetGuid(rHandle)), 0, 0, 0, 0);
                }
                else
                {
                    m_Watch.SetItem( nItems, 3, LVIF_TEXT, "???", 0, 0, 0, 0);
                }
            }
            break;
        default: 
            {
                m_Watch.SetItem( nItems, 2, LVIF_TEXT, "???", 0, 0, 0, 0);
                m_Watch.SetItem( nItems, 3, LVIF_TEXT, "???", 0, 0, 0, 0);
            }
            break;
        }
        nItems++;
    }

    //refresh properties
    for (i = 0; i < m_propsList.GetCount(); i++)
    {
        watch_obj_props &PropItem = m_propsList.GetAt(i);

        m_Watch.InsertItem( nItems, "P");
        m_Watch.SetItem( nItems, 1, LVIF_TEXT, xfs("%s (%s)", (const char*)PropItem.PropName, (const char*)guid_ToString(PropItem.ObjGuid) ), 0, 0, 0, 0);

        object* pObj = g_ObjMgr.GetObjectByGuid(PropItem.ObjGuid);
        if (pObj)
        {
            prop_query pq;
            switch(PropItem.PropType & PROP_TYPE_BASIC_MASK )
            {
	        case PROP_TYPE_FILENAME:     
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "filename", 0, 0, 0, 0);
                    char cString[MAX_PATH];
                    pq.RQueryFileName( PropItem.PropName, &cString[0]);
                    if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, cString, 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_STRING:     
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "string", 0, 0, 0, 0);
                    char cString[MAX_PATH];
                    pq.RQueryString( PropItem.PropName, &cString[0]);
                    if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, cString, 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_FLOAT:     
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "float", 0, 0, 0, 0);
                    f32 f;
                    pq.RQueryFloat( PropItem.PropName, f );
			        if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("%g",f), 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_INT:       
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "int", 0, 0, 0, 0);
                    s32 n;
                    pq.RQueryInt( PropItem.PropName, n );
			        if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("%d",n), 0, 0, 0, 0);
                    }
                }				
		        break;
            case PROP_TYPE_BOOL:
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "bool", 0, 0, 0, 0);
                    xbool b;
                    pq.RQueryBool( PropItem.PropName, b );
			        if (pObj->OnProperty(pq)) 
                    {
                        if (b)
                            m_Watch.SetItem( nItems, 3, LVIF_TEXT, "true", 0, 0, 0, 0);
                        else
                            m_Watch.SetItem( nItems, 3, LVIF_TEXT, "false", 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_VECTOR3:
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "vector3", 0, 0, 0, 0);
                    vector3 v3;
                    pq.RQueryVector3( PropItem.PropName, v3 );
			        if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("%g, %g, %g",v3[0],v3[1],v3[2]), 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_ANGLE:     
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "angle", 0, 0, 0, 0);
                    radian r;
                    pq.RQueryAngle( PropItem.PropName, r );
			        if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("%g",RAD_TO_DEG(r)), 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_GUID:     
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "guid", 0, 0, 0, 0);
                    guid g;
                    pq.RQueryGUID( PropItem.PropName, g );
			        if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, guid_ToString(g), 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_ROTATION:  			
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "radian3", 0, 0, 0, 0);
                    radian3 r3;
                    pq.RQueryRotation( PropItem.PropName, r3 );
			        if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("%g, %g, %g",RAD_TO_DEG(r3.Roll),RAD_TO_DEG(r3.Pitch),RAD_TO_DEG(r3.Yaw)), 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_BBOX:      
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "bbox", 0, 0, 0, 0);
                    bbox box;
                    pq.RQueryBBox( PropItem.PropName, box );
			        if (pObj->OnProperty(pq))
                    {
                        vector3 vExtent = box.GetSize();
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("Extent {%g, %g, %g}",vExtent[0], vExtent[1], vExtent[2]), 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_COLOR:      
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "color", 0, 0, 0, 0);
                    xcolor xc;
                    pq.RQueryColor( PropItem.PropName, xc );
			        if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, xfs("%d, %d, %d, %d",xc.R,xc.G,xc.B,xc.A), 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_ENUM:  
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "enum", 0, 0, 0, 0);
                    char cString[MAX_PATH];
                    pq.RQueryEnum( PropItem.PropName, &cString[0]);
                    if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, cString, 0, 0, 0, 0);
                    }
                }
		        break;
            case PROP_TYPE_EXTERNAL:
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "external", 0, 0, 0, 0);
                    char cString[MAX_PATH];
                    pq.RQueryExternal( PropItem.PropName, &cString[0]);
                    if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, cString, 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_BUTTON:     
                {
                    m_Watch.SetItem( nItems, 2, LVIF_TEXT, "button", 0, 0, 0, 0);
                    char cString[MAX_PATH];
                    pq.RQueryButton( PropItem.PropName, &cString[0]);
                    if (pObj->OnProperty(pq))
                    {
                        m_Watch.SetItem( nItems, 3, LVIF_TEXT, cString, 0, 0, 0, 0);
                    }
                }
		        break;
	        case PROP_TYPE_NULL: 
            default:
                m_Watch.SetItem( nItems, 2, LVIF_TEXT, "???", 0, 0, 0, 0);
                m_Watch.SetItem( nItems, 3, LVIF_TEXT, "???", 0, 0, 0, 0);
                break;
            }
        }
        else
        {
            m_Watch.SetItem( nItems, 2, LVIF_TEXT, "???", 0, 0, 0, 0);
            m_Watch.SetItem( nItems, 3, LVIF_TEXT, "???", 0, 0, 0, 0);
        }
        
        nItems++;
    }
}

//=========================================================================

void CEditorWatchView::OnUpdateWvtbClearInfo(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorWatchView::OnWvtbClearInfo() 
{
    m_propsList.Clear();
    m_globalsList.Clear();
    RefreshView();
}

//=========================================================================

void CEditorWatchView::OnUpdateWvtbDeleteWatch(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorWatchView::OnWvtbDeleteWatch() 
{
    POSITION pos = m_Watch.GetFirstSelectedItemPosition();
    while (pos)
    {
        int nItem = m_Watch.GetNextSelectedItem(pos);
        CString strType = m_Watch.GetItemText( nItem, 0 );
        CString strName = m_Watch.GetItemText( nItem, 1 );
        if (strType.CompareNoCase("G") == 0)
        {
            for (s32 i = 0; i < m_globalsList.GetCount(); i++)
            {
                watch_globals &GlobalItem = m_globalsList.GetAt(i);
                if (strName.CompareNoCase(GlobalItem.GlobalName) == 0)
                {
                    //found it
                    m_globalsList.Delete(i);
                    break;
                }
            }            
        }
        else //Property
        {
            for (s32 i = 0; i < m_propsList.GetCount(); i++)
            {
                watch_obj_props &PropItem = m_propsList.GetAt(i);
                if (strName.CompareNoCase(xfs("%s (%s)", (const char*)PropItem.PropName, (const char*)guid_ToString(PropItem.ObjGuid) )) == 0)
                {
                    //found it
                    m_propsList.Delete(i);
                    break;
                }
            }
        }
    }
    RefreshView();
}

//=========================================================================

void CEditorWatchView::OnUpdateWvtbPropWatch(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen() && (g_WorldEditor.GetSelectedCount() == 1));
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorWatchView::OnWvtbPropWatch() 
{
    CListBoxDlg dlg;
    dlg.SetDisplayText("Which Property do you want to watch?");

    xarray<guid> selectedObjects;
    g_WorldEditor.GetSelectedList(selectedObjects);
    if (selectedObjects.GetCount() != 1)
        return;

    guid ObjGuid = selectedObjects.GetAt(0);
    object* pObj = g_ObjMgr.GetObjectByGuid(ObjGuid);
    if (pObj)
    {
        prop_enum List;
        pObj->OnEnumProp(List);
        for (int i=0; i < List.GetCount(); i++)
        {
            prop_enum::node& enData = List[i];
            dlg.AddString( enData.GetName() );
        }

        if (dlg.DoModal() == IDOK)
        {
            CString strValue = dlg.GetSelection();

            watch_obj_props NewPropWatch;
            NewPropWatch.PropName = strValue;
            NewPropWatch.ObjGuid  = ObjGuid;

            prop_enum ListCompare;
            pObj->OnEnumProp(ListCompare);
            for (int j=0; j < ListCompare.GetCount(); j++)
            {
                prop_enum::node& enData = ListCompare[j];
                if (x_strcmp(enData.GetName(), strValue) ==0 )
                {
                    //found it
                    NewPropWatch.PropType = (prop_type)enData.GetType();
                    break;
                }
            }
                
            m_propsList.Append(NewPropWatch);
            RefreshView();
        }
    }
}

//=========================================================================

void CEditorWatchView::OnUpdateWvtbGlobalWatch(CCmdUI* pCmdUI) 
{
    pCmdUI->Enable(g_Project.IsProjectOpen());
    pCmdUI->SetCheck(FALSE);
}

//=========================================================================

void CEditorWatchView::OnWvtbGlobalWatch() 
{
    CListBoxDlg dlg;
    dlg.SetDisplayText("Which Global do you want to watch?");

    xharray<var_mngr::global_def> hGlobalArray;
    g_VarMgr.GetGlobalsList(hGlobalArray);
    //we should now have all the current globals...
    for ( int i = 0; i < hGlobalArray.GetCount(); i++ )
    {
        var_mngr::global_def& Def = hGlobalArray[i];
        CString strGlobal(Def.Name.Get());
        dlg.AddString(strGlobal);
    }

    if (dlg.DoModal() == IDOK)
    {
        CString strValue = dlg.GetSelection();

        xhandle rHandle;
        watch_globals NewGlobalWatch;
        NewGlobalWatch.GlobalName = strValue;
        
        if (g_VarMgr.GetVarHandle(strValue, &rHandle) )
        {
            if (g_VarMgr.GetType(rHandle) == var_mngr::TYPE_FLOAT)
            {
                NewGlobalWatch.GlobalType = var_mngr::GLOBAL_FLOAT;
            }
            else if (g_VarMgr.GetType(rHandle) == var_mngr::TYPE_INT)
            {
                NewGlobalWatch.GlobalType = var_mngr::GLOBAL_INT;
            }
            else if (g_VarMgr.GetType(rHandle) == var_mngr::TYPE_BOOL)
            {
                NewGlobalWatch.GlobalType = var_mngr::GLOBAL_BOOL;
            }
            else
            {
                ASSERT(FALSE);
                return;
            }
        }
        else if (g_VarMgr.GetGuidHandle(strValue, &rHandle) )
        {
            NewGlobalWatch.GlobalType = var_mngr::GLOBAL_GUID;
        }
        else if (g_VarMgr.GetTimerHandle(strValue, &rHandle) )
        {
            NewGlobalWatch.GlobalType = var_mngr::GLOBAL_TIMER;
        }
        else
        {
            ASSERT(FALSE);
            return;
        }

        m_globalsList.Append(NewGlobalWatch);
        RefreshView();
    }
}
