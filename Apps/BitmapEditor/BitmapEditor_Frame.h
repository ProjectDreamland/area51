#if !defined(AFX_BitmapEDITOR_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_)
#define AFX_BitmapEDITOR_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BitmapEditor.h : header file
//
#include "BaseFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CBitmapEditor_Frame frame
class CPropertyEditorDoc;
class CBitmapEditor_Doc;

class CBitmapEditor_Frame : public CBaseFrame
{
public:
    CBitmapEditor_Doc& GetDoc( void ) { return *((CBitmapEditor_Doc*)GetActiveDocument()); }

    CXTTabCtrlBar       m_TabCtrl;
    CPropertyEditorDoc* m_pPropEditor;
    CXTToolBar          m_wndToolBar;
    CBitmapEditor_Doc*    m_pDoc;
    xbool               m_Init;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
protected:
	virtual ~CBitmapEditor_Frame();
	CBitmapEditor_Frame();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBitmapEditor_Frame)

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBitmapEditor_Frame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CBitmapEditor_Frame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
	afx_msg void OnSaveDesc();
	afx_msg void OnOpen();
	afx_msg void OnNewDesc();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BitmapEDITOR_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_)
