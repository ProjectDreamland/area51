#if !defined(AFX_FRAMELOG_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_)
#define AFX_FRAMELOG_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FrameLog.h : header file
//

#include "FrameBase.h"

class CViewChannels;
class CViewLog;

/////////////////////////////////////////////////////////////////////////////
// CFrameLog window

class CFrameLog : public CFrameBase
{
// Construction
public:
	CFrameLog();

// Attributes
public:
    CXTSplitterWnd  m_Splitter;
    CViewChannels*  m_pViewChannels;
    CViewLog*       m_pViewLog;
    CXTToolBar      m_wndToolBar;
    CBitmap         m_bToolBarCold;
    CBitmap         m_bToolBarHot;
    CImageList      m_ilToolBarCold;
    CImageList      m_ilToolBarHot;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFrameLog)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CFrameLog();

	// Generated message map functions
protected:
	//{{AFX_MSG(CFrameLog)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnLogViewFixed();
	afx_msg void OnUpdateLogViewFixed(CCmdUI* pCmdUI);
	afx_msg void OnLogViewMessage();
	afx_msg void OnLogViewWarning();
	afx_msg void OnLogViewError();
	afx_msg void OnLogViewRtf();
	afx_msg void OnUpdateLogViewMessage(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLogViewWarning(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLogViewError(CCmdUI* pCmdUI);
	afx_msg void OnUpdateLogViewRtf(CCmdUI* pCmdUI);
	afx_msg void OnLogViewMemory();
	afx_msg void OnUpdateLogViewMemory(CCmdUI* pCmdUI);
	afx_msg void OnLogViewSearch();
	afx_msg void OnUpdateLogViewSearch(CCmdUI* pCmdUI);
	afx_msg void OnPopupLogFind();
	afx_msg void OnPopupLogFindnext();
	afx_msg void OnPopupLogFindprevious();
	afx_msg void OnPopupLogGotonextchannelhighlight();
	afx_msg void OnPopupLogGotoprevchannelhighlight();
	afx_msg void OnPopupLogGotonexterrorwarning();
	afx_msg void OnPopupLogGotopreviouserrorwarning();
	afx_msg void OnPopupLogTogglemark();
	afx_msg void OnPopupLogGotonextmark();
	afx_msg void OnPopupLogGotoprevmark();
    afx_msg void OnPopupLogHidechannel();
    afx_msg void OnLogClear();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FRAMELOG_H__E99C1A15_7B26_4D8D_B872_3366844B77DC__INCLUDED_)
