// xToolView.h : interface of the CxToolView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_XTOOLVIEW_H__20196312_4427_4A11_B158_F90CD8FFA276__INCLUDED_)
#define AFX_XTOOLVIEW_H__20196312_4427_4A11_B158_F90CD8FFA276__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FrameLog.h"
#include "FrameMemory.h"
#include "Frame3D.h"

class CxToolView : public CXTTabView
{
protected: // create from serialization only
	CxToolView();
	DECLARE_DYNCREATE(CxToolView)

// Attributes
public:
	CxToolDoc*      GetDocument();

    CFrameLog       m_wndLog;
    CFrameMemory    m_wndMemory;
    CFrame3D        m_wnd3D;
    CFrameBase      m_wndIO;
    CFrameBase      m_wndStats;
    CFrameBase      m_wndTweaks;
    CFrameBase      m_wndProfile;
    CFrameBase      m_wndScreenshot;

// Operations
public:
    virtual void ActivateView( CWnd* pTabView );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CxToolView)
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CxToolView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CxToolView)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnLogExportCsv();
	afx_msg void OnFileImportMapfile();
	afx_msg void OnMemlogExportCsv();
	afx_msg void OnMemlogExportActiveCsv();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in xToolView.cpp
inline CxToolDoc* CxToolView::GetDocument()
   { return (CxToolDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_XTOOLVIEW_H__20196312_4427_4A11_B158_F90CD8FFA276__INCLUDED_)
