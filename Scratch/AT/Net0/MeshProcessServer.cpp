
#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CMeshProcessTrayApp, CWinApp)
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

CMeshProcessTrayApp::CMeshProcessTrayApp()
{
}

CMeshProcessTrayApp theApp;

BOOL CMeshProcessTrayApp::InitInstance()
{
	InitCommonControls();

	CWinApp::InitInstance();

	SetRegistryKey(_T("MeshProcess"));

	CMeshProcessTrayDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	return FALSE;
}
