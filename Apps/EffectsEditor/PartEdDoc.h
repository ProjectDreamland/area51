// PartEdDoc.h : interface of the CPartEdDoc class
//
/////////////////////////////////////////////////////////////////////////////
#include "Auxiliary/fx_core/controller.hpp"
#include "Auxiliary/fx_core/element.hpp"
#include "Auxiliary/fx_core/effect.hpp"
#include "SelectGizmo.hpp"
#include "Auxiliary/fx_core/TextureMgr.hpp"

#if !defined(AFX_PARTEDDOC_H__5F767A16_6E7A_4033_85C9_A499EB2E6561__INCLUDED_)
#define AFX_PARTEDDOC_H__5F767A16_6E7A_4033_85C9_A499EB2E6561__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CPartEdDoc : public CDocument
{
protected: // create from serialization only
	CPartEdDoc();
	DECLARE_DYNCREATE(CPartEdDoc)

// Attributes
public:
    fx_core::effect                             m_Effect;       // the effect
    CList<fx_core::element*, fx_core::element*> m_SelSet;       // active selection set
    xbool                                       m_Animate;      // is the animate button on


// Operations
public:
    void        PopulatePropertyControl     ( void );
    void        UpdateKeyBar                ( void );

    void        SetAnimMode                 ( xbool Animate )   { m_Animate = Animate; m_Effect.SetAnimMode(m_Animate); }
    xbool       GetAnimMode                 ( void )            { return m_Animate; }
    void        SelectAll                   ( void );
    void        SelectNone                  ( void );
    void        SelectInvert                ( void );

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPartEdDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	virtual void OnCloseDocument();
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName );
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName );
	virtual void DeleteContents();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CPartEdDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CPartEdDoc)
	afx_msg void OnFileExport();
	afx_msg void OnFileExportvideoavi();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PARTEDDOC_H__5F767A16_6E7A_4033_85C9_A499EB2E6561__INCLUDED_)
