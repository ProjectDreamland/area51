#if !defined(AFX_PROJECTDOC_H__02A79FF9_2FC3_4148_85FD_D56CD4CF63EE__INCLUDED_)
#define AFX_PROJECTDOC_H__02A79FF9_2FC3_4148_85FD_D56CD4CF63EE__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProjectDoc.h : header file
//
#include "BaseDocument.h"
#include "ProjectFrame.h"
#include "ProjectView.h"

/////////////////////////////////////////////////////////////////////////////
// CProjectDoc document
class CPropertyEditorDoc;

class CProjectDoc : public CBaseDocument
{
public:
    CProjectView* GetView();

    const char* GetProjectName  ( void );
    void        CreateNewTheme  ( void );
    void        InsertTheme     ( void );
    void        InitFormFrame   ( CPropertyEditorDoc* pSettingsProp, CPropertyEditorDoc* pPropEditor );
    void        RemoveTheme     ( void );
    
    xbool       FileClose       ( void );
    xbool       FileOpen        ( void );
    xbool       FileSave        ( void );
    xbool       FileNew         ( void );
    
    virtual void    OnProjectRefresh();
    
    xbool        LoadProject     ( const char* fullLevelName );

	virtual BOOL SaveModified(); // return TRUE if ok to continue

    xbool        HandleChangeSave( void );

protected:

    CPropertyEditorDoc* m_pProjectProp;
    CPropertyEditorDoc* m_pSettingsProp;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
public:
	virtual ~CProjectDoc();
	CProjectDoc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CProjectDoc)

/////////////////////////////////////////////////////////////////////////////
public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProjectDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

/////////////////////////////////////////////////////////////////////////////
public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

/////////////////////////////////////////////////////////////////////////////
protected:
	//{{AFX_MSG(CProjectDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


inline CProjectView* CProjectDoc::GetView()
{ 
	POSITION pos = GetFirstViewPosition();
	return (CProjectView*)GetNextView (pos); 
}

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_PROJECTDOC_H__02A79FF9_2FC3_4148_85FD_D56CD4CF63EE__INCLUDED_)
