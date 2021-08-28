// Editor.cpp : Defines the class behaviors for the application.
//


#include "BaseStdAfx.h"
#include "Editor.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "..\WorldEditor\EditorDoc.h"
#include "..\WorldEditor\EditorView.h"
#include "..\WorldEditor\EditorFrame.h"
#include "RscView.h"
#include "Project.hpp"
#include "Lighting.hpp"

//////////////////////////////////////////////////////////////////////////////
// GAME RELATED INCLUDES
//////////////////////////////////////////////////////////////////////////////
#include "Entropy.hpp"
#include "obj_mgr\obj_mgr.hpp"
#include "Render\Render.hpp"
#include "ProjectView.h"
#include "ProjectFrame.h"
#include "ProjectDoc.h"
#include "Gamelib\Link.hpp"
#include "IOManager\io_mgr.hpp"
#include "AudioMgr\AudioMgr.hpp"
#include "..\..\Support\Tracers\TracerMgr.hpp"
#include "PhysicsMgr\PhysicsMgr.hpp"
#include "Font\font.hpp"
#include "Music_Mgr\Music_Mgr.hpp"
#include "ConversationMgr\ConversationMgr.hpp"
#include "Objects\Render\PostEffectMgr.hpp"
#include "GameTextMgr\GameTextMgr.hpp"
#include "AI\AIMgr.hpp"
#include "Decals\DecalMgr.hpp"
#include "SMTP\smtp.h"

#include "..\WorldEditor\nav_connection2_editor.hpp"
#include "..\WorldEditor\nav_connection2_anchor.hpp"

/////////////////////////////////////////////////////////////////////////////
// MUST FORCE TO LINK WITH CERTAIN FILES
/////////////////////////////////////////////////////////////////////////////
#define FORCELINK( SYMBOL )                                             \
void SYMBOL( void );                                                    \
static struct SYMBOL##_force_link                                       \
{ SYMBOL##_force_link(void){ SYMBOL(); } } s_##SYMBOL##_ForceLink;      \

FORCELINK( LinkBitmapEditor );
FORCELINK( LinkLocoEditor );
FORCELINK( LinkResourceEditor );
//FORCELINK( LinkEventEditor );
FORCELINK( LinkAudioEditor );
FORCELINK( LinkMeshViewer );


/////////////////////////////////////////////////////////////////////////////
// MFC STUFF
/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CEditorApp

BEGIN_MESSAGE_MAP(CEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CEditorApp)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// GLOBAL VARIABLES
/////////////////////////////////////////////////////////////////////////////

CEditorApp theApp;

char *g_CommandLine = NULL;

xbool       g_MirrorWeapon   = FALSE;

extern s32 g_Changelist;
extern const char* g_pBuildDate;

/////////////////////////////////////////////////////////////////////////////
// FUNCTIONS
/////////////////////////////////////////////////////////////////////////////

//===========================================================================
// MiniDump
//===========================================================================

#include <dbghelp.h>

typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

bool FileExists( const char* pPath )
{
    FILE* pFile = fopen( pPath, "rb" );
    if( pFile )
        fclose( pFile );
    return pFile != NULL;
}

bool PathExists( const char* pPath )
{
    DWORD Attributes = GetFileAttributes( pPath );
    if( Attributes != INVALID_FILE_ATTRIBUTES )
    {
        return true;
    }
    else
    {
        return false;
    }
}

BOOL MyCreateDirectory( const char* pPath )
{
    DWORD Attributes = GetFileAttributes( pPath );
    if( Attributes != INVALID_FILE_ATTRIBUTES )
    {
        // Is it a directory?
        return( Attributes & FILE_ATTRIBUTE_DIRECTORY ) ? true : false;
    }
    else
    {
        // Try to create it
        return CreateDirectory( pPath, NULL );
    }
}

PVOID   g_pHandler = NULL;
bool    g_HandlerTrap = false;
CString g_MiniDumpFile;
CString g_MiniDumpFolder;

#pragma comment(linker, "/defaultlib:dbghelp.lib")

LONG WINAPI Handler( PEXCEPTION_POINTERS pExceptionInfo )
{
    // Handle these exceptions
    bool ForceHandle = false;
    switch( pExceptionInfo->ExceptionRecord->ExceptionCode )
    {
    case EXCEPTION_ACCESS_VIOLATION:
    case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
    case EXCEPTION_DATATYPE_MISALIGNMENT:
    case EXCEPTION_FLT_DENORMAL_OPERAND:
    case EXCEPTION_FLT_DIVIDE_BY_ZERO:
    case EXCEPTION_FLT_INEXACT_RESULT:
    case EXCEPTION_FLT_INVALID_OPERATION:
    case EXCEPTION_FLT_OVERFLOW:
    case EXCEPTION_FLT_STACK_CHECK:
    case EXCEPTION_FLT_UNDERFLOW:
    case EXCEPTION_ILLEGAL_INSTRUCTION:
    case EXCEPTION_IN_PAGE_ERROR:
    case EXCEPTION_INT_DIVIDE_BY_ZERO:
    case EXCEPTION_INT_OVERFLOW:
    case EXCEPTION_INVALID_DISPOSITION:
    case EXCEPTION_NONCONTINUABLE_EXCEPTION:
    case EXCEPTION_PRIV_INSTRUCTION:
        //EXCEPTION_SINGLE_STEP
    case EXCEPTION_STACK_OVERFLOW:
        g_HandlerTrap = true;
        ForceHandle = true;
        ;
    }

    if( (g_HandlerTrap && (pExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)) || ForceHandle )
    {
	    // firstly see if dbghelp.dll is around and has the function we need
	    // look next to the EXE first, as the one in System32 might be old 
	    // (e.g. Windows 2000)
	    HMODULE hDll = NULL;
        CString ExePath;
        CString ExeFileName;
        CString PDBFileName;

        // Get the version of DBGHELP that in the .exe directory
        char szExePath[_MAX_PATH];
	    if (GetModuleFileName( NULL, szExePath, _MAX_PATH ))
	    {
            ExeFileName = szExePath;
            PDBFileName = ExeFileName;
            PDBFileName.Replace( ".exe", ".pdb" );

		    char *pSlash = _tcsrchr( szExePath, '\\' );
		    if (pSlash)
		    {
                *(pSlash+1) = 0;
                ExePath = szExePath;
                CString DllPath = ExePath + "DBGHELP.DLL";
			    hDll = ::LoadLibrary( DllPath );
		    }
	    }

        // Failed to load from the .exe directory so try to generic version on DBGHELP
	    if (hDll==NULL)
	    {
		    // load any version we can
		    hDll = ::LoadLibrary( "DBGHELP.DLL" );
	    }

        if( hDll )
        {
            MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		    if (pDump)
		    {
                bool AtInevitable = true;
                CString Prefix = "t:\\EditorCrash\\";
                if( !MyCreateDirectory( Prefix ) )
                {
                    Prefix = "c:\\EditorCrash\\";
                    MyCreateDirectory( Prefix );
                    AtInevitable = false;
                }

                {
                    // Get the Date as a string
                    CTime Now = CTime::GetCurrentTime();
                    CString DateString = Now.Format( "%Y-%m-%d" );
                    CString TimeString = Now.Format( "%H-%M-%S" );

                    // Create the dump file name
                    CString sFile;
                    s32 i=0;
                    do
                    {
                        Prefix = AtInevitable ? "t:\\EditorCrash\\" : "c:\\EditorCrash\\";
                        Prefix += DateString;
                        MyCreateDirectory( Prefix );

                        Prefix += "\\";
                        Prefix += TimeString;
                        if( i != 0 )
                            Prefix.AppendFormat( "_%d", i );
                    } while( PathExists( Prefix ) );

                    MyCreateDirectory( Prefix );

                    sFile = Prefix;
                    sFile += "\\";
                    sFile += "Editor.dmp";

                    // Create the file and write the minidump to the file
                    HANDLE hFile = CreateFile( sFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
                    if( hFile )
                    {
                        MINIDUMP_EXCEPTION_INFORMATION eInfo;
                        eInfo.ThreadId = GetCurrentThreadId();
                        eInfo.ExceptionPointers = pExceptionInfo;
                        eInfo.ClientPointers = FALSE;

                        //MINIDUMP_CALLBACK_INFORMATION cbMiniDump;
                        //cbMiniDump.CallbackRoutine = CExceptionReport::miniDumpCallback;
                        //cbMiniDump.CallbackParam = 0;

                        MiniDumpWriteDump(
                            GetCurrentProcess(),
                            GetCurrentProcessId(),
                            hFile,
                            MiniDumpWithDataSegs,
                            &eInfo,
                            NULL,
                            NULL ); //&cbMiniDump);

                        // Copy .exe and .pdb to the minidump directory
                        if( ExeFileName.GetLength() > 0 )
                        {
                            CopyFile( ExeFileName, Prefix + "\\Editor.exe", FALSE );
                            CopyFile( PDBFileName, Prefix + "\\Editor.pdb", FALSE );
                        }
                        else
                        {
                        }
                    }

                    // Close file
                    CloseHandle(hFile);

                    // Save details for later email
                    g_MiniDumpFolder = Prefix;
                    g_MiniDumpFile   = sFile;
                }

                // Reset handler
                g_HandlerTrap = false;

                // Exit and stop or continue execution
                if( ForceHandle )
                {
                    return EXCEPTION_CONTINUE_SEARCH;
                }
                else
                {
                    pExceptionInfo->ContextRecord->Eip += 1;
                    return EXCEPTION_CONTINUE_EXECUTION;
                }
            }
        }
    }
/*
    else if( (pExceptionInfo->ExceptionRecord->ExceptionCode & 0xffff0000) != 0x40010000 )
    {
        char Buffer[1024];
        sprintf( Buffer, "0x%08x - 0x%08x", pExceptionInfo->ExceptionRecord->ExceptionCode, (DWORD)pExceptionInfo->ExceptionRecord->ExceptionAddress );
        ::MessageBox( NULL, Buffer, "Exception", MB_OK );
    }
*/

    g_HandlerTrap = false;
    return EXCEPTION_CONTINUE_SEARCH;
}

//===========================================================================

void EditorRTFMailer( const char* pSubject, const char* pReport )
{
#if defined(autobuild)
    // Only email ASSERTs
    if( x_strcmp( pSubject, "ASSERT" ) == 0 )
    {
        CONTEXT c;
        EXCEPTION_POINTERS ep;
        ZeroMemory( &c, sizeof(c) );
        ZeroMemory( &ep, sizeof(ep) );
        ep.ContextRecord = &c;

        // Get Context
        HANDLE hThread = GetCurrentThread();
        c.ContextFlags = CONTEXT_FULL;
        VERIFY( GetThreadContext( hThread, &c ) );

        // Add the textual report to the head of the report string
        CString Subject;
        CString Body;

        // Read Username from the environment
        char UserName[256];
        GetEnvironmentVariable( "USERNAME", UserName, sizeof(UserName) );

        // Subject = ASSERT - (Build nnnn) (user) (level)
        Subject.Format( "%s - (Build %d) (%s) (%s)", pSubject, g_Changelist, UserName, g_Project.GetName() );

        // Generate the MiniDump
        if( !IsDebuggerPresent() )
        {
            g_HandlerTrap = true;
            g_MiniDumpFolder.Empty();
            g_MiniDumpFile.Empty();
            DebugBreak();
            g_HandlerTrap = false;
        }

        // Add link to MiniDump file
        if( g_MiniDumpFile.GetLength() > 0 )
        {
            Body += "MiniDump file is here: " + g_MiniDumpFolder;
            Body += "\n";
            Body += "\n";
        }

        // Add the low level report
        Body += pReport;
        Body += "\r\n";

        CPJNSMTPConnection smtp;
        smtp.Connect( "hermes.inevitable.com" );
        CSMTPMessage m;
        m.AddRecipient( CSMTPAddress("EditorRTF@inevitable.com") );
        m.m_From        = CSMTPAddress("EditorRTF@inevitable.com");
        m.m_sSubject    = Subject;
        m.AddTextBody( Body );
        smtp.SendMessage( m );
    }
#endif
}

//===========================================================================

extern xbool ExecuteCMD( const xstring& InputStr, xstring& OutputStr );

static void CheckForNewVersion( void )
{
    return;

    char szPath[_MAX_PATH];
    if( GetModuleFileName( NULL, szPath, _MAX_PATH ) )
    {
        xstring Have;
        xstring Latest;

        // Read the version we have
        if( ExecuteCMD( xstring("p4 have ") + szPath, Have ) )
        {
            // Read the latest version in perforce
            if( ExecuteCMD( xstring("p4 files ") + szPath, Latest ) )
            {
                // Find the version number
                s32 iHave   = Have.Find( "#" );
                s32 iLatest = Latest.Find( "#" );

                // Did we find them?
                if( (iHave != -1) && (iLatest != -1) )
                {
                    // Skip the #
                    iHave++;
                    iLatest++;

                    // Compare the 2 numbers
                    while( x_isdigit(Have[iHave]) && x_isdigit(Latest[iLatest]) )
                    {
                        if( Have[iHave] != Latest[iLatest] )
                        {
                            MessageBox( NULL, "There is a new editor available in perforce.\nYou should consider upgrading!", "New Editor Alert", MB_ICONSTOP | MB_OK );
                            break;
                        }
                        iHave++;
                        iLatest++;
                    }
                }
            }
        }
    }
}

CEditorApp::CEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
    m_pConfigFile  = "EditorConfigs.ini" ;
    m_pSettingFile = "EditorSettings.settings";

    LinkMeshViewer();
}

//===========================================================================

void CEditorApp::InstallExceptionHandler( void )
{
    // Setup our exception mailer
    x_SetRTFMailer( EditorRTFMailer );
    if( !IsDebuggerPresent() )
    {
        g_pHandler = AddVectoredExceptionHandler( 1, Handler );
    }
}

//===========================================================================

CEditorApp::~CEditorApp()
{
    if( g_pHandler )
    {
        RemoveVectoredExceptionHandler( g_pHandler );
    }
}

//===========================================================================

void BluePrintAnchor_Link( void );
void NotepadObject_Link( void );
void NavConnectionEditor(void);
void NavNodeEditor(void);
void NavAnchor2_Link();
void NavConn2_Link();
void Turret_Link();
void StaticDecal_Link( void );

//===========================================================================

void CEditorApp::InitGame( void )
{
    //
    // Force all files to be linked
    //
    ForceLink();

    BluePrintAnchor_Link();
    NotepadObject_Link();
    NavConn2_Link();
    NavAnchor2_Link();
    Turret_Link();
    StaticDecal_Link();
    //NavConnectionEditor();
    //NavNodeEditor();

    //
    // Intitialize general systems
    //
    guid_Init();
    g_ObjMgr.Init();
    g_SpatialDBase.Init ( 400.0f );

    // Init data vault and load tweaks
    g_DataVault.Init();
    LoadTweaks( xfs("%s\\PC",g_Settings.GetReleasePath()) );
    LoadPain( xfs("%s\\PC",g_Settings.GetReleasePath()) );

    // Init static anim event stuff
    anim_event::Init();

    // Create permanent objects
    g_ObjMgr.CreateObject("god") ;

    // Initialize the IO system
    g_IoMgr.Init();

    //
    // Initialize the resource system
    //
    g_RscMgr.Init();
    g_RscMgr.SetRootDirectory( xfs("%s\\PC",g_Settings.GetReleasePath()) );
    g_RscMgr.SetOnDemandLoading( TRUE );

    //
    // Initialize the graphics system
    //
    render::Init();
    g_DecalMgr.Init();
    g_TracerMgr.Init();
    g_PostEffectMgr.Init();
    g_Font.Load( xfs( "%s\\%s", g_RscMgr.GetRootDirectory(), "Font_small.xbmp" ) );

    //
    // Initialize the physics manager.
    //
    g_PhysicsMgr.Init();

    //
    // Initialize the audio manager.
    //
    g_AudioManager.Init(8192*1024);

    g_GameTextMgr.Init();
    
    //
    // Initialize the music manager.
    //
//    g_MusicMgr.Init();
    g_ConverseMgr.Init();

    //
    // Initialiaze the lighting system
    //
    lighting_Initialize();
    
    g_AIMgr.Init();

    // Initialize SaveMgr
#ifdef OLD_SAVE
    g_SaveMgr.Init();
#endif
//    poly_Initialize();
}

//===========================================================================
// CEditorApp initialization

BOOL CEditorApp::InitInstance()
{
    // Loads in the settings file.
    LoadSettings();

	if (!AfxSocketInit())
	{
		AfxMessageBox(IDP_SOCKETS_INIT_FAILED);
		return FALSE;
	}

    g_CommandLine = m_lpCmdLine;

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#if _MSC_VER <= 1200 // MFC 6.0 or earlier
#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif
#endif // MFC 6.0 or earlier

	// Change the registry key under which our settings are stored.
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization.
	SetRegistryKey(_T("Inevitable"));

	LoadStdProfileSettings(8);  // Load standard INI file options (including MRU)

	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views.
/*
    m_pRscViewDocTemplate = new CMultiDocTemplate(
		IDR_RSC_VIEW,
		RUNTIME_CLASS(CEditorDoc),
		RUNTIME_CLASS(CChildFrame), // custom MDI child frame
		RUNTIME_CLASS(RscView));
	AddDocTemplate(m_pRscViewDocTemplate);
*/
	CMultiDocTemplate *pWorldEditDocTemplate = new CMultiDocTemplate(
		IDR_EDITORTYPE,
		RUNTIME_CLASS(CEditorDoc),
		RUNTIME_CLASS(CEditorFrame), // custom MDI child frame
		RUNTIME_CLASS(CEditorView));
	AddDocTemplate(pWorldEditDocTemplate);
    
    CMultiDocTemplate *pProjectDocTemplate = new CMultiDocTemplate(
		IDR_PROJECT_VIEW,
		RUNTIME_CLASS(CProjectDoc),
		RUNTIME_CLASS(CProjectFrame), // custom MDI child frame
		RUNTIME_CLASS(CProjectView));
	AddDocTemplate(pProjectDocTemplate);

    //
    // Collect all the known editors
    // 
    {
        reg_editor* pEditor = reg_editor::m_pHead;
        while( pEditor )
        {            
            AddDocTemplate( pEditor->Register( this ) );
            pEditor = pEditor->m_pNext;
        }
    }

	// create main MDI Frame window
	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame->LoadFrame(IDR_MAINFRAME))
		return FALSE;
	m_pMainWnd = pMainFrame;

    InitGame();

    pMainFrame->m_pWorldEditDocTemplate = pWorldEditDocTemplate;
    pMainFrame->m_pProjectDocTemplate = pProjectDocTemplate;

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);

    // Check for a newer version of the editor being available if this editor
    // was built by the autobuild system and there is no command line.
#if defined(autobuild)
    if( g_CommandLine[0] == 0 )
        CheckForNewVersion();
#endif


	// Dispatch commands specified on the command line
    // TOMAS: Comment it out because I wanted to stop asking at the begging for new docs.
//	if (!ProcessShellCommand(cmdInfo))
//		return FALSE;

    //
    // Open a few default documents
    //
    pProjectDocTemplate->OpenDocumentFile(NULL);
    pWorldEditDocTemplate->OpenDocumentFile(NULL);

    //open resource editor by default
    {
        reg_editor* pEditor = reg_editor::FindEditorByName("Resource Editor");
        ASSERT( pEditor );
        pEditor->m_pTemplate->OpenDocumentFile( NULL );
    }


    //set the project as the active view
    POSITION posProjectDoc = pProjectDocTemplate->GetFirstDocPosition( );
    if (posProjectDoc)
    {
        CProjectDoc* pProjectDoc = (CProjectDoc*)pProjectDocTemplate->GetNextDoc( posProjectDoc );
        if (pProjectDoc)
        {
            POSITION posProjectView = pProjectDoc->GetFirstViewPosition();
            if (posProjectView)
            {
                CProjectView* pProjView = (CProjectView*)pProjectDoc->GetNextView(posProjectView);
                pProjView->GetParentFrame()->SetActiveView(pProjView);
            }   
        }
    }

	// The main window has been initialized, so show and update it.
	pMainFrame->ShowWindowEx(m_nCmdShow);
	pMainFrame->UpdateWindow();

    pMainFrame->LoadProjectFromCommandLine();


    //!!
    //!!NOTE:TODO: check the virtual function/overwrite "run" in CWinAPP it may be the place for redraw notify some views
    //!!

    return TRUE;
}


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
		// No message handlers
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// App command to run the dialog
void CEditorApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

/////////////////////////////////////////////////////////////////////////////
// CEditorApp message handlers

void CEditorApp::SaveSettings( void )
{
    x_try;

    g_Settings.OnSave( m_pSettingFile );

    x_catch_display;
}

void CEditorApp::LoadSettings   ( void )
{
    x_try;

    g_Settings.LoadConfigs( m_pConfigFile ) ;
    g_Settings.OnLoad( m_pSettingFile );

    x_catch_display;
}
