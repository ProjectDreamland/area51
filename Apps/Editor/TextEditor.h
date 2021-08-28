#if !defined(AFX_CTextEditor_H__941E8674_3A4D_4483_9142_D57B135D4463__INCLUDED_)
#define AFX_CTextEditor_H__941E8674_3A4D_4483_9142_D57B135D4463__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProjectView.h : header file
//
class CProjectDoc;

#include "MyRichEditCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CTextEditor view
template<class BASE_CLASS>
class CNotifyCombo : public BASE_CLASS
{
public:
	
	CNotifyCombo()
	{
	}
	
	virtual ~CNotifyCombo()
	{
	}

	void NotifyOwner(UINT nCode)
	{
		NMHDR nm;
		nm.hwndFrom = m_hWnd;
		nm.idFrom = GetDlgCtrlID();
		nm.code = nCode;

		CWnd* pWndOwner = GetOwner();
		if ( pWndOwner )
		{
			pWndOwner->SendMessage( WM_NOTIFY, nm.idFrom, ( LPARAM )&nm );
		}
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		if ( pMsg->message == WM_KEYDOWN )
		{
			switch (pMsg->wParam)
			{
			case VK_ESCAPE:
			case VK_RETURN:
				if ( GetDroppedState( ) == TRUE )
				{
					ShowDropDown( false );
				}
				NotifyOwner( NM_RETURN );
				return TRUE;
				
			case VK_UP:
			case VK_DOWN:
				if ( ( ::GetKeyState( VK_MENU ) >= 0 ) &&
					( ::GetKeyState( VK_CONTROL ) >= 0 ) && 
					( GetDroppedState( ) == FALSE ) )
				{
					ShowDropDown( true );
					return TRUE;
				}
			}
		}
		
		return BASE_CLASS::PreTranslateMessage(pMsg);
	}
};

class CTextEditor : public CFrameWnd
{
protected:
	CTextEditor();           // protected constructor used by dynamic creation

	DECLARE_DYNCREATE(CTextEditor)

    void FileOpen   ( CString filename );
    void FileSave   ( CString filename );
    void ImportText ( CString filename );

    CMyRichEditCtrl                m_rtf;
    CXTToolBar                     m_wndToolBar;
    CString                        m_strFilename;

	CNotifyCombo <CXTFontCombo>    m_wndComboFont;
	CNotifyCombo <CXTFlatComboBox> m_wndComboSize;
	CString                        m_strFontSize;
	CString                        m_strFontName;

    bool InitComboFont();
    bool InitComboSize();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextEditor)
	protected:
	virtual void OnDraw(CDC* pDC);
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~CTextEditor();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CTextEditor)
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnEditUndo();
    afx_msg void OnEditRedo();
    afx_msg void OnEditUndoEnable(CCmdUI* pCmdUI);
    afx_msg void OnEditRedoEnable(CCmdUI* pCmdUI);
    afx_msg void OnBold();
    afx_msg void OnItalic();
    afx_msg void OnUnderline();
    afx_msg void OnBoldEnable(CCmdUI* pCmdUI);
    afx_msg void OnItalicEnable(CCmdUI* pCmdUI);
    afx_msg void OnUnderlineEnable(CCmdUI* pCmdUI);
    afx_msg void OnSetColor();
    afx_msg void OnParagraphLeft();
    afx_msg void OnParagraphCenter();
    afx_msg void OnParagraphRight();
    afx_msg void OnParagraphBulleted();
    afx_msg void OnParagraphLeftEnable(CCmdUI* pCmdUI);
    afx_msg void OnParagraphCenterEnable(CCmdUI* pCmdUI);
    afx_msg void OnParagraphRightEnable(CCmdUI* pCmdUI);
    afx_msg void OnParagraphBulletedEnable(CCmdUI* pCmdUI);
    afx_msg void OnCut();
    afx_msg void OnCopy();
    afx_msg void OnPaste();
    afx_msg void OnCutEnable(CCmdUI* pCmdUI);
    afx_msg void OnCopyEnable(CCmdUI* pCmdUI);
    afx_msg void OnPasteEnable(CCmdUI* pCmdUI);
    afx_msg void OnFormatFont();
    afx_msg void OnSelectAll();        
	//}}AFX_MSG
    afx_msg LRESULT OnSelEndOkColor( WPARAM wParam, LPARAM lParam );
	afx_msg void OnSelEndOk();
	afx_msg void OnReturn(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CTextEditor_H__941E8674_3A4D_4483_9142_D57B135D4463__INCLUDED_)
