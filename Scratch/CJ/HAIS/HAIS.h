// HAIS.h : main header file for the HAIS application
//
#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols


// CHAISApp:
// See HAIS.cpp for the implementation of this class
//

class CHAISApp : public CWinApp
{
public:
	CHAISApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CHAISApp theApp;