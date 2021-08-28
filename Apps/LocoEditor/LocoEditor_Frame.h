#if !defined(AFX_LOCOEDITOR_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_)
#define AFX_LOCOEDITOR_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LocoEditor.h : header file
//
#include "BaseFrame.h"

/////////////////////////////////////////////////////////////////////////////
// CLocoEditor_Frame frame
class CPropertyEditorDoc;
class CLocoEditor_Doc;

class CLocoEditor_Frame : public CBaseFrame
{
public:
    CLocoEditor_Doc& GetDoc( void ) { return *((CLocoEditor_Doc*)GetActiveDocument()); }

    CXTTabCtrlBar       m_TabCtrl;
    CPropertyEditorDoc* m_pPropEditor;
    CXTToolBar          m_wndToolBar;
    CLocoEditor_Doc*    m_pDoc;
    xbool               m_Init;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
protected:
	virtual ~CLocoEditor_Frame();
	CLocoEditor_Frame();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CLocoEditor_Frame)

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLocoEditor_Frame)
	public:
	virtual void ActivateFrame(int nCmdShow = -1);
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CLocoEditor_Frame)
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

#endif // !defined(AFX_LOCOEDITOR_H__5D2484F1_9747_4728_A5C8_4A246A003F20__INCLUDED_)
