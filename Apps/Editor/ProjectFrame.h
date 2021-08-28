#if !defined(AFX_PROJECTFRAME_H__7F3B792C_7026_42F2_B74D_D983BD3E2FDA__INCLUDED_)
#define AFX_PROJECTFRAME_H__7F3B792C_7026_42F2_B74D_D983BD3E2FDA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProjectFrame.h : header file
//
#include "BaseFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CProjectFrame frame
class CPropertyEditorDoc;
class CProjectDoc;
class CProjectFrame : public CBaseFrame
{
public:

    void SetProject(CString strProject);
    BOOL CanEdit();

protected:

    CXTTabCtrlBar       m_TabCtrl;
    CProjectDoc*        m_pDoc;
    CPropertyEditorDoc* m_pProjectProp;
    CPropertyEditorDoc* m_pSettingsProp;
    CString             m_strCurrentProject;
    CXTToolBar          m_wndToolBar;

/////////////////////////////////////////////////////////////////////////////
// MFC STUFF
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
public:
	DECLARE_DYNCREATE(CProjectFrame)
	CProjectFrame();           // protected constructor used by dynamic creation
	virtual ~CProjectFrame();

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectFrame)
	//}}AFX_VIRTUAL

// Implementation
/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CProjectFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd);
	afx_msg void OnClose();
	afx_msg void OnProjCreateNewTheme();
	afx_msg void OnInsertTheme();
	afx_msg void OnRemoveTheme();
	afx_msg void OnUpdateProjCreateNewTheme(CCmdUI* pCmdUI);
	afx_msg void OnUpdateInsertTheme(CCmdUI* pCmdUI);
	afx_msg void OnUpdateRemoveTheme(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTFRAME_H__7F3B792C_7026_42F2_B74D_D983BD3E2FDA__INCLUDED_)
