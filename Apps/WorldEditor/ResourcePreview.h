#if !defined(AFX_RESOURCEPREVIEW_H__1B56CD8A_429D_4A57_880A_52C9741D689D__INCLUDED_)
#define AFX_RESOURCEPREVIEW_H__1B56CD8A_429D_4A57_880A_52C9741D689D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ResourcePreview.h : header file
//
#include "..\Editor\View3d.h"
#include "..\Editor\Grid3d.hpp"
#include "..\Editor\Axis3d.hpp"
#include "..\MeshViewer\MeshViewer.hpp"

/////////////////////////////////////////////////////////////////////////////
// CResourcePreview window

class CResourcePreview : public CWnd
{
// Construction
public:
	CResourcePreview();

public:
    void ClearGeom();   
    BOOL LoadGeom(CString strType, CString strPath, BOOL bAdditional = FALSE, const vector3& Pos = vector3(0,0,0), const radian3& Rot = radian3(0,0,0));
    void OnStartTimer();
    void OnStopTimer();
    void SetMultiplier(const vector3& v3Mult) { m_v3Mult = v3Mult; }

    // Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CResourcePreview)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CResourcePreview();

	// Generated message map functions
protected:
	//{{AFX_MSG(CResourcePreview)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

    view            m_View;
    grid3d          m_Grid;
    axis3d          m_Axis;
    mesh_viewer     m_Viewer;
    UINT            m_nTimer;
    vector3         m_v3Mult;
    f32             m_LastMaxY;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RESOURCEPREVIEW_H__1B56CD8A_429D_4A57_880A_52C9741D689D__INCLUDED_)
