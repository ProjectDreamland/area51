// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__6A646CDA_8160_43E2_B60F_73C96E117573__INCLUDED_)
#define AFX_MAINFRM_H__6A646CDA_8160_43E2_B60F_73C96E117573__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "OutputBar.h"

class CMainFrame : public CXTMDIFrameWnd
{
public:  // control bar embedded members
	enum ALIGNMENT { left, top, right, bottom };

    void AddResDesc ( int ID );
	void ShowMDITabs();
	void AddLogo();
	void RemoveLogo();
    void AddProgress();
    void RemoveProgress();


	CXTLogoPane      m_wndLogoPane;
	CXTTrayIcon	     m_trayIcon;
	COutputBar       m_wndOutputBar;
	CXTStatusBar     m_wndStatusBar;
//	CXTToolBar       m_wndToolBar;
    CXTMDIWndTab     m_wndMDITabWindow;
    CProgressCtrl*   m_pwndProgCtrl;
    CProgressCtrl*   m_pwndProgCtrl2;
    static CMainFrame* s_pMainFrame;


public:

    CMultiDocTemplate* m_pProjectDocTemplate;    
    CMultiDocTemplate* m_pWorldEditDocTemplate;    

/////////////////////////////////////////////////////////////////////////////
protected:
	CXTWindowPos        m_wndPosition;
    CMenu               m_PopupMenuAddedViews;
    CRecentFileList*    m_pRecentFiles;

public:
	// Overrode CWnd implementation to restore saved window position.
	BOOL ShowWindowEx(int nCmdShow);

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
public:
	CMainFrame();
	DECLARE_DYNAMIC(CMainFrame)
	virtual ~CMainFrame();
    
    void LoadProjectFromCommandLine( void );
    bool DoFileOpen( const char* pFileName = NULL );

protected:
    bool CloseProject( void );

/////////////////////////////////////////////////////////////////////////////
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void OnUpdateFrameTitle(BOOL Nada);
//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
    afx_msg LRESULT UpdateStatusBar(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT ModifyMenuBar(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnTrayIconNotify(WPARAM wParam, LPARAM lParam);
    afx_msg void OnUpdateAdditionalViews(CCmdUI* pCmdUI);
    afx_msg void OnAdditionalView(UINT nID);
	afx_msg void OnMenuFileClose();
	afx_msg void OnUpdateMenuFileClose(CCmdUI* pCmdUI);
	afx_msg void OnMenuFileNew();
	afx_msg void OnUpdateMenuFileNew(CCmdUI* pCmdUI);
	afx_msg void OnMenuFileOpen();
	afx_msg void OnUpdateMenuFileOpen(CCmdUI* pCmdUI);
	afx_msg void OnMenuFileSave();
	afx_msg void OnUpdateMenuFileSave(CCmdUI* pCmdUI);
	afx_msg void OnEditorSaveSettings();
	afx_msg void OnMenuFileExport();
    afx_msg void OnMenuFileExportTo3DMax();
    afx_msg void OnMenuFileBuildDFS();
	afx_msg void OnUpdateMenuFileExport(CCmdUI* pCmdUI);   
	afx_msg void OnMenuFileImport();
	afx_msg void OnUpdateMenuFileImport(CCmdUI* pCmdUI);   
	afx_msg void OnUpdateMruList(CCmdUI* pCmdUI);
	afx_msg BOOL OnMenuFileMru( UINT nID );
    afx_msg void OnViewResetWindows();
    afx_msg void OnUpdateCleanupZones(CCmdUI* pCmdUI);   
    afx_msg void OnCleanupZones();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern xbool g_bAutoLoad;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__6A646CDA_8160_43E2_B60F_73C96E117573__INCLUDED_)
