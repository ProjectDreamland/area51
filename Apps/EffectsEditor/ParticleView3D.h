#if !defined(AFX_PARTICLEVIEW3D_H__F963D65C_6CC2_4607_8E9F_838FF0277657__INCLUDED_)
#define AFX_PARTICLEVIEW3D_H__F963D65C_6CC2_4607_8E9F_838FF0277657__INCLUDED_

#include "Auxiliary/fx_core/effect.hpp"
#include "View3D.h"
#include "grid3d.hpp"
#include "axis3d.hpp"
#include "MouseState.hpp"
#include "KeySet.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ParticleView3D.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CParticleView3D view

class CParticleView3D : public CView3D
{
    protected:
	    CParticleView3D();           // protected constructor used by dynamic creation
	    DECLARE_DYNCREATE(CParticleView3D)

    // Attributes
    protected:

        grid3d      m_Grid;

        void        CollectKeys             ( fx_core::ctrl_key* pCont, KeySet* pKeySet );

//        xbool       m_GridEnabled;

    // Operations
    public:

        void        SetCameraPos            ( float PosX, float PosY, float PosZ );
        void        SetCameraPos            ( const vector3& Pos );
        void        GetCameraInfo           ( vector3& Pos, quaternion& Quat );
        void        SetCameraInfo           ( const vector3& Pos, const quaternion& Quat );

        void        SetCamera_Top           ( void );
        void        SetCamera_Bottom        ( void );
        void        SetCamera_Left          ( void );
        void        SetCamera_Right         ( void );
        void        SetCamera_Front         ( void );
        void        SetCamera_Back          ( void );
        void        SetCamera_User          ( void );
        void        SetCamera_Perspective   ( void );

        void        UpdateKeyBar            ( void );
    // Attributes
    protected:

        CXTMenu     m_PopupMenu;
        bool        m_IsPopupMenuActive;

        vector3     m_CameraPos;
        vector3     m_TargetPos;


    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CParticleView3D)
	protected:
	    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

    // Implementation
    protected:
	    virtual ~CParticleView3D();
    #ifdef _DEBUG
	    virtual void AssertValid() const;
	    virtual void Dump(CDumpContext& dc) const;
    #endif

	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CParticleView3D)
	    afx_msg void    OnPaint                     ( void );
	    afx_msg void    OnMouseMove                 (UINT nFlags, CPoint point);
	    afx_msg int     OnCreate                    (LPCREATESTRUCT lpCreateStruct);
	    afx_msg void    OnLButtonDown               (UINT nFlags, CPoint point);
	    afx_msg void    OnLButtonUp                 (UINT nFlags, CPoint point);
	    afx_msg BOOL    OnMouseWheel                (UINT nFlags, short zDelta, CPoint pt);
        afx_msg void    OnRButtonDown               (UINT nFlags, CPoint point);
	    afx_msg void    OnMButtonDown               (UINT nFlags, CPoint point);
	    afx_msg void    OnMButtonUp                 (UINT nFlags, CPoint point);
	    afx_msg BOOL    OnSetCursor                 (CWnd* pWnd, UINT nHitTest, UINT message);
	    afx_msg void    OnKeyUp                     (UINT nChar, UINT nRepCnt, UINT nFlags);
	    afx_msg void    OnUpdateViewViewportGrids   (CCmdUI* pCmdUI);
	    afx_msg void    OnUpdateViewBbox            (CCmdUI* pCmdUI);
	    afx_msg void    OnCreateCylinder            ( void );
	    afx_msg void    OnCreateMesh                ( void );
	    afx_msg void    OnCreatePlane               ( void );
	    afx_msg void    OnCreateRibbon              ( void );
	    afx_msg void    OnCreateShockWave           ( void );
	    afx_msg void    OnCreateSpemitter           ( void );
	    afx_msg void    OnCreateSphere              ( void );
	    afx_msg void    OnCreateSprite              ( void );
	    afx_msg void    OnEditSelectByname          ( void );
	    afx_msg void    OnEditClone                 ( void );
	    afx_msg void    OnEditMoveToZero            ( void );
	    afx_msg void    OnViewViewportGrids         ( void );
	    afx_msg void    OnViewRedrawAll             ( void );
	    afx_msg void    OnViewBbox                  ( void );
	    afx_msg void    OnViewTrajectory            ( void );
	    afx_msg void    OnViewVelocity              ( void );
	    afx_msg void    OnViewHideSelected          ( void );
	    afx_msg void    OnViewUnhideAll             ( void );
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTICLEVIEW3D_H__F963D65C_6CC2_4607_8E9F_838FF0277657__INCLUDED_)
