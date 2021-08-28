#if !defined(AFX_FxEditor_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_)
#define AFX_FxEditor_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FxEditor.h : header file
//
#include "BaseFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CFxEditor_Frame frame
class CPropertyEditorDoc;
class CFxEditor_Doc;

class CFxEditor_Frame : public CBaseFrame
{
public:
    CFxEditor_Doc& GetDoc( void ) { return *((CFxEditor_Doc*)GetActiveDocument()); }

    CXTTabCtrlBar       m_TabCtrl;
    CPropertyEditorDoc* m_pPropEditor;
    CXTToolBar          m_wndToolBar;
    CFxEditor_Doc*    m_pDoc;
    xbool               m_Init;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
protected:
	virtual ~CFxEditor_Frame();
	CFxEditor_Frame();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CFxEditor_Frame)

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFxEditor_Frame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CFxEditor_Frame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
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

#endif // !defined(AFX_FxEditor_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_)
