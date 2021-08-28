#if !defined(AFX_MESHVIEWER_VIEW_H__B0974EC0_45DB_43E9_B117_286826E7CB26__INCLUDED_)
#define AFX_MESHVIEWER_VIEW_H__B0974EC0_45DB_43E9_B117_286826E7CB26__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MeshViewer_View.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////////////////////////
#include "..\Editor\View3d.h"
#include "..\Editor\Grid3d.hpp"
#include "..\Editor\Axis3d.hpp"
#include "meshViewer_Doc.h"

class CMeshViewer_Frame;

/////////////////////////////////////////////////////////////////////////////
// CLASS
/////////////////////////////////////////////////////////////////////////////
class CMeshViewer_View : public CView3D
{
/////////////////////////////////////////////////////////////////////////////
public:

    void    SetOrbitCam( void );
    mesh_viewer&        GetViewer( void ) { return ((CMeshViewer_Doc*)GetDocument())->m_Viewer; }
    CMeshViewer_Frame*  GetFrame() { return m_pFrameEdit; }

/////////////////////////////////////////////////////////////////////////////
protected:

    grid3d      m_Grid;
    axis3d      m_Axis;
    CMeshViewer_Frame* m_pFrameEdit;

/////////////////////////////////////////////////////////////////////////////
// MFC 
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
protected:
	virtual ~CMeshViewer_View();
	CMeshViewer_View();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMeshViewer_View)

/////////////////////////////////////////////////////////////////////////////
public:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshViewer_View)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnActivateFrame( UINT nState, CFrameWnd* pFrameWnd );
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	// Generated message map functions
	//{{AFX_MSG(CMeshViewer_View)
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESHVIEWER_VIEW_H__B0974EC0_45DB_43E9_B117_286826E7CB26__INCLUDED_)
