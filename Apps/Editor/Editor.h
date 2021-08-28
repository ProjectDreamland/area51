// Editor.h : main header file for the EDITOR application
//

#if !defined(AFX_EDITOR_H__CF20484E_9B0F_41AE_B7D9_27F4A267717F__INCLUDED_)
#define AFX_EDITOR_H__CF20484E_9B0F_41AE_B7D9_27F4A267717F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols
#include "DisableScreenSave.h"

/////////////////////////////////////////////////////////////////////////////
// CEditorApp:
// See Editor.cpp for the implementation of this class
//

class CEditorApp : public CWinApp
{
public:
    void InitGame       ( void );
    void SaveSettings   ( void );
    void LoadSettings   ( void );

protected:

    char* m_pSettingFile;
    char* m_pConfigFile ;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
public:
	 CEditorApp();
    ~CEditorApp();

    void InstallExceptionHandler( void );

/////////////////////////////////////////////////////////////////////////////
public:
    // ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditorApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
public:
	//{{AFX_MSG(CEditorApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    CDisableScreenSaver m_MonitorToDisableScreenSaver;
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITOR_H__CF20484E_9B0F_41AE_B7D9_27F4A267717F__INCLUDED_)
