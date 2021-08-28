#if !defined(AFX_STACKLISTBOX_H__81478EE6_EA21_4AA9_A1EA_9135D7FD59DE__INCLUDED_)
#define AFX_STACKLISTBOX_H__81478EE6_EA21_4AA9_A1EA_9135D7FD59DE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// StackListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CStackListBox window

class CStackListBox : public CListBox
{
// Construction
public:
	CStackListBox();

// Attributes
public:

// Operations
public:
    void    SetStackPtrIndex(int i) { m_iStackPtrIndex = i; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CStackListBox)
	public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CStackListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CStackListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
    int m_iStackPtrIndex;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STACKLISTBOX_H__81478EE6_EA21_4AA9_A1EA_9135D7FD59DE__INCLUDED_)
