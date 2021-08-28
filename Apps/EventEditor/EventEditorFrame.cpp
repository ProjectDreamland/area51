// EventEditorFrame.cpp : implementation file
//

#include "stdafx.h"
#include "editor.h"
#include "EventEditorFrame.h"
#include "resource.h"
#include "EventTabView.h"
#include "EventEditorDoc.h"



#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
//#include "..\EDRscDesc\EDRscDesc_View.h"
//#include "..\EDRscDesc\FlatRscList_View.h"
//#include "..\EDRscDesc\EDRscDesc_Doc.h"
#include "..\Editor\Resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
/*
static col_data columns[] =
{
	{ _T("Sound Labels"),                428, LVCFMT_LEFT,  DT_STRING   },
	{ _T("Particles"),                   428, LVCFMT_LEFT,  DT_STRING   }
};
*/
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// GLOBALS
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// CEventEditorFrame

IMPLEMENT_DYNCREATE(CEventEditorFrame, CBaseFrame)

CEventEditorFrame::CEventEditorFrame() :
m_OnInit( TRUE )
{
}

CEventEditorFrame::~CEventEditorFrame()
{
}


BEGIN_MESSAGE_MAP(CEventEditorFrame, CBaseFrame)
	//{{AFX_MSG_MAP(CEventEditorFrame)
	ON_WM_CREATE()
	ON_MESSAGE			( WM_USER_MSG_SELECTION_CHANGE, 	OnPropertyEditorSelChange )
//    ON_MESSAGE			( WM_USER_MSG_LBUTTON_DBLCLK, 	    OnResourceEditorLbuttonDblClk )
	ON_COMMAND          ( ID_EVENT_EDITOR_NEW_SOUND,        OnNewSound      )
    ON_COMMAND          ( ID_EVENT_EDITOR_NEW_PARTICLE,     OnNewParticle   )
    ON_COMMAND          ( ID_EVENT_EDITOR_DEL,              OnDelete   )
	ON_COMMAND          ( ID_LOAD_SOUND_PACKAGE,            OnLoadSndPkg      )
    ON_COMMAND          ( ID_LOAD_GLOBAL_EVENTS,            OnLoadGlbEvents   )
    ON_COMMAND          ( ID_SAVE_GLOBAL_EVENTS,            OnSaveGlbEvents   )
    ON_COMMAND          ( ID_SAVE_EVENT_PACKAGE,            OnSavePackage   )
    ON_COMMAND          ( ID_NEW_EVENT_PACKAGE,             OnNewPackage    )
    ON_COMMAND          ( ID_LOAD_EVENT_PACKAGE,            OnLoadPackage   )
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEventEditorFrame message handlers


BOOL CEventEditorFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if( !CBaseFrame::PreCreateWindow(cs) )
		return FALSE;
	
	cs.style = WS_CHILD | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

	cs.style |= WS_CLIPCHILDREN;

	return TRUE;
}

//=========================================================================

void CEventEditorFrame::ActivateFrame(int nCmdShow)
{
    nCmdShow = SW_SHOWMAXIMIZED;
	CBaseFrame::ActivateFrame(nCmdShow);
    
    CWnd* pMain = AfxGetMainWnd();
    if (pMain != NULL)
    {
        int nAdded = pMain->SendMessage( WM_USER_MSG_MODIFY_MENU_BAR, IDR_EVENT_EDITOR_MENU, MAKELPARAM(TRUE,-1) ); //append at end
        pMain->SendMessage( WM_USER_MSG_MODIFY_MENU_BAR, IDR_EVENT_EDITOR_MENU, MAKELPARAM(FALSE,nAdded) ); //remove menu
        pMain->SendMessage( WM_USER_MSG_MODIFY_MENU_BAR, IDR_EVENT_EDITOR_MENU, MAKELPARAM(TRUE,2) ); //insert index 2
    }
    
    //
    // Connect the event editor with the property editor and resource view.
    //
    if( m_OnInit )
    {
        m_OnInit = FALSE;
	    // TODO: Add your own view and documents to the workspace window.
	    m_wndWrkspBar.AddView(_T("Tab 1"), RUNTIME_CLASS(CEventTabView), &GetDoc() );

        CEventEditorDoc& Doc = GetDoc();
        Doc.m_pPropEditor    = m_pPropEditor;
        Doc.m_pPropEditor->SetInterface( Doc.m_EventEditor );


        // We need to delete all the columns and insert new ones in.
/*        CFlatRscList_View* pView = (CFlatRscList_View*)m_wndResource.GetView(RUNTIME_CLASS(CFlatRscList_View));
        pView->DeleteAllColumn();
        pView->InsertColumns( columns, 2 );

        /////////////////////////////////////////////////////////////////////////////////////////////////////
        // This is temporary.
        /////////////////////////////////////////////////////////////////////////////////////////////////////
        CListCtrl* pListCtrl = &pView->GetListCtrl();
        for( s32 i = 0; i < 10; i++ )
        {
            pListCtrl->InsertItem( i, _T(xfs("Sound Item %d",i)) );
            pListCtrl->SetItemText(i, 1, _T(xfs("Particle Item %d", i)) );


        }
*/
    }

}

//=========================================================================

int CEventEditorFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBaseFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the workspace bar.
	if( !m_wndWrkspBar.Create(this, IDW_WORKSPBAR, _T("Global Events"),
		CSize(200, 150), CBRS_LEFT, (AFX_IDW_TOOLBAR + 7) ))
	{
		TRACE0("Failed to create workspace dock window\n");
		return -1;		// fail to create
	}
	
    // Create the ToolBar
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | TBBS_WRAPPED) ||
		!m_wndToolBar.LoadToolBar(IDR_EVENT_EDITOR_TOOL_BAR))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	
	// Create the property bar.
	if( !m_wndProperty.Create(this, IDW_PROPERTY_BAR, _T("Association Package"),
		CSize(200, 150), CBRS_LEFT, (AFX_IDW_TOOLBAR + 6) ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}

/*	// Create the property bar.
	if( !m_wndResource.Create(this, IDW_RESOURCE_BAR, _T("Resources"),
		CSize(900, 150), CBRS_LEFT, (AFX_IDW_TOOLBAR + 8) ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}

*/
	m_wndToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
	m_wndWrkspBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndProperty.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
//    m_wndResource.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);

	EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);

	DockControlBar(&m_wndToolBar,AFX_IDW_DOCKBAR_TOP);
	DockControlBar(&m_wndWrkspBar,AFX_IDW_DOCKBAR_LEFT);
	DockControlBar(&m_wndProperty,AFX_IDW_DOCKBAR_LEFT);
//    DockControlBar(&m_wndResource,AFX_IDW_DOCKBAR_LEFT);

	// Create the image list used with the tab control bar.
	if (!m_imageList.Create(IDB_IMGLIST_VIEW, 16, 1, RGB(0x00,0xff,0x00)))
	{
		TRACE0("Failed to create image list.\n");
		return -1;
	}

	// Associate the image list with the tab control bar.
	m_wndWrkspBar.SetTabImageList(&m_imageList);

    
/*    m_pResourceEditor = new EDRscDesc_Doc;
    if( m_pResourceEditor == NULL )
        x_throw( "Out of memory" );

    m_wndResource.AddView(_T("Resources"), RUNTIME_CLASS(CFlatRscList_View), m_pResourceEditor );
    if (m_pResourceEditor) m_pResourceEditor->SetCommandHandler(this);
*/        
    m_pPropEditor = new CPropertyEditorDoc;
    if( m_pPropEditor == NULL )
        x_throw( "Out of memory" );

	m_wndProperty.AddView(_T("Association Package"), RUNTIME_CLASS(CPropertyEditorView), m_pPropEditor );
    if (m_pPropEditor) m_pPropEditor->SetCommandHandler(this);

//    m_wndToolBar.EnableButton( ID_EVENT_EDITOR_NEW_DIR, TRUE );

	return 0;
}

//=========================================================================

LRESULT CEventEditorFrame::OnPropertyEditorSelChange(WPARAM wParam, LPARAM lParam)
{
    m_CurrentPropertyName = ((char*)wParam);
//    TRACE(xfs("Property Selchange to %s\n",strData));
    return 1;
}

//=========================================================================

LRESULT CEventEditorFrame::OnResourceEditorLbuttonDblClk(WPARAM wParam, LPARAM lParam)
{
    m_CurrentResourceName = ((char*)wParam);
    CEventEditorDoc&    Doc = *GetEventEditorDoc();

    s32 i = 0;
    s32 j = 0;
    s32 k = 0;
    s32 AudioEventCount = 0;
    s32 ParticleEventCount = 0;
    
    // Check if we have anything to begin with.
    if( Doc.m_EventEditor.m_pDesc->m_EventList.GetCount() <= 0 )
        return 1;
    
    // Start with the first event.
    s32 EventCount          = Doc.m_EventEditor.m_pDesc->m_EventList.GetCount();
    
    // Go through all the particles and sound list for each event.
    for( i = 0; i < EventCount; i++ )
    {
        event_description::event& Event = Doc.m_EventEditor.m_pDesc->m_EventList[ i ];
        AudioEventCount                 = Doc.m_EventEditor.m_pDesc->m_EventList[ i ].m_EventAudioList.GetCount();
        ParticleEventCount              = Doc.m_EventEditor.m_pDesc->m_EventList[ i ].m_EventParticleList.GetCount();

        for( j = 0; j < AudioEventCount; j++ )
        {
            // If it matches copy the Resource name over.
            if( !x_strcmp( (xfs( "%s\\Sound[%d]", Event.m_EventName, j)), (LPCTSTR)m_CurrentPropertyName ) )
            {
                x_strncpy( Event.m_EventAudioList[ j ].m_Name, (LPCTSTR)m_CurrentResourceName, 128 );

                // Refresh the property view.
                Doc.m_pPropEditor->Refresh();
                return 1;
            }
        }
        
        for( k = 0; k < ParticleEventCount; k++ )
        {
            // If it matches copy the Resource name over.
            if( !x_strcmp( (xfs( "%s\\Particle[%d]", Event.m_EventName, k)), (LPCTSTR)m_CurrentPropertyName ) )
            {
                x_strncpy( Event.m_EventParticleList[ k ].m_Name, (LPCTSTR)m_CurrentResourceName, 128 );

                // Refresh the property view.
                Doc.m_pPropEditor->Refresh();
                return 1;
            }
        }
    }

    return 1;
}

//=========================================================================

void CEventEditorFrame::OnNewSound ( void )
{
    CEventEditorDoc&    Doc = *GetEventEditorDoc();
    s32 EventIndex = FindEventIndex();
    
    if( EventIndex == -1 )
        return;

    // Get a new type from the selected event in the property view.
    Doc.m_EventEditor.AddAudioEvent( EventIndex );
    Doc.m_pPropEditor->Refresh();
}

//=========================================================================

void CEventEditorFrame::OnNewParticle ( void )
{
    CEventEditorDoc&    Doc = *GetEventEditorDoc();
    s32 EventIndex = FindEventIndex();
    
    if( EventIndex == -1 )
        return;

    // Get a new type from the selected event in the property view.
    Doc.m_EventEditor.AddParticleEvent( EventIndex );
    Doc.m_pPropEditor->Refresh();
}

//=========================================================================

CEventEditorDoc* CEventEditorFrame::GetEventEditorDoc()
{
    // Since the event_editor resides in EventEditorDoc we have that doc wether its active or not.
    CEventTabView* pView = (CEventTabView*)m_wndWrkspBar.GetView(RUNTIME_CLASS(CEventTabView));
    if (pView)
    {
        return pView->GetDocument();
    }
    x_throw( "The Global Event View is corrupted " );
    return NULL;
}

//=========================================================================

s32 CEventEditorFrame::FindEventIndex( void )
{
    // Get the event editor doc and get how many event there are.
    CEventEditorDoc&    Doc = *GetEventEditorDoc();
    s32 EventCount          = Doc.m_EventEditor.m_pDesc->m_EventList.GetCount();
    s32 i = 0;

    // Check which event is selected.
    for( i = 0; i < EventCount; i++ )
    {
        if( !(x_strcmp((LPCTSTR)m_CurrentPropertyName, Doc.m_EventEditor.m_pDesc->m_EventList[i].m_EventName)) )
            return i;
    }

    return -1;
}

//=========================================================================

void CEventEditorFrame::OnDelete ( void )
{
    CEventEditorDoc&    Doc = *GetEventEditorDoc();

    s32 i = 0;
    s32 j = 0;
    s32 k = 0;
    s32 AudioEventCount = 0;
    s32 ParticleEventCount = 0;
    
    // Check if we have anything to begin with.
    if( Doc.m_EventEditor.m_pDesc->m_EventList.GetCount() <= 0 )
        return;
    
    // Start with the first event.
    s32 EventCount          = Doc.m_EventEditor.m_pDesc->m_EventList.GetCount();
    
    // Go through all the particles and sound list for each event.
    for( i = 0; i < EventCount; i++ )
    {
        event_description::event& Event = Doc.m_EventEditor.m_pDesc->m_EventList[ i ];
        AudioEventCount                 = Doc.m_EventEditor.m_pDesc->m_EventList[ i ].m_EventAudioList.GetCount();
        ParticleEventCount              = Doc.m_EventEditor.m_pDesc->m_EventList[ i ].m_EventParticleList.GetCount();

        // Check the base event if that is going to be the one that is deleted.
        if( !x_strcmp( Event.m_EventName, (LPCTSTR)m_CurrentPropertyName ) )
        {
            Doc.m_EventEditor.m_pDesc->m_EventList.Delete( i );

            // Refresh the property view.
            Doc.m_pPropEditor->Refresh();
            return;
        }

        for( j = 0; j < AudioEventCount; j++ )
        {
            // If it matches then that the one we have to delete.
            if( !x_strcmp( (xfs( "%s\\Sound[%d]", Event.m_EventName, j)), (LPCTSTR)m_CurrentPropertyName ) )
            {
                Event.m_EventAudioList.Delete( j );

                // Refresh the property view.
                Doc.m_pPropEditor->Refresh();
                return;
            }
        }
        
        for( k = 0; k < ParticleEventCount; k++ )
        {
            // If it matches then that the one we have to delete.
            if( !x_strcmp( (xfs( "%s\\Particle[%d]", Event.m_EventName, k)), (LPCTSTR)m_CurrentPropertyName ) )
            {
                Event.m_EventParticleList.Delete( k );

                // Refresh the property view.
                Doc.m_pPropEditor->Refresh();
                return;
            }
        }
    }

    return;
}

//=========================================================================

void CEventEditorFrame::OnLoadSndPkg   ( void )
{
    // TODO.

}

//=========================================================================

void CEventEditorFrame::OnLoadGlbEvents( void )
{

}

//=========================================================================

void CEventEditorFrame::OnSaveGlbEvents( void )
{
    // Since the event_editor resides in EventEditorDoc we have that doc wether its active or not.
    CEventTabView* pView = (CEventTabView*)m_wndWrkspBar.GetView(RUNTIME_CLASS(CEventTabView));

//    pView->GetTreeCtrl();
}

//=========================================================================

void CEventEditorFrame::OnSavePackage ( void )
{
    CEventEditorDoc&    Doc = *GetEventEditorDoc();
    
    x_try;

    Doc.m_EventEditor.Save();

    x_catch_display;
}

//=========================================================================
void CEventEditorFrame::OnNewPackage ( void )
{
    CEventEditorDoc&    Doc = *GetEventEditorDoc();
    Doc.m_EventEditor.New();
}

//=========================================================================

void CEventEditorFrame::OnLoadPackage  ( void )
{
    CEventEditorDoc&    Doc = *GetEventEditorDoc();
    
    x_try;

    CFileDialog Fileopendialog( TRUE, NULL, NULL, OFN_ALLOWMULTISELECT | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT );

    if( Fileopendialog.DoModal() == IDOK )
    {
        CString String = Fileopendialog.GetPathName();
        Doc.m_EventEditor.Load( (LPCTSTR)String );
    }

    x_catch_display;
}

//=========================================================================