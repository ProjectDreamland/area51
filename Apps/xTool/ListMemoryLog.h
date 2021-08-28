#if !defined(AFX_LISTMEMORYLOG_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_)
#define AFX_LISTMEMORYLOG_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListMemoryLog.h : header file
//

#include "VirtualListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CListMemoryLog window

class log_memory;

class CListMemoryLog : public CVirtualListCtrl
{
// Construction
public:
	CListMemoryLog();

// Attributes
protected:
    CxToolDoc*      m_pDocument;
    CString         m_FindString;

// Operations
public:
        void        SetDocument                 ( CxToolDoc* pDoc );
const   CString&    GetCellText                 ( log_memory* pEntry, int iCol );
        int         FindNextItem                ( int nItem, const CString& SearchText, int Delta );
        int         FindNextMemMark             ( int nItem, int Delta );
        int         FindNextActiveAllocation    ( int nItem, int Delta );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListMemoryLog)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListMemoryLog();

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
virtual void            OnFocusChanged      ( void );

virtual void            BuildSelectionSet   ( void );

virtual void            OnCopy              ( void );

	// Generated message map functions
protected:
	//{{AFX_MSG(CListMemoryLog)
		// NOTE - the ClassWizard will add and remove member functions here.
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
    afx_msg LRESULT OnMessageFind   ( WPARAM wParam, LPARAM lParam );

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTMEMORYLOG_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_)
