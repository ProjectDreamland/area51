#if !defined(AFX_AUDIOFILETREECTRL_H__A46AC15C_96B1_4741_AF5E_3C3999DD6802__INCLUDED_)
#define AFX_AUDIOFILETREECTRL_H__A46AC15C_96B1_4741_AF5E_3C3999DD6802__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AudioFileTreeCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// EXTERNAL CLASSES
/////////////////////////////////////////////////////////////////////////////
class CSoundDoc;

/////////////////////////////////////////////////////////////////////////////
// Audio Tree Ctrl
/////////////////////////////////////////////////////////////////////////////

class CAudioFileTreeCtrl : public CTreeCtrl
{

protected:




public:




/////////////////////////////////////////////////////////////////////////////
// MFC STUFF
/////////////////////////////////////////////////////////////////////////////
	CAudioFileTreeCtrl();
	virtual ~CAudioFileTreeCtrl();

protected:
	//{{AFX_MSG(CAudioFileTreeCtrl)
    afx_msg void OnLclick(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_AUDIOFILETREECTRL_H__A46AC15C_96B1_4741_AF5E_3C3999DD6802__INCLUDED_)
