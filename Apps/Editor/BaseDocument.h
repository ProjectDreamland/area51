#if !defined(AFX_BASEDOCUMENT_H__1D7E5677_F8BC_4827_91F2_1272198267B0__INCLUDED_)
#define AFX_BASEDOCUMENT_H__1D7E5677_F8BC_4827_91F2_1272198267B0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// BaseDocument.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBaseDocument document

class CBaseDocument : public CDocument
{
// Operations
public:

    virtual void    OnInitialize( void ){};
    virtual void    OnProjectOpen() {};
    virtual void    OnProjectClose() {};
    virtual void    OnProjectSave() {};
    virtual void    OnProjectNew() {};
    virtual void    OnProjectImport() {};
    virtual void    OnProjectRefresh() {};

    void            SetDocumentActive( xbool bActive ){ m_bDocActive=bActive; }
    xbool           IsDocumentActive ( void )         { return m_bDocActive; }

protected:

    xbool               m_bDocActive;

/////////////////////////////////////////////////////////////////////////////
// MFC Specific stuff
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
public:
	virtual ~CBaseDocument();
	CBaseDocument();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CBaseDocument)

/////////////////////////////////////////////////////////////////////////////
protected:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBaseDocument)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
// Implementation
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	// Generated message map functions
	//{{AFX_MSG(CBaseDocument)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_BASEDOCUMENT_H__1D7E5677_F8BC_4827_91F2_1272198267B0__INCLUDED_)
