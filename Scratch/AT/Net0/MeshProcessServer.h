
#pragma once

class CMeshProcessTrayApp : public CWinApp
{
public:
	CMeshProcessTrayApp();

	public:
	virtual BOOL InitInstance();

	// Parse the command line for stock options and commands.
	void ParseCommandLine(CCommandLineInfo& rCmdInfo);

	DECLARE_MESSAGE_MAP()
};

extern CMeshProcessTrayApp theApp;