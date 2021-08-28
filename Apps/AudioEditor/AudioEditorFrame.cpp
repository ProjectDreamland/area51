//=========================================================================
// CAUDIOEDITORFRAME.CPP
//=========================================================================

//=========================================================================
// INCLUDES
//=========================================================================
#include "stdafx.h"
#include "AudioEditorFrame.h"
#include "AudioEditorView.h"
#include "IntensityView.h"
#include "AudioDirectoryView.h"
#include "SoundView.h"
#include "SoundDoc.h"
#include "Resource.h"
#include "FaderDialog.h"
#include "ParamsDialog.h"
#include "ElementParamsDialog.h"
#include "..\Editor\Resource.h"
#include "..\Editor\UserMessage.h"
#include "..\PropertyEditor\PropertyEditorView.h"
#include "..\PropertyEditor\PropertyEditorDoc.h"
#include "..\Editor\Project.hpp"
#include "..\WinControls\StringEntryDlg.h"
#include "..\Support\ResourceMgr\ResourceMgr.hpp"
#include "..\xCore\Entropy\Audio\IAL\IAL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

xarray<xstring> g_AuditionPackages;

enum status_ids
{
    STATUS_STATUS           = 0,
    STATUS_SAMPLE_RATE,
    STATUS_LENGTH,
    STATUS_SELECTION_START,
    STATUS_SELECTION_END,
    STATUS_ZOOM,
    STATUS_END
};

//=========================================================================
// SetStatusText
//=========================================================================
static CString PrettyNumber( int Number )
{
    CString String;

    ASSERT( Number < (INT_MAX/10) );

    // Determine Order of number
    int Order    = 0;
    int Divisor  = 1;
    while( Divisor <= Number )
    {
        Order++;
        Divisor *= 10;
    }
    Divisor /= 10;
    Order--;

    // Generate digits
    while( Order >= 0 )
    {
        int Value = Number / Divisor;
        String += (char)('0'+Value);
        if( Order && ((Order % 3) == 0) )
            String += ',';
        Number -= Value * Divisor;
        Divisor /= 10;
        Order--;
    }

    // Zero if nothing in string
    if( String.GetLength() == 0 )
        String = "0";

    return String;
}

IMPLEMENT_DYNCREATE(CAudioEditorFrame, CBaseFrame)

//=========================================================================
// MESSAGE PUMP
//=========================================================================

BEGIN_MESSAGE_MAP(CAudioEditorFrame, CBaseFrame)
	//{{AFX_MSG_MAP(CAudioEditorFrame)
        ON_WM_TIMER()
        ON_WM_CREATE()
        ON_WM_DESTROY()
	    ON_CBN_SELENDOK( ID_SOUND_UNITS,                    OnSoundUnits                    )
        ON_MESSAGE( WM_USER_MSG_SELECTION_CHANGE, 	        OnPropertyEditorSelChange       )
        ON_MESSAGE( WM_USER_MSG_OPEN_SOUND_FILE,            OnOpenSoundFile                 )
        ON_MESSAGE( WM_USER_MSG_CLEAR_ALL_SOUND_FILE,       OnClearAllSoundFile             )
        ON_MESSAGE( WM_USER_MSG_SET_STATUS_TEXT,            SetStatusText                   )
        ON_MESSAGE( WM_USER_MSG_SET_STATUS_SAMPLE_RATE,     SetStatusSampleRate             )
        ON_MESSAGE( WM_USER_MSG_SET_STATUS_NUM_SAMPLES,     SetStatusNumSamples             )
        ON_MESSAGE( WM_USER_MSG_SET_STATUS_SELECTION_START, SetStatusSelectionStart         )
        ON_MESSAGE( WM_USER_MSG_SET_STATUS_SELECTION_END,   SetStatusSelectionEnd           )
        ON_MESSAGE( WM_USER_MSG_SET_STATUS_ZOOM,            SetStatusZoom                   )
        ON_COMMAND( ID_ADD_ELEMENT,                         OnNewAudioElement               )
        ON_COMMAND( ID_ADD_DESCRIPTOR,                      OnNewAudioDescriptor            )
        ON_COMMAND( ID_ADD_PACKAGE,                         OnNewAudioPackage               )
        ON_COMMAND( ID_REFRENCE_DESCRIPTOR,                 OnRefDescriptor                 )
        ON_COMMAND( ID_SAVE_AUDIO_EDITOR,                   OnSave                          )
        ON_COMMAND( ID_LOAD_AUDIO_EDITOR,                   OnLoad                          )
        ON_COMMAND( ID_SET_AUDIO_SOURCE_PATH,               OnGetSourcePath                 )
        ON_COMMAND( ID_ADD_FADER,                           OnAddFader                      )
        ON_COMMAND( ID_DEFAULT_PARAMS,                      OnChangeDefaultParams           )
        ON_COMMAND( ID_DEFAULT_ELEMENT_PARAMS,              OnChangeDefaultElementParams    )
        ON_COMMAND( ID_SORT_DESCENDING,                     OnSortDescending                )
        ON_COMMAND( ID_SOUND_PLAY,                          OnPlaySound                     )
        ON_COMMAND( ID_SOUND_STOP,                          OnStopSound                     )        

        ON_COMMAND( ID_EDIT_COPY,                           OnCopy                          )
        ON_COMMAND( ID_EDIT_PASTE,                          OnPaste                         )

	    ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=========================================================================

CAudioEditorFrame::CAudioEditorFrame() :
m_OnInit( TRUE )
{
    m_SoundViewCount = 0;
}

//=========================================================================

CAudioEditorFrame::~CAudioEditorFrame()
{
/*
    for( s32 i = 0; i < m_pDoc->m_AudioEditor.m_pDesc.GetCount(); i++ )
    {
        if( m_pDoc->m_AudioEditor.m_pDesc[i].m_PackageLoaded )
        {
            //char FilePath[256];
    
            //sprintf( FilePath, "%s\\PC\\%s", g_Settings.GetReleasePath(), m_pDoc->m_AudioEditor.m_pDesc[ i ].GetName() );
            //g_AudioMgr.UnloadPackage( FilePath );
            
        }
    }
*/
}

//=========================================================================

BOOL CAudioEditorFrame::PreCreateWindow(CREATESTRUCT& cs) 
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs
	if( !CBaseFrame::PreCreateWindow(cs) )
		return FALSE;
	
//	cs.style = WS_CHILD | WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU
//		| FWS_ADDTOTITLE | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
//
//	cs.style |= WS_CLIPCHILDREN;

	// Helps to reduce screen flicker.
	cs.lpszClass = AfxRegisterWndClass(0, NULL, NULL,
		AfxGetApp()->LoadIcon(IDR_SOUNDFRAME));

	return TRUE;
}

//=========================================================================

void CAudioEditorFrame::ActivateFrame(int nCmdShow) 
{
    nCmdShow = SW_SHOWMAXIMIZED;
	CBaseFrame::ActivateFrame(nCmdShow);

    if( m_OnInit )
    {
        m_OnInit = FALSE;
        
        m_pDoc = ((CSoundDoc*)GetActiveDocument());

        // OLD WAY
	    //m_wndWrkspBar.AddView(_T("Package View"), RUNTIME_CLASS(CAudioEditorView), m_pDoc );
        //m_wndWrkspBar.AddView(_T("Intensity View"), RUNTIME_CLASS(CIntensityView), m_pDoc );
        //m_wndSample.AddView(_T("Sample Pool"), RUNTIME_CLASS(CAudioDirectoryView), m_pDoc );

        CFrameWnd* pFrameWnd        = NULL;
        CFrameWnd* pActiveFrameWnd  = NULL;

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
                        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CAudioEditorView), m_pDoc);
                        
        BOOL Bool = m_wndWrkspBar.AddControl(_T("Package View"), pFrameWnd);
        
        // Make the package view the active view.
        pActiveFrameWnd = pFrameWnd;

        pFrameWnd = m_wndWrkspBar.CreateFrameDocView(
                        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CIntensityView), m_pDoc);
        Bool = m_wndWrkspBar.AddControl(_T("Intensity View"), pFrameWnd);

        pFrameWnd = m_wndSample.CreateFrameDocView(
                        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CAudioDirectoryView), m_pDoc);
        m_wndSample.AddControl(_T("Sample Pool"), pFrameWnd);
        
        // Set the active view.
        m_wndWrkspBar.SetActiveView( pActiveFrameWnd );
        m_pDoc->SetCommandHandler( this );
        m_pDoc->m_pPropEditor    = m_pPropEditor;
        m_pDoc->m_pPropEditor->SetInterface( m_pDoc->m_AudioEditor );
    }
}

//=========================================================================

int CAudioEditorFrame::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CBaseFrame::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Create the workspace bar.
	if( !m_wndWrkspBar.Create(this, IDW_WORKSPBAR, _T("Virtual Directory View"),
		CSize(250, 350), CBRS_LEFT, CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //|(AFX_IDW_TOOLBAR + 7) ))
	{
		TRACE0("Failed to create workspace dock window\n");
		return -1;		// fail to create
	}

	// Create the workspace bar.
	if( !m_wndSample.Create(this, IDW_SAMPLE_POOL, _T("SamplePool"),
		CSize(250, 150), CBRS_LEFT, CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //|(AFX_IDW_TOOLBAR + 21) ))
	{
		TRACE0("Failed to create workspace dock window\n");
		return -1;		// fail to create
	}

	// Create the property bar.
	if( !m_wndProperty.Create(this, IDW_PROPERTY_BAR, _T("Property View"),
		CSize(200, 150), CBRS_LEFT, CBRS_XT_BUTTONS | CBRS_XT_GRIPPER | CBRS_XT_CLIENT_STATIC)) //|(AFX_IDW_TOOLBAR + 6) ))
	{
		TRACE0("Failed to create property dock window\n");
		return -1;		// fail to create
	}

    // Create the ToolBar
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_GRIPPER | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0,0,0,0), IDW_TOOLBAR_SOUND) ||
		!m_wndToolBar.LoadToolBar(IDR_SOUND))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndToolBar.SetWindowText( _T("Sound Toolbar") );


    // Create m_wndPackageToolBar ToolBar
	if (!m_wndPackageToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_GRIPPER | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC, CRect(0,0,0,0), AFX_IDW_TOOLBAR + 15) ||
		!m_wndPackageToolBar.LoadToolBar(IDR_PACKAGE))
	{
		TRACE0("Failed to create toolbar\n");
		return -1;      // fail to create
	}
	m_wndPackageToolBar.SetWindowText( _T("Package Toolbar") );

	// Create the combo box control to be inserted into the toolbar
	if( !m_wndComboUnits.Create( WS_CHILD|WS_VISIBLE|CBS_DROPDOWNLIST|WS_CLIPCHILDREN,
		                         CRect(0,0,80,64), &m_wndToolBar, ID_SOUND_UNITS ) )
	{
		TRACE0( "Failed to create combo box.\n" );
		return -1;      // fail to create
	}

    m_wndToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);
    m_wndPackageToolBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);

	m_wndSample.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndWrkspBar.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);
	m_wndProperty.EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT|CBRS_XT_GRIPPER_GRAD);

	EnableDockingEx(CBRS_ALIGN_ANY, CBRS_XT_ALL_FLAT);

    DockControlBar(&m_wndToolBar,AFX_IDW_DOCKBAR_TOP);
    DockControlBarLeftOf(&m_wndPackageToolBar,&m_wndToolBar);

	DockControlBar(&m_wndSample,AFX_IDW_DOCKBAR_LEFT);
	DockControlBar(&m_wndProperty,AFX_IDW_DOCKBAR_LEFT);
	DockControlBarLeftOf(&m_wndWrkspBar, &m_wndSample);

	// Create the image list used with the tab control bar.
	if (!m_imageList.Create(IDB_IMGLIST_VIEW, 16, 1, RGB(0x00,0xff,0x00)))
	{
		TRACE0("Failed to create image list.\n");
		return -1;
	}

	// Associate the image list with the tab control bar.
	m_wndWrkspBar.SetTabImageList(&m_imageList);

    m_pPropEditor = new CPropertyEditorDoc;
    if( m_pPropEditor == NULL )
        x_throw( "Out of memory" );

// OLD WAY
// 	m_wndProperty.AddView(_T("Property"), RUNTIME_CLASS(CPropertyEditorView), m_pPropEditor );

    CFrameWnd* pFrameWnd = NULL;
    pFrameWnd = m_wndProperty.CreateFrameDocView(
        RUNTIME_CLASS(CFrameWnd), RUNTIME_CLASS(CPropertyEditorView), m_pPropEditor);
    m_wndProperty.AddControl(_T("Properties"), pFrameWnd);
    m_pPropEditor->GetView()->LoadColumnState( "BarState - Audio Properties" );

    if (m_pPropEditor) m_pPropEditor->SetCommandHandler(this);

    // Insert into statusbar & Add values
    m_wndToolBar.InsertControl( &m_wndComboUnits );
    
    s32 ComboIndex = m_wndComboUnits.AddString( _T("Seconds") );
    m_wndComboUnits.SetItemData( ComboIndex, (DWORD)CSoundDoc::SECONDS );

    m_wndComboUnits.SetCurSel ( 0 );

    // Clear out the worl status bar.
    CWnd *pWnd = ::AfxGetMainWnd();
    pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_MODE,0);
    pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_DATA,0);

    m_nTimer = SetTimer(1, 30, NULL);

    // Start the audio timer.
    m_AudioDeltaTime.Reset();
    m_AudioDeltaTime.Start();
    m_LastTime = m_AudioDeltaTime.ReadSec();

    // Load control bar postion.
    LoadBarState(_T("BarState - Audio"));

    return 0;
}

//=========================================================================

void CAudioEditorFrame::OnDestroy( )
{
    // Save control bar postion.
    SaveBarState(_T("BarState - Audio"));
    m_pPropEditor->GetView()->SaveColumnState( "BarState - Audio Properties" );
}

//=========================================================================

LRESULT CAudioEditorFrame::SetStatusText(WPARAM wParam, LPARAM lParam)
{
    //CString String = ((char*)wParam);
    //m_wndStatusBar.SetPaneText( STATUS_STATUS, String );

    return 1;
}

//=========================================================================

LRESULT CAudioEditorFrame::SetStatusSampleRate(WPARAM wParam, LPARAM lParam)
{
    m_StatusSampleRate = PrettyNumber( (s32)wParam ) + " Hz";
    CString String = m_StatusSampleRate + "\t" + m_StatusNumSamples;
//    m_wndStatusBar.SetPaneText( STATUS_SAMPLE_RATE, String );
    CWnd *pWnd = ::AfxGetMainWnd();
    pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_MODE,(long)((LPCTSTR)String));

    return 1;
}

//=========================================================================

LRESULT CAudioEditorFrame::SetStatusNumSamples(WPARAM wParam, LPARAM lParam)
{
    m_StatusNumSamples = PrettyNumber( (s32)wParam );
    CString String = m_StatusSampleRate + "\t" + m_StatusNumSamples;
//    m_wndStatusBar.SetPaneText( STATUS_LENGTH, String );
    CWnd *pWnd = ::AfxGetMainWnd();
    pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_MODE,(long)((LPCTSTR)String));

    return 1;
}

//=========================================================================

LRESULT CAudioEditorFrame::SetStatusSelectionStart(WPARAM wParam, LPARAM lParam)
{
    m_StatusSelStart = PrettyNumber( (s32)wParam );
    CString String = m_StatusSelStart + "\t" + m_StatusSelEnd + "\t" + m_StatusZoom;
//    m_wndStatusBar.SetPaneText( STATUS_SELECTION_START, String );
    CWnd *pWnd = ::AfxGetMainWnd();
    pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_DATA,(long)((LPCTSTR)String));

    return 1;
}

//=========================================================================

LRESULT CAudioEditorFrame::SetStatusSelectionEnd(WPARAM wParam, LPARAM lParam)
{
    m_StatusSelEnd = PrettyNumber( (s32)wParam );
    CString String = m_StatusSelStart + "\t" + m_StatusSelEnd + "\t" + m_StatusZoom;
//    m_wndStatusBar.SetPaneText( STATUS_SELECTION_END, String );
    CWnd *pWnd = ::AfxGetMainWnd();
    pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_DATA,(long)((LPCTSTR)String));

    return 1;
}

//=========================================================================

LRESULT CAudioEditorFrame::SetStatusZoom(WPARAM wParam, LPARAM lParam)
{   
    m_StatusZoom = PrettyNumber( (s32)wParam ) + "x";
    CString String = m_StatusSelStart + "\t" + m_StatusSelEnd + "\t" + m_StatusZoom;
//    m_wndStatusBar.SetPaneText( STATUS_ZOOM, String );
    CWnd *pWnd = ::AfxGetMainWnd();
    pWnd->SendMessage(WM_USER_MSG_UPDATE_STATUS_BAR,ID_INDICATOR_DATA,(long)((LPCTSTR)String));

    return 1;
}

//=========================================================================

LRESULT CAudioEditorFrame::OnOpenSoundFile(WPARAM wParam, LPARAM lParam)
{            

    CString PathName = ((char*)wParam);
    m_pDoc->LoadSample( (LPCTSTR)PathName);

    return 1;
}

//=========================================================================

LRESULT CAudioEditorFrame::OnClearAllSoundFile(WPARAM wParam, LPARAM lParam)
{            

    m_pDoc->ClearAllSamples();
        
    return 1;
}

//=========================================================================

void CAudioEditorFrame::OnNewAudioPackage      ( void )
{
    m_pDoc->OnNewAudioPackage();
}

//=========================================================================

void CAudioEditorFrame::OnNewAudioDescriptor   ( void )
{
    m_pDoc->OnNewAudioDescriptor();
}

//=========================================================================

void CAudioEditorFrame::OnNewAudioElement ( void )
{
    m_pDoc->OnAddAudioElement();
}

//=========================================================================

LRESULT CAudioEditorFrame::OnPropertyEditorSelChange(WPARAM wParam, LPARAM lParam)
{
    m_CurrentPropertyName = ((char*)wParam);
    return 1;
}

//=========================================================================

void CAudioEditorFrame::OnRefDescriptor ( void )
{
    m_pDoc->RefrenceDescriptor( );
}

//================================================================

void CAudioEditorFrame::OnSave ( void )
{
    m_pDoc->OnProjectSave();
}

//================================================================

void CAudioEditorFrame::OnLoad ( void )
{
    m_pDoc->OnLoadFaderList();
}

//================================================================

void CAudioEditorFrame::OnGetSourcePath ( void )
{
    CStringEntryDlg DlgBox;
    DlgBox.SetDisplayText( "Please enter the path for the audio source directory" );
    DlgBox.SetEntryText( m_pDoc->m_AudioSourcePath );
    if( DlgBox.DoModal() == IDOK )
    {
        m_pDoc->SetAudioSourcePath( DlgBox.GetEntryText() );
    }
}

//================================================================

void CAudioEditorFrame::OnAddFader ( void )
{
    CFaderDialog Fader;

    Fader.DoModal();

    m_pDoc->m_pPropEditor->Refresh();
}

//================================================================

void CAudioEditorFrame::OnChangeDefaultParams  ( void )
{
    //CParamsDialog ParamsDialog;

    //ParamsDialog.DoModal();
    m_pDoc->m_AudioEditor.ClearParamsMode();
    //m_pDoc->m_pPropEditor->ClearGrid();
    m_pDoc->m_AudioEditor.SetDefaultParamsMode( TRUE );
    m_pDoc->m_pPropEditor->Refresh();
}

//================================================================

void CAudioEditorFrame::OnChangeDefaultElementParams  ( void )
{
    //CElementParamsDialog ParamsDialog;

    //ParamsDialog.DoModal();
    m_pDoc->m_AudioEditor.ClearParamsMode();
    //m_pDoc->m_pPropEditor->ClearGrid();
    m_pDoc->m_AudioEditor.SetDefaultElementParamsMode( TRUE );
    m_pDoc->m_pPropEditor->Refresh();

}

//================================================================

void CAudioEditorFrame::OnSortDescending( void )
{
    m_pDoc->OnSortDescending();
}

//================================================================

void CAudioEditorFrame::OnCopy( void )
{
    m_pDoc->OnCopy();
}

//================================================================

void CAudioEditorFrame::OnPaste( void )
{
    m_pDoc->OnPaste();
}

//================================================================

xbool g_IncludeInAudioBudget = TRUE;

void CAudioEditorFrame::OnPlaySound ( void )
{
    x_try;
    
    OnStopSound();
   
    if( m_wndSample.IsTabControlActive() )
    {
        hChannel = IAL_allocate_channel();

        sound_file SoundFile;
        if( SoundFile.Load( m_pDoc->GetRecentAudioSampleName() ) == FALSE )
            x_throw( xfs("Unable to Load sample [%s]", (const char*)m_pDoc->GetRecentAudioSampleName() ) );

        
        void*   pData       = (void*)SoundFile.GetChannelData( 0 );
        s32     nSamples    = SoundFile.GetNumSamples();
        s32     SampleRate  = SoundFile.GetSampleRate();

        xbool   bLoop       = SoundFile.IsLooped();
        s32     LoopStart   = SoundFile.GetLoopStart();
        s32     LoopEnd     = SoundFile.GetLoopEnd();

        IAL_init_channel( hChannel, pData, nSamples, bLoop, LoopStart, LoopEnd, IAL_PCM, SampleRate, 1.0f, 0.0f, 1.0f );
        IAL_start_channel( hChannel );

    }
    else if( m_wndWrkspBar.IsTabControlActive() )
    {
    
        if( g_RescDescMGR.IsCompiling() )
            x_throw( "Please wait to finish comipiling before playing this sample." );

        s32 PackageSelected = m_pDoc->m_AudioEditor.m_PackageSelected;
    
        // Is the package already loaded.
        if( m_pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_PackageLoaded == FALSE )
        {
            m_pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_PackageLoaded = TRUE;
            g_IncludeInAudioBudget = FALSE;
            g_RscMgr.Load( m_pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].GetName() );
            g_AuditionPackages.Append( m_pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].GetName() );
            g_IncludeInAudioBudget = TRUE;
        }
    
        // If the package is loaded then play the sound.
        if( m_pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_PackageLoaded == TRUE )
        {
            s32 DescriptorSelected = m_pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_DescriptorSelected;

            xbool Playing = g_AudioMgr.Play( m_pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_pDescriptorList[ DescriptorSelected ].m_Label );

            if( !Playing )
                x_DebugMsg( "Failed to Play %s\n", m_pDoc->m_AudioEditor.m_pDesc[ PackageSelected ].m_pDescriptorList[ DescriptorSelected ].m_Label );
        }
        else
        {
            x_throw( "Please compile the package before playing the sample." );
        }
    }

    x_catch_display;

    g_IncludeInAudioBudget = TRUE;
}

//=========================================================================

void CAudioEditorFrame::OnStopSound ( void )
{
    if( hChannel )
    {
        IAL_stop_channel    ( hChannel );
        IAL_release_channel ( hChannel );

        hChannel = NULL;
    }

    // Stop playing everything.
    g_AudioMgr.ReleaseAll();
}

//=========================================================================

void CAudioEditorFrame::OnTimer(UINT nIDEvent) 
{
    // Compute the duration of the last frame
    f32 CurrentTime = m_AudioDeltaTime.ReadSec();
    f32 DeltaTime   = MAX( CurrentTime - m_LastTime, 0.001f );
    m_LastTime      = CurrentTime;

    if( hChannel )
    {
        ial_state State = IAL_channel_status( hChannel );
        if( (State == IAL_DONE) || (State == IAL_STOP) )
        {
            IAL_stop_channel    ( hChannel );
            IAL_release_channel ( hChannel );

            hChannel = NULL;
        }
    }

    // Update the audio manager.
    g_AudioMgr.Update( DeltaTime );
    
	CBaseFrame::OnTimer(nIDEvent);
}

//=========================================================================

void CAudioEditorFrame::OnClose() 
{
    // If the timer was started then the windows timer was created.
    if( m_AudioDeltaTime.IsRunning() )
        KillTimer(m_nTimer);   
	
	CBaseFrame::OnClose();
}

//=========================================================================

void CAudioEditorFrame::OnSoundUnits ( void )
{
    m_pDoc->m_SoundUnits = (s32)m_wndComboUnits.GetItemData( m_wndComboUnits.GetCurSel() );
    m_pDoc->RefreshSampleView();
}

//=========================================================================
