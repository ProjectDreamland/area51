#if !defined(AFX_EDRSCDESC_FRAME_H__C2EC18B8_CBAD_4865_9F00_AC09146A7C61__INCLUDED_)
#define AFX_EDRSCDESC_FRAME_H__C2EC18B8_CBAD_4865_9F00_AC09146A7C61__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EDRscDesc_Frame.h : header file
//


#include "EDRscDesc_Doc.h"
#include "BaseFrame.h"

class EDRscDesc_View;
class EDRscDesc_Doc;
class CPropertyEditorDoc;

/////////////////////////////////////////////////////////////////////////////
// EDRscDesc_Frame frame

class EDRscDesc_Frame : public CBaseFrame
{
/////////////////////////////////////////////////////////////////////////////
protected:

    void                OnAddItem   ( int Num );

/////////////////////////////////////////////////////////////////////////////
protected:

    CXTToolBar          m_BuildToolBar;
    CXTToolBar          m_EditToolBar;
    CXTTabCtrlBar       m_TabCtrl;
    CPropertyEditorDoc* m_pPropEditor;
    EDRscDesc_Doc*      m_pDoc;
    xbool               m_Init;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
protected:
	DECLARE_DYNCREATE(EDRscDesc_Frame)
	EDRscDesc_Frame();           // protected constructor used by dynamic creation
	virtual ~EDRscDesc_Frame();

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(EDRscDesc_Frame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

    void         UpdateButtons               ( void );
    void         BuildButtons                ( void );


/////////////////////////////////////////////////////////////////////////////
protected:
	// Generated message map functions
	//{{AFX_MSG(EDRscDesc_Frame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnRscAddResDesc();
    afx_msg void OnPopupAddRscDesc( UINT nID );
	afx_msg void OnRscBuild();
	afx_msg void OnRscBuildStop();
	afx_msg void OnSaveActive();
	afx_msg void OnEditRescdesc();
    afx_msg void OnEditRescdescUpdate( CCmdUI* pCmdUI );
	afx_msg void OnCheckoutRescdesc();
    afx_msg void OnCheckoutRescdescUpdate( CCmdUI* pCmdUI );
	afx_msg void OnCompileNintendo();
	afx_msg void OnCompilePs2();
	afx_msg void OnCompileXbox();
	afx_msg void OnVerboseMode();
    afx_msg void OnColorMips();
	afx_msg void OnCompilePC();
	afx_msg void OnCompileNintendoUpdate( CCmdUI* pCmdUI );
	afx_msg void OnCompilePs2Update( CCmdUI* pCmdUI );
	afx_msg void OnCompileXboxUpdate( CCmdUI* pCmdUI );
	afx_msg void OnVerboseModeUpdate( CCmdUI* pCmdUI );
    afx_msg void OnColorMipsUpdate( CCmdUI* pCmdUI );
	afx_msg void OnCompilePCUpdate( CCmdUI* pCmdUI );
	afx_msg void OnCleanResource();
	afx_msg void OnRefreshViews();
	afx_msg void OnDeleteRscdesc();
	afx_msg void OnRebuildAll();
    afx_msg void OnScanResources();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDRSCDESC_FRAME_H__C2EC18B8_CBAD_4865_9F00_AC09146A7C61__INCLUDED_)
