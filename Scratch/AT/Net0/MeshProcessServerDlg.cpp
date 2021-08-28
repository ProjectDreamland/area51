
#include "stdafx.h"

#include "..\Core\DistributedProcessing.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
// Server functions
//

//	getNumCPUs

uint32 getNumCPUs()
{
	SYSTEM_INFO	SystemInfo;
	GetSystemInfo(&SystemInfo);
	return SystemInfo.dwNumberOfProcessors;
}

//	processServer

struct processServer
{
private:

	uint32	NumThreads;

	//	remoteDataConnection

	struct remoteDataConnection: connection
	{
	protected:

		processServer*	Server;

		rwMutex			DataMutex;
		processData*	Data;

		friend struct processingThread;

		struct processingThread: thread
		{
		private:

			remoteDataConnection*	Connection;

		public:

			job*	Job;

			processingThread(remoteDataConnection* InConnection,job* InJob):
				Connection(InConnection),
				Job(InJob)
			{}

			~processingThread()
			{
				delete Job;
			}

			virtual uint32 threadProc()
			{
				Connection->DataMutex.readLock();
				Job->execute(Connection->Data);
				Connection->DataMutex.readUnlock();

				return 0;
			}
		};

		array<processingThread*>	Threads;

	public:

		remoteDataConnection(processServer* InServer,dataSocket* InSocket):
			connection(InSocket),
			Server(InServer),
			Data(NULL)
		{}

		virtual ~remoteDataConnection()
		{
			while(Threads.num())
			{
				for(uint32 ThreadIndex = 0;ThreadIndex < Threads.num();ThreadIndex++)
				{
					if(!Threads[ThreadIndex]->isRunning())
					{
						delete Threads[ThreadIndex];
						Threads = Threads.remove(ThreadIndex--);
					}
				}

				Sleep(1000);
			};

			if(Data)
			{
				delete Data;
				Data = NULL;
			}
		}

		virtual void processMessage(messageReader& Reader)
		{
			switch(Reader.MessageType)
			{
				case msgProcessData:
				{
					DataMutex.writeLock();
					Data = new processData();
					Data->serialize(Reader);
					DataMutex.writeUnlock();

					for(uint32 ThreadIndex = 0;ThreadIndex < Server->NumThreads;ThreadIndex++)
						sendMessage(messageWriter(msgRequestJob).Message);

					break;
				}
				case msgJob:
				{
					job*	Job = new job;
					Reader << *Job;

					processingThread*	Thread = new processingThread(this,Job);
					Threads *= Thread;
					Thread->start();

					break;
				}
				default:
					connection::processMessage(Reader);
					break;
			};
		}

		void tick()
		{
			connection::tick();

			for(uint32 ThreadIndex = 0;ThreadIndex < Threads.num();ThreadIndex++)
			{
				if(!Threads[ThreadIndex]->isRunning())
				{
					messageWriter	Writer(msgJobResult);
					Writer << *Threads[ThreadIndex]->Job;
					sendMessage(Writer.Message);

					sendMessage(messageWriter(msgRequestJob).Message);

					delete Threads[ThreadIndex];
					Threads = Threads.remove(ThreadIndex--);
				}
			}
		}
	};

	udpDataSocket*			ListenSocket;
	remoteDataConnection*	DataConnection;

public:

	processServer():
		DataConnection(NULL),
		ListenSocket(NULL)
	{
		// Determine the number of threads to use.

		NumThreads = getNumCPUs();
		cout << "Processing server using " << NumThreads << " threads.\n";
	}

	~processServer()
	{
		if(DataConnection)
		{
			delete DataConnection;
			DataConnection = NULL;
		}

		if(ListenSocket)
		{
			delete ListenSocket;
			ListenSocket = NULL;
		}
	}

	void tick()
	{
		if(DataConnection)
		{
			if(ListenSocket)
			{
				delete ListenSocket;
				ListenSocket = NULL;
			}

			if(DataConnection->closed())
			{
				cout << "Disconnected from data server.\n";
				delete DataConnection;
				DataConnection = NULL;
			}
			else
				DataConnection->tick();
		}
		else
		{
			if(!ListenSocket)
				ListenSocket = udpDataSocket::bind(ipAddress(ipAddress::Any,HELP_PORT));

			while(ListenSocket->recvDataQueued())
			{
				ipAddress	SourceAddress;
				uint16		DataPort;
				if(ListenSocket->recv(SourceAddress,&DataPort,sizeof(uint16)))
				{
					cout << "Helping " << stringToC<char>(ipAddress(SourceAddress.Address,DataPort).describe()).Data << "\n";
					try
					{
						DataConnection = new remoteDataConnection(this,dataSocket::connect(ipAddress(SourceAddress.Address,DataPort)));
					}
					catch(...)
					{
						// If the connection fails, continue.
					}
					break;
				}
			};
		}
	}
};

FILE* GNewStdOut = NULL;
processServer* GProcessServer = NULL;

void StartServer()
{
	HANDLE	ProcessHandle = GetCurrentProcess();
	SetPriorityClass(ProcessHandle,IDLE_PRIORITY_CLASS);
	CloseHandle(ProcessHandle);

	GNewStdOut = freopen( "c:\\stdout.txt", "w", stdout );

	GProcessServer = new processServer;
}

void TickServer()
{
	GProcessServer->tick();

	fflush( GNewStdOut );
}

void StopServer()
{
	delete GProcessServer;

	fclose( GNewStdOut );
}

// -----------------------------------------------------------------
// CMeshProcessTrayDlg
// -----------------------------------------------------------------

BEGIN_MESSAGE_MAP(CMeshProcessTrayDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_TRAY_ICON_NOTIFY_MESSAGE,OnTrayNotify)
	ON_COMMAND(ID_CONTEXT_OPEN,OnOpen)
	ON_COMMAND(ID_CONTEXT_EXIT,OnExit)
END_MESSAGE_MAP()

CMeshProcessTrayDlg::CMeshProcessTrayDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMeshProcessTrayDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMeshProcessTrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDLB_OUTPUT, OutputList);
}

BOOL CMeshProcessTrayDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set up the tray notifications

	::strcpy( TrayToolTip, "Mesh Processing Server" );

	::ZeroMemory( &IconData, sizeof(NOTIFYICONDATA) );
	IconData.cbSize				= sizeof(NOTIFYICONDATA);
	IconData.hWnd				= m_hWnd;
	IconData.uID				= 1;
	IconData.uCallbackMessage	= WM_TRAY_ICON_NOTIFY_MESSAGE;
	IconData.hIcon				= m_hIcon;
	::strcpy( IconData.szTip, TrayToolTip );
	IconData.uFlags				= NIF_MESSAGE | NIF_ICON | NIF_TIP;

	Shell_NotifyIcon( NIM_ADD, &IconData );

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// Command line

	CommandLine = GetCommandLine();
	OurFilename = "";
	bool	Quote = false;
	for(int CharacterIndex = 0;CharacterIndex < CommandLine.GetLength();CharacterIndex++)
	{
		CString::XCHAR	C = CommandLine.GetAt(CharacterIndex);
		if(C == '\"')
			Quote = !Quote;
		else if(!Quote && (C == '\t' || C == ' '))
			break;
		else
			OurFilename.AppendChar(C);
	}

	StartServer();
	SetTimer( SERVER_TIMER_ID, SERVER_TIMER_MS, NULL );
	SetTimer( UPGRADE_TIMER_ID, UPGRADE_TIMER_MS, NULL );
	SetTimer( OUTPUT_TIMER_ID, OUTPUT_TIMER_MS, NULL );

	ShowWindow( SW_SHOW );
	PostMessage( WM_SYSCOMMAND, SC_MINIMIZE, 0 );

	return TRUE;
}

void CMeshProcessTrayDlg::CheckForUpgrade()
{
	if(!strstr(CommandLine,"noupgrade"))
	{
		FILE* VersionFile = fopen("\\\\server\\WarfareDev\\DistributedMeshProcess\\LatestServerVersion.txt","rt");
		if(VersionFile)
		{
			char	UpgradeFilename[MAX_PATH];
			UpgradeFilename[ fread(UpgradeFilename,sizeof(char),MAX_PATH,VersionFile) ] = 0;
			fclose(VersionFile);

			if( OurFilename.CompareNoCase( UpgradeFilename ) )
			{
				PostQuitMessage(0);
				ShellExecute( NULL, "open", UpgradeFilename, "", "", 2 );
			}
		}
	}
}

void CMeshProcessTrayDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	Shell_NotifyIcon( NIM_DELETE, &IconData );

	KillTimer( SERVER_TIMER_ID );

	StopServer();
}

void CMeshProcessTrayDlg::OnTimer( UINT nIDEvent )
{
	switch( nIDEvent )
	{
		case OUTPUT_TIMER_ID:
		{
			CString line, final;

			try
			{
				CStdioFile file( "c:\\stdout.txt", CFile::modeRead | CFile::shareDenyNone );

				// Skip any lines that are already in the list box

				int PreRead = OutputList.GetCount();
				while( PreRead )
				{
					file.ReadString( line );
					PreRead--;
				}

				// Add any new lines into the control

				while( file.ReadString( line ) )
				{
					OutputList.AddString( line );
				}

				file.Close();
			}
			catch(...)
			{
			}
		}
		break;

		case UPGRADE_TIMER_ID:
			CheckForUpgrade();
		break;

		case SERVER_TIMER_ID:
			try
			{
				TickServer();
			}
			catch(string Error)
			{
				MessageBox(stringToC<char>(Error+"\nRestarting server...").Data,"MeshProcessServer Error");
				PostQuitMessage(0);
				ShellExecute( NULL, "open", OurFilename.GetBuffer(0), "", "", 2 );
			}
			catch(...)
			{
				MessageBox("Unknown error\nRestarting server...","MeshProcessServer Error");
				PostQuitMessage(0);
				ShellExecute( NULL, "open", OurFilename.GetBuffer(0), "", "", 2 );
			}
	break;
	}
}

void CMeshProcessTrayDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	UINT id = nID & 0xFFF0;

	switch( id )
	{
		case SC_MINIMIZE:
			this->ShowWindow(SW_HIDE);
		break;

		case IDM_ABOUTBOX:
		{
			CAboutDlg dlgAbout;
			dlgAbout.DoModal();
		}
		break;

		default:
			CDialog::OnSysCommand(nID, lParam);
		break;
	}
}

LRESULT CMeshProcessTrayDlg::OnTrayNotify(WPARAM wParam, LPARAM lParam) 
{ 
	UINT uID;
	UINT uMsg;

	uID = (UINT)wParam; 
	uMsg = (UINT)lParam; 

	if( uID != 1 )
		return 1;

	CPoint pt;	

	switch( uMsg )
	{ 
		case WM_LBUTTONDBLCLK:
			ShowWindow( 1 );
		break;

		case WM_RBUTTONDOWN:
		{
			CPoint pt;
			GetCursorPos( &pt );

			CMenu menu;
			VERIFY(menu.LoadMenu(IDMN_CONTEXT));
			CMenu* Popup = menu.GetSubMenu(0);
			Popup->TrackPopupMenu( TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, this );
		}
		break;
	} 

	return 0;
} 

void CMeshProcessTrayDlg::OnOpen()
{
	ShowWindow( 1 );
}

void CMeshProcessTrayDlg::OnExit()
{
	EndDialog(IDOK);
}

// -----------------------------------------------------------------
// CAboutDlg
// -----------------------------------------------------------------

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}
