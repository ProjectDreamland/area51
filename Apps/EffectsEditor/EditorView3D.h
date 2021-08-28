#if !defined(AFX_EDITORVIEW3D_H__91333801_83E6_431B_A269_382985C69ED9__INCLUDED_)
#define AFX_EDITORVIEW3D_H__91333801_83E6_431B_A269_382985C69ED9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// EditorView3D.h : header file
//

#include "grid3d.hpp"       /*********** NEED TO REWORK THIS *************/
#include "SelectGizmo.hpp"  /*********** NEED TO REWORK THIS *************/
#include "MouseMgr.h"
#include "ViewportCamera.hpp"

/////////////////////////////////////////////////////////////////////////////
// CEditorView3D view

class CEditorView3D : public CView
{
    protected:
	    CEditorView3D();           // protected constructor used by dynamic creation
	    DECLARE_DYNCREATE(CEditorView3D)

    //-------------------------------------------------------------------------
    // General Functions
    //-------------------------------------------------------------------------
    public:
        xbool   IsViewActive        ( void )        { return m_IsViewActive; }
        
    //-------------------------------------------------------------------------
    // Display Values
    //-------------------------------------------------------------------------
    protected:
        enum ShadingMode
        {
            SHADING_WIRE,
            SHADING_SHADED,
            SHADING_SHADED_AND_WIRE,
        };

        ShadingMode     m_ShadingMode;

        // Local Display Options (Unique for each viewport)
        xbool           m_HiliteSelected;
        xbool           m_ShowBackground;
        xbool           m_ShowGrid;
        xbool           m_ShowTrajectories;
        xbool           m_ShowEffectBounds;
        xbool           m_ShowElementBounds;

        // Global Display Options (Shared for all viewports)
        static  xbool   m_ShowTransformGizmos;
        static  xbool   m_ShowEmitterIcons;

    //-------------------------------------------------------------------------
    // UI
    //-------------------------------------------------------------------------
    protected:
        MouseMgr        m_MouseMgr;

        view            m_View;
        xbool           m_IsViewActive;

        s32             m_Width;
        s32             m_Height;

        grid3d          m_Grid;

        CMenu           m_PopupMenu;
        xbool           m_IsPopupMenuActive;

        xbool           m_DrawSelectMarquee;

    //-------------------------------------------------------------------------
    // Drawing Functions
    //-------------------------------------------------------------------------
    protected:
        void    PreRenderSetup      ( void );

        void    DrawMarquee         ( CDC* pDC, CPoint pt1, CPoint pt2 );

    //-------------------------------------------------------------------------
    // Internal Utilities
    //-------------------------------------------------------------------------
    protected:




    //-------------------------------------------------------------------------
    // MFC Stuff below
    //-------------------------------------------------------------------------
    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CEditorView3D)
	protected:
	    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

    // Implementation
    protected:
	    virtual ~CEditorView3D();
    #ifdef _DEBUG
	    virtual void AssertValid() const;
	    virtual void Dump(CDumpContext& dc) const;
    #endif

	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CEditorView3D)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EDITORVIEW3D_H__91333801_83E6_431B_A269_382985C69ED9__INCLUDED_)
