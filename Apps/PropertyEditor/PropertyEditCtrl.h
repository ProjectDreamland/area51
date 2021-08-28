#if !defined(AFX_PROPERTYEDITCTRL_H__93989EC5_9253_412F_B368_3DD97C604AE2__INCLUDED_)
#define AFX_PROPERTYEDITCTRL_H__93989EC5_9253_412F_B368_3DD97C604AE2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// PropertyEditCtrl.h : header file
//
#include "GridEditCtrl.h"

/////////////////////////////////////////////////////////////////////////////
// CPropertyEditCtrl window

class CPropertyEditCtrl : public CGridEditCtrl
{
// Construction
public:
	CPropertyEditCtrl(int iItem, int iSubItem, CString sInitText);
	virtual ~CPropertyEditCtrl();

	enum PROP_EDIT_TYPE {
		PET_STRING,
		PET_INT,
		PET_FLOAT,
		PET_DEGREE,
		PET_FLOAT_LIST,
		PET_INT_LIST,
		PET_DEGREE_LIST,
	};

	void SetType(PROP_EDIT_TYPE enType, int iList = 0) { m_enumType = enType; m_iListLen = iList; }
	void SetMinMax(float fMin, float fMax) { m_fMin = fMin; m_fMax = fMax; }
	
	//determination code is exposed
    static BOOL EvaluateFloat(CString &strValue, float &f);
    static BOOL EvaluateInt(CString &strValue, int &i);

	static BOOL TestFloat(CString &strValue, float fMin, float fMax);
	static BOOL TestInt(CString &strValue, int iMin, int iMax);
	static BOOL TestDegree(CString &strValue);
	static BOOL TestFloatList(CString &strValue, float fMin, float fMax, int iLength);
	static BOOL TestIntList(CString &strValue, int iMin, int iMax, int iLength);
	static BOOL TestDegreeList(CString &strValue, int iLength);

	static BOOL ParseCompoundString(CString &strValue, CStringList &lstStrings, int iLength);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPropertyEditCtrl)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

	// Generated message map functions
protected:
	//{{AFX_MSG(CPropertyEditCtrl)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	BOOL ValidateFormat();
	BOOL CheckSize(CString &strText);

private:
	PROP_EDIT_TYPE		m_enumType;
	float				m_fMax;		//max chars or value
	float				m_fMin;		//min chars or value
	int					m_iListLen; //length for PET_FLOAT_LIST, PET_DEGREE or PET_INT_LIST
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROPERTYEDITCTRL_H__93989EC5_9253_412F_B368_3DD97C604AE2__INCLUDED_)
