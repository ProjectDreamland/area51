#if !defined(AFX_MESHVIEWER_FRAME_H__CF7E254D_BCB2_4890_9581_FDA03740051E__INCLUDED_)
#define AFX_MESHVIEWER_FRAME_H__CF7E254D_BCB2_4890_9581_FDA03740051E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// DEPENDENCIES
/////////////////////////////////////////////////////////////////////////////

#include "meshViewer_Doc.h"
#include "..\Editor\BaseFrame.h"

class mesh_viewer;
class CMeshViewer_View;

/////////////////////////////////////////////////////////////////////////////

class CMeshViewer_Frame : public CBaseFrame
{
/////////////////////////////////////////////////////////////////////////////
public:

    CXTToolBar          m_wndToolBar;
    CXTTabCtrlBar       m_wndWrkspBar;
    CMeshViewer_Doc*    m_pDoc;

    void RefreshThemeRsc();

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshViewer_Frame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    virtual void ActivateFrame(int nCmdShow);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
// Implementation
public:
	virtual ~CMeshViewer_Frame();
	CMeshViewer_Frame();
	DECLARE_DYNCREATE(CMeshViewer_Frame)

/////////////////////////////////////////////////////////////////////////////
public:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
// Generated message map functions
protected:
	//{{AFX_MSG(CMeshViewer_Frame)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnOrbitcam();
	afx_msg void OnFreeflyCamera();
	afx_msg void OnLoadMesh();
	afx_msg void OnPlayAnim();
	afx_msg void OnPauseAnim();
	afx_msg void OnMvAddMatxAsSkinGeom();
	afx_msg void OnMvAddMatxAsRigidGeom();
	afx_msg void OnUpdateMvAddMatxAsRigidGeom(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMvAddMatxAsSkinGeom(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMvOpenTheme(CCmdUI* pCmdUI);
	afx_msg void OnUpdateOrbitcam(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFreeflyCamera(CCmdUI* pCmdUI);
	afx_msg void OnAnimInPlace();
	afx_msg void OnUpdateAnimInPlace(CCmdUI* pCmdUI);
	afx_msg void OnRenderSkeleton();
	afx_msg void OnUpdateRenderSkeleton(CCmdUI* pCmdUI);
	afx_msg void OnTakeToBind();
	afx_msg void OnUpdateTakeToBind(CCmdUI* pCmdUI);
	afx_msg void OnRenderBoneLavels();
	afx_msg void OnUpdateRenderBoneLavels(CCmdUI* pCmdUI);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

protected:
    void OnMvAddMatxAsRes(CString strType, BOOL bIncludeCollision);

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESHVIEWER_FRAME_H__CF7E254D_BCB2_4890_9581_FDA03740051E__INCLUDED_)
