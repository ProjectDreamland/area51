// TabDoc.h : interface of the CTabDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TABDOC_H__AD8635E6_991D_4145_82D3_9AC9150B6E65__INCLUDED_)
#define AFX_TABDOC_H__AD8635E6_991D_4145_82D3_9AC9150B6E65__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTabDoc : public CDocument
{
protected: // create from serialization only
	DECLARE_DYNCREATE(CTabDoc)

// Attributes
public:
	CTabDoc();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTabDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTabDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTabDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TABDOC_H__AD8635E6_991D_4145_82D3_9AC9150B6E65__INCLUDED_)

