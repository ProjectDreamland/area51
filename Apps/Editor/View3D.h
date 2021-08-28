#if !defined(AFX_VIEW3D1_H__DE135630_86E1_4F5F_93B8_74493D2C3F1F__INCLUDED_)
#define AFX_VIEW3D1_H__DE135630_86E1_4F5F_93B8_74493D2C3F1F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// View3D1.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// INCLUDES
/////////////////////////////////////////////////////////////////////////////
#include "BaseView.h"

/////////////////////////////////////////////////////////////////////////////
// CView3D
/////////////////////////////////////////////////////////////////////////////
class CView3D : public CBaseView
{
/////////////////////////////////////////////////////////////////////////////
public:

    void   CameraRotate             ( s32 MouseDeltaX, s32 MouseDeltaY );
    void   CameraTranslate          ( s32 MouseDeltaX, s32 MouseDeltaY, xbool bVertical );
    void   CameraOrbitMode          ( const vector3& Focus );
    void   CameraOrbitMode          ( void );
    void   CameraFreeFlyMode        ( void );
    void   CameraSetFocus           ( const vector3& Focus );
    xbool  IsActionMode             ( void );
    
    virtual void   SetViewDirty     ( void );

/////////////////////////////////////////////////////////////////////////////
protected:

    enum camera_type
    {
        CAMERA_FREEFLY,
        CAMERA_ORBIT,
    };

/////////////////////////////////////////////////////////////////////////////
protected:


    view            m_View;
    camera_type     m_CamType;
    f32             m_CamRotateSpeed;
    f32             m_CamTranslateSpeed;
    vector3         m_CamFocus;
    xbool           m_bDirtyView;

/////////////////////////////////////////////////////////////////////////////
// MFC Specific stuff
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
protected:
	DECLARE_DYNCREATE(CView3D)
          	 CView3D();           // protected constructor used by dynamic creation
	virtual ~CView3D();

/////////////////////////////////////////////////////////////////////////////
protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CView3D)
	public:
	virtual void OnInitialUpdate();
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	// Generated message map functions
	//{{AFX_MSG(CView3D)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_VIEW3D1_H__DE135630_86E1_4F5F_93B8_74493D2C3F1F__INCLUDED_)
