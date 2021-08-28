#if !defined(AFX_MeshWorkspaceDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_)
#define AFX_MeshWorkspaceDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MeshWorkspaceDoc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMeshWorkspaceDoc document
class CMeshViewer_Frame;

class CMeshWorkspaceDoc : public CDocument
{
public:
	CMeshWorkspaceDoc();           // protected constructor used by dynamic creation
    void SetFramePointer(CMeshViewer_Frame* pFrame) { m_pFrameMV = pFrame; }
    CMeshViewer_Frame* GetFramePointer() { return m_pFrameMV; }
    void SetTabParent(CXTTabCtrlBar *pParent) { m_pTabParent = pParent; }
    CXTTabCtrlBar* GetTabParent() { return m_pTabParent; }

private:
    CMeshViewer_Frame*   m_pFrameMV;
    CXTTabCtrlBar*       m_pTabParent;

protected: // create from serialization only
	DECLARE_DYNCREATE(CMeshWorkspaceDoc)

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMeshWorkspaceDoc)
	public:
	virtual void Serialize(CArchive& ar);   // overridden for document i/o
	protected:
	virtual BOOL OnNewDocument();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMeshWorkspaceDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	// Generated message map functions
protected:
	//{{AFX_MSG(CMeshWorkspaceDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MeshWorkspaceDOC_H__177984E8_8C63_4002_A367_8331E5E348CA__INCLUDED_)
