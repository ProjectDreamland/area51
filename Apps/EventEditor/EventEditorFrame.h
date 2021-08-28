#if !defined(AFX_EVENTEDITORFRAME_H__84EB1EF6_2EF9_4EF0_81D1_EE5BF7BD2390__INCLUDED_)
#define AFX_EVENTEDITORFRAME_H__84EB1EF6_2EF9_4EF0_81D1_EE5BF7BD2390__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EventEditorFrame.h : header file
//
#include "..\Editor\BaseFrame.h"

/////////////////////////////////////////////////////////////////////////////
// EXTERNAL CLASSES
/////////////////////////////////////////////////////////////////////////////
class CEventEditorDoc;
class CPropertyEditorDoc;
class EDRscDesc_Doc;

//=========================================================================

class CEventEditorFrame : public CBaseFrame
{
public:

    CEventEditorDoc&    GetDoc( void ) { return *((CEventEditorDoc*)GetActiveDocument()); }
    CEventEditorDoc*    GetEventEditorDoc();
    s32                 FindEventIndex( void );
protected:
    
    CXTTabCtrlBar       m_wndWrkspBar;
    CXTToolBar		    m_wndToolBar;
    CXTTabCtrlBar       m_wndProperty;
    
	CImageList          m_imageList;
    CPropertyEditorDoc* m_pPropEditor;
    EDRscDesc_Doc*      m_pResourceEditor;
    CString             m_CurrentPropertyName;
    CString             m_CurrentResourceName;
    xbool               m_OnInit;

    void OnNewSound     ( void );
    void OnNewParticle  ( void );
    void OnDelete       ( void );
    void OnLoadSndPkg   ( void );
    void OnLoadGlbEvents( void );
    void OnSaveGlbEvents( void );
    void OnSavePackage  ( void );
    void OnNewPackage   ( void );
    void OnLoadPackage  ( void );

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
protected:
	virtual ~CEventEditorFrame();
	CEventEditorFrame();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CEventEditorFrame)

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEventEditorFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void ActivateFrame(int nCmdShow);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:
	// Generated message map functions
	//{{AFX_MSG(CEventEditorFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnPropertyEditorSelChange(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnResourceEditorLbuttonDblClk(WPARAM wParam, LPARAM lParam);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EVENTEDITORFRAME_H__84EB1EF6_2EF9_4EF0_81D1_EE5BF7BD2390__INCLUDED_)
