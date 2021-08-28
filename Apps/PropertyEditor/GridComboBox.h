#if !defined(AFX_GridComboBox_H__60972C62_A33C_11D1_9798_002018026B76__INCLUDED_)
#define AFX_GridComboBox_H__60972C62_A33C_11D1_9798_002018026B76__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// GridComboBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGridComboBox window

class CGridComboBox : public CComboBox
{
public:

	CGridComboBox(int iItem, int iSubItem, CStringList *plstItems);	
	virtual ~CGridComboBox();

	// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGridComboBox)
	public:
		virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

protected:
	// Generated message map functions
	//{{AFX_MSG(CGridComboBox)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnNcDestroy();
	afx_msg void OnCloseup();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	int 		m_iItem;
	int 		m_iSubItem;
	CStringList m_lstItems;
	BOOL		m_bVK_ESCAPE;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GridComboBox_H__60972C62_A33C_11D1_9798_002018026B76__INCLUDED_)
