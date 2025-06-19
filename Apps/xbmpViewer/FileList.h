#if !defined(AFX_FILELIST_H__DB3E584B_778C_4705_9E62_BF5A4AB0B2F9__INCLUDED_)
#define AFX_FILELIST_H__DB3E584B_778C_4705_9E62_BF5A4AB0B2F9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FileList.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// file_rec

#include "ListViewCtrlEx.h"

class file_rec
{
public:
                    file_rec    ( ) { LockCount = 0; }
    void            Lock        ( void ) { InterlockedIncrement( &LockCount ); }
    void            Unlock      ( void ) { InterlockedDecrement( &LockCount ); if( LockCount == 0 ) delete this; }

    long            LockCount;
    xstring         Name;
    xbool           GotInfo;
    xbool           InfoDisplayed;
    s32             Size;
    s32             Width;
    s32             Height;
    s32             BitDepth;
    s32             nMips;
    xbitmap::format FormatID;
    xstring         Format;
};

/////////////////////////////////////////////////////////////////////////////
// CFileList window

class CFileList : public CWnd
{
// Construction
public:
    CFileList();
    
public:    
    void OnContextConvertTga();
    void OnContextConvertXbmp();

// Attributes
public:
    CListCtrlEx     m_List;
    CString         m_Path;

// Operations
public:
    void                Thread              ( void );
    xarray<file_rec*>   GetSelected         ( void );
    xstring             GetPath             ( void );

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CFileList)
    protected:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CFileList();

    // Generated message map functions
protected:
    //{{AFX_MSG(CFileList)
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
    //}}AFX_MSG
    afx_msg void OnItemChanged( NMHDR* pHeader, LRESULT* pResult );
    afx_msg void OnColumnClick( NMHDR* pHeader, LRESULT* pResult );
    afx_msg void OnDeleteAllItems( NMHDR* pHeader, LRESULT* pResult );
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg LRESULT OnDirChanged(WPARAM, LPARAM);
    afx_msg LRESULT OnPopulateList(WPARAM, LPARAM);
    afx_msg LRESULT OnRefreshList(WPARAM, LPARAM);

    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FILELIST_H__DB3E584B_778C_4705_9E62_BF5A4AB0B2F9__INCLUDED_)
