#if !defined(AFX_MYRICHEDITCTRL_H__DEFD107A_0DA6_46D9_A0F0_0683B7E1091F__INCLUDED_)
#define AFX_MYRICHEDITCTRL_H__DEFD107A_0DA6_46D9_A0F0_0683B7E1091F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyRichEditCtrl.h : header file
//

class CMyRichEditCtrl : public CRichEditCtrl
{
// Construction
// @access Public Member Functions and Variables
public:
	// @cmember
	// constructor
	CMyRichEditCtrl();

// Operations
public:

    BOOL FileOpen               ( CString filename );
    BOOL FileSave               ( CString filename );
    BOOL ImportText             ( CString filename );

    BOOL CanRedo                ( void );
    void Redo                   ( void );
    BOOL IsParagraphLeft        ( void );
    BOOL IsParagraphCenter      ( void );
    BOOL IsParagraphRight       ( void );
    BOOL IsBulleted             ( void );
    BOOL IsBold                 ( void );
    BOOL IsItalic               ( void );
    BOOL IsUnderline            ( void );
    void SetFont                ( void );
    void SetText                ( CFile* pInputFile );
    void ClearAll               ( void );

    void SetRtf                 ( CFile* pInputFile );
	void SetSelectionBold       ( void );
	void SetSelectionItalic     ( void );
	void SetSelectionUnderline  ( void );
	void SetColor               ( void );
	void SetColor               ( COLORREF clr );
	void SetParagraphLeft       ( void );
	void SetParagraphRight      ( void );
	void SetParagraphCenter     ( void );
	void SetParagraphBulleted   ( void );
	void SetFontName            ( CString sFontName );
	void SetFontSize            ( int nFontSize );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMyRichEditCtrl)
	//}}AFX_VIRTUAL

// Implementation
public:
	// @cmember,mfunc
	// destructor
	virtual ~CMyRichEditCtrl();
// @access Private Member Variables
private:
	
	// Generated message map functions
protected:
	//{{AFX_MSG(CMyRichEditCtrl)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYRICHEDITCTRL_H__DEFD107A_0DA6_46D9_A0F0_0683B7E1091F__INCLUDED_)
