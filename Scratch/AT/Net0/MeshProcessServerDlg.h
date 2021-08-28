
#define WM_TRAY_ICON_NOTIFY_MESSAGE (WM_USER + 1)

#define UPGRADE_TIMER_ID	100
#define UPGRADE_TIMER_MS	300000		// 5 minutes
//#define UPGRADE_TIMER_MS	5000		// 5 minutes

#define SERVER_TIMER_ID		101
#define SERVER_TIMER_MS		20

#define OUTPUT_TIMER_ID		102
#define OUTPUT_TIMER_MS		500

// -----------------------------------------------------------------
// CMeshProcessTrayDlg
// -----------------------------------------------------------------

class CMeshProcessTrayDlg : public CDialog
{
public:
	CMeshProcessTrayDlg(CWnd* pParent = NULL);

	enum { IDD = IDD_MESHPROCESSSERVER_DIALOG };
	HICON m_hIcon;

	NOTIFYICONDATA IconData;
	bool bIsInTray;					// If true, the app is minimized to the tray
	char TrayToolTip[64];			// Tooltip for the tray icon
	CString	CommandLine;
	CString	OurFilename;

	void CheckForUpgrade();
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTrayNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnTimer( UINT nIDEvent );

	afx_msg void OnOpen();
	afx_msg void OnExit();

	DECLARE_MESSAGE_MAP()
	CListBox OutputList;
};

// -----------------------------------------------------------------
// CAboutDlg
// -----------------------------------------------------------------

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

protected:
	DECLARE_MESSAGE_MAP()
};

