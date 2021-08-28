#if !defined(AFX_FXEDITORVIEW3D_H__9E5CF21E_3768_4C5B_ACF7_BC686F36BE0D__INCLUDED_)
#define AFX_FXEDITORVIEW3D_H__9E5CF21E_3768_4C5B_ACF7_BC686F36BE0D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FXEditorView3D.h : header file
//

#include "EditorView3D.h"

/////////////////////////////////////////////////////////////////////////////
// CFXEditorView3D view

class CFXEditorView3D : public CEditorView3D
{
    protected:
	    CFXEditorView3D();           // protected constructor used by dynamic creation
	    DECLARE_DYNCREATE(CFXEditorView3D)

    //-------------------------------------------------------------------------
    // General Functions
    //-------------------------------------------------------------------------
    public:


    //-------------------------------------------------------------------------
    // Navigation
    //-------------------------------------------------------------------------
    protected:
        enum NavMode
        {
            NAV_NONE,
            NAV_PAN,
            NAV_FLY,
            NAV_LOOK,
            NAV_ORBIT,
            NAV_ORBIT_SELECTED,
            NAV_ZOOM,
            NAV_ZOOM_WHEEL,
            NAV_ZOOM_REGION
        };

        NavMode         m_NavMode;
        ViewportCamera  m_Camera;
        xbool           m_DrawZoomMarquee;

        void        UpdateView      ( void );
        void        SetNavigateMode ( void );
        NavMode     GetNavMode      ( void );






    //-------------------------------------------------------------------------
    // MFC Stuff below
    //-------------------------------------------------------------------------
    // Overrides
	    // ClassWizard generated virtual function overrides
	    //{{AFX_VIRTUAL(CFXEditorView3D)
	    protected:
	    virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	    //}}AFX_VIRTUAL

    // Implementation
    protected:
	    virtual ~CFXEditorView3D();
    #ifdef _DEBUG
	    virtual void AssertValid() const;
	    virtual void Dump(CDumpContext& dc) const;
    #endif

	    // Generated message map functions
    protected:
	    //{{AFX_MSG(CFXEditorView3D)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	//}}AFX_MSG
	    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FXEDITORVIEW3D_H__9E5CF21E_3768_4C5B_ACF7_BC686F36BE0D__INCLUDED_)
