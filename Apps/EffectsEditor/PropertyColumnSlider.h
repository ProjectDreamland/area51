#if !defined(AFX_MYSLIDERCTL_H__8B85C506_15A8_4386_B319_2D678DB72506__INCLUDED_)
#define AFX_MYSLIDERCTL_H__8B85C506_15A8_4386_B319_2D678DB72506__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MySliderCtl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPropertyColumnSlider window


class CPropertyColumnSlider : public CSliderCtrl
{
// Construction
public:
	CPropertyColumnSlider();

// Attributes
private:
	BOOL		 m_bDragging;
	HCURSOR		 m_hCursor;
// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyColumnSlider)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPropertyColumnSlider();

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyColumnSlider)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYSLIDERCTL_H__8B85C506_15A8_4386_B319_2D678DB72506__INCLUDED_)
