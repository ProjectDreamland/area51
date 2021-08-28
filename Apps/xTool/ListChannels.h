#if !defined(AFX_LISTCHANNELS_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_)
#define AFX_LISTCHANNELS_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ListChannels.h : header file
//

#include "VirtualListCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CListChannels window

class log_channel;

class CListChannels : public CVirtualListCtrl
{
// Construction
public:
	CListChannels();

// Attributes
protected:
    CxToolDoc*      m_pDocument;
    CString         m_FindString;

// Operations
public:
        void        SetDocument     ( CxToolDoc* pDoc );
const   CString&    GetCellText     ( log_channel* pEntry, int iCol );
        int         FindNextItem    ( int nItem, const CString& SearchText, int Delta );
        void        Sort            ( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CListChannels)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CListChannels();

virtual bool            OnGetSelected       ( int iRow );
virtual void            OnSetSelected       ( int iRow, bool State );
virtual bool            OnGetMarked         ( int iRow );
virtual void            OnSetMarked         ( int iRow, bool State );
virtual bool            OnGetChecked        ( int iRow );
virtual void            OnSetChecked        ( int iRow, bool State );
virtual BOOL            OnDrawCell          ( CDC* pDC, CRect& rCell, int iRow, int iCol );
virtual void            OnSort              ( void );
virtual int             OnGetColumnFitWidth ( int iCol );
virtual void            OnFind              ( void );
virtual void            OnFindNext          ( void );
virtual void            OnFindPrevious      ( void );

virtual void            BuildSelectionSet   ( void );

	// Generated message map functions
protected:
	//{{AFX_MSG(CListChannels)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
    afx_msg LRESULT OnMessageFind   ( WPARAM wParam, LPARAM lParam );
    afx_msg LRESULT OnMessageMarkAll( WPARAM wParam, LPARAM lParam );

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_LISTCHANNELS_H__56663DE1_20BD_4C64_BA22_47DE2E4D51EA__INCLUDED_)
