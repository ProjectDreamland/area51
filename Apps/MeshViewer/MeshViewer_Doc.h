#if !defined(AFX_MESHVIEWER_DOC_H__74B893B4_AC9F_4C4F_B6F6_CD0D95CFB262__INCLUDED_)
#define AFX_MESHVIEWER_DOC_H__74B893B4_AC9F_4C4F_B6F6_CD0D95CFB262__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MeshViewer_Doc.h : header file
//
#include "..\Editor\BaseDocument.h"
#include "MeshViewer.hpp"

/////////////////////////////////////////////////////////////////////////////

class CMeshViewer_Doc : public CBaseDocument
{
public:
    void    LoadMesh        ( void );
    xbool   IsPause         ( void );

    void  LoadMeshFromFile   ( CString strFile );
    void  PlayAnimation      ( void );
    void  PauseAnimation     ( void );
    void  CameraFreeFlyMode  ( void );
    void  CameraOrbitMode    ( void );
    xbool IsOrbitMode        ( void );

    virtual void    OnProjectOpen();
    virtual void    OnProjectClose();
    virtual void    OnProjectSave();
    virtual void    OnProjectNew();

    mesh_viewer     m_Viewer;
    xbool           m_bOrbitMode;
    xbool           m_bPlayInPlace;
    xbool           m_bRenderBoneLavels;
    xbool           m_bTakeToBindPose;
    xbool           m_bRenderSkeleton;

/////////////////////////////////////////////////////////////////////////////
// MFC
/////////////////////////////////////////////////////////////////////////////
public:
	virtual ~CMeshViewer_Doc();
	CMeshViewer_Doc();           // protected constructor used by dynamic creation
	DECLARE_DYNCREATE(CMeshViewer_Doc)

public:
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshViewer_Doc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL


/////////////////////////////////////////////////////////////////////////////
#ifdef _DEBUG
public:
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif //_DEBUG
/////////////////////////////////////////////////////////////////////////////
protected:
	// Generated message map functions
	//{{AFX_MSG(CMeshViewer_Doc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MESHVIEWER_DOC_H__74B893B4_AC9F_4C4F_B6F6_CD0D95CFB262__INCLUDED_)
