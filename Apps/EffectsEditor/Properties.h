#if !defined(AFX_PROPERTIES_H__9286603F_56AF_46F6_BF26_8D9A781E6423__INCLUDED_)
#define AFX_PROPERTIES_H__9286603F_56AF_46F6_BF26_8D9A781E6423__INCLUDED_

#include "PropertyGrid.h"
#include "XTDockWindow.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// Properties.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProperties window

#define WM_USER_MSG_PROPERTIES_PROPERTYCHANGED      WM_USER + 2000

class CProperties : public CXTDockWindow
{
// Construction
public:
	CProperties();

// Attributes
public:
    CPropertyGrid   m_PropertyGrid;
    CStringList     m_PropList;

// Operations
public:
    CGridItemInfo*  AddGridDataElement ( CString strName,
                                         CString strValue,
                                         CString strComment,
							             CGridItemInfo::CONTROLTYPE type,
                                         COLORREF fieldColor,
                                         int iXaIndex,
							             BOOL bReadOnly,
                                         BOOL bMustEnum,
                                         BOOL bHeader );

    void ExpandAll          ( void );
    void EraseAll           ( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProperties)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProperties();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProperties)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg LRESULT OnItemChange(WPARAM, LPARAM);
	afx_msg void OnWindowPosChanged(WINDOWPOS FAR* lpwndpos);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTIES_H__9286603F_56AF_46F6_BF26_8D9A781E6423__INCLUDED_)
