// EditorTriggerView.cpp : implementation file
//

#include "StdAfx.h"
#include "EditorTriggerView.h"
#include "EditorPaletteDoc.h"
#include "EditorFrame.h"
#include "EditorDoc.h"
#include "EditorView.h"
#include "Resource.h"
#include "WorldEditor.hpp"
#include "Auxiliary\MiscUtils\guid.hpp"
#include "Obj_Mgr\Obj_Mgr.hpp"
#include "TriggerEx\TriggerEx_Object.hpp"
#include "objects\object.hpp"
#include "..\Editor\Project.hpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//=========================================================================
// CEditorTriggerView
//=========================================================================

IMPLEMENT_DYNCREATE(CEditorTriggerView, CPaletteView)

CEditorTriggerView::CEditorTriggerView()
{
}

//=========================================================================

CEditorTriggerView::~CEditorTriggerView()
{ 
}

//=========================================================================

BEGIN_MESSAGE_MAP(CEditorTriggerView, CPaletteView)
	//{{AFX_MSG_MAP(CEditorTriggerView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_NOTIFY(NM_CLICK, IDR_TRIGGER_VIEW_LIST, OnClickTriggerItem)
	ON_COMMAND(ID_TRTB_REFRESH, OnTrtbRefresh)
	ON_UPDATE_COMMAND_UI(ID_TRTB_REFRESH, OnUpdateTrtbRefresh)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()


//=========================================================================
// CEditorTriggerView drawing
//=========================================================================

void CEditorTriggerView::OnDraw(CDC* pDC)
{
//	CDocument* pDoc = GetDocument();
	// TODO: add draw code here
}

//=========================================================================
// CEditorTriggerView diagnostics
//=========================================================================

#ifdef _DEBUG
void CEditorTriggerView::AssertValid() const
{
	CPaletteView::AssertValid();
}

void CEditorTriggerView::Dump(CDumpContext& dc) const
{
	CPaletteView::Dump(dc);
}
#endif //_DEBUG

//=========================================================================
// CEditorTriggerView message handlers
//=========================================================================


int CEditorTriggerView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
    m_ToolbarResourceId = IDR_TRIGGER_VIEW_BAR;   

	if (CPaletteView::OnCreate(lpCreateStruct) == -1)
		return -1;

    if (!m_TriggerLst.Create(WS_VISIBLE | WS_CHILD | LVS_REPORT, CRect(0,0,0,0), this, IDR_TRIGGER_VIEW_LIST))
    {
        ASSERT(FALSE);
        return -1;
    }

	m_TriggerLst.InsertColumn(0, "Name",        LVCFMT_LEFT, 120);
	m_TriggerLst.InsertColumn(1, "Type",        LVCFMT_LEFT, 60);
	m_TriggerLst.InsertColumn(2, "Active",      LVCFMT_LEFT, 40);
    m_TriggerLst.InsertColumn(3, "Status",      LVCFMT_LEFT, 40);
	m_TriggerLst.InsertColumn(4, "Description", LVCFMT_LEFT, 130);
	m_TriggerLst.InsertColumn(5, "Zone1",       LVCFMT_LEFT, 120);
	m_TriggerLst.InsertColumn(6, "Zone2",       LVCFMT_LEFT, 120);
	m_TriggerLst.InsertColumn(7, "Guid",        LVCFMT_LEFT, 120);
	m_TriggerLst.InsertColumn(8, "State",       LVCFMT_LEFT, 80);
	m_TriggerLst.InsertColumn(9, "Dialog",      LVCFMT_LEFT, 40);
    m_TriggerLst.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_TriggerLst.SubclassHeader();
	m_TriggerLst.GetFlatHeaderCtrl()->ShowSortArrow(TRUE);
    m_TriggerLst.SetSortImage(0, false);	

	return 0;
}

//=========================================================================

void CEditorTriggerView::OnInitialUpdate() 
{
	CPaletteView::OnInitialUpdate();
}	

//=========================================================================

void CEditorTriggerView::OnSize(UINT nType, int cx, int cy) 
{
	CPaletteView::OnSize(nType, cx, cy);

    CSize size = SizeToolBar(cx, cy);
    m_TriggerLst.MoveWindow(0,size.cy,cx,cy - size.cy);
}

//=========================================================================

void CEditorTriggerView::OnTabActivate(BOOL bActivate) 
{
    CPaletteView::OnTabActivate(bActivate);

    if (bActivate)
    {
        GetDocument()->GetTabParent()->SetCaption("Workspace::Triggers");
        RefreshView();
    }
}

//=========================================================================

void CEditorTriggerView::RefreshView( void )
{
    m_TriggerLst.DeleteAllItems();
    s32 nItems = 0;

    slot_id SlotID = g_ObjMgr.GetFirst( object::TYPE_TRIGGER_EX );
    while( SlotID != SLOT_NULL )
    {
        object* pObject = g_ObjMgr.GetObjectBySlot( SlotID );
        SlotID = g_ObjMgr.GetNext(SlotID);

        if (pObject && pObject->IsKindOf( trigger_ex_object::GetRTTI()))
        {
            trigger_ex_object* pTrigger = (trigger_ex_object*)pObject;

            m_TriggerLst.InsertItem( nItems, pTrigger->GetTriggerName());
            m_TriggerLst.SetItem( nItems, 1, LVIF_TEXT, pTrigger->GetTriggerTypeString(), 0, 0, 0, 0);

            if (pTrigger->IsActive())
                m_TriggerLst.SetItem( nItems, 2, LVIF_TEXT, "True", 0, 0, 0, 0);
            else
                m_TriggerLst.SetItem( nItems, 2, LVIF_TEXT, "False", 0, 0, 0, 0);

            m_TriggerLst.SetItem( nItems, 3, LVIF_TEXT, pTrigger->GetTriggerActionString(), 0, 0, 0, 0);
            m_TriggerLst.SetItem( nItems, 4, LVIF_TEXT, pTrigger->GetTriggerDescription(), 0, 0, 0, 0);
            m_TriggerLst.SetItem( nItems, 5, LVIF_TEXT, g_WorldEditor.GetZoneForId((u8)pTrigger->GetZone1()), 0, 0, 0, 0);
            m_TriggerLst.SetItem( nItems, 6, LVIF_TEXT, g_WorldEditor.GetZoneForId((u8)pTrigger->GetZone2()), 0, 0, 0, 0);
            m_TriggerLst.SetItem( nItems, 7, LVIF_TEXT, guid_ToString(pTrigger->GetGuid()), 0, 0, 0, 0);
            m_TriggerLst.SetItem( nItems, 8, LVIF_TEXT, pTrigger->GetTriggerStatusString(), 0, 0, 0, 0);

            if (pTrigger->HasDialogLine())
                m_TriggerLst.SetItem( nItems, 9, LVIF_TEXT, "True", 0, 0, 0, 0);
            else
                m_TriggerLst.SetItem( nItems, 9, LVIF_TEXT, "False", 0, 0, 0, 0);

            nItems++;
        }
    }
}

//=========================================================================

void CEditorTriggerView::OnClickTriggerItem(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMLISTVIEW* lphdi = (NMLISTVIEW*)pNMHDR;

    if (lphdi->iItem != -1)
    {
        CString strGuid = m_TriggerLst.GetItemText( lphdi->iItem, 7 );
        guid Guid = guid_FromString(strGuid);
        if (( GetKeyState( VK_CONTROL ) & ~1 ) != 0)
        {
            g_WorldEditor.SelectObject(Guid, FALSE);
        }
        else
        {
            g_WorldEditor.SelectObject(Guid);
        }

        GetDocument()->GetFramePointer()->GetEditorDoc()->RefreshPropertyView();
        GetDocument()->GetFramePointer()->GetEditorView()->SetViewDirty();
    }

	*pResult = 0;
}

//=========================================================================

void CEditorTriggerView::OnUpdateTrtbRefresh(CCmdUI* pCmdUI)
{
    pCmdUI->SetCheck(FALSE);	
    pCmdUI->Enable(g_Project.IsProjectOpen());	
}

//=========================================================================

void CEditorTriggerView::OnTrtbRefresh()
{
    RefreshView();
}