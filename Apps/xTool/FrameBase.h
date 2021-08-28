#if !defined(AFX_FRAMEBASE_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_)
#define AFX_FRAMEBASE_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FrameBase.h : header file
//

extern int g_NextToolBarID;

/////////////////////////////////////////////////////////////////////////////
// CFrameBase window

class CFrameBase : public CXTFrameWnd
{
	DECLARE_DYNCREATE(CFrameBase)

// Construction
public:
	CFrameBase();

// Attributes
public:
    int         m_Shade;
    int         m_ToolBarID;
    CxToolDoc*  m_pDoc;

// Operations
public:

    virtual void PostNcDestroy( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFrameBase)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFrameBase();

    CxToolDoc*  GetDocument( void ) { return m_pDoc; };

	// Generated message map functions
protected:
	//{{AFX_MSG(CFrameBase)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRAMEBASE_H__E697ACA5_6CAD_4C91_8F03_46C8C0794970__INCLUDED_)
