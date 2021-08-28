#if !defined(AFX_LISTLOG_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_)
#define AFX_LISTLOG_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListLog.h : header file
//

#include "VirtualListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CListLog window

class log_entry;

class CListLog : public CVirtualListCtrl
{
// Construction
public:
	CListLog();

// Attributes
protected:
    CxToolDoc*      m_pDocument;
    CString         m_FindString;

// Operations
public:
        void        SetDocument     ( CxToolDoc* pDoc );
const   CString&    GetCellText     ( log_entry* pEntry, int iCol );
        int         FindNextItem    ( int nItem, const CString& SearchText, int Delta );
        int         FindNextError   ( int nItem, int Delta );
        int         FindNextChannel ( int nItem, int Delta );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListLog)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListLog();

virtual bool            OnGetSelected       ( int iRow );
virtual void            OnSetSelected       ( int iRow, bool State );
virtual bool            OnGetMarked         ( int iRow );
virtual void            OnSetMarked         ( int iRow, bool State );
virtual BOOL            OnDrawCell          ( CDC* pDC, CRect& rCell, int iRow, int iCol );
virtual void            OnSort              ( void );
virtual int             OnGetColumnFitWidth ( int iCol );
virtual void            OnFind              ( void );
virtual void            OnFindNext          ( void );
virtual void            OnFindPrevious      ( void );

virtual void            BuildSelectionSet   ( void );

virtual void            OnCopy              ( void );

        void            GotoNextError       ( s32 Direction = 1 );
        void            GotoNextChannel     ( s32 Direction = 1 );

	// Generated message map functions
protected:
	//{{AFX_MSG(CListLog)
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
    afx_msg LRESULT OnMessageFind   ( WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnMessageMarkAll( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTLOG_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_)
