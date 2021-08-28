#if !defined(AFX_GridEditCtrl_H__3C02B0B1_A395_11D1_9799_002018026B76__INCLUDED_)
#define AFX_GridEditCtrl_H__3C02B0B1_A395_11D1_9799_002018026B76__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GridEditCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridEditCtrl window

class CGridEditCtrl : public CEdit
{
// Construction
public:
	CGridEditCtrl(int iItem, int iSubItem, CString sInitText);
	virtual ~CGridEditCtrl(){};

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridEditCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CGridEditCtrl)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);	
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:	
	int			m_iItem;
	int			m_iSubItem;	
	CString		m_strInitText;
	BOOL		m_bIsESCKeyDown;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GridEditCtrl_H__3C02B0B1_A395_11D1_9799_002018026B76__INCLUDED_)

