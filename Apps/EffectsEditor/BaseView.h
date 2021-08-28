#if !defined(AFX_BASEVIEW_H__18486051_967E_44E5_93AC_4E1A4D4E9FCD__INCLUDED_)
#define AFX_BASEVIEW_H__18486051_967E_44E5_93AC_4E1A4D4E9FCD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CBaseView
/////////////////////////////////////////////////////////////////////////////
class CBaseView : public CView
{
/////////////////////////////////////////////////////////////////////////////
protected:
    
           xbool  IsActionMode       ( void );
           xbool  IsViewActive       ( void ) { return m_bViewActive; }

/////////////////////////////////////////////////////////////////////////////
protected:

    inline xbool   GetMouseLeftButton ( void )  { return m_MouseLeftButton;  }
    inline xbool   GetMouseRightButton( void )  { return m_MouseRightButton; }
    inline s32     GetMouseDeltaX     ( void )  { return m_MouseDeltaX;      }
    inline s32     GetMouseDeltaY     ( void )  { return m_MouseDeltaY;      }
    inline s32     GetMouseX          ( void )  { return m_MouseOldPos.x;    }
    inline s32     GetMouseY          ( void )  { return m_MouseOldPos.y;    }

/////////////////////////////////////////////////////////////////////////////
private:
    xbool            m_bViewActive;


/////////////////////////////////////////////////////////////////////////////
protected:
	DECLARE_DYNCREATE(CBaseView)
	virtual ~CBaseView();
          	 CBaseView();           

/////////////////////////////////////////////////////////////////////////////
protected:
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseView)
	protected:
	virtual void OnDraw(CDC* pDC);      // overridden to draw this view
	virtual void OnActivateView(BOOL bActivate, CView* pActivateView, CView* pDeactiveView);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump       (CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CBaseView)
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnSysKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );		
	afx_msg void OnSysKeyUp( UINT nChar, UINT nRepCnt, UINT nFlags );		
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
private:

    xbool       m_bActionView;

/////////////////////////////////////////////////////////////////////////////
private:

    xbool       m_MouseLeftButton;
    xbool       m_MouseRightButton;
    CPoint      m_MouseOldPos;
    s32         m_MouseDeltaX;
    s32         m_MouseDeltaY;
};

/////////////////////////////////////////////////////////////////////////////
// END
/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASEVIEW_H__18486051_967E_44E5_93AC_4E1A4D4E9FCD__INCLUDED_)
